#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 51.72% (60/116 words; target includes padding). */
#include "libsnd_private.h"
extern u32 D_8013BF0C;
extern int _SsReadDeltaValue(s16, s16);
void _SsGetMetaEvent(s16 id, s16 seq) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    int value, tempo;
    u32 product, rate;
    value = *s->seq_ptr++ << 16;
    value |= *s->seq_ptr++ << 8;
    value |= *s->seq_ptr++;
    tempo = 60000000 / value;
    s->unk94 = tempo;
    product = s->resolution * tempo;
    rate = D_8013BF0C * 60;
    if (product * 10 < rate) {
        s->unk54 = D_8013BF0C * 600 / product;
    } else {
        u32 quotient = product * 10 / rate;
        u32 remainder = product * 10 % rate;
        s->unk52 = -1;
        s->unk54 = quotient;
        if (remainder > D_8013BF0C * 30) s->unk54 = quotient + 1;
    }
    s->delta_value = _SsReadDeltaValue(id, seq);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/midimeta", _SsGetMetaEvent);
#endif
