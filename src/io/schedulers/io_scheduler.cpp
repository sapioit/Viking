#include <io/schedulers/io_scheduler.h>
#include <io/buffers/utils.h>
#include <misc/common.h>
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

void IO::Scheduler::Remove(const Channel *channel) noexcept {
    schedule_.erase(channel->socket->GetFD());
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
        if (CanWrite(event)) {
            ProcessWrite(event.context);
            continue;
        }

        if (CanRead(event)) {
            //&& (!HasDataScheduled(event.context->socket->GetFD()))) {
            Resolution callback_response = read_callback(event.context->socket.get());
            if (callback_response) {
                /* We schedule the item in the epoll instance with just the Write flag,
                 * since it already has the others
                 */
                poller_.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::Write));
                auto &cb_front = *callback_response.Front();
                std::type_index type = typeid(cb_front);
                if (type == typeid(MemoryBuffer) || type == typeid(MemoryBuffer)) {
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
    auto item_it = schedule_.find(ev.context->socket->GetFD());
    if (item_it == schedule_.end())
        schedule_.emplace(std::make_pair(ev.context->socket->GetFD(), std::move(item)));
    else if (back)
        item_it->second.PutBack(std::move(item));
    else
        item_it->second.PutAfterFirstIntact(std::move(item));
}

void IO::Scheduler::ScheduledItemFinished(const IO::Channel *channel, ScheduleItem &sched_item) {
    if (!sched_item.KeepFileOpen()) {
        Remove(channel);
    } else {
        schedule_.erase(channel->socket->GetFD());
        poller_.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
        /* Also, if we don't close the socket, we might want to switch back to level-triggered
         * mode, or else the client might keep sending requests and we won't get anything.
         */
    }
}
void IO::Scheduler::ProcessWrite(IO::Channel *channel) noexcept {
    do {
        auto sch_it = schedule_.find(channel->socket->GetFD());
        if (sch_it == schedule_.end())
            return;
        auto &sched_item = sch_it->second;
        auto sched_item_front = sched_item.Front();
        std::type_index sched_item_type = typeid(*sched_item_front);
        if (unlikely(channel->flags & IO::Channel::Barrier && sched_item_type != typeid(MemoryBuffer) &&
                     sched_item_type != typeid(UnixFile))) {
            std::unique_ptr<MemoryBuffer> new_buffer;
            if (!barrier_callback(sched_item, new_buffer)) {
                poller_.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                break;
            } else {
                sched_item.ReplaceFront(std::move(new_buffer));
                sched_item_front = sched_item.Front();
                poller_.Modify(channel, static_cast<std::uint32_t>(SysEpoll::EdgeTriggered));
                channel->flags |= ~IO::Channel::Barrier;
            }
        }

        if (sched_item_type == typeid(MemoryBuffer)) {
            MemoryBuffer *mem_buffer = dynamic_cast<MemoryBuffer *>(sched_item_front);
            auto &data = mem_buffer->data;
            try {
                const auto written = channel->socket->WriteSome(data);
                if (written == 0)
                    break;
                if (written == data.size())
                    sched_item.RemoveFront();
                else
                    DataType(data.begin() + written, data.end()).swap(data);
            } catch (const Socket::WriteError &) {
                Remove(channel);
                break;
            } catch (const Socket::ConnectionClosedByPeer &) {
                Remove(channel);
                break;
            }
        } else if (sched_item_type == typeid(UnixFile)) {
            UnixFile *unix_file = dynamic_cast<UnixFile *>(sched_item_front);
            try {
                auto size_left = unix_file->SizeLeft();
                auto written = unix_file->SendTo(channel->socket->GetFD());
                if (written == 0)
                    break;
                if (written == size_left)
                    sched_item.RemoveFront();
            } catch (const UnixFile::BrokenPipe &) {
                /* Received EPIPE (in this case, with sendfile, the connection was
                 * closed by the peer).
                 */
                Scheduler::Remove(channel);
                break;
            } catch (const UnixFile::BadFile &) {
                Scheduler::Remove(channel);
                break;
            } catch (const UnixFile::DIY &e) {
                /* This is how Linux tells you that you'd better do it yourself in userspace.
                 * We need to replace this item with a MemoryBuffer version of this
                 * data, at the right offset.
                 */
                try {
                    auto buffer = From(*e.ptr);
                    sched_item.ReplaceFront(std::move(buffer));
                } catch (const UnixFile::BadFile &) {
                    /* For some reason, we could not generate a MemoryBuffer out of this */
                    Scheduler::Remove(channel);
                    break;
                }
            }
        }
        if (!sched_item) {
            ScheduledItemFinished(channel, sched_item);
            break;
        }
    } while (true);
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
    auto item = schedule_.find(file_descriptor);
    return (item != schedule_.end());
}
