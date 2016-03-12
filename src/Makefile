# Compiler
CXX = g++
CC = gcc
CXX_OPTS = -fPIC -std=c++1z -O3 -Wall -Werror
C_OPTS = -fPIC -O3 -Wall -Werror
DEBUG_OPTS =
INCLUDEDIRS = -I.
.DEFAULT_GOAL := lib

# Project name
PROJECT = viking

# Libraries
LDFLAGS = -shared
LIBNAME = libviking.so
LIBNAME_ABBREVIATED = -lviking
BINFOLDER = ../lib

SUBDIR_ROOTS := .
DIRS := . $(shell find $(SUBDIR_ROOTS) -type d)
GARBAGE_PATTERNS := *.o *.o.d *~ core .depend .*.cmd *.ko *.mod.c
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

TOBJS	= test.o

CPP_FILES = $(shell find ./ -type f -name '*.cpp' ! -name "*test*")
OBJS = $(patsubst ./%.cpp, ./%.o, $(CPP_FILES))

C_FILES = $(shell find ./ -type f -name '*.c' ! -name "*test*")
OBJS += $(patsubst ./%.c, ./%.o, $(C_FILES))

-include $(OBJS:.o=.o.d)

lib: $(OBJS)
	$(CXX) $(CXX_OPTS) $(DEBUG_OPTS) $(LDFLAGS) $^ -o $(LIBNAME)
	mkdir -p $(BINFOLDER)
	mv $(LIBNAME) $(BINFOLDER)

clean:
	rm -rf $(GARBAGE)
	rm *.o *.d -Rf
	rm -rf $(BINFOLDER)

install:
	cp $(BINFOLDER)/$(LIBNAME) /usr/lib
	mkdir -p /usr/include/viking
	mkdir -p /usr/include/viking/server
	mkdir -p /usr/include/viking/misc
	mkdir -p /usr/include/viking/http
	mkdir -p /usr/include/viking/http/dispatcher
	mkdir -p /usr/include/viking/io
	mkdir -p /usr/include/viking/io/schedulers
	mkdir -p /usr/include/viking/io/buffers
	mkdir -p /usr/include/viking/io/socket
	mkdir -p /usr/include/viking/http
	mkdir -p /usr/include/viking/json
	mkdir -p /usr/include/viking/inl
	mkdir -p /usr/include/viking/misc
	cp json/json.h /usr/include/viking/json/json.h
	cp misc/settings.h /usr/include/viking/misc/settings.h
	cp misc/resource.h /usr/include/viking/misc/resource.h
	cp server/server.h /usr/include/viking/server/server.h
	cp http/dispatcher/dispatcher.h /usr/include/viking/http/dispatcher/dispatcher.h
	cp io/schedulers/sched_item.h /usr/include/viking/io/schedulers/sched_item.h
	cp io/buffers/unix_file.h /usr/include/viking/io/buffers/unix_file.h
	cp io/buffers/mem_buffer.h /usr/include/viking/io/buffers/mem_buffer.h
	cp io/buffers/asyncbuffer.h /usr/include/viking/io/buffers/asyncbuffer.h
	cp io/buffers/datasource.h /usr/include/viking/io/buffers/datasource.h
	cp io/socket/socket.h /usr/include/viking/io/socket/socket.h
	cp io/filesystem.h /usr/include/viking/io/filesystem.h
	cp http/engine.h /usr/include/viking/http/engine.h
	cp http/request.h /usr/include/viking/http/request.h
	cp http/header.h /usr/include/viking/http/header.h
	cp http/version.h /usr/include/viking/http/version.h
	cp http/parser.h /usr/include/viking/http/parser.h
	cp http/response.h /usr/include/viking/http/response.h
	cp http/resolution.h /usr/include/viking/http/resolution.h
	cp http/routeutility.h /usr/include/viking/http/routeutility.h
	cp inl/methods.h /usr/include/viking/inl/methods.h
	cp inl/status_codes.h /usr/include/viking/inl/status_codes.h
	cp misc/common.h /usr/include/viking/misc/common.h

uninstall:
	rm /usr/lib/$(LIBNAME)
	rm -rf /usr/include/$(PROJECT)

testapp:
	cd ../test && $(MAKE)

%.o : %.cpp
	$(CXX) -c $(CXX_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(CXX_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< > $@.d

%.o : %.c
	$(CC) -c $(C_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< -o $@
	$(CC) -MM $(C_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< > $@.d
