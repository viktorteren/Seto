#!/usr/bin/env bash
bench_dir=large_stgs/*
for file in $bench_dir
do
    if [ ${file: -2} == ".g" ]
    then
        echo "File corrente:"
	    echo $file
	    INPUT=${file}
	    OUTPUT="${file/.g/.sg}"
	    echo "Output:"
	    echo $OUTPUT
        ./write_sg $file -o $OUTPUT -huge
    fi
done

#find ./benchmark_dir/ -iname '*.g' -exec ./cmake-build-debug/TS_splitter "{} M" \;
