#include "common.h"

#ifdef NON_MATCHING
typedef struct { u8 bytes[16]; } Program;
extern u8 D_8013CFE0[], D_8013CFC1[];
extern s16 D_8013CFAA;
extern void *D_8013CF20[], *D_8013CF68[];
extern Program *D_8013CEE0[], *D_8013CFAC;
extern void *D_8013CFB4, *D_8013CFB8;
/* Best observed instruction-sequence similarity: 62.50% (30/48 words;
 * GCC 2.8.0, -mno-split-addresses). */
int _SsVmVSetUp(s16 vab, s16 program) {
    if ((u16)vab >= 16) return -1;
    if (D_8013CFE0[vab] != 1) return -1;
    if (program < D_8013CFAA) {
        void *a = D_8013CF20[vab];
        Program *b = D_8013CEE0[vab];
        void *c = D_8013CF68[vab];
        D_8013CFC1[0] = vab;
        D_8013CFC1[5] = program;
        D_8013CFB8 = c;
        D_8013CFB4 = a;
        D_8013CFAC = b;
        D_8013CFC1[6] = b[program].bytes[8];
        return 0;
    }
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_vsu", _SsVmVSetUp);
#endif
