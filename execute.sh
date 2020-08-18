#!/bin/bash
if [ $# -eq 0 ]
then
    echo "argomento 1: percorso input, argomento 2: (opzionale) S per statistiche intermedie"
else
param=$1
param2=$2
new_end="_PN.dot"
new_output_end="_PN.ps"
new_ects="_ECTS.dot"
new_ects_output_end="_ECTS.ps"
ts="${param/.ts/.dot}"
ts="${param/.g/.dot}"
ts_output="${ts/.dot/.ps}"
new="${ts/.dot/$new_end}"
ects="${ts/.dot/$new_ects}"
ects_ps="${ts/.dot/$new_ects_output_end}"
new_ps="${ts/.dot/$new_output_end}"
./cmake-build-debug/TS_splitter $1 $2
dot -T ps $ts -o $ts_output
dot -T ps $ects -o $ects_ps
dot -T ps $new -o $new_ps
fi
