/*
 * commands.c
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#include "commands.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "stdlib.h"

#include "assembler_specific_utilities.h"
#include "binary_utils.h"
#include "program_ds_manipulators.h"
#include "line_parser.h"

const char instantAddressingIndicator[] = "#";

void validateNoExcessCharsAfterArg(const token* argToken, Line* line)
{
	if (!isSpaces(argToken->end + 1))
	{
		printf("At line %d, Found non-space characters after last expected argument.\n",
				line->lineNumber);
		line->hasError = True;
	}
}

Register* referencedRegister(ProgramData* data, const char* arg)
{
	return lhash_find(&data->registers, arg);
}

boolean isRandomAddressing(const char* arg)
{
	return arg[0] == '*';
}

boolean isInstantAddressing(const char* arg)
{
	return arg[0] == instantAddressingIndicator[0];
}

#define EXT_FILE_NAME ".ext"

void initExtFile(ProgramData* data)
{
	FILE* f = getOutFile(data->inputFileName, EXT_FILE_NAME);

	if (f == NULL)
	{
		data->numberOfErrors++;
		return;
	}

	data->externalReferencesFile = f;
}

void writeExternalReferenceToFile(ProgramData* data, Symbol* label, unsigned int address)
{
	static char addr32base[10];

	if (data->externalReferencesFile == NULL)
		initExtFile(data);

	if (data->externalReferencesFile == NULL)
	{
		data->numberOfErrors++;
		return;
	}

	to_32base(address, addr32base);
	fprintf(data->externalReferencesFile, "%s %s\n", label->name, addr32base);
}

void handleDirectAddressing(const char* argument, ProgramData* data, Line* line, ArgType argType, UserCommandResult* result)
{
	Symbol* label;

	/* This will only happen on the first run */
	if ((label = lhash_find(&data->symbols, argument)) == NULL)
		insertUnresolvedLabel(data, argument, line);
	else
	{
		putDirectAddressLabel(result, label, argType);
		if (label->isExternal && data->inFirstRun == False)
			writeExternalReferenceToFile(data, label, data->instruction_counter + 1);
	}
}

boolean isInRange_InstantAddressing(int number)
{
	return number >= INSTANT_ADDRESSING_LOWER_BOUND && number <= INSTANT_ADDRESSING_UPPER_BOUND;
}

int extractNumberArg(const char* argument, Line* line)
{
	int number;
	boolean isNumber;
	argument = strtok_begin(argument, instantAddressingIndicator).start;
	isNumber = sscanf(argument, "%d", &number);
	if (isNumber == False || isInRange_InstantAddressing(number) == False)
	{
		printf("At line %d, the argument \"%s\" is not a valid decimal number in the range [%d, %d].\n",
				line->lineNumber, argument, INSTANT_ADDRESSING_LOWER_BOUND,
				INSTANT_ADDRESSING_UPPER_BOUND);
		line->hasError = True;
	}

	return number;
}

void handleInstantAddressing(const char* argument, Line* line, ArgType argType, UserCommandResult* result)
{
	int number = extractNumberArg(argument, line);

	if (!line->hasError)
		putInstantArgument(result, number, argType);
}

#ifdef __DEBUG__

int rand_range(int min_n, int max_n)
{
	UNUSED(min_n);
	UNUSED(max_n);
	/* Best random ever */
    return 3;
}

#else

int rand_range(int min_n, int max_n)
{
    return rand() % (max_n - min_n + 1) + min_n;
}

#endif

Symbol* getRandomSymbol(ProgramData* data)
{
	int randomNumber = rand_range(0, data->symbols.numberOfUsed - 1);
	lhash_iter i;
	Symbol* current;
	Symbol* lastGood = NULL;

	/* Definitly not uniform distribution.. but good enough */
	for (i = lhash_begin(&data->symbols); i.isValid && (randomNumber > 0 || lastGood == NULL) ; lhash_set_next(&i))
	{
		current = (Symbol*) i.current->data;
		if (current->isExternal == False)
		{
			lastGood = current;
			randomNumber--;
		}
	}

	return lastGood;
}

int* newInt(int num)
{
	int* ret = malloc(sizeof(int));

	if (ret == NULL)
	{
		printf("Internal error while allocating memory, can't allocate for unresolved random label.\n");
		exit(AllocationFailure);
	}

	*ret = num;
	return ret;
}

const char* handleRandomAddressing(ProgramData* data, const char* argument, UserCommandResult* result, Line* line)
{
	static char randomedArgument[10];
	Symbol* label;
	int temp;

	if (strcmp("*", argument) == 0)
	{
		temp = rand_range(0, NUM_OF_GENERAL_REGISTERS - 1);
		sprintf(randomedArgument, "r%d", temp);

		putRndBits(result, RandomRegister);
		return randomedArgument;
	}
	else if (strcmp("**", argument) == 0)
	{
		temp = rand_range(INSTANT_ADDRESSING_LOWER_BOUND, INSTANT_ADDRESSING_UPPER_BOUND);
		sprintf(randomedArgument, "#%d", temp);

		putRndBits(result, RandomInstant);
		return randomedArgument;
	}
	else if (strcmp("***", argument) == 0)
	{
		if (data->inFirstRun && numOfLabels(data) == 0)
		{
			lhash_insert(&data->randomLabelLines, &line->lineNumber, newInt(line->lineNumber));
			return "PlaceHolderUnresolvedRandom";
		}

		label = getRandomSymbol(data);

		sprintf(randomedArgument, "%s", label->name);

		putRndBits(result, RandomLabel);
		return randomedArgument;
	}

	return argument;
}

boolean isUnresolvedRandomLabel(ProgramData* data, Line* line)
{
	return lhash_find(&data->randomLabelLines, &line->lineNumber) != NULL;
}

UserCommandResult handleSourceOperand(ProgramData* data, Line* line, const UserCommand* command)
{
	static char argumentWithSpaces[MAX_LINE_SIZE];
	static char argumentStorage[MAX_LINE_SIZE];
	/* This way i can define a static storage but change where it is pointing to */
	const char* argument = argumentStorage;
	UserCommandResult result = nullInstruction();
	Register* reg;

	strtok_begin_cp(line->firstArgumentLoc, ",", argumentWithSpaces);
	strtok_begin_cp(argumentWithSpaces, space_chars, argumentStorage);

	if (isRandomAddressing(argument))
		argument = handleRandomAddressing(data, argument, &result, line);

	if (isInstantAddressing(argument))
	{
		if (command->addressingTypes.sourceAddressingTypes.isInstantAllowed == False)
		{
			printf("At line %d, source instant addressing ('%s') is not allowed for instruction of type '%s'.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;
			return result;
		}

		handleInstantAddressing(argument, line, SourceArg, &result);
	}
	else if ((reg = referencedRegister(data, argument)) != NULL &&
			 command->addressingTypes.sourceAddressingTypes.isRegisterAllowed)
	{
		putSrcRegister(&result, reg);
	}
	else if (validateLabelName_(data, line, argument))
	{
		if (command->addressingTypes.sourceAddressingTypes.isDirectAllowed == False)
		{
			printf("At line %d, source direct addressing was used with label \"%s\", but it is not valid for \"%s\" instruction.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;

			return result;
		}

		/* If label name is pending from random access, the label name would be garbage */
		if (!isUnresolvedRandomLabel(data, line))
			handleDirectAddressing(argument, data, line, SourceArg, &result);
	}
	else
		line->hasError = True;

	return result;
}

UserCommandResult handleDestOperand(ProgramData* data, Line* line, const UserCommand* command, UserCommandResult result)
{
	static char argument[MAX_LINE_SIZE];
	Register* reg;

	strtok_begin_cp(line->secondArgumentLoc, space_chars, argument);

	if (isInstantAddressing(argument))
	{
		if (command->addressingTypes.destAddressingTypes.isInstantAllowed == False)
		{
			printf("At line %d, destination instant addressing ('%s') is not allowed for instruction of type '%s'.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;
			return result;
		}

		handleInstantAddressing(argument, line, DestArg, &result);
	}
	else if ((reg = referencedRegister(data, argument)) != NULL &&
			 command->addressingTypes.destAddressingTypes.isRegisterAllowed)
	{
		putDestRegister(&result, reg);
	}
	else if (validateLabelName_(data, line, argument))
	{
		if (command->addressingTypes.destAddressingTypes.isDirectAllowed == False)
		{
			printf("At line %d, destination direct addressing was used with label \"%s\", but it is not valid for \"%s\" instruction.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;

			return result;
		}

		handleDirectAddressing(argument, data, line, DestArg, &result);
	}

	return result;
}

void validateTwoArgCommandArgNum(const UserCommand* command, Line* line)
{
	if (line->firstArgumentLoc == NULL)
	{
		printf("At line %d, missing arguments for command '%s'.\n",
				line->lineNumber, command->name);
		line->hasError = True;
	}
	if (line->secondArgumentLoc == NULL)
	{
		printf("At line %d, missing second argument for command '%s'.\n",
				line->lineNumber, command->name);
		line->hasError = True;
	}
	if (line->thirdArgumentLoc != NULL)
	{
		printf("At line %d, Too many arguments given, expected 2.\n",
				line->lineNumber);
		line->hasError = True;
	}
}


UserCommandResult genericTwoArgCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	UserCommandResult result = nullInstruction();

	validateTwoArgCommandArgNum(command, line);
	if (line->hasError)
		return result;

	result = handleSourceOperand(data, line, command);
	if (line->hasError)
		return result;

	result = handleDestOperand(data, line, command, result);

	putCommandMetadata(&result, command);

	if (isRegister(&result, SourceArg) && isRegister(&result, DestArg))
	{
		result.destArgBytes.bits |= result.sourceArgBytes.bits;
		result.instructionSize = 2;
	}
	else
		result.instructionSize = 3;

	if (hasRandomAddressing(&result))
	{
		putCommandAddrMethod(&result, RandomAddressing, SourceArg);
	}

	return result;
}

UserCommandResult genericSingleArgCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char argument[MAX_LINE_SIZE];
	Register* reg;
	token argToken;
	UserCommandResult result = nullInstruction();

	if (line->firstArgumentLoc == NULL)
	{
		printf("At line %d, missing argument for command '%s'.\n", line->lineNumber, command->name);
		line->hasError = True;
		return result;
	}

	if (line->secondArgumentLoc != NULL)
	{
		printf("At line %d, too many arguments after command '%s', expected one.\n",
				line->lineNumber, command->name);
		line->hasError = True;
		return result;
	}

	argToken = strtok_begin_cp(line->firstArgumentLoc, space_chars, argument);

	validateNoExcessCharsAfterArg(&argToken, line);

	if (line->hasError)
		return result;

	if (isInstantAddressing(argument))
	{
		if (command->addressingTypes.destAddressingTypes.isInstantAllowed == False)
		{
			printf("At line %d, instant addressing ('%s') is not allowed for instruction of type '%s'.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;
			return result;
		}

		handleInstantAddressing(argument, line, DestArg, &result);
	}
	else if (isRandomAddressing(argument) &&
			 command->addressingTypes.destAddressingTypes.isRandomAllowed == False)
	{
		printf("At line %d, random addressing ('%s') is not allowed for instruction of type '%s'.\n",
				line->lineNumber, argument, command->name);
		line->hasError = True;
		return result;
	}
	else if ((reg = referencedRegister(data, argument)) != NULL &&
			 command->addressingTypes.destAddressingTypes.isRegisterAllowed)
	{
		putDestRegister(&result, reg);
	}
	else if (validateLabelName_(data, line, argument))
	{
		if (command->addressingTypes.destAddressingTypes.isDirectAllowed == False)
		{
			printf("At line %d, direct addressing was used with label \"%s\", but it is not valid for \"%s\" instruction.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;

			return result;
		}

		handleDirectAddressing(argument, data, line, DestArg, &result);
	}
	else /* Invalid label name, error printed by validateLabelName_ */
	{
		line->hasError = True;
		return result;
	}

	putCommandMetadata(&result, command);

	result.instructionSize = 2;

	return result;
}

boolean isNum(char c)
{
	return c >= '0' && c <= '9';
}

const char* nextNumber(const char* originalToken, Line* line)
{
	static char argument[MAX_LINE_SIZE];
	const char* currentChar = argument;
	strtok_begin_cp(originalToken, "\n", argument);

	++currentChar;

	while (isspace(currentChar[0]))
		currentChar++;

	if (currentChar[0] == '\0')
		return NULL;

	if (currentChar[0] == ',')
		currentChar++;
	else
	{
		printf("At line %d, expected to find comma after number.\n", line->lineNumber);
		line->hasError = True;
		return currentChar;
	}

	while (isspace(currentChar[0]))
		currentChar++;

	if (currentChar[0] != '-' && !isNum(currentChar[0]))
	{
		printf("At line %d, missing a valid number after comma \"%s\".\n",
						line->lineNumber, argument);
		line->hasError = True;
	}

	return currentChar;
}

const char* endOfNumber(const char* number)
{
	while (isNum(number[1]))
		++number;

	return number;
}

void addDataStorage(ProgramData* data, CustomByte toAdd)
{
	CustomByte* newStorageArray;
	int newSize;

	if (!data->inFirstRun)
	{
		if (data->dataStorageCapacity < data->data_counter + 1)
		{
			newSize = data->dataStorageCapacity * 2;
			if (newSize == 0) /* First time */
			{
				newSize = 2;
				newStorageArray = calloc(newSize, sizeof(CustomByte));
			}
			else
				newStorageArray = realloc(data->dataStorage, newSize * sizeof(CustomByte));
			if (newStorageArray == NULL)
			{
				printf("Internal error allocating memory, data storage is probably too big.\n");
				exit(AllocationFailure);
			}
			data->dataStorage = newStorageArray;
			data->dataStorageCapacity = newSize;
		}

		data->dataStorage[data->data_counter] = toAdd;
	}

	data->data_counter++;
}

void addStringStorage(ProgramData* data, token toAdd)
{
	static char stringToAdd[MAX_LINE_SIZE];
	int i = -1;
	CustomByte byte;

	copy_token(&toAdd, stringToAdd);

	do
	{
		++i;
		byte.bits = stringToAdd[i];
		addDataStorage(data, byte);
	}
	while (stringToAdd[i] != '\0');
}

const char* findEndOfString(const char* arg)
{
	/* arg[0] is '"' */
	while(arg[1] != '"')
	{
		if (arg[1] == '\0')
			return NULL;
		++arg;
	}

	return arg;
}

UserCommandResult parseStringCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char newLineDelimiter[] = { '\n', '\0' };
	static char argument[MAX_LINE_SIZE];
	const char* endOfString;
	token stringToAdd;

	UserCommandResult empty = nullInstruction();

	if (line->firstArgumentLoc == NULL)
	{
		printf("At line %d, missing argument for command '%s'.\n", line->lineNumber, command->name);
		line->hasError = True;
		return empty;
	}

	if (line->hasLabel && data->inFirstRun)
	{
		line->label->isDataLabel = True;
		line->label->referencedMemAddr = data->data_counter;
	}

	strtok_begin_cp(line->firstArgumentLoc, newLineDelimiter, argument);

	if (argument[0] != '"')
	{
		printf("At line %d, argument '%s', for \"%s\" command must be quoted (missing opening quote).\n",
				line->lineNumber, argument, command->name);
		line->hasError = True;
		return empty;
	}

	endOfString = findEndOfString(argument);

	if (endOfString == NULL)
	{
		printf("At line %d, argument '%s', for \"%s\" command must be quoted (missing closing quote).\n",
					line->lineNumber, argument, command->name);
				line->hasError = True;
		return empty;
	}

	/* having a '"' followed by another non-space char is invalid */
	if (!isSpaces(endOfString + 2))
	{
		printf("At line %d, while parsing '%s' command, did not expect more characters after string definition.\n",
				line->lineNumber, command->name);
		line->hasError = True;
		return empty;
	}

	stringToAdd.start = argument + 1;
	stringToAdd.end = endOfString;

	addStringStorage(data, stringToAdd);

	return empty;
}

UserCommandResult parseDataCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char argument[MAX_LINE_SIZE];
	token lastArg;
	int number = 0;

	UserCommandResult empty = nullInstruction();

	if (line->firstArgumentLoc == NULL)
	{
		printf("At line %d, missing argument for command '%s'.\n", line->lineNumber, command->name);
		line->hasError = True;
		return empty;
	}

	if (line->hasLabel && data->inFirstRun)
	{
		line->label->isDataLabel = True;
		line->label->referencedMemAddr = data->data_counter;
	}

	lastArg.start = line->firstArgumentLoc;
	lastArg.end = line->firstArgumentLoc;

	while (lastArg.start != NULL)
	{
		if (line->hasError)
			return empty;

		if (lastArg.start[0] == '\0')
		{
			printf("At line %d, missing a valid number after last comma.\n", line->lineNumber);
			line->hasError = True;
			return empty;
		}

		lastArg.end = endOfNumber(lastArg.start);

		copy_token(&lastArg, argument);

		if (sscanf(argument, "%d", &number) == 0 ||
			(number < decimal_data_lower_bound() || number > decimal_data_upper_bound()))
		{
			printf("At line %d, argument \"%s\" must be a number between [%d, %d].\n",
					line->lineNumber, argument, decimal_data_lower_bound(), decimal_data_upper_bound());
			line->hasError = True;
			return empty;
		}

		addDataStorage(data, getDataStorageNumber(number));

		lastArg.start = nextNumber(lastArg.end, line);
	}

	return empty;
}

UserCommandResult parseEntryCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char referencedLabel[MAX_LINE_SIZE];
	UserCommandResult result = nullInstruction();
	token argToken;
	const Symbol* existing;

	if (line->hasLabel)
	{
		printf("At line %d, meaningless label \"%s\" on \"%s\" instruction.\n",
				line->lineNumber, line->label->name, command->name);
		line->hasError = True;
		return result;
	}

	if (line->firstArgumentLoc != NULL)
	{
		argToken = strtok_begin_cp(line->firstArgumentLoc, space_chars, referencedLabel);

		validateNoExcessCharsAfterArg(&argToken, line);

		if (line->hasError)
			return result;

		existing = lhash_find(&data->entries, referencedLabel);
		/* new entry */
		if (existing == NULL)
		{
			if (validateLabelName_(data, line, referencedLabel))
				insertEntry(data, referencedLabel, line);
			else /* Invalid entry */
			{
				line->hasError = True;
				return result;
			}
		}
		/* entry exists in first run is invalid */
		else if (data->inFirstRun)
		{
			printf("At line %d, entry '%s' defined twice, previous definition at %d.\n", line->lineNumber, referencedLabel, existing->lineNumber);
			line->hasError = True;
		}

		if (line->secondArgumentLoc != NULL)
		{
			printf("At line %d, unexpected more then one argument '%s', entry instruction takes one argument.\n",
					line->lineNumber, referencedLabel);
			line->hasError = True;
		}
	}
	else
	{
		printf("At line %d, expected label name for entry command, none found.\n", line->lineNumber);
		line->hasError = True;
	}

	return result;
}

UserCommandResult parseExternCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char referencedLabel[MAX_LINE_SIZE];
	UserCommandResult result = nullInstruction();
	token labelToken;
	const Symbol* existing;

	if (line->hasLabel)
	{
		printf("At line %d, meaningless label \"%s\" on \"%s\" instruction.\n",
				line->lineNumber, line->label->name, command->name);
		line->hasError = True;
		return result;
	}

	if (line->firstArgumentLoc != NULL)
	{
		labelToken = strtok_begin_cp(line->firstArgumentLoc, space_chars, referencedLabel);

		validateNoExcessCharsAfterArg(&labelToken, line);

		if (line->hasError)
			return result;

		existing = lhash_find(&data->symbols, referencedLabel);
		/* new entry */
		if (existing == NULL)
		{
			if (validateLabelName_(data, line, referencedLabel))
				insertExtern(data, referencedLabel, line);
			else
			{
				/* Invalid entry */
				line->hasError = True;
				return result;
			}
		}
		/* entry exists in first run is invalid */
		else if (data->inFirstRun)
		{
			printf("At line %d, extern '%s' defined twice, previous definition at %d.\n", line->lineNumber, referencedLabel, existing->lineNumber);
			line->hasError = True;
		}

		if (line->secondArgumentLoc != NULL)
		{
			printf("At line %d, unexpected more then one argument '%s', extern instruction takes one argument.\n",
					line->lineNumber, referencedLabel);
			line->hasError = True;
		}
	}
	else
	{
		printf("At line %d, expected label name for extern command, none found.\n", line->lineNumber);
		line->hasError = True;
	}

	UNUSED(command);
	return result;
}

UserCommandResult parseNoArgsCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	UserCommandResult result = nullInstruction();
	UNUSED(data);

	if (line->firstArgumentLoc == NULL)
	{
		putCommandMetadata(&result, command);
		result.instructionSize = 1;
	}
	else
	{
		printf("line %d: invalid arguments after \"%s\". Expected '0' arguments.\n", line->lineNumber, command->name);
		line->hasError = True;
	}

	return result;
}
