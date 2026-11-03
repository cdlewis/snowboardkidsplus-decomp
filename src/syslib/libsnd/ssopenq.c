#include "common.h"
#include "sscall.h"

#ifdef NON_MATCHING
extern u32 _snd_openflag;
extern const char sndSequenceSlotsFullMessage[];
extern int printf(const char *, ...), _SsInitSoundSeq(s16, s16, void *);
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
    if (_snd_openflag == 0xffffffff) { printf(sndSequenceSlotsFullMessage); return -1; }
    for (i = 0; i < 32;) {
        if (!(_snd_openflag & (1U << i))) { slot = i; found = 1; }
        i++;
        if (i >= 32) { printf(sndSequenceSlotsFullMessage); return -1; }
        if (found) break;
    }
    _snd_openflag |= 1U << slot;
    result = _SsInitSoundSeq(slot, vab, data);
    SsFCALL.noteon = _SsNoteOn;
    SsFCALL.programchange = _SsSetProgramChange;
    SsFCALL.metaevent = _SsGetMetaEvent;
    SsFCALL.pitchbend = _SsSetPitchBend;
    SsFCALL.control[CC_NUMBER] = _SsSetControlChange;
    SsFCALL.control[CC_BANKCHANGE] = _SsContBankChange;
    SsFCALL.control[CC_MAINVOL] = _SsContMainVol;
    SsFCALL.control[CC_PANPOT] = _SsContPanpot;
    SsFCALL.control[CC_EXPRESSION] = _SsContExpression;
    SsFCALL.control[CC_DAMPER] = _SsContDamper;
    SsFCALL.control[CC_NRPN1] = _SsContNrpn1;
    SsFCALL.control[CC_NRPN2] = _SsContNrpn2;
    SsFCALL.control[CC_RPN1] = _SsContRpn1;
    SsFCALL.control[CC_RPN2] = _SsContRpn2;
    SsFCALL.control[CC_EXTERNAL] = _SsContExternal;
    SsFCALL.control[CC_RESETALL] = _SsContResetAll;
    SsFCALL.control[CC_DATAENTRY] = _SsContDataEntry;
    SsFCALL.ccentry[0] = _SsSetNrpnVabAttr0;
    SsFCALL.ccentry[1] = _SsSetNrpnVabAttr1;
    SsFCALL.ccentry[2] = _SsSetNrpnVabAttr2;
    SsFCALL.ccentry[3] = _SsSetNrpnVabAttr3;
    SsFCALL.ccentry[4] = _SsSetNrpnVabAttr4;
    SsFCALL.ccentry[5] = _SsSetNrpnVabAttr5;
    SsFCALL.ccentry[6] = _SsSetNrpnVabAttr6;
    SsFCALL.ccentry[7] = _SsSetNrpnVabAttr7;
    SsFCALL.ccentry[8] = _SsSetNrpnVabAttr8;
    SsFCALL.ccentry[9] = _SsSetNrpnVabAttr9;
    SsFCALL.ccentry[10] = _SsSetNrpnVabAttr10;
    SsFCALL.ccentry[11] = _SsSetNrpnVabAttr11;
    SsFCALL.ccentry[12] = _SsSetNrpnVabAttr12;
    SsFCALL.ccentry[13] = _SsSetNrpnVabAttr13;
    SsFCALL.ccentry[14] = _SsSetNrpnVabAttr14;
    SsFCALL.ccentry[15] = _SsSetNrpnVabAttr15;
    SsFCALL.ccentry[16] = _SsSetNrpnVabAttr16;
    SsFCALL.ccentry[17] = func_800AB214;
    SsFCALL.ccentry[18] = func_800AB244;
    SsFCALL.ccentry[19] = func_800AB274;
    if (result == -1) return -1;
    return slot;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssopenq", SsSeqOpen);
#endif
