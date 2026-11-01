#include "common.h"

#ifdef NON_MATCHING
#include "libspu_private.h"
/* Best observed instruction-sequence similarity: 41.01% (89/217 words;
 * GCC 2.8.0, -mno-split-addresses). */
void _spu_gcSPU(void) {
    int i, j;
    SPU_MALLOC tmp;
    for (i = 0; i <= _spu_AllocLastNum;) {
        if (_spu_memList[i].addr & 0x80000000) {
            j = i + 1;
            while (_spu_memList[j].addr == 0x2fffffff) j++;
            if ((_spu_memList[j].addr & 0x80000000) &&
                (_spu_memList[j].addr & 0x0fffffff) ==
                (_spu_memList[i].addr & 0x0fffffff) + _spu_memList[i].size) {
                _spu_memList[j].addr = 0x2fffffff;
                _spu_memList[i].size += _spu_memList[j].size;
                continue;
            }
        }
        i++;
    }
    for (i = 0; i <= _spu_AllocLastNum; i++)
        if (!_spu_memList[i].size) _spu_memList[i].addr = 0x2fffffff;
    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) break;
        for (j = i + 1; j <= _spu_AllocLastNum; j++) {
            if (_spu_memList[j].addr & 0x40000000) break;
            if ((_spu_memList[j].addr & 0x0fffffff) < (_spu_memList[i].addr & 0x0fffffff)) {
                tmp = _spu_memList[i];
                _spu_memList[i] = _spu_memList[j];
                _spu_memList[j] = tmp;
            }
        }
    }
    for (i = 0; i <= _spu_AllocLastNum; i++) {
        if (_spu_memList[i].addr & 0x40000000) break;
        if (_spu_memList[i].addr == 0x2fffffff) {
            _spu_memList[i] = _spu_memList[_spu_AllocLastNum];
            _spu_AllocLastNum = i;
            break;
        }
    }
    for (i = _spu_AllocLastNum - 1; i >= 0; i--) {
        if (!(_spu_memList[i].addr & 0x80000000)) break;
        _spu_memList[i].addr = (_spu_memList[i].addr & 0x0fffffff) | 0x40000000;
        _spu_memList[i].size += _spu_memList[_spu_AllocLastNum].size;
        _spu_AllocLastNum = i;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_m_int", _spu_gcSPU);
#endif
