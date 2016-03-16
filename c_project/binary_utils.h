/*
 * binary_utils.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef BINARY_UTILS_H_
#define BINARY_UTILS_H_
#include "types.h"

void putCommandOpcode(UserCommandResult* dest, const UserCommand* cmd);
void putCommandDestAddrMethod(UserCommandResult* dest, AddressMethod method);
void putDestRegister(UserCommandResult* dest, Register* reg);
void putDirectAddressLabel(UserCommandResult* dest, Symbol* label);

void printInstruction(FILE* f, const ProgramData* data, UserCommandResult i);
void printCounterHeader(FILE* f, const ProgramData* data);

char to_32bit(unsigned int value);
void reverseString(int end, char* buffer);
int pad_to(int len, char* buffer, int padNum);
void to_32basePadded(unsigned int value, char* buffer, int padSize);
void to_32base(unsigned int value, char* buffer);

#endif /* BINARY_UTILS_H_ */
