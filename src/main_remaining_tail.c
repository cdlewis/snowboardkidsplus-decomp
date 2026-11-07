#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/graphics.h"
#include "game/coordinate.h"
#include "game/race_camera.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/cd_queue.h"
#include "game/controller.h"
#include "game/race_player.h"
#define GAME_OPTIONS_GP_ADDRESSING
#include "game/game_options.h"
#define SELECTED_RECORDS_GP_ADDRESSING
#include "game/selected_records.h"
#include "psx/libsnd.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"
#include "psx/libmcrd.h"
#include "psx/libcd.h"
#include "psx/libpress.h"

#include "main_remaining.h"

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



s32 pollMemoryCardWrite(void) {
    s32 command, result;
    D_8013B60C[D_8013AF38] = 0;
    if (D_8013B608[D_8013AF38] == 1) {
        switch (MemCardCreateFile(D_8013AF34, D_8013AF40, (s8)gMemoryCardSaveHeader.blockCount)) {
        case 0: readMemoryCardDirectory();
            D_8013B608[D_8013AF38] = 0; break;
        case 1: return 2;
        case 2: return 3;
        case 4: return 4;
        case 6: D_8013B608[D_8013AF38] = 0; break;
        case 7: return 5;
        default: return 0;
        }
    }
    switch (MemCardSync(1, &command, &result)) {
    case -1:
        MemCardWriteFile(D_8013AF34, D_8013AF40, &gMemoryCardSaveHeader, 0, gMemoryCardSaveCapacities[D_8013AF48]);
        break;
    case 0: break;
    case 1:
        if (command == 4) {
            switch (result) {
            case 0: return 1;
            case 1: return 2;
            case 2: return 3;
            case 3: MemCardAccept(D_8013AF34);
            case 5: D_8013B608[D_8013AF38] = 1; break;
            }
        }
        break;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001983C);

void resetMemoryCardTaskState(void) {
    D_8013AF3E = 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_800198C4);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_800198EC);

void beginMemoryCardBufferRead(void) {
    if (tryAcceptMemoryCard() == 1) {
        selectMemoryCardLoadFile(0, D_8013AF4C);
        setCurrentTaskCallback(pollMemoryCardBufferReadTask);
    }
}

void pollMemoryCardBufferReadTask(void) {
    switch (func_80019288()) {
    case 0: D_8013AF3E = 1; return;
    case 5: D_8013AF3E = 2; return;
    case 2: D_8013AF3E = 6; break;
    case 1: D_8013AF3E = 5; break;
    case 3: D_8013AF3E = 7; break;
    case 4: D_8013AF3E = 8; break;
    case 6: D_8013AF3E = 9; break;
    case 7: D_8013AF3E = 10; break;
    case 8: D_8013AF3E = 11; break;
    }
    func_800149A8();
}

s32 startMemoryCardReadTask(void) {
    if (D_8013AF3E == 5) {
        func_80014874(pollMemoryCardReadTask);
        D_8013AF3E = 3;
        return 1;
    }
    return 0;
}

void pollMemoryCardReadTask(void) {
    s32 result = pollMemoryCardRead();
    if (result != 0) {
        switch (result) {
        case 1: D_8013AF3E = 0; break;
        case 2: D_8013AF3E = 7; break;
        case 3: D_8013AF3E = 8; break;
        case 4: D_8013AF3E = 11; break;
        case 5: D_8013AF3E = 12; break;
        }
        func_800149A8();
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_80019AF4);

void waitForMemoryCardAcceptTask(void) {
    if (tryAcceptMemoryCard() == 1) {
        captureMemoryCardSaveIcon();
        setCurrentTaskCallback(pollMemoryCardStatusTask);
    }
}

void pollMemoryCardStatusTask(void) {
    switch (func_80019288()) {
    case 0: D_8013AF3E = 1; return;
    case 5: D_8013AF3E = 2; return;
    case 2: D_8013AF3E = 6; break;
    case 8: D_8013AF3E = 11; break;
    case 1: D_8013AF3E = 5; break;
    case 3: D_8013AF3E = 7; break;
    case 4: D_8013AF3E = 8; break;
    case 6: D_8013AF3E = 9; break;
    case 7: D_8013AF3E = 10; break;
    }
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_80019C40);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_80019C84);

void pollMemoryCardWriteTask(void) {
    s32 result = pollMemoryCardWrite();
    if (result != 0) {
        switch (result) {
        case 1: D_8013AF3E = 0; break;
        case 2: D_8013AF3E = 7; break;
        case 3: D_8013AF3E = 8; break;
        case 4: D_8013AF3E = 9; break;
        case 5: D_8013AF3E = 10; break;
        }
        func_800149A8();
    }
}


INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_80019D3C);

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

void waitForFrameHcount(u32 minimumHcount) {
    DrawSync(0);
    while ((u32)VSync(1) < minimumHcount) {
    }
    VSync(0);
    gSynchronizedFrameCount++;
}
