/*
 * line_parser.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef LINE_PARSER_H_
#define LINE_PARSER_H_

#include "types.h"

#define MAX_LABEL_LENGTH 30

/* Created using isspace function */
extern const char space_chars[];
boolean isSpaces(const char* line);

extern const char labelDelimiters[];
boolean isDelimiter(char c, const char* delimiters);

const char* validateLabelName(ProgramData* data, const char* name,	Line* line);
boolean validateLabelName_(ProgramData* data, Line* line, const char* labelName);
boolean hasLabel(const char* data);

UserCommandResult parseLine(ProgramData* data, Line* line);

#endif /* LINE_PARSER_H_ */
