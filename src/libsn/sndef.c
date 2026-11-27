#include "common.h"

#define SNDEF_DATA __attribute__((section(".data.sn_defaults"), aligned(4)))

u32 _stacksize SNDEF_DATA = 0x00008000;
u32 _ramsize SNDEF_DATA = 0x00800000;
