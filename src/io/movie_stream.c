#include "game/io/dat_resources.h"
#include "card_movie_internal.h"

#define MOVIE_SKIP_BUTTON 0x0800

MovieInfo gMovieInfo[] __attribute__((section(".data.movie_info"))) = {
    {194, 0, 0},
    {76, 0, 0},
    {973, 0, 0},
};
s16 gMovieFrameWidth __attribute__((section(".sdata.movie_dimensions"))) = 0;
s16 gMovieFrameHeight __attribute__((section(".sdata.movie_dimensions"))) = 0;
s16 gCurrentMovieIndex __attribute__((section(".sdata.movie_index"))) = 0;
static u16 sMovieIndexPadding __attribute__((section(".sdata.movie_index"))) = 0;
u8 gMovieReachedEnd __attribute__((section(".sdata.movie_flags"))) = 0;
u8 gMovieStartupFrames __attribute__((section(".sdata.movie_flags"))) = 0;
static u16 sMovieFlagsPadding __attribute__((section(".sdata.movie_flags"))) = 0;

MovieDecodeState gMovieDecodeState __attribute__((section(".bss.movie_decode_state")));

void playMovie(s32 movieIndex) {
    DISPENV display;
    u8 padding[0x60]; /* Preserves the original stack frame. */
    CdlLOC location;
    RECT clearRect = {0, 0, 639, 511};
    s32 displayY;

    gCurrentMovieIndex = movieIndex;
    ClearImage(&clearRect, 0, 0, 0);
    DrawSync(0);
    SsSetSerialVol(0, 127, 127);
    gMovieStartupFrames = 0;
    initMovieDecodeState(&gMovieDecodeState, gMovieInfo[gCurrentMovieIndex].x, gMovieInfo[gCurrentMovieIndex].y,
                        gMovieInfo[gCurrentMovieIndex].x, gMovieInfo[gCurrentMovieIndex].y + 240);
    startMovieStream(&gMovieFiles[gCurrentMovieIndex].pos, handleMovieOutput);
    DecDCTvlcBuild(MOVIE_VLC_TABLE);
    while (decodeNextMovieFrame(&gMovieDecodeState) == -1) {
        location = gMovieFiles[gCurrentMovieIndex].pos;
        startMovieCdRead(&location);
    }
    gMovieReachedEnd = 0;
    D_8013AEEC[0] = D_8013AEEC[1] = MOVIE_SKIP_BUTTON;
    while (1) {
        updateControllers();
        DecDCTin(gMovieDecodeState.vlcBuffers[gMovieDecodeState.vlcBufferIndex], 3);
        DecDCTout(gMovieDecodeState.imageBuffers[gMovieDecodeState.imageBufferIndex],
                  gMovieDecodeState.slice.w * gMovieDecodeState.slice.h / 2);
        decodeNextMovieFrame(&gMovieDecodeState);
        waitForMovieOutput(&gMovieDecodeState, 0);
        VSync(0);
        displayY = !gMovieDecodeState.frameIndex;
        SetDefDispEnv(&display, 0, displayY * 240, 480, 240);
        display.disp.w = display.disp.w * 2 / 3;
        display.isrgb24 = 1;
        PutDispEnv(&display);
        SetDispMask(1);
        if (gMovieReachedEnd == 1 || (D_8013AEE8[0] & MOVIE_SKIP_BUTTON)) break;
        if (gMovieStartupFrames < 2) gMovieStartupFrames++;
    }
    SsSetSerialVol(0, 0, 0);
    DecDCToutCallback(0);
    StUnSetRing();
    CdControlB(CdlPause, 0, 0);
    VSync(2);
    ClearImage(&clearRect, 0, 0, 0);
    DrawSync(0);
}

void initMovieDecodeState(MovieDecodeState *state, s32 x0, s32 y0, s32 x1, s32 y1) {
    state->vlcBuffers[0] = MOVIE_VLC_BUFFER_0;
    state->vlcBuffers[1] = MOVIE_VLC_BUFFER_1;
    state->vlcBufferIndex = 0;
    state->imageBuffers[0] = MOVIE_IMAGE_BUFFER_0;
    state->imageBuffers[1] = MOVIE_IMAGE_BUFFER_1;
    state->imageBufferIndex = 0;
    state->frames[0].x = x0;
    state->frames[0].y = y0;
    state->frames[1].x = x1;
    state->frameIndex = 0;
    state->slice.x = x0;
    state->slice.y = y0;
    state->slice.w = 24;
    state->outputComplete = 0;
    state->frames[1].y = y1;
}

void startMovieStream(CdlLOC *location, DecDCCb outputCallback) {
    DecDCTReset(0);
    DecDCToutCallback(outputCallback);
    StSetRing(DAT_SCRATCH, 32);
    StSetStream(1, 1, -1, 0, 0);
    startMovieCdRead(location);
}

void handleMovieOutput(void) {
    RECT slice;
    s32 imageIndex;
    if (StCdIntrFlag) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    imageIndex = gMovieDecodeState.imageBufferIndex;
    slice = gMovieDecodeState.slice;
    gMovieDecodeState.imageBufferIndex = !gMovieDecodeState.imageBufferIndex;
    gMovieDecodeState.slice.x += gMovieDecodeState.slice.w;
    if (gMovieDecodeState.slice.x < gMovieDecodeState.frames[gMovieDecodeState.frameIndex].x + gMovieDecodeState.frames[gMovieDecodeState.frameIndex].w) {
        DecDCTout(gMovieDecodeState.imageBuffers[gMovieDecodeState.imageBufferIndex], gMovieDecodeState.slice.w * gMovieDecodeState.slice.h / 2);
    } else {
        gMovieDecodeState.outputComplete = 1;
        gMovieDecodeState.frameIndex = !gMovieDecodeState.frameIndex;
        gMovieDecodeState.slice.x = gMovieDecodeState.frames[gMovieDecodeState.frameIndex].x;
        gMovieDecodeState.slice.y = gMovieDecodeState.frames[gMovieDecodeState.frameIndex].y;
    }
    if (gMovieStartupFrames > 1) {
        LoadImage(&slice, gMovieDecodeState.imageBuffers[imageIndex]);
    }
}

s32 decodeNextMovieFrame(MovieDecodeState *state) {
    u32 *frame;
    s32 timeout = 2000;
    while (timeout != 0) {
        frame = getNextMovieStreamFrame(state);
        if (frame != 0) {
            state->vlcBufferIndex = !state->vlcBufferIndex;
            DecDCTvlc2(frame, state->vlcBuffers[state->vlcBufferIndex], MOVIE_VLC_TABLE);
            StFreeRing(frame);
            return 0;
        }
        timeout--;
    }
    return -1;
}

u32 *getNextMovieStreamFrame(MovieDecodeState *state) {
    u32 *data;
    MovieStreamHeader *header;
    s32 timeout = 2000;
    while (StGetNext(&data, &header)) {
        if (--timeout == 0) return 0;
    }
    if (header->frameNumber >= gMovieInfo[gCurrentMovieIndex].endFrame) gMovieReachedEnd = 1;
    if (gMovieFrameWidth != header->width || gMovieFrameHeight != header->height) {
        RECT rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = 480;
        rect.h = 480;
        ClearImage(&rect, 0, 0, 0);
        gMovieFrameWidth = header->width;
        gMovieFrameHeight = header->height;
    }
    state->frames[0].w = state->frames[1].w = gMovieFrameWidth * 3 / 2;
    state->slice.h = state->frames[0].h = state->frames[1].h = gMovieFrameHeight;
    return data;
}

void waitForMovieOutput(MovieDecodeState *state, s32 unusedMode) {
    volatile s32 timeout = 0x800000;
    while (state->outputComplete == 0) {
        if (--timeout == 0) {
            state->outputComplete = 1;
            state->frameIndex = !state->frameIndex;
            state->slice.x = state->frames[state->frameIndex].x;
            state->slice.y = state->frames[state->frameIndex].y;
        }
    }
    state->outputComplete = 0;
}

void startMovieCdRead(CdlLOC *location) {
    u8 mode = CdlModeSpeed;
    do {
        while (!CdControl(CdlSetloc, (u8 *)location, 0)) {}
        while (!CdControl(CdlSetmode, &mode, 0)) {}
        VSync(3);
    } while (!CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT | CdlModeSize1));
}
