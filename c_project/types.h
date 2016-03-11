/*
 * types.h
 *
 *  Created on: Feb 3, 2016
 *      Author: dorav
 */

#ifndef TYPES_H_
#define TYPES_H_

typedef enum
{
	RtsOpcode = 14,
	StopOpcode = 15
} CommandOpcode;

typedef struct
{
	unsigned int bits;
} CPUInstruction;

typedef struct
{
	CommandOpcode opcode;
	char name[5];
} UserCommand;

#define MAX_LINE_SIZE 100

typedef struct
{
	int lineNumber;
	char data[MAX_LINE_SIZE];
	int hasError;
	const char* commandNameLoc;
	const char* firstArgumentLoc;
	const char* secondArgumentLoc;
	const char* thirdArgumentLoc;
} Line;

typedef struct
{
	int isEmptyLine;
	int isComment;
	int isParsingRequired;
} LineType;


typedef struct
{
	char name[30];
	unsigned int lineNumber;
} Symbol;


#endif /* TYPES_H_ */
