#include "common.h"

#ifdef NON_MATCHING
extern u32 D_8013BF08;
extern const char D_800123D0[];
extern int printf(const char *, ...), _SsInitSoundSeq(s16, s16, void *);
extern void (*D_8013BE70[])(void);
extern void _SsNoteOn(void);
extern void _SsSetProgramChange(void);
extern void _SsGetMetaEvent(void);
extern void _SsSetPitchBend(void);
extern void _SsSetControlChange(void);
extern void _SsContBankChange(void);
extern void _SsContMainVol(void);
extern void _SsContPanpot(void);
extern void _SsContExpression(void);
extern void _SsContDamper(void);
extern void _SsContNrpn1(void);
extern void _SsContNrpn2(void);
extern void _SsContRpn1(void);
extern void _SsContRpn2(void);
extern void _SsContExternal(void);
extern void _SsContResetAll(void);
extern void _SsContDataEntry(void);
extern void _SsSetNrpnVabAttr0(void);
extern void _SsSetNrpnVabAttr1(void);
extern void _SsSetNrpnVabAttr2(void);
extern void _SsSetNrpnVabAttr3(void);
extern void _SsSetNrpnVabAttr4(void);
extern void _SsSetNrpnVabAttr5(void);
extern void _SsSetNrpnVabAttr6(void);
extern void _SsSetNrpnVabAttr7(void);
extern void _SsSetNrpnVabAttr8(void);
extern void _SsSetNrpnVabAttr9(void);
extern void _SsSetNrpnVabAttr10(void);
extern void _SsSetNrpnVabAttr11(void);
extern void _SsSetNrpnVabAttr12(void);
extern void _SsSetNrpnVabAttr13(void);
extern void _SsSetNrpnVabAttr14(void);
extern void _SsSetNrpnVabAttr15(void);
extern void _SsSetNrpnVabAttr16(void);
extern void func_800AB214(void);
extern void func_800AB244(void);
extern void func_800AB274(void);
/* Best observed instruction-sequence similarity: 89.29% (150/168 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
s16 SsSeqOpen(void *data, s16 vab) {
    int i, found = 0;
    s16 slot = 0, result;
    if (D_8013BF08 == 0xffffffff) { printf(D_800123D0); return -1; }
    for (i = 0; i < 32;) {
        if (!(D_8013BF08 & (1U << i))) { slot = i; found = 1; }
        i++;
        if (i >= 32) { printf(D_800123D0); return -1; }
        if (found) break;
    }
    D_8013BF08 |= 1U << slot;
    result = _SsInitSoundSeq(slot, vab, data);
    D_8013BE70[0] = _SsNoteOn;
    D_8013BE70[1] = _SsSetProgramChange;
    D_8013BE70[3] = _SsGetMetaEvent;
    D_8013BE70[2] = _SsSetPitchBend;
    D_8013BE70[4] = _SsSetControlChange;
    D_8013BE70[5] = _SsContBankChange;
    D_8013BE70[7] = _SsContMainVol;
    D_8013BE70[8] = _SsContPanpot;
    D_8013BE70[9] = _SsContExpression;
    D_8013BE70[10] = _SsContDamper;
    D_8013BE70[11] = _SsContNrpn1;
    D_8013BE70[12] = _SsContNrpn2;
    D_8013BE70[13] = _SsContRpn1;
    D_8013BE70[14] = _SsContRpn2;
    D_8013BE70[15] = _SsContExternal;
    D_8013BE70[16] = _SsContResetAll;
    D_8013BE70[6] = _SsContDataEntry;
    D_8013BE70[17] = _SsSetNrpnVabAttr0;
    D_8013BE70[18] = _SsSetNrpnVabAttr1;
    D_8013BE70[19] = _SsSetNrpnVabAttr2;
    D_8013BE70[20] = _SsSetNrpnVabAttr3;
    D_8013BE70[21] = _SsSetNrpnVabAttr4;
    D_8013BE70[22] = _SsSetNrpnVabAttr5;
    D_8013BE70[23] = _SsSetNrpnVabAttr6;
    D_8013BE70[24] = _SsSetNrpnVabAttr7;
    D_8013BE70[25] = _SsSetNrpnVabAttr8;
    D_8013BE70[26] = _SsSetNrpnVabAttr9;
    D_8013BE70[27] = _SsSetNrpnVabAttr10;
    D_8013BE70[28] = _SsSetNrpnVabAttr11;
    D_8013BE70[29] = _SsSetNrpnVabAttr12;
    D_8013BE70[30] = _SsSetNrpnVabAttr13;
    D_8013BE70[31] = _SsSetNrpnVabAttr14;
    D_8013BE70[32] = _SsSetNrpnVabAttr15;
    D_8013BE70[33] = _SsSetNrpnVabAttr16;
    D_8013BE70[34] = func_800AB214;
    D_8013BE70[35] = func_800AB244;
    D_8013BE70[36] = func_800AB274;
    if (result == -1) return -1;
    return slot;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssopenq", SsSeqOpen);
#endif
