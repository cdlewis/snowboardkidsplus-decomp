#include "common.h"
#include "libsnd_private.h"


struct SndSeqTickEnv {
    s32 tick_mode;
    s32 manual_tick;
    void (*tick_cb)(void);
    void (*vsync_cb)(void);
    u8 vsync_tick;
    u8 unk11;
    u8 alarm_tick;
    s32 unk14;
};

extern void SsSeqCalledTbyT(void);

struct SndSeqTickEnv _snd_seq_tick_env
    __attribute__((section(".data.snd_seq_tick_env"), aligned(4))) = {
        60, 1, SsSeqCalledTbyT, 0, 0, 0, 127, 0
    };

#ifdef NON_MATCHING
#include "psx/libetc.h"
extern int EnterCriticalSection(void), ExitCriticalSection(void), ResetRCnt(u32), SetRCnt(u32, u16, int);
extern void func_800AC754(void), _SsSeqCalledTbyT_1per2(void);
/* Best observed instruction-sequence similarity: 59.29% (83/140 words;
 * GCC 2.8.0, -mno-split-addresses). */
void _SsStart(int use_vsync) {
    int i, period;
    u32 timer;
    struct SndSeqTickEnv *env;
    for (i = 999; i >= 0; i--) { }
    timer = 0xf2000002; period = 0x44e8; env = &_snd_seq_tick_env;
    env->alarm_tick = 6; env->vsync_tick = 0; env->unk11 = 0; env->vsync_cb = 0;
    switch (env->tick_mode) {
    case 0: env->alarm_tick = 127; return;
    case 2: break;
    case 3: period = 0x89d0; break;
    case 5:
        env->alarm_tick = 0;
        if (!use_vsync) env->vsync_tick = 1;
        else { timer = 0xf2000003; period = 1; }
        break;
    default:
        if (env->manual_tick) return;
        if (env->tick_mode < 70) { period = 0x204cc0 / env->tick_mode; env->unk11++; }
        else period = 0x409980 / env->tick_mode;
    }
    if (env->vsync_tick) {
        EnterCriticalSection();
        VSyncCallback(env->tick_cb);
    } else {
        EnterCriticalSection();
        ResetRCnt(timer);
        SetRCnt(timer, period, 0x1000);
        if (!env->alarm_tick) {
            env->vsync_cb = InterruptCallback(0, 0);
            InterruptCallback(env->alarm_tick, func_800AC754);
        } else {
            InterruptCallback(env->alarm_tick, env->unk11 ? _SsSeqCalledTbyT_1per2 : env->tick_cb);
        }
    }
    ExitCriticalSection();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssstart", _SsStart);
#endif

void SsStart(void) { _SsStart(1); }

void SsStart2(void) { _SsStart(0); }

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 75.00% (15/20 words, -O1). */
void func_800AC754(void) {
    struct SndSeqTickEnv *env = &_snd_seq_tick_env;

    if (env->vsync_cb) env->vsync_cb();
    env->tick_cb();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssstart", func_800AC754);
#endif

void _SsSeqCalledTbyT_1per2(void) {
    struct SndSeqTickEnv *env = &_snd_seq_tick_env;

    if (env->unk14 == 0) {
        env->unk14 = 1;
    } else {
        env->unk14 = 0;
        env->tick_cb();
    }
}
