#! /usr/bin/env bash

set -e

make $1

echo "--------$1---------" >> test_bench.txt
gdate -u +%Y-%m-%dT%H:%M:%SZ >> test_bench.txt
startTime=`gdate +%s%3N`;
mpiexec -n 3 ./build/$1 > /dev/null
endTime=`gdate +%s%3N`;
diffMilliSeconds="$(($endTime-$startTime))"
echo "Time elapsed $1 : $diffMilliSeconds ms" >> test_bench.txt
