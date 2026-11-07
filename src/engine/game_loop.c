#define FRAME_COUNTER_GP_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define GRAPHICS_GP_ADDRESSING
#define COORDINATE_GP_ADDRESSING
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
#include "game/engine/linear_random.h"
#include "psx/libetc.h"
#include "game/engine/system_runtime.h"

GsOT D_800EBFF0[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC018[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC040[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC068[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC090[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC0B8[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC0E0[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC108[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC130[2] __attribute__((section(".data.render_state"))) = {0};
RECT D_800EC158[3] __attribute__((section(".data.render_state"))) = {0};
MATRIX *D_800EC170[16] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC1B0[2] __attribute__((section(".data.render_state"))) = {0};
GsOT D_800EC1D8[2] __attribute__((section(".data.render_state"))) = {0};
u32 D_800EC200[8192] __attribute__((section(".data.render_buffers"))) = {0};
u32 D_800F4200[8192] __attribute__((section(".data.render_buffers"))) = {0};
u32 D_800FC200[4096] __attribute__((section(".data.render_buffers"))) = {0};
u8 D_80100200[2][0x10000] __attribute__((section(".data.render_buffers"))) = {0};

void updateCdVolumeFade(void);
void func_80013F98(void);
void func_80016628(void);
void initializeControllers(void);
void initializeSoundSystem(s32);
void func_80049CDC(void);
void func_8001AB34(void);
void func_8003DEE0(void);

void runGameLoop(void) {
    GameTask *nextTask;
    u16 waitFlags;
    TaskCallback callback;

    while (1) {
        gFrameCounter = ((u16)gFrameCounter + 1) & 0xFFF;
        updateControllers();
        if (D_8013AFAA & 0x5800) {
            if (gFrameCounter != 0) {
                DrawSync(0);
            }
            if (D_8013AFAA & 0x800) {
                D_8013AFAA |= 0x400;
            }
            if (D_8013AFAA & 0x1000) {
                D_8013AFAA &= 0xFBFF;
                DrawSync(0);
                GsSetOffset((u16)D_8013AF7A >> 1, (u16)D_8013AF7C >> 1);
                GsSetClip(&D_800EC158[0]);
            }
            if (D_8013AFAA & 0x4000) {
                D_8013AFAA |= 0x2000;
            }
            D_8013AFAA &= 0xA7FF;
            gFrameCounter = (u16)gFrameCounter & 0xFFE;
        }
        if (!(D_8013AFAA & 0x400)) {
            D_8013AF78 = GsGetActiveBuff();
        } else {
            D_8013AF78 = (u16)gFrameCounter & 1;
        }
        func_800BF2C8((u32)D_80100200[D_8013AF78]);
        GsClearOt(0, 0, &D_800EBFF0[D_8013AF78]);
        if (!(D_8013AFAA & 0x400) || !((u16)gFrameCounter & 1)) {
            GsClearOt(0, 0, &D_800EC018[D_8013AF78]);
            D_8013AF6C = D_800EC018[D_8013AF78].org;
            D_8013AF70 = D_800EC1B0[D_8013AF78].org;
            D_8013AF74 = D_800EC090[D_8013AF78].org;
        } else {
            GsClearOt(0, 0, &D_800EC0B8[D_8013AF78]);
            D_8013AF6C = D_800EC0B8[D_8013AF78].org;
            D_8013AF70 = D_800EC1D8[D_8013AF78].org;
            D_8013AF74 = D_800EC130[D_8013AF78].org;
        }
        D_8013AE88 = gGameTasks;
        do {
            waitFlags = D_8013AE88->unk10;
            if (waitFlags & 3) {
                if ((waitFlags & 1) && !(func_80014B14() & 0xFFFF)) {
                    D_8013AE88->unk10 &= 0xFFFE;
                }
                if ((D_8013AE88->unk10 & 2) && (isCdCommandQueueIdle() == 1)) {
                    D_8013AE88->unk10 &= 0xFFFD;
                }
                if (!(D_8013AE88->unk10 & 3)) {
                    goto run_task;
                }
            } else {
            run_task:
                callback = D_8013AE88->callback;
                D_8013AFC8 = callback;
                D_8013AE98 = D_8013AE88->parent;
                callback();
            }
            nextTask = D_8013AE88->next;
            D_8013AE88 = nextTask;
        } while (nextTask != NULL);
        updateCdRequestQueue();
        func_80013F98();
        updateCdVolumeFade();
        updateMusicSequenceFade();
        if ((GsDISPENV.disp.w != D_8013AF7A) || (GsDISPENV.disp.h != D_8013AF7C)) {
            GsInitGraph2(D_8013AF7A, D_8013AF7C, 4, 1, 0);
            GsDefDispBuff2(0, 0, 0, 0x100);
            GsInit3D();
        }
        if (!(D_8013AFAA & 0x400)) {
            waitForFrameHcount(0x190U);
            func_80016628();
            GsSwapDispBuff();
            GsSortClear(0, 0, 0, &D_800EC018[D_8013AF78]);
            GsSetClip(&D_800EC158[0]);
            GsDrawOt(&D_800EC018[D_8013AF78]);
            GsDrawOt(&D_800EBFF0[D_8013AF78]);
        } else {
            if (!((u16)gFrameCounter & 1)) {
                waitForFrameHcount(0x384U);
                func_80016628();
                GsSwapDispBuff();
                GsSortClear(0, 0, 0, &D_800EC018[D_8013AF78]);
                GsSetOffset(((u16)D_8013AF7A >> 1) - ((s16)D_8013AF7E / 2),
                            ((u16)D_8013AF7C >> 1) - D_8013AF80);
                GsSetClip(&D_800EC158[1]);
                GsDrawOt(&D_800EC018[D_8013AF78]);
                continue;
            }
            func_80016628();
            DrawSync(0);
            GsSetOffset(((u16)D_8013AF7A >> 1) - ((s16)D_8013AF7E / 2),
                        (((u16)D_8013AF7C >> 1) + D_8013AF80) - 8);
            GsSetClip(&D_800EC158[2]);
            GsDrawOt(&D_800EC0B8[D_8013AF78]);
            DrawSync(0);
            GsSetOffset((u16)D_8013AF7A >> 1, (u16)D_8013AF7C >> 1);
            GsSetClip(&D_800EC158[0]);
            GsDrawOt(&D_800EBFF0[D_8013AF78]);
        }
    }
}

void func_8001AB34(void) {}

void initializeGameSystems(void) {
    u8 matrixIndex;
    RECT *clip;

    D_8013AF88 = (VECTOR *)0x1F800000;
    D_8013AF8C = (VECTOR *)0x1F800010;
    D_8013AF90 = (SVECTOR *)0x1F800020;
    D_8013AF94 = (SVECTOR *)0x1F800028;
    D_8013AF98 = (MATRIX *)0x1F800030;
    D_8013AF9C = (MATRIX *)0x1F800050;
    D_8013AFA0 = (u8 *)0x1F800070;
    D_8013AFA4 = (u8 *)0x1F8000C0;
    matrixIndex = 0;
    do {
        D_800EC170[matrixIndex] = (MATRIX *)(0x1F800110 + matrixIndex * sizeof(MATRIX));
        matrixIndex += 1;
    } while (matrixIndex < 16);
    D_8013AF7A = 0x140;
    D_8013AF7C = 0xF0;
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
    func_80049CDC();
    GsInitGraph(D_8013AF7A, D_8013AF7C, 4, 1, 0);
    GsDefDispBuff(0, 0, 0, 0x100);
    GsInit3D();
    clip = D_800EC158;
    clip->x = 0;
    clip->y = 0;
    D_800EBFF0[1].length = 0xC;
    D_800EBFF0->length = 0xC;
    D_800EBFF0->org = D_800EC200;
    D_800EBFF0[1].org = D_800EC200 + 0x1000;
    D_800EC018[1].length = 0xC;
    D_800EC018->length = 0xC;
    D_800EC040[1].length = 0xC;
    D_800EC040[0].length = 0xC;
    D_800EC068[1].length = 0xC;
    D_800EC068[0].length = 0xC;
    D_800EC1B0[1].length = 0xC;
    D_800EC1B0[0].length = 0xC;
    D_800EC090[1].length = 0xC;
    D_800EC090[0].length = 0xC;
    D_800EC018->org = D_800F4200;
    D_800EC018[1].org = D_800F4200 + 0x1000;
    D_800EC040[0].org = (D_800F4200 + 0x7);
    D_800EC040[1].org = (u32 *)(D_800F4200 + 0x1007);
    D_800EC068[0].org = (D_800F4200 + 0x1);
    D_800EC068[1].org = (u32 *)(D_800F4200 + 0x1001);
    D_800EC1B0[0].org = (D_800F4200 + 0x3);
    D_800EC1B0[1].org = (u32 *)(D_800F4200 + 0x1003);
    D_800EC090[0].org = (D_800F4200 + 0xa);
    D_800EC090[1].org = (u32 *)(D_800F4200 + 0x100a);
    D_800EC0B8[1].length = 0xC;
    D_800EC0B8[0].length = 0xC;
    D_800EC0E0[1].length = 0xC;
    clip->w = (u16)D_8013AF7A;
    clip->h = (u16)D_8013AF7C;
    D_800EC0E0[0].length = 0xC;
    D_800EC108[1].length = 0xC;
    D_800EC108[0].length = 0xC;
    D_800EC1D8[1].length = 0xC;
    D_800EC1D8[0].length = 0xC;
    D_800EC130[1].length = 0xC;
    D_800EC130[0].length = 0xC;
    D_800EC0B8[0].org = D_800F4200;
    D_800EC0B8[1].org = D_800FC200;
    D_800EC0E0[1].org = (void *)(D_800FC200 + 0x7);
    D_800EC0E0[0].org = (D_800F4200 + 0x7);
    D_800EC108[1].org = (void *)(D_800FC200 + 0x1);
    D_800EC108[0].org = (D_800F4200 + 0x1);
    D_800EC1D8[0].org = (D_800F4200 + 0x3);
    D_800EC1D8[1].org = (void *)(D_800FC200 + 0x3);
    D_800EC130[0].org = (D_800F4200 + 0xa);
    D_800EC130[1].org = (void *)(D_800FC200 + 0xa);
    initPlayerCamera(0U);
    func_8003DEE0();
    initPrimitiveTemplates();
}
