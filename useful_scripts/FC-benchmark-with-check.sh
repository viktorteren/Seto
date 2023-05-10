#!/usr/bin/env bash
bench_dir=../auto_benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ] || [ ${file: -3} == ".sg" ]
    then
        #./cmake-build-debug/TS_splitter "$file" FC COMPOSE
        if [ ${file: -2} == ".g" ]
        then
        	tmp=${file/.g/_coposition.aux}
        	echo $tmp
        	echo ltscompare --equivalence=bisim ${file/.g/.aux} $tmp
        fi	
        if [ ${file: -3} == ".sg" ]
        then
        	tmp=${file/.sg/_coposition.aux}
        	echo $tmp
        	echo ltscompare --equivalence=bisim ${file/.sg/.aux} $tmp
        fi
    fi
done


