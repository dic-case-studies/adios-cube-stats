#! /usr/bin/env bash

if [ "$#" -ne 1 ]; then
    echo "usage: bench.sh host"
    exit 0
fi

host=$1

mkdir -p stats/${host}
rm stats/${host}/time.txt &> /dev/null
touch stats/${host}/time.txt

set -e

# imstat.c
startTime=`gdate +%s%3N`;
./build/imstat.out fits-images/image.cut1000x1000.fits > stats/${host}/imstat_result.txt
endTime=`gdate +%s%3N`;
diffMilliSeconds="$(($endTime-$startTime))"
echo "Time elapsed imstat.c : $diffMilliSeconds ms" >> stats/${host}/time.txt
echo "Done with imstat.c"

# imstat_mpi.cpp
startTime=`gdate +%s%3N`;
mpiexec -n 5 ./build/imstat_mpi.out fits-images/image.cut1000x1000.fits > stats/${host}/imstat_mpi_result.txt
endTime=`gdate +%s%3N`;
diffMilliSeconds="$(($endTime-$startTime))"
echo "Time elapsed imstat_mpi.cpp : $diffMilliSeconds ms" >> stats/${host}/time.txt
echo "Done with imstat_mpi.cpp"


declare -a FILES=(imstat_adios imstat_adios_single_read imstat_adios_ll imstat_adios_single_read_ll)
for file in ${FILES[@]}
do
    startTime=`gdate +%s%3N`;
    ./build/$file.out > stats/${host}/${file}_result.txt
    endTime=`gdate +%s%3N`;

    diffMilliSeconds="$(($endTime-$startTime))"
    echo "Time elapsed $file.cpp : $diffMilliSeconds ms" >> stats/${host}/time.txt
    echo "Done with ${file}"
done

declare -a MPI_FILES=(imstat_adios_mpi imstat_adios_mpi_grouped_reads imstat_adios_mpi_ll imstat_adios_mpi_grouped_reads_ll)
for file in ${MPI_FILES[@]}
do
    startTime=`gdate +%s%3N`;
    mpiexec -n 5 ./build/$file.out > stats/${host}/${file}_result.txt
    endTime=`gdate +%s%3N`;

    diffMilliSeconds="$(($endTime-$startTime))"
    echo "Time elapsed $file.cpp : $diffMilliSeconds ms" >> stats/${host}/time.txt
    echo "Done with ${file}"
done
