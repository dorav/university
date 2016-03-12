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
#include "utility.h"

/* Created using isspace function */
static char space_chars[] = {9, 10, 11, 12, 13, 32};

boolean isSpaces(const char* line)
{
	while (*line)
		if (!isspace(*line))
			return False;
		else
			++line;

	return True;
}

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

#define OPCODE_LOCATION 6
#define OPCODE_MASK (0xF << OPCODE_LOCATION)

/* This define is here for compile-time calculation
 * Compile time calculation is needed in c90 for {} initialization of structs */
#define OPCODE_TO_BINARY(opcode) ((opcode << OPCODE_LOCATION) & OPCODE_MASK)

void putCommandOpcode(UserCommandResult* dest, CommandOpcode value)
{
	dest->instructionBits &= (!OPCODE_MASK); /* set all opcode bits off */
	dest->instructionBits ^= OPCODE_TO_BINARY(value); /* set only the wanted bits on */
}

UserCommandResult nullInstruction()
{
	UserCommandResult i = { 0 };
	return i;
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

#define UNUSED(thing) (void)thing

unsigned int stupidhash(ObjectType command)
{
	UNUSED(command);
	return 0;
}

#define Equal 1
#define NotEqual 0

int isEqual(const UserCommand* cmd, const char* name)
{
	return strcmp(cmd->name, name) == 0;
}

#define NotFound ""

#define MAX_LABEL_LENGTH 30

boolean validateLabelName_(ProgramData* data, Line* line, const char* labelName)
{
	int len = 0;
	/* first char must be a letter */
	if (!isalpha(*labelName))
	{
		printf("At line %d, label name \"%s\", cannot start with a non alphabetic character.\n", line->lineNumber, labelName);
		return False;
	}

	++len;

	/* rest of the label name must contain only alpha-numerical characters */
	while (*(labelName + len) != '\0')
	{
		if (!isalnum(*(labelName + len)))
		{
			printf("At line %d, label name \"%s\", cannot contain alpha-numeric character.\n", line->lineNumber, labelName);
			return False;
		}

		++len;
	}

	if (len > MAX_LABEL_LENGTH)
	{
		printf("At line %d, label name \"%s\", cannot be more then %d chars long.\n", line->lineNumber, labelName, MAX_LABEL_LENGTH);
		return False;
	}

	if (ohash_find(&data->cmds, labelName) != NULL)
	{
		printf("At line %d, label name \"%s\", is an instruction keyword, choose a different name.\n", line->lineNumber, labelName);
		return False;
	}

	if (lhash_find(&data->registers, labelName) != NULL)
	{
		printf("At line %d, label name \"%s\", is a register name, choose a different name.\n", line->lineNumber, labelName);
		return False;
	}

	return True;
}
static const char labelDelimiters[] = ":";

boolean hasLabel(const char* data)
{
	/* This can also be a comment or a string
	 * But comments are filtered before and strings also have a label. */
	while (*data != '\0')
	{
		if (isDelimiter(*data, labelDelimiters))
			return True;
		data++;
	}

	return False;
}

const char* validateLabelName(ProgramData* data, const char* name,	Line* line)
{
	static char labelName[MAX_LINE_SIZE];
	strtok_begin_cp(name, labelDelimiters, labelName);
	if (validateLabelName_(data, line, labelName) == False)
		return NULL;

	return labelName;
}

Symbol* newSymbol(const char* labelName, Line* line)
{
	Symbol* s = calloc(sizeof(Symbol), 1);

	s->lineNumber = line->lineNumber;
	strcpy(s->name, labelName);

	return s;
}

void insertEntry(ProgramData* data, const char* labelName, Line* line)
{
	lhash_insert(&data->entries, labelName, newSymbol(labelName, line));
}

void insertLabel(ProgramData* data, const char* labelName, Line* line)
{
	lhash_insert(&data->symbols, labelName, newSymbol(labelName, line));
}

void registerSymbol(ProgramData* data, const char* labelName, Line* line)
{
	const Symbol* s = lhash_find(&data->symbols, labelName);
	if (s == NULL)
		insertLabel(data, labelName, line);
	else
	{
		printf("At line %d, invalid label definition '%s', collision with label defined at line %d\n", line->lineNumber, labelName, s->lineNumber);
		line->hasError = True;
	}
}

/* Validates and register the label in the symbol table.
 *
 * Returns a pointer to the next token in the line
 */
char* parseLabel(ProgramData* data, Line* line, char* firstToken)
{
	token labelToken = strtok_begin(line->data, labelDelimiters);
	const char* labelDelimiter = labelToken.end + 1; /* Not '\0' because label exists */
	const char* restOfTheLine = labelDelimiter + 1;
	const char* labelName;

	/* If line starts with spaces and followed by label delimiter */
	if (isDelimiter(*firstToken, labelDelimiters))
	{
		printf("At line %d, missing label name before ':'\n", line->lineNumber);
		line->hasError = True;
		return NULL;
	}

	if ((labelName = validateLabelName(data, firstToken, line)) == NULL)
	{
		line->hasError = True;
		return NULL;
	}

	/* Checking if the rest of the line is not empty or only filled with white spaces */
	if (*restOfTheLine == '\0' || isSpaces(restOfTheLine))
	{
		printf("At line %d, after label '%s', line must contain more content\n", line->lineNumber, labelName);
		line->hasError = True;
		return NULL;
	}
	else if (isDelimiter(*restOfTheLine, labelDelimiters))
	{
		printf("At line %d, after label '%s', too many label delimiters ':'\n", line->lineNumber, labelName);
		line->hasError = True;
		return NULL;
	}

	if (data->inFirstRun)
		registerSymbol(data, labelName, line);

	line->hasLabel = True;
	strcpy(line->labelName, labelName);

	return (char*)strtok_next(labelToken, labelDelimiters).start;
}

boolean containsLabel(token firstToken)
{
	return isDelimiter(*(firstToken.end + 1), labelDelimiters);
}

UserCommandResult parseNoArgsCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	UserCommandResult result = nullInstruction();
	UNUSED(data);

	if (line->firstArgumentLoc == NULL)
	{
		putCommandOpcode(&result, command->opcode);
		result.instructionSize = 1;
	}
	else
	{
		printf("line %d: invalid arguments after \"%s\". Expected '0' arguments.\n", line->lineNumber, command->name);
		line->hasError = True;
	}

	return result;
}

UserCommandResult parseEntryCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char referencedLabel[LABEL_MAX_LEN];
	UserCommandResult result = nullInstruction();
	const Symbol* existing;

	if (line->firstArgumentLoc != NULL)
	{
		strtok_begin_cp(line->firstArgumentLoc, space_chars, referencedLabel);
		existing = lhash_find(&data->entries, referencedLabel);
		/* new entry */
		if (existing == NULL)
		{
			if (validateLabelName_(data, line, referencedLabel))
				insertEntry(data, referencedLabel, line);
			else
				/* Invalid entry */
				line->hasError = True;
		}
		else if (data->inFirstRun)/* entry exists */
		{
			printf("At line %d, entry '%s' defined twice, previous definition at %d.", line->lineNumber, referencedLabel, existing->lineNumber);
			line->hasError = True;
		}

		if (line->secondArgumentLoc != NULL)
		{
			printf("At line %d, unexpected more then one argument '%s', entry instruction takes one argument.\n",
					line->lineNumber, referencedLabel);
			line->hasError = True;
		}
	}
	else
	{
		printf("line %d: invalid arguments after \"%s\". Expected '0' arguments.\n", line->lineNumber, command->name);
		line->hasError = True;
	}

	return result;
}

UserCommandResult parseExternCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	UNUSED(line);
	UNUSED(data);
	UNUSED(command);
	return nullInstruction();
}

UserCommandResult handleCommand(ProgramData* data, Line* line, const char* commandNameString)
{
	const UserCommand* command;

	command = ohash_find(&data->cmds, commandNameString);
	if (command != NULL)
		return command->handler(line, command, data);

	printf("line %d: No such command \"%s\"\n", line->lineNumber, commandNameString);
	line->hasError = True;

	return nullInstruction();
}

void interpolateParsedData(Line* line, token token)
{
	line->commandNameLoc = token.start;

	token = strtok_next(token, space_chars);
	line->firstArgumentLoc = token.start;

	token = strtok_next(token, space_chars);
	line->secondArgumentLoc = token.start;

	token = strtok_next(token, space_chars);
	line->thirdArgumentLoc = token.start;
}

UserCommandResult parseLine(ProgramData* data, Line* line)
{
	static char commandNameString[MAX_LINE_SIZE];
	char* restOfTheLine;
	UserCommandResult i = {0};

	token cmdNameTok = strtok_begin_cp(line->data, space_chars, commandNameString);

	if (hasLabel(commandNameString))
	{
		/* Using the firstToken and not line data because it is filtered from prefixed spaces */
		restOfTheLine = parseLabel(data, line, commandNameString);
		if (line->hasError)
			return i;

		/* We need the command name with a null terminator.
		 * This also returns the command's first argument ptr, so we place it in line */
		cmdNameTok = strtok_begin_cp(restOfTheLine, space_chars, commandNameString);
	}

	interpolateParsedData(line, cmdNameTok);
	return handleCommand(data, line, commandNameString);
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

void printInstruction(FILE* f, const ProgramData* data, UserCommandResult i)
{
	static char instruction32base[10];
	static char instructionCounter32base[10];

	if (i.instructionSize == 0)
		return;

	to_32basePadded(data->instruction_counter, instructionCounter32base, 3);
	to_32basePadded(i.instructionBits, instruction32base, 3);

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

int getLine(Line* line, FILE* inputFile)
{
	Line newLine = { 0 };
	newLine.lineNumber = line->lineNumber + 1;
	*line = newLine;
	return fgets(line->data, MAX_LINE_SIZE, inputFile) == line->data && !feof(inputFile);
}

void firstRun(FILE* inputFile,
			  ProgramData* data)
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
	data->inFirstRun = False;
}

void secondRun(ProgramData* data,
			   FILE* inputFile,
			   FILE* objectOutFile)
{
	UserCommandResult parsed;
	Symbol* lineLabel;
	Line line = { 0 };

	while (getLine(&line, inputFile))
	{
		if (shouldParse(&line))
		{
			parsed = parseLine(data, &line);
			printInstruction(objectOutFile, data, parsed);

			if (line.hasLabel)
			{
				lineLabel = (Symbol*)lhash_find(&data->symbols, line.labelName);
				lineLabel->referencedMemAddr += data->instruction_counter;
			}

			data->instruction_counter += parsed.instructionSize;
		}
	}
}

boolean commandNameCmp(const KeyType key, const ObjectType object)
{
	return strcmp((const char*)key, ((const UserCommand*)object)->name) == 0;
}

void initCommandsTable(ProgramData* data)
{
	static UserCommand rtsCommand = { RtsOpcode, parseNoArgsCommand, "rts" };
	static UserCommand stopCommand = { StopOpcode, parseNoArgsCommand,  "stop" };
	static UserCommand entryCommand = { NoOpcode, parseEntryCommand, ".entry" };
	static UserCommand externCommand = { NoOpcode, parseExternCommand, ".extern" };

	ObjectMetadata meta = { prehashCommand, commandNameCmp, sizeof(UserCommand) };

	data->cmds = newOHashTable(meta);
	ohash_insert(&data->cmds, stopCommand.name, &stopCommand);
	ohash_insert(&data->cmds, rtsCommand.name, &rtsCommand);
	ohash_insert(&data->cmds, entryCommand.name, &entryCommand);
	ohash_insert(&data->cmds, externCommand.name, &externCommand);
}

boolean symbolNameCmp(const KeyType key, const ObjectType object)
{
	return strcmp((const char*)key, ((const Symbol*)object)->name) == 0;
}

void initSymbolsTable(ProgramData* data)
{
	ObjectMetadata meta = { stupidhash, symbolNameCmp, sizeof(Symbol) };

	data->symbols = newLHashTable(meta, 1);
}

boolean registerNameCmp(const KeyType key, const ObjectType object)
{
	return strcmp((const char*)key, ((const Register*)object)->name) == 0;
}

#define NUM_OF_GENERAL_REGISTERS 1

void insertGeneralRegisters(ProgramData* data)
{
	static Register generals[NUM_OF_GENERAL_REGISTERS];
	int i;

	for (i = 0; i < NUM_OF_GENERAL_REGISTERS; ++i)
	{
		generals[i].name[0] = 'r';
		generals[i].name[1] = i + '1';
		generals[i].name[2] = '\0';

		lhash_insert(&data->registers, generals[i].name, &generals[i]);
	}
}

void insertPCRegister(ProgramData* data)
{
	static Register r;
	strcpy(r.name, "PC");
	lhash_insert(&data->registers, r.name, &r);
}

void insertSPRegister(ProgramData* data)
{
	static Register r;
	strcpy(r.name, "SP");
	lhash_insert(&data->registers, r.name, &r);
}

void insertPSWRegister(ProgramData* data)
{
	static Register r;
	strcpy(r.name, "PSW");
	lhash_insert(&data->registers, r.name, &r);
}

void initRegisters(ProgramData* data)
{
	ObjectMetadata meta = { stupidhash, registerNameCmp, sizeof(Register) };

	data->registers = newLHashTable(meta, 1);

	insertGeneralRegisters(data);
	insertPSWRegister(data);
	insertPCRegister(data);
	insertSPRegister(data);

}

void initEntries(ProgramData* data)
{
	ObjectMetadata meta = { stupidhash, registerNameCmp, sizeof(Register) };

	data->entries = newLHashTable(meta, 1);
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
		/*else if (referencedLabel->isExternal)
		{
			printf("At line %d, expected label '%s' cannot be an external reference from line %d.\n",
					entry->lineNumber, entry->name, referencedLabel->lineNumber);
			data->numberOfErrors++;
		}*/
	}
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
	ProgramData data = {0};

	FILE* status = freopen("log", "w", stdout);
	if (status == NULL)
		puts("Bad redirect");

	inputFile = getInputfile(argv[FILE_NAME_ARG_INDEX]);
	if (inputFile == NULL)
		puts("BAD FILE");

	validateNumberOfArguments(argc, argv);

	initCommandsTable(&data);
	initSymbolsTable(&data);
	initRegisters(&data);
	initEntries(&data);

	firstRun(inputFile, &data);

	fclose(inputFile);

	validateEntries(&data);

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
