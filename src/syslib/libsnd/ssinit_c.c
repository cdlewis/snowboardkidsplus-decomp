#include "common.h"

extern void ResetCallback(void), func_800A8A24(void), _SsInit(void);

void func_800A89F4(void) {
    ResetCallback();
    func_800A8A24();
    _SsInit();
}
