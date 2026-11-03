#include "common.h"
#include "vm_g.h"

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_keyv", SsUtKeyOnV);

#ifdef NON_MATCHING
extern s32 _snd_ev_flag;

extern void _SsVmKeyOffNow(int);
/* Best observed instruction-sequence similarity: 59.38% (19/32 words, including padding). */
s16 SsUtKeyOffV(s16 voice) {
    if (_snd_ev_flag == 1)
        return -1;
    _snd_ev_flag = 1;
    if (voice < 0 || voice >= 24) {
        _snd_ev_flag = 0;
        return -1;
    }
    _svm_cur.voice = voice;
    _SsVmKeyOffNow(0);
    _snd_ev_flag = 0;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_keyv", SsUtKeyOffV);
#endif
