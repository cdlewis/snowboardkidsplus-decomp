#include "common.h"

#ifdef NON_MATCHING
#include "libspu_private.h"
extern int D_80132E1C, D_80132E20;
extern u32 D_80132EB4;
extern void _spu_gcSPU(void);
/* Best observed instruction-sequence similarity: 43.24% (80/185 words;
 * GCC 2.7.2, assembler reordering). */
s32 SpuMalloc(s32 size) {
    int i, index = -1;
    s32 reserve;
    SPU_MALLOC last;
    if (D_80132E1C) reserve = (0x10000 - D_80132E20) << _spu_mem_mode_plus;
    else reserve = 0;
    if (size & ~D_80132EB4) size += D_80132EB4;
    size = (size >> _spu_mem_mode_plus) << _spu_mem_mode_plus;
    if (_spu_memList[0].addr & 0x40000000) index = 0;
    else {
        _spu_gcSPU();
        for (i = 0; i < _spu_AllocBlockNum; i++) {
            if ((_spu_memList[i].addr & 0x40000000) ||
                ((_spu_memList[i].addr & 0x80000000) && _spu_memList[i].size >= (u32)size)) {
                index = i;
                break;
            }
        }
    }
    if (index == -1) return -1;
    if (_spu_memList[index].addr & 0x40000000) {
        if (index >= _spu_AllocBlockNum || _spu_memList[index].size - reserve < (u32)size) return -1;
        _spu_memList[index + 1].addr = ((_spu_memList[index].addr & 0x0fffffff) + size) | 0x40000000;
        _spu_memList[index + 1].size = _spu_memList[index].size - size;
        _spu_AllocLastNum = index + 1;
        _spu_memList[index].size = size;
        _spu_memList[index].addr &= 0x0fffffff;
        _spu_gcSPU();
        return _spu_memList[index].addr;
    }
    if ((u32)size < _spu_memList[index].size && _spu_AllocLastNum < _spu_AllocBlockNum) {
        last = _spu_memList[_spu_AllocLastNum];
        _spu_memList[_spu_AllocLastNum].addr = (_spu_memList[index].addr + size) | 0x80000000;
        _spu_memList[_spu_AllocLastNum].size = _spu_memList[index].size - size;
        _spu_AllocLastNum++;
        _spu_memList[_spu_AllocLastNum] = last;
    }
    _spu_memList[index].size = size;
    _spu_memList[index].addr &= 0x0fffffff;
    _spu_gcSPU();
    return _spu_memList[index].addr;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_m_m", SpuMalloc);
#endif
