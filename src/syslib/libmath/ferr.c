#include "common.h"

s32 math_errno __attribute__((section(".data.math_error"), aligned(4))) = 0;
s32 math_err_point __attribute__((section(".data.math_error"), aligned(4))) = 0;
extern int DeliverEvent(u32, u32);

int _err_math(int err, int value) {
    math_errno = err;
    math_err_point = value;
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
