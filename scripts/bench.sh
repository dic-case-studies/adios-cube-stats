#! /usr/bin/env bash

rm stats/time.txt &> /dev/null
touch stats/time.txt
mkdir -p stats

set -e

# imstat.c
startTime=`gdate +%s%3N`;
./build/imstat.out fits-images/image.cut1000x1000.fits > stats/imstat_result.txt
endTime=`gdate +%s%3N`;
diffMilliSeconds="$(($endTime-$startTime))"
echo "Time elapsed imstat.c : $diffMilliSeconds ms" >> stats/time.txt

declare -a FILES=(imstat_adios imstat_adios_single_read imstat_adios_ll imstat_adios_single_read_ll)
for file in ${FILES[@]}
do
    startTime=`gdate +%s%3N`;
    ./build/$file.out > stats/${file}_result.txt
    endTime=`gdate +%s%3N`;

    diffMilliSeconds="$(($endTime-$startTime))"
    echo "Time elapsed $file.cpp : $diffMilliSeconds ms" >> stats/time.txt
done

declare -a MPI_FILES=(imstat_adios_mpi imstat_adios_mpi_less_reads imstat_adios_mpi_ll imstat_adios_mpi_less_reads_ll)
for file in ${MPI_FILES[@]}
do
    startTime=`gdate +%s%3N`;
    mpiexec -n 5 ./build/$file.out > stats/${file}_result.txt
    endTime=`gdate +%s%3N`;

    diffMilliSeconds="$(($endTime-$startTime))"
    echo "Time elapsed $file.cpp : $diffMilliSeconds ms" >> stats/time.txt
done
