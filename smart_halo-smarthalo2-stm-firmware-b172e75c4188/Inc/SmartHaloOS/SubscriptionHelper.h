/*
 * SubscriptionHelper.h
 *
 *  Created on: 24 Sep 2019
 *      Author: Nzo
 */

#ifndef SMARTHALOOS_SUBSCRIPTIONHELPER_H_
#define SMARTHALOOS_SUBSCRIPTIONHELPER_H_

#include "main.h"
#include "string.h"

void addSubscription(void (*array[])(), uint8_t * count, uint8_t limit, char * name, void * function);
void removeSubscription(void (*array[])(), uint8_t * count, char * name, void * function);

#endif /* SMARTHALOOS_SUBSCRIPTIONHELPER_H_ */
