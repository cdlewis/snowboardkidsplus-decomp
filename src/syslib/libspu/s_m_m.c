#include "common.h"

#ifdef NON_MATCHING
#include "libspu_private.h"
extern int _spu_rev_reserve_wa, _spu_rev_offsetaddr;
extern u32 _spu_mem_mode_unitM;
extern void _spu_gcSPU(void);
/* Best observed instruction-sequence similarity: 43.24% (80/185 words;
 * GCC 2.7.2, assembler reordering). */
s32 SpuMalloc(s32 size) {
    int i, index = -1;
    s32 reserve;
    SPU_MALLOC last;
    if (_spu_rev_reserve_wa) reserve = (0x10000 - _spu_rev_offsetaddr) << _spu_mem_mode_plus;
    else reserve = 0;
    if (size & ~_spu_mem_mode_unitM) size += _spu_mem_mode_unitM;
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
