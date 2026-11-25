#pragma once

#include "game/io/memory_card.h"
#include "game/engine/task.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libmcrd.h"

#include "game/io/movie_stream.h"

extern u8 D_80120200[16];
extern u8 D_80120210[16];
extern s16 *D_801203C4[10];


extern char D_8013AF3C[];
extern s32 D_8013B600[2];
extern struct DIRENTRY D_8013B700[2][15];
extern s16 D_8013AF3E;
extern u16 D_8013AFAA;
extern u8 D_8013B562 __attribute__((section(".data")));
extern s16 D_8013B104;
extern u8 D_8013AFE2;
extern u8 D_8013B1AA;
extern u8 D_8013B1AB;
extern u16 D_8013AFE8;
extern void *gMemoryCardLoadDestination;
extern u8 D_8013AF3A;

int strcmp(const char *, const char *);

void func_800A89F4(void);
void func_800131B4(void);
void initializeGameSystems(void);
void runGameLoop(void);
void initializeNewGameSaveData(void);
void finishTaskTransitionAfterCountdown(void);
void resetStateAndBeginResourceLoadTransition(u8 loadResources);
void initializeRaceRestartTask(void);
void initializeRacePresentationTask(void);
void updateThreeFrameScreenBlend(void);
void *memcpy(void *destination, const void *source, unsigned int size);
void initializePaletteTransition(void);
void updatePaletteTransition(void);
void selectMemoryCardLoadFile(u8 slot, void *destination);
void pollMemoryCardBufferReadTask(void);
void waitThenStartSplitScreenTransition(void);
void initializeDefaultSceneLighting(void);
void pollMemoryCardReadTask(void);
void captureMemoryCardSaveIcon(void);
s32 pollMemoryCardAvailability(void);

void pollMemoryCardStatusTask(void);
void finishRaceExitAfterCountdown(void);
void advanceTaskWhenFlagAFE4Bit8Set(void);
void startResourceLoadWhenFlagAFE4Bit9Set(void);
void initializeRaceRulesAndCounters(void);
