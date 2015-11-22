#include <io/schedulers/io_scheduler.h>
#include <io/buffers/utils.h>

IO::Scheduler::Scheduler(std::unique_ptr<Socket> sock, IO::Scheduler::Callback callback) : callback(callback) {
    try {
        Scheduler::Add(std::move(sock), static_cast<std::uint32_t>(SysEpoll::Description::Read) |
                                            static_cast<std::uint32_t>(SysEpoll::Description::Termination));
    } catch (const SysEpoll::Error &) {
        throw;
    }
}

void IO::Scheduler::Add(std::unique_ptr<IO::Socket> socket, uint32_t flags) {
    try {
        poller_.Schedule(socket.get(), flags);
        sockets_.emplace_back(std::move(socket));
    } catch (const SysEpoll::Error &) {
        throw;
    }
}

void IO::Scheduler::Remove(const IO::Socket &socket) {
    schedule_.erase(socket.GetFD());
    poller_.Remove(&socket);
    sockets_.erase(std::remove_if(sockets_.begin(), sockets_.end(), [&socket](auto &sockptr) {
                       return sockptr->GetFD() == socket.GetFD();
                   }), sockets_.end());
}

void IO::Scheduler::Run() {
    if (sockets_.size() == 0)
        return;
    try {
        const auto events = poller_.Wait(sockets_.size());
        for (const auto &associated_event : events) {
            const auto &associated_socket = *associated_event.socket; // GetSocket(associated_event);

            if (associated_socket.IsAcceptor()) {
                debug("The master socket is active");
                AddNewConnections(associated_socket);
                continue;
            }

            if (CanTerminate(associated_event)) {
                debug("Event with fd = " + std::to_string(associated_event.file_descriptor) + " must be closed");
                Remove(associated_socket);
                continue;
            }
            if (CanWrite(associated_event)) {
                try {
                    ProcessWrite(associated_socket, schedule_.at(associated_socket.GetFD()));
                } catch (const DataCorruption &e) {
                    Remove(*e.sock);
                    debug("Data corruption occured!");
                } catch (const std::out_of_range &) {
                    debug("Could not find scheduled item!");
                }
                continue;
            }

            if (CanRead(associated_event) && (!IsScheduled(associated_event.socket->GetFD()))) {
                debug("Socket with fd = " + std::to_string(associated_socket.GetFD()) +
                      " can be read from, and there is no write "
                      "scheduled for it");
                CallbackResponse callback_response = callback(associated_socket);
                if (callback_response) {
                    /* Schedule the item in the epoll instance with just the Write flag,
                     * since it already has the others
                     */
                    poller_.Modify(&associated_socket, static_cast<std::uint32_t>(SysEpoll::Description::Write));
                    AddSchedItem(associated_event, std::move(callback_response));
                }
                continue;
            }
        }
    } catch (...) {
        std::rethrow_exception(std::current_exception());
    }
}

void IO::Scheduler::AddSchedItem(const SysEpoll::Event &ev, ScheduleItem item, bool append) noexcept {
    auto item_it = schedule_.find(ev.socket->GetFD());
    if (item_it == schedule_.end()) {
        schedule_.emplace(std::make_pair(ev.socket->GetFD(), std::move(item)));
    } else {
        if (!append) {
            /* Unlikely */
        } else {
            item_it->second.AddData(std::move(item));
        }
    }
}

void IO::Scheduler::ScheduledItemFinished(const IO::Socket &socket, ScheduleItem &sched_item) {
    if (sched_item.CloseWhenDone()) {
        Remove(socket);
    } else {
        schedule_.erase(socket.GetFD());
    }
}

void IO::Scheduler::ProcessWrite(const IO::Socket &socket, ScheduleItem &sched_item) {
    auto stop = false;
    do {
        DataSource *sched_item_front = sched_item.Front();

        if (typeid(*sched_item_front) == typeid(MemoryBuffer)) {
            MemoryBuffer *mem_buffer = dynamic_cast<MemoryBuffer *>(sched_item_front);
            if (mem_buffer != nullptr) {
                auto &data = mem_buffer->data;
                try {
                    const auto written = socket.WriteSome(data);
                    if (written == 0) {
                        stop = true;
                    }
                    if (written == data.size()) {
                        sched_item.RemoveFront();
                    } else {
                        auto old_data_size = data.size();
                        DataType(data.begin() + written, data.end()).swap(data);
                        if (old_data_size - written != data.size())
                            throw DataCorruption{std::addressof(socket)};
                    }
                } catch (const Socket::WriteError &e) {
                    debug("Write error on socket with "
                          "sockfd = " +
                          std::to_string(e.fd) + " errno = " + std::to_string(errno));
                    Remove(socket);
                    break;
                } catch (const Socket::ConnectionClosedByPeer &) {
                    Remove(socket);
                    break;
                }
            }
        } else if (typeid(*sched_item_front) == typeid(UnixFile)) {
            UnixFile *unix_file = dynamic_cast<UnixFile *>(sched_item_front);
            if (unix_file != nullptr) {
                try {
                    stop = !(unix_file->SendTo(socket.GetFD()));
                    if (!(*unix_file))
                        sched_item.RemoveFront();
                } catch (const UnixFile::BrokenPipe &) {
                    /* Received EPIPE. Remove the scheduled
                     * item */
                    Scheduler::Remove(socket);
                    break;
                } catch (const UnixFile::BadFile &) {
                    /* The file has been somehow removed or it cannot be read from anymore.
                     * We need to remove the whole scheduled item, since everything has been corrupted.
                     * TODO in the future, maybe pass this decision to a callback
                     * function.
                     */
                    Scheduler::Remove(socket);
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
                        /* For some reason, we could not generate a MemoryBuffer out of
                         * this, so we'll remove the whole scheduled item.
                         */
                        Scheduler::Remove(socket);
                        break;
                    }
                }
            }
        }
        if (!sched_item) {
            ScheduledItemFinished(socket, sched_item);
            stop = true;
        }
    } while (!stop);
}

void IO::Scheduler::AddNewConnections(const IO::Socket &acceptor) noexcept {
    do {
        auto new_connection = acceptor.Accept();
        if (*new_connection) {
            new_connection->MakeNonBlocking();
            Scheduler::Add(std::move(new_connection),
                           static_cast<std::uint32_t>(SysEpoll::Description::Read) |
                               static_cast<std::uint32_t>(SysEpoll::Description::Termination));
        } else
            break;
    } while (true);
}

bool IO::Scheduler::CanWrite(const SysEpoll::Event &event) const noexcept {
    return (event.description & static_cast<std::uint32_t>(SysEpoll::Description::Write));
}

bool IO::Scheduler::CanRead(const SysEpoll::Event &event) const noexcept {
    return (event.description & static_cast<std::uint32_t>(SysEpoll::Description::Read));
}

bool IO::Scheduler::IsScheduled(int file_descriptor) const noexcept {
    auto item = schedule_.find(file_descriptor);
    return (item != schedule_.end());
}

bool IO::Scheduler::CanTerminate(const SysEpoll::Event &event) const noexcept {
    return (event.description & static_cast<std::uint32_t>(SysEpoll::Description::Termination) ||
            event.description & static_cast<std::uint32_t>(SysEpoll::Description::Error));
}
