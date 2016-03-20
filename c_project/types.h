/*
 * types.h
 *
 *  Created on: Feb 3, 2016
 *      Author: dorav
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdio.h>
#include "hash_table.h"

typedef enum
{
	InvalidOpcode = -1,

	/* Two args commands */
	MovOpcode = 0,
	CmpOpcode = 1,
	AddOpcode = 2,
	SubOpcode = 3,

	/* Single args commands */
	NotOpcode = 4,
	ClrOpcode = 5,
	IncOpcode = 7,
	DecOpcode = 8,
	JmpOpcode = 9,
	BneOpcode = 10,
	RedOpcode = 11,
	PrnOpcode = 12,
	JsrOpcode = 13,

	/* No args commands */
	RtsOpcode = 14,
	StopOpcode = 15
} CommandOpcode;

typedef enum
{
	InstantAddressing = 0,
	DirectAddressing = 1,
	RandomAddressing = 2,
	RegisterNameAddressing = 3
} AddressMethod;

typedef enum
{
	RandomRegister = 1,
	RandomInstant = 2,
	RandomLabel = 3
} RandomType;

typedef struct
{
	unsigned int bits : 15;
} CustomByte;

typedef struct
{
	CustomByte instructionBytes;
	CustomByte sourceArgBytes;
	CustomByte destArgBytes;
	unsigned int instructionSize;
} UserCommandResult;

UserCommandResult nullInstruction();

#define MAX_LINE_SIZE 101
#define LABEL_MAX_LEN 30

typedef struct
{
	char name[LABEL_MAX_LEN];
	unsigned int lineNumber;
	unsigned int referencedMemAddr;
	boolean isExternal;
	boolean isDataLabel;
} Symbol;

typedef struct
{
	int lineNumber;
	int hasError;
	boolean hasLabel;
	char data[MAX_LINE_SIZE];
	Symbol* label;
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
	CustomByte* dataStorage;
	unsigned int dataStorageCapacity;

	unsigned int numberOfErrors;
	OHashTable cmds;
	LHashTable symbols;
	LHashTable registers;
	LHashTable entries;
	LHashTable unresolvedSymbols;
	LHashTable randomLabelLines;

	const char* inputFileName;
	FILE* externalReferencesFile;
} ProgramData;

struct UserCommand;

typedef UserCommandResult (*CommandHandler)(Line*, const struct UserCommand*, ProgramData* data);

typedef enum
{
	SpecialGroup = 0,
	NoArgsGroup = 0,
	SingleArgGroup = 1,
	TwoArgsGroup = 2
} CommandGroup;

typedef struct
{
	boolean isInstantAllowed;
	boolean isDirectAllowed;
	boolean isRandomAllowed;
	boolean isRegisterAllowed;
} AllowedAddressingTypes;

typedef struct
{
	AllowedAddressingTypes sourceAddressingTypes;
	AllowedAddressingTypes destAddressingTypes;
} AllowedAddressings;

struct UserCommand
{
	CommandOpcode opcode;
	CommandHandler handler;
	CommandGroup group;

	AllowedAddressings addressingTypes;

	char name[10];
};

typedef struct UserCommand UserCommand;

#define NUM_OF_GENERAL_REGISTERS 8

typedef struct
{
	int number;
	char name[10];
} Register;

typedef enum
{
	AllocationFailure = 1
} ExitStatus;

#endif /* TYPES_H_ */
