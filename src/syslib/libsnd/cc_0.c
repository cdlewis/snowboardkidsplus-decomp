#include "common.h"
#include "libsnd_private.h"

extern s32 _SsReadDeltaValue(s16, s16);

void _SsContBankChange(s16 id, s16 seq) {
    struct SeqStruct *s = &D_8013BF10[id][seq];
    u8 bank = *s->seq_ptr;
    s->seq_ptr++;
    s->vab_id = bank;
    s->delta_value = _SsReadDeltaValue(id, seq);
}
