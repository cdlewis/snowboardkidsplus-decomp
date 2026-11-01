#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 81.29% (113/139 words; target includes padding). */
#include "libsnd_private.h"
extern u32 D_8013BF0C;
void _SsSndTempo(s16 id, s16 seq) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    s->unkA8--;
    if (s->unkA8 < 0) {
        D_8013BF10[id][seq].flags &= ~0x40;
        D_8013BF10[id][seq].flags &= ~0x80;
        return;
    }
    if (s->unk4E > 0) {
        if (s->unkA8 % s->unk4E) return;
        if (s->unk94 > s->unkAC) s->unk94--;
        else if (s->unk94 < s->unkAC) s->unk94++;
    } else {
        if (s->unk94 > s->unkAC) {
            s->unk94 += s->unk4E;
            if (s->unk94 < s->unkAC) s->unk94 = s->unkAC;
        } else if (s->unk94 < s->unkAC) {
            s->unk94 -= s->unk4E;
            if (s->unk94 > s->unkAC) s->unk94 = s->unkAC;
        }
    }
    s->unk54 = (s->resolution * s->unk94 * 10) / (D_8013BF0C * 60);
    if (s->unk54 <= 0) s->unk54 = 1;
    if (!s->unkA8 || s->unk94 == s->unkAC) {
        D_8013BF10[id][seq].flags &= ~0x40;
        D_8013BF10[id][seq].flags &= ~0x80;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/tempo", _SsSndTempo);
#endif
