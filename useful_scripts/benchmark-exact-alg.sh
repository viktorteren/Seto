#!/usr/bin/env bash
bench_dir=../benchmark_dir/*
echo 'Start of benchmark-all' >> ./cmake-build-debug/stats.csv
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ] || [ ${file: -3} == ".sg" ]
    then
        ../cmake-build-debug/Seto "$file" SM -ALL
    fi
done
