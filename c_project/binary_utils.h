/*
 * binary_utils.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef BINARY_UTILS_H_
#define BINARY_UTILS_H_
#include "types.h"

#define OPCODE_LOCATION 6
#define OPCODE_MASK (0xF << OPCODE_LOCATION)

/* This define is here for compile-time calculation
 * Compile time calculation is needed in c90 for {} initialization of structs */
#define OPCODE_TO_BINARY(opcode) ((opcode << OPCODE_LOCATION) & OPCODE_MASK)

void putCommandOpcode(UserCommandResult* dest, CommandOpcode value);
char to_32bit(unsigned int value);
void reverseString(int end, char* buffer);
int pad_to(int len, char* buffer, int padNum);
void to_32basePadded(unsigned int value, char* buffer, int padSize);
void to_32base(unsigned int value, char* buffer);

#endif /* BINARY_UTILS_H_ */
