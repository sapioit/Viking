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
        channels.emplace_back(std::make_unique<IO::Channel>(std::move(socket)));
        poll.Schedule(&(*channels.back()), flags);
    } catch (const SysEpoll::PollError &) {
        throw;
    }
}

void IO::Scheduler::Remove(Channel *channel) noexcept {
    poll.Remove(channel);
    channels.erase(std::remove_if(channels.begin(), channels.end(), [&channel](auto &ctx) { return channel == &*ctx; }),
                   channels.end());
}

void IO::Scheduler::Run() noexcept {
    if (channels.size() == 0)
        return;
    const auto events = poll.Wait(channels.size());
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
            if (unlikely(!(event.context->flags & Channel::Full)))
                poll.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::EdgeTriggered));
            event.context->flags &= ~Channel::Full;
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
                    QueueItem(event.context, callback_response, true);
                } else {
                    QueueItem(event.context, callback_response, false);
                    event.context->flags |= Channel::Barrier;
                }
            }
            continue;
        }
    }
}

void IO::Scheduler::QueueItem(Channel *channel, ScheduleItem &item, bool back) noexcept {
    back ? channel->queue.PutBack(std::move(item)) : channel->queue.PutAfterFirstIntact(std::move(item));
}

void IO::Scheduler::ProcessWrite(Channel *channel) noexcept {
    try {
        while (channel->queue && !(channel->flags & Channel::Full)) {
            if (channel->flags & Channel::Barrier) {
                poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                return;
            }
            FillChannel(channel);
            if (!channel->queue) {
                if (channel->queue.KeepFileOpen()) {
                    poll.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
                } else {
                    Remove(channel);
                    return;
                }
            }
            if (!(channel->flags & Channel::Full) || channel->flags & Channel::Barrier)
                poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
        }
    } catch (...) {
        Remove(channel);
    }
}

// TODO fix the support for async buffers
void IO::Scheduler::FillChannel(Channel *channel) {
    auto &front = *channel->queue.Front();
    std::type_index sched_item_type = typeid(front);

    if (sched_item_type == typeid(MemoryBuffer)) {
        MemoryBuffer *mem_buffer = reinterpret_cast<MemoryBuffer *>(channel->queue.Front());
        try {
            if (const auto written = channel->socket->WriteSome(mem_buffer->data)) {
                if (written == mem_buffer->data.size()) {
                    channel->queue.RemoveFront();
                    if (channel->queue.BuffersLeft() && channel->queue.IsFrontAsync())
                        channel->flags |= Channel::Barrier;
                } else
                    std::vector<char>(mem_buffer->data.begin() + written, mem_buffer->data.end())
                        .swap(mem_buffer->data);
            } else {
                channel->flags |= Channel::Full;
            }
        } catch (...) {
            throw WriteError{};
        }

    } else if (sched_item_type == typeid(UnixFile)) {
        UnixFile *unix_file = reinterpret_cast<UnixFile *>(channel->queue.Front());
        try {
            auto size_left = unix_file->SizeLeft();
            if (auto written = unix_file->SendTo(channel->socket->GetFD())) {
                if (written == size_left) {
                    channel->queue.RemoveFront();
                    if (channel->queue.BuffersLeft() && channel->queue.IsFrontAsync())
                        channel->flags |= Channel::Barrier;
                }
            } else {
                channel->flags |= Channel::Full;
            }

        } catch (const UnixFile::DIY &e) {
            /* This is how Linux tells you that you'd better do it yourself in userspace.
             * We need to replace this item with a MemoryBuffer version of this
             * data, at the right offset.
             */
            try {
                debug("diy");
                auto buffer = From(*e.ptr);
                channel->queue.ReplaceFront(std::move(buffer));
                FillChannel(channel);
            } catch (...) {
                throw WriteError{};
            }
        } catch (...) {
            throw WriteError{};
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
