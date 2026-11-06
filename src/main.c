#include "common.h"
#include "psx/libcd.h"
#include "psx/libpad.h"
#include "game/cd_queue.h"
#include "psx/libsnd.h"
#include "game/controller.h"
#include "game/task.h"
#include "game/game_options.h"
#include "game/controller_effects.h"

typedef struct CdCommand {
    s16 command;
    s16 value;
} CdCommand;

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

typedef struct ControllerReceiveBuffer {
    u8 status;
    u8 deviceId;
    u8 payload[32];
} ControllerReceiveBuffer;

GameTask gGameTasks[200] __attribute__((section(".data.game_tasks"))) = {0};

GameTask *D_8013AE88 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AE8C __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AE90 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AE94 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AE98 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AE9C __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AEA0 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AEA4 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AEA8 __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AEAC __attribute__((section(".sdata.task_scheduler"))) = 0;
GameTask *D_8013AEB0 __attribute__((section(".sdata.task_scheduler"))) = 0;
u16 D_8013AEB4 __attribute__((section(".sdata.task_scheduler"))) = 0;
static u16 sTaskSchedulerPadding __attribute__((section(".sdata.task_scheduler"))) = 0;

CdRequest gCdRequests[64] __attribute__((section(".data.cd_requests"))) = {0};

CdCommand gCdCommands[8] __attribute__((section(".data.cd_commands"))) = {0};

u16 gCdRequestPhase = 0;
u16 gCdRequestWriteIndex = 0;
u16 gCdRequestReadIndex = 0;
u16 D_8013AE76 = 0;

ControllerReceiveBuffer D_800E9558[2] __attribute__((section(".data.controller_receive_buffers"))) = {0};
extern ControllerInputRecord *D_8013AED4[];
extern u16 D_8013AEDC[];
extern u8 D_8013AEE0[];

u8 D_8013AEE4 __attribute__((section(".sdata.controller_input_state"))) = 0;
static u8 sControllerInputStatePadding0 __attribute__((section(".sdata.controller_input_state"))) = 0;
static u8 sControllerInputStatePadding1 __attribute__((section(".sdata.controller_input_state"))) = 0;
static u8 sControllerInputStatePadding2 __attribute__((section(".sdata.controller_input_state"))) = 0;
u16 D_8013AEE8[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
u16 D_8013AEEC[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
s16 D_8013AEF0[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
s16 D_8013AEF4[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
s16 D_8013AEF8[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
s16 D_8013AEFC[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
ControllerActuatorOutput D_8013AF00[2] __attribute__((section(".data.controller_input_arrays"))) = {0};
static u16 sControllerActuatorPadding __attribute__((section(".data.controller_input_arrays"))) = 0;
u16 D_8013AF0C[2] __attribute__((section(".sdata.controller_previous_buttons"))) = {0};

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

TaskCallback gInitialTaskCallbacks[6] __attribute__((section(".data.initial_task_callbacks"))) = {
    initializeGameOptionTask, func_8001E0C0, func_8001E0C0,
    func_8001E148, func_8001E148, func_8001E1B8,
};
/* Scratch links used while removing a subtree from the task list. */
extern GameTask *D_8013AEA0;
extern GameTask *D_8013AEA4;
extern GameTask *D_8013AEA8;
extern GameTask *D_8013AEAC;
extern GameTask *D_8013AEB0;
extern u16 D_8013AEB4;

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

void updateCdVolumeFade(void) {
    if (gCdVolumeFadeActive) {
        if (--gCdVolumeFadeTimer <= 0) {
            gCdVolumeFadeTimer = gCdVolumeFadeInterval;
            if (gCdVolumeFadeTarget == gCdVolumeFadeCurrent) {
                gCdVolumeFadeActive = 0;
            }
            if (gCdVolumeFadeTarget < gCdVolumeFadeCurrent) {
                gCdVolumeFadeCurrent -= gCdVolumeFadeStep;
                if (gCdVolumeFadeCurrent < gCdVolumeFadeTarget) {
                    gCdVolumeFadeActive = 0;
                    if (gCdVolumeFadeTarget == 0) {
                        resetPendingCdRequests();
                    }
                }
            } else {
                gCdVolumeFadeCurrent += gCdVolumeFadeStep;
                if (gCdVolumeFadeCurrent > gCdVolumeFadeTarget) {
                    gCdVolumeFadeActive = 0;
                }
            }
            SsSetSerialVol(0, gCdVolumeFadeCurrent, gCdVolumeFadeCurrent);
        }
    }
}

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

void queueCdCommand(s16 command, s16 value) {
    s32 index = D_8013AE4E & 7;
    D_8013AE4E = index;
    if (command == -1) {
        if (D_8013AE78 != 0 && D_8013AE4C == -1) return;
        gCdVolumeFadeActive = 0;
        gCdRequestReadIndex = 0;
        gCdRequestWriteIndex = 0;
        gCdRequestPhase = 0;
        D_8013AE50 = 0;
        D_8013AE4E = 0;
        D_8013AE4C = -1;
        D_8013AE48 = 0;
        D_8013AE78 = 1;
    } else {
        SsSetSerialVol(0, 103, 103);
        gCdCommands[index].command = command;
        gCdCommands[index].value = value;
        D_8013AE4E++;
    }
}

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
        queueCdCommand(-1, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_80013F28);

INCLUDE_ASM("asm/nonmatchings/main", func_80013F98);

void initializeTaskScheduler(void) {
    u16 i;
    for (i = 0; i < 6; i++) {
        gGameTasks[i].next = &gGameTasks[i + 1];
        gGameTasks[i].previous = &gGameTasks[i - 1];
        gGameTasks[i].parent = 0;
        gGameTasks[i].callback = gInitialTaskCallbacks[i];
        gGameTasks[i].unk10 = 0;
        gGameTasks[i].depth = 0;
        gGameTasks[i].resource16 = -1;
        gGameTasks[i].resource14 = -1;
    }
    gGameTasks[0].previous = 0;
    gGameTasks[5].next = 0;
    D_8013AE90 = &gGameTasks[1];
    D_8013AE94 = &gGameTasks[2];
    D_8013AE9C = &gGameTasks[6];
    for (i = 6; i < 200; i++) {
        gGameTasks[i].next = &gGameTasks[i + 1];
        gGameTasks[i].previous = 0;
        gGameTasks[i].parent = 0;
        gGameTasks[i].callback = 0;
        gGameTasks[i].unk10 = -1;
        gGameTasks[i].depth = 0;
        gGameTasks[i].resource16 = -1;
        gGameTasks[i].resource14 = -1;
    }
    D_8013AEA0 = &gGameTasks[199];
    D_8013AEB4 = 193;
    for (i = 0; i < 16; i++) {
        gTaskResourceFreeBits[i] = -1;
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_800147B8);

INCLUDE_ASM("asm/nonmatchings/main", func_80014838);

INCLUDE_ASM("asm/nonmatchings/main", func_80014874);

INCLUDE_ASM("asm/nonmatchings/main", func_800148B4);

INCLUDE_ASM("asm/nonmatchings/main", func_800148F4);

INCLUDE_ASM("asm/nonmatchings/main", func_80014930);

INCLUDE_ASM("asm/nonmatchings/main", func_8001496C);

INCLUDE_ASM("asm/nonmatchings/main", func_800149A8);

INCLUDE_ASM("asm/nonmatchings/main", func_800149E0);

void deletePendingTaskSubtree(void) {
    u32 depth = D_8013AE88->depth;
    while (1) {
        D_8013AEB4++;
        if (D_8013AEB0->resource14 != 0xFFFF) {
            releaseTaskResource(D_8013AEB0->resource14);
        }
        if (D_8013AEB0->resource16 != 0xFFFF) {
            releaseTaskResource(D_8013AEB0->resource16);
        }
        D_8013AEB0->unk10 = 0xFFFF;
        D_8013AEA4 = D_8013AEB0->next;
        if (D_8013AEA4 != 0 && D_8013AEA4->depth > depth) {
            D_8013AEB0 = D_8013AEA4;
        } else {
            break;
        }
    }
    D_8013AEA0->next = D_8013AEAC;
    D_8013AEA4 = D_8013AEB0->next;
    D_8013AEA8 = D_8013AEAC->previous;
    D_8013AEA0 = D_8013AEB0;
    D_8013AEA8->next = D_8013AEA4;
    if (D_8013AEA4 != 0) {
        D_8013AEA4->previous = D_8013AEA8;
    }
}

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

void initializeControllers(void) {
    PadInitDirect((u8 *)&D_800E9558[0], (u8 *)&D_800E9558[1]);
    PadSetAct(0, &D_8013AF00[0].control, 2);
    PadSetAct(0x10, &D_8013AF00[1].control, 2);
    PadStartCom();
    D_8013AF0C[0] = 0;
    D_8013AEE8[0] = 0;
    D_8013AEEC[0] = 0;
    D_8013AEE8[1] = 0;
    D_8013AEEC[1] = 0;
    D_8013AF0C[1] = 0;
    D_8013AEFC[0] = 0;
    D_8013AEF8[0] = 0;
    D_8013AEF4[0] = 0;
    D_8013AEF0[0] = 0;
    D_8013AEFC[1] = 0;
    D_8013AEF8[1] = 0;
    D_8013AEF4[1] = 0;
    D_8013AEF0[1] = 0;
    setControllerAxesFromButtons(0);
    setControllerAxesFromButtons(1);
    resetControllerEffects();
    D_8013AEE4 = 0;
}

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
