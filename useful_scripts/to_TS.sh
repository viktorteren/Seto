#!/usr/bin/env bash
bench_dir=../benchmark_dir/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ] || [ ${file: -3} == ".sg" ]
    then
        if [[ $file != *"art"* ]]; then
            echo "File corrente:"
	        echo $file
	        INPUT=${file}
	        OUTPUT="${file/.g/.sg}"
	        echo "Output:"
	        echo $OUTPUT
            ../write_sg $file -o $OUTPUT -huge
        fi
    fi
done
