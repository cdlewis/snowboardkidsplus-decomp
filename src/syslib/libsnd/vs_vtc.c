#include "common.h"
#include "libsnd_private.h"
#include "psx/libspu.h"

/* SDK wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

s16 SsVabTransCompleted(s16 immediate) { return SpuIsTransferCompleted(immediate); }
