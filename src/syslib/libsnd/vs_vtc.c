#include "common.h"
#include "libsnd_private.h"
#include "psx/libspu.h"


s16 SsVabTransCompleted(s16 immediate) { return SpuIsTransferCompleted(immediate); }
