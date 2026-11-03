#include "common.h"

#ifdef NON_MATCHING
#include "reverb_private.h"

/* Best observed instruction-sequence similarity: 81.25% (13/16 words,
 * -mno-split-addresses -fno-delayed-branch; target includes one padding word).
 * The compiler reuses mask=16 for variable-count sign-extension shifts;
 * the original uses immediate shifts. */
void SsUtSetReverbFeedback(s16 feedback) {
    SpuReverbAttr *attr = &_svm_rattr;
    attr->mask = 16;
    attr->feedback = feedback;
    SpuSetReverbModeParam(attr);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_rfb", SsUtSetReverbFeedback);
#endif
