#include "common.h"

#ifdef NON_MATCHING
#include "reverb_private.h"

/* Best observed instruction-sequence similarity: 81.25% (13/16 words,
 * -mno-split-addresses -fno-delayed-branch; target includes one padding word). */
void SsUtSetReverbFeedback(s16 feedback) {
    SpuReverbAttr *attr = &D_8013CEC8;
    attr->mask = 16;
    attr->feedback = feedback;
    SpuSetReverbModeParam(attr);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_rfb", SsUtSetReverbFeedback);
#endif
