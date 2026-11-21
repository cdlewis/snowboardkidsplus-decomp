#include "../engine/input_scheduler_internal.h"
#include "game/audio/music.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/dat_resources.h"
#include "psx/libapi.h"
#include "psx/libcd.h"
#include "psx/libsnd.h"

#define CD_READ_PENDING 0x02000000
#define CD_CONTROL_PENDING 0x04000000
#define CD_XA_ACTIVE 1
#define CD_XA_FILTER_MODE 8
#define CD_SET_FILTER 0x0D
#define CD_GET_LOCATION 0x11
#define CD_COMPLETE 2
#define CD_DISK_ERROR 5

typedef union XaCommandResult {
    u8 bytes[16];
    struct {
        u8 trackAndRelativeTime[5];
        CdlLOC position;
    } getLocation;
} XaCommandResult;

typedef struct XaFilter {
    u8 file;
    u8 channel;
    u8 padding[2];
} XaFilter;

u16 gCdRequestPhase __attribute__((section(".sdata.cd_request_state"))) = 0;
u16 gCdRequestWriteIndex __attribute__((section(".sdata.cd_request_state"))) = 0;
u16 gCdRequestReadIndex __attribute__((section(".sdata.cd_request_state"))) = 0;
s16 D_8013AE76 __attribute__((section(".sdata.cd_request_state"))) = 0;

static char sAtMoviePath[16] __attribute__((section(".data.cd_paths"), aligned(4))) =
    "\\MV\\ATMOV.STR;1";

static char sRaMoviePath[16] __attribute__((section(".data.cd_paths"), aligned(4))) =
    "\\MV\\RAMOV.STR;1";

static char sOpeningMoviePath[16] __attribute__((section(".data.cd_paths"), aligned(4))) =
    "\\MV\\OPMOV.STR;1";

static char sDatPath[16] __attribute__((section(".data.cd_paths"), aligned(4))) =
    "\\SNB\\SNB.DAT;1";

CdlFILE gMovieFiles[MOVIE_FILE_COUNT] __attribute__((section(".data.cd_files"))) = {0};

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

static char sXaBgmShortPath[16] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\BGM_S.XA;1";

static char sXaBgmLongPath[16] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\BGM_L.XA;1";

static char sXaVoice01Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_01.XA;1";

static char sXaVoice02Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_02.XA;1";

static char sXaVoice03Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_03.XA;1";

static char sXaVoice04Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_04.XA;1";

static char sXaVoice05Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_05.XA;1";

static char sXaVoice06Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_06.XA;1";

static char sXaVoice07Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_07.XA;1";

static char sXaVoice08Path[20] __attribute__((section(".data.xa_paths"), aligned(4))) =
    "\\XA\\VOICE_08.XA;1";

static char *sXaPaths[10] __attribute__((section(".data.xa_tracks"))) = {
    sXaBgmShortPath,
    sXaBgmLongPath,
    sXaVoice01Path,
    sXaVoice02Path,
    sXaVoice03Path,
    sXaVoice04Path,
    sXaVoice05Path,
    sXaVoice06Path,
    sXaVoice07Path,
    sXaVoice08Path,
};

static s32 sXaTrackSectorLengths[] __attribute__((section(".data.xa_tracks"))) = {
    0x1A18, 0x0195, 0x0391, 0x37C2, 0x6F46, 0x6D3F, 0x609B, 0x38BF,
    0x7DE1, 0x86A6, 0xA274, 0x7226, 0xACA5, 0x74B0, 0x934E, 0x9186,
    0x0D40, 0x09E0, 0x0960, 0x0900, 0x0820, 0x0800, 0x07C0, 0x07A0,
    0x0780, 0x0740, 0x0720, 0x0700, 0x0680, 0x0660, 0x0660, 0x0600,
    0x05A0, 0x0580, 0x0580, 0x0560, 0x0540, 0x0540, 0x0500, 0x04E0,
    0x04C0, 0x04C0, 0x04C0, 0x04A0, 0x04A0, 0x04A0, 0x04A0, 0x04A0,
    0x0480, 0x0480, 0x0480, 0x0480, 0x0440, 0x0440, 0x0440, 0x0440,
    0x0440, 0x0440, 0x0420, 0x0420, 0x0420, 0x0420, 0x0400, 0x0400,
    0x0400, 0x0400, 0x0400, 0x03E0, 0x03E0, 0x03C0, 0x03C0, 0x03C0,
    0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03A0, 0x03A0, 0x03A0,
    0x04E0, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380,
    0x0360, 0x0360, 0x0360, 0x0360, 0x0340, 0x0340, 0x0340, 0x0320,
    0x0320, 0x0320, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x02E0,
    0x02E0, 0x02E0, 0x02E0, 0x02E0, 0x02E0, 0x02E0, 0x02C0, 0x02C0,
    0x03E0, 0x02C0, 0x02C0, 0x02C0, 0x02C0, 0x02C0, 0x02A0, 0x02A0,
    0x02A0, 0x02A0, 0x02A0, 0x02A0, 0x02A0, 0x0280, 0x0280, 0x0280,
    0x0280, 0x0280, 0x0260, 0x0260, 0x0260, 0x0260, 0x0260, 0x0260,
    0x0260, 0x0260, 0x0260, 0x0240, 0x0240, 0x0240, 0x0240, 0x0240,
    0x0360, 0x0240, 0x0220, 0x0220, 0x0220, 0x0220, 0x0220, 0x0220,
    0x0220, 0x0200, 0x0200, 0x01E0, 0x01E0, 0x01E0, 0x01E0, 0x01E0,
    0x01E0, 0x01E0, 0x01E0, 0x01C0, 0x01C0, 0x01C0, 0x01C0, 0x01C0,
    0x01C0, 0x01C0, 0x01A0, 0x01A0, 0x01A0, 0x01A0, 0x01A0, 0x01A0,
    0x02C0, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180,
    0x0180, 0x0180, 0x0180, 0x01A0, 0x0160, 0x0160, 0x0160, 0x0160,
    0x0160, 0x0160, 0x0160, 0x0140, 0x0140, 0x0140, 0x0140, 0x0140,
    0x0140, 0x0140, 0x0140, 0x0140, 0x0140, 0x0120, 0x0120, 0x0120,
    0x0240, 0x0120, 0x0100, 0x0100, 0x0100, 0x00E0, 0x00E0, 0x00E0,
    0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00A0, 0x00A0, 0x00A0,
    0x00C0, 0x00A0, 0x00A0, 0x00A0, 0x0080, 0x0080, 0x0080, 0x0080,
    0x0080, 0x0080, 0x0080, 0x0060, 0x0060, 0x0040, 0x0080, 0x0000,
    0x0740,
};

static u8 sXaModeDelay[4] __attribute__((section(".sdata.xa_mode_delay"), aligned(4))) = {0};

static CdlLOC sXaSeekPosition __attribute__((section(".data.xa_seek_position"), aligned(4))) = {0};

static XaFilter sXaFilter __attribute__((section(".sdata"), aligned(4))) = {0};

/* This unit accesses the shared frame counter by its absolute address. */
extern u32 gSynchronizedFrameCount __attribute__((section(".data")));

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

    } while (CdSearchFile(&gMovieFiles[MOVIE_FILE_AT], sAtMoviePath) == 0);
    do {

    } while (CdSearchFile(&gMovieFiles[MOVIE_FILE_RA], sRaMoviePath) == 0);
    do {

    } while (CdSearchFile(&gMovieFiles[MOVIE_FILE_OPENING], sOpeningMoviePath) == 0);
    gDatFile.pos.minute = 0;
    gDatFile.pos.second = 0;
    gDatFile.pos.sector = 0;
    do {

    } while (CdSearchFile(&gDatFile, sDatPath) == 0);
    gDatStartSector = CdPosToInt(&gDatFile.pos);
}

void stopCdAndResetRequestQueue(void) {
    D_8013AE76 = 1;
    func_80013540();
    gCdRequestReadIndex = 0;
    gCdRequestWriteIndex = 0;
    gCdRequestPhase = 0;
    D_8013AE76 = 0;
}

void queueCdStop(void) {
    queueCdCommand(-1, 0);
}

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
    if (D_8013AE76 != 0)
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
            loadSoundBank(0, gCdRequests[index].destination, DAT_SCRATCH);
            break;
        case 4:
            loadSoundBank(1, gCdRequests[index].destination, DAT_SCRATCH);
            break;
        case 5:
            loadSoundBank(2, gCdRequests[index].destination, DAT_SCRATCH);
            break;
        case 6:
            loadSoundBank(3, gCdRequests[index].destination, DAT_SCRATCH);
            break;
        case 7:
            loadSoundBank(4, gCdRequests[index].destination, DAT_SCRATCH);
            break;
        case 8:
            loadSoundBank(5, gCdRequests[index].destination, DAT_SCRATCH);
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

void updateCdCommandQueue(void) {
    XaCommandResult result;
    CdlFILE file;
    s32 i = 15;
    u8 *clear = &result.bytes[15];
    u16 readIndex, writeIndex;
    s16 fileIndex;
    char **path, **paths;
    s32 sector, status, readStatus, pathIndex;
    s16 activeCommand;

    do {
        *clear = 0;
        i--;
        clear--;
    } while (i >= 0);

    readIndex = D_8013AE50 & 7;
    writeIndex = D_8013AE4E & 7;
    D_8013AE50 = readIndex;
    D_8013AE4E = writeIndex;
    if (readIndex != writeIndex) {
        CdCommand *commands = gCdCommands;
        s16 command, value;

        D_8013AE50 = readIndex + 1;
        command = commands[readIndex].command;
        value = commands[readIndex].value;
        D_8013AE48 = 0;
        D_8013AE78 = 1;
        D_8013AE4C = command;
        D_8013AE4A = value;
    }

    if (D_8013AE78 == 0)
        return;
    activeCommand = D_8013AE4C;
    if (activeCommand != -1) {
        switch (D_8013AE48) {
        case 0:
            if (CdSync(1, result.bytes) != 0) {
                if (!(D_8013AE68 & CD_XA_ACTIVE)) {
                    D_8013AE48++;
                } else {
                    CdControlF(CdlPause, NULL);
                    D_8013AE7C |= CD_CONTROL_PENDING;
                    D_8013AE48++;
                }
            }
            break;
        case 1:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                result.bytes[0] = CdlModeSpeed;
                D_8013AE68 &= ~CD_XA_ACTIVE;
                CdControlF(CdlSetmode, result.bytes);
                sXaModeDelay[0] = 4;
                D_8013AE7C |= CD_CONTROL_PENDING;
                D_8013AE48++;
            } else if (status == CD_DISK_ERROR) {
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        case 2:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE)
                D_8013AE48++;
            if (status == CD_DISK_ERROR) {
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        case 3:
            sXaModeDelay[0]--;
            if ((s8)sXaModeDelay[0] > 0)
                break;
            if (D_8013AE4C < 16)
                fileIndex = D_8013AE4C / 8;
            else
                fileIndex = (D_8013AE4C - 16) / 32 + 2;
            pathIndex = fileIndex;
            paths = sXaPaths;
            path = paths + pathIndex;
            while (CdSearchFile(&file, *path) == NULL) {
            }
            sector = CdPosToInt(&file.pos);
            D_8013AE64 = sector;
            D_8013AE5C = sector;
            D_8013AE58 = 0;
            D_8013AE60 = sector + sXaTrackSectorLengths[D_8013AE4C];
            CdIntToPos(sector, &sXaSeekPosition);
            CdControlF(CdlSetloc, (u8 *)&sXaSeekPosition);
            D_8013AE7C |= CD_CONTROL_PENDING;
            D_8013AE48++;
            break;
        case 4:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                D_8013AE48++;
                D_8013AE7C |= CD_CONTROL_PENDING;
            } else if (status == CD_DISK_ERROR) {
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        case 5:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                D_8013AE7C &= ~CD_CONTROL_PENDING;
                D_8013AE48++;
            } else if (status == CD_DISK_ERROR) {
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        case 6:
            sXaFilter.file = 1;
            if (D_8013AE4C < 16)
                sXaFilter.channel = D_8013AE4C & 7;
            else
                sXaFilter.channel = (D_8013AE4C - 16) & 31;
            CdControlF(CD_SET_FILTER, (u8 *)&sXaFilter);
            D_8013AE7C |= CD_CONTROL_PENDING;
            D_8013AE48++;
            break;
        case 7:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                D_8013AE7C &= ~CD_CONTROL_PENDING;
                D_8013AE48++;
            } else if (status == CD_DISK_ERROR) {
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        case 8:
            D_8013AE68 |= CD_XA_ACTIVE;
            if (CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSize1 | CD_XA_FILTER_MODE) == 1) {
                D_8013AE54 = 0;
                gSynchronizedFrameCount = 1;
                D_8013AE48++;
            }
            break;
        case 9:
            if (gSynchronizedFrameCount & 7)
                break;
            status = CdSync(1, result.bytes);
            if (status == CD_DISK_ERROR) {
                D_8013AE7C &= ~CD_CONTROL_PENDING;
                D_8013AE48++;
                break;
            }
            if (status == CD_COMPLETE) {
                if (D_8013AE5C > D_8013AE60 || D_8013AE5C < D_8013AE64) {
                    if (D_8013AE4A == 0) {
                        D_8013AE48++;
                    } else {
                        D_8013AE48 = 0;
                        D_8013AE4C = -1;
                    }
                    break;
                }
                if (D_8013AE54 >= 6 && CdLastCom() == CD_GET_LOCATION) {
                    sector = CdPosToInt(&result.getLocation.position);
                    if (sector > 0)
                        D_8013AE5C = sector;
                }
                CdControlF(CD_GET_LOCATION, NULL);
                D_8013AE7C |= CD_CONTROL_PENDING;
            }
            D_8013AE54++;
            break;
        case 10:
            CdControlF(CdlPause, NULL);
            D_8013AE7C |= CD_CONTROL_PENDING;
            D_8013AE48++;
            break;
        case 11:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                D_8013AE54 = 0;
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
                D_8013AE68 &= ~CD_XA_ACTIVE;
            } else if (status == CD_DISK_ERROR) {
                D_8013AE48 = 10;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        }
    } else {
        switch (D_8013AE48) {
        case 0:
            if (D_8013AE7C & CD_READ_PENDING) {
                readStatus = CdReadSync(1, result.bytes);
                if (readStatus == 0) {
                    D_8013AE7C &= ~CD_READ_PENDING;
                    D_8013AE7C &= ~CD_CONTROL_PENDING;
                    D_8013AE48++;
                }
                if (readStatus == activeCommand) {
                    D_8013AE7C &= ~CD_READ_PENDING;
                    D_8013AE7C &= ~CD_CONTROL_PENDING;
                    D_8013AE48++;
                }
                break;
            }
            D_8013AE48 = 1;
            /* fallthrough */
        case 1:
            if (!(D_8013AE68 & CD_XA_ACTIVE)) {
                D_8013AE78 = 0;
            } else if (CdSync(1, result.bytes) != 0) {
                D_8013AE7C &= ~CD_CONTROL_PENDING;
                D_8013AE48++;
            }
            break;
        case 2:
            CdControlF(CdlPause, NULL);
            D_8013AE7C |= CD_CONTROL_PENDING;
            D_8013AE48++;
            break;
        case 3:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                D_8013AE78 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
                D_8013AE68 &= ~CD_XA_ACTIVE;
            } else if (status == CD_DISK_ERROR) {
                D_8013AE48 = 0;
                D_8013AE7C &= ~CD_CONTROL_PENDING;
            }
            break;
        }
    }
}
