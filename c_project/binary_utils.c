#include "binary_utils.h"

#include <stdlib.h>
#define THIRTEEN_BITS_MASK 0x1FFF
#define SIX_BITS_MASK 0x3F
#define FOUR_BITS_MASK 0xF
#define TWO_BITS_MASK 0x3

#define RND_TYPE_LOCATION 12
#define RND_TYPE_MASK (TWO_BITS_MASK << RND_TYPE_LOCATION)

#define GROUP_LOCATION 10
#define GROUP_MASK (TWO_BITS_MASK << GROUP_LOCATION)

#define OPCODE_LOCATION 6
#define OPCODE_MASK (FOUR_BITS_MASK << OPCODE_LOCATION)

#define SRC_METHOD_LOCATION 4
#define SRC_METHOD_MASK (TWO_BITS_MASK << SRC_METHOD_LOCATION)

#define DEST_METHOD_LOCATION 2
#define DEST_METHOD_MASK (TWO_BITS_MASK << DEST_METHOD_LOCATION)

#define ARE_LOCATION 0
#define ARE_MASK (TWO_BITS_MASK << ARE_LOCATION)

#define SET_BITS_ON(DATA, PART, BITS) \
	/* Set the relevant part to zero */\
	{\
		DATA &= (~PART##_MASK);\
	/* Right side of the expression is all zeros other then the bits we want to set
	 * We then do XOR on the data to change the bits */\
		DATA ^= ((BITS << PART##_LOCATION) & PART##_MASK);\
	}

void putCommandMetadata(UserCommandResult* dest, const UserCommand* cmd)
{
	SET_BITS_ON(dest->instructionBytes.bits, OPCODE, cmd->opcode);
	SET_BITS_ON(dest->instructionBytes.bits, GROUP, cmd->group);
}

void putCommandAddrMethod(UserCommandResult* dest, AddressMethod method, ArgType argType)
{
	if (argType == DestArg)
	{
		SET_BITS_ON(dest->instructionBytes.bits, DEST_METHOD, method);
	}
	else
		SET_BITS_ON(dest->instructionBytes.bits, SRC_METHOD, method);
}

#define DEST_REGISTER_LOCATION 2
#define DEST_REGISTER_MASK (SIX_BITS_MASK << DEST_REGISTER_LOCATION)

#define SRC_REGISTER_LOCATION 8
#define SRC_REGISTER_MASK (SIX_BITS_MASK << SRC_REGISTER_LOCATION)

void putDestRegister(UserCommandResult* dest, Register* reg)
{
	putCommandAddrMethod(dest, RegisterNameAddressing, DestArg);
	SET_BITS_ON(dest->destArgBytes.bits, DEST_REGISTER, reg->number);
}

void putSrcRegister(UserCommandResult* dest, Register* reg)
{
	putCommandAddrMethod(dest, RegisterNameAddressing, SourceArg);
	SET_BITS_ON(dest->sourceArgBytes.bits, SRC_REGISTER, reg->number);
}

boolean hasRandomAddressing(UserCommandResult* result)
{
	return ((RND_TYPE_MASK) & result->instructionBytes.bits) != 0;
}

boolean isRegister(UserCommandResult* result, ArgType argType)
{
	if (argType == SourceArg)
		return ((SRC_METHOD_MASK) & result->instructionBytes.bits) == (RegisterNameAddressing << SRC_METHOD_LOCATION);

	return ((DEST_METHOD_MASK) & result->instructionBytes.bits) == (RegisterNameAddressing << DEST_METHOD_LOCATION);
}

#define DIRECT_ADDRESSING_LOCATION 2
#define DIRECT_ADDRESSING_MASK (THIRTEEN_BITS_MASK << DIRECT_ADDRESSING_LOCATION)

typedef enum
{
	AbsoluteAddressCoding = 0,
	ExternalAddressCoding = 1,
	RelocatableAddressCoding = 2
} AddressCodingType;

void putDirectAddressLabel(UserCommandResult* dest, Symbol* label, ArgType argType)
{
	CustomByte* byteToChange = argType == DestArg ? &dest->destArgBytes : &dest->sourceArgBytes;

	putCommandAddrMethod(dest, DirectAddressing, argType);
	if (label->isExternal)
		SET_BITS_ON(byteToChange->bits, ARE, ExternalAddressCoding)
	else
	{
		SET_BITS_ON(byteToChange->bits, DIRECT_ADDRESSING, label->referencedMemAddr);
		SET_BITS_ON(byteToChange->bits, ARE, RelocatableAddressCoding);
	}
}

#define INSTANT_ADDRESSING_LOCATION 2
#define INSTANT_ADDRESSING_MASK (THIRTEEN_BITS_MASK << INSTANT_ADDRESSING_LOCATION)

void putInstantArgument(UserCommandResult* dest, int argument, ArgType argType)
{
	CustomByte* argumentResult;

	putCommandAddrMethod(dest, InstantAddressing, argType);
	if (argType == DestArg)
		argumentResult = &dest->destArgBytes;
	else
		argumentResult = &dest->sourceArgBytes;
	SET_BITS_ON(argumentResult->bits, INSTANT_ADDRESSING, argument);
}

void putRndBits(UserCommandResult* dest, RandomType rnd)
{
	SET_BITS_ON(dest->instructionBytes.bits, RND_TYPE, rnd);
}

char to_32bit(unsigned int value)
{
	if (value < 10)
		return '0' + value;
	return 'A' + value - 10;
}

void reverseString(int end, char* buffer)
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

	reverseString(bitNum, buffer);
}

void to_32base(unsigned int value, char* buffer)
{
	to_32basePadded(value, buffer, 0);
}

void printByte(FILE* f, unsigned int address, CustomByte b)
{
	static char instructionCounter32base[10];
	static char instruction32base[10];

	to_32basePadded(address, instructionCounter32base, 3);
	to_32basePadded(b.bits, instruction32base, 3);
	fprintf(f, "%s %s\n", instructionCounter32base, instruction32base);
}

void printInstruction(FILE* f, const ProgramData* data, UserCommandResult i)
{
	int instructionCounter = data->instruction_counter;
	if (i.instructionSize > 0)
		printByte(f, instructionCounter, i.instructionBytes);

	if (i.instructionSize > 2)
	{
		printByte(f, instructionCounter + 1, i.sourceArgBytes);
		instructionCounter++;
	}

	if (i.instructionSize > 1)
		printByte(f, instructionCounter + 1, i.destArgBytes);

}

void printCounterHeader(FILE* f, const ProgramData* data)
{
	static char instructionCounter32base[10];
	static char dataCounter2base[10];

	to_32base(data->instruction_counter, instructionCounter32base);
	to_32base(data->data_counter, dataCounter2base);

	fprintf(f, "%s %s\n", instructionCounter32base, dataCounter2base);
}

CustomByte getDataStorageNumber(int number)
{
	CustomByte byte = { 0 };

	byte.bits = number;

	return byte;
}

int decimal_data_upper_bound()
{
	CustomByte b;

	/* ~0 on the bitfield results in the exact number of bits we want */
	b.bits = ~0;

	/* Shifting right because we want the number unsigned */
	return b.bits >> 1;
}

int decimal_data_lower_bound()
{
	return - (decimal_data_upper_bound() + 1);
}
