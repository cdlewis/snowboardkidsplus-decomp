#include "common.h"
#include "libcd_private.h"
#include "psx/libetc.h"

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

extern char *CD_comstr[], *CD_intstr[];
extern char cdUnknownCommandName[];

char *CdComstr(u8 index) {
    char *result;
    if (index <= 27) result = CD_comstr[index];
    else result = cdUnknownCommandName;
    return result;
}

char *CdIntstr(u8 index) {
    char *result;
    if (index <= 6) result = CD_intstr[index];
    else result = cdUnknownCommandName;
    return result;
}

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

#ifdef NON_MATCHING
extern int cdCommandHasLocation[];
extern int CD_cw(u8, u8 *, u8 *, int);

/* Best observed instruction-sequence similarity: 83.54% (66/79 words). */
int CdControl(u8 com, u8 *param, u8 *response) {
    int retries = 3;
    int result = 0;
    CdlCB callback = CD_cbsync;
    do {
        CD_cbsync = 0;
        if (com != 1 && (CD_status & 0x10)) CD_cw(1, 0, 0, 0);
        if (!param || !cdCommandHasLocation[com] || !CD_cw(2, param, response, 0)) {
            CD_cbsync = callback;
            if (!CD_cw(com, param, response, 0)) break;
        }
    } while (retries--);
    if (retries < 0) { CD_cbsync = callback; result = -1; }
    return result + 1;
}

/* Best observed instruction-sequence similarity: 77.92% (60/77 words). */
int CdControlF(u8 com, u8 *param) {
    int retries = 3;
    int result = 0;
    CdlCB callback = CD_cbsync;
    do {
        CD_cbsync = 0;
        if (com != 1 && (CD_status & 0x10)) CD_cw(1, 0, 0, 0);
        if (!param || !cdCommandHasLocation[com] || !CD_cw(2, param, 0, 0)) {
            CD_cbsync = callback;
            if (!CD_cw(com, param, 0, 1)) break;
        }
    } while (retries--);
    if (retries < 0) { CD_cbsync = callback; result = -1; }
    return result + 1;
}

/* Best observed instruction-sequence similarity: 81.18% (69/85 words). */
int CdControlB(u8 com, u8 *param, u8 *response) {
    int retries = 3;
    int result = 0;
    CdlCB callback = CD_cbsync;
    do {
        CD_cbsync = 0;
        if (com != 1 && (CD_status & 0x10)) CD_cw(1, 0, 0, 0);
        if (!param || !cdCommandHasLocation[com] || !CD_cw(2, param, response, 0)) {
            CD_cbsync = callback;
            if (!CD_cw(com, param, response, 0)) break;
        }
    } while (retries--);
    if (retries < 0) { CD_cbsync = callback; result = -1; }
    if (!result) return CD_sync(0, response) == 2;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdControl);
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdControlF);
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/sys", CdControlB);
#endif

s32 CdMix(CdlATV *vol) { CD_vol(vol); return 1; }

s32 CdGetSector(void *madr, s32 size) { return CD_getsector(madr, size) == 0; }

s32 CdGetSector2(void *madr, s32 size) { return CD_getsector2(madr, size) == 0; }

DmaCallback CdDataCallback(DmaCallback cb) { return DMACallback(3, cb); }

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
