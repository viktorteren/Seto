#!/usr/bin/env bash
bench_dir=./benchmark_all_flag/*
echo 'Start of benchmark-all' >> ./cmake-build-debug/stats.csv
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ]
    then
        ./cmake-build-debug/TS_splitter "$file" M -ALL
    fi
done
