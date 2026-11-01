#include "common.h"
#include "libsnd_private.h"

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_seq", _SsVmSetSeqVol);

#ifdef NON_MATCHING
extern s16 D_8013CFD6;

/* Best observed instruction-sequence similarity: 56.00% (14/25 words, -O1). */
s16 _SsVmGetSeqVol(s16 id, s16 *left, s16 *right) {
    struct SeqStruct *score = D_8013BF10[id & 0xFF];
    D_8013CFD6 = id;
    *left = score[(id & 0xFF00) >> 8].voll;
    *right = score[(id & 0xFF00) >> 8].volr;
    return D_8013CFD6;
}

/* Best observed instruction-sequence similarity: 72.22% (13/18 words, -O1). */
s16 _SsVmGetSeqLVol(s16 id) {
    struct SeqStruct *score = D_8013BF10[id & 0xFF];
    D_8013CFD6 = id;
    return score[(id & 0xFF00) >> 8].voll;
}

/* Best observed instruction-sequence similarity: 72.22% (13/18 words, -O1). */
s16 _SsVmGetSeqRVol(s16 id) {
    struct SeqStruct *score = D_8013BF10[id & 0xFF];
    D_8013CFD6 = id;
    return score[(id & 0xFF00) >> 8].volr;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_seq", _SsVmGetSeqVol);
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_seq", _SsVmGetSeqLVol);
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_seq", _SsVmGetSeqRVol);
#endif

extern s8 D_8013CFBC;
extern u32 D_80132DC4;
extern u16 D_8013CFDA;
extern void _SsVmKeyOffNow(int);
void _SsVmSeqKeyOff(s16 id) {
    u8 i;
    for (i = 0; i < D_8013CFBC; i++) {
        if (!(D_80132DC4 & (1 << i)) && D_8013C7C8[i].seq_id == id) {
            D_8013CFDA = i;
            _SsVmKeyOffNow(0);
        }
    }
}
