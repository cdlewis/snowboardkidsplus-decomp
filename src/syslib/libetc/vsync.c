#include "common.h"

extern volatile u32 *vsyncGpuStatusRegister, *vsyncCounter1Register;
extern volatile int Hcount;
extern int prevVcount;
extern volatile int Vcount;
extern const char vsyncTimeoutMessage[];
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
