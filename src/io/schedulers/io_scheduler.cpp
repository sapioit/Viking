#include <io/schedulers/io_scheduler.h>
#include <io/buffers/utils.h>
#include <misc/common.h>
#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <typeindex>

IO::Scheduler::Scheduler(std::unique_ptr<Socket> sock, IO::Scheduler::ReadCallback callback,
                         BarrierCallback barrier_callback)
    : read_callback(callback), barrier_callback(barrier_callback) {
    try {
        Add(std::move(sock),
            static_cast<std::uint32_t>(SysEpoll::Read) | static_cast<std::uint32_t>(SysEpoll::Termination));
    } catch (const SysEpoll::PollError &) {
        throw;
    }
}

void IO::Scheduler::Add(std::unique_ptr<IO::Socket> socket, uint32_t flags) {
    try {
        contexts_.emplace_back(std::make_unique<IO::Channel>(std::move(socket)));
        poller_.Schedule(&(*contexts_.back()), flags);
    } catch (const SysEpoll::PollError &) {
        throw;
    }
}

void IO::Scheduler::Remove(Channel *channel) noexcept {
    schedule_map_.erase(channel->socket->GetFD());
    poller_.Remove(channel);
    contexts_.erase(std::remove_if(contexts_.begin(), contexts_.end(), [&channel](auto &ctx) {
                        return channel == &*ctx;
                    }), contexts_.end());
}

void IO::Scheduler::Run() noexcept {
    if (contexts_.size() == 0)
        return;
    const auto events = poller_.Wait(contexts_.size());
    for (const auto &event : events) {
        if (event.context->socket->IsAcceptor()) {
            AddNewConnections(event.context);
            continue;
        }

        if (CanTerminate(event)) {
            Remove(event.context);
            continue;
        }
        if (CanWrite(event) && !(event.context->flags & IO::Channel::Barrier)) {
            ProcessWrite(event.context);
            continue;
        }

        if (CanRead(event)) {
            Resolution callback_response = read_callback(event.context->socket.get());
            if (callback_response) {
                /* We schedule the item in the epoll instance with just the Write flag,
                 * since it already has the others
                 */
                poller_.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::Write));
                auto &cb_front = *callback_response.Front();
                std::type_index type = typeid(cb_front);
                if (type == typeid(MemoryBuffer) || type == typeid(UnixFile)) {
                    AddSchedItem(event, std::move(callback_response), true);
                } else {
                    AddSchedItem(event, std::move(callback_response), false);
                    event.context->flags |= IO::Channel::Barrier;
                }
            }
            continue;
        }
    }
}

void IO::Scheduler::AddSchedItem(const SysEpoll::Event &ev, ScheduleItem item, bool back) noexcept {
    auto item_it = schedule_map_.find(ev.context->socket->GetFD());
    if (item_it == schedule_map_.end())
        schedule_map_.emplace(std::make_pair(ev.context->socket->GetFD(), std::move(item)));
    else {
        if (back)
            item_it->second.PutBack(std::move(item));
        else
            item_it->second.PutAfterFirstIntact(std::move(item));
    }
}

void IO::Scheduler::ProcessWrite(IO::Channel *channel) noexcept {
    for (auto sched_item_it = schedule_map_.find(channel->socket->GetFD()); sched_item_it != schedule_map_.end();
         sched_item_it = schedule_map_.find(channel->socket->GetFD())) {
        auto &item = sched_item_it->second;
        try {
            ConsumeItem(item, channel);
            if (!item) {
                if (item.KeepFileOpen()) {
                    schedule_map_.erase(channel->socket->GetFD());
                    poller_.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
                    /* Also, if we don't close the socket, we might want to switch back to level-triggered
                     * mode, or else the client might keep sending requests and we won't get anything.
                     */
                } else {
                    Remove(channel);
                    return;
                }
            }
        } catch (...) {
            Remove(channel);
            return;
        }
    }
}

void IO::Scheduler::ConsumeItem(ScheduleItem &item, IO::Channel *channel) {
    auto sched_item_front = item.Front();
    std::type_index sched_item_type = typeid(*sched_item_front);
    if (unlikely(channel->flags & IO::Channel::Barrier && sched_item_type != typeid(MemoryBuffer) &&
                 sched_item_type != typeid(UnixFile))) {
        std::unique_ptr<MemoryBuffer> new_buffer;
        if (!barrier_callback(item, new_buffer)) {
            poller_.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
            return;
        } else {
            item.ReplaceFront(std::move(new_buffer));
            sched_item_front = item.Front();
            poller_.Modify(channel, static_cast<std::uint32_t>(SysEpoll::EdgeTriggered));
            channel->flags |= ~IO::Channel::Barrier;
        }
    }

    if (sched_item_type == typeid(MemoryBuffer)) {
        MemoryBuffer *mem_buffer = reinterpret_cast<MemoryBuffer *>(sched_item_front);
        try {
            if (const auto written = channel->socket->WriteSome(mem_buffer->data))
                item.UpdateFrontMemoryBuffer(written);
        } catch (...) {
            throw WriteError{};
        }

    } else if (sched_item_type == typeid(UnixFile)) {
        UnixFile *unix_file = reinterpret_cast<UnixFile *>(sched_item_front);
        try {
            auto size_left = unix_file->SizeLeft();
            if (auto written = unix_file->SendTo(channel->socket->GetFD()))
                if (written == size_left)
                    item.RemoveFront();
        } catch (const UnixFile::BrokenPipe &) {
            throw WriteError{};
        } catch (const UnixFile::BadFile &) {
            throw WriteError{};
        } catch (const UnixFile::DIY &e) {
            /* This is how Linux tells you that you'd better do it yourself in userspace.
             * We need to replace this item with a MemoryBuffer version of this
             * data, at the right offset.
             */
            try {
                debug("diy");
                auto buffer = From(*e.ptr);
                item.ReplaceFront(std::move(buffer));
            } catch (const UnixFile::BadFile &) {
                throw WriteError{};
            }
        }
    }
}

void IO::Scheduler::AddNewConnections(const Channel *channel) noexcept {
    do {
        try {
            auto new_connection = channel->socket->Accept();
            if (*new_connection) {
                new_connection->MakeNonBlocking();
                Scheduler::Add(std::move(new_connection), static_cast<std::uint32_t>(SysEpoll::Read) |
                                                              static_cast<std::uint32_t>(SysEpoll::Termination));
            } else
                break;
        } catch (SysEpoll::PollError &) {
        }
    } while (true);
}

bool IO::Scheduler::CanWrite(const SysEpoll::Event &event) const noexcept {
    return (event.description & static_cast<std::uint32_t>(SysEpoll::Write));
}

bool IO::Scheduler::CanRead(const SysEpoll::Event &event) const noexcept {
    return (event.description & static_cast<std::uint32_t>(SysEpoll::Read));
}

bool IO::Scheduler::CanTerminate(const SysEpoll::Event &event) const noexcept {
    return (event.description & static_cast<std::uint32_t>(SysEpoll::Termination) ||
            event.description & static_cast<std::uint32_t>(SysEpoll::Error));
}

bool IO::Scheduler::HasDataScheduled(int file_descriptor) const noexcept {
    auto item = schedule_map_.find(file_descriptor);
    return (item != schedule_map_.end());
}
