#include "common.h"
#include "libsnd_private.h"

/* SDK wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

extern void _SsVmSeqKeyOff(s16 id);

void _SsSndStop(short sep_access_num, short seq_num) {
    struct SeqStruct* score;
    int i;

    score = &D_8013BF10[sep_access_num][seq_num];
    D_8013BF10[sep_access_num][seq_num].flags &= ~1;
    D_8013BF10[sep_access_num][seq_num].flags &= ~2;
    D_8013BF10[sep_access_num][seq_num].flags &= ~8;
    D_8013BF10[sep_access_num][seq_num].flags &= ~0x400;
    D_8013BF10[sep_access_num][seq_num].flags |= 4;
    _SsVmSeqKeyOff(sep_access_num | (seq_num << 8));
    _SsVmDamperOff();
    score->play_mode = 0;
    score->unk88 = 0;
    score->unk1C = 0;
    score->unk18 = 0;
    score->unk19 = 0;
    score->unk1E = 0;
    score->fn_idx = 0;
    score->unk1B = 0;
    score->unk1F = 0;
    score->channel_idx = 0;
    score->unk21 = 0;
    score->unk1C = 0;
    score->unk1D = 0;
    score->unk15 = 0;
    score->running_status = 0;
    score->delta_value = score->unk84;
    score->unk94 = score->tempo;
    score->unk54 = score->unk56;
    score->seq_ptr = score->read_pos;
    score->next_sep_pos = score->read_pos;
    for (i = 0; i < 16; i++) {
        score->programs[i] = i;
        score->panpot[i] = 64;
        score->vol[i] = 127;
    }
    score->unk5C = 127;
    score->unk5E = 127;
}

void SsSeqStop(s16 id) { _SsSndStop(id, 0); }

void SsSepStop(s16 id,s16 sequence) { _SsSndStop(id, sequence); }
