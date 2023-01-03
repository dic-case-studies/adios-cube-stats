CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic -O3

CFLAGS=-std=c99 -Wall -Wextra -pedantic -O3 

CFITSIO=`pkg-config --libs --cflags cfitsio`

ADIOS=`adios2-config --cxx-flags --cxx-libs`

DEBUGFLAGS=-fsanitize=address -g

CXX=mpicxx

CC=gcc

SRC = imstat_adios_mpi.out \
      imstat_adios.out \
      imstat.out \
	  imstat_adios_single_read.out \
	  imstat_adios_mpi_less_reads.out \

all: $(SRC)

%.out: src/%.c dir
	$(CC) -o build/$@ $< $(CFLAGS) $(OPT) $(CFITSIO)

%.out: src/%.cpp dir
	$(CXX) -o build/$@ $< $(CXXFLAGS) $(OPT) $(ADIOS)

clean:
	rm -rf build/*.out

dir:
	mkdir -p build

.PHONY: all clean dir

