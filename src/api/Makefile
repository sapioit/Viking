# Compiler
CXX = clang++
CC = clang
CXX_OPTS = -fPIC -std=c++14 -O3 -Wall -Werror
C_OPTS = -fPIC -O3 -Wall -Werror
DEBUG_OPTS = 
INCLUDEDIRS = -I.

# Project name
PROJECT = cppweb_test

# Libraries
LIBS = -lpthread
LDFLAGS = -shared
TESTAPP_LDFLAGS = -L../bin
LIBNAME = libcppweb.so
LIBNAME_ABBREVIATED = -lcppweb

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
	mkdir -p ../bin
	mv $(LIBNAME) ../bin

testapp: test.o
	$(CXX) $(CXX_OPTS) $(DEBUG_OPTS) $(TESTAPP_LDFLAGS) $(LIBS) $(LIBNAME_ABBREVIATED) $^ -o $(PROJECT)

clean: 
	rm -rf $(GARBAGE)
	rm *.o *.d $(PROJECT) $(LIBNAME) -Rf
# TODO install the required headers also

install: 
	cp ../bin/$(LIBNAME) /usr/lib

%.o : %.cpp
	$(CXX) -c $(CXX_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(CXX_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< > $@.d

%.o : %.c
	$(CC) -c $(C_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< -o $@
	$(CC) -MM $(C_OPTS) $(DEBUG_OPTS) $(INCLUDEDIRS) $< > $@.d
