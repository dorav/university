#!/bin/bash

DIFFTOOL=diff
[[ $1 == '-g' ]] && DIFFTOOL=vimdiff

for i in `find . -iname input.as`; do
	ACTUAL_OB=ps.ob
	EXPECTED_OB=$(dirname $i)/expected_ps.ob
	
	ACTUAL_ENT=`basename $i`.ent
	EXPECTED_ENT=$(dirname $i)/expected_ps.ent
	
	ACTUAL_LOG=log
	EXPECTED_LOG=$(dirname $i)/expected_log
	
	./Debug/assembler $i
	
	if [ -e $EXPECTED_LOG ]; then
		diff -b $ACTUAL_LOG $EXPECTED_LOG > /dev/null
		[ $? -ne 0 ] && (echo File $i: ; $DIFFTOOL $ACTUAL_LOG $EXPECTED_LOG)
		rm $ACTUAL_LOG
		
	elif [ -e $EXPECTED_OB ]; then
		if [ ! -e $ACTUAL_OB ]; then
			echo File "$i " had problems, log file:
			cat $ACTUAL_LOG
		else
			if [ -e $EXPECTED_ENT ]; then
				if [ ! -e $ACTUAL_ENT ]; then
					echo File "$i " had problems, no ent file found, log file:
					cat $ACTUAL_LOG
				fi
			fi
			if [ -e $ACTUAL_ENT ]; then
				diff -b $ACTUAL_ENT $EXPECTED_ENT > /dev/null
				[ $? -ne 0 ] && (echo File $i: ; $DIFFTOOL $ACTUAL_ENT $EXPECTED_ENT)
				rm $ACTUAL_ENT
			fi
			diff -b $ACTUAL_OB $EXPECTED_OB > /dev/null
			[ $? -ne 0 ] && (echo File $i: ; $DIFFTOOL $ACTUAL_OB $EXPECTED_OB)
			rm $ACTUAL_OB
		fi
	else
		echo "No expectations given for file $i"
	fi
done
