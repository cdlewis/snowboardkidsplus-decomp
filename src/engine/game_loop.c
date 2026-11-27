#include "psx/libetc.h"
#include "common.h"
#include "game/engine/task.h"
#include "game/engine/game_task_scheduler.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/engine/game_loop.h"
#include "game/io/cd_queue.h"
#include "game/audio/music.h"
#include "game/io/controller.h"
#include "game/race/camera/race_camera.h"
#include "psx/libetc.h"
#include "game/engine/system_runtime.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

/* Reused workspace views published by initializeGameSystems. */
typedef struct GameScratchpad {
    VECTOR vectors[2];
    SVECTOR shortVectors[2];
    MATRIX matrices[2];
    u8 unk70[0x50];
    u8 unkC0[0x50];
    MATRIX matrixPool[16];
} GameScratchpad;

#define GAME_SCRATCHPAD ((GameScratchpad *)getScratchAddr(0))

extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

GsOT gMainOrderingTables[2] __attribute__((section(".data.render_state"))) = {0};
GsOT gPlayer1OrderingTables[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC040[2] __attribute__((section(".data.render_state"))) = {0};
GsOT gPlayer1ModelOrderingTables[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC090[2] __attribute__((section(".data.render_state"))) = {0};
GsOT gPlayer2OrderingTables[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC0E0[2] __attribute__((section(".data.render_state"))) = {0};
GsOT gPlayer2ModelOrderingTables[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC130[2] __attribute__((section(".data.render_state"))) = {0};
RECT gViewClipRects[3] __attribute__((section(".data.render_state"))) = {0};
MATRIX *D_800EC170[16] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC1B0[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC1D8[2] __attribute__((section(".data.render_state"))) = {0};
u32 D_800EC200[8192] __attribute__((section(".data.render_buffers"))) = {0};
u32 D_800F4200[8192] __attribute__((section(".data.render_buffers"))) = {0};
u32 D_800FC200[4096] __attribute__((section(".data.render_buffers"))) = {0};
u8 D_80100200[2][0x10000] __attribute__((section(".data.render_buffers"))) = {0};

extern u32 gSynchronizedFrameCount;

void func_8001AB34(void);

void runGameLoop(void) {
    GameTask *nextTask;
    u16 waitFlags;
    TaskCallback callback;

    while (1) {
        gFrameCounter = ((u16)gFrameCounter + 1) & 0xFFF;
        updateControllers();
        if (gGameStateFlags & 0x5800) {
            if (gFrameCounter != 0) {
                DrawSync(0);
            }
            if (gGameStateFlags & 0x800) {
                gGameStateFlags |= 0x400;
            }
            if (gGameStateFlags & 0x1000) {
                gGameStateFlags &= 0xFBFF;
                DrawSync(0);
                GsSetOffset(gDisplayWidth >> 1, gDisplayHeight >> 1);
                GsSetClip(&gViewClipRects[0]);
            }
            if (gGameStateFlags & 0x4000) {
                gGameStateFlags |= 0x2000;
            }
            gGameStateFlags &= 0xA7FF;
            gFrameCounter = (u16)gFrameCounter & 0xFFE;
        }
        if (!(gGameStateFlags & 0x400)) {
            gActiveGraphicsBuffer = GsGetActiveBuff();
        } else {
            gActiveGraphicsBuffer = (u16)gFrameCounter & 1;
        }
        GsSetWorkBase((PACKET *)D_80100200[gActiveGraphicsBuffer]);
        GsClearOt(0, 0, &gMainOrderingTables[gActiveGraphicsBuffer]);
        if (!(gGameStateFlags & 0x400) || !((u16)gFrameCounter & 1)) {
            GsClearOt(0, 0, &gPlayer1OrderingTables[gActiveGraphicsBuffer]);
            gCurrentPlayerOrderingTable = gPlayer1OrderingTables[gActiveGraphicsBuffer].org;
            D_8013AF70 = D_800EC1B0[gActiveGraphicsBuffer].org;
            D_8013AF74 = D_800EC090[gActiveGraphicsBuffer].org;
        } else {
            GsClearOt(0, 0, &gPlayer2OrderingTables[gActiveGraphicsBuffer]);
            gCurrentPlayerOrderingTable = gPlayer2OrderingTables[gActiveGraphicsBuffer].org;
            D_8013AF70 = D_800EC1D8[gActiveGraphicsBuffer].org;
            D_8013AF74 = D_800EC130[gActiveGraphicsBuffer].org;
        }
        gCurrentTask = gGameTasks;
        do {
            waitFlags = gCurrentTask->unk10;
            if (waitFlags & 3) {
                if ((waitFlags & 1) && !(currentTaskHasChildren() & 0xFFFF)) {
                    gCurrentTask->unk10 &= 0xFFFE;
                }
                if ((gCurrentTask->unk10 & 2) && (isCdCommandQueueIdle() == 1)) {
                    gCurrentTask->unk10 &= 0xFFFD;
                }
                if (!(gCurrentTask->unk10 & 3)) {
                    goto run_task;
                }
            } else {
            run_task:
                callback = gCurrentTask->callback;
                D_8013AFC8 = callback;
                gCurrentTaskParent = gCurrentTask->parent;
                callback();
            }
            nextTask = gCurrentTask->next;
            gCurrentTask = nextTask;
        } while (nextTask != NULL);
        updateCdRequestQueue();
        updateCdCommandQueue();
        updateCdVolumeFade();
        updateMusicSequenceFade();
        if ((GsDISPENV.disp.w != gDisplayWidth) || (GsDISPENV.disp.h != gDisplayHeight)) {
            GsInitGraph2(gDisplayWidth, gDisplayHeight, 4, 1, 0);
            GsDefDispBuff2(0, 0, 0, 0x100);
            GsInit3D();
        }
        if (!(gGameStateFlags & 0x400)) {
            waitForFrameHcount(0x190U);
            updatePaletteRecords();
            GsSwapDispBuff();
            GsSortClear(0, 0, 0, &gPlayer1OrderingTables[gActiveGraphicsBuffer]);
            GsSetClip(&gViewClipRects[0]);
            GsDrawOt(&gPlayer1OrderingTables[gActiveGraphicsBuffer]);
            GsDrawOt(&gMainOrderingTables[gActiveGraphicsBuffer]);
        } else {
            if (!((u16)gFrameCounter & 1)) {
                waitForFrameHcount(0x384U);
                updatePaletteRecords();
                GsSwapDispBuff();
                GsSortClear(0, 0, 0, &gPlayer1OrderingTables[gActiveGraphicsBuffer]);
                GsSetOffset((gDisplayWidth >> 1) - (gPlayerViewWidthReduction / 2),
                            (gDisplayHeight >> 1) - gPlayerViewVerticalOffset);
                GsSetClip(&gViewClipRects[1]);
                GsDrawOt(&gPlayer1OrderingTables[gActiveGraphicsBuffer]);
                continue;
            }
            updatePaletteRecords();
            DrawSync(0);
            GsSetOffset((gDisplayWidth >> 1) - (gPlayerViewWidthReduction / 2),
                        ((gDisplayHeight >> 1) + gPlayerViewVerticalOffset) - 8);
            GsSetClip(&gViewClipRects[2]);
            GsDrawOt(&gPlayer2OrderingTables[gActiveGraphicsBuffer]);
            DrawSync(0);
            GsSetOffset(gDisplayWidth >> 1, gDisplayHeight >> 1);
            GsSetClip(&gViewClipRects[0]);
            GsDrawOt(&gMainOrderingTables[gActiveGraphicsBuffer]);
        }
    }
}

void func_8001AB34(void) {}

void initializeGameSystems(void) {
    u8 matrixIndex;
    RECT *clip;

    gScratchVector0 = &GAME_SCRATCHPAD->vectors[0];
    gScratchVector1 = &GAME_SCRATCHPAD->vectors[1];
    gScratchShortVector0 = &GAME_SCRATCHPAD->shortVectors[0];
    gScratchShortVector1 = &GAME_SCRATCHPAD->shortVectors[1];
    gScratchMatrix0 = &GAME_SCRATCHPAD->matrices[0];
    D_8013AF9C = &GAME_SCRATCHPAD->matrices[1];
    D_8013AFA0 = GAME_SCRATCHPAD->unk70;
    D_8013AFA4 = GAME_SCRATCHPAD->unkC0;
    matrixIndex = 0;
    do {
        D_800EC170[matrixIndex] = &GAME_SCRATCHPAD->matrixPool[matrixIndex];
        matrixIndex += 1;
    } while (matrixIndex < 16);
    gDisplayWidth = 0x140;
    gDisplayHeight = 0xF0;
    D_8013AF84 = 0x100;
    initializeCdFiles();
    initializeSoundSystem(0);
    startSoundSystem();
    gSynchronizedFrameCount = 0;
    VSyncCallback(func_8001AB34);
    ResetGraph(0);
    resetGameplayRng();
    resetPaletteRecords();
    initializeMemoryCardSaveHeader();
    initializeControllers();
    setControllerFlag0(0U);
    setControllerFlag0(1U);
    initializeTaskScheduler();
    initDebugTextSprite();
    GsInitGraph(gDisplayWidth, gDisplayHeight, 4, 1, 0);
    GsDefDispBuff(0, 0, 0, 0x100);
    GsInit3D();
    clip = gViewClipRects;
    clip->x = 0;
    clip->y = 0;
    gMainOrderingTables[1].length = 0xC;
    gMainOrderingTables->length = 0xC;
    gMainOrderingTables->org = D_800EC200;
    gMainOrderingTables[1].org = D_800EC200 + 0x1000;
    gPlayer1OrderingTables[1].length = 0xC;
    gPlayer1OrderingTables->length = 0xC;
    D_800EC040[1].length = 0xC;
    D_800EC040[0].length = 0xC;
    gPlayer1ModelOrderingTables[1].length = 0xC;
    gPlayer1ModelOrderingTables[0].length = 0xC;
    D_800EC1B0[1].length = 0xC;
    D_800EC1B0[0].length = 0xC;
    D_800EC090[1].length = 0xC;
    D_800EC090[0].length = 0xC;
    gPlayer1OrderingTables->org = D_800F4200;
    gPlayer1OrderingTables[1].org = D_800F4200 + 0x1000;
    D_800EC040[0].org = (D_800F4200 + 0x7);
    D_800EC040[1].org = (u32 *)(D_800F4200 + 0x1007);
    gPlayer1ModelOrderingTables[0].org = (D_800F4200 + 0x1);
    gPlayer1ModelOrderingTables[1].org = (u32 *)(D_800F4200 + 0x1001);
    D_800EC1B0[0].org = (D_800F4200 + 0x3);
    D_800EC1B0[1].org = (u32 *)(D_800F4200 + 0x1003);
    D_800EC090[0].org = (D_800F4200 + 0xa);
    D_800EC090[1].org = (u32 *)(D_800F4200 + 0x100a);
    gPlayer2OrderingTables[1].length = 0xC;
    gPlayer2OrderingTables[0].length = 0xC;
    D_800EC0E0[1].length = 0xC;
    clip->w = gDisplayWidth;
    clip->h = gDisplayHeight;
    D_800EC0E0[0].length = 0xC;
    gPlayer2ModelOrderingTables[1].length = 0xC;
    gPlayer2ModelOrderingTables[0].length = 0xC;
    D_800EC1D8[1].length = 0xC;
    D_800EC1D8[0].length = 0xC;
    D_800EC130[1].length = 0xC;
    D_800EC130[0].length = 0xC;
    gPlayer2OrderingTables[0].org = D_800F4200;
    gPlayer2OrderingTables[1].org = D_800FC200;
    D_800EC0E0[1].org = (void *)(D_800FC200 + 0x7);
    D_800EC0E0[0].org = (D_800F4200 + 0x7);
    gPlayer2ModelOrderingTables[1].org = (void *)(D_800FC200 + 0x1);
    gPlayer2ModelOrderingTables[0].org = (D_800F4200 + 0x1);
    D_800EC1D8[0].org = (D_800F4200 + 0x3);
    D_800EC1D8[1].org = (void *)(D_800FC200 + 0x3);
    D_800EC130[0].org = (D_800F4200 + 0xa);
    D_800EC130[1].org = (void *)(D_800FC200 + 0xa);
    initPlayerCamera(0U);
    initializeDefaultSceneLighting();
    initPrimitiveTemplates();
}
