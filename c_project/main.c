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

void printInstruction(FILE* f, const ProgramData* data, UserCommandResult i)
{
	static char instruction32base[10];
	static char instructionCounter32base[10];

	if (i.instructionSize == 0)
		return;

	to_32basePadded(data->instruction_counter, instructionCounter32base, 3);
	to_32basePadded(i.instructionBytes.bits, instruction32base, 3);

	fprintf(f, "%s %s\n", instructionCounter32base, instruction32base);

	if (i.instructionSize == 1)
		return;

	to_32basePadded(data->instruction_counter + 1, instructionCounter32base, 3);
	to_32basePadded(i.firstArgBytes.bits, instruction32base, 3);

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

	if (!hasLineEnd(line->data))
	{
		printf("At line %d, line is too long, max line length = %d.\n",
				line->lineNumber, MAX_LINE_SIZE);
		line->hasError = True;
		return False;
	}

	return True;
}

void fixSymbolAddresses(ProgramData* data)
{
	lhash_iter i;
	Symbol* current;

	for (i = lhash_begin(&data->symbols); i.isValid; lhash_set_next(&i))
	{
		current = (Symbol*) i.current->data;
		current->referencedMemAddr += data->instruction_counter;
	}
}

void firstRun(FILE* inputFile,
			  ProgramData* data)
{
	UserCommandResult parsedLine;
	Symbol* lineLabel;
	Line line = { 0 };

	data->inFirstRun = True;
	while (getLine(&line, inputFile))
	{
		if (shouldParse(&line))
		{
			parsedLine = parseLine(data, &line);
			if (line.hasError == True)
				++(data->numberOfErrors);

			if (line.hasLabel)
			{
				lineLabel = (Symbol*)lhash_find(&data->symbols, line.labelName);
				lineLabel->referencedMemAddr = data->instruction_counter;
			}

			data->instruction_counter += parsedLine.instructionSize;
		}
	}

	/* Error while fetching line */
	if (line.hasError)
		data->numberOfErrors++;

	data->inFirstRun = False;
}

void secondRun(ProgramData* data,
			   FILE* inputFile,
			   FILE* objectOutFile)
{
	UserCommandResult parsed;
	Line line = { 0 };

	fixSymbolAddresses(data);

	while (getLine(&line, inputFile))
	{
		if (shouldParse(&line))
		{
			parsed = parseLine(data, &line);
			printInstruction(objectOutFile, data, parsed);

			data->instruction_counter += parsed.instructionSize;
		}
	}
}

#define ENTRY_FILE_EXT ".ent"
/* For both linux and windows */
#define DIRECTORY_DELIMITERS "/\\"

char* getFileNameWithExt(const char* inputFileName, const char* ext)
{
	int outputFileNameLen = strlen(inputFileName) + strlen(ext) + 1;
	char* outputFileName = calloc(1, outputFileNameLen);

	token t = strtok_begin_cp(inputFileName, DIRECTORY_DELIMITERS, outputFileName);
	while (strtok_next(t, DIRECTORY_DELIMITERS).start != NULL)
		t = strtok_next_cp(t, DIRECTORY_DELIMITERS, outputFileName);

	strcat(outputFileName, ext);

	return outputFileName;
}

FILE* getEntriesOutFile(const char* inputFileName)
{
	char* outputFileName = getFileNameWithExt(inputFileName, ENTRY_FILE_EXT);
	FILE* file;

	file = fopen(outputFileName, "w");

	if (file == NULL)
	{
		puts("BAD OUTPUT FILE");
	}

	free (outputFileName);

	return file;
}

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

void validateUnresolvedSymbols(ProgramData* data)
{
	lhash_iter i;
	Symbol* entry;
	Symbol* referencedLabel;

	for (i = lhash_begin(&data->unresolvedSymbols); i.isValid; lhash_set_next(&i))
	{
		entry = (Symbol*) i.current->data;
		referencedLabel  = lhash_find(&data->symbols, entry->name);
		if (referencedLabel == NULL)
		{
			printf("At line %d, expected label '%s', to be defined somewhere. Label never found.\n",
					entry->lineNumber, entry->name);
			data->numberOfErrors++;
		}
	}

	/* TODO: empty the unresolved table */
}


void writeEntriesFile(ProgramData* data, const char* inputFileName)
{
	static char addr32base[10];

	FILE* entriesFile = getEntriesOutFile(inputFileName);
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
	ProgramData data = initProgramData();

	FILE* status = freopen("log", "w", stdout);
	if (status == NULL)
		puts("Bad redirect");

	inputFile = getInputfile(argv[FILE_NAME_ARG_INDEX]);
	if (inputFile == NULL)
		puts("BAD FILE");

	validateNumberOfArguments(argc, argv);

	firstRun(inputFile, &data);

	fclose(inputFile);

	validateEntries(&data);
	validateUnresolvedSymbols(&data);

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

		if (data.entries.numberOfUsed > 0)
			writeEntriesFile(&data, argv[FILE_NAME_ARG_INDEX]);
	}

	fclose(status);
	fclose(inputFile);

	return 0;
}
