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



INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_80019670);

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
        func_80018AA4();
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
    s32 result = func_80019670();
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

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001A540);

void func_8001AB34(void) {
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001AB3C);

void runMovieAndGameLoop(void) {
    func_800131B4();
    while (1) {
        func_8001AB3C();
        func_80019D3C(0);
        func_80019D3C(1);
        func_8001A540();
        ResetGraph(3);
        StopCallback();
    }
}

void initCurrentPlayerModelParts(void) {
    s32 i;
    s16 *values;
    RacePlayer *player;
    for (i = 0; i < 14; i++) {
        D_8013B0D8->unk13E[i] = D_80120200[i];
        D_8013B0D8->unk14C[i] = D_80120210[i];
    }
    values = D_801203C4[D_8013B0D8->unk014];
    player = D_8013B0D8;
    for (i = 0; i < 14; i++) {
        player->unk5BC[i].x = player->unk5BC[i].y = player->unk5BC[i].z = 0;
        player->unk62C[i].x = *values++;
        player->unk62C[i].y = -*values++;
        player->unk62C[i].z = -*values++;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001AF64);

void sortLitObject(GsDOBJ2 *object) {
    GsGetLw(object->coord2, D_8013AF98);
    GsSetLightMatrix(D_8013AF98);
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    GsSortObject4(object, &D_800EC018[D_8013AF78], 2, (u32 *)0x1F800000);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001B4EC);

void initializeGameOptionTask(void) {
    queueCdSectorCommand(2, 0x130, 0x32, (void *)0x80180000, 0);
    D_8013B562 = 0;
    D_8013AFD7 = 0;
    D_8013AE88->data18 = 0;
    func_8002EE0C();
    resetGameOptionState();
    setCurrentTaskCallback(initializeGameOptionViewTask);
    D_8013AE88->unk10 |= 2;
}

void initializeGameOptionViewTask(void) {
    initPlayerCamera(0);
    func_8003DEE0();
    setControllerFlag0(0);
    setControllerFlag0(1);
    setCurrentTaskCallback(func_8001B858);
    func_80014838(func_8002FFC0);
    D_8013AE88->unk10 |= 1;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001B858);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001BD50);

void beginTask60TickCountdown(void) {
    D_8013AE88->data1A = 60;
    setCurrentTaskCallback(func_8001C0DC);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001C0DC);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001C128);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001C2E0);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001C85C);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001C9F0);

void begin30TickTransitionOnFlag10(void) {
    if (D_8013AFE4 & 0x10) {
        D_8013AE88->data18 = 30;
        func_80070274(2, 0, 30);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(func_8001D100);
    }
}

void initializeRaceRestartTask(void) {
    u16 i;
    D_8013AF08[1] = 0;
    D_8013AF08[0] = 0;
    D_8013AFE4 = 1;
    stopPlayerSound(0);
    if (gameOptionPlayerModeGp == 2) {
        stopPlayerSound(1);
    }
    for (i = 0; i < activeRacePlayerCountGp; i++) {
        D_80120E90[i].unk024 = 1;
    }
    initializeActiveRacePlayers();
    func_80040A60();
    setCurrentTaskCallback(func_8001BD50);
}

void advanceTaskWhenFlagAFE4Bit8Set(void) {
    if (D_8013AFE4 & 0x100) {
        D_8013AFE4 = 0;
        D_8013AFAA |= 0x90;
        setCurrentTaskCallback(initializeGameOptionViewTask);
    }
}

void startResourceLoadWhenFlagAFE4Bit9Set(void) {
    if (D_8013AFE4 & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    }
}

void beginResourceLoadOrUnlockNotification(void) {
    D_8013AFAA |= 0x48;
    if (D_8013AFE4 & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    } else {
        startUnlockNotification(D_80120EA4[0] + 1);
        setCurrentTaskCallback(startResourceLoadWhenFlagAFE4Bit9Set);
    }
}

void resetStateAndBeginResourceLoadTransition(u8 requestLoad) {
    GameTask *savedTask;
    resetPendingCdRequests();
    D_8013AFD6 = 0;
    D_8013B20C[0] = 0;
    resetPaletteRecords();
    func_800149E0();
    savedTask = D_8013AE88;
    D_8013AE88 = D_8013B304;
    func_800149E0();
    D_8013AE88 = savedTask;
    D_8013AFAA |= 0x90;
    if (requestLoad) {
        D_8013AFE4 |= 0x200;
    }
    D_8013AFE4 |= 0x80;
    setCurrentTaskCallback(beginResourceLoadOrUnlockNotification);
}

void startTaskTransitionWhenFlagAEE8Set(void) {
    if (D_8013AE88->data18 == 0) {
        if (D_8013AEE8[0] & 0xB0) {
            D_8013AE88->data18 = 30;
            func_80070274(2, 0, 30);
            setCurrentTaskCallback(finishTaskTransitionAfterCountdown);
        }
    } else {
        D_8013AE88->data18--;
    }
}

void finishTaskTransitionAfterCountdown(void) {
    if (D_8013AE88->data18 == 0) {
        resetPendingCdRequests();
        D_8013AFAA |= 0x90;
        D_8013AFD6 = 0;
        D_8013B20C[0] = 0;
        D_8013AFE4 &= ~0x10;
        resetPaletteRecords();
        func_800149E0();
        if (D_8013AE88->data1A == 3) {
            resetStateAndBeginResourceLoadTransition(0);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        D_8013AE88->data18--;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001D100);

void updateThreeFrameScreenBlend(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013B1AB) {
            setCurrentTaskCallback(initializeRaceRestartTask);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    }
}

void startThreeFrameScreenBlend(void) {
    func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    D_8013AE88->data18 = 3;
    setCurrentTaskCallback(updateThreeFrameScreenBlend);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001D528);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001D678);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001D89C);

void animateAlternatingVramRow(void) {
    RECT rect;
    if (--D_8013AE88->data1A == 0) {
        rect.x = 640;
        if (D_8013AE88->data18 == 0) {
            rect.y = 503;
            D_8013AE88->data1A = 4;
        } else {
            rect.y = 504;
            D_8013AE88->data1A = 6;
        }
        rect.w = 16;
        rect.h = 1;
        MoveImage(&rect, 640, 502);
        D_8013AE88->data18 ^= 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001D97C);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001DB7C);

void initializeRaceTaskState(void) {
    D_8013AFE4 = 1;
    if (D_8013AFE2 == 2) {
        resetSecondaryRng();
    }
    initializeActiveRacePlayers();
    func_80040A60();
    func_800149A8();
}

void initializeSelectedCourseTransition(void) {
    D_8013AFD0 = D_80123D80[D_8013AFCE].unk02;
    if (D_8013AFE2 != 0 || D_8013AFCD == 1 || D_8013AFDA == 0) {
        func_80070274(1, 0, 30);
        func_80071490(3, D_8013B104, 0);
        setCurrentTaskCallback(func_8001DB7C);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001E0C0);

void processTaskWhenFlagAFAABit7Set(void) {
    if (D_8013AFAA & 0x80) {
        func_800149E0();
        D_8013AFAA &= ~0xC0;
        setCurrentTaskCallback(func_8001E0C0);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001E148);

void resetTaskCallbackOnFlag4(void) {
    if (D_8013AFAA & 0x10) {
        func_800149E0();
        setCurrentTaskCallback(func_8001E148);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001E1B8);

void restoreSavedStateAndHandleFlag4(void) {
    if (D_8013AFD6) {
        D_8013AEEC[0] = D_8013AFE8;
    }
    if (D_8013AFAA & 0x10) {
        D_8013AFAA &= ~0x18;
        setCurrentTaskCallback(func_8001E1B8);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001E244);

INCLUDE_ASM("asm/nonmatchings/main_remaining_tail", func_8001E3E4);

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate) {
    D_8013AF98->t[0] = coordinate->coord.t[0];
    D_8013AF98->t[1] = coordinate->coord.t[1];
    D_8013AF98->t[2] = coordinate->coord.t[2];
    RotMatrixZYX(rotation, D_8013AF98);
    coordinate->coord = *D_8013AF98;
    coordinate->flg = 0;
}

/* This routine accesses the shared player pointer through GP. */
#undef D_8013B0D8

u16 advanceCurrentPlayerInputSequence(u8 sequence[][2], s16 slot) {
    u8 input = D_8013AEEC[D_8013AE88->data18] >> 12;
    switch (D_8013B0D8->unk0CA[slot]) {
    case 0:
        if (input != sequence[D_8013B0D8->unk0D6[slot]][0]) {
            D_8013B0D8->unk0D6[slot] = 0;
            break;
        }
        if (sequence[D_8013B0D8->unk0D6[slot]][1] != 0)
            return sequence[D_8013B0D8->unk0D6[slot]][1];
        D_8013B0D8->unk0CA[slot]++;
    case 1:
        if (input != sequence[D_8013B0D8->unk0D6[slot]][0] && input != 0) {
            D_8013B0D8->unk0CA[slot] = 0;
            D_8013B0D8->unk0D6[slot]++;
        }
        break;
    }
    return 0;
}
