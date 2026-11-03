#include "common.h"
#include "vm_g.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"

extern VagAtr *_svm_tn;
/* Best observed instruction-sequence similarity: 73.08% (38/52 words;
 * target includes padding). */
u8 _SsVmSelectToneAndVag(u8 *tones, u8 *vags) {
    u8 count = 0;
    s8 i;
    for (i = 0; i < _svm_cur.prog_tones; i++) {
        VagAtr *attr = &_svm_tn[_svm_cur.fake_program * 16 + i];
        if (_svm_cur.note >= attr->min && _svm_cur.note <= attr->max) {
            vags[count] = ((u8 *)attr)[22];
            tones[count++] = i;
        }
    }
    return count;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_stav", _SsVmSelectToneAndVag);
#endif
