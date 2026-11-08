#pragma once

#include "decomp/types.h"

/* Fixed hardware RAM, not storage allocated in the executable.
 * Keep the extent in sync with scratchpad in symbol_addrs.txt.
 */
extern u8 scratchpad[0x400];
