#include "common.h"

#include "../libspu/libspu_private.h"
extern u8 _svm_vab_used[];
extern u32 _svm_vab_start[], _svm_vab_total[];
s16 SsVabTransBody(u8 *body, s16 id) {
    if ((u16)id <= 16 && _svm_vab_used[id] == 2) {
        u32 addr = _svm_vab_start[id];
        SpuSetTransferMode(0);
        SpuSetTransferStartAddr(addr);
        SpuWrite(body, _svm_vab_total[id]);
        _svm_vab_used[id] = 1;
        return id;
    }
    _spu_setInTransfer(0);
    return -1;
}
