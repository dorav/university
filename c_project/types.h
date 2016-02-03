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


#endif /* TYPES_H_ */
