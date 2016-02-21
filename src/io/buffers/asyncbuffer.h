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
#ifndef ASYNCBUFFER
#define ASYNCBUFFER

#include <io/buffers/datasource.h>
#include <future>

template <typename T> struct AsyncBuffer : public data_source {
    std::future<T> future;

    public:
    AsyncBuffer(std::future<T> future) : future(std::move(future)) {}

    operator bool() const noexcept { return true; }
    bool Intact() const noexcept { return true; }
    bool IsReady() {
        auto result = future.wait_for(std::chrono::seconds(0));
        return (result == std::future_status::ready || result == std::future_status::deferred);
    }
};

#endif // ASYNCBUFFER
