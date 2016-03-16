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
#include "binary_utils.h"
#include "program_ds_manipulators.h"
#include "line_parser.h"

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
	return arg[0] == '#';
}

#define EXT_FILE_NAME ".ext"

void initExtFile(ProgramData* data)
{
	data->externalReferencesFile = getOutFile(data->inputFileName, EXT_FILE_NAME);
}

void writeExternalReferenceToFile(ProgramData* data, Symbol* label, unsigned int address)
{
	static char addr32base[10];

	if (data->externalReferencesFile == NULL)
		initExtFile(data);

	to_32base(address, addr32base);
	fprintf(data->externalReferencesFile, "%s %s\n", label->name, addr32base);
}

UserCommandResult genericSingleArgCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char argument[MAX_LINE_SIZE];
	Register* reg;
	Symbol* label;
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

	strtok_begin_cp(line->firstArgumentLoc, space_chars, argument);

	if (isInstantAddressing(argument) &&
		command->addressingTypes.destAddressingTypes.isInstantAllowed == False)
	{
		printf("At line %d, instant addressing ('%s') is not allowed for instruction of type '%s'.\n",
				line->lineNumber, argument, command->name);
		line->hasError = True;
		return result;
	}

	if (isRandomAddressing(argument) &&
		command->addressingTypes.destAddressingTypes.isRandomAllowed == False)
	{
		printf("At line %d, random addressing ('%s') is not allowed for instruction of type '%s'.\n",
				line->lineNumber, argument, command->name);
		line->hasError = True;
		return result;
	}

	if ((reg = referencedRegister(data, argument)) != NULL &&
		command->addressingTypes.destAddressingTypes.isRegisterAllowed)
	{
		putDestRegister(&result, reg);
	}
	else if (validateLabelName_(data, line, argument))
	{
		if (command->addressingTypes.destAddressingTypes.isDirectAllowed)
		{
			/* This will only happen on the first run */
			if ((label = lhash_find(&data->symbols, argument)) == NULL)
				insertUnresolvedLabel(data, argument, line);
			else
			{
				putDirectAddressLabel(&result, label);

				if (label->isExternal && data->inFirstRun == False)
					writeExternalReferenceToFile(data, label, data->instruction_counter + 1);
			}
		}
		else
		{
			printf("At line %d, direct addressing was used with label \"%s\", but it is not valid for \"%s\" instruction.\n",
					line->lineNumber, argument, command->name);
			line->hasError = True;
			return result;
		}
	}
	else /* Invalid label name, error printed by validateLabelName_ */
	{
		line->hasError = True;
		return result;
	}

	putCommandOpcode(&result, command);

	result.instructionSize = 2;

	return result;
}

UserCommandResult parseEntryCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char referencedLabel[MAX_LINE_SIZE];
	UserCommandResult result = nullInstruction();
	const Symbol* existing;

	if (line->hasLabel)
	{
		printf("At line %d, meaningless label \"%s\" on \"%s\" instruction.\n",
				line->lineNumber, line->labelName, command->name);
		line->hasError = True;
		return result;
	}

	if (line->firstArgumentLoc != NULL)
	{
		strtok_begin_cp(line->firstArgumentLoc, space_chars, referencedLabel);
		existing = lhash_find(&data->entries, referencedLabel);
		/* new entry */
		if (existing == NULL)
		{
			if (validateLabelName_(data, line, referencedLabel))
				insertEntry(data, referencedLabel, line);
			else
				/* Invalid entry */
				line->hasError = True;
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

	UNUSED(command);
	return result;
}

UserCommandResult parseExternCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char referencedLabel[MAX_LINE_SIZE];
	UserCommandResult result = nullInstruction();
	const Symbol* existing;

	if (line->hasLabel)
	{
		printf("At line %d, meaningless label \"%s\" on \"%s\" instruction.\n",
				line->lineNumber, line->labelName, command->name);
		line->hasError = True;
		return result;
	}

	if (line->firstArgumentLoc != NULL)
	{
		strtok_begin_cp(line->firstArgumentLoc, space_chars, referencedLabel);
		existing = lhash_find(&data->symbols, referencedLabel);
		/* new entry */
		if (existing == NULL)
		{
			if (validateLabelName_(data, line, referencedLabel))
				insertExtern(data, referencedLabel, line);
			else
				/* Invalid entry */
				line->hasError = True;
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
		putCommandOpcode(&result, command);
		result.instructionSize = 1;
	}
	else
	{
		printf("line %d: invalid arguments after \"%s\". Expected '0' arguments.\n", line->lineNumber, command->name);
		line->hasError = True;
	}

	return result;
}
