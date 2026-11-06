#pragma once

#include "decomp/types.h"

void PadInitDirect(u8 *port0, u8 *port1);
void PadSetAct(s32 port, u8 *data, s32 size);
void PadStartCom(void);
