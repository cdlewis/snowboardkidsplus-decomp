#include "common.h"
#include "libsnd_private.h"

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_seq", _SsVmSetSeqVol);

s16 _SsVmGetSeqVol(s16 id, s16 *left, s16 *right) {
    struct SeqStruct *score = _ss_score[id & 0xFF];
    s16 *current = &_svm_cur.seq_sep_no;
    *current = id;
    score += (id & 0xFF00) >> 8;
    *left = score->voll;
    *right = score->volr;
    return *current;
}

s16 _SsVmGetSeqLVol(s16 id) {
    struct SeqStruct *score = _ss_score[id & 0xFF];
    _svm_cur.seq_sep_no = id;
    score += (id & 0xFF00) >> 8;
    return (s16)score->voll;
}

s16 _SsVmGetSeqRVol(s16 id) {
    struct SeqStruct *score = _ss_score[id & 0xFF];
    _svm_cur.seq_sep_no = id;
    score += (id & 0xFF00) >> 8;
    return (s16)score->volr;
}

extern s8 _SsVmMaxVoice;
extern u32 svmReservedVoiceMask;

extern void _SsVmKeyOffNow(int);
void _SsVmSeqKeyOff(s16 id) {
    u8 i;
    for (i = 0; i < _SsVmMaxVoice; i++) {
        u32 voiceMask = 1;
        if (!(svmReservedVoiceMask & (voiceMask << i)) && _svm_voice[i].seq_id == id) {
            _svm_cur.voice = i;
            _SsVmKeyOffNow(0);
        }
    }
}
