/*
Copyright (C) 2015 Voinea Constantin Vladimir

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#include <io/schedulers/io_scheduler.h>
#include <io/schedulers/sys_epoll.h>
#include <io/buffers/utils.h>
#include <misc/common.h>
#include <misc/debug.h>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <typeindex>

using namespace IO;

class Scheduler::SchedulerImpl {
    struct SocketNotFound {
        const SysEpoll::Event *event;
    };
    struct WriteError {};

    std::vector<std::unique_ptr<Channel>> channels;
    SysEpoll poll;
    Scheduler::ReadCallback read_callback;
    Scheduler::BarrierCallback barrier_callback;
    Scheduler::BeforeRemovingCallback before_removing;

    public:
    SchedulerImpl() = default;
    SchedulerImpl(std::unique_ptr<Socket> sock, Scheduler::ReadCallback read_callback,
                  Scheduler::BarrierCallback barrier_callback, Scheduler::BeforeRemovingCallback before_removing)
        : read_callback(read_callback), barrier_callback(barrier_callback), before_removing(before_removing) {
        try {
            Add(std::move(sock),
                static_cast<std::uint32_t>(SysEpoll::Read) | static_cast<std::uint32_t>(SysEpoll::Termination));
        } catch (const SysEpoll::PollError &) {
            throw;
        }
    }

    void Add(std::unique_ptr<Socket> socket, std::uint32_t flags) {
        try {
            auto ctx = std::make_unique<IO::Channel>(std::move(socket));
            poll.Schedule(ctx.get(), flags);
            channels.emplace_back(std::move(ctx));
        } catch (const SysEpoll::PollError &) {
            throw;
        }
    }

    void Run() noexcept {
        if (channels.size() == 0)
            return;
        const auto events = poll.Wait(channels.size());
        for (const auto &event : events) {
            poll.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::EdgeTriggered));
            event.context->marked_for_removal = false;
            if (CanTerminate(event.description)) {
                event.context->journal.push_back(event.description);
                Remove(event.context);
                continue;
            }

            if (event.context->socket->IsAcceptor()) {
                event.context->journal.push_back(event.description);
                AddNewConnections(event.context);
                continue;
            }

            if (CanWrite(event.description)) {
                event.context->journal.push_back(event.description);
                ProcessWrite(event.context);
            }

            if (CanRead(event.description)) {
                event.context->journal.push_back(event.description);
                try {
                    if (auto callback_response = read_callback(event.context)) {
                        poll.Modify(event.context, static_cast<std::uint32_t>(SysEpoll::Write));
                        auto &front = *callback_response.Front();
                        std::type_index type = typeid(front);
                        if (type == typeid(MemoryBuffer) || type == typeid(UnixFile))
                            EnqueueItem(event.context, callback_response, true);
                        else
                            EnqueueItem(event.context, callback_response, false);
                    }
                } catch (const IO::Socket::ConnectionClosedByPeer &) {
                    Remove(event.context);
                }
            }
        }
        channels.erase(std::remove_if(channels.begin(), channels.end(), [](auto &ch) {
                           return ch->marked_for_removal;
                       }), channels.end());
    }

    void AddNewConnections(const Channel *channel) noexcept {
        do {
            try {
                auto new_connection = channel->socket->Accept();
                if (*new_connection) {
                    new_connection->MakeNonBlocking();
                    Add(std::move(new_connection),
                        static_cast<std::uint32_t>(SysEpoll::Read) | static_cast<std::uint32_t>(SysEpoll::Termination));
                } else
                    break;
            } catch (SysEpoll::PollError &) {
            }
        } while (true);
    }

    void ProcessWrite(Channel *channel) noexcept {
        try {
            bool filled = false;
            while (channel->queue && !filled) {
                if (channel->queue.IsFrontAsync()) {
                    auto new_sync_buffer = barrier_callback(channel->queue);
                    if (*new_sync_buffer) {
                        channel->queue.ReplaceFront(std::move(new_sync_buffer));
                    } else {
                        poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                        return;
                    }
                }
                auto result = FillChannel(channel);
                if (!(channel->queue)) {
                    if (channel->queue.KeepFileOpen()) {
                        poll.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
                        poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                    } else {
                        /* There is a chance that the channel has pending data right now, so we must not close it yet.
                         * Instead we mark it, and the next time we poll, if it's not in the event list, we remove it */
                        channel->marked_for_removal = true;
                        return;
                    }
                }
                filled = result;
            }
            if (!filled)
                poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
        } catch (...) {
            Remove(channel);
        }
    }

    bool FillChannel(Channel *channel) {
        auto &front = *channel->queue.Front();
        std::type_index sched_item_type = typeid(front);

        if (sched_item_type == typeid(MemoryBuffer)) {
            MemoryBuffer *mem_buffer = reinterpret_cast<MemoryBuffer *>(channel->queue.Front());
            try {
                if (const auto written = channel->socket->WriteSome(mem_buffer->data) > 0) {
                    if (written == mem_buffer->data.size())
                        channel->queue.RemoveFront();
                    else
                        std::vector<char>(mem_buffer->data.begin() + written, mem_buffer->data.end())
                            .swap(mem_buffer->data);
                } else {
                    return true;
                }
            } catch (...) {
                debug("Caught exception when writing a memory buffer");
                throw WriteError{};
            }

        } else if (sched_item_type == typeid(UnixFile)) {
            UnixFile *unix_file = reinterpret_cast<UnixFile *>(channel->queue.Front());
            try {
                auto size_left = unix_file->SizeLeft();
                if (const auto written = unix_file->SendTo(channel->socket->GetFD())) {
                    if (written == size_left)
                        channel->queue.RemoveFront();
                } else {
                    return true;
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
                    return FillChannel(channel);
                } catch (...) {
                    throw WriteError{};
                }
            } catch (...) {
                debug("Caught exception when writing a unix file");
                throw WriteError{};
            }
        }
        return false;
    }

    void EnqueueItem(Channel *c, ScheduleItem &item, bool back) noexcept {
        back ? c->queue.PutBack(std::move(item)) : c->queue.PutAfterFirstIntact(std::move(item));
    }

    void Remove(Channel *c) noexcept {
        before_removing(c);
        poll.Remove(c);
        channels.erase(std::remove_if(channels.begin(), channels.end(), [&c](auto &ctx) { return c == &*ctx; }),
                       channels.end());
    }

    inline bool CanWrite(std::uint32_t ev) const noexcept { return (ev & SysEpoll::Write); }
    inline bool CanRead(std::uint32_t ev) const noexcept { return (ev & SysEpoll::Read); }
    inline bool CanTerminate(std::uint32_t ev) const noexcept {
        return (ev & SysEpoll::Termination) || (ev & SysEpoll::Error);
    }
};

Scheduler::Scheduler() {
    try {
        impl = new SchedulerImpl();
    } catch (...) {
        throw;
    }
}

Scheduler::Scheduler(std::unique_ptr<Socket> sock, Scheduler::ReadCallback read_callback,
                     BarrierCallback barrier_callback, BeforeRemovingCallback before_removing) {
    try {
        impl = new SchedulerImpl(std::move(sock), read_callback, barrier_callback, before_removing);
    } catch (...) {
        throw;
    }
}

Scheduler::~Scheduler() { delete impl; }

void Scheduler::Add(std::unique_ptr<IO::Socket> socket, uint32_t flags) {
    try {
        impl->Add(std::move(socket), flags);
    } catch (const SysEpoll::PollError &) {
        throw;
    }
}

void Scheduler::Run() noexcept { impl->Run(); }

Scheduler &Scheduler::operator=(Scheduler &&other) {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

Scheduler::Scheduler(Scheduler &&other) { *this = std::move(other); }
