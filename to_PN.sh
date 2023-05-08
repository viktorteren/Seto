#!/usr/bin/env bash
bench_dir=../benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ] || [ ${file: -3} == ".sg" ]
    then
	echo $file
	INPUT=${file}
	SUBSTRING=$(echo $INPUT| cut -d'.')
        #./petrify "$file" -o 
    fi
done
