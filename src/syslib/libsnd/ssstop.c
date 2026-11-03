#include "common.h"
#include "libsnd_private.h"


extern void _SsVmSeqKeyOff(s16 id);

void _SsSndStop(short sep_access_num, short seq_num) {
    struct SeqStruct* score;
    int i;

    score = &_ss_score[sep_access_num][seq_num];
    _ss_score[sep_access_num][seq_num].flags &= ~1;
    _ss_score[sep_access_num][seq_num].flags &= ~2;
    _ss_score[sep_access_num][seq_num].flags &= ~8;
    _ss_score[sep_access_num][seq_num].flags &= ~0x400;
    _ss_score[sep_access_num][seq_num].flags |= 4;
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
