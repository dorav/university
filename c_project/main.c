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

#include "assembler_specific_utilities.h"
#include "binary_utils.h"
#include "types.h"
#include "hash_table.h"
#include "utility.h"
#include "commands.h"
#include "program_ds_manipulators.h"

boolean shouldParse(Line* line)
{
	const char* data = line->data;

	if (isSpaces(data))
		return False;

	while (isspace(*data))
		data++;

	/* Could be combined to a single if, but it's easier to debug this */
	if (data[0] == ';')
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
	FILE* f = fopen(fileName, "r");

	if (f == NULL)
		printf("Error, can't open file \"%s\" for reading, skipping.\n", fileName);

	return f;
}

FILE* getObjectOutFile(const char* outFileName)
{
	FILE* f = fopen(outFileName, "w");
	if (f == NULL)
		printf("Can't open output file: \"%s\".\n", outFileName);

	return f;
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

void firstRunImpl(ProgramData* data, FILE* inputFile)
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

void firstRun(ProgramData* data)
{
	FILE* inputFile = getInputfile(data->inputFileName);

	if(inputFile == NULL)
	{
		data->numberOfErrors++;
		return;
	}

	firstRunImpl(data, inputFile);
	fclose(inputFile);
}

void printDataStorage(ProgramData* data, FILE* objectOutFile)
{
	unsigned int i;

	for (i = 0; i < data->data_counter; ++i)
		printByte(objectOutFile, data->instruction_counter + i, data->dataStorage[i]);
}

void secondRunImpl(FILE* inputFile, ProgramData* data, FILE* objectOutFile)
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

void secondRun(ProgramData* data,
			   FILE* objectOutFile)
{
	FILE* inputFile = getInputfile(data->inputFileName);

	if (inputFile == NULL)
	{
		data->numberOfErrors++;
		return;
	}

	secondRunImpl(inputFile, data, objectOutFile);

	fclose(inputFile);
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

	lhash_free(&data->randomLabelLines);
}

void validateUnresolvedSymbols(ProgramData* data)
{
	lhash_iter i;
	Symbol* current;
	Symbol* referencedLabel;
	int numberOfInternalSymbols = numOfLabels(data);

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

	lhash_free(&data->unresolvedSymbols);
}

void writeToEntriesFile(ProgramData* data, FILE* entriesFile)
{
	static char addr32base[10];

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
}

void writeEntriesFile(ProgramData* data)
{
	FILE* entriesFile = getOutFile(data->inputFileName, ENTRY_FILE_EXT);

	if (entriesFile == NULL)
	{
		data->numberOfErrors++;
		return;
	}

	writeToEntriesFile(data, entriesFile);
	fclose(entriesFile);
}

#define OBJECT_FILE_EXT ".ob"

void writeData(ProgramData* data)
{
	FILE* objectOutFile = getOutFile(data->inputFileName, OBJECT_FILE_EXT);

	if (objectOutFile == NULL)
		return;

	printCounterHeader(objectOutFile, data);

	prepareDataFor2ndRun(data);

	secondRun(data, objectOutFile);

	fclose(objectOutFile);

	if (data->entries.numberOfUsed > 0)
		writeEntriesFile(data);
}

void runOnfile(const char* fileName)
{
	ProgramData data = initProgramData(fileName);

	firstRun(&data);

	validateEntries(&data);
	validateUnresolvedSymbols(&data);

	if (data.numberOfErrors == 0)
		writeData(&data);

	if (data.externalReferencesFile != NULL)
		fclose(data.externalReferencesFile);

	lhash_free(&data.symbols);
	lhash_free(&data.registers);
	lhash_free(&data.entries);
	ohash_free(&data.cmds);
	if (data.dataStorage != NULL)
		free(data.dataStorage);
}

int main(int argc, char** argv)
{
	int i;
	char* fileName;
	FILE* status = freopen("log", "w", stdout);

	srand(time(NULL));
	validateNumberOfArguments(argc, argv);

	for (i = 0; i < argc - FILE_NAME_ARG_INDEX; ++i)
	{
		fileName = argv[i + FILE_NAME_ARG_INDEX];
		printf("Parsing input file - \"%s\"\n", fileName);
		runOnfile(fileName);
	}

	fclose(status);
	return 0;
}
