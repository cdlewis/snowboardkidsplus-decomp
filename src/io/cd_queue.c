#include "../engine/input_scheduler_internal.h"

u16 gCdRequestPhase = 0;
u16 gCdRequestWriteIndex = 0;
u16 gCdRequestReadIndex = 0;
s16 D_8013AE76 = 0;

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

CdRequest gCdRequests[64] __attribute__((section(".data.cd_requests"))) = {0};

CdCommand gCdCommands[8] __attribute__((section(".data.cd_commands"))) = {0};

u8 gCdVolumeFadeActive __attribute__((section(".sdata.cd_volume_fade"))) = 0;

u8 gCdVolumeFadeInterval __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeCurrent __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeTarget __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeStep __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeTimer __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;

static u16 sCdVolumeFadeTimerPadding __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;

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

INCLUDE_ASM("asm/nonmatchings/io/cd_queue", func_800134E4);

INCLUDE_ASM("asm/nonmatchings/io/cd_queue", func_8001351C);

INCLUDE_ASM("asm/nonmatchings/io/cd_queue", func_80013540);

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

INCLUDE_ASM("asm/nonmatchings/io/cd_queue", func_80013F28);

INCLUDE_ASM("asm/nonmatchings/io/cd_queue", func_80013F98);
