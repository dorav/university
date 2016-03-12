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

UserCommandResult parseEntryCommand(Line* line, const UserCommand* command, ProgramData* data)
{
	static char referencedLabel[LABEL_MAX_LEN];
	UserCommandResult result = nullInstruction();
	const Symbol* existing;

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
			printf("At line %d, entry '%s' defined twice, previous definition at %d.", line->lineNumber, referencedLabel, existing->lineNumber);
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
	static char referencedLabel[LABEL_MAX_LEN];
	UserCommandResult result = nullInstruction();
	const Symbol* existing;

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
			printf("At line %d, extern '%s' defined twice, previous definition at %d.", line->lineNumber, referencedLabel, existing->lineNumber);
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
		putCommandOpcode(&result, command->opcode);
		result.instructionSize = 1;
	}
	else
	{
		printf("line %d: invalid arguments after \"%s\". Expected '0' arguments.\n", line->lineNumber, command->name);
		line->hasError = True;
	}

	return result;
}
