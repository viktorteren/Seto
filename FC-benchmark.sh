#!/usr/bin/env bash
bench_dir=./auto_benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ] || [ ${file: -3} == ".sg" ]
    then
        ./cmake-build-debug/TS_splitter "$file" FC
    fi
done

#find ./benchmark_dir/ -iname '*.g' -exec ./cmake-build-debug/TS_splitter "{} M" \;
