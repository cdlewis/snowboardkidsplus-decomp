#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/race/camera/race_camera.h"
#include "game/engine/linear_random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/race/player/race_player.h"
#define GAME_OPTIONS_GP_ADDRESSING
#include "game/engine/game_options.h"
#define SELECTED_RECORDS_GP_ADDRESSING
#include "game/menu/selected_records.h"
#include "psx/libsnd.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"
#include "psx/libmcrd.h"
#include "psx/libcd.h"
#include "psx/libpress.h"

#include "game/engine/system_runtime.h"

typedef struct MovieStreamHeader {
    u16 id, type;
    u16 sectorIndex, sectorCount;
    u32 frameNumber, frameSize;
    u16 width, height;
} MovieStreamHeader;
typedef struct MovieInfo { s16 endFrame; u8 x, y; } MovieInfo;
extern MovieInfo D_800EBFE4[];
extern s16 D_8013AF60, D_8013AF54, D_8013AF56;
extern u8 D_8013B610;
s32 StGetNext(u32 **data, MovieStreamHeader **header);

s32 pollMemoryCardWrite(void);
void beginMemoryCardBufferRead(void);
void pollMemoryCardBufferReadTask(void);
s32 startMemoryCardReadTask(void);
void pollMemoryCardReadTask(void);
void waitForMemoryCardAcceptTask(void);
void pollMemoryCardStatusTask(void);
void pollMemoryCardWriteTask(void);
void initMovieDecodeState(MovieDecodeState *state, s16 x0, s16 y0, s16 x1, s16 y1);
void startMovieStream(CdlLOC *location, DecDCCb outputCallback);
void handleMovieOutput(void);
s32 decodeNextMovieFrame(MovieDecodeState *state);
u32 *getNextMovieStreamFrame(MovieDecodeState *state);
void waitForMovieOutput(MovieDecodeState *state);
void startMovieCdRead(CdlLOC *location);
void waitForFrameHcount(u32 minimumHcount);
