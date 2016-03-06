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
