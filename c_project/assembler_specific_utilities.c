/*
 * assembler_specific_utilities.c
 *
 *  Created on: Mar 21, 2016
 *      Author: dorav
 */
#include "assembler_specific_utilities.h"

int numOfLabels(ProgramData* data)
{
	Symbol* current;
	lhash_iter i;
	int numberOfInternalSymbols = 0;

	for (i = lhash_begin(&data->symbols); i.isValid; lhash_set_next(&i))
	{
		current = (Symbol*) i.current->data;
		if (!current->isExternal)
			numberOfInternalSymbols++;
	}
	return numberOfInternalSymbols;
}
