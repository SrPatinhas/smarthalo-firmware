/*
 * SubscriptionHelper.c
 *
 *  Created on: 24 Sep 2019
 *      Author: Matt
 */

#include <SystemUtilitiesTask.h>
#include "SubscriptionHelper.h"
#include "reboot.h"

void addSubscription(void (*array[])(), uint8_t * count, uint8_t limit, char name[], void * function){
    if(*count >= limit){
        log_Shell(name);
        log_Shell("^ subscribers already full!!");
#ifdef MEGABUG_HUNT
        while(1); // will hang the firmware
#else
        SOFT_CRASH(eSUBSCRIPTIONHELPER);
#endif
    }
    array[*count] = function;
    (*count)++;
}


void removeSubscription(void (*array[])(), uint8_t * count, char * name, void * function){
    if(*count == 0){
        log_Shell(name);
        log_Shell("^ has no subscribers to remove!");
#ifdef MEGABUG_HUNT
        while(1); // will hang the firmware
#else
        SOFT_CRASH(eSUBSCRIPTIONHELPER);
#endif
    }
    bool found = false;
    for(int i=0;i<*count;i++){
        if(found){
            array[i-1] = array[i];
        }
        if(array[i] == function){
            array[i] = NULL;
            found = true;
        }
    }
    if(found){
        (*count)--;
    }
}
