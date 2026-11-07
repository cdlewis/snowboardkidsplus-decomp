#include "card_movie_internal.h"

MovieDecodeState gMovieDecodeState __attribute__((section(".bss.movie_decode_state")));

INCLUDE_ASM("asm/nonmatchings/io/movie_stream", func_80019D3C);

void initMovieDecodeState(MovieDecodeState *state, s16 x0, s16 y0, s16 x1, s16 y1) {
    state->vlcBuffers[0] = (u32 *)0x801A1000;
    state->vlcBuffers[1] = (u32 *)0x801C9000;
    state->vlcBufferIndex = 0;
    state->imageBuffers[0] = (u32 *)0x801F1000;
    state->imageBuffers[1] = (u32 *)0x801F4000;
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
    StSetRing((u8 *)0x80180000, 32);
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
    if (D_8013B611 > 1) {
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
            DecDCTvlc2(frame, state->vlcBuffers[state->vlcBufferIndex], (u16 *)0x80190000);
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
    if (header->frameNumber >= D_800EBFE4[D_8013AF60].endFrame) D_8013B610 = 1;
    if (D_8013AF54 != header->width || D_8013AF56 != header->height) {
        RECT rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = 480;
        rect.h = 480;
        ClearImage(&rect, 0, 0, 0);
        D_8013AF54 = header->width;
        D_8013AF56 = header->height;
    }
    state->frames[0].w = state->frames[1].w = D_8013AF54 * 3 / 2;
    state->slice.h = state->frames[0].h = state->frames[1].h = D_8013AF56;
    return data;
}

void waitForMovieOutput(MovieDecodeState *state) {
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
