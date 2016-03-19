/*
 * program_ds_manipulators.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef PROGRAM_DS_MANIPULATORS_H_
#define PROGRAM_DS_MANIPULATORS_H_

#include "types.h"

ProgramData initProgramData(const char* inputFileName);

void insertUnresolvedLabel(ProgramData* data, const char* labelName, Line* line);
void insertEntry(ProgramData* data, const char* labelName, Line* line);
void insertExtern(ProgramData* data, const char* labelName, Line* line);
void insertLabel(ProgramData* data, const char* labelName, Line* line);
void registerSymbol(ProgramData* data, const char* labelName, Line* line);

ProgramData initCommandsTable();
void initSymbolsTable(ProgramData* data);
void initRegisters(ProgramData* data);
void initEntries(ProgramData* data);


#endif /* PROGRAM_DS_MANIPULATORS_H_ */
