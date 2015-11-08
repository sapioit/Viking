#-------------------------------------------------
#
# Project created by QtCreator 2015-08-09T11:26:46
#
#-------------------------------------------------

QT       -= core gui

TARGET = api
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Ijson/ -I$$PWD
DEFINES += API_LIBRARY


QMAKE_CXXFLAGS_DEBUG += -O0 -g
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

#HTTP
SOURCES += \
    http/cachemanager.cpp \
    http/parser.cpp \
    http/request.cpp \
    http/response.cpp \
    http/responder.cpp \
    http/routeutility.cpp \

HEADERS += \
    http/cachemanager.h \
    http/components.h \
    http/header.h \
    http/parser.h \
    http/request.h \
    http/response.h \
    http/responder.h \
    http/routeutility.h
#HTTP-END

#IO
SOURCES += \
    io/filesystem.cpp \
    io/socket/socket.cpp \
    io/watchers/sys_epoll.cpp \
    io/schedulers/out.cpp \

HEADERS += \
    io/filesystem.h \
    io/socket/socket.h \
    io/watchers/file_watcher.h \
    io/watchers/socket_watcher.h \
    io/watchers/sys_epoll.h \
    io/schedulers/out.h \

#IO-END

#JSON
SOURCES += \
    json/jsoncpp.cpp

HEADERS += \
    json/json.h
#JSON-END

#MISC
SOURCES += \
    misc/dfa.cpp \
    misc/log.cpp \
    misc/resource.cpp \
    misc/settings.cpp \
    misc/storage.cpp \
    misc/debug.cpp \

HEADERS += \
    misc/date.h \
    misc/dfa.h \
    misc/log.h \
    misc/resource.h \
    misc/settings.h \
    misc/storage.h \
    misc/debug.h \
#MISC-END

#UTIL
SOURCES +=

HEADERS += \
    util/utility.h \
    util/set.h \
#UTIL-END

#SERVER
SOURCES += \
    server/dispatcher.cpp \
    server/server.cpp

HEADERS += \
    server/dispatcher.h \
    server/server.h \
#SERVER-END

SOURCES += \
    main.cpp

unix {
    target.path = /mnt/exthdd/debugg
    INSTALLS += target
}

DISTFILES += \
    LICENSE
