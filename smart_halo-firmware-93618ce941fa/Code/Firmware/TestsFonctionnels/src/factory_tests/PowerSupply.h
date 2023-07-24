/*
 * PowerSupply.h
 *
 *  Created on: 2016-06-22
 *      Author: Seb
 */

#ifndef SRC_POWERSUPPLY_H_
#define SRC_POWERSUPPLY_H_

#include <stdbool.h>

typedef enum
{
	INVALID_SUPPLY = -1,
	SUPPLY_VLED,
	SUPPLY_VPIEZO,
	SUPPLY_S2_8V,
	SUPPLY_ALL
} PowerSupplyType;

void PowerSupply_setup();

bool PowerSupply_parseAndExecuteCommand(char * RxBuff, int cnt);

void PowerSupply_printHelp();

void PowerSupply_enable(PowerSupplyType supply);

void PowerSupply_disable(PowerSupplyType supply);

bool PowerSupply_getState(PowerSupplyType supply);

#endif /* SRC_POWERSUPPLY_H_ */
