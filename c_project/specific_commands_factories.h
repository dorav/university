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

AllowedAddressings genericTwoArgCommandRestrictions()
{
	AllowedAddressings allowed = { 0 };

	allowed.destAddressingTypes.isRegisterAllowed = True;
	allowed.destAddressingTypes.isDirectAllowed = True;

	allowed.sourceAddressingTypes.isDirectAllowed = True;
	allowed.sourceAddressingTypes.isRegisterAllowed= True;
	allowed.sourceAddressingTypes.isRandomAllowed = True;
	allowed.sourceAddressingTypes.isInstantAllowed = True;

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

UserCommand makeDataCommand()
{
	UserCommand u;
	u.opcode = InvalidOpcode;
	u.group = SpecialGroup;
	u.handler = parseDataCommand;
	u.addressingTypes = addressingIsIrrelevant,
	strcpy(u.name, ".data");
	return u;
}

UserCommand makeStringCommand()
{
	UserCommand u;
	u.opcode = InvalidOpcode;
	u.group = SpecialGroup;
	u.handler = parseStringCommand;
	u.addressingTypes = addressingIsIrrelevant,
	strcpy(u.name, ".string");
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

UserCommand singleArgCommand(const char* name, CommandOpcode opcode)
{
	UserCommand u;

	u.opcode = opcode;
	u.group = SingleArgGroup;
	u.handler = genericSingleArgCommand;
	u.addressingTypes = genericSingleArgCommandRestrictions();
	strcpy(u.name, name);

	return u;
}

UserCommand twoArgCommand(const char* name, CommandOpcode opcode)
{
	UserCommand u;

	u.opcode = opcode;
	u.group = TwoArgsGroup;
	u.handler = genericTwoArgCommand;
	u.addressingTypes = genericTwoArgCommandRestrictions();
	strcpy(u.name, name);

	return u;
}

UserCommand makeCmpCommand()
{
	UserCommand prn = twoArgCommand("cmp", CmpOpcode);
	prn.addressingTypes.destAddressingTypes.isInstantAllowed = True;
	return prn;
}

UserCommand makeNotCommand()
{
	UserCommand u = singleArgCommand("not", NotOpcode);
	u.addressingTypes.destAddressingTypes.isDirectAllowed = False;
	return u;
}

UserCommand makePrnCommand()
{
	UserCommand prn = singleArgCommand("prn", PrnOpcode);
	prn.addressingTypes.destAddressingTypes.isInstantAllowed = True;
	return prn;
}

#endif /* SPECIFIC_COMMANDS_FACTORIES_H_ */
