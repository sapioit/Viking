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
        poll.Schedule(&(*contexts_.back()), flags);
    } catch (const SysEpoll::PollError &) {
        throw;
    }
}

void IO::Scheduler::Remove(Channel *channel) noexcept {
    poll.Remove(channel);
    contexts_.erase(std::remove_if(contexts_.begin(), contexts_.end(), [&channel](auto &ctx) {
                        return channel == &*ctx;
                    }), contexts_.end());
}

void IO::Scheduler::Run() noexcept {
    if (contexts_.size() == 0)
        return;
    const auto events = poll.Wait(contexts_.size());
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
            if (unlikely(!(event.context->flags & IO::Channel::Full)))
                poll.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::EdgeTriggered));
            event.context->flags &= ~IO::Channel::Full;
            ProcessWrite(event.context);
            continue;
        }

        if (CanRead(event)) {
            if (auto callback_response = read_callback(event.context->socket.get())) {
                /* We schedule the item in the epoll instance with just the Write flag,
                 * since it already has the others
                 */
                poll.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::Write));
                auto &front = *callback_response.Front();
                std::type_index type = typeid(front);
                if (type == typeid(MemoryBuffer) || type == typeid(UnixFile)) {
                    AddSchedItem(event.context, callback_response, true);
                } else {
                    AddSchedItem(event.context, callback_response, false);
                    event.context->flags |= IO::Channel::Barrier;
                }
            }
            continue;
        }
    }
}

void IO::Scheduler::AddSchedItem(IO::Channel *channel, ScheduleItem &item, bool back) noexcept {
    if (back)
        channel->queue.PutBack(std::move(item));
    else
        channel->queue.PutAfterFirstIntact(std::move(item));
}

void IO::Scheduler::ProcessWrite(IO::Channel *channel) noexcept {
    try {
        while (channel->queue && !(channel->flags & IO::Channel::Full)) {
            if (channel->flags & IO::Channel::Barrier) {
                poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                return;
            }
            FillChannel(channel);
            if (!channel->queue) {
                if (channel->queue.KeepFileOpen())
                    poll.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
                else
                    Remove(channel);
            } else {
                if (!(channel->flags & IO::Channel::Full))
                    poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
            }
        }
    } catch (...) {
        Remove(channel);
    }
}

// TODO fix the support for async buffers
void IO::Scheduler::FillChannel(IO::Channel *channel) {
    auto &front = *channel->queue.Front();
    std::type_index sched_item_type = typeid(front);

    if (sched_item_type == typeid(MemoryBuffer)) {
        MemoryBuffer *mem_buffer = reinterpret_cast<MemoryBuffer *>(channel->queue.Front());
        try {
            if (const auto written = channel->socket->WriteSome(mem_buffer->data))
                channel->queue.UpdateFrontMemoryBuffer(written);
            else
                channel->flags |= IO::Channel::Full;
        } catch (...) {
            throw WriteError{};
        }

    } else if (sched_item_type == typeid(UnixFile)) {
        UnixFile *unix_file = reinterpret_cast<UnixFile *>(channel->queue.Front());
        try {
            auto size_left = unix_file->SizeLeft();
            if (auto written = unix_file->SendTo(channel->socket->GetFD())) {
                if (written == size_left)
                    channel->queue.RemoveFront();
            } else {
                channel->flags |= IO::Channel::Full;
            }
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
                channel->queue.ReplaceFront(std::move(buffer));
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
