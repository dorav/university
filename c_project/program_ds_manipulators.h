/*
 * program_ds_manipulators.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef PROGRAM_DS_MANIPULATORS_H_
#define PROGRAM_DS_MANIPULATORS_H_

#include "types.h"

void insertEntry(ProgramData* data, const char* labelName, Line* line);
void insertExtern(ProgramData* data, const char* labelName, Line* line);
void insertLabel(ProgramData* data, const char* labelName, Line* line);
void registerSymbol(ProgramData* data, const char* labelName, Line* line);

void initCommandsTable(ProgramData* data);
void initSymbolsTable(ProgramData* data);
void initRegisters(ProgramData* data);
void initEntries(ProgramData* data);


#endif /* PROGRAM_DS_MANIPULATORS_H_ */
