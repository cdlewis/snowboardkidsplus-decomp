#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern void _SsVmSetSeqVol(s16, u16, u16, s16);
/* Best observed instruction-sequence similarity: 67.67% (157/232 words;
 * GCC 2.8.0, assembler reordering). */
void _SsSndDecrescendo(s16 id, s16 seq) {
    struct SeqStruct *s = &_ss_score[id][seq];
    s16 key = id | (seq << 8);
    s16 l, r;
    s->v_time_r--;
    if (s->v_time_r < 0) { _ss_score[id][seq].flags &= ~0x20; goto done; }
    if (s->unk4C > 0) {
        if (s->v_time_r % s->unk4C) goto done;
        s->vol_r--;
        if (s->vol_r < 0) {
            _SsVmSetSeqVol(key, 0, 0, 1);
            _ss_score[id][seq].flags &= ~0x20;
        } else {
            _SsVmGetSeqVol(key, &l, &r);
            if (!((u16)l - 1 < (u16)l - s->vol_r && (u16)r - 1 < (u16)r - s->vol_r)) {
                if (!l || !r) _ss_score[id][seq].flags &= ~0x20;
                else _SsVmSetSeqVol(key, l - 1, r - 1, 1);
            }
        }
    } else if (s->unk4C < 0) {
        s->vol_r += s->unk4C;
        if (s->vol_r < 0) {
            _SsVmSetSeqVol(key, 0, 0, 1);
            _ss_score[id][seq].flags &= ~0x20;
        } else {
            _SsVmGetSeqVol(key, &l, &r);
            if ((u16)l + s->unk4C <= 0 && (u16)r + s->unk4C <= 0) {
                _SsVmSetSeqVol(key, 0, 0, 1);
                _ss_score[id][seq].flags &= ~0x20;
            }
            if ((s->v_time_l - s->v_time_r) * -s->unk4C < s->vol_l) {
                if (!l || !r) _ss_score[id][seq].flags &= ~0x20;
                else _SsVmSetSeqVol(key, l + s->unk4C, r + s->unk4C, 1);
            }
        }
    } else goto done;
    if (!s->v_time_r || s->vol_r <= 0) _ss_score[id][seq].flags &= ~0x20;
done:
    _SsVmGetSeqVol(key, &s->unk5C, &s->unk5E);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/decres", _SsSndDecrescendo);
#endif
