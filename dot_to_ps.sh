#!/usr/bin/env bash
bench_dir=./benchmark_dir/vme_write_unsafe_FCPNs/*
for file in $bench_dir
do
    dot -Tps $file -o "${file::-3}ps"
done



