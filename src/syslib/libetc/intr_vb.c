#include "common.h"
#include "psx/libetc.h"

#define VSYNC_DATA __attribute__((section(".data.vsync_callbacks"), aligned(4)))

DmaCallback vsync_cb[8] VSYNC_DATA = {0};
s32 Vcount VSYNC_DATA = 0;
volatile u32 *gVSyncCounterMode VSYNC_DATA = (volatile u32 *)0x1F801114;

extern void VSync_memclr(s32 *, int);
extern void trapIntrVSync(void);
extern DmaCallback setIntrVSync(int, DmaCallback);
void *startIntrVSync(void) {
    *gVSyncCounterMode = 0x107;
    Vcount = 0;
    VSync_memclr((s32 *)vsync_cb, 8);
    InterruptCallback(0, trapIntrVSync);
    return (void *)setIntrVSync;
}

void trapIntrVSync(void) {
    int i = 0;
    DmaCallback *cb;
    Vcount++;
    cb = vsync_cb;
    do {
        if (*cb) (*cb)();
        i++;
        cb++;
    } while (i < 8);
}

DmaCallback setIntrVSync(int index, DmaCallback callback) {
    DmaCallback previous = vsync_cb[index];
    if (callback != previous)
        vsync_cb[index] = callback;
    return previous;
}

void VSync_memclr(s32 *p, int n) {
    int i = n - 1;
    if (n != 0) {
        do {
            *p = 0;
            i--;
            p++;
        } while (i != -1);
    }
}
