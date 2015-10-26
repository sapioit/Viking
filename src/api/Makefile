# Compiler
CXX = clang++
OPTS = -fPIC -std=c++14 -O3 -Wall
DEBUGOPTS = -g
INCLUDEDIRS = -I.

# Project name
PROJECT = srv

# Libraries
LIBS = -lpthread
LDFLAGS = -shared
LIBNAME = libcppweb.so
LIBNAME_ABBREVIATED = -lcppweb

# Files and folders
#include main.o back
OBJS    = cachemanager.o parser.o request.o response.o responsemanager.o routeutility.o filesystem.o \
	outputscheduler.o socket.o watcher.o jsoncpp.o dfa.o log.o resource.o settings.o storage.o dispatcher.o \
	server.o file_watcher.o socket_watcher.o sys_epoll.o
TOBJS	= main.o

-include $(OBJS:.o=.o.d)

# Targets
$(PROJECT): $(OBJS) $(TOBJS)
	$(CXX) $(OPTS) $(DEBUGOPTS) $(LIBS) $^ -o $@

%.o : %.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d
	
%.o : http/%.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d

%.o : io/%.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d
	
%.o : json/%.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d
	
%.o : misc/%.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d
	
%.o : server/%.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d

clean:
	rm *.o *d $(PROJECT) $(LIBNAME) -Rf

lib: $(OBJS)
	$(CXX) $(OPTS) $(DEBUGOPTS) $(LIBS) $(LDFLAGS) $^ -o $(LIBNAME)
#$^ -o $(LIBNAME)


testapp: main.o
	$(CXX) $(OPTS) $(DEBUGOPTS) $(LIBS) -L./ $(LIBNAME_ABBREVIATED) $^ -o $(PROJECT)

install: 
	cp $(LIBNAME) /usr/lib
