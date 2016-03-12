/*
 * types.h
 *
 *  Created on: Feb 3, 2016
 *      Author: dorav
 */

#ifndef TYPES_H_
#define TYPES_H_

#include "hash_table.h"

typedef enum
{
	NoOpcode = -1,
	RtsOpcode = 14,
	StopOpcode = 15
} CommandOpcode;

typedef struct
{
	unsigned int instructionBits;
	unsigned int instructionSize;
	unsigned int dataSize;
	char* data;
} UserCommandResult;

UserCommandResult nullInstruction();

#define MAX_LINE_SIZE 100
#define LABEL_MAX_LEN 30

typedef struct
{
	int lineNumber;
	int hasError;
	boolean hasLabel;
	char data[MAX_LINE_SIZE];
	char labelName[LABEL_MAX_LEN];
	const char* commandNameLoc;
	const char* firstArgumentLoc;
	const char* secondArgumentLoc;
	const char* thirdArgumentLoc;
} Line;

typedef struct
{
	boolean inFirstRun;
	unsigned int instruction_counter;
	unsigned int data_counter;
	unsigned int numberOfErrors;
	OHashTable cmds;
	LHashTable symbols;
	LHashTable registers;
	LHashTable entries;
} ProgramData;

struct UserCommand;

typedef UserCommandResult (*CommandHandler)(Line*, const struct UserCommand*, ProgramData* data);

struct UserCommand
{
	CommandOpcode opcode;
	CommandHandler handler;
	char name[10];
};

typedef struct UserCommand UserCommand;

typedef struct
{
	char name[LABEL_MAX_LEN];
	unsigned int lineNumber;
	unsigned int referencedMemAddr;
	boolean isExternal;
} Symbol;

typedef struct
{
	char name[10];
} Register;

#endif /* TYPES_H_ */
