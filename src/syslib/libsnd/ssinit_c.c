#include "common.h"

extern void ResetCallback(void), SpuInit(void), _SsInit(void);

void func_800A89F4(void) {
    ResetCallback();
    SpuInit();
    _SsInit();
}
