#!/bin/bash

DIFFTOOL=diff
[[ $1 == '-g' ]] && DIFFTOOL=vimdiff

for i in `find . -iname input.as`; do
	ACTUAL=ps.ob
	EXPECTED=$(dirname $i)/expected_ps.ob
	ACTUAL_LOG=log
	EXPECTED_LOG=$(dirname $i)/expected_log
	./Debug/assembler $i
	if [ -e $EXPECTED_LOG ]; then
		diff -b $ACTUAL_LOG $EXPECTED_LOG > /dev/null
		[ $? -ne 0 ] && $DIFFTOOL $ACTUAL_LOG $EXPECTED_LOG
		rm $ACTUAL_LOG
	elif [ -e $EXPECTED ]; then
		if [ ! -e $ACTUAL ]; then
			echo File "$i " had problems, log file:
			cat $ACTUAL_LOG
		else
			diff -b $ACTUAL $EXPECTED > /dev/null
			[ $? -ne 0 ] && $DIFFTOOL $ACTUAL $EXPECTED
			rm $ACTUAL
		fi
	else
		echo "No expectations given!"
	fi
done
