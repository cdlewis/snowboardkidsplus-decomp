#include "common.h"
#include "psx/libspu.h"


s16 SsVabTransCompleted(s16 immediate) { return SpuIsTransferCompleted(immediate); }
