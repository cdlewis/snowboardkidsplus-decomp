#include "common.h"
#include "psx/libcd.h"
#include "game/cd_queue.h"
#include "psx/libsnd.h"
#include "game/controller.h"
#include "game/task.h"
#include "game/controller_effects.h"

typedef struct ControllerInputRecord {
    u8 repeatCount;
    u8 horizontal;
    u8 vertical;
    u8 unk_03;
    u16 buttons;
    u16 pressedButtons;
} ControllerInputRecord;

typedef struct CdRequest {
    s32 unk_00;
    void *destination;
    char *filename;
    s32 sector;
    s32 sectorCount;
    s16 command;
    s16 unk_16;
    s32 unk_18;
} CdRequest;

CdRequest gCdRequests[64] __attribute__((section(".data.cd_requests"))) = {0};

u16 gCdRequestPhase = 0;
u16 gCdRequestWriteIndex = 0;
u16 gCdRequestReadIndex = 0;
u16 D_8013AE76 = 0;
typedef struct ControllerReceiveBuffer {
    u8 status;
    u8 deviceId;
    u8 payload[32];
} ControllerReceiveBuffer;

extern ControllerReceiveBuffer D_800E9558[2];
extern ControllerInputRecord *D_8013AED4[];
extern u16 D_8013AEDC[];
extern u8 D_8013AEE0[];


u8 gCdVolumeFadeActive __attribute__((section(".sdata.cd_volume_fade"))) = 0;
u8 gCdVolumeFadeInterval __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeCurrent __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeTarget __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeStep __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeTimer __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;
static u16 sCdVolumeFadeTimerPadding __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;

u16 gTaskResourceFreeBits[16] __attribute__((section(".data.task_resource_bitmap"))) = {0};

u8 gControllerFlags[4] __attribute__((section(".data"))) = {0, 0, 0, 0};

u8 gControllerHorizontalRepeatTimers[2] __attribute__((section(".sdata.controller_repeat_timing"))) = {0, 0};
u8 gControllerRepeatInitialDelay __attribute__((section(".sdata.controller_repeat_timing"))) = 12;
u8 gControllerRepeatInterval __attribute__((section(".sdata.controller_repeat_timing"))) = 2;

s16 D_8013AF08[2] __attribute__((section(".data.controller_values"))) = {0, 0};

u16 D_8013AE48 __attribute__((section(".sdata.cd_command_state"))) = 0;
s16 D_8013AE4A __attribute__((section(".sdata.cd_command_state"))) = 0;
s16 D_8013AE4C __attribute__((section(".sdata.cd_command_state"))) = 0;
u16 D_8013AE4E __attribute__((section(".sdata.cd_command_state"))) = 0;
u16 D_8013AE50 __attribute__((section(".sdata.cd_command_state"))) = 0;
static u16 sCdCommandStatePadding __attribute__((section(".sdata.cd_command_state"))) = 0;
s32 D_8013AE54 __attribute__((section(".sdata.cd_command_state"))) = 0;
s32 D_8013AE58 __attribute__((section(".sdata.cd_command_state"))) = 0;
s32 D_8013AE5C __attribute__((section(".sdata.cd_command_state"))) = 0;
s32 D_8013AE60 __attribute__((section(".sdata.cd_command_state"))) = 0;
s32 D_8013AE64 __attribute__((section(".sdata.cd_command_state"))) = 0;
s16 D_8013AE78 __attribute__((section(".sdata.cd_command_active"))) = 0;
static u16 sCdCommandActivePadding __attribute__((section(".sdata.cd_command_active"))) = 0;

void func_800131B4(void) {
}

INCLUDE_ASM("asm/nonmatchings/main", func_800131BC);

void resetCdVolumeFade(void) {
    gCdVolumeFadeInterval = 0;
    gCdVolumeFadeActive = 0;
    gCdVolumeFadeCurrent = 0;
    gCdVolumeFadeTarget = 0;
    gCdVolumeFadeStep = 0;
}

void startCdVolumeFade(s16 startVolume, s16 step, s16 targetVolume, u8 interval) {
    gCdVolumeFadeTimer = 0;
    gCdVolumeFadeActive = 1;
    gCdVolumeFadeInterval = interval;
    gCdVolumeFadeCurrent = startVolume;
    gCdVolumeFadeTarget = targetVolume;
    gCdVolumeFadeStep = step;
    SsSetSerialVol(0, startVolume, startVolume);
    if (startVolume == targetVolume) {
        gCdVolumeFadeActive = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_8001330C);

INCLUDE_ASM("asm/nonmatchings/main", func_800133EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800134E4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001351C);

INCLUDE_ASM("asm/nonmatchings/main", func_80013540);

s32 queueCdSectorCommand(s16 command, s16 sector, s16 count, void *destination, s32 value) {
    CdRequest *entry;
    if (gCdVolumeFadeActive) {
        resetPendingCdRequests();
        gCdVolumeFadeActive = 0;
    }
    entry = &gCdRequests[gCdRequestWriteIndex & 0x3F];
    entry->unk_16 = 1;
    entry->command = command;
    entry->sector = sector;
    entry->sectorCount = count;
    entry->destination = destination;
    gCdRequestWriteIndex &= 0x3F;
    entry->unk_18 = value;
    gCdRequestWriteIndex++;
    return 1;
}

s32 isCdReading(void) {
    u8 result[8];

    if (D_8013AE78) {
        CdControlB(CdlNop, 0, result);
        if (result[0] & CdlStatRead) {
            return 1;
        }
    }
    return 0;
}

s32 isCdCommandQueueIdle(void) {
    if (D_8013AE78 == 0 || D_8013AE4C == -1) {
        if (gCdRequestPhase == 0) {
            gCdRequestReadIndex &= 0x3F;
            gCdRequestWriteIndex &= 0x3F;
            if (gCdRequestReadIndex == gCdRequestWriteIndex) {
                return 1;
            }
        }
    }
    return 0;
}

// Ready when empty, or when the current search request reaches phase 7.
s32 isCdRequestQueueReady(void) {
    s32 result;
    u16 index, end;
    index = gCdRequestReadIndex & 63;
    end = gCdRequestWriteIndex & 63;
    gCdRequestReadIndex = index;
    gCdRequestWriteIndex = end;
    if (index == end) {
        result = 1;
    } else if (gCdRequests[index].command == 15) {
        result = gCdRequestPhase >= 7;
    } else {
        result = 0;
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/main", func_800138AC);

INCLUDE_ASM("asm/nonmatchings/main", func_80013DD0);

void resetCdCommandState(void) {
    D_8013AE78 = 0;
    D_8013AE50 = 0;
    D_8013AE4E = 0;
    D_8013AE48 = 0;
    D_8013AE64 = 0;
    D_8013AE60 = 0;
    D_8013AE5C = 0;
    D_8013AE58 = 0;
    D_8013AE54 = 0;
    D_8013AE4C = 0;
    D_8013AE4A = 0;
}

void resetPendingCdRequests(void) {
    if (D_8013AE78 != 0) {
        D_8013AE50 &= 7;
        D_8013AE4E &= 7;
        if (D_8013AE50 != D_8013AE4E) {
            D_8013AE50 = 0;
            D_8013AE4E = 0;
        }
        func_80013DD0(-1, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_80013F28);

INCLUDE_ASM("asm/nonmatchings/main", func_80013F98);

INCLUDE_ASM("asm/nonmatchings/main", func_8001467C);

INCLUDE_ASM("asm/nonmatchings/main", func_800147B8);

INCLUDE_ASM("asm/nonmatchings/main", func_80014838);

INCLUDE_ASM("asm/nonmatchings/main", func_80014874);

INCLUDE_ASM("asm/nonmatchings/main", func_800148B4);

INCLUDE_ASM("asm/nonmatchings/main", func_800148F4);

INCLUDE_ASM("asm/nonmatchings/main", func_80014930);

INCLUDE_ASM("asm/nonmatchings/main", func_8001496C);

INCLUDE_ASM("asm/nonmatchings/main", func_800149A8);

INCLUDE_ASM("asm/nonmatchings/main", func_800149E0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014A2C);

INCLUDE_ASM("asm/nonmatchings/main", func_80014B14);

void setCurrentTaskCallback(TaskCallback callback) {
    D_8013AE88->callback = callback;
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014B58);

void deleteCurrentTask(void) {
    func_800149A8();
}

u32 allocateTaskResource(void) {
    u16 i, bit;
    u32 mask;
    for (i = 0; i < 16; i++) {
        if (gTaskResourceFreeBits[i]) {
            for (mask = 1, bit = 0; bit < 16; bit++) {
                if (gTaskResourceFreeBits[i] & mask) {
                    gTaskResourceFreeBits[i] ^= mask;
                    return i * 16 + bit;
                }
                mask = (u16)mask << 1;
            }
        }
    }
    return 0;
}

void releaseTaskResource(u16 resource) {
    u32 index = resource >> 4;
    resource %= 16;
    gTaskResourceFreeBits[index] |= 1 << resource;
}

void setControllerFlag0(u16 controller) {
    gControllerFlags[controller] |= 1;
}

void clearControllerFlag0(u16 controller) {
    u8 *flags = controller + gControllerFlags;
    *flags &= ~1;
}

void setControllerRepeatTiming(u8 initialDelay, u8 repeatInterval) {
    gControllerRepeatInitialDelay = initialDelay;
    gControllerRepeatInterval = repeatInterval;
}

void setControllerFlag5(u16 controller) {
    gControllerFlags[controller] |= 0x20;
    D_8013AF08[controller] = -1;
}

void clearControllerFlag5(u16 controller) {
    u8 *flags = controller + gControllerFlags;
    *flags &= ~0x20;
    D_8013AF08[controller] = -1;
}

s32 isControllerFlag5Clear(u16 controller) {
    s32 flags = gControllerFlags[controller];
    flags &= 0x20;
    if (flags) {
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014D60);

void setControllerAxesFromButtons(u16 controller) {
    if (D_8013AEEC[controller] & 0x2000) {
        D_8013AEF8[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x8000) {
        D_8013AEF8[controller] = -31;
    } else {
        D_8013AEF8[controller] = 0;
    }
    if (D_8013AEEC[controller] & 0x4000) {
        D_8013AEFC[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x1000) {
        D_8013AEFC[controller] = -31;
    } else {
        D_8013AEFC[controller] = 0;
    }
}

void updateControllerAxesFromButtons(u16 controller) {
    if (D_8013AEEC[controller] & 0x2000) {
        D_8013AEF8[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x8000) {
        D_8013AEF8[controller] = -31;
    }
    if (D_8013AEEC[controller] & 0x4000) {
        D_8013AEFC[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x1000) {
        D_8013AEFC[controller] = -31;
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_80015154);

INCLUDE_ASM("asm/nonmatchings/main", func_800153EC);

s16 isControllerInputSupported(u16 controller) {
    ControllerReceiveBuffer *buffer = D_800E9558;
    ControllerReceiveBuffer *device;
    buffer += controller;
    device = buffer;
    if (buffer->status != 0) {
        return 0;
    }
    switch (device->deviceId) {
    case 0x41:
    case 0x53:
    case 0x73:
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/main", func_8001553C);

INCLUDE_ASM("asm/nonmatchings/main", func_800157DC);

void updateControllers(void) {
    func_8001553C(0);
    func_80014D60(0);
    func_800157DC(0);
    func_8001553C(1);
    func_80014D60(1);
    func_800157DC(1);
}

INCLUDE_ASM("asm/nonmatchings/main", func_8001596C);

INCLUDE_ASM("asm/nonmatchings/main", func_80015A30);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C94);

void playbackControllerInput(u16 controller) {
    u8 *counter;
    D_8013AEEC[controller] = D_8013AED4[controller][D_8013AEDC[controller]].buttons;
    D_8013AEE8[controller] = D_8013AED4[controller][D_8013AEDC[controller]].pressedButtons;
    D_8013AEF8[controller] = (s8)D_8013AED4[controller][D_8013AEDC[controller]].horizontal;
    D_8013AEFC[controller] = (s8)D_8013AED4[controller][D_8013AEDC[controller]].vertical;
    counter = &D_8013AEE0[controller];
    *counter = *counter ? *counter - 1 :
        (D_8013AEDC[controller]++,
         D_8013AED4[controller][D_8013AEDC[controller]].repeatCount);
}
