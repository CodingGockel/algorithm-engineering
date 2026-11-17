CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall
INCLUDES  = -Iinclude -Ilibs

LIB_SRC = $(wildcard include/*.cpp)
HEADERS = $(wildcard include/*.h) src/constants.h

.PHONY: all run test clean

all: build/main

build/main: src/main.cpp $(LIB_SRC) $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) src/main.cpp $(LIB_SRC) -o $@

build/test: tests/test_main.cpp $(LIB_SRC) $(HEADERS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) tests/test_main.cpp $(LIB_SRC) -o $@

run: build/main
	./build/main

test: build/test
	./build/test

clean:
	rm -rf build/main build/test build/output
