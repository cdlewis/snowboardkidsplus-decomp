#include "common.h"
#include "vm_g.h"

#ifdef NON_MATCHING
typedef struct { u8 bytes[16]; } Program;
extern u8 _svm_vab_used[];
extern s16 kMaxPrograms;
extern void *_svm_vab_vh[], *_svm_vab_tn[];
extern Program *_svm_vab_pg[], *_svm_pg;
extern void *_svm_vh, *_svm_tn;
/* Best observed instruction-sequence similarity: 62.50% (30/48 words;
 * GCC 2.8.0, -mno-split-addresses). */
int _SsVmVSetUp(s16 vab, s16 program) {
    if ((u16)vab >= 16) return -1;
    if (_svm_vab_used[vab] != 1) return -1;
    if (program < kMaxPrograms) {
        void *a = _svm_vab_vh[vab];
        Program *b = _svm_vab_pg[vab];
        void *c = _svm_vab_tn[vab];
        _svm_cur.vabId = vab;
        _svm_cur.prog = program;
        _svm_tn = c;
        _svm_vh = a;
        _svm_pg = b;
        _svm_cur.fake_program = b[program].bytes[8];
        return 0;
    }
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_vsu", _SsVmVSetUp);
#endif
