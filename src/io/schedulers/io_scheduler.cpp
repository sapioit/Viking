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
template <typename BiIt, typename UnPred> auto gather(BiIt f, BiIt l, BiIt p, UnPred s) -> std::pair<BiIt, BiIt> {
    return {std::stable_partition(f, p, [s](auto p) { return !s(p); }), std::stable_partition(p, l, s)};
}

class Scheduler::SchedulerImpl {
    struct SocketNotFound {
        const SysEpoll::Event *event;
    };
    struct WriteError {};

    std::vector<std::unique_ptr<Channel>> channels;
    SysEpoll poll;
    Scheduler::ReadCallback read_callback;
    Scheduler::BarrierCallback barrier_callback;

    public:
    void Remove(Channel *c) noexcept {
        poll.Remove(c);
        channels.erase(std::remove_if(channels.begin(), channels.end(), [&c](auto &ctx) { return c == &*ctx; }),
                       channels.end());
    }

    template <typename T, typename C> void Remove(T begin, T end, C condition) {
        if (begin == end)
            return;
        auto first_ok = std::partition(begin, end, condition);
        for (auto it = begin; it != first_ok; ++it) {
            poll.Remove(it->get());
        }
        channels.erase(channels.begin(), first_ok);
    }

    private:
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

    template <typename T> auto MakeChannels(T begin, T end) const noexcept {
        std::vector<Channel *> event_channels;
        event_channels.reserve(end - begin);
        std::for_each(begin, end,
                      [&event_channels](const SysEpoll::Event &ev) { event_channels.push_back(ev.context); });
        return event_channels;
    }

    public:
    enum write_result { full, not_full, error, empty };
    void Run() noexcept {
        if (channels.size() == 0)
            return;
        auto events = poll.Wait(channels.size());

        auto first_event = CreateOrRemoveConnections(events.begin(), events.end());
        events.erase(events.begin(), first_event);

        if (events.size()) {
            auto last_to_read_from =
                std::partition(events.begin(), events.end(), [](auto &ev) { return ev.CanRead(); });

            if (events.begin() != last_to_read_from) {
                auto read_events = MakeChannels(events.begin(), last_to_read_from);
                read_callback(read_events);
            }

            std::vector<Channel *> to_remove;
            auto written = true;
            while (written) {
                written = false;
                for (auto it = events.begin(); it != events.end(); ++it) {
                    if (it->context->flags & Channel::Tainted) {
                        to_remove.push_back(it->context);
                        continue;
                    }
                    if (it->CanWrite()) {
                        const write_result result = this->WriteToChannel(it->context);
                        debug(result);
                        if(!it->context->queue) {
                            if (it->context->queue.KeepFileOpen()) {
                                poll.Modify(it->context, ~static_cast<std::uint32_t>(SysEpoll::Write));
                                //poll.Modify(it->context, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                            } else {
                                it->description &= SysEpoll::Write;
                                to_remove.push_back(it->context);
                                continue;
                            }
                        }
                        switch (result) {
                        case full:
                            it->description &= SysEpoll::Write;
                            written = true;
                            break;
                        case error:
                            to_remove.push_back(it->context);
                            continue;
                        case empty:
                            it->description &= SysEpoll::Write;
                            break;
                        default:
                            break;
                        }

                    }
                }
            }

            for (auto ptr : to_remove)
                Remove(ptr);
        }
    }

    write_result WriteToChannel(Channel *channel) noexcept {
        if (!channel->queue)
            return empty;
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
                    return full;
                }
            } catch (...) {
                debug("Caught exception when writing a memory buffer");
                return error;
            }

        } else if (sched_item_type == typeid(UnixFile)) {
            UnixFile *unix_file = reinterpret_cast<UnixFile *>(channel->queue.Front());
            try {
                auto size_left = unix_file->SizeLeft();
                if (const auto written = unix_file->SendTo(channel->socket->GetFD())) {
                    if (written == size_left)
                        channel->queue.RemoveFront();
                } else {
                    return full;
                }
            } catch (const UnixFile::DIY &e) {
                /* This is how Linux tells you that you'd better do it yourself in userspace.
                 * We need to replace this item with a MemoryBuffer version of this
                 * data, at the right offset.
                 */
                debug("diy");
                auto buffer = From(*e.ptr);
                channel->queue.ReplaceFront(std::move(buffer));
                return WriteToChannel(channel);
            } catch (...) {
                debug("Caught exception when writing a unix file");
                return error;
            }
        }
        return not_full;
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
                auto result = WriteToChannel(channel);
                if (!(channel->queue)) {
                    if (channel->queue.KeepFileOpen()) {
                        poll.Modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
                        poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                    } else {
                        /* There is a chance that the channel has pending data right now, so we must not close it
                         * yet.
                         * Instead we mark it, and the next time we poll, if it's not in the event list, we remove
                         * it */
                        Remove(channel);
                        return;
                    }
                }
                filled = result;
                poll.Modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
            }
        } catch (...) {
            Remove(channel);
        }
    }

    SchedulerImpl() = default;
    SchedulerImpl(std::unique_ptr<Socket> sock, Scheduler::ReadCallback read_callback,
                  Scheduler::BarrierCallback barrier_callback)
        : read_callback(read_callback), barrier_callback(barrier_callback) {
        try {
            Add(std::move(sock), SysEpoll::Read | SysEpoll::Write | SysEpoll::Termination);
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

    void AddNewConnections(const Channel *channel) noexcept {
        do {
            try {
                auto new_connection = channel->socket->Accept();
                if (*new_connection) {
                    new_connection->MakeNonBlocking();
                    Add(std::move(new_connection), SysEpoll::Read | SysEpoll::Write | SysEpoll::Termination);
                } else
                    break;
            } catch (SysEpoll::PollError &) {
            }
        } while (true);
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
                     BarrierCallback barrier_callback) {
    try {
        impl = new SchedulerImpl(std::move(sock), read_callback, barrier_callback);
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

void Scheduler::Remove(Channel *c) noexcept { impl->Remove(c); }

void Scheduler::Run() noexcept { impl->Run(); }

Scheduler &Scheduler::operator=(Scheduler &&other) {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

Scheduler::Scheduler(Scheduler &&other) { *this = std::move(other); }
