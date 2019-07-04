#!/usr/bin/env bash
exe=/cmake-build-debug/TS_splitter
bench_dir=./benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ]
    then
        ./cmake-build-debug/TS_splitter "$file" M
    fi
done

#find ./benchmark_dir/ -iname '*.g' -exec ./cmake-build-debug/TS_splitter "{} M" \;