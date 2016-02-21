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

class scheduler::scheduler_impl {
    struct SocketNotFound {
        const SysEpoll::Event *event;
    };
    struct WriteError {};

    std::vector<std::unique_ptr<Channel>> channels;
    SysEpoll poll;
    scheduler::read_cb read_callback;
    scheduler::barrier_cb barrier_callback;
    scheduler::before_removing_cb before_removing;

    public:
    scheduler_impl() = default;
    scheduler_impl(std::unique_ptr<Socket> sock, scheduler::read_cb read_callback,
                  scheduler::barrier_cb barrier_callback, scheduler::before_removing_cb before_removing)
        : read_callback(read_callback), barrier_callback(barrier_callback), before_removing(before_removing) {
        try {
            add(std::move(sock),
                static_cast<std::uint32_t>(SysEpoll::Read) | static_cast<std::uint32_t>(SysEpoll::Termination));
        } catch (const SysEpoll::PollError &) {
            throw;
        }
    }

    void add(std::unique_ptr<Socket> socket, std::uint32_t flags) {
        try {
            auto ctx = std::make_unique<IO::Channel>(std::move(socket));
            poll.Schedule(ctx.get(), flags);
            channels.emplace_back(std::move(ctx));
        } catch (const SysEpoll::PollError &) {
            throw;
        }
    }

    void run() noexcept {
        if (channels.size() == 0)
            return;
        const auto events = poll.Wait(channels.size());
        for (const auto &event : events) {

            poll.modify(event.context, static_cast<std::uint32_t>(SysEpoll::EdgeTriggered));

            if (event.context->socket->IsAcceptor()) {
                add_new_connections(event.context);
                continue;
            }

            if (event.CanTerminate()) {
                remove(event.context);
                continue;
            }

            if (event.CanRead()) {
                try {
                    if (auto callback_response = read_callback(event.context)) {
                        poll.modify(event.context, static_cast<std::uint32_t>(SysEpoll::Write));
                        auto &front = *callback_response.front();
                        std::type_index type = typeid(front);
                        if (type == typeid(MemoryBuffer) || type == typeid(UnixFile))
                            enqueue_item(event.context, callback_response, true);
                        else
                            enqueue_item(event.context, callback_response, false);
                    }
                } catch (const IO::Socket::ConnectionClosedByPeer &) {
                    remove(event.context);
                }
            }

            if (event.CanWrite()) {
                process_write(event.context);
                continue;
            }
        }
    }

    void add_new_connections(const Channel *channel) noexcept {
        do {
            try {
                auto new_connection = channel->socket->Accept();
                if (*new_connection) {
                    new_connection->MakeNonBlocking();
                    add(std::move(new_connection),
                        static_cast<std::uint32_t>(SysEpoll::Read) | static_cast<std::uint32_t>(SysEpoll::Termination));
                } else
                    break;
            } catch (SysEpoll::PollError &) {
            }
        } while (true);
    }

    void process_write(Channel *channel) noexcept {
        try {
            bool filled = false;
            while (channel->queue && !filled) {
                if (channel->queue.is_front_async()) {
                    auto new_sync_buffer = barrier_callback(channel->queue);
                    if (*new_sync_buffer) {
                        channel->queue.replace_front(std::move(new_sync_buffer));
                    } else {
                        poll.modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                        return;
                    }
                }
                auto result = fill_channel(channel);
                if (!(channel->queue)) {
                    if (channel->queue.keep_file_open()) {
                        poll.modify(channel, ~static_cast<std::uint32_t>(SysEpoll::Write));
                        poll.modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
                    } else {
                        /* There is a chance that the channel has pending data right now, so we must not close it yet.
                         * Instead we mark it, and the next time we poll, if it's not in the event list, we remove it */
                        remove(channel);
                        return;
                    }
                }
                filled = result;
            }
            if (!filled)
                poll.modify(channel, static_cast<std::uint32_t>(SysEpoll::LevelTriggered));
        } catch (...) {
            remove(channel);
        }
    }

    bool fill_channel(Channel *channel) {
        auto &front = *channel->queue.front();
        std::type_index sched_item_type = typeid(front);

        if (sched_item_type == typeid(MemoryBuffer)) {
            MemoryBuffer *mem_buffer = reinterpret_cast<MemoryBuffer *>(channel->queue.front());
            try {
                if (const auto written = channel->socket->WriteSome(mem_buffer->data)) {
                    if (written == mem_buffer->data.size())
                        channel->queue.remove_front();
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
            UnixFile *unix_file = reinterpret_cast<UnixFile *>(channel->queue.front());
            try {
                auto size_left = unix_file->SizeLeft();
                if (const auto written = unix_file->SendTo(channel->socket->GetFD())) {
                    if (written == size_left)
                        channel->queue.remove_front();
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
                    channel->queue.replace_front(std::move(buffer));
                    return fill_channel(channel);
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

    void enqueue_item(Channel *c, schedule_item &item, bool back) noexcept {
        back ? c->queue.put_back(std::move(item)) : c->queue.put_after_first_intact(std::move(item));
    }

    void remove(Channel *c) noexcept {
        before_removing(c);
        poll.remove(c);
        channels.erase(std::remove_if(channels.begin(), channels.end(), [&c](auto &ctx) { return c == &*ctx; }),
                       channels.end());
    }
};

scheduler::scheduler() {
    try {
        impl = new scheduler_impl();
    } catch (...) {
        throw;
    }
}

scheduler::scheduler(std::unique_ptr<Socket> sock, scheduler::read_cb read_callback,
                     barrier_cb barrier_callback, before_removing_cb before_removing) {
    try {
        impl = new scheduler_impl(std::move(sock), read_callback, barrier_callback, before_removing);
    } catch (...) {
        throw;
    }
}

scheduler::~scheduler() { delete impl; }

void scheduler::add(std::unique_ptr<IO::Socket> socket, uint32_t flags) {
    try {
        impl->add(std::move(socket), flags);
    } catch (const SysEpoll::PollError &) {
        throw;
    }
}

void scheduler::run() noexcept { impl->run(); }

scheduler &scheduler::operator=(scheduler &&other) {
    if (this != &other) {
        impl = other.impl;
        other.impl = nullptr;
    }
    return *this;
}

scheduler::scheduler(scheduler &&other) { *this = std::move(other); }
