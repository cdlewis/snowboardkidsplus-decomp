#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 39.02% (64/164 words; GCC 2.8.0, -mno-split-addresses). */
#include "libsnd_private.h"
extern s8 D_8013CFBC, D_8013CFCF;
extern u32 D_80132DC4;
u8 _SsVmAlloc(void) {
    u8 chosen = 99, best = 99, count = 0, i;
    u16 level = 0xffff, age = 0, priority = D_8013CFCF;
    for (i = 0; i < D_8013CFBC; i++) {
        u8 *v = (u8 *)&D_8013C7C8[i];
        if (D_80132DC4 & (1 << i)) continue;
        if (!*(s8 *)(v + 29) && !*(u16 *)(v + 6)) { chosen = i; break; }
        if (*(s16 *)(v + 26) < priority) {
            priority = *(u16 *)(v + 26);
            best = i;
            level = *(u16 *)(v + 6);
            age = *(u16 *)(v + 2);
            count = 1;
        } else if (*(s16 *)(v + 26) == priority) {
            count++;
            if (*(u16 *)(v + 6) < level) {
                age = *(u16 *)(v + 2);
                level = *(u16 *)(v + 6);
                best = i;
            } else if (*(u16 *)(v + 6) == level && age < *(s16 *)(v + 2)) {
                age = *(u16 *)(v + 2);
                best = i;
            }
        }
    }
    if (chosen == 99) chosen = count ? best : D_8013CFBC;
    if (chosen < D_8013CFBC) {
        for (i = 0; i < D_8013CFBC; i++)
            if (!(D_80132DC4 & (1 << i))) (*(u16 *)((u8 *)&D_8013C7C8[i] + 2))++;
        *(u16 *)((u8 *)&D_8013C7C8[chosen] + 2) = 0;
        *(u16 *)((u8 *)&D_8013C7C8[chosen] + 42) = 0;
        *(u16 *)((u8 *)&D_8013C7C8[chosen] + 30) = 0;
        *(s16 *)((u8 *)&D_8013C7C8[chosen] + 26) = D_8013CFCF;
    }
    return chosen;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_aloc1", _SsVmAlloc);
#endif
