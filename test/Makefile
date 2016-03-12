CXX = g++
CC = gcc
CXX_OPTS = -fPIC -std=c++1z -O3 -Wall -Werror
LIBS = -lpthread -lviking -lstdc++fs -lz
TESTAPP_INCLUDEDIRS = -I/usr/include/viking/
OBJS = test.cpp
PROJECT = viking_test

all: $(OBJS)
	$(CXX) $(CXX_OPTS) $(TESTAPP_INCLUDEDIRS) $^ -o $(PROJECT) $(LIBS)

clean:
	rm $(PROJECT)
