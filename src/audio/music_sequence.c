#include "../menu/selection_tasks_internal.h"
#include "game/audio/music.h"
#include "psx/libsnd.h"

extern s16 gMusicSequenceId __attribute__((section(".data")));
extern u8 gMusicSequenceActive __attribute__((section(".data")));
extern s16 gMusicSequenceVolume __attribute__((section(".data")));

void playMusicSequence(s16 count, u8 left, u8 right) {
    if (gMusicSequenceActive) {
        gMusicSequenceVolume = left << 8;
        SsSeqSetVol(gMusicSequenceId, left, right);
        SsSeqPlay(gMusicSequenceId, 1, count);
    }
}
