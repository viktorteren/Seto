#!/bin/bash
if [ $# -eq 0 ]
then
    echo "The script needs an input argument corresponding to the path of the TS"
else
param=$1
new_end="_PN.dot"
new_output_end="_PN.ps"
new_ects="_ECTS.dot"
new_ects_output_end="_ECTS.ps"
ts="${param/.ts/.dot}"
ts_output="${ts/.dot/.ps}"
new="${ts/.dot/$new_end}"
ects="${ts/.dot/$new_ects}"
ects_ps="${ts/.dot/$new_ects_output_end}"
new_ps="${ts/.dot/$new_output_end}"
./cmake-build-debug/Progetto_SSE $1
dot -T ps $ts -o $ts_output
dot -T ps $ects -o $ects_ps
dot -T ps $new -o $new_ps
fi
