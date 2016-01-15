#!/bin/bash

help ()
{
	echo "Will open vim to edit input and output files with the given suffix"
	echo "Can receive regular expression to edit existing example."
	echo "For exampel $0 first_example*"
	exit;
}

if [ $# != 1 ] ; then
	echo "Must receive atleast one parameter!"
	help	
fi

if [ $1 == "--help" ] || [ $1 == "-h" ] ; then
	help
fi

NAME=$1
echo $NAME

vim -O2 examples/input-$NAME examples/output-$NAME
