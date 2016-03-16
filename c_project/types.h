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
	InvalidOpcode = -1,
	NotOpcode = 4,
	RedOpcode = 11,
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

typedef struct
{
	unsigned int bits : 15;
} CustomByte;

typedef struct
{
	CustomByte instructionBytes;
	CustomByte firstArgBytes;
	CustomByte secondArgBytes;
	unsigned int instructionSize;
	unsigned int dataSize;
	char* data;
} UserCommandResult;

UserCommandResult nullInstruction();

#define MAX_LINE_SIZE 101
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
	LHashTable unresolvedSymbols;
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

typedef struct
{
	char name[LABEL_MAX_LEN];
	unsigned int lineNumber;
	unsigned int referencedMemAddr;
	boolean isExternal;
} Symbol;

typedef struct
{
	int number;
	char name[10];
} Register;

#endif /* TYPES_H_ */
