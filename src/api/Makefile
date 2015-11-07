# Compiler
CXX = clang++
OPTS = -fPIC -std=c++14 -O3 -Wall
DEBUGOPTS = 
INCLUDEDIRS = -I.

# Project name
PROJECT = srv

# Libraries
LIBS = -lpthread
LDFLAGS = -shared
LIBNAME = libcppweb.so
LIBNAME_ABBREVIATED = -lcppweb

SUBDIR_ROOTS := .
DIRS := . $(shell find $(SUBDIR_ROOTS) -type d)
GARBAGE_PATTERNS := *.o *.o.d *~ core .depend .*.cmd *.ko *.mod.c
GARBAGE := $(foreach DIR,$(DIRS),$(addprefix $(DIR)/,$(GARBAGE_PATTERNS)))

TOBJS	= main.o

CPP_FILES = $(shell find ./ -type f -name '*.cpp')
OBJS = $(patsubst ./%.cpp, ./%.o, $(CPP_FILES))

-include $(OBJS:.o=.o.d)

# Targets
$(PROJECT): $(OBJS) $(TOBJS)
	$(CXX) $(OPTS) $(DEBUGOPTS) $(LIBS) $^ -o $@

%.o : %.cpp
	$(CXX) -c $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< -o $@
	$(CXX) -MM $(OPTS) $(DEBUGOPTS) $(INCLUDEDIRS) $< > $@.d

clean: 
	rm -rf $(GARBAGE)
	rm *.o *.d $(PROJECT) $(LIBNAME) -Rf

lib: $(OBJS)
	$(CXX) $(OPTS) $(DEBUGOPTS) $(LIBS) $(LDFLAGS) $^ -o $(LIBNAME)

testapp: main.o
	$(CXX) $(OPTS) $(DEBUGOPTS) $(LIBS) -L./ $(LIBNAME_ABBREVIATED) $^ -o $(PROJECT)

install: 
	cp $(LIBNAME) /usr/lib
