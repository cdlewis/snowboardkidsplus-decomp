#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern u32 D_8013BF0C;
extern const char D_80012400[], D_80012418[];
extern int printf(const char *, ...), _SsReadDeltaValue(s16, s16);
/* Best observed instruction-sequence similarity: 46.23% (98/212 words;
 * GCC 2.7.2, assembler reordering; target includes padding). */
int _SsInitSoundSeq(s16 id, s16 vab, u8 *data) {
    struct SeqStruct *s = D_8013BF10[id];
    int i, us, bpm, remainder;
    u32 rate, product, q, rem;
    s->vab_id = vab;
    s->resolution = 0;
    s->unk18 = s->unk19 = s->unk1E = s->fn_idx = s->unk1B = s->unk1F = s->channel_idx = 0;
    s->unk84 = s->unk88 = s->tempo = 0;
    s->unk56 = 0; s->unk21 = 0; s->l_count = 1;
    s->play_mode = 0; s->delta_value = 0;
    s->unk1C = s->unk1D = s->unk15 = s->running_status = 0;
    s->channel_mute = 0; s->rhythm_n = s->rhythm_d = 0;
    for (i = 0; i < 16; i++) { s->programs[i] = i; s->panpot[i] = 64; s->vol[i] = 127; }
    s->unk52 = 1;
    s->seq_ptr = data;
    if (*data != 'S' && *data != 'p') { printf(D_80012418); return 0; }
    s->seq_ptr = data + 7;
    if (*s->seq_ptr++ != 1) { printf(D_80012400); return -1; }
    s->resolution = *s->seq_ptr++ << 8;
    s->resolution |= *s->seq_ptr++;
    us = *s->seq_ptr++ << 16;
    us |= *s->seq_ptr++ << 8;
    us |= *s->seq_ptr++;
    bpm = 60000000 / us;
    remainder = 60000000 % us;
    s->tempo = bpm + (remainder > ((u32)us >> 1));
    s->unk94 = s->tempo;
    s->rhythm_n = *s->seq_ptr++;
    s->rhythm_d = *s->seq_ptr++;
    s->delta_value = _SsReadDeltaValue(id, 0);
    s->unk84 = s->delta_value;
    s->next_sep_pos = s->loop_pos = s->read_pos = s->seq_ptr;
    s->unk10 = 0;
    product = s->resolution * s->tempo;
    rate = D_8013BF0C * 60;
    if (product * 10 < rate) {
        s->unk52 = D_8013BF0C * 600 / product;
        s->unk54 = s->unk52;
    } else {
        q = (s->resolution * s->tempo * 10) / rate;
        rem = (s->resolution * s->tempo * 10) % rate;
        s->unk52 = -1;
        s->unk54 = q;
        if (rem > D_8013BF0C * 30) s->unk54 = q + 1;
    }
    s->unk56 = s->unk54;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/seqinit", _SsInitSoundSeq);
#endif
