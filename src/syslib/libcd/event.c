#include "common.h"
#include "psx/libcd.h"

extern int DeliverEvent(u32 event, u32 spec);

const char sCdInitFailed[32] __attribute__((section(".rodata.cdinit"), aligned(4))) =
    "CdInit: Init failed\n";

extern int printf(const char *, ...);
extern CdlCB CdReadCallback(CdlCB callback);
extern int CdReadMode(int mode);
extern void def_cbsync(void), def_cbready(void), def_cbread(void);

int CdInit(void) {
    int retries = 4;
    while (CdReset(1) != 1) {
        if (!retries--) {
            printf(sCdInitFailed);
            return 0;
        }
    }
    CdSyncCallback((CdlCB)def_cbsync);
    CdReadyCallback((CdlCB)def_cbready);
    CdReadCallback((CdlCB)def_cbread);
    CdReadMode(0);
    return 1;
}

void def_cbsync(void) {
    DeliverEvent(0xF0000003, 0x20);
}

void def_cbready(void) {
    DeliverEvent(0xF0000003, 0x40);
}

void def_cbread(void) {
    DeliverEvent(0xF0000003, 0x40);
}
