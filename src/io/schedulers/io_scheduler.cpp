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

    void Remove(Channel *c) noexcept {
        before_removing(c);
        poll.Remove(c);
        channels.erase(std::remove_if(channels.begin(), channels.end(), [&c](auto &ctx) { return c == &*ctx; }),
                       channels.end());
    }

    template <typename T> T CreateOrRemoveConnections(T begin, T end) {
        auto after_terminate =
            std::partition(begin, end, [](auto &ev) { return ev.CanTerminate() || ev.context->socket->IsAcceptor(); });

        std::for_each(begin, after_terminate, [this](auto &ev) {
            if (ev.context->socket->IsAcceptor())
                this->AddNewConnections(ev.context);
            if (ev.CanTerminate())
                this->Remove(ev.context);
        });

        return after_terminate;
    }

    public:
    void Run() noexcept {
        if (channels.size() == 0)
            return;
        auto events = poll.Wait(channels.size());

        auto first_event = CreateOrRemoveConnections(events.begin(), events.end());
        events.erase(events.begin(), first_event);

        auto last_to_read_from = std::partition(events.begin(), events.end(), [](auto &ev) { return ev.CanRead(); });

        for (auto it = events.begin(); it != last_to_read_from; ++it) {
            try {
                if (auto callback_response = read_callback(it->context)) {
                    poll.Modify(it->context, static_cast<std::uint32_t>(SysEpoll::Write));
                    auto &front = *callback_response.Front();
                    std::type_index type = typeid(front);
                    this->EnqueueItem(it->context, callback_response,
                                      type == typeid(MemoryBuffer) || type == typeid(UnixFile) ? true : false);
                }
            } catch (const IO::Socket::ConnectionClosedByPeer &) {
                this->Remove(it->context);
                it->context->flags |= Channel::Tainted;
            }
        }

        std::for_each(events.begin(), events.end(), [this](auto &ev) {
            if (!(ev.context->flags & Channel::Tainted))
                this->ProcessWrite(ev.context);
        });
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
                        Remove(channel);
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
                if (const auto written = channel->socket->WriteSome(mem_buffer->data)) {
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
