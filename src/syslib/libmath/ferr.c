#include "common.h"

s32 D_80132DBC __attribute__((section(".data.math_error"), aligned(4))) = 0;
s32 D_80132DC0 __attribute__((section(".data.math_error"), aligned(4))) = 0;
extern int DeliverEvent(u32, u32);

int _err_math(int err, int value) {
    D_80132DBC = err;
    D_80132DC0 = value;
    switch (err) {
    case 33:
        DeliverEvent(0xf4000002, 0x301);
        break;
    case 34:
        DeliverEvent(0xf4000002, 0x302);
        break;
    }
    return 0;
}
