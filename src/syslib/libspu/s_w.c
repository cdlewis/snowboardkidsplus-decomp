#include "libspu_private.h"
extern u32 _spu_Fw(u8 *address, u32 size);
extern void (*_spu_transferCallback)(void);
u32 SpuWrite(u8 *address, u32 size) {
    if (size > 0x7eff0)
        size = 0x7eff0;
    _spu_Fw(address, size);
    if (_spu_transferCallback == 0)
        _spu_inTransfer = 0;
    return size;
}
