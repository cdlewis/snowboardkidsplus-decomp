#include "common.h"

#ifdef NON_MATCHING
extern struct TickEnv { s32 mode, manual; } _snd_seq_tick_env;
extern s32 VBLANK_MINUS;
extern int GetVideoMode(void);
/* Best observed instruction-sequence similarity: 51.19% (43/84 words;
 * GCC 2.8.0, -mno-split-addresses; original uses a jump table). */
void SsSetTickMode(int mode) {
    int video = GetVideoMode();
    if (mode & 0x1000) {
        _snd_seq_tick_env.manual = 1;
        _snd_seq_tick_env.mode = mode & 0xfff;
    } else {
        _snd_seq_tick_env.manual = 0;
        _snd_seq_tick_env.mode = mode;
    }
    mode = _snd_seq_tick_env.mode;
    if (mode == 4) {
        VBLANK_MINUS = 50;
        _snd_seq_tick_env.mode = video == 1 ? 5 : 50;
    } else if (mode == 1) {
        VBLANK_MINUS = 60;
        _snd_seq_tick_env.mode = video ? 60 : 5;
    } else if (mode == 3) {
        VBLANK_MINUS = 120;
    } else if (mode == 2) {
        VBLANK_MINUS = 240;
    } else if (mode == 0 || mode == 5 || mode < 0) {
        VBLANK_MINUS = video == 1 ? 50 : 60;
    } else {
        VBLANK_MINUS = mode;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/sstick", SsSetTickMode);
#endif
