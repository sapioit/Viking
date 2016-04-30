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
#include <algorithm>
#include <execinfo.h>
#include <io/buffers/utils.h>
#include <io/schedulers/io_scheduler.h>
#include <io/schedulers/sys_epoll.h>
#include <misc/common.h>
#include <misc/debug.h>
#include <stdexcept>
#include <typeindex>
#include <utility>
using namespace io;

class scheduler::scheduler_impl {
    struct SocketNotFound {
        const epoll::event *event;
    };
    struct write_error {};

    std::vector<std::unique_ptr<channel>> channels;
    epoll poll;
    scheduler::callback_set callbacks;

    public:
    scheduler_impl() = default;
    scheduler_impl(std::unique_ptr<tcp_socket> sock, callback_set callbacks) : callbacks(callbacks) {
        try {
            add(std::move(sock), epoll::read | epoll::termination);
        } catch (const epoll::poll_error &) {
            throw;
        }
    }

    void add(std::unique_ptr<tcp_socket> socket, std::uint32_t flags) {
        try {
            auto ctx = std::make_unique<io::channel>(std::move(socket), flags);
            poll.schedule(ctx.get());
            channels.emplace_back(std::move(ctx));
        } catch (const epoll::poll_error &) {
            throw;
        }
    }

    void run() noexcept {
        if (channels.size() == 0)
            return;
        auto events = poll.await(channels.size());
        for (auto &event : events) {
            if (!(event.context->flags & epoll::edge_triggered)) {
                event.context->flags |= epoll::edge_triggered;
                poll.update(event.context);
            }
            if (event.context->socket->is_acceptor()) {
                add_new_connections(event.context);
                continue;
            }
            if (event.can_terminate()) {
                remove(event.context);
                continue;
            }
            if (event.has_error()) {
                debug("epoll event has error");
                remove(event.context);
                continue;
            }
            if (event.can_read()) {
                process_read(event.context);
            }
            if (event.can_write()) {
                process_write(event.context);
                continue;
            }
        }
    }

    void add_new_connections(const channel *channel) noexcept {
        do {
            try {
                auto new_connection = channel->socket->accept();
                if (*new_connection) {
                    new_connection->make_non_blocking();
                    add(std::move(new_connection), epoll::read | epoll::termination);
                } else
                    break;
            } catch (epoll::poll_error &) {
            }
        } while (true);
    }

    void process_read(channel *channel) noexcept {
        try {
            if (auto callback_response = callbacks.on_read(channel)) {
                channel->flags |= epoll::write;
                poll.update(channel);
                auto &front = *callback_response.front();
                std::type_index type = typeid(front);
                if (type == typeid(memory_buffer) || type == typeid(unix_file))
                    enqueue_item(channel, callback_response, true);
                else
                    enqueue_item(channel, callback_response, false);
                process_write(channel);
            }
        } catch (...) {
            remove(channel);
        }
    }

    void process_write(channel *channel) noexcept {
        try {
            auto blocked = false;
            while (channel->queue && !blocked) {
                if (channel->queue.is_front_async()) {

                    /* We've encountered a barrier, that means we have to check if the
                     * future is ready. If it is, we put it in the front of the queue.
                     * If not, we make the channel level triggered and return
                     */

                    auto new_sync_buffer = callbacks.on_barrier(channel->queue);
                    if (*new_sync_buffer) {
                        channel->queue.replace_front(std::move(new_sync_buffer));
                    } else {
                        channel->flags &= ~epoll::edge_triggered;
                        poll.update(channel);
                        return;
                    }
                }

                blocked = fill_channel(channel);

                if (blocked) {
                    if (!channel->queue) {
                        if (channel->queue.keep_file_open()) {
                            channel->flags &= ~epoll::write;
                            channel->flags |= epoll::read;
                            callbacks.on_remove(channel);
                            poll.update(channel);
                        } else {
                            remove(channel);
                        }
                    }
                } else {
                    if (channel->queue) {
                        channel->flags |= epoll::write;
                        channel->flags |= epoll::read;
                        channel->flags &= ~epoll::edge_triggered;
                        poll.update(channel);
                    } else {
                        if (channel->queue.keep_file_open()) {

                            channel->flags &= ~epoll::write;
                            channel->flags |= epoll::read;
                            callbacks.on_remove(channel);
                            poll.update(channel);
                        } else {
                            remove(channel);
                        }
                    }
                }
            }
        } catch (...) {
            remove(channel);
        }
    }

    bool fill_channel(channel *channel) {
        if (!channel->queue.buffers_left())
            return false;
        auto &front = *(channel->queue.front());
        std::type_index sched_item_type = typeid(front);

        if (sched_item_type == typeid(memory_buffer)) {
            memory_buffer *mem_buffer = reinterpret_cast<memory_buffer *>(channel->queue.front());
            io::tcp_socket::error_code ec;
            const auto written = channel->socket->write(mem_buffer->data.data(), mem_buffer->data.size(), ec);
            if (ec == io::tcp_socket::error_code::blocked || ec == io::tcp_socket::error_code::none) {
                if (written == mem_buffer->data.size()) {
                    channel->queue.remove_front();
                    return fill_channel(channel);
                } else {
                    std::vector<char>(mem_buffer->data.begin() + written, mem_buffer->data.end())
                        .swap(mem_buffer->data);
                }
                return true;
            } else {
                throw write_error{};
            }
        } else if (sched_item_type == typeid(io::unix_file)) {
            io::unix_file *unix_file = reinterpret_cast<io::unix_file *>(channel->queue.front());
            io::unix_file::error_code ec;

            do {
                auto size_left = unix_file->size_left();
                auto written = unix_file->send_to_fd(channel->socket->get_fd(), ec);
                // TODO handle ec
                if (written == size_left) {
                    channel->queue.remove_front();
                    return fill_channel(channel);
                }
            } while (ec == io::unix_file::error_code::none);
            if (ec == io::unix_file::error_code::blocked)
                return true;
            if (ec == io::unix_file::error_code::diy) {
                /* This is how Linux tells you that you'd better do it yourself in userspace.
                 * We need to replace this item with a MemoryBuffer version of this
                 * data, at the right offset.
                 */
                debug("diy");
                auto buffer = from(*unix_file);
                channel->queue.replace_front(std::move(buffer));
                return fill_channel(channel);
            }
            if (ec == io::unix_file::error_code::broken_pipe) {
                debug("Error when writing a unix file: broken pipe");
                throw write_error{};
            }
            if (ec == io::unix_file::error_code::bad_file) {
                debug("Error when writing a unix file: bad file");
                throw write_error{};
            }
        }
        return false;
    }

    void enqueue_item(channel *c, schedule_item &item, bool back) noexcept {
        back ? c->queue.put_back(std::move(item)) : c->queue.put_after_first_intact(std::move(item));
    }

    void remove(channel *channel) noexcept {
        if (channel->queue) {
            debug("Attempted to kill a channel with buffers still left");
            int j, nptrs;
            void *buffer[500];
            char **strings;

            nptrs = backtrace(buffer, 500);
            printf("backtrace() returned %d addresses\n", nptrs);

            /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
               would produce similar output to the following: */

            strings = backtrace_symbols(buffer, nptrs);
            if (strings == NULL) {
                perror("backtrace_symbols");
                exit(EXIT_FAILURE);
            }

            for (j = 0; j < nptrs; j++)
                printf("%s\n", strings[j]);

            free(strings);
            exit(EXIT_FAILURE);
        }
        callbacks.on_remove(channel);
        poll.remove(channel);
        channels.erase(
            std::remove_if(channels.begin(), channels.end(), [&channel](auto &ctx) { return channel == ctx.get(); }),
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

scheduler::scheduler(std::unique_ptr<tcp_socket> sock, callback_set callbacks) {
    try {
        impl = new scheduler_impl(std::move(sock), callbacks);
    } catch (...) {
        throw;
    }
}

scheduler::~scheduler() { delete impl; }

void scheduler::add(std::unique_ptr<io::tcp_socket> socket, uint32_t flags) {
    try {
        impl->add(std::move(socket), flags);
    } catch (const epoll::poll_error &) {
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
