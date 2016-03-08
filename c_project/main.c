/*
 * main.c
 *
 *  Created on: Jan 25, 2016
 *      Author: dorav
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "hash_table.h"
#define MAX_LINE_SIZE 100
#define FALSE 0
#define TRUE 1

/* Created using isspace function */
static char space_chars[] = {9, 10, 11, 12, 13, 32};

typedef struct
{
	int lineNumber;
	char data[MAX_LINE_SIZE];
	int hasError;
} Line;

typedef struct
{
	int isEmptyLine;
	int isComment;
	int isParsingRequired;
} LineType;

LineType defaultLineType()
{
	LineType returnValue = { FALSE };

	return returnValue;
}

short isSpaces(const char* line)
{
	while (*line)
		if (! isspace(*line))
			return 0;
		else
			++line;

	return 1;
}

typedef struct
{
	unsigned int instruction_counter;
	unsigned int data_counter;
	unsigned int numberOfErrors;
	hash_table cmds;
} ProgramData;

LineType getLineType(Line* line)
{
	LineType returnValue = defaultLineType();

	if (line->data[0] == ';')
		returnValue.isComment = TRUE;
	else if (isSpaces(line->data))
		returnValue.isEmptyLine = TRUE;
	else
		returnValue.isParsingRequired = TRUE;

	return returnValue;
}

#define NO_ARGUMENTS_ERROR 1

#define PROGRAM_ARG 0
#define FIRST_ARG_INDEX 1
#define FILE_NAME_ARG_INDEX 1

void printUsage(char* programName)
{
	printf("Usage: %s [filename]\n", programName);
	printf("For example - %s /tmp/program.as", programName);
}

#define USAGE printUsage(argv[PROGRAM_ARG])

void validateNumberOfArguments(int argc, char* argv[])
{
	int realArgIndex = argc - FIRST_ARG_INDEX;
	if (realArgIndex != FILE_NAME_ARG_INDEX)
	{
		printf("Bad usage - no arguments given\n");
		USAGE;
		exit(NO_ARGUMENTS_ERROR);
	}
}

FILE* getInputfile(char* fileName)
{
	return fopen(fileName, "r");
}

FILE* getObjectOutFile()
{
	return fopen("ps.ob", "w");
}

#define OPCODE_LOCATION 6
#define OPCODE_MASK (0xF << OPCODE_LOCATION)

/* This define is here for compile-time calculation
 * Compile time calculation is needed in c90 for {} initialization of structs */
#define OPCODE_TO_BINARY(opcode) ((opcode << OPCODE_LOCATION) & OPCODE_MASK)

void putCommandOpcode(CPUInstruction* dest, CommandOpcode value)
{
	dest->bits &= (!OPCODE_MASK); /* set all opcode bits off */
	dest->bits ^= OPCODE_TO_BINARY(value); /* set only the wanted bits on */
}

void parseNoArgsCommand(const Line* line, CPUInstruction* instruction, const UserCommand* command)
{
	char* argument = strtok(NULL, space_chars);

	if (argument == NULL || isSpaces(argument))
		putCommandOpcode(instruction, command->opcode);
	else
	{
		printf("line %d: invalid arguments after \"%s\". Expected '0' arguments.\n", line->lineNumber, command->name);
	}
}

/* Internal function for calculating a given command's hash code.
 * command parameter must not be NULL.
 *
 * Do not try to use it for a general case hash table.
 * It is a very specific implementation for hashing the given commands.
 */
unsigned int prehashCommand(ObjectType command)
{
	/* Good enough prehash for this use-case.
	 * All valid commands are length < 5 and small letters*/
	return ((const char*) command)[0] - 'a';
}

#define Equal 1
#define NotEqual 0

int isEqual(const UserCommand* cmd, const char* name)
{
	return strcmp(cmd->name, name) == 0;
}

#define NotFound ""

UserCommand rtsCommand = { RtsOpcode, "rts" };
UserCommand stopCommand = { StopOpcode, "stop" };

CPUInstruction parseLine(ProgramData* data, Line* line)
{
	CPUInstruction i = {0};

	char* commandName = strtok(line->data, space_chars);
	const UserCommand* command = hash_find(&data->cmds, commandName);

	if (command != NULL)
	{
		parseNoArgsCommand(line, &i, command);
	}
	else
	{
		printf("line %d: No such command \"%s\"\n", line->lineNumber, commandName);
		line->hasError = TRUE;
	}

	return i;
}

char to_32bit(unsigned int value)
{
	if (value < 10)
		return '0' + value;
	return 'A' + value - 10;
}

void reverseStart(int end, char* buffer)
{
	int start = 0;
	for (; end > start; --end, ++start)
	{
		char temp = buffer[end];
		buffer[end] = buffer[start];
		buffer[start] = temp;
	}
}

int pad_to(int len, char* buffer, int padNum)
{
	while (len < padNum)
		buffer[len++] = '0';

	return len;
}

void to_32basePadded(unsigned int value, char* buffer, int padSize)
{
	int bitNum = 0;

	do
	{
		buffer[bitNum++] = to_32bit(value % 32);
		value /= 32;
	} while (value != 0);

	bitNum = pad_to(bitNum, buffer, padSize);
	buffer[bitNum--] = '\0';

	reverseStart(bitNum, buffer);
}

void to_32base(unsigned int value, char* buffer)
{
	to_32basePadded(value, buffer, 0);
}

void printInstruction(FILE* f, const ProgramData* data, CPUInstruction i)
{
	static char instruction32base[10];
	static char instructionCounter32base[10];

	to_32basePadded(data->instruction_counter, instructionCounter32base, 3);
	to_32basePadded(i.bits, instruction32base, 3);

	fprintf(f, "%s %s\n", instructionCounter32base, instruction32base);
}

void printCounterHeader(FILE* f, const ProgramData* data)
{
	static char instructionCounter32base[10];
	static char dataCounter2base[10];

	to_32base(data->instruction_counter, instructionCounter32base);
	to_32base(data->data_counter, dataCounter2base);

	fprintf(f, "%s %s\n", instructionCounter32base, dataCounter2base);
}

void validateInstruction(ProgramData* data, CPUInstruction i)
{
	++(data->instruction_counter);
	(void)i;
}

int getLine(Line* line, FILE* inputFile)
{
	++(line->lineNumber);
	return fgets(line->data, MAX_LINE_SIZE, inputFile) == line->data && !feof(inputFile);
}

int shouldIgnoreLine(const LineType* instruction)
{
	return instruction->isEmptyLine || instruction->isComment;
}

void firstRun(FILE* inputFile,
			  ProgramData* data)
{
	Line line = { 0 };
	LineType instruction;

	while (getLine(&line, inputFile))
	{
		instruction = getLineType(&line);

		if (shouldIgnoreLine(&instruction) == FALSE)
		{
			validateInstruction(data, parseLine(data, &line));
			if (line.hasError == TRUE)
				++(data->numberOfErrors);
		}
	}
}

void secondRun(ProgramData* data,
			   FILE* inputFile,
			   FILE* objectOutFile)
{
	LineType instruction;
	Line l = { 0 };

	while (getLine(&l, inputFile))
	{
		instruction = getLineType(&l);

		if (shouldIgnoreLine(&instruction) == FALSE)
		{
			printInstruction(objectOutFile, data, parseLine(data, &l));
			data->instruction_counter += 1;
		}
	}
}

boolean strcmp_(const KeyType key, const ObjectType object)
{
	return strcmp((const char*)key, ((const UserCommand*)object)->name) == 0;
}


int main(int argc, char** argv)
{
	FILE* inputFile;
	FILE* objectOutFile;
	ProgramData data = {0};
	ObjectMetadata meta = { prehashCommand, strcmp_, sizeof(UserCommand) };

	FILE* status = freopen("log", "w", stdout);

	data.cmds = newHashTable(meta);
	if (status == NULL)
		puts("Bad redirect");

	validateNumberOfArguments(argc, argv);

	inputFile = getInputfile(argv[FILE_NAME_ARG_INDEX]);

	if (inputFile == NULL)
	{
		puts("BAD FILE");
	}

	hash_insert(&data.cmds, stopCommand.name, &stopCommand);
	hash_insert(&data.cmds, rtsCommand.name, &rtsCommand);

	firstRun(inputFile, &data);

	fclose(inputFile);
	inputFile = getInputfile(argv[FILE_NAME_ARG_INDEX]);


	if (data.numberOfErrors == 0)
	{
		objectOutFile = getObjectOutFile();

		if (objectOutFile == NULL)
		{
			puts("BAD OUTPUT FILE");
		}

		printCounterHeader(objectOutFile, &data);
		data.instruction_counter = 100;

		secondRun(&data, inputFile, objectOutFile);
		fclose(objectOutFile);
	}

	fclose(status);
	fclose(inputFile);

	return 0;
}
