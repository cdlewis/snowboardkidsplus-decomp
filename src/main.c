#include "common.h"
#include "psx/libcd.h"
#include "psx/libpad.h"
#include "psx/libapi.h"
#include "psx/libapi.h"
#include "game/graphics.h"
#include "game/music.h"
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

typedef struct CdRequest {
    s32 unk_00;
    void *destination;
    char *filename;
    s32 sector;
    s32 sectorCount;
    s16 command;
    s16 unk_16; /* Nonzero selects sectors relative to SNB.DAT. */
    s32 unk_18;
} CdRequest;

static char sAtMoviePath[16] __attribute__((section(".rodata.cd_paths"), aligned(4))) =
    "\\MV\\ATMOV.STR;1";
static char sRaMoviePath[16] __attribute__((section(".rodata.cd_paths"), aligned(4))) =
    "\\MV\\RAMOV.STR;1";
static char sOpeningMoviePath[16] __attribute__((section(".rodata.cd_paths"), aligned(4))) =
    "\\MV\\OPMOV.STR;1";
static char sDatPath[16] __attribute__((section(".rodata.cd_paths"), aligned(4))) =
    "\\SNB\\SNB.DAT;1";

CdlFILE gAtMovieFile __attribute__((section(".data.cd_files"))) = {0};
CdlFILE gRaMovieFile __attribute__((section(".data.cd_files"))) = {0};
CdlFILE gOpeningMovieFile __attribute__((section(".data.cd_files"))) = {0};
CdlFILE gDatFile __attribute__((section(".data.dat_file"))) = {0};
s32 gDatStartSector __attribute__((section(".sdata.dat_start_sector"))) = 0;
u16 D_8013AE68 __attribute__((section(".sdata.cd_read_state"))) = 0;
static u16 sCdReadStatePadding __attribute__((section(".sdata.cd_read_state"))) = 0;
s32 D_8013AE6C __attribute__((section(".sdata.cd_read_state"))) = 0;
s32 D_8013AE7C __attribute__((section(".sdata.cd_io_flags"))) = 0;
u8 D_8013B5DC[8] __attribute__((section(".sdata.cd_read_delay"))) = {0};
u8 D_8013B5E4[8] __attribute__((section(".data.cd_sync_result"))) = {0};
CdlFILE D_8013B6C8 __attribute__((section(".data.cd_search_file"))) = {0};

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
s16 D_8013AE76 = 0;
typedef struct ControllerReceiveBuffer {
    u8 status;
    u8 deviceId;
    u8 payload[32];
} ControllerReceiveBuffer;

ControllerReceiveBuffer D_800E9558[2] __attribute__((section(".data.controller_receive_buffers"))) = {0};
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

u8 gCdVolumeFadeActive __attribute__((section(".sdata.cd_volume_fade"))) = 0;
u8 gCdVolumeFadeInterval __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeCurrent __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeTarget __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeStep __attribute__((section(".sdata.cd_volume_fade"))) = 0;
s16 gCdVolumeFadeTimer __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;
static u16 sCdVolumeFadeTimerPadding __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;

u16 gTaskResourceFreeBits[16] __attribute__((section(".data.task_resource_bitmap"))) = {0};

u8 gControllerAnalogResponse[52] __attribute__((section(".data.controller_analog_response"))) = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x07, 0x0A, 0x0D, 0x10, 0x12, 0x14,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1B, 0x1C, 0x1C, 0x1D, 0x1D, 0x1D, 0x1E,
    0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00,
};

u8 gControllerFlags[4] __attribute__((section(".data"))) = {0, 0, 0, 0};

u8 gControllerHorizontalRepeatTimers[2]
    __attribute__((section(".sdata.controller_repeat_timing"))) = {0, 0};
u8 gControllerRepeatInitialDelay __attribute__((section(".sdata.controller_repeat_timing"))) = 12;
u8 gControllerRepeatInterval __attribute__((section(".sdata.controller_repeat_timing"))) = 2;

u8 gControllerActuatorAlignment[6] __attribute__((section(".data.controller_actuator_alignment"))) = {0, 1, 0xFF, 0xFF, 0xFF, 0xFF};
static u16 sControllerActuatorAlignmentPadding __attribute__((section(".data.controller_actuator_alignment"))) = 0;

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
    initializeGameOptionTask,
    func_8001E0C0,
    func_8001E0C0,
    func_8001E148,
    func_8001E148,
    func_8001E1B8,
};
/* Scratch links used while removing a subtree from the task list. */
extern GameTask *D_8013AEA0;
extern GameTask *D_8013AEA4;
extern GameTask *D_8013AEA8;
extern GameTask *D_8013AEAC;
extern GameTask *D_8013AEB0;
extern u16 D_8013AEB4;

void func_80013540(void);
void updateControllerAxesFromAnalog(u16 controller);

void func_800131B4(void) {}

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

void initializeCdFiles(void) {
    _96_remove();
    _96_init();
    do {

    } while (CdInit() == 0);
    gCdRequestReadIndex = 0;
    gCdRequestWriteIndex = 0;
    D_8013AE76 = 0;
    gCdRequestPhase = 0;
    D_8013AE68 = 0;
    D_8013AE50 = 0;
    D_8013AE4E = 0;
    D_8013AE78 = 0;
    resetCdCommandState();
    do {

    } while (CdSearchFile(&gAtMovieFile, sAtMoviePath) == 0);
    do {

    } while (CdSearchFile(&gRaMovieFile, sRaMoviePath) == 0);
    do {

    } while (CdSearchFile(&gOpeningMovieFile, sOpeningMoviePath) == 0);
    gDatFile.pos.minute = 0;
    gDatFile.pos.second = 0;
    gDatFile.pos.sector = 0;
    do {

    } while (CdSearchFile(&gDatFile, sDatPath) == 0);
    gDatStartSector = CdPosToInt(&gDatFile.pos);
}

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

void updateCdRequestQueue(void) {
    CdlLOC position;
    u8 result[16];
    s32 i;
    u8 *clear;
    s32 writeIndex;
    s16 index;
    s32 status;
    s32 readStatus;
    if ((s16)D_8013AE76 != 0)
        return;
    i = 15;
    clear = &result[15];
    do {
        *clear = 0;
        i--;
        clear--;
    } while (i >= 0);
    index = gCdRequestReadIndex & 63;
    writeIndex = gCdRequestWriteIndex & 63;
    gCdRequestReadIndex = index;
    gCdRequestWriteIndex = writeIndex;
    if (index == writeIndex)
        return;
    if (D_8013AE78 != 0) {
        if (D_8013AE4C == -1)
            return;
        func_80013540();
    }
    switch (gCdRequestPhase) {
    case 0:
        if (CdSync(1, result) == 0)
            break;
        result[0] = CdlModeSpeed;
        CdControlF(CdlSetmode, result);
        D_8013B5DC[0] = 3;
        D_8013AE7C |= 0x04000000;
        gCdRequestPhase++;
        break;
    case 1:
        status = CdSync(1, D_8013B5E4);
        if (status == 2) {
            D_8013AE7C &= ~0x04000000;
            gCdRequestPhase++;
        } else if (status == 5) {
            gCdRequestPhase = 0;
            D_8013AE7C &= ~0x04000000;
        }
        break;
    case 2:
        D_8013B5DC[0]--;
        if ((s8)D_8013B5DC[0] > 0)
            break;
        /* Bit 7 skips reading; the low seven bits select processing below. */
        if ((s32)(gCdRequests[index].command & 0xFF) >= 0x80)
            goto read_started;
        if (gCdRequests[index].command == 15)
            goto stream_seek;
        if (gCdRequests[index].unk_16 != 0) {
            D_8013AE6C = gCdRequests[index].sectorCount << 11;
            CdIntToPos(gCdRequests[index].sector + gDatStartSector, &position);
            CdControlF(CdlSetloc, (u8 *)&position);
            D_8013AE7C |= 0x04000000;
            gCdRequestPhase++;
            break;
        }
        D_8013AE7C |= 0x02000000;
        do {
            D_8013AE6C = CdReadFile(gCdRequests[index].filename, gCdRequests[index].destination, 0);
        } while (D_8013AE6C == 0);
    read_started:
        gCdRequestPhase = 6;
        break;
    stream_seek:
        while (CdSearchFile(&D_8013B6C8, gCdRequests[index].filename) == NULL) {
        }
        CdControlF(CdlSetloc, (u8 *)&D_8013B6C8.pos);
        D_8013AE7C |= 0x04000000;
        gCdRequestPhase++;
        break;
    case 3:
        gCdRequestPhase++;
        /* fallthrough */
    case 4:
        status = CdSync(1, result);
        if (status == 2) {
            D_8013AE7C &= ~0x04000000;
            gCdRequestPhase++;
        } else if (status == 5) {
            gCdRequestPhase = 0;
            D_8013AE7C &= ~0x04000000;
        }
        break;
    case 5:
        if (gCdRequests[index].command != 15) {
            D_8013AE7C |= 0x02000000;
            if (CdRead(gCdRequests[index].sectorCount, gCdRequests[index].destination,
                       CdlModeSpeed))
                gCdRequestPhase++;
        } else {
            D_8013AE68 |= 1;
            if (CdRead2(0x1E0))
                gCdRequestPhase = 7;
        }
        break;
    case 6:
        readStatus = CdReadSync(1, result);
        if (readStatus == 0) {
            gCdRequestPhase = 7;
            D_8013AE7C &= ~0x02000000;
        }
        if (readStatus == -1) {
            gCdRequestPhase = 0;
            D_8013AE7C &= ~0x02000000;
        }
        break;
    case 7:
        gCdRequestPhase++;
        /* fallthrough */
    case 8:
        switch ((u32)(u16)gCdRequests[index].command & 0x7F) {
        case 0:
            break;
        case 1:
            uploadTimImageWithOffset(gCdRequests[index].destination,
                                     (s16 *)gCdRequests[index].unk_18);
            break;
        case 2:
            uploadTimArchive(gCdRequests[index].destination, (s16 *)gCdRequests[index].unk_18);
            break;
        case 3:
            loadSoundBank(0, gCdRequests[index].destination, (u8 *)0x80180000);
            break;
        case 4:
            loadSoundBank(1, gCdRequests[index].destination, (u8 *)0x80180000);
            break;
        case 5:
            loadSoundBank(2, gCdRequests[index].destination, (u8 *)0x80180000);
            break;
        case 6:
            loadSoundBank(3, gCdRequests[index].destination, (u8 *)0x80180000);
            break;
        case 7:
            loadSoundBank(4, gCdRequests[index].destination, (u8 *)0x80180000);
            break;
        case 8:
            loadSoundBank(5, gCdRequests[index].destination, (u8 *)0x80180000);
            break;
        case 9:
            openMusicSequence(2, gCdRequests[index].destination);
            break;
        case 16:
            uploadImage64x256(gCdRequests[index].destination, 0x280, 0x100);
            break;
        }
        gCdRequestPhase = 0;
        D_8013AE7C &= ~0x04000000;
        gCdRequestReadIndex++;
        break;
    }
}

void queueCdCommand(s16 command, s16 value) {
    s32 index = D_8013AE4E & 7;
    D_8013AE4E = index;
    if (command == -1) {
        if (D_8013AE78 != 0 && D_8013AE4C == -1)
            return;
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

void setCurrentTaskCallback(TaskCallback callback) { D_8013AE88->callback = callback; }

INCLUDE_ASM("asm/nonmatchings/main", func_80014B58);

void deleteCurrentTask(void) { func_800149A8(); }

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

void setControllerFlag0(u16 controller) { gControllerFlags[controller] |= 1; }

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

void updateControllerAxesFromAnalog(u16 controller) {
    s16 x, y;
    x = D_800E9558[controller].payload[4] * 100 / 256 - 50;
    y = D_800E9558[controller].payload[5] * 100 / 256 - 50;
    if (x > 45) x = 45;
    if (x < -45) x = -45;
    if (y > 45) y = 45;
    if (y < -45) y = -45;
    if (x >= 0) D_8013AEF8[controller] = (s8)gControllerAnalogResponse[x];
    else D_8013AEF8[controller] = -(s8)gControllerAnalogResponse[-x];
    if (y >= 0) D_8013AEFC[controller] = (s8)gControllerAnalogResponse[y];
    else D_8013AEFC[controller] = -(s8)gControllerAnalogResponse[-y];
    updateControllerAxesFromButtons(controller);
    if (D_8013AEF8[controller] > 26) D_8013AEEC[controller] |= 0x2000;
    if (D_8013AEF8[controller] < -26) D_8013AEEC[controller] |= 0x8000;
    if (D_8013AEFC[controller] > 26) D_8013AEEC[controller] |= 0x4000;
    if (D_8013AEFC[controller] < -26) D_8013AEEC[controller] |= 0x1000;
}

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

void updateControllerInput(u16 controller) {
    ControllerReceiveBuffer *buffer;
    ControllerReceiveBuffer *packet;
    s32 buttons;
    s32 device;
    D_8013AF0C[controller] = D_8013AEEC[controller];
    if (!isControllerInputSupported(controller)) {
        D_8013AEE8[controller] = 0;
        D_8013AEEC[controller] = 0;
        D_8013AEFC[controller] = 0;
        D_8013AEF8[controller] = 0;
        return;
    }
    buffer = D_800E9558;
    packet = buffer + controller;
    device = packet->deviceId & 0xF0;
    if (device == 0x40) {
        D_8013AEEC[controller] = ((u8)~packet->payload[0] << 8) | (u8)~packet->payload[1];
        setControllerAxesFromButtons(controller);
    } else if (device == 0x70) {
        D_8013AEEC[controller] = ((u8)~packet->payload[0] << 8) | (u8)~packet->payload[1];
        updateControllerAxesFromAnalog(controller);
    } else if (device == 0x50) {
        D_8013AEEC[controller] = (u8)~packet->payload[0] << 8;
        buttons = ~packet->payload[1];
        if (buttons & 1) {
            D_8013AEEC[controller] |= 1;
        }
        if (buttons & 2) {
            D_8013AEEC[controller] |= 4;
        }
        if (buttons & 4) {
            D_8013AEEC[controller] |= 0x80;
        }
        if (buttons & 8) {
            D_8013AEEC[controller] |= 0x10;
        }
        if (buttons & 0x10) {
            D_8013AEEC[controller] |= 8;
        }
        if (buttons & 0x20) {
            D_8013AEEC[controller] |= 0x20;
        }
        if (buttons & 0x40) {
            D_8013AEEC[controller] |= 0x40;
        }
        if (buttons & 0x80) {
            D_8013AEEC[controller] |= 2;
        }
        updateControllerAxesFromAnalog(controller);
    }
    D_8013AEE8[controller] = ~D_8013AF0C[controller] & D_8013AEEC[controller];
}

void updateControllerActuators(u16 controller) {
    int state;
    if (gControllerFlags[controller] & 0x20) {
        D_8013AF00[controller].control = 0;
        D_8013AF00[controller].value = 0;
    } else if (PadInfoMode(controller << 4, 2, 0)) {
        func_8006BA4C(controller);
    } else {
        updateLegacyControllerActuators(controller);
    }
    state = PadGetState(controller << 4);
    if (state != 0) {
        if (state == 1) {
            D_8013AF00[controller].aligned = 0;
        }
        if (D_8013AF00[controller].aligned == 0) {
            PadSetAct(controller << 4, &D_8013AF00[controller].control, 2);
            if (state == 2) {
                D_8013AF00[controller].aligned = 1;
            } else if (state == 6) {
                if (PadSetActAlign(controller << 4, gControllerActuatorAlignment)) {
                    D_8013AF00[controller].aligned = 1;
                }
            }
        }
    }
}

void updateControllers(void) {
    updateControllerInput(0);
    func_80014D60(0);
    updateControllerActuators(0);
    updateControllerInput(1);
    func_80014D60(1);
    updateControllerActuators(1);
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
    *counter = *counter ? *counter - 1
                        : (D_8013AEDC[controller]++,
                           D_8013AED4[controller][D_8013AEDC[controller]].repeatCount);
}
