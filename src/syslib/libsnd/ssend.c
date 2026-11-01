#include "common.h"

#ifdef NON_MATCHING
#include "psx/libetc.h"
struct SndSeqTickEnv {
    s32 tick_mode, manual_tick;
    DmaCallback tick_cb, vsync_cb;
    u8 vsync_tick, unk11, alarm_tick;
    s32 unk14;
};
extern struct SndSeqTickEnv _snd_seq_tick_env;
extern int EnterCriticalSection(void), ExitCriticalSection(void);
/* Best observed instruction-sequence similarity: 68.18% (30/44 words,
 * including target padding; GCC 2.8.0, -mno-split-addresses). */
void SsEnd(void) {
    struct SndSeqTickEnv *env = &_snd_seq_tick_env;
    if (!env->manual_tick) {
        env->unk11 = 0;
        if (env->alarm_tick != 127) {
            EnterCriticalSection();
            if (env->vsync_tick) {
                VSyncCallback(0);
                env->vsync_tick = 0;
            } else if (env->alarm_tick == 0) {
                InterruptCallback(0, env->vsync_cb);
                env->vsync_cb = 0;
            } else {
                InterruptCallback(6, 0);
            }
            ExitCriticalSection();
            env->alarm_tick = 127;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssend", SsEnd);
#endif
