#!/bin/bash

DIFFTOOL=diff
TEST_NAMES=()
FILES=()
VERBOSE=false

for arg in "$@"
do
    if [[ $arg == '-g' ]]; then
		DIFFTOOL=vimdiff
	elif [[ $arg == '-v' ]]; then
		VERBOSE=true
	else
		TEST_NAMES+=($arg)
	fi
done

if [[ ${#TEST_NAMES[@]} -eq 0 ]]; then
	if [[ $VERBOSE == true ]]; then
		echo "No args given, running all tests"
	fi
	TEST_NAMES=("?")
fi

for name in ${TEST_NAMES[*]}; do
	if [[ $VERBOSE == true ]]; then
		echo "Will run tests for the files *$name*"
	fi
	
	# this could produce duplicates.
	FILES+=(`find examples/*${name}*/ -iname input.as`)
done

for i in ${FILES[*]}; do
	if [[ $VERBOSE == true ]]; then
		echo "Running test $i.. "
	fi

	ACTUAL_OB=ps.ob
	EXPECTED_OB=$(dirname $i)/expected_ps.ob
	
	ACTUAL_ENT=`basename $i`.ent
	EXPECTED_ENT=$(dirname $i)/expected_ps.ent
	
	ACTUAL_EXT=`basename $i`.ext
	EXPECTED_EXT=$(dirname $i)/expected_ps.ext
	
	ACTUAL_LOG=log
	EXPECTED_LOG=$(dirname $i)/expected_log
	
	./Debug/assembler $i
	
	if [ -e $EXPECTED_LOG ]; then
		diff -b $ACTUAL_LOG $EXPECTED_LOG > /dev/null
		[ $? -ne 0 ] && (echo File $i: ; $DIFFTOOL $ACTUAL_LOG $EXPECTED_LOG)		
		
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
			elif [ -e $ACTUAL_ENT ]; then
				echo .ent file was created for test "$i ", but not expectations given
				$DIFFTOOL $ACTUAL_ENT $EXPECTED_ENT
			fi
						
			if [ -e $EXPECTED_EXT ]; then
				if [ ! -e $ACTUAL_EXT ]; then
					echo File "$i " had problems, no ent file found, log file:
					cat $ACTUAL_LOG
				fi
			elif [ -e $ACTUAL_EXT ]; then
				echo .ext file was created for test "$i ", but not expectations given
				$DIFFTOOL $ACTUAL_EXT $EXPECTED_EXT
			fi
			
			if [ -e $ACTUAL_ENT ]; then
				diff -b $ACTUAL_ENT $EXPECTED_ENT > /dev/null
				[ $? -ne 0 ] && (echo File $i: ; $DIFFTOOL $ACTUAL_ENT $EXPECTED_ENT)
			fi
			diff -b $ACTUAL_OB $EXPECTED_OB > /dev/null
			[ $? -ne 0 ] && (echo File $i: ; $DIFFTOOL $ACTUAL_OB $EXPECTED_OB)
		fi
	else
		echo "No expectations given for file $i"
	fi
	
	rm -f $ACTUAL_LOG
	rm -f $ACTUAL_ENT
	rm -f $ACTUAL_EXT
	rm -f $ACTUAL_OB
done
