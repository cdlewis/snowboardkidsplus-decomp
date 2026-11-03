#include "common.h"
#include "sscall.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern int _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 54.29% (76/140 words;
 * GCC 2.7.2). */
void _SsSetControlChange(s16 id, s16 seq, u8 control) {
    struct SeqStruct *s = &_ss_score[id][seq];
    u8 value = *s->seq_ptr++;
    switch (control) {
    case 0: s->vab_id = value; break;
    case 6: SsFCALL.control[CC_DATAENTRY](id, seq, value); return;
    case 7: SsFCALL.control[CC_MAINVOL](id, seq, value); return;
    case 10: SsFCALL.control[CC_PANPOT](id, seq, value); return;
    case 11: SsFCALL.control[CC_EXPRESSION](id, seq, value); return;
    case 64: SsFCALL.control[CC_DAMPER](id, seq, value); return;
    case 91: SsFCALL.control[CC_EXTERNAL](id, seq, value); return;
    case 98: SsFCALL.control[CC_NRPN1](id, seq, value); return;
    case 99: SsFCALL.control[CC_NRPN2](id, seq, value); return;
    case 100: SsFCALL.control[CC_RPN1](id, seq, value); return;
    case 101: SsFCALL.control[CC_RPN2](id, seq, value); return;
    case 121: SsFCALL.control[CC_RESETALL](id, seq); return;
    }
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midicc", _SsSetControlChange);
#endif
