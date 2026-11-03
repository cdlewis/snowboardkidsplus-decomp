#pragma once

#include "common.h"

/* https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/include/psyq/libsnd.h */
#define CC_NUMBER 0
#define CC_BANKCHANGE 1
#define CC_DATAENTRY 2
#define CC_MAINVOL 3
#define CC_PANPOT 4
#define CC_EXPRESSION 5
#define CC_DAMPER 6
#define CC_NRPN1 7
#define CC_NRPN2 8
#define CC_RPN1 9
#define CC_RPN2 10
#define CC_EXTERNAL 11
#define CC_RESETALL 12

typedef struct {
    void (*noteon)();
    void (*programchange)();
    void (*pitchbend)();
    void (*metaevent)();
    void (*control[13])();
    void (*ccentry[20])();
} _SsFCALL;

extern _SsFCALL SsFCALL;
