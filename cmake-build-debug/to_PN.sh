#!/usr/bin/env bash
bench_dir=../benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ]
    then
	echo $file
	INPUT=${file}
	SUBSTRING=$(echo $INPUT| cut -d'.')
        #./petrify "$file" -o 
    fi
done

#find ./benchmark_dir/ -iname '*.g' -exec ./cmake-build-debug/TS_splitter "{} M" \;
