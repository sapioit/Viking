# Compiler
CXX = clang++
CC = clang
CXX_OPTS = -fPIC -std=c++1z -O3 -Wall -Werror
C_OPTS = -fPIC -O3 -Wall -Werror
DEBUG_OPTS =
INCLUDEDIRS = -I.
.DEFAULT_GOAL := lib

# Project name
PROJECT = cppweb

# Libraries
LIBS = -lpthread -lstdc++fs
LDFLAGS = -shared
LIBNAME = libcppweb.so
LIBNAME_ABBREVIATED = -lcppweb
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
	$(CXX) $(CXX_OPTS) $(DEBUG_OPTS) $(LIBS) $(LDFLAGS) $^ -o $(LIBNAME)
	mkdir -p $(BINFOLDER)
	mv $(LIBNAME) $(BINFOLDER)

clean:
	rm -rf $(GARBAGE)
	rm *.o *.d -Rf
	rm -rf $(BINFOLDER)

install:
	cp $(BINFOLDER)/$(LIBNAME) /usr/lib
	mkdir -p /usr/include/cppweb
	mkdir -p /usr/include/cppweb/server
	mkdir -p /usr/include/cppweb/misc
	mkdir -p /usr/include/cppweb/http
	mkdir -p /usr/include/cppweb/http/dispatcher
	mkdir -p /usr/include/cppweb/io
	mkdir -p /usr/include/cppweb/io/schedulers
	mkdir -p /usr/include/cppweb/io/buffers
	mkdir -p /usr/include/cppweb/io/socket
	mkdir -p /usr/include/cppweb/http
	mkdir -p /usr/include/cppweb/json
	cp json/json.h /usr/include/cppweb/json/json.h
	cp misc/settings.h /usr/include/cppweb/misc/settings.h
	cp misc/resource.h /usr/include/cppweb/misc/resource.h
	cp server/server.h /usr/include/cppweb/server/server.h
	cp http/dispatcher/dispatcher.h /usr/include/cppweb/http/dispatcher/dispatcher.h
	cp io/schedulers/sched_item.h /usr/include/cppweb/io/schedulers/sched_item.h
	cp io/buffers/unix_file.h /usr/include/cppweb/io/buffers/unix_file.h
	cp io/buffers/mem_buffer.h /usr/include/cppweb/io/buffers/mem_buffer.h
	cp io/buffers/asyncbuffer.h /usr/include/cppweb/io/buffers/asyncbuffer.h
	cp io/buffers/datasource.h /usr/include/cppweb/io/buffers/datasource.h
	cp io/socket/socket.h /usr/include/cppweb/io/socket/socket.h
	cp http/engine.h /usr/include/cppweb/http/engine.h
	cp http/request.h /usr/include/cppweb/http/request.h
	cp http/header.h /usr/include/cppweb/http/header.h
	cp http/components.h /usr/include/cppweb/http/components.h
	cp http/version.h /usr/include/cppweb/http/version.h
	cp http/parser.h /usr/include/cppweb/http/parser.h
	cp http/response.h /usr/include/cppweb/http/response.h
	cp http/resolution.h /usr/include/cppweb/http/resolution.h

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
