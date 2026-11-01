#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern s8 D_8013CFC0[];
extern VagAtr *D_8013CFB8;
/* Best observed instruction-sequence similarity: 73.08% (38/52 words;
 * target includes padding). */
u8 _SsVmSelectToneAndVag(u8 *tones, u8 *vags) {
    u8 count = 0;
    s8 i;
    for (i = 0; i < D_8013CFC0[0]; i++) {
        VagAtr *attr = &D_8013CFB8[D_8013CFC0[7] * 16 + i];
        if (D_8013CFC0[2] >= attr->min && D_8013CFC0[2] <= attr->max) {
            vags[count] = ((u8 *)attr)[22];
            tones[count++] = i;
        }
    }
    return count;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_stav", _SsVmSelectToneAndVag);
#endif
