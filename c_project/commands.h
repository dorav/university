/*
 * commands.h
 *
 *  Created on: Mar 12, 2016
 *      Author: dorav
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "types.h"
#include "line_parser.h"

UserCommandResult nullInstruction();

UserCommandResult parseEntryCommand(Line* line, const UserCommand* command, ProgramData* data);
UserCommandResult parseNoArgsCommand(Line* line, const UserCommand* command, ProgramData* data);
UserCommandResult parseExternCommand(Line* line, const UserCommand* command, ProgramData* data);

#endif /* COMMANDS_H_ */
