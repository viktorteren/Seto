#!/bin/bash
if [ $# -eq 0 ]
then
    echo "The script needs an input argument corresponding to the path of the TS"
else
	param=$1
	new_end="_PN.dot"
	new_output_end="_PN.ps"
	ts="${param/.*/.dot}"
	#echo "ts = $ts" 
	ts_output="${param/.*/.ps}"
	#echo "ts_output = $ts_output"
	new="${param/.*/$new_end}"
	#echo "new = $new"
	new_ps="${param/.*/$new_output_end}"
	#echo "new_ps = $new_ps"
	./cmake-build-debug/Progetto_SSE $1
	dot -T ps $ts -o $ts_output
	dot -T ps $new -o $new_ps
fi
