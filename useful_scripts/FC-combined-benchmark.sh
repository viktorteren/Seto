#!/usr/bin/env bash
bench_dir=../auto_benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ] || [ ${file: -3} == ".sg" ]
    then
        ../cmake-build-debug/TS_splitter "$file" B GE
    fi
done

