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
#ifndef METHODS_H
#define METHODS_H

#include <misc/common.h>
namespace http {
enum class method {
    Delete,
    Get,
    Head,
    Post,
    Put,
    Connect,
    Options,
    Trace,
    Copy,
    Lock,
    Mkcol,
    Move,
    Propfind,
    Proppatch,
    Unlock,
    Report,
    Mkactivity,
    Checkout,
    Merge,
    M_Search,
    Notify,
    Subscribe,
    Unsubscribe
};

const std::unordered_map<std::string, method> method_map{{"GET", method::Get},
                                                         {"POST", method::Post},
                                                         {"PUT", method::Put},
                                                         {"DELETE", method::Delete},
                                                         {"Connect", method::Connect},
                                                         {"HEAD", method::Head},
                                                         {"OPTIONS", method::Options},
                                                         {"TRACE", method::Trace},
                                                         {"COPY", method::Copy},
                                                         {"LOCK", method::Lock},
                                                         {"MKCOL", method::Mkcol},
                                                         {"MOVE", method::Move},
                                                         {"PROPFIND", method::Propfind},
                                                         {"PROPPATCH", method::Proppatch},
                                                         {"UNLOCK", method::Unlock},
                                                         {"REPORT", method::Report},
                                                         {"MKACTIVITY", method::Mkactivity},
                                                         {"CHECKOUT", method::Checkout},
                                                         {"MERGE", method::Merge},
                                                         {"M-SEARCH", method::M_Search},
                                                         {"NOTIFY", method::Notify},
                                                         {"SUBSCRIBE", method::Subscribe},
                                                         {"UNSUBSCRIBE", method::Unsubscribe}};
}

#endif // METHODS_H
