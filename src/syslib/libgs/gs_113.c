#include "libgs_private.h"

extern u32 *ClearOTagR(u32 *ot, s32 count);

void GsClearOt(u16 offset, u16 point, GsOT *ot) {
    ot->offset = offset;
    ot->point = point;
    ot->tag = ot->org + (1 << ot->length) - 1;
    ClearOTagR(ot->org, 1 << ot->length);
}
