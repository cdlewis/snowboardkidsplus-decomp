#include "common.h"
#include "libspu_private.h"

u32 SpuSetTransferStartAddr(u32 addr) {
    u32 result;
    u32 limit = 0x7EFE8;
    u32 offset = addr - 0x1010;

    if (limit < offset) return 0;
    result = _spu_FsetRXXa(-1, addr);
    _spu_tsa = result;
    return _spu_tsa << _spu_mem_mode_plus;
}
