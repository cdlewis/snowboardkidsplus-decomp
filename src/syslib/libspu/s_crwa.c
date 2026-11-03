#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 60.58% (63/104 words; target includes padding). */
#include "libspu_private.h"
extern u32 _spu_rev_startaddr[];
extern s32 _spu_mem_mode_plus, _spu_transMode, _spu_EVdma;
extern void (*volatile _spu_transferCallback)(void);
extern u8 _spu_zerobuf[];
extern void _spu_t(int, ...);
extern int WaitEvent(int);
int SpuClearReverbWorkArea(int mode) {
    u32 size, address, chunk;
    int changed = 0, more = 1, oldmode;
    void (*saved)(void) = 0;
    if ((u32)mode >= 10 || _SpuIsInAllocateArea_(_spu_rev_startaddr[mode])) return -1;
    if (mode == 0) { size = 16 << _spu_mem_mode_plus; address = 0xfff0 << _spu_mem_mode_plus; }
    else { size = (0x10000 - _spu_rev_startaddr[mode]) << _spu_mem_mode_plus; address = _spu_rev_startaddr[mode] << _spu_mem_mode_plus; }
    oldmode = _spu_transMode;
    if (oldmode == 1) { _spu_transMode = 0; changed = 1; }
    if (_spu_transferCallback) { saved = _spu_transferCallback; _spu_transferCallback = 0; }
    do {
        chunk = 0x400;
        if (size <= 0x400) { chunk = size; more = 0; }
        _spu_t(2, address);
        _spu_t(1);
        _spu_t(3, _spu_zerobuf, chunk);
        size -= 0x400;
        address += 0x400;
        WaitEvent(_spu_EVdma);
    } while (more);
    if (changed) _spu_transMode = oldmode;
    if (saved) _spu_transferCallback = saved;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_crwa", SpuClearReverbWorkArea);
#endif
