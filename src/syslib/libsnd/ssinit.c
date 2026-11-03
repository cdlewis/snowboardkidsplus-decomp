#include "common.h"

#ifdef NON_MATCHING
extern u16 svmInitialVoiceRegisters[], svmInitialGlobalRegisters[];
extern u32 _SsMarkCallback[32][16];
extern int VBLANK_MINUS, _snd_openflag, _snd_ev_flag;
extern void _SsVmInit(int);
/* Best observed instruction-sequence similarity: 43.33% (26/60 words;
 * target includes padding). */
void _SsInit(void) {
    volatile u16 *spu = (volatile u16 *)0x1f801c00;
    int i, j;
    for (i = 0; i < 24; i++)
        for (j = 0; j < 8; j++) *spu++ = svmInitialVoiceRegisters[j];
    spu = (volatile u16 *)0x1f801d80;
    for (i = 0; i < 16; i++) *spu++ = svmInitialGlobalRegisters[i];
    _SsVmInit(24);
    for (i = 0; i < 32; i++)
        for (j = 15; j >= 0; j--) _SsMarkCallback[i][j] = 0;
    VBLANK_MINUS = 60;
    _snd_openflag = 0;
    _snd_ev_flag = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssinit", _SsInit);
#endif
