CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic -O3

CFLAGS=-std=c99 -Wall -Wextra -pedantic -O3 `pkg-config --libs --cflags cfitsio` 

DEBUGFLAGS=-fsanitize=address -g

CXX=g++

CC=gcc

%.out: src/%.c dir
	$(CC) -o build/$@ $< $(CFLAGS) $(OPT)

%.out: src/%.cpp dir
	$(CXX) -o build/$@ $< $(CXXFLAGS) $(OPT)

clean:
	rm -rf build/*.out

dir:
	mkdir -p build

.PHONY: all clean dir

