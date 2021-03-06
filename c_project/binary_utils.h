/*
 * binary_utils.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef BINARY_UTILS_H_
#define BINARY_UTILS_H_
#include "types.h"

int decimal_data_upper_bound();
int decimal_data_lower_bound();

#define INSTANT_ADDRESSING_UPPER_BOUND 4095
#define INSTANT_ADDRESSING_LOWER_BOUND -4096

typedef enum
{
	SourceArg,
	DestArg
} ArgType;

void putCommandMetadata(UserCommandResult* dest, const UserCommand* cmd);
void putCommandAddrMethod(UserCommandResult* dest, AddressMethod method, ArgType argType);
void putDestRegister(UserCommandResult* dest, Register* reg);
void putSrcRegister(UserCommandResult* dest, Register* reg);
void putDirectAddressLabel(UserCommandResult* dest, Symbol* label, ArgType argType);
void putInstantArgument(UserCommandResult* dest, int argument, ArgType argType);
void putRndBits(UserCommandResult* dest, RandomType rnd);

boolean hasRandomAddressing(UserCommandResult* result);
boolean isRegister(UserCommandResult* result, ArgType argType);

CustomByte getDataStorageNumber(int number);

void printByte(FILE* f, unsigned int address, CustomByte b);
void printInstruction(FILE* f, const ProgramData* data, UserCommandResult i);
void printCounterHeader(FILE* f, const ProgramData* data);

char to_32bit(unsigned int value);
void reverseString(int end, char* buffer);
int pad_to(int len, char* buffer, int padNum);
void to_32basePadded(unsigned int value, char* buffer, int padSize);
void to_32base(unsigned int value, char* buffer);

#endif /* BINARY_UTILS_H_ */
