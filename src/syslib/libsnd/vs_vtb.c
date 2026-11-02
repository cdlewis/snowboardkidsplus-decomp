#include "common.h"

#include "../libspu/libspu_private.h"
extern u8 D_8013CFE0[];
extern u32 D_8013D040[], D_8013CFF8[];
s16 SsVabTransBody(u8 *body, s16 id) {
    if ((u16)id <= 16 && D_8013CFE0[id] == 2) {
        u32 addr = D_8013D040[id];
        SpuSetTransferMode(0);
        SpuSetTransferStartAddr(addr);
        SpuWrite(body, D_8013CFF8[id]);
        D_8013CFE0[id] = 1;
        return id;
    }
    _spu_setInTransfer(0);
    return -1;
}
