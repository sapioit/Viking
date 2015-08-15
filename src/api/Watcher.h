//
// Created by Vladimir on 8/2/2015.
//

#ifndef SOCKET_OBSERVER_H
#define SOCKET_OBSERVER_H

#include "Socket.h"
#include <sys/epoll.h>
#include <memory>
#include <vector>

namespace IO {
    class Watcher {
    public:
        Watcher(std::shared_ptr<Socket>
                socket);

        void Stop();

        void Close(std::shared_ptr<IO::Socket>);

        void Start(std::function<void(std::shared_ptr<Socket>)>);

        void Start(std::function<void(std::vector<std::shared_ptr<Socket>>)>);

        ~Watcher();


    private:
        static constexpr int _maxEvents = 30;
        std::vector<std::shared_ptr<Socket>> _to_observe;
        std::shared_ptr<Socket> _socket;
        int _efd;
        std::vector<epoll_event> _events;
        bool _stopRequested = false;
        std::vector<std::shared_ptr<Socket>> Watch();
        void AddSocket(std::shared_ptr<Socket> socket);

        bool stopRequested() const;
        void setStopRequested(bool stopRequested);
    };
};

#endif //SOCKET_OBSERVER_H
