CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic -O3 `pkg-config --libs --cflags cfitsio`

CFLAGS=-std=c99 -Wall -Wextra -pedantic -O3 `pkg-config --libs --cflags cfitsio`

DEBUGFLAGS=-fsanitize=address -g

CXX=g++

CC=gcc

%.out: %.c
	$(CC) -o $@ $< $(CFLAGS) $(OPT)

clean:
	rm -rf *.out

dir:
	mkdir -p build

.PHONY: all clean dir

