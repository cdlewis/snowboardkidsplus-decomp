#include "common.h"
#include "psx/libcd.h"
#include "psx/libetc.h"

s32 cdCommandParameterCount[32]
    __attribute__((section(".data.cd_command_params"), aligned(4))) = {
        0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0,
        0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

#define CD_REGISTERS __attribute__((section(".data.cd_registers"), aligned(4)))
volatile u8 *cdIndexRegister CD_REGISTERS = (volatile u8 *)0x1F801800;
volatile u8 *cdRegister1 CD_REGISTERS = (volatile u8 *)0x1F801801;
volatile u8 *cdRegister2 CD_REGISTERS = (volatile u8 *)0x1F801802;
volatile u8 *cdRegister3 CD_REGISTERS = (volatile u8 *)0x1F801803;
volatile s32 *cdCommonDelayRegister CD_REGISTERS = (volatile s32 *)0x1F801020;
volatile u16 *cdSpuRegisters CD_REGISTERS = (volatile u16 *)0x1F801C00;

struct CdInterruptState {
    s8 state;
    u8 unk1;
    u8 unk2;
    u8 padding;
};

volatile struct CdInterruptState cdInterruptState
    __attribute__((section(".data.cd_interrupt_state"), aligned(4))) = {0, 0, 0, 0};

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", func_800B6B38);

#ifdef NON_MATCHING
extern s32 cdAlarmDeadline, cdAlarmPollCount;
extern const char *cdAlarmOperationName;
extern const char cdSyncOperationName[], cdTimeoutMessage[], cdTimeoutStateFormat[];
extern char *CD_comstr[], *CD_intstr[];
extern u8 CD_com, cdSyncResult[], cdReadyResult[];
extern void (*CD_cbready)(int, u8 *), (*CD_cbsync)(int, u8 *);
extern int VSync(int), func_800B6B38(void), puts(const char *), printf(const char *, ...);
extern void CD_flush(void);

/* Best observed instruction-sequence similarity: 61.88% (99/160 words;
 * GCC 2.7.2). */
int CD_sync(int mode, u8 *result) {
    int flags, bank, state, i;
    cdAlarmDeadline = VSync(-1) + 0x3c0;
    cdAlarmPollCount = 0;
    cdAlarmOperationName = cdSyncOperationName;
    do {
        if (VSync(-1) > cdAlarmDeadline || cdAlarmPollCount++ > 0x3c0000) {
            puts(cdTimeoutMessage);
            printf(cdTimeoutStateFormat, cdAlarmOperationName, CD_comstr[CD_com],
                   CD_intstr[(u8)cdInterruptState.state], CD_intstr[cdInterruptState.unk1]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
            bank = *cdIndexRegister & 3;
            while ((flags = func_800B6B38()) != 0) {
                if ((flags & 4) && CD_cbready) CD_cbready(cdInterruptState.unk1, cdReadyResult);
                if ((flags & 2) && CD_cbsync) CD_cbsync((u8)cdInterruptState.state, cdSyncResult);
            }
            *cdIndexRegister = bank;
        }
        state = (u8)cdInterruptState.state;
        if (state == 2 || state == 5) {
            cdInterruptState.state = 2;
            if (result) {
                u8 *p = cdSyncResult;
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
extern const char cdReadyOperationName[];
extern u8 cdAsyncResult[];

/* Best observed instruction-sequence similarity: 61.80% (110/178 words;
 * GCC 2.7.2). */
int CD_ready(int mode, u8 *result) {
    int flags, bank, state, i;
    cdAlarmDeadline = VSync(-1) + 0x3c0;
    cdAlarmPollCount = 0;
    cdAlarmOperationName = cdReadyOperationName;
    do {
        if (VSync(-1) > cdAlarmDeadline || cdAlarmPollCount++ > 0x3c0000) {
            puts(cdTimeoutMessage);
            printf(cdTimeoutStateFormat, cdAlarmOperationName, CD_comstr[CD_com],
                   CD_intstr[(u8)cdInterruptState.state], CD_intstr[cdInterruptState.unk1]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
            bank = *cdIndexRegister & 3;
            while ((flags = func_800B6B38()) != 0) {
                if ((flags & 4) && CD_cbready) CD_cbready(cdInterruptState.unk1, cdReadyResult);
                if ((flags & 2) && CD_cbsync) CD_cbsync((u8)cdInterruptState.state, cdSyncResult);
            }
            *cdIndexRegister = bank;
        }
        state = cdInterruptState.unk2;
        if (state) {
            cdInterruptState.unk2 = 0;
            if (result) {
                u8 *p = cdAsyncResult;
                for (i = 7; i >= 0; i--) *result++ = *p++;
            }
            return state;
        }
        state = cdInterruptState.unk1;
        if (state) {
            cdInterruptState.unk1 = 0;
            if (result) {
                u8 *p = cdReadyResult;
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
    *cdIndexRegister = 2;
    *cdRegister2 = vol->val0;
    *cdRegister3 = vol->val1;
    *cdIndexRegister = 3;
    *cdRegister1 = vol->val2;
    *cdRegister2 = vol->val3;
    *cdRegister3 = 0x20;
    return 0;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 85.45% (47/55 words). */
void CD_flush(void) {
    *cdIndexRegister = 1;
    while (*cdRegister3 & 7) {
        *cdIndexRegister = 1;
        *cdRegister3 = 7;
        *cdRegister2 = 7;
    }
    cdInterruptState.unk2 = 0;
    cdInterruptState.unk1 = cdInterruptState.unk2;
    cdInterruptState.state = 2;
    *cdIndexRegister = 0;
    *cdRegister3 = 0;
    *cdCommonDelayRegister = 0x1325;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_flush);
#endif

s32 CD_initvol(void) {
    CdlATV vol;

    if (cdSpuRegisters[0x1B8 / 2] == 0) {
        if (cdSpuRegisters[0x1BA / 2] == 0) {
            cdSpuRegisters[0x180 / 2] = 0x3FFF;
            cdSpuRegisters[0x182 / 2] = 0x3FFF;
        }
    }
    cdSpuRegisters[0x1B0 / 2] = 0x3FFF;
    cdSpuRegisters[0x1B2 / 2] = 0x3FFF;
    cdSpuRegisters[0x1AA / 2] = 0xC001;
    vol.val2 = 128;
    vol.val0 = 128;
    vol.val3 = 0;
    vol.val1 = 0;
    *cdIndexRegister = 2;
    *cdRegister2 = vol.val0;
    *cdRegister3 = vol.val1;
    *cdIndexRegister = 3;
    *cdRegister1 = vol.val2;
    *cdRegister2 = vol.val3;
    *cdRegister3 = 0x20;
    return 0;
}

extern void (*CD_cbready)(int, u8 *), (*CD_cbsync)(int, u8 *), (*CD_status1)(void);
extern u8 CD_status;
extern void func_800B81C0(void);

void CD_initintr(void) {
    CD_cbready = 0;
    CD_cbsync = 0;
    CD_status1 = 0;
    *(u32 *)&CD_status = 0;
    ResetCallback();
    InterruptCallback(2, func_800B81C0);
}

#ifdef NON_MATCHING
extern char cdInitMessage[], cdInitAddressFormat[], cdInitInfo[];
extern u8 CD_com, CD_mode;
extern int puts(const char *), printf(const char *, ...);
extern int CD_cw(int, u8 *, u8 *, int), CD_sync(int, u8 *);
/* Best observed instruction-sequence similarity: 87.50% (105/120 words;
 * GCC 2.7.2). */
int CD_init(void) {
    puts(cdInitMessage);
    printf(cdInitAddressFormat, cdInitInfo);
    CD_com = 0; CD_mode = 0;
    CD_cbready = 0; CD_cbsync = 0; CD_status1 = 0;
    *(u32 *)&CD_status = 0;
    ResetCallback();
    InterruptCallback(2, func_800B81C0);
    *cdIndexRegister = 1;
    while (*cdRegister3 & 7) {
        *cdIndexRegister = 1; *cdRegister3 = 7; *cdRegister2 = 7;
    }
    cdInterruptState.unk2 = 0;
    cdInterruptState.unk1 = cdInterruptState.unk2;
    cdInterruptState.state = 2;
    *cdIndexRegister = 0; *cdRegister3 = 0; *cdCommonDelayRegister = 0x1325;
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
extern volatile u32 *cdDmaControlRegister;
extern u8 CD_com;
extern int cdAlarmDeadline, cdAlarmPollCount;
extern const char *cdAlarmOperationName;
extern const char cdDataSyncOperationName[], cdTimeoutMessage[], cdTimeoutStateFormat[];
extern char *CD_comstr[], *CD_intstr[];
extern int VSync(int), puts(const char *), printf(const char *, ...);
/* Best observed instruction-sequence similarity: 61.11% (55/90 words;
 * GCC 2.7.2). */
int CD_datasync(int mode) {
    cdAlarmDeadline = VSync(-1) + 0x3c0;
    cdAlarmPollCount = 0;
    cdAlarmOperationName = cdDataSyncOperationName;
    for (;;) {
        if (VSync(-1) > cdAlarmDeadline || cdAlarmPollCount++ > 0x3c0000) {
            puts(cdTimeoutMessage);
            printf(cdTimeoutStateFormat, cdAlarmOperationName, CD_comstr[CD_com],
                   CD_intstr[(u8)cdInterruptState.state], CD_intstr[cdInterruptState.unk1]);
            CD_flush();
            return -1;
        }
        if (!(*cdDmaControlRegister & 0x1000000)) return 0;
        if (mode) return 1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_datasync);
#endif

#ifdef NON_MATCHING
extern volatile u32 *cdBusDelayRegister, *cdDmaPriorityRegister, *cdDmaAddressRegister, *cdDmaBlockRegister, *cdDmaControlRegister;
/* Best observed instruction-sequence similarity: 98.44% (63/64 words).
 * The readiness-loop branch targets the preceding load-delay nop. */
int CD_getsector(void *address, int size) {
    *cdIndexRegister = 0;
    *cdRegister3 = 0x80;
    *cdBusDelayRegister = 0x20943;
    *cdCommonDelayRegister = 0x1323;
    *cdDmaPriorityRegister |= 0x8000;
    *cdDmaAddressRegister = (u32)address;
    *cdDmaBlockRegister = size | 0x10000;
    while (!(*cdIndexRegister & 0x40)) {}
    *cdDmaControlRegister = 0x11000000;
    while (*cdDmaControlRegister & 0x1000000) {}
    *cdCommonDelayRegister = 0x1325;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/bios", CD_getsector);
#endif

extern volatile u32 *cdBusDelayRegister, *cdDmaPriorityRegister, *cdDmaAddressRegister, *cdDmaBlockRegister, *cdDmaControlRegister;

int CD_getsector2(void *address, int size) {
    volatile u32 dummy;
    *cdIndexRegister = 0;
    *cdRegister3 = 0x80;
    *cdBusDelayRegister = 0x21020843;
    *cdCommonDelayRegister = 0x1325;
    *cdDmaPriorityRegister |= 0x8000;
    *cdDmaAddressRegister = (u32)address;
    *cdDmaBlockRegister = size | 0x10000;
    while (!(*cdIndexRegister & 0x40)) {}
    *cdDmaControlRegister = 0x11400100;
    dummy = *cdDmaControlRegister;
    return 0;
}

void CD_set_test_parmnum(int count) { cdCommandParameterCount[25] = count; }

extern u8 cdSyncResult[], cdReadyResult[];
extern int func_800B6B38(void);

void func_800B81C0(void) {
    int flags;
    int bank = *cdIndexRegister & 3;
    while ((flags = func_800B6B38()) != 0) {
        if ((flags & 4) && CD_cbready) CD_cbready(cdInterruptState.unk1, cdReadyResult);
        if ((flags & 2) && CD_cbsync) CD_cbsync((u8)cdInterruptState.state, cdSyncResult);
    }
    *cdIndexRegister = bank;
}
