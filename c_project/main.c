/*
 * main.c
 *
 *  Created on: Jan 25, 2016
 *      Author: dorav
 */
#include <ctype.h>
#include <stdio.h>

#define MAX_LINE_SIZE 100

typedef struct
{
	unsigned int is_empty_line : 1;
	unsigned int is_comment : 1;
} InstructionType;

InstructionType defaultInstructionType()
{
	InstructionType returnValue;

	returnValue.is_empty_line = 0;
	returnValue.is_comment = 0;

	return returnValue;
}

short isEmptyLine(char* line)
{
	while (*line)
		if (! isspace(*line))
			return 0;
		else
			++line;

	return 1;
}

InstructionType getInstructionType(char* buffer)
{
	InstructionType returnValue = defaultInstructionType();

	if (buffer[0] == ';')
		returnValue.is_comment = 1;

	if (isEmptyLine(buffer))
		returnValue.is_empty_line = 1;

	return returnValue;
}

int main()
{
	InstructionType instruction;
	char buffer[MAX_LINE_SIZE];

	do
	{
		if (fgets(buffer, MAX_LINE_SIZE, stdin) != buffer || feof(stdin))
			break;

		instruction = getInstructionType(buffer);

		if (instruction.is_empty_line)
			puts("Empty line");
		else if (instruction.is_comment)
		puts("Comment");

	} while(1);

	return 0;
}
