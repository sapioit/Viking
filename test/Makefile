CXX = clang++
CC = clang
CXX_OPTS = -fPIC -std=c++14 -O3 -Wall -Werror
LIBS = -lpthread -lcppweb
TESTAPP_INCLUDEDIRS = -I/usr/include/cppweb/
OBJS = test.cpp
PROJECT = cppweb_test

all: $(OBJS)
	$(CXX) $(CXX_OPTS) $(TESTAPP_INCLUDEDIRS) $(LIBS) $^ -o $(PROJECT)

clean: 
	rm $(PROJECT)
