#include "common.h"
#include "libsnd_private.h"

void _SsUtResolveADSR(u16 adsr1, u16 adsr2, Envelope *env) {
    env->attack_mode = adsr1 & 0x8000;
    env->sustain_mode = adsr2 & 0x8000;
    env->sustain_direction = adsr2 & 0x4000;
    env->release_mode = adsr2 & 0x20;
    env->attack_rate = (adsr1 >> 8) & 0x7f;
    env->decay_rate = (adsr1 >> 4) & 0xf;
    env->sustain_level = adsr1 & 0xf;
    env->sustain_rate = (adsr2 >> 6) & 0x7f;
    env->release_rate = adsr2 & 0x1f;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 31.71% (13/41 words;
 * GCC 2.7.2). */
void _SsUtBuildADSR(Envelope *env, u16 *adsr1, u16 *adsr2) {
    int smode = -(env->sustain_mode != 0) & 0x8000;
    int amode = -(env->attack_mode != 0) & 0x8000;
    if (env->sustain_direction) smode |= 0x4000;
    if (env->release_mode) smode |= 0x20;
    *adsr1 = amode | ((env->attack_rate << 8) & 0x7f00)
             | ((env->decay_rate << 4) & 0xf0) | (env->sustain_level & 15);
    *adsr2 = smode | ((env->sustain_rate << 6) & 0x1fc0)
             | (env->release_rate & 31);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ccadsr", _SsUtBuildADSR);
#endif
