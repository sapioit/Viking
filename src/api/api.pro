#-------------------------------------------------
#
# Project created by QtCreator 2015-08-09T11:26:46
#
#-------------------------------------------------

QT       -= core gui

TARGET = api
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++14
QMAKE_CXXFLAGS += -Wall -Werror
QMAKE_CXXFLAGS += -Ijson/ -I$$PWD
DEFINES += API_LIBRARY


QMAKE_CXXFLAGS_DEBUG += -O0 -g
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

#HTTP
SOURCES += \
    http/cachemanager.cpp \
    http/request.cpp \
    http/response.cpp \
    http/routeutility.cpp \
    http/engine.cpp \
    http/parser.cpp \
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
    io/schedulers/file_container.cpp \
    io/schedulers/io_scheduler.cpp \
    io/socket/socket.cpp \
    io/schedulers/sys_epoll.cpp

HEADERS += \
    io/filesystem.h \
    io/socket/socket.h \
    io/schedulers/sys_epoll.h \
    io/schedulers/file_container.h \
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
    misc/log.cpp \
    misc/resource.cpp \
    misc/settings.cpp \
    misc/storage.cpp \

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
