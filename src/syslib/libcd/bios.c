#include "common.h"
#include "psx/libcd.h"
#include "psx/libetc.h"

s32 D_80134BFC[32]
    __attribute__((section(".data.cd_command_params"), aligned(4))) = {
        0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0,
        0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

#define CD_REGISTERS __attribute__((section(".data.cd_registers"), aligned(4)))
volatile u8 *D_80134C7C CD_REGISTERS = (volatile u8 *)0x1F801800;
volatile u8 *D_80134C80 CD_REGISTERS = (volatile u8 *)0x1F801801;
volatile u8 *D_80134C84 CD_REGISTERS = (volatile u8 *)0x1F801802;
volatile u8 *D_80134C88 CD_REGISTERS = (volatile u8 *)0x1F801803;
volatile s32 *D_80134C8C CD_REGISTERS = (volatile s32 *)0x1F801020;
volatile u16 *D_80134C90 CD_REGISTERS = (volatile u16 *)0x1F801C00;

struct CdInterruptState {
    s8 state;
    u8 unk1;
    u8 unk2;
    u8 padding;
};

volatile struct CdInterruptState D_80134C94
    __attribute__((section(".data.cd_interrupt_state"), aligned(4))) = {0, 0, 0, 0};

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", func_800B6B38);

#ifdef NON_MATCHING
extern s32 D_8013D1F0, D_8013D1F4;
extern const char *D_8013D1F8;
extern const char D_80012784[], D_800126FC[], D_8001270C[];
extern char *D_801349DC[], *D_80134A5C[];
extern u8 CD_com, D_8013D1D8[], D_8013D1E0[];
extern void (*CD_cbready)(int, u8 *), (*CD_cbsync)(int, u8 *);
extern int VSync(int), func_800B6B38(void), puts(const char *), printf(const char *, ...);
extern void CD_flush(void);

/* Best observed instruction-sequence similarity: 61.88% (99/160 words;
 * GCC 2.7.2). */
int CD_sync(int mode, u8 *result) {
    int flags, bank, state, i;
    D_8013D1F0 = VSync(-1) + 0x3c0;
    D_8013D1F4 = 0;
    D_8013D1F8 = D_80012784;
    do {
        if (VSync(-1) > D_8013D1F0 || D_8013D1F4++ > 0x3c0000) {
            puts(D_800126FC);
            printf(D_8001270C, D_8013D1F8, D_801349DC[CD_com],
                   D_80134A5C[(u8)D_80134C94.state], D_80134A5C[D_80134C94.unk1]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
            bank = *D_80134C7C & 3;
            while ((flags = func_800B6B38()) != 0) {
                if ((flags & 4) && CD_cbready) CD_cbready(D_80134C94.unk1, D_8013D1E0);
                if ((flags & 2) && CD_cbsync) CD_cbsync((u8)D_80134C94.state, D_8013D1D8);
            }
            *D_80134C7C = bank;
        }
        state = (u8)D_80134C94.state;
        if (state == 2 || state == 5) {
            D_80134C94.state = 2;
            if (result) {
                u8 *p = D_8013D1D8;
                for (i = 7; i >= 0; i--) *result++ = *p++;
            }
            return state;
        }
    } while (!mode);
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_sync);
#endif

#ifdef NON_MATCHING
extern const char D_8001278C[];
extern u8 D_8013D1E8[];

/* Best observed instruction-sequence similarity: 61.80% (110/178 words;
 * GCC 2.7.2). */
int CD_ready(int mode, u8 *result) {
    int flags, bank, state, i;
    D_8013D1F0 = VSync(-1) + 0x3c0;
    D_8013D1F4 = 0;
    D_8013D1F8 = D_8001278C;
    do {
        if (VSync(-1) > D_8013D1F0 || D_8013D1F4++ > 0x3c0000) {
            puts(D_800126FC);
            printf(D_8001270C, D_8013D1F8, D_801349DC[CD_com],
                   D_80134A5C[(u8)D_80134C94.state], D_80134A5C[D_80134C94.unk1]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
            bank = *D_80134C7C & 3;
            while ((flags = func_800B6B38()) != 0) {
                if ((flags & 4) && CD_cbready) CD_cbready(D_80134C94.unk1, D_8013D1E0);
                if ((flags & 2) && CD_cbsync) CD_cbsync((u8)D_80134C94.state, D_8013D1D8);
            }
            *D_80134C7C = bank;
        }
        state = D_80134C94.unk2;
        if (state) {
            D_80134C94.unk2 = 0;
            if (result) {
                u8 *p = D_8013D1E8;
                for (i = 7; i >= 0; i--) *result++ = *p++;
            }
            return state;
        }
        state = D_80134C94.unk1;
        if (state) {
            D_80134C94.unk1 = 0;
            if (result) {
                u8 *p = D_8013D1E0;
                for (i = 7; i >= 0; i--) *result++ = *p++;
            }
            return state;
        }
    } while (!mode);
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_ready);
#endif

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_cw);

s32 CD_vol(CdlATV *vol) {
    *D_80134C7C = 2;
    *D_80134C84 = vol->val0;
    *D_80134C88 = vol->val1;
    *D_80134C7C = 3;
    *D_80134C80 = vol->val2;
    *D_80134C84 = vol->val3;
    *D_80134C88 = 0x20;
    return 0;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 85.45% (47/55 words). */
void CD_flush(void) {
    *D_80134C7C = 1;
    while (*D_80134C88 & 7) {
        *D_80134C7C = 1;
        *D_80134C88 = 7;
        *D_80134C84 = 7;
    }
    D_80134C94.unk2 = 0;
    D_80134C94.unk1 = D_80134C94.unk2;
    D_80134C94.state = 2;
    *D_80134C7C = 0;
    *D_80134C88 = 0;
    *D_80134C8C = 0x1325;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_flush);
#endif

s32 CD_initvol(void) {
    CdlATV vol;

    if (D_80134C90[0x1B8 / 2] == 0) {
        if (D_80134C90[0x1BA / 2] == 0) {
            D_80134C90[0x180 / 2] = 0x3FFF;
            D_80134C90[0x182 / 2] = 0x3FFF;
        }
    }
    D_80134C90[0x1B0 / 2] = 0x3FFF;
    D_80134C90[0x1B2 / 2] = 0x3FFF;
    D_80134C90[0x1AA / 2] = 0xC001;
    vol.val2 = 128;
    vol.val0 = 128;
    vol.val3 = 0;
    vol.val1 = 0;
    *D_80134C7C = 2;
    *D_80134C84 = vol.val0;
    *D_80134C88 = vol.val1;
    *D_80134C7C = 3;
    *D_80134C80 = vol.val2;
    *D_80134C84 = vol.val3;
    *D_80134C88 = 0x20;
    return 0;
}

extern void (*CD_cbready)(int, u8 *), (*CD_cbsync)(int, u8 *), (*D_801349C8)(void);
extern u8 CD_status;
extern void func_800B81C0(void);

void CD_initintr(void) {
    CD_cbready = 0;
    CD_cbsync = 0;
    D_801349C8 = 0;
    *(u32 *)&CD_status = 0;
    ResetCallback();
    InterruptCallback(2, func_800B81C0);
}

#ifdef NON_MATCHING
extern char D_800127EC[], D_800127F8[], D_80134C98[];
extern u8 CD_com, CD_mode;
extern int puts(const char *), printf(const char *, ...);
extern int CD_cw(int, u8 *, u8 *, int), CD_sync(int, u8 *);
/* Best observed instruction-sequence similarity: 87.50% (105/120 words;
 * GCC 2.7.2). */
int CD_init(void) {
    puts(D_800127EC);
    printf(D_800127F8, D_80134C98);
    CD_com = 0; CD_mode = 0;
    CD_cbready = 0; CD_cbsync = 0; D_801349C8 = 0;
    *(u32 *)&CD_status = 0;
    ResetCallback();
    InterruptCallback(2, func_800B81C0);
    *D_80134C7C = 1;
    while (*D_80134C88 & 7) {
        *D_80134C7C = 1; *D_80134C88 = 7; *D_80134C84 = 7;
    }
    D_80134C94.unk2 = 0;
    D_80134C94.unk1 = D_80134C94.unk2;
    D_80134C94.state = 2;
    *D_80134C7C = 0; *D_80134C88 = 0; *D_80134C8C = 0x1325;
    CD_cw(1, 0, 0, 0);
    if (*(u32 *)&CD_status & 0x10) CD_cw(1, 0, 0, 0);
    if (CD_cw(10, 0, 0, 0)) return -1;
    if (CD_cw(12, 0, 0, 0)) return -1;
    if (CD_sync(0, 0) != 2) return -1;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_init);
#endif

#ifdef NON_MATCHING
extern volatile u32 *D_80134CC0;
extern u8 CD_com;
extern int D_8013D1F0, D_8013D1F4;
extern const char *D_8013D1F8;
extern const char D_80012804[], D_800126FC[], D_8001270C[];
extern char *D_801349DC[], *D_80134A5C[];
extern int VSync(int), puts(const char *), printf(const char *, ...);
/* Best observed instruction-sequence similarity: 61.11% (55/90 words;
 * GCC 2.7.2). */
int CD_datasync(int mode) {
    D_8013D1F0 = VSync(-1) + 0x3c0;
    D_8013D1F4 = 0;
    D_8013D1F8 = D_80012804;
    for (;;) {
        if (VSync(-1) > D_8013D1F0 || D_8013D1F4++ > 0x3c0000) {
            puts(D_800126FC);
            printf(D_8001270C, D_8013D1F8, D_801349DC[CD_com],
                   D_80134A5C[(u8)D_80134C94.state], D_80134A5C[D_80134C94.unk1]);
            CD_flush();
            return -1;
        }
        if (!(*D_80134CC0 & 0x1000000)) return 0;
        if (mode) return 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_datasync);
#endif

#ifdef NON_MATCHING
extern volatile u32 *D_80134CB0, *D_80134CB4, *D_80134CB8, *D_80134CBC, *D_80134CC0;
/* Best observed instruction-sequence similarity: 98.44% (63/64 words).
 * The readiness-loop branch targets the preceding load-delay nop. */
int CD_getsector(void *address, int size) {
    *D_80134C7C = 0;
    *D_80134C88 = 0x80;
    *D_80134CB0 = 0x20943;
    *D_80134C8C = 0x1323;
    *D_80134CB4 |= 0x8000;
    *D_80134CB8 = (u32)address;
    *D_80134CBC = size | 0x10000;
    while (!(*D_80134C7C & 0x40)) {}
    *D_80134CC0 = 0x11000000;
    while (*D_80134CC0 & 0x1000000) {}
    *D_80134C8C = 0x1325;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_getsector);
#endif

extern volatile u32 *D_80134CB0, *D_80134CB4, *D_80134CB8, *D_80134CBC, *D_80134CC0;

int CD_getsector2(void *address, int size) {
    volatile u32 dummy;
    *D_80134C7C = 0;
    *D_80134C88 = 0x80;
    *D_80134CB0 = 0x21020843;
    *D_80134C8C = 0x1325;
    *D_80134CB4 |= 0x8000;
    *D_80134CB8 = (u32)address;
    *D_80134CBC = size | 0x10000;
    while (!(*D_80134C7C & 0x40)) {}
    *D_80134CC0 = 0x11400100;
    dummy = *D_80134CC0;
    return 0;
}

void CD_set_test_parmnum(int count) { D_80134BFC[25] = count; }

extern u8 D_8013D1D8[], D_8013D1E0[];
extern int func_800B6B38(void);

void func_800B81C0(void) {
    int flags;
    int bank = *D_80134C7C & 3;
    while ((flags = func_800B6B38()) != 0) {
        if ((flags & 4) && CD_cbready) CD_cbready(D_80134C94.unk1, D_8013D1E0);
        if ((flags & 2) && CD_cbsync) CD_cbsync((u8)D_80134C94.state, D_8013D1D8);
    }
    *D_80134C7C = bank;
}
