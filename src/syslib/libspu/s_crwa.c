#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 60.58% (63/104 words; target includes padding). */
#include "libspu_private.h"
extern u32 D_80132EF4[];
extern s32 _spu_mem_mode_plus, _spu_transMode, D_80132E0C;
extern void (*volatile _spu_transferCallback)(void);
extern u8 D_80132F44[];
extern void _spu_t(int, ...);
extern int WaitEvent(int);
int SpuClearReverbWorkArea(int mode) {
    u32 size, address, chunk;
    int changed = 0, more = 1, oldmode;
    void (*saved)(void) = 0;
    if ((u32)mode >= 10 || _SpuIsInAllocateArea_(D_80132EF4[mode])) return -1;
    if (mode == 0) { size = 16 << _spu_mem_mode_plus; address = 0xfff0 << _spu_mem_mode_plus; }
    else { size = (0x10000 - D_80132EF4[mode]) << _spu_mem_mode_plus; address = D_80132EF4[mode] << _spu_mem_mode_plus; }
    oldmode = _spu_transMode;
    if (oldmode == 1) { _spu_transMode = 0; changed = 1; }
    if (_spu_transferCallback) { saved = _spu_transferCallback; _spu_transferCallback = 0; }
    do {
        chunk = 0x400;
        if (size <= 0x400) { chunk = size; more = 0; }
        _spu_t(2, address);
        _spu_t(1);
        _spu_t(3, D_80132F44, chunk);
        size -= 0x400;
        address += 0x400;
        WaitEvent(D_80132E0C);
    } while (more);
    if (changed) _spu_transMode = oldmode;
    if (saved) _spu_transferCallback = saved;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_crwa", SpuClearReverbWorkArea);
#endif
