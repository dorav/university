/*
 * specific_commands_factories.h
 *
 *  Created on: Mar 15, 2016
 *      Author: dorav
 */

#ifndef SPECIFIC_COMMANDS_FACTORIES_H_
#define SPECIFIC_COMMANDS_FACTORIES_H_
#include "types.h"

AllowedAddressings addressingIsIrrelevant = { 0 };
AllowedAddressings genericSingleArgCommandRestrictions()
{
	AllowedAddressings allowed = { 0 };

	allowed.destAddressingTypes.isRegisterAllowed = True;
	allowed.destAddressingTypes.isDirectAllowed = True;

	return allowed;
}

UserCommand makeEntryCommand()
{
	UserCommand u;
	u.opcode = InvalidOpcode;
	u.group = SpecialGroup;
	u.handler = parseEntryCommand;
	u.addressingTypes = addressingIsIrrelevant,
	strcpy(u.name, ".entry");
	return u;
}

UserCommand makeExternCommand()
{
	UserCommand u;
	u.opcode = InvalidOpcode;
	u.group = SpecialGroup;
	u.handler = parseExternCommand;
	u.addressingTypes = addressingIsIrrelevant,
	strcpy(u.name, ".extern");
	return u;
}

UserCommand makeRtsCommand()
{
	UserCommand u;
	u.opcode = RtsOpcode;
	u.group = NoArgsGroup;
	u.handler = parseNoArgsCommand;
	u.addressingTypes = addressingIsIrrelevant,
	strcpy(u.name, "rts");
	return u;
}

UserCommand makeStopCommand()
{
	UserCommand u;
	u.opcode = StopOpcode;
	u.group = NoArgsGroup;
	u.handler = parseNoArgsCommand;
	u.addressingTypes = addressingIsIrrelevant,
	strcpy(u.name, "stop");
	return u;
}

UserCommand makeNotCommand()
{
	UserCommand u;
	u.opcode = NotOpcode;
	u.group = SingleArgGroup;
	u.handler = genericSingleArgCommand;
	u.addressingTypes = genericSingleArgCommandRestrictions();
	u.addressingTypes.destAddressingTypes.isDirectAllowed = False;
	strcpy(u.name, "not");
	return u;
}

UserCommand makeRedCommand()
{
	UserCommand u;
	u.opcode = RedOpcode;
	u.group = SingleArgGroup;
	u.handler = genericSingleArgCommand;
	u.addressingTypes = genericSingleArgCommandRestrictions();
	strcpy(u.name, "red");
	return u;
}


#endif /* SPECIFIC_COMMANDS_FACTORIES_H_ */
