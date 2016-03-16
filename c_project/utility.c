/*
 * utility.c
 *
 *  Created on: Mar 10, 2016
 *      Author: dorav
 */

#include "utility.h"

#include <ctype.h>
#include <string.h>
#include "stdlib.h"

#include "types.h"
boolean isDelimiter(char c, const char* delimiters)
{
	while (*delimiters != '\0')
	{
		if (c == *delimiters)
			return True;

		delimiters++;
	}

	return False;

}

void advance_token_to_next_delimiter(token* tok, const char* delimiters)
{
	if (tok->start == NULL)
		return;

	/* Loop over all chars in string, search for delimiter */
	while (*tok->end != '\0')
	{
		/* Once delimiter is found:
		 * return a token from start to one char before delimiter*/
		if (isDelimiter(*(tok->end + 1), delimiters))
			return;
		tok->end++;
	}

	/* last token will be here, */
}

void skip_sticky_delimiters(token* t, const char* delimiters)
{
	/* Skipping all prefix delimiters */
	while(*t->start != '\0' && isDelimiter(*t->start, delimiters))
		t->start++;

	if (*t->start == '\0')
		t->start = NULL;

	t->end = t->start;
}

token token_start(const char* string, const char* delimiters)
{
	token t;
	t.start = string;
	t.end = string;

	skip_sticky_delimiters(&t, delimiters);

	advance_token_to_next_delimiter(&t, delimiters);

	return t;
}

token strtok_begin(const char* newstring, const char* delimiters)
{
	token tok = token_start(newstring, delimiters);

	return tok;
}

void copy_token(const token* t, char* buffer)
{
	int len = t->end - t->start + 1;
	if (t->start == NULL)
	{
		buffer[0] = '\0';
		return;
	}
	memmove(buffer, t->start, len);
	buffer[len] = '\0';
}

token strtok_begin_cp(const char* newstring, const char* delimiters, char* buffer)
{
	token t = strtok_begin(newstring, delimiters);
	copy_token(&t, buffer);
	return t;
}

token end_token()
{
	token tok = { NULL, NULL };
	return tok;
}

token strtok_next(token prev, const char* delimiters)
{
	if (prev.start == NULL)
		return prev;

	if (*prev.end == '\0')
		return end_token();

	return token_start(prev.end + 1, delimiters);
}

token strtok_next_cp(token prev, const char* delimiters, char* buffer)
{
	token tok = strtok_next(prev, delimiters);
	copy_token(&tok, buffer);
	return tok;
}

char* trim(char* input, const char* prefixes)
{
	return (char*)trim_c(input, prefixes);
}

const char* trim_c(const char* input, const char* prefixes)
{
	while (*input != '\0' && isDelimiter(*input, prefixes))
		input++;

	return input;
}

boolean hasLineEnd(const char* line)
{
	while (*line != '\0')
		if ((*line++) == '\n')
			return True;
	return False;
}

boolean isSpaces(const char* line)
{
	while (*line)
		if (!isspace(*line))
			return False;
		else
			++line;

	return True;
}

UserCommandResult nullInstruction()
{
	UserCommandResult i = { 0 };
	return i;
}

/* For both linux and windows */
#define DIRECTORY_DELIMITERS "/\\"

char* getFileNameWithExt(const char* inputFileName, const char* ext)
{
	int outputFileNameLen = strlen(inputFileName) + strlen(ext) + 1;
	char* outputFileName = calloc(1, outputFileNameLen);

	token t = strtok_begin_cp(inputFileName, DIRECTORY_DELIMITERS, outputFileName);
	while (strtok_next(t, DIRECTORY_DELIMITERS).start != NULL)
		t = strtok_next_cp(t, DIRECTORY_DELIMITERS, outputFileName);

	strcat(outputFileName, ext);

	return outputFileName;
}

FILE* getOutFile(const char* inputFileName, const char* extention)
{
	char* outputFileName = getFileNameWithExt(inputFileName, extention);
	FILE* file;

	file = fopen(outputFileName, "w");

	if (file == NULL)
		puts("BAD OUTPUT FILE");

	free (outputFileName);

	return file;
}
