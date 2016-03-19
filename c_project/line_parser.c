/*
 * line_parser.c
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#include "line_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "program_ds_manipulators.h"
#include "utility.h"

const char space_chars[7] = {9, 10, 11, 12, 13, 32 , '\0'};
const char labelDelimiters[] = ":";

boolean containsLabel(token firstToken)
{
	return isDelimiter(*(firstToken.end + 1), labelDelimiters);
}

boolean validateLabelName_(ProgramData* data, Line* line, const char* labelName)
{
	int len = 0;
	/* first char must be a letter */
	if (!isalpha(*labelName))
	{
		printf("At line %d, label name \"%s\", cannot start with a non alphabetic character.\n", line->lineNumber, labelName);
		return False;
	}

	++len;

	/* rest of the label name must contain only alpha-numerical characters */
	while (*(labelName + len) != '\0')
	{
		if (!isalnum(*(labelName + len)))
		{
			printf("At line %d, label name \"%s\", cannot contain alpha-numeric character.\n", line->lineNumber, labelName);
			return False;
		}

		++len;
	}

	if (len > MAX_LABEL_LENGTH)
	{
		printf("At line %d, label name \"%s\", cannot be more then %d chars long.\n", line->lineNumber, labelName, MAX_LABEL_LENGTH);
		return False;
	}

	if (ohash_find(&data->cmds, labelName) != NULL)
	{
		printf("At line %d, label name \"%s\", is an instruction keyword, choose a different name.\n", line->lineNumber, labelName);
		return False;
	}

	if (lhash_find(&data->registers, labelName) != NULL)
	{
		printf("At line %d, label name \"%s\", is a register name, choose a different name.\n", line->lineNumber, labelName);
		return False;
	}

	return True;
}

const char* validateLabelName(ProgramData* data, const char* name,	Line* line)
{
	static char labelName[MAX_LINE_SIZE];
	strtok_begin_cp(name, labelDelimiters, labelName);
	if (validateLabelName_(data, line, labelName) == False)
		return NULL;

	return labelName;
}

/* Validates and register the label in the symbol table.
 *
 * Returns a pointer to the next token in the line
 */
char* parseLabel(ProgramData* data, Line* line, char* firstToken)
{
	token labelToken = strtok_begin(line->data, labelDelimiters);
	const char* labelDelimiter = labelToken.end + 1; /* Not '\0' because label exists */
	const char* restOfTheLine = labelDelimiter + 1;
	const char* labelName;

	/* If line starts with spaces and followed by label delimiter */
	if (isDelimiter(*firstToken, labelDelimiters))
	{
		printf("At line %d, missing label name before ':'\n", line->lineNumber);
		line->hasError = True;
		return NULL;
	}

	if ((labelName = validateLabelName(data, firstToken, line)) == NULL)
	{
		line->hasError = True;
		return NULL;
	}

	/* Checking if the rest of the line is not empty or only filled with white spaces */
	if (*restOfTheLine == '\0' || isSpaces(restOfTheLine))
	{
		printf("At line %d, after label '%s', line must contain more content\n", line->lineNumber, labelName);
		line->hasError = True;
		return NULL;
	}
	else if (isDelimiter(*restOfTheLine, labelDelimiters))
	{
		printf("At line %d, after label '%s', too many label delimiters ':'\n", line->lineNumber, labelName);
		line->hasError = True;
		return NULL;
	}

	if (data->inFirstRun)
		registerSymbol(data, labelName, line);

	line->label = lhash_find(&data->symbols, labelName);
	line->hasLabel = True;

	return (char*)strtok_next(labelToken, labelDelimiters).start;
}

void interpolateParsedData(Line* line, token token)
{
	line->commandNameLoc = token.start;

	token = strtok_next(token, space_chars);
	line->firstArgumentLoc = token.start;

	token = strtok_begin(line->firstArgumentLoc, ",");
	token = strtok_next(token, ",");
	line->secondArgumentLoc = strtok_begin(token.start, space_chars).start; /* trimming spaces */

	token = strtok_next(token, ",");
	line->thirdArgumentLoc = token.start;
}

UserCommandResult handleCommand(ProgramData* data, Line* line, const char* commandNameString)
{
	const UserCommand* command;

	command = ohash_find(&data->cmds, commandNameString);
	if (command != NULL)
		return command->handler(line, command, data);

	printf("line %d: No such command \"%s\"\n", line->lineNumber, commandNameString);
	line->hasError = True;

	return nullInstruction();
}

UserCommandResult parseLine(ProgramData* data, Line* line)
{
	static char commandNameString[MAX_LINE_SIZE];
	char* restOfTheLine;
	UserCommandResult i = {0};

	token cmdNameTok = strtok_begin_cp(line->data, space_chars, commandNameString);

	if (hasLabel(commandNameString))
	{
		/* Using the firstToken and not line data because it is filtered from prefixed spaces */
		restOfTheLine = parseLabel(data, line, commandNameString);
		if (line->hasError)
			return i;

		/* We need the command name with a null terminator.
		 * This also returns the command's first argument ptr, so we place it in line */
		cmdNameTok = strtok_begin_cp(restOfTheLine, space_chars, commandNameString);

		line->label->isDataLabel = False;
	}

	interpolateParsedData(line, cmdNameTok);
	return handleCommand(data, line, commandNameString);
}

boolean hasLabel(const char* data)
{
	/* This can also be a comment or a string
	 * But comments are filtered before and strings also have a label. */
	while (*data != '\0')
	{
		if (isDelimiter(*data, labelDelimiters))
			return True;
		data++;
	}

	return False;
}
