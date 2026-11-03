#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern void _SsGetSeqData(s16, s16);
/* Best observed instruction-sequence similarity: 54.29% (38/70 words;
 * GCC 2.8.0, -mno-split-addresses). */
void _SsSeqPlay(s16 id, s16 seq) {
    struct SeqStruct *s = &_ss_score[id][seq];
    int remaining = s->delta_value - s->unk54;
    if (remaining > 0) {
        if (s->unk52 > 0) s->unk52--;
        else if (s->unk52 == 0) { s->unk52 = s->unk54; s->delta_value--; }
        else s->delta_value = remaining;
    } else if (s->unk54 >= s->delta_value) {
        int sum = s->delta_value;
        do {
            _SsGetSeqData(id, seq);
            if (s->delta_value) sum += s->delta_value;
        } while (!s->delta_value || sum < s->unk54);
        s->delta_value = sum - s->unk54;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midiread", _SsSeqPlay);
#endif

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern void _SsSndNextSep(s16, s16), _SsVmSeqKeyOff(s16);
/* Best observed instruction-sequence similarity: 86.21% (125/145 words;
 * GCC 2.8.0). */
void _SsSeqGetEof(s16 id, s16 seq) {
    struct SeqStruct *s = &_ss_score[id][seq];
    s->unk21++;
    if (!(s8)s->l_count) {
        s->unk88 = 0;
        s->unk1C = 0;
        s->delta_value = 0;
        s->seq_ptr = (_ss_score[id][seq].flags & 0x400) ? s->loop_pos : s->read_pos;
    } else if ((s8)s->unk21 < (s8)s->l_count) {
        s->unk88 = 0;
        s->unk1C = 0;
        s->delta_value = 0;
        if (_ss_score[id][seq].flags & 0x400) s->seq_ptr = s->next_sep_pos = s->loop_pos;
        else s->seq_ptr = s->next_sep_pos = s->read_pos;
    } else {
        _ss_score[id][seq].flags &= ~1;
        _ss_score[id][seq].flags &= ~8;
        _ss_score[id][seq].flags &= ~2;
        _ss_score[id][seq].flags |= 0x200;
        _ss_score[id][seq].flags |= 4;
        s->play_mode = 0;
        s->next_sep_pos = (_ss_score[id][seq].flags & 0x400) ? s->loop_pos : s->read_pos;
        if ((s8)s->unk22 != -1) {
            s->play_mode = 0;
            _SsSndNextSep((s8)s->unk22, (s8)s->unk23);
            _SsVmSeqKeyOff((s16)(id | (seq << 8)));
        }
        _SsVmSeqKeyOff((s16)(id | (seq << 8)));
        s->delta_value = s->unk54;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midiread", _SsSeqGetEof);
#endif

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midiread", _SsGetSeqData);
