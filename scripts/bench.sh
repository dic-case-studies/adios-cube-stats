# imstat.c
# start time
startTime=`gdate +%s%3N`;
./build/imstat.out fits-images/image.cut1000x1000.fits > imstat-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat-result.txt 
echo "Time elapsed : $diffMilliSeconds ms" >> imstat-result.txt

# imstat_adios.cpp
# start time
startTime=`gdate +%s%3N`;
./build/imstat_adios.out > imstat_adios-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat_adios-result.txt 
echo "Time elapsed : $diffMilliSeconds ms" >> imstat_adios-result.txt

# imstat_adios_mpi.cpp
# start time
startTime=`gdate +%s%3N`;
mpiexec -n 5 build/imstat_adios_mpi.out > imstat_adios_mpi-result.txt 
#end time
endTime=`gdate +%s%3N`;

diffMilliSeconds="$(($endTime-$startTime))"

echo >> imstat_adios_mpi-result.txt 
echo "Time elapsed : $diffMilliSeconds ms" >> imstat_adios_mpi-result.txt