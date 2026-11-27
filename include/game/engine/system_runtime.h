#pragma once

#include "game/io/memory_card.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/engine/game_loop.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libmcrd.h"
#include "psx/libc.h"

#include "game/io/movie_stream.h"

extern u8 D_80120200[16];
extern u8 D_80120210[16];
extern s16 *D_801203C4[10];


extern s16 gMemoryCardTaskState;
extern u8 D_8013B562 __attribute__((section(".data")));
extern s16 gSelectedCourseMenuItem;
extern s16 selectedCourseMenuItemAbsolute __attribute__((section(".data")));
extern u8 D_8013B1AA;
extern u8 D_8013B1AB;
extern u16 D_8013AFE8;

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
void initializePaletteTransition(void);
void updatePaletteTransition(void);
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
