#include "../race_presentation_internal.h"
#include "../race_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/menu/selected_records.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_hud.h"
#include "game/save/save_data.h"
#include "psx/libgs.h"

#define PAUSE_MENU_START 0x0800
#define PAUSE_MENU_UP 0x1000
#define PAUSE_MENU_DOWN 0x4000
#define PAUSE_MENU_CONFIRM 0x00B0
#define PAUSE_MENU_RESUME_PENDING 0x80

extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 D_8013AFCE __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern u32 gGameOptionFlags __attribute__((section(".data")));
extern u8 D_8013AFD5 __attribute__((section(".data")));
extern u8 D_8013AFD3 __attribute__((section(".data")));

const char gLapTimeLabel[12] __attribute__((section(".rodata.lap_time_label"), aligned(4))) = "LAP TIME";

const char gLapTimeFormat[20] __attribute__((section(".rodata.lap_time_format"), aligned(4))) = "%2.2d'%2.2d\"%2.2d";

const char gBestLapLabel[16] __attribute__((section(".rodata.best_lap_label"), aligned(4))) = "BEST LAP";

void func_80043FBC(void);

void drawLapTimeHud(void) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    char text[16];
    s32 icon;
    setSpriteColor(128,128,128);
    drawHudDecimalNumber8x16(72,80,2,1,gRaceElapsedTime.minutes,1,-1,ot,1);
    drawHudDecimalNumber8x16(96,80,2,1,gRaceElapsedTime.seconds,1,-1,ot,1);
    drawHudDecimalNumber8x16(120,80,2,1,gRaceElapsedTime.centiseconds >> 8,1,-1,ot,1);
    func_8006C998(88,80,25,1,-1,ot,1);
    func_8006C998(112,80,24,1,-1,ot,1);
    drawLimitedText8x8(72,71,10,gLapTimeLabel,5,-1,ot,1);
    sprintf(text,gLapTimeFormat,((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][0].minutes,((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][0].seconds,((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][0].centiseconds >> 8);
    drawLimitedText8x8(72,-88,20,text,7,-1,ot,1);
    drawLimitedText8x8(72,-97,10,gBestLapLabel,7,-1,ot,1);
    icon = (s8)gRacePlayers[0].heldItem;
    if (icon) drawPaletteMappedIcon(-16,-96,icon,ot,1);
    func_8006C998(-116,64,425,1,-1,ot,1);
    func_8006C998(-136,64,420,14,-1,ot,1);
}

void drawFirstPlayerStatusHud(void) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    RacePlayer *player;
    s16 frame;
    s32 palette;
    setSpriteColor(128, 128, 128);
    player = gRacePlayers;
    drawMenuKindSprite(-32, -96, player->unk93E, player->unk940, ot, 1);
    drawPaletteMappedIcon(0, -96, (s8)player->heldItem, ot, 1);
    frame = gFrameCounter % 12;
    func_8006C998(120, 80, (s16)(frame / 2) + 472, 7, -1, ot, 1);
    palette = player->unk01C < 100 ? 2 : 3;
    drawHudDecimalNumber8x16(78, 80, 5, 0, player->unk01C, palette, -1, ot, 1);
    func_8006C998(-116, 80, 426, 1, -1, ot, 1);
    func_8006C998(-136, 64, 420, 14, -1, ot, 1);
}

void advanceRaceElapsedTime(void) {
    if (!D_8013AFD3) {
        gRaceElapsedTime.centiseconds += 0x358;
        if (gRaceElapsedTime.centiseconds > 0x6400) {
            gRaceElapsedTime.centiseconds = 0;
            gRaceElapsedTime.seconds++;
            if (gRaceElapsedTime.seconds >= 60) {
                gRaceElapsedTime.seconds = 0;
                gRaceElapsedTime.minutes++;
                if (gRaceElapsedTime.minutes >= 99) {
                    gRaceElapsedTime.centiseconds = 0x6300;
                    gRaceElapsedTime.seconds = 59;
                    gRaceElapsedTime.minutes = 99;
                }
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_hud", func_80043A4C);

s32 calculateRaceTimeDifference(const RaceTime *a, const RaceTime *b, RaceTime *difference) {
    s32 first = (a->centiseconds & 0xFF00) + a->seconds * 25600 + a->minutes * 1536000;
    s32 second = (b->centiseconds & 0xFF00) + b->seconds * 25600 + b->minutes * 1536000;
    s32 smaller;
    if (first >= second) {
        smaller = 0;
        first -= second;
    } else {
        smaller = 1;
        first = second - first;
    }
    difference->centiseconds = first % 25600;
    first /= 25600;
    difference->seconds = first % 60;
    first /= 60;
    difference->minutes = first % 100;
    return smaller;
}

void initializeRacePauseMenuTask(void) {
    D_8013AFD5 = 0;
    D_8013B1AA = 0;
    D_8013B1AB = 0;
    setCurrentTaskCallback(updateRacePauseMenu);
}

void updateRacePauseMenu(void) {
    u8 pause = D_8013AFD3;
    s16 player = pause - 1;
    u8 resumeState;

    if (D_8013AEE8[player] & PAUSE_MENU_START) {
        resumeState = pause | PAUSE_MENU_RESUME_PENDING;
        goto resume;
    }
    if ((D_8013AEE8[player] & PAUSE_MENU_UP) && D_8013AFD5 != 0) {
        D_8013AFD5--;
        playSoundEffect(0, 0x100, 0x20);
    }
    if (D_8013AEE8[player] & PAUSE_MENU_DOWN) {
        if (D_8013AFD5 < 4) {
            D_8013AFD5++;
            playSoundEffect(0, 0x100, 0x20);
        }
    }

    switch (D_8013AFD5) {
    case 0:
        if (D_8013AEE8[player] & PAUSE_MENU_CONFIRM) {
            playSoundEffect(0, 0, 0x20);
            resumeState = D_8013AFD3 | PAUSE_MENU_RESUME_PENDING;
resume:
            D_8013AFD3 = resumeState;
            deleteCurrentTaskSubtree();
            return;
        }
        break;
    case 1:
        if (D_8013AEE8[player] & PAUSE_MENU_CONFIRM) {
            playSoundEffect(0, 0, 0x20);
            D_8013B1AA = 1;
            D_8013AFD3 |= PAUSE_MENU_RESUME_PENDING;
            resetControllersAndDeleteCurrentTask();
            return;
        }
        break;
    case 2:
        if (D_8013AEE8[player] & PAUSE_MENU_CONFIRM) {
            playSoundEffect(0, 0, 0x20);
            D_8013B1AB = 1;
            D_8013AFD3 |= PAUSE_MENU_RESUME_PENDING;
            resetControllersAndDeleteCurrentTask();
            return;
        }
        break;
    case 3:
        if (D_8013AEE8[player] & PAUSE_MENU_CONFIRM) {
            playSoundEffect(0, 0, 0x20);
            if (D_8013AEC4[player]) {
                clearControllerFlag5(player);
                D_8013AEC4[player] = 0;
            } else {
                setControllerFlag5(player);
                D_8013AEC4[player] = 0x20;
            }
        }
        break;
    case 4:
        if (D_8013AEE8[player] & PAUSE_MENU_CONFIRM) {
            playSoundEffect(0, 0, 0x20);
            if ((s16)getPlayerOptionFlag(player)) {
                gGameOptionFlags &= ~(0x10 << player);
            } else {
                gGameOptionFlags |= 0x10 << player;
            }
        }
        break;
    }

    func_80043FBC();
}

void resetControllersAndDeleteCurrentTask(void) {
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetControllerEffects();
    deleteCurrentTaskSubtree();
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_hud", func_80043FBC);

s16 isFirstPlayerUsingCallback26D10(void) {
    return gRacePlayers[0].task->callback == updateCurrentPlayerTaskCountdown;
}
