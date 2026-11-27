#include "common.h"

#define VSYNC_DATA __attribute__((section(".data.vsync"), aligned(4)))

const char vsyncTimeoutMessage[16] __attribute__((aligned(8))) = "VSync: timeout\n";

/* The SDK object retains these unreferenced words before its register pointers. */
static u32 vsyncDataPrefix[2] VSYNC_DATA = {0x21067350, 0x004238AD};
volatile u32 *vsyncGpuStatusRegister VSYNC_DATA = (volatile u32 *)0x1F801814;
volatile u32 *vsyncCounter1Register VSYNC_DATA = (volatile u32 *)0x1F801110;
volatile int Hcount VSYNC_DATA = 0;
int prevVcount VSYNC_DATA = 0;
static u32 vsyncDataPadding[2] VSYNC_DATA = {0, 0};
extern volatile int Vcount;
extern int puts(const char *);
extern void ChangeClearPAD(int), ChangeClearRCnt(int, int);
extern void func_800B5730(int, int);

int VSync(int mode) {
    u32 status = *vsyncGpuStatusRegister;
    volatile int timer;
    int elapsed;

    do { timer = *vsyncCounter1Register; } while (timer != *vsyncCounter1Register);
    elapsed = (timer - Hcount) & 0xffff;
    if (mode < 0) return Vcount;
    if (mode != 1) {
        func_800B5730(prevVcount + (mode > 0 ? mode - 1 : 0),
                      mode > 0 ? mode - 1 : 0);
        status = *vsyncGpuStatusRegister;
        func_800B5730(Vcount + 1, 1);
        if (status & 0x400000) {
            while (!((status ^ *vsyncGpuStatusRegister) & 0x80000000)) { }
        }
        prevVcount = Vcount;
        do { Hcount = *vsyncCounter1Register; } while (Hcount != *vsyncCounter1Register);
    }
    return elapsed;
}

void func_800B5730(int target, int timeout) {
    volatile int count = timeout << 15;
    while (Vcount < target) {
        if (count-- == 0) {
            puts(vsyncTimeoutMessage);
            ChangeClearPAD(0);
            ChangeClearRCnt(3, 0);
            break;
        }
    }
}
