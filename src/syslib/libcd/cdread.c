#include "common.h"
#include "psx/libcd.h"
#include "psx/libetc.h"
struct CdReadState {
    s32 count;
    u32 *buffer;
    u8 *next;
    s32 readmode, sectorsize;
    s32 active;
    s32 last, start, completed;
    CdlCB sync, ready;
    DmaCallback data;
    s32 mode;
    u8 *result;
};
extern struct CdReadState cdReadState;

CdlCB cdReadCallback __attribute__((section(".data.cd_read_callback"), aligned(4))) = 0;

extern DmaCallback CdDataCallback(DmaCallback);
extern s32 CdControlF(u8, u8 *);
#ifdef NON_MATCHING
extern const char cdReadSectorErrorMessage[];
extern int puts(const char *), VSync(int), func_800B8F34(int);
extern void func_800B8E68(void);
/* Best observed instruction-sequence similarity: 75.66% (115/152 words). */
void func_800B8C08(u8 intr, u8 *result) {
    volatile struct CdReadState *s = &cdReadState;
    u32 header[3];
    s->result = result;
    if (intr == 1) {
        if (s->active > 0) {
            if (s->sectorsize == 0x200) {
                if (s->mode & 1) {
                    CdDataCallback(0);
                    CdGetSector2(header, 3);
                    CdDataSync(0);
                    CdDataCallback(func_800B8E68);
                } else CdGetSector(header, 3);
                if (CdPosToInt((CdlLOC *)header) != s->completed) {
                    puts(cdReadSectorErrorMessage);
                    s->active = -1;
                }
            }
            if (s->mode & 1) CdGetSector2(s->next, s->sectorsize);
            else {
                CdGetSector(s->next, s->sectorsize);
                s->next += s->sectorsize * 4;
                s->active--;
                s->completed++;
            }
        }
    } else s->active = -1;
    s->last = VSync(-1);
    if (s->active < 0) func_800B8F34(1);
    if (VSync(-1) > s->start + 1200) s->active = -1;
    if (!s->active || VSync(-1) > s->start + 1200) {
        CdSyncCallback(s->sync);
        CdReadyCallback(s->ready);
        if (s->mode & 1) CdDataCallback(s->data);
        CdControlF(9, 0);
        if (cdReadCallback) cdReadCallback(s->active ? 5 : 2, result);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdread", func_800B8C08);
#endif

extern DmaCallback CdDataCallback(DmaCallback);
extern s32 CdControlF(u8, u8 *);

void func_800B8E68(void) {
    volatile struct CdReadState *s = &cdReadState;
    s->next += s->sectorsize * 4;
    s->active--;
    s->completed++;
    if (!s->active) {
        CdSyncCallback(s->sync);
        CdReadyCallback(s->ready);
        if (s->mode & 1) CdDataCallback(s->data);
        CdControlF(9, 0);
        if (cdReadCallback) cdReadCallback(2, s->result);
    }
}

#ifdef NON_MATCHING
extern const char cdReadShellOpenMessage[], cdReadRetryMessage[];
extern int puts(const char *), VSync(int), CdControl(u8, u8 *, u8 *);
extern void func_800B8C08(u8, u8 *);
/* Best observed instruction-sequence similarity: 71.77% (89/124 words). */
int func_800B8F34(int retry) {
    volatile struct CdReadState *s = &cdReadState;
    u8 mode;
    CdSyncCallback(0);
    CdReadyCallback(0);
    if (s->mode & 1) CdDataCallback(0);
    if (CdStatus() & 0x10) {
        if (!(VSync(-1) & 0x3f)) puts(cdReadShellOpenMessage);
        CdControlF(1, 0);
        s->start = VSync(-1);
        s->active = -1;
        return s->active;
    }
    if (retry) {
        puts(cdReadRetryMessage);
        CdControl(9, 0, 0);
        if (!CdControl(2, (u8 *)CdLastPos(), 0)) { s->active = -1; return s->count; }
    }
    CdFlush();
    mode = s->readmode;
    if (mode != CdMode() || retry) {
        if (!CdControl(0xe, &mode, 0)) { s->active = -1; return s->active; }
    }
    s->completed = CdPosToInt(CdLastPos());
    CdReadyCallback(func_800B8C08);
    if (s->mode & 1) CdDataCallback(func_800B8E68);
    s->next = (u8 *)s->buffer;
    CdControlF(6, 0);
    s->active = s->count;
    s->last = VSync(-1);
    return s->active;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdread", func_800B8F34);
#endif

#ifdef NON_MATCHING
extern DmaCallback CdDataCallback(DmaCallback);
extern s32 CdControlF(u8, u8 *);
/* Best observed instruction-sequence similarity: 54.05% (20/37 words;
 * GCC 2.8.0, -mno-split-addresses). */
s32 CdReadBreak(void) {
    struct CdReadState *s = &cdReadState;
    if (s->mode & 1)
        CdDataSync(0);
    s->active = 0;
    CdSyncCallback(s->sync);
    CdReadyCallback(s->ready);
    if (s->mode & 1)
        CdDataCallback(s->data);
    return CdControlF(9, 0);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdread", CdReadBreak);
#endif

#ifdef NON_MATCHING
extern int CdControlB(u8, u8 *, u8 *), VSync(int), CdStatus(void), func_800B8F34(int);
/* Best observed instruction-sequence similarity: 62.50% (40/64 words). */
s32 CdRead(s32 sectors, u32 *buffer, s32 mode) {
    volatile struct CdReadState *s = &cdReadState;
    s->readmode = mode;
    switch (s->readmode & 0x30) {
    case 0: s->sectorsize = 0x200; break;
    case 0x20: s->sectorsize = 0x249; break;
    default: s->sectorsize = 0x246; break;
    }
    s->readmode |= 0x20;
    s->buffer = buffer;
    s->count = sectors;
    s->sync = CdSyncCallback(0);
    s->ready = CdReadyCallback(0);
    if (s->mode & 1) s->data = CdDataCallback(0);
    s->start = VSync(-1);
    if (CdStatus() & 0xe0) CdControlB(9, 0, 0);
    return func_800B8F34(0) > 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdread", CdRead);
#endif

extern int VSync(int), func_800B8F34(int);

s32 CdReadSync(s32 mode, u8 *result) {
    s32 *s = &cdReadState.start;
    int remaining;
    do {
        if (VSync(-1) > s[0] + 1200) remaining = -1;
        else if (s[-2] < 0 || VSync(-1) > s[-1] + 60) {
            func_800B8F34(1);
            remaining = s[-7];
        } else remaining = s[-2];
    } while (!mode && remaining > 0);
    CdReady(1, result);
    return remaining;
}

CdlCB CdReadCallback(CdlCB cb) {
    CdlCB prev = cdReadCallback;
    cdReadCallback = cb;
    return prev;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 44.44% (4/9 words,
 * including target padding). */
s32 CdReadMode(s32 mode) {
    s32 old = cdReadState.mode;
    cdReadState.mode = mode;
    return old;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdread", CdReadMode);
#endif
