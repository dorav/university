/*
 * utility.h
 *
 *  Created on: Mar 8, 2016
 *      Author: dorav
 */

#ifndef UTILITY_H_
#define UTILITY_H_

typedef int boolean;
#define Equal 1
#define NotEqual 0
#define True 1
#define False 0

typedef struct
{
	const char* start;
	const char* end;
} token;

token strtok_begin(const char* newstring, const char* delimiters);
token strtok_begin_cp(const char* newstring, const char* delimiters, char* buffer);
token strtok_next(token prev, const char* delimiters);
token strtok_next_cp(token prev, const char* delimiters, char* buffer);
void copy_token(const token* t, char* buffer);
boolean isDelimiter(char c, const char* delimiters);

char* trim(char* input, const char* prefixes);
const char* trim_c(const char* input, const char* prefixes);

#endif /* UTILITY_H_ */
