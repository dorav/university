#!/bin/bash

# this is a really stupid integration test as it includes
# formatted log commands, but it's the best i have energy for right now

DIFFTOOL=diff

[[ $1 == '-g' ]] && DIFFTOOL=vimdiff

test_failed ()
{
	echo "Test $1 Failed, stopping"
	$DIFFTOOL $2 $3
	exit;
}

EXAMPLES_FOLDER="examples/"
PARAM_EXAMPLES_FOLDER="$EXAMPLES_FOLDER/parameterized"

for INPUT in `find $EXAMPLES_FOLDER/ -maxdepth 1 -iname "input*"`; do
	TEST_NAME=$(echo $INPUT | cut -d'-' -f2)
	OUTPUT_NAME=output-$TEST_NAME
	ACTUAL=/tmp/$OUTPUT_NAME
	EXPECTED=$EXAMPLES_FOLDER/$OUTPUT_NAME
	(./n_char `cat $INPUT`) > $ACTUAL 2> /dev/null ;
	echo Comparing $EXPECTED with $ACTUAL
	diff -b $EXPECTED $ACTUAL > /dev/null || test_failed $INPUT $EXPECTED $ACTUAL
done

change_parameter()
{
	FILE=$1;
	WANTED_TOKEN=$2
	local WANTED_TOKEN_EXCLUDED=$WANTED_TOKEN
	if [[ $WANTED_TOKEN == "B" ]]; then 
		WANTED_TOKEN_EXCLUDED="A"; 
	fi
	sed -i "s@ARG_WITHOUT_B@$WANTED_TOKEN_EXCLUDED@g" $FILE
	sed -i "s@ARG@$WANTED_TOKEN@g" $FILE
}

diff_with_parameter ()
{
	for LETTER in {A..F}; do
		INPUT_P=/tmp/$(basename ${INPUT})_$LETTER
		ACTUAL_P=/tmp/$(basename ${ACTUAL})_$LETTER
		EXPECTED_P=/tmp/expected_$(basename ${EXPECTED})_$LETTER
		cp $EXPECTED $EXPECTED_P
		cp $INPUT $INPUT_P
		change_parameter $EXPECTED_P $LETTER
		change_parameter $INPUT_P $LETTER
		./complex < $INPUT_P > $ACTUAL_P 2> /dev/null ;
		echo Comparing ${EXPECTED_P} with ${ACTUAL_P}
		diff $EXPECTED_P $ACTUAL_P > /dev/null || test_failed $INPUT $EXPECTED_P $ACTUAL_P
	done
}

if [[ -e $PARAM_EXAMPLES_FOLDER ]]; then
	for INPUT in `find $PARAM_EXAMPLES_FOLDER -iname "input*"`; do
		TEST_NAME=$(echo $INPUT | cut -d'-' -f2)
		OUTPUT_NAME=output-$TEST_NAME
		ACTUAL=/tmp/$OUTPUT_NAME
		EXPECTED=$PARAM_EXAMPLES_FOLDER/$OUTPUT_NAME
		diff_with_parameter
	done
fi

echo "Tests passed"
