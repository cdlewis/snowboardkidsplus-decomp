#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern int _SsVmKeyOn(s16, s16, s16, u16, u16, u16);
extern int _SsVmKeyOff(s16, s16, s16, u16);
/* Best observed instruction-sequence similarity: 75.00% (42/56 words;
 * target includes padding). */
void _SsNoteOn(s16 id, s16 seq, u8 note, u8 velocity) {
    struct SeqStruct *s = &_ss_score[id][seq];
    int channel = s->channel_idx;
    u8 pan = s->panpot[channel];
    if (velocity) {
        if (!((s->channel_mute >> channel) & 1))
            _SsVmKeyOn((s16)(id | (seq << 8)), (s8)s->vab_id,
                       s->programs[channel], note, velocity, pan);
    } else {
        _SsVmKeyOff((s16)(id | (seq << 8)), (s8)s->vab_id,
                    s->programs[channel], note);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midinote", _SsNoteOn);
#endif
