#include "psx/libcd.h"
#include "psx/libetc.h"

volatile u8 *streamCdIndexRegister __attribute__((section(".data.stream_unset_registers"))) = (volatile u8 *)0x1F801800;
volatile u8 *streamCdRegister1 __attribute__((section(".data.stream_unset_registers"))) = (volatile u8 *)0x1F801801;
volatile u8 *streamCdRegister2 __attribute__((section(".data.stream_unset_registers"))) = (volatile u8 *)0x1F801802;
volatile u8 *streamCdRequestRegister __attribute__((section(".data.stream_unset_registers"))) = (volatile u8 *)0x1F801803;

extern int EnterCriticalSection(void), ExitCriticalSection(void);
extern DmaCallback DsDataCallback(DmaCallback callback), CdDataCallback(DmaCallback callback);
extern CdlCB DsReadyCallback(CdlCB callback);
extern s32 DS_active;
void StUnSetRing(void) {
    EnterCriticalSection();
    if (DS_active == 1) {
        DsDataCallback(0);
        DsReadyCallback(0);
    } else {
        CdDataCallback(0);
        CdReadyCallback(0);
    }
    *streamCdIndexRegister = 0;
    *streamCdRequestRegister = 0;
    ExitCriticalSection();
}
