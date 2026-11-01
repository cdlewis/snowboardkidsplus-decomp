#include "psx/libcd.h"
#include "psx/libetc.h"
extern int EnterCriticalSection(void), ExitCriticalSection(void);
extern DmaCallback DsDataCallback(DmaCallback callback), CdDataCallback(DmaCallback callback);
extern CdlCB DsReadyCallback(CdlCB callback);
extern s32 DS_active;
extern volatile u8 *D_80134D2C, *D_80134D38;
void StUnSetRing(void) {
    EnterCriticalSection();
    if (DS_active == 1) {
        DsDataCallback(0);
        DsReadyCallback(0);
    } else {
        CdDataCallback(0);
        CdReadyCallback(0);
    }
    *D_80134D2C = 0;
    *D_80134D38 = 0;
    ExitCriticalSection();
}
