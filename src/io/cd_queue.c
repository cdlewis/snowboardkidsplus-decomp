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
s16 gCdRequestQueuePaused __attribute__((section(".sdata.cd_request_state"))) = 0;

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

u16 gCdStreamingFlags __attribute__((section(".sdata.cd_read_state"))) = 0;

static u16 sCdReadStatePadding __attribute__((section(".sdata.cd_read_state"))) = 0;

s32 gCdReadByteCount __attribute__((section(".sdata.cd_read_state"))) = 0;

s32 gCdIoFlags __attribute__((section(".sdata.cd_io_flags"))) = 0;

u8 gCdReadModeDelay[8] __attribute__((section(".sdata.cd_read_delay"))) = {0};

u8 gCdRequestSyncResult[8] __attribute__((section(".data.cd_sync_result"))) = {0};

CdlFILE gCdStreamSearchFile __attribute__((section(".data.cd_search_file"))) = {0};

CdRequest gCdRequests[64] __attribute__((section(".data.cd_requests"))) = {0};

CdCommand gCdCommands[8] __attribute__((section(".data.cd_commands"))) = {0};

u8 gCdVolumeFadeActive __attribute__((section(".sdata.cd_volume_fade"))) = 0;

u8 gCdVolumeFadeInterval __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeCurrent __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeTarget __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeStep __attribute__((section(".sdata.cd_volume_fade"))) = 0;

s16 gCdVolumeFadeTimer __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;

static u16 sCdVolumeFadeTimerPadding __attribute__((section(".sdata.cd_volume_fade_timer"))) = 0;

u16 gCdCommandPhase __attribute__((section(".sdata.cd_command_state"))) = 0;

s16 gXaStopAtTrackEnd __attribute__((section(".sdata.cd_command_state"))) = 0;

s16 gCdActiveCommand __attribute__((section(".sdata.cd_command_state"))) = 0;

u16 gCdCommandWriteIndex __attribute__((section(".sdata.cd_command_state"))) = 0;

u16 gCdCommandReadIndex __attribute__((section(".sdata.cd_command_state"))) = 0;

static u16 sCdCommandStatePadding __attribute__((section(".sdata.cd_command_state"))) = 0;

s32 gXaPositionPollCount __attribute__((section(".sdata.cd_command_state"))) = 0;

s32 D_8013AE58 __attribute__((section(".sdata.cd_command_state"))) = 0;

s32 gXaCurrentSector __attribute__((section(".sdata.cd_command_state"))) = 0;

s32 gXaTrackEndSector __attribute__((section(".sdata.cd_command_state"))) = 0;

s32 gXaTrackStartSector __attribute__((section(".sdata.cd_command_state"))) = 0;

s16 gCdCommandActive __attribute__((section(".sdata.cd_command_active"))) = 0;

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
    gCdRequestQueuePaused = 0;
    gCdRequestPhase = 0;
    gCdStreamingFlags = 0;
    gCdCommandReadIndex = 0;
    gCdCommandWriteIndex = 0;
    gCdCommandActive = 0;
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
    gCdRequestQueuePaused = 1;
    restoreCdReadModeAndResetQueues();
    gCdRequestReadIndex = 0;
    gCdRequestWriteIndex = 0;
    gCdRequestPhase = 0;
    gCdRequestQueuePaused = 0;
}

void queueCdStop(void) {
    queueCdCommand(-1, 0);
}

void restoreCdReadModeAndResetQueues(void) {
    u8 result[16];
    s32 phase;
    s32 delay;
    s32 status;

    if (gCdIoFlags & CD_READ_PENDING) {
        CdReadSync(0, result);
        gCdIoFlags &= ~CD_READ_PENDING;
    }
    startCdVolumeFade(0, 1, 0, 1);
    gCdVolumeFadeActive = 0;
    phase = 0;
    while (1) {
        VSync(0);
        switch (phase) {
        case 0:
            if (CdSync(1, result) != 0) {
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                phase++;
            }
            break;
        case 1:
            if (!(gCdStreamingFlags & CD_XA_ACTIVE)) {
                phase = 3;
            } else {
                CdControlF(CdlPause, NULL);
                phase++;
            }
            break;
        case 2:
            status = CdSync(1, result);
            if (status == CD_DISK_ERROR) {
                phase = 0;
            }
            if (status == CD_COMPLETE) {
                phase++;
            }
            break;
        case 3:
            result[0] = CdlModeSpeed;
            gCdStreamingFlags &= ~CD_XA_ACTIVE;
            CdControlF(CdlSetmode, result);
            delay = 3;
            phase++;
            break;
        case 4:
            status = CdSync(1, result);
            if (status == CD_DISK_ERROR) {
                phase = 0;
            }
            if (status == CD_COMPLETE) {
                phase++;
            }
            break;
        case 5:
            if (--delay > 0) {
                break;
            }
            gCdRequestReadIndex = 0;
            gCdRequestWriteIndex = 0;
            gCdRequestPhase = 0;
            gCdCommandReadIndex = 0;
            gCdCommandWriteIndex = 0;
            gCdCommandActive = 0;
            return;
        }
    }
}

s32 queueCdSectorCommand(s16 command, s16 sector, s16 count, void *destination, s32 value) {
    CdRequest *entry;
    if (gCdVolumeFadeActive) {
        resetPendingCdRequests();
        gCdVolumeFadeActive = 0;
    }
    entry = &gCdRequests[gCdRequestWriteIndex & 0x3F];
    entry->useDatSectors = 1;
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

    if (gCdCommandActive) {
        CdControlB(CdlNop, 0, result);
        if (result[0] & CdlStatRead) {
            return 1;
        }
    }
    return 0;
}

s32 isCdCommandQueueIdle(void) {
    if (gCdCommandActive == 0 || gCdActiveCommand == -1) {
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
    if (gCdRequestQueuePaused != 0)
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
    if (gCdCommandActive != 0) {
        if (gCdActiveCommand == -1)
            return;
        restoreCdReadModeAndResetQueues();
    }
    switch (gCdRequestPhase) {
    case 0:
        if (CdSync(1, result) == 0)
            break;
        result[0] = CdlModeSpeed;
        CdControlF(CdlSetmode, result);
        gCdReadModeDelay[0] = 3;
        gCdIoFlags |= 0x04000000;
        gCdRequestPhase++;
        break;
    case 1:
        status = CdSync(1, gCdRequestSyncResult);
        if (status == 2) {
            gCdIoFlags &= ~0x04000000;
            gCdRequestPhase++;
        } else if (status == 5) {
            gCdRequestPhase = 0;
            gCdIoFlags &= ~0x04000000;
        }
        break;
    case 2:
        gCdReadModeDelay[0]--;
        if ((s8)gCdReadModeDelay[0] > 0)
            break;
        /* Bit 7 skips reading; the low seven bits select processing below. */
        if ((s32)(gCdRequests[index].command & 0xFF) >= 0x80)
            goto read_started;
        if (gCdRequests[index].command == 15)
            goto stream_seek;
        if (gCdRequests[index].useDatSectors != 0) {
            gCdReadByteCount = gCdRequests[index].sectorCount << 11;
            CdIntToPos(gCdRequests[index].sector + gDatStartSector, &position);
            CdControlF(CdlSetloc, (u8 *)&position);
            gCdIoFlags |= 0x04000000;
            gCdRequestPhase++;
            break;
        }
        gCdIoFlags |= 0x02000000;
        do {
            gCdReadByteCount = CdReadFile(gCdRequests[index].filename, gCdRequests[index].destination, 0);
        } while (gCdReadByteCount == 0);
    read_started:
        gCdRequestPhase = 6;
        break;
    stream_seek:
        while (CdSearchFile(&gCdStreamSearchFile, gCdRequests[index].filename) == NULL) {
        }
        CdControlF(CdlSetloc, (u8 *)&gCdStreamSearchFile.pos);
        gCdIoFlags |= 0x04000000;
        gCdRequestPhase++;
        break;
    case 3:
        gCdRequestPhase++;
        /* fallthrough */
    case 4:
        status = CdSync(1, result);
        if (status == 2) {
            gCdIoFlags &= ~0x04000000;
            gCdRequestPhase++;
        } else if (status == 5) {
            gCdRequestPhase = 0;
            gCdIoFlags &= ~0x04000000;
        }
        break;
    case 5:
        if (gCdRequests[index].command != 15) {
            gCdIoFlags |= 0x02000000;
            if (CdRead(gCdRequests[index].sectorCount, gCdRequests[index].destination,
                       CdlModeSpeed))
                gCdRequestPhase++;
        } else {
            gCdStreamingFlags |= 1;
            if (CdRead2(0x1E0))
                gCdRequestPhase = 7;
        }
        break;
    case 6:
        readStatus = CdReadSync(1, result);
        if (readStatus == 0) {
            gCdRequestPhase = 7;
            gCdIoFlags &= ~0x02000000;
        }
        if (readStatus == -1) {
            gCdRequestPhase = 0;
            gCdIoFlags &= ~0x02000000;
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
        gCdIoFlags &= ~0x04000000;
        gCdRequestReadIndex++;
        break;
    }
}

void queueCdCommand(s16 command, s16 value) {
    s32 index = gCdCommandWriteIndex & 7;
    gCdCommandWriteIndex = index;
    if (command == -1) {
        if (gCdCommandActive != 0 && gCdActiveCommand == -1)
            return;
        gCdVolumeFadeActive = 0;
        gCdRequestReadIndex = 0;
        gCdRequestWriteIndex = 0;
        gCdRequestPhase = 0;
        gCdCommandReadIndex = 0;
        gCdCommandWriteIndex = 0;
        gCdActiveCommand = -1;
        gCdCommandPhase = 0;
        gCdCommandActive = 1;
    } else {
        SsSetSerialVol(0, 103, 103);
        gCdCommands[index].command = command;
        gCdCommands[index].value = value;
        gCdCommandWriteIndex++;
    }
}

void resetCdCommandState(void) {
    gCdCommandActive = 0;
    gCdCommandReadIndex = 0;
    gCdCommandWriteIndex = 0;
    gCdCommandPhase = 0;
    gXaTrackStartSector = 0;
    gXaTrackEndSector = 0;
    gXaCurrentSector = 0;
    D_8013AE58 = 0;
    gXaPositionPollCount = 0;
    gCdActiveCommand = 0;
    gXaStopAtTrackEnd = 0;
}

void resetPendingCdRequests(void) {
    if (gCdCommandActive != 0) {
        gCdCommandReadIndex &= 7;
        gCdCommandWriteIndex &= 7;
        if (gCdCommandReadIndex != gCdCommandWriteIndex) {
            gCdCommandReadIndex = 0;
            gCdCommandWriteIndex = 0;
        }
        queueCdCommand(-1, 0);
    }
}

void stopCdAudioPlayback(void) {
    startCdVolumeFade(0, 1, 0, 1);
    gCdVolumeFadeActive = 0;
    if (gCdCommandActive != 0) {
        gCdCommandReadIndex &= 7;
        gCdCommandWriteIndex &= 7;
        if (gCdCommandReadIndex != gCdCommandWriteIndex) {
            gCdCommandReadIndex = 0;
            gCdCommandWriteIndex = 0;
        }
        gCdCommandActive = 0;
        restoreCdReadModeAndResetQueues();
    }
}

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

    readIndex = gCdCommandReadIndex & 7;
    writeIndex = gCdCommandWriteIndex & 7;
    gCdCommandReadIndex = readIndex;
    gCdCommandWriteIndex = writeIndex;
    if (readIndex != writeIndex) {
        CdCommand *commands = gCdCommands;
        s16 command, value;

        gCdCommandReadIndex = readIndex + 1;
        command = commands[readIndex].command;
        value = commands[readIndex].value;
        gCdCommandPhase = 0;
        gCdCommandActive = 1;
        gCdActiveCommand = command;
        gXaStopAtTrackEnd = value;
    }

    if (gCdCommandActive == 0)
        return;
    activeCommand = gCdActiveCommand;
    if (activeCommand != -1) {
        switch (gCdCommandPhase) {
        case 0:
            if (CdSync(1, result.bytes) != 0) {
                if (!(gCdStreamingFlags & CD_XA_ACTIVE)) {
                    gCdCommandPhase++;
                } else {
                    CdControlF(CdlPause, NULL);
                    gCdIoFlags |= CD_CONTROL_PENDING;
                    gCdCommandPhase++;
                }
            }
            break;
        case 1:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                result.bytes[0] = CdlModeSpeed;
                gCdStreamingFlags &= ~CD_XA_ACTIVE;
                CdControlF(CdlSetmode, result.bytes);
                sXaModeDelay[0] = 4;
                gCdIoFlags |= CD_CONTROL_PENDING;
                gCdCommandPhase++;
            } else if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        case 2:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE)
                gCdCommandPhase++;
            if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        case 3:
            sXaModeDelay[0]--;
            if ((s8)sXaModeDelay[0] > 0)
                break;
            if (gCdActiveCommand < 16)
                fileIndex = gCdActiveCommand / 8;
            else
                fileIndex = (gCdActiveCommand - 16) / 32 + 2;
            pathIndex = fileIndex;
            paths = sXaPaths;
            path = paths + pathIndex;
            while (CdSearchFile(&file, *path) == NULL) {
            }
            sector = CdPosToInt(&file.pos);
            gXaTrackStartSector = sector;
            gXaCurrentSector = sector;
            D_8013AE58 = 0;
            gXaTrackEndSector = sector + sXaTrackSectorLengths[gCdActiveCommand];
            CdIntToPos(sector, &sXaSeekPosition);
            CdControlF(CdlSetloc, (u8 *)&sXaSeekPosition);
            gCdIoFlags |= CD_CONTROL_PENDING;
            gCdCommandPhase++;
            break;
        case 4:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                gCdCommandPhase++;
                gCdIoFlags |= CD_CONTROL_PENDING;
            } else if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        case 5:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                gCdCommandPhase++;
            } else if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        case 6:
            sXaFilter.file = 1;
            if (gCdActiveCommand < 16)
                sXaFilter.channel = gCdActiveCommand & 7;
            else
                sXaFilter.channel = (gCdActiveCommand - 16) & 31;
            CdControlF(CD_SET_FILTER, (u8 *)&sXaFilter);
            gCdIoFlags |= CD_CONTROL_PENDING;
            gCdCommandPhase++;
            break;
        case 7:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                gCdCommandPhase++;
            } else if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        case 8:
            gCdStreamingFlags |= CD_XA_ACTIVE;
            if (CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSize1 | CD_XA_FILTER_MODE) == 1) {
                gXaPositionPollCount = 0;
                gSynchronizedFrameCount = 1;
                gCdCommandPhase++;
            }
            break;
        case 9:
            if (gSynchronizedFrameCount & 7)
                break;
            status = CdSync(1, result.bytes);
            if (status == CD_DISK_ERROR) {
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                gCdCommandPhase++;
                break;
            }
            if (status == CD_COMPLETE) {
                if (gXaCurrentSector > gXaTrackEndSector || gXaCurrentSector < gXaTrackStartSector) {
                    if (gXaStopAtTrackEnd == 0) {
                        gCdCommandPhase++;
                    } else {
                        gCdCommandPhase = 0;
                        gCdActiveCommand = -1;
                    }
                    break;
                }
                if (gXaPositionPollCount >= 6 && CdLastCom() == CD_GET_LOCATION) {
                    sector = CdPosToInt(&result.getLocation.position);
                    if (sector > 0)
                        gXaCurrentSector = sector;
                }
                CdControlF(CD_GET_LOCATION, NULL);
                gCdIoFlags |= CD_CONTROL_PENDING;
            }
            gXaPositionPollCount++;
            break;
        case 10:
            CdControlF(CdlPause, NULL);
            gCdIoFlags |= CD_CONTROL_PENDING;
            gCdCommandPhase++;
            break;
        case 11:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                gXaPositionPollCount = 0;
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                gCdStreamingFlags &= ~CD_XA_ACTIVE;
            } else if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 10;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        }
    } else {
        switch (gCdCommandPhase) {
        case 0:
            if (gCdIoFlags & CD_READ_PENDING) {
                readStatus = CdReadSync(1, result.bytes);
                if (readStatus == 0) {
                    gCdIoFlags &= ~CD_READ_PENDING;
                    gCdIoFlags &= ~CD_CONTROL_PENDING;
                    gCdCommandPhase++;
                }
                if (readStatus == activeCommand) {
                    gCdIoFlags &= ~CD_READ_PENDING;
                    gCdIoFlags &= ~CD_CONTROL_PENDING;
                    gCdCommandPhase++;
                }
                break;
            }
            gCdCommandPhase = 1;
            /* fallthrough */
        case 1:
            if (!(gCdStreamingFlags & CD_XA_ACTIVE)) {
                gCdCommandActive = 0;
            } else if (CdSync(1, result.bytes) != 0) {
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                gCdCommandPhase++;
            }
            break;
        case 2:
            CdControlF(CdlPause, NULL);
            gCdIoFlags |= CD_CONTROL_PENDING;
            gCdCommandPhase++;
            break;
        case 3:
            status = CdSync(1, result.bytes);
            if (status == CD_COMPLETE) {
                gCdCommandActive = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
                gCdStreamingFlags &= ~CD_XA_ACTIVE;
            } else if (status == CD_DISK_ERROR) {
                gCdCommandPhase = 0;
                gCdIoFlags &= ~CD_CONTROL_PENDING;
            }
            break;
        }
    }
}
