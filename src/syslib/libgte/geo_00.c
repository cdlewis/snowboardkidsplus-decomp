#include "libgte_private.h"

extern s32 sin_1(s32);

s32 rsin(s32 a) {
    s32 value;
    if (a >= 0) value = sin_1(a & 0xfff);
    else value = -sin_1(-a & 0xfff);
    return value;
}

s32 sin_1(s32 a) {
    if (a <= 0x800) {
        if (a <= 0x400) return rsin_tbl[a];
        return rsin_tbl[0x800 - a];
    }
    if (a <= 0xc00) return -rsin_tbl[a - 0x800];
    else return -rsin_tbl[0x1000 - a];
}
