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
#include <cache/resource_cache.h>
#include <cache/file_descriptor.h>
#include <http/dispatcher/dispatcher.h>
#include <http/engine.h>
#include <http/util.h>
#include <http/resolution.h>
#include <http/response_serializer.h>
#include <http/routeutility.h>
#include <http/engine.h>
#include <io/filesystem.h>
#include <io/socket/socket.h>
#include <io/buffers/asyncbuffer.h>
#include <inl/mime_types.h>
#include <misc/storage.h>
#include <misc/common.h>
#include <misc/debug.h>
#include <algorithm>
#include <type_traits>

//using namespace Web;
//using namespace Cache;

//class Dispatcher::DispatcherImpl {

//};

//void Dispatcher::AddRoute(RouteUtility::Route route) noexcept { impl->AddRoute(route); }

//ScheduleItem Dispatcher::HandleConnection(const IO::Channel *connection) {
//    try {
//        return impl->HandleConnection(connection);
//    } catch (...) {
//        throw;
//    }
//}

//std::unique_ptr<MemoryBuffer> Dispatcher::HandleBarrier(AsyncBuffer<Http::Response> *item) noexcept {
//    return impl->HandleBarrier(item);
//}

//void Dispatcher::WillRemove(const IO::Channel *s) noexcept { impl->RemovePendingContexts(s); }

//Dispatcher::Dispatcher() : impl(nullptr) { impl = new DispatcherImpl(); }

//Dispatcher::~Dispatcher() { delete impl; }

//Dispatcher::Dispatcher(Dispatcher &&other) noexcept { *this = std::move(other); }
//Dispatcher &Dispatcher::operator=(Dispatcher &&other) noexcept {
//    if (this != &other) {
//        impl = other.impl;
//        other.impl = nullptr;
//    }
//    return *this;
//}
