#include "../menu/selection_tasks_internal.h"
#include "game/audio/music.h"
#include "psx/libsnd.h"

void playMusicSequence(s16 count, u8 left, u8 right) {
    if (gMusicSequenceActive) {
        D_8013AF30 = left << 8;
        SsSeqSetVol(gMusicSequenceId, left, right);
        SsSeqPlay(gMusicSequenceId, 1, count);
    }
}
