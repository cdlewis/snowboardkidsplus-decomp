/* Adapted from PsyZ decomp; see THIRD_PARTY.md and licenses/psyz-decomp.txt. */
#include "libspu_private.h"

#define ALLOC_DATA __attribute__((section(".data.spu_allocator"), aligned(4)))
s32 _spu_AllocBlockNum ALLOC_DATA = 0;
s32 _spu_AllocLastNum ALLOC_DATA = 0;
SPU_MALLOC *_spu_memList ALLOC_DATA = NULL;

s32 SpuInitMalloc(s32 num, char* top) {
    if (num > 0) {
        _spu_memList = (SPU_MALLOC*)top;
        _spu_memList[0].addr = 0x40001010;
        _spu_memList[0].size = (0x10000 << _spu_mem_mode_plus) - 0x1010;
        _spu_AllocLastNum = 0;
        _spu_AllocBlockNum = num;
        return num;
    }
    return 0;
}
