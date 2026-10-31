#include "common.h"
#include "libcd_private.h"

/* Wrappers and BCD conversions adapted from PsyZ decomp; see THIRD_PARTY.md. */
int CdStatus(void) { return CD_status; }

int CdMode(void) { return CD_mode; }

int CdLastCom(void) { return CD_com; }

CdlLOC* CdLastPos(void) { return &CD_pos; }

int CdReset(int mode) {
    if (mode == CdlModeAP) {
        CD_initintr();
        return 1;
    }

    if (CD_init()) {
        return 0;
    }

    if (mode == CdlModeDA && CD_initvol()) {
        return 0;
    }

    return 1;
}

void CdFlush(void) { CD_flush(); }

int CdSetDebug(int level) {
    int prev = CD_debug;
    CD_debug = level;
    return prev;
}

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdComstr);

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdIntstr);

s32 CdSync(s32 mode, u8 *result) { return CD_sync(mode, result); }

s32 CdReady(s32 mode, u8 *result) { return CD_ready(mode, result); }

CdlCB CdSyncCallback(CdlCB func) {
    CdlCB prev = CD_cbsync;
    CD_cbsync = func;
    return prev;
}

CdlCB CdReadyCallback(CdlCB func) {
    CdlCB prev = CD_cbready;
    CD_cbready = func;
    return prev;
}

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdControl);

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdControlF);

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdControlB);

s32 CdMix(CdlATV *vol) { CD_vol(vol); return 1; }

s32 CdGetSector(void *madr, s32 size) { return CD_getsector(madr, size) == 0; }

s32 CdGetSector2(void *madr, s32 size) { return CD_getsector2(madr, size) == 0; }

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdDataCallback);

s32 CdDataSync(s32 mode) { return CD_datasync(mode); }

static inline int ENCODE_BCD(int n) { return ((n / 10) << 4) + (n % 10); }
CdlLOC* CdIntToPos(int i, CdlLOC* p) {
    i += 150;
    p->sector = ENCODE_BCD(i % 75);
    p->second = ENCODE_BCD(i / 75 % 60);
    p->minute = ENCODE_BCD(i / 75 / 60);
    return p;
}

int CdPosToInt(CdlLOC* p) {
#define DECODE_BCD(x) (((x) >> 4) * 10 + ((x) & 0xF))
    u8 sector = p->sector;
    u8 second = p->second;
    u8 minute = p->minute;

    return (DECODE_BCD(minute) * 60 + DECODE_BCD(second)) * 75 +
           DECODE_BCD(sector) - 150;
}
