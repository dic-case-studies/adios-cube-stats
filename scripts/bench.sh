rm time.txt
touch time.txt

# imstat.c
# start time
startTime=`gdate +%s%3N`;
./build/imstat.out fits-images/image.cut1000x1000.fits > imstat-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo "Time elapsed imstat.c : $diffMilliSeconds ms" >> time.txt

# imstat_adios.cpp
# start time
startTime=`gdate +%s%3N`;
./build/imstat_adios.out > imstat_adios-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat_adios-result.txt 
echo "Time elapsed imstat_adios.cpp : $diffMilliSeconds ms" >> time.txt

# imstat_adios_single_read.cpp
# start time
startTime=`gdate +%s%3N`;
./build/imstat_adios_single_read.out > imstat_adios_single_read-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat_adios_single_read-result.txt 
echo "Time elapsed imstat_adios_single_read.cpp : $diffMilliSeconds ms" >> time.txt

# imstat_adios_mpi.cpp
# start time
startTime=`gdate +%s%3N`;
mpiexec -n 5 build/imstat_adios_mpi.out > imstat_adios_mpi-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat_adios_mpi-result.txt 
echo "Time elapsed imstat_adios_mpi.cpp with n=5 : $diffMilliSeconds ms" >> time.txt

# imstat_adios_mpi_less_reads.cpp
# start time
startTime=`gdate +%s%3N`;
mpiexec -n 5 build/imstat_adios_mpi_less_reads.out > imstat_adios_mpi_less_reads-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat_adios_mpi_less_reads-result.txt 
echo "Time elapsed imstat_adios_mpi_less_reads.cpp with n=5 : $diffMilliSeconds ms" >> time.txt