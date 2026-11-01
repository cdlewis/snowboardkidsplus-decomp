#ifdef NON_MATCHING
#include "libsnd_private.h"
extern void func_800B0604(void), func_800B0624(void);
/* Best observed instruction-sequence similarity: 58.33% (28/48 words,
 * including target padding). */
void _SsSetNrpnVabAttr1(s16 a, s16 b, s16 c, VagAtr attr, s16 d, unsigned int value) {
    SsUtGetVagAtr(a, b, c, &attr);
    attr.mode = value;
    SsUtSetVagAtr(a, b, c, &attr);
    if ((u8)value == 0)
        func_800B0604();
    else if ((u8)value == 4)
        func_800B0624();
}
#else
#include "common.h"
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/de_1", _SsSetNrpnVabAttr1);
#endif
