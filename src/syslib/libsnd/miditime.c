#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
/* Best observed instruction-sequence similarity: 75.00% (33/44 words,
 * including target padding). */
s32 _SsReadDeltaValue(s16 id, s16 seq) {
    struct SeqStruct *s = &_ss_score[id][seq];
    u32 value = *s->seq_ptr++;
    if (value) {
        if (value & 0x80) {
            u8 byte;
            value &= 0x7f;
            do {
                byte = *s->seq_ptr++;
                value = (value << 7) + (byte & 0x7f);
            } while (byte & 0x80);
        }
        value *= 10;
        s->unk88 += value;
    }
    return value;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/miditime", _SsReadDeltaValue);
#endif
