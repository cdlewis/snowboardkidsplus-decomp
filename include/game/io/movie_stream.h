#pragma once

#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libpress.h"

typedef struct MovieDecodeState {
    u32 *vlcBuffers[2];
    s32 vlcBufferIndex;
    u32 *imageBuffers[2];
    s32 imageBufferIndex;
    RECT frames[2];
    s32 frameIndex;
    RECT slice;
    s32 outputComplete;
} MovieDecodeState;

extern MovieDecodeState gMovieDecodeState;
extern s32 StCdIntrFlag __attribute__((section(".data")));
void StCdInterrupt(void);
u32 *getNextMovieStreamFrame(MovieDecodeState *state);
void playMovie(s32 movieIndex);
void startMovieCdRead(CdlLOC *location);
void initMovieDecodeState(MovieDecodeState *state, s32 x0, s32 y0, s32 x1, s32 y1);
void startMovieStream(CdlLOC *location, DecDCCb outputCallback);
void handleMovieOutput(void);
s32 decodeNextMovieFrame(MovieDecodeState *state);
void waitForMovieOutput(MovieDecodeState *state, s32 unusedMode);

typedef char MovieDecodeStateSizeCheck[(sizeof(MovieDecodeState) == 0x38) ? 1 : -1];
