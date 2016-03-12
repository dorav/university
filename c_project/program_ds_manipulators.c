/*
 * program_ds_manipulators.c
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */
#include "program_ds_manipulators.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"

#include "commands.h"

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

unsigned int stupidhash(ObjectType command)
{
	UNUSED(command);
	return 0;
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

boolean commandNameCmp(const KeyType key, const ObjectType object)
{
	return strcmp((const char*)key, ((const UserCommand*)object)->name) == 0;
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

#define NUM_OF_GENERAL_REGISTERS 8

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
