#pragma once

#include "game/memory_card.h"
#include "game/task.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libmcrd.h"

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
extern u8 D_80120200[16];
extern u8 D_80120210[16];
extern s16 *D_801203C4[10];

extern s32 StCdIntrFlag __attribute__((section(".data")));
extern u8 D_8013B611;

extern char D_8013AF3C[];
extern s32 D_8013B600[2];
extern struct DIRENTRY D_8013B700[2][15];
extern s16 D_8013AF3E;
extern u16 D_8013AFAA __attribute__((section(".data")));
extern u8 D_8013B562 __attribute__((section(".data")));
extern u8 D_80120EA4[];
extern s16 D_8013B104 __attribute__((section(".data")));
extern u8 D_8013AFE2;
extern u8 D_8013B1AB __attribute__((section(".data")));
extern u16 D_8013AFE8;
extern void *D_8013AF4C;
extern void *gMemoryCardLoadDestination;
extern u8 D_8013AF3A;

int strcmp(const char *, const char *);

void func_800A89F4(void);
void StCdInterrupt(void);
u32 *getNextMovieStreamFrame(MovieDecodeState *state);
void func_800131B4(void);
void func_8001AB3C(void);
void func_80019D3C(s32 movieIndex);
void func_8001A540(void);
void startMovieCdRead(CdlLOC *location);
void func_8002EE0C(void);
void func_8001DB7C(void);
void finishTaskTransitionAfterCountdown(void);
void resetStateAndBeginResourceLoadTransition(u8 loadResources);
void initializeRaceRestartTask(void);
void func_8001BD50(void);
void updateThreeFrameScreenBlend(void);
void *memcpy(void *destination, const void *source, unsigned int size);
u16 func_800162C8(void);
void func_80016A7C(void);
s32 func_80017268(void);
void selectMemoryCardLoadFile(u8 slot, void *destination);
void pollMemoryCardBufferReadTask(void);
void func_8001C0DC(void);
void func_8003DEE0(void);
void func_8001B858(void);
void func_8002FFC0(void);
s32 func_80014838(TaskCallback callback);
void pollMemoryCardReadTask(void);
void captureMemoryCardSaveIcon(void);
s32 func_80019288(void);
s32 func_80019670(void);
void pollMemoryCardStatusTask(void);
void func_8001D100(void);
void advanceTaskWhenFlagAFE4Bit8Set(void);
void startResourceLoadWhenFlagAFE4Bit9Set(void);
void func_80040A60(void);

