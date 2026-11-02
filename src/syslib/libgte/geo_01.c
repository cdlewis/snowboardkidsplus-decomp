/* Adapted from PsyZ decomp; see THIRD_PARTY.md. */
#include "libgte_private.h"

s32 rcos(s32 a) {
    if (a < 0) a = -a;
    a &= 0xfff;
    if (a <= 0x800) {
        if (a <= 0x400) return rsin_tbl[0x400 - a];
        else return -rsin_tbl[a - 0x400];
    } else {
        if (a <= 0xc00) return -rsin_tbl[0xc00 - a];
        else return rsin_tbl[a - 0xc00];
    }
}
