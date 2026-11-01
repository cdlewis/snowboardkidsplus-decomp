#include "common.h"
#include "psx/libcd.h"
#include "psx/libetc.h"

extern s32 StMode;
extern int CdControl(u8 command, u8 *param, u8 *result);
extern DmaCallback CdDataCallback(DmaCallback callback);
extern void data_ready_callback(void);
extern void StCdInterrupt2(u8 intr, u8 *result);

int CdRead2(s32 mode) {
    u8 param = mode;
    CdControl(0x0e, &param, 0);
    if (mode & 0x100) {
        if (mode & 0x20)
            StMode = 0;
        else
            StMode = 1;
        CdDataCallback(data_ready_callback);
        CdReadyCallback(StCdInterrupt2);
    }
    return CdControl(0x1b, 0, 0);
}

extern void StCdInterrupt(void);

void StCdInterrupt2(u8 intr, u8 *result) {
    StCdInterrupt();
}
