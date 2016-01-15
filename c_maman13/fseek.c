/*
 * main.c
 *
 *  Created on: Jan 11, 2016
 *      Author: dorav
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define NO_ARGUMENTS 1
#define NOT_ENOUGH_ARGUMENTS 2
#define BAD_CHARACTERS_NUMBER 3
#define BAD_FILE 4

/*
 * Prints a nice usage message
 */
void printUsage(char* programName)
{
	printf("Usage: %s [character-number] [file-list...]\n", programName);
	printf("For example - %s 5 /tmp/first.in /tmp/second.in", programName);
}

#define NEGATIVE 0
#define POSITIVE 1
/*
 * Returns 0 if input contains '-'.
 * Returns 1 otherwise.
 */
int isPositive(const char* input)
{
	while (*input != '\0')
	{
		if (*input == '-')
			return NEGATIVE;
		++input;
	}

	return POSITIVE;
}

/*
 * Tries to convert a given char array to an unsigned integer.
 * The result will be placed in the result parameter.
 *
 * Will fail if the given string is not a positive integer.
 * Will not modify the result parameter on failure.
 * Returns 0 on success and non-zero on failure.
 */
int isCharacterNumberInvalid(const char* input, unsigned long* result)
{
	char* firstBadChar;
	unsigned long tempValue = strtoul(input, &firstBadChar, 10);
	if (*firstBadChar == '\0' && errno != ERANGE && isPositive(input) == POSITIVE)
	{
		*result = tempValue;
		return 0;
	}

	return ERANGE;
}

/*
 * Prints the character 'c' that was #n in the file
 * Will not print spaces or unprintable characters like \b
 * It will print their ASCII values instead
 */
void printToUser(char fileName[], unsigned long n, char c)
{
	printf("character number %lu, in file '%s' is ", n, fileName);
	if (isgraph(c))
		printf("'%c'\n", c);
	else
		printf("%d (ASCII value, because it can't be printed nicely)\n", (int) c);
}

#define CHARS_TO_READ 1
#define SIZE_OF_CHARS_TO_READ sizeof(char) * CHARS_TO_READ

/*
 * Reads a char from an opened file and puts the value in c.
 * Returns non-zero if successful and zero otherwise.
 */
int readChar(char* c, FILE* file)
{
	return fread(c, sizeof(char), CHARS_TO_READ, file) == SIZE_OF_CHARS_TO_READ;
}

/*
 * We define this as binary because we want to read all characters,
 * including the given example of \n which should lead to 10 and 13
 */
#define FILE_MODE "rb"

/*
 * Prints the n'th character of the given file
 * If the file can't be opened for reading, it will exit with BAD_FILE
 */
void printCharNOfFile(unsigned long n, char* fileName)
{
	char c;

	FILE* file = fopen(fileName, FILE_MODE);
	if (file == NULL)
	{
		printf("Can't open file '%s', %s.\n", fileName, strerror(errno));
		exit(BAD_FILE);
	}

	fseek(file, n, SEEK_SET);
	if (readChar(&c, file))
		printToUser(fileName, n + 1, c);
	else
		printf("The following file did not have %lu characters in it, %s\n", n + 1, fileName);

	fclose(file);
}

/*
 * Prints the n'th character of the wanted files in the array.
 * The number of wanted files should be specified in numberOfFiles.
 */
void printNthOfFiles(int numberOfFiles, char* files[], unsigned long characterNumber)
{
	int i;
	printf("Showing character number %lu of %d files\n", characterNumber, numberOfFiles);
	for (i = 0; i < numberOfFiles; ++i)
		printCharNOfFile(characterNumber - 1, files[i]);
}

#define FIRST_ARG_INDEX 1
#define PROGRAM_ARG 0
#define CHARACTER_NUM_ARG 1
#define FIRST_FILE_ARG 2

#define USAGE printUsage(argv[PROGRAM_ARG])

/*
 * Verifies that the user had enough arguments for input.
 * If not, will print usage and exit with the proper exit code
 */
void validateNumberOfArguments(int argc, char* argv[])
{
	int realArgIndex = argc - FIRST_ARG_INDEX;
	if (realArgIndex < CHARACTER_NUM_ARG)
	{
		printf("Bad usage - no arguments given\n");
		USAGE;
		exit(NO_ARGUMENTS);
	}

	if (realArgIndex == CHARACTER_NUM_ARG)
	{
		printf("Not enough arguments, need to be > %d\n", CHARACTER_NUM_ARG);
		USAGE;
		exit(NOT_ENOUGH_ARGUMENTS);
	}
}

/*
 * Returns the number of wanted characters to be printed from file.
 * Assumes that the input given in argv is validated with validateNumberOfArguments()
 *
 * Will exit if the argument is invalid.
 */
unsigned long getCharacterNumberFromArgs(char* argv[])
{
	unsigned long characterNum;
	char* characterNumArgument = argv[CHARACTER_NUM_ARG];
	if (isCharacterNumberInvalid(characterNumArgument, &characterNum) || characterNum == 0)
	{
		printf("Invalid 1'st parameter [character-number], must be positive integer "
		       "(zero not included) and smaller then %lu. Was given - '%s'.\n", ULONG_MAX, characterNumArgument);
		USAGE;
		exit(BAD_CHARACTERS_NUMBER);
	}
	return characterNum;
}

int main(int argc, char *argv[])
{
	unsigned long characterNum;

	validateNumberOfArguments(argc, argv);

	characterNum = getCharacterNumberFromArgs(argv);
	printNthOfFiles(argc - FIRST_FILE_ARG, argv + FIRST_FILE_ARG, characterNum);

	return 0;
}
