/*!
    @file BoardRev.h

    Interfaces to return PCB revision

    @author     Georg Nikodym
    @copyright  Copyright (c) 2020 SmartHalo Inc
*/

#ifndef _BOARDREV_H_
#define _BOARDREV_H_

#include "main.h"

#define BASE_BOARD_REV  0

void init_BoardRev(void);
uint8_t get_BoardRev(void);

#endif // _BOARDREV_H_
