#include "common.h"

extern u8 _svm_vab_used[];
extern u32 _svm_vab_start[];
extern u16 _svm_vab_count;
extern void SpuFree(u32);
void SsVabClose(s16 id) {
    if (id >= 0 && id < 16 && _svm_vab_used[id] == 1) {
        SpuFree(_svm_vab_start[id]);
        _svm_vab_used[id] = 0;
        _svm_vab_count--;
    }
}
