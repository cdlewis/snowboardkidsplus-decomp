#include "common.h"

extern volatile u32 *D_80133794, *D_80133798;
extern volatile int D_8013379C;
extern int D_801337A0;
extern volatile int Vcount;
extern const char D_800124E0[];
extern int puts(const char *);
extern void ChangeClearPAD(int), ChangeClearRCnt(int, int);
extern void func_800B5730(int, int);

int VSync(int mode) {
    u32 status = *D_80133794;
    volatile int timer;
    int elapsed;

    do { timer = *D_80133798; } while (timer != *D_80133798);
    elapsed = (timer - D_8013379C) & 0xffff;
    if (mode < 0) return Vcount;
    if (mode != 1) {
        func_800B5730(D_801337A0 + (mode > 0 ? mode - 1 : 0),
                      mode > 0 ? mode - 1 : 0);
        status = *D_80133794;
        func_800B5730(Vcount + 1, 1);
        if (status & 0x400000) {
            while (!((status ^ *D_80133794) & 0x80000000)) { }
        }
        D_801337A0 = Vcount;
        do { D_8013379C = *D_80133798; } while (D_8013379C != *D_80133798);
    }
    return elapsed;
}

void func_800B5730(int target, int timeout) {
    volatile int count = timeout << 15;
    while (Vcount < target) {
        if (count-- == 0) {
            puts(D_800124E0);
            ChangeClearPAD(0);
            ChangeClearRCnt(3, 0);
            break;
        }
    }
}
