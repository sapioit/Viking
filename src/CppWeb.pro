#-------------------------------------------------
#
# Project created by QtCreator 2015-08-09T11:26:46
#
#-------------------------------------------------

QT       -= core gui

TARGET = cppweb_test
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -Wall -Werror
QMAKE_CXXFLAGS += -Ijson/ -I$$PWD
DEFINES += API_LIBRARY


QMAKE_CXXFLAGS_DEBUG += -O0 -g
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -flto

#CACHE
SOURCES += \
    cache/file_descriptor.cpp \
    http/util.cpp \
    misc/string_util.cpp \
    http/response_serializer.cpp \
    io/schedulers/channel.cpp \
    http/resolution.cpp

HEADERS += \
    cache/file_descriptor.h \
#CACHE-END
    http/util.h \
    misc/string_util.h \
    http/version.h \
    http/cache_policy.h \
    io/schedulers/channel.h \
    io/buffers/asyncbuffer.h \
    misc/common.h \
    http/resolution.h

#HTTP
SOURCES += \
    http/cachemanager.cpp \
    http/request.cpp \
    http/response.cpp \
    http/routeutility.cpp \
    http/engine.cpp \
    http/parser.c \
    io/buffers/unix_file.cpp \
    io/schedulers/sched_item.cpp \
    io/buffers/utils.cpp

HEADERS += \
    http/cachemanager.h \
    http/components.h \
    http/header.h \
    http/parser.h \
    http/engine.h \
    http/request.h \
    http/response.h \
    http/routeutility.h \
#HTTP-END
    http/response_serializer.h \
    io/buffers/datasource.h \
    io/buffers/unix_file.h \
    io/schedulers/sched_item.h \
    io/buffers/utils.h

#IO
SOURCES += \
    io/filesystem.cpp \
    io/schedulers/io_scheduler.cpp \
    io/socket/socket.cpp \
    io/schedulers/sys_epoll.cpp

HEADERS += \
    io/filesystem.h \
    io/socket/socket.h \
    io/schedulers/sys_epoll.h \
    io/schedulers/io_scheduler.h \
    io/buffers/mem_buffer.h

#IO-END

#JSON
SOURCES += \
    json/jsoncpp.cpp

HEADERS += \
    json/json.h
#JSON-END

#MISC
SOURCES += \
    misc/resource.cpp \
    misc/settings.cpp \
    misc/storage.cpp \

HEADERS += \
    misc/date.h \
    misc/dfa.h \
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
    http/dispatcher/dispatcher.cpp \
    server/server.cpp

HEADERS += \
    http/dispatcher/dispatcher.h \
    server/server.h \
#SERVER-END

SOURCES += \
    ../test/test.cpp

unix {
    target.path = /mnt/exthdd/debugg
    INSTALLS += target
}

DISTFILES += \
    LICENSE
