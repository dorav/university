#include "binary_utils.h"

#define THIRTEEN_BITS_MASK 0x1FFF
#define TWELVE_BITS_MASK 0xFFF
#define SIX_BITS_MASK 0x3F
#define FOUR_BITS_MASK 0xF
#define TWO_BITS_MASK 0x3

#define OPCODE_LOCATION 6
#define OPCODE_MASK (FOUR_BITS_MASK << OPCODE_LOCATION)

#define DEST_METHOD_LOCATION 2
#define DEST_METHOD_MASK (TWO_BITS_MASK << DEST_METHOD_LOCATION)

#define GROUP_LOCATION 10
#define GROUP_MASK (TWO_BITS_MASK << GROUP_LOCATION)

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

void putCommandDestAddrMethod(UserCommandResult* dest, AddressMethod method)
{
	SET_BITS_ON(dest->instructionBytes.bits, DEST_METHOD, method);
}

#define DEST_REGISTER_LOCATION 2
#define DEST_REGISTER_MASK (SIX_BITS_MASK << DEST_REGISTER_LOCATION)

void putDestRegister(UserCommandResult* dest, Register* reg)
{
	putCommandDestAddrMethod(dest, RegisterNameAddressing);
	SET_BITS_ON(dest->firstArgBytes.bits, DEST_REGISTER, reg->number);
}

#define DIRECT_ADDRESSING_LOCATION 2
#define DIRECT_ADDRESSING_MASK (TWELVE_BITS_MASK << DIRECT_ADDRESSING_LOCATION)

typedef enum
{
	AbsoluteAddressCoding = 0,
	ExternalAddressCoding = 1,
	RelocatableAddressCoding = 2
} AddressCodingType;

void putDirectAddressLabel(UserCommandResult* dest, Symbol* label)
{
	putCommandDestAddrMethod(dest, DirectAddressing);
	if (label->isExternal)
		SET_BITS_ON(dest->firstArgBytes.bits, ARE, ExternalAddressCoding)
	else
	{
		SET_BITS_ON(dest->firstArgBytes.bits, DIRECT_ADDRESSING, label->referencedMemAddr);
		SET_BITS_ON(dest->firstArgBytes.bits, ARE, RelocatableAddressCoding);
	}
}

#define INSTANT_ADDRESSING_LOCATION 2
#define INSTANT_ADDRESSING_MASK (THIRTEEN_BITS_MASK << INSTANT_ADDRESSING_LOCATION)

void putInstantArgument(UserCommandResult* dest, int argument)
{
	putCommandDestAddrMethod(dest, InstantAddressing);
	SET_BITS_ON(dest->firstArgBytes.bits, INSTANT_ADDRESSING, argument);
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

void printInstruction(FILE* f, const ProgramData* data, UserCommandResult i)
{
	static char instruction32base[10];
	static char instructionCounter32base[10];

	if (i.instructionSize == 0)
		return;

	to_32basePadded(data->instruction_counter, instructionCounter32base, 3);
	to_32basePadded(i.instructionBytes.bits, instruction32base, 3);

	fprintf(f, "%s %s\n", instructionCounter32base, instruction32base);

	if (i.instructionSize == 1)
		return;

	to_32basePadded(data->instruction_counter + 1, instructionCounter32base, 3);
	to_32basePadded(i.firstArgBytes.bits, instruction32base, 3);

	fprintf(f, "%s %s\n", instructionCounter32base, instruction32base);
}

void printCounterHeader(FILE* f, const ProgramData* data)
{
	static char instructionCounter32base[10];
	static char dataCounter2base[10];

	to_32base(data->instruction_counter, instructionCounter32base);
	to_32base(data->data_counter, dataCounter2base);

	fprintf(f, "%s %s\n", instructionCounter32base, dataCounter2base);
}
