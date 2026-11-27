#include "common.h"
#include "psx/libsnd.h"
#include "psx/libetc.h"

extern void SpuInit(void), _SsInit(void);

void SsInit(void) {
    ResetCallback();
    SpuInit();
    _SsInit();
}
