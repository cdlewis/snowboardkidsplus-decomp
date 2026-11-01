#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
typedef struct { u32 voice, mask; u8 rest[56]; } VoiceAttr;
extern s8 D_8013CFBC;
extern u32 D_80132DC4;
extern s16 D_8013CFDA;
extern void SpuSetVoiceAttr(VoiceAttr *), _SsVmKeyOffNow(int);
/* Best observed instruction-sequence similarity: 47.50% (38/80 words). */
void SsUtAllKeyOff(void) {
    VoiceAttr attr;
    s16 i;
    attr.mask = 0x60093;
    *(s16 *)(attr.rest + 0) = 0;
    *(s16 *)(attr.rest + 2) = 0;
    *(s16 *)(attr.rest + 12) = 0x1000;
    *(u32 *)(attr.rest + 20) = 0x1000;
    *(u16 *)(attr.rest + 50) = 0x80ff;
    *(u16 *)(attr.rest + 52) = 0x4000;
    for (i = 0; i < D_8013CFBC; i++) {
        u8 *v = (u8 *)&D_8013C7C8[i];
        u32 bit = 1 << i;
        if (!(D_80132DC4 & bit)) {
            *(s16 *)(v + 2) = 24;
            *(s16 *)(v + 6) = 0;
            *(s16 *)(v + 16) = 255;
            *(s16 *)(v + 18) = 0;
            *(s16 *)(v + 20) = 0;
            *(s16 *)(v + 22) = 255;
            attr.voice = bit;
            SpuSetVoiceAttr(&attr);
            D_8013CFDA = i;
            _SsVmKeyOffNow(1);
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_ako", SsUtAllKeyOff);
#endif
