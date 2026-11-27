#pragma once

#include "decomp/types.h"
#include "psx/libcd.h"
#include "game/io/dat_resources.h"

typedef struct ResourceSectorRange {
    s16 sector;
    s16 sectorCount;
} ResourceSectorRange;

extern ResourceSectorRange gGridResourceFirstSectorRanges[10][5];
extern ResourceSectorRange gGridResourceSecondSectorRanges[10][5];
void queuePlayerResourceReadsAndUploadTextures(void);

/* Shared CD state remains in the original data owner. */
extern u16 gCdCommandPhase;
extern s16 gCdCommandActive;
extern s16 gCdActiveCommand;
extern u16 gCdRequestPhase;
extern u16 gCdRequestReadIndex;
extern u16 gCdRequestWriteIndex;
extern u16 gCdCommandWriteIndex;
extern u16 gCdCommandReadIndex;
extern s32 gXaPositionPollCount;

s32 isCdCommandQueueIdle(void);
s32 queueCdSectorCommand(s16 command, s16 sector, s16 count, void *destination, s32 value);
void queueGridResourceSectorReads(s16 slot, s16 row, s16 column);
void queueMenuResourceReads(s16 resource);
void queueStartupSectorReads(void);
void queueCourseResourceSectorReads(void);
void queueCdCommand(s16 command, s16 value);
void stopCdAndResetRequestQueue(void);
void queueCdStop(void);
void queueBackgroundTextureSectorCommands(void);
void queueBackgroundTextureArchiveRead(void);
void queueMode11TransitionResources(void);
void resetPendingCdRequests(void);
void stopCdAudioPlayback(void);
void resetCdCommandState(void);

extern u8 gCdVolumeFadeActive;
extern u8 gCdVolumeFadeInterval;
extern s16 gCdVolumeFadeCurrent;
extern s16 gCdVolumeFadeTarget;
extern s16 gCdVolumeFadeStep;
extern s16 gCdVolumeFadeTimer;

void resetCdVolumeFade(void);
void startCdVolumeFade(s16 startVolume, s16 step, s16 targetVolume, u8 interval);
void updateCdVolumeFade(void);

void initializeCdFiles(void);
typedef enum MovieFileIndex {
    MOVIE_FILE_AT,
    MOVIE_FILE_RA,
    MOVIE_FILE_OPENING,
    MOVIE_FILE_COUNT
} MovieFileIndex;

extern CdlFILE gMovieFiles[MOVIE_FILE_COUNT];
extern CdlFILE gDatFile;
extern s32 gDatStartSector;

void updateCdRequestQueue(void);
void updateCdCommandQueue(void);
extern u16 gCdStreamingFlags;
extern s32 gCdReadByteCount __attribute__((section(".sdata.cd_read_state")));
extern s32 gCdIoFlags;
