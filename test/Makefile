CXX = g++
CC = gcc
CXX_OPTS = -fPIC -std=c++1z -O3 -Wall -Werror
LIBS = -lpthread -lcppweb -lstdc++fs
TESTAPP_INCLUDEDIRS = -I/usr/include/cppweb/
OBJS = test.cpp
PROJECT = cppweb_test

all: $(OBJS)
	$(CXX) $(CXX_OPTS) $(TESTAPP_INCLUDEDIRS) $^ -o $(PROJECT) $(LIBS)

clean:
	rm $(PROJECT)
