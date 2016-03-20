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
#include <time.h>

#include "binary_utils.h"
#include "types.h"
#include "hash_table.h"
#include "utility.h"
#include "commands.h"
#include "program_ds_manipulators.h"

boolean shouldParse(Line* line)
{
	/* Could be combined to a single if, but it's easier to debug this */
	if (line->data[0] == ';')
		return False;
	else if (isSpaces(line->data))
		return False;

	return True;
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
	if (realArgIndex < FILE_NAME_ARG_INDEX)
	{
		printf("Bad usage - no arguments given\n");
		USAGE;
		exit(NO_ARGUMENTS_ERROR);
	}
}

FILE* getInputfile(const char* fileName)
{
	return fopen(fileName, "r");
}

FILE* getObjectOutFile()
{
	return fopen("ps.ob", "w");
}

boolean getLine(Line* line, FILE* inputFile)
{
	Line newLine = { 0 };
	newLine.lineNumber = line->lineNumber + 1;
	*line = newLine;

	if (fgets(line->data, MAX_LINE_SIZE, inputFile) == NULL)
	{
		if (ferror(inputFile))
		{
			printf("At line %d, Unknown error occurred. Program was unable to read the line, "
					"errorno = %d.\n", line->lineNumber, feof(inputFile));
			line->hasError = True;
		}
		return False;
	}

	if (!hasLineEnd(line->data) && !feof(inputFile))
	{
		printf("At line %d, line is too long, max line length = %d.\n",
				line->lineNumber, MAX_LINE_SIZE);
		line->hasError = True;
		return False;
	}

	return True;
}

#define BASE_ADDRESS 100

void prepareDataFor2ndRun(ProgramData* data)
{
	lhash_iter i;
	Symbol* current;

	for (i = lhash_begin(&data->symbols); i.isValid; lhash_set_next(&i))
	{
		current = (Symbol*) i.current->data;
		current->referencedMemAddr += BASE_ADDRESS;
		if (current->isDataLabel)
			current->referencedMemAddr += data->instruction_counter;
	}

	data->instruction_counter = BASE_ADDRESS;
	data->data_counter = 0;
}

void firstRun(FILE* inputFile, ProgramData* data)
{
	UserCommandResult parsedLine;
	Line line = { 0 };

	data->inFirstRun = True;
	while (getLine(&line, inputFile))
	{
		if (shouldParse(&line))
		{
			parsedLine = parseLine(data, &line);
			if (line.hasError == True)
				++(data->numberOfErrors);

			data->instruction_counter += parsedLine.instructionSize;
		}
	}

	/* Error while fetching line */
	if (line.hasError)
		data->numberOfErrors++;

	data->inFirstRun = False;
}

void printDataStorage(ProgramData* data, FILE* objectOutFile)
{
	unsigned int i;

	for (i = 0; i < data->data_counter; ++i)
		printByte(objectOutFile, data->instruction_counter + i, data->dataStorage[i]);
}

void secondRun(ProgramData* data,
			   FILE* inputFile,
			   FILE* objectOutFile)
{
	UserCommandResult parsed;
	Line line = { 0 };

	while (getLine(&line, inputFile))
	{
		if (shouldParse(&line))
		{
			parsed = parseLine(data, &line);
			printInstruction(objectOutFile, data, parsed);

			/* The print instruction needs to know the current instruction counter
			 * thats why this can't be done inside parseLine */
			data->instruction_counter += parsed.instructionSize;
		}
	}

	printDataStorage(data, objectOutFile);
}

#define ENTRY_FILE_EXT ".ent"

void validateEntries(ProgramData* data)
{
	lhash_iter i;
	Symbol* entry;
	Symbol* referencedLabel;

	for (i = lhash_begin(&data->entries); i.isValid; lhash_set_next(&i))
	{
		entry = (Symbol*) i.current->data;
		referencedLabel  = lhash_find(&data->symbols, entry->name);
		if (referencedLabel == NULL)
		{
			printf("At line %d, expected label '%s', to be defined somewhere. Label never found.\n",
					entry->lineNumber, entry->name);
			data->numberOfErrors++;
		}
		else if (referencedLabel->isExternal)
		{
			printf("At line %d, expected label '%s' cannot be an external reference from line %d.\n",
					entry->lineNumber, entry->name, referencedLabel->lineNumber);
			data->numberOfErrors++;
		}
	}
}

void printUnresolvedRandomLabels(ProgramData* data)
{
	lhash_iter i;

	for (i = lhash_begin(&data->randomLabelLines); i.isValid; lhash_set_next(&i))
		printf("At line %d, Requested random label, but no labels were found.\n", *(int*) i.current->data);

	/* TODO: empty the randomLabelLines table */
}

void validateUnresolvedSymbols(ProgramData* data)
{
	lhash_iter i;
	Symbol* current;
	Symbol* referencedLabel;
	int numberOfInternalSymbols = 0;

	for (i = lhash_begin(&data->symbols); i.isValid; lhash_set_next(&i))
	{
		current = (Symbol*) i.current->data;
		if (!current->isExternal)
			numberOfInternalSymbols++;
	}

	/* Has unresolved random address labels */
	if (numberOfInternalSymbols == 0 && data->randomLabelLines.numberOfUsed > 0)
	{
		printUnresolvedRandomLabels(data);
		data->numberOfErrors++;
	}

	for (i = lhash_begin(&data->unresolvedSymbols); i.isValid; lhash_set_next(&i))
	{
		current = (Symbol*) i.current->data;
		referencedLabel  = lhash_find(&data->symbols, current->name);
		if (referencedLabel == NULL)
		{
			printf("At line %d, expected label '%s', to be defined somewhere. Label never found.\n",
					current->lineNumber, current->name);
			data->numberOfErrors++;
		}
	}

	/* TODO: empty the unresolved table */
}


void writeEntriesFile(ProgramData* data, const char* inputFileName)
{
	static char addr32base[10];

	FILE* entriesFile = getOutFile(inputFileName, ENTRY_FILE_EXT);
	lhash_iter i;
	Symbol* entry;
	Symbol* referencedLabel;

	for (i = lhash_begin(&data->entries); i.isValid; lhash_set_next(&i))
	{
		entry = (Symbol*) i.current->data;
		referencedLabel = lhash_find(&data->symbols, entry->name);
		to_32base(referencedLabel->referencedMemAddr, addr32base);
		fprintf(entriesFile, "%s %s\n", referencedLabel->name, addr32base);
	}

	fclose(entriesFile);
}

int main(int argc, char** argv)
{
	FILE* inputFile;
	FILE* objectOutFile;
	ProgramData data = { 0 };

	FILE* status = freopen("log", "w", stdout);
	if (status == NULL)
		puts("Bad redirect");

	srand(time(NULL));

	validateNumberOfArguments(argc, argv);

	data = initProgramData(argv[FILE_NAME_ARG_INDEX]);

	inputFile = getInputfile(data.inputFileName);
	if (inputFile == NULL)
		puts("BAD FILE");

	firstRun(inputFile, &data);

	fclose(inputFile);

	validateEntries(&data);
	validateUnresolvedSymbols(&data);

	inputFile = getInputfile(data.inputFileName);

	if (data.numberOfErrors == 0)
	{
		objectOutFile = getObjectOutFile();

		if (objectOutFile == NULL)
		{
			puts("BAD OUTPUT FILE");
		}

		printCounterHeader(objectOutFile, &data);

		prepareDataFor2ndRun(&data);

		secondRun(&data, inputFile, objectOutFile);
		fclose(objectOutFile);

		if (data.entries.numberOfUsed > 0)
			writeEntriesFile(&data, data.inputFileName);
	}

	fclose(status);
	fclose(inputFile);

	if (data.externalReferencesFile != NULL)
		fclose(data.externalReferencesFile);

	return 0;
}
