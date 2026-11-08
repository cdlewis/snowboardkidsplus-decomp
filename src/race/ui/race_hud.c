#include "../race_presentation_internal.h"

const char gLapTimeLabel[12] __attribute__((section(".rodata.lap_time_label"), aligned(4))) = "LAP TIME";

const char gLapTimeFormat[20] __attribute__((section(".rodata.lap_time_format"), aligned(4))) = "%2.2d'%2.2d\"%2.2d";

const char gBestLapLabel[16] __attribute__((section(".rodata.best_lap_label"), aligned(4))) = "BEST LAP";

void drawLapTimeHud(void) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    char text[16];
    s32 icon;
    setSpriteColor(128,128,128);
    func_8006DFB4(72,80,2,1,gRaceElapsedTime.minutes,1,-1,ot,1);
    func_8006DFB4(96,80,2,1,gRaceElapsedTime.seconds,1,-1,ot,1);
    func_8006DFB4(120,80,2,1,gRaceElapsedTime.centiseconds >> 8,1,-1,ot,1);
    func_8006C998(88,80,25,1,-1,ot,1);
    func_8006C998(112,80,24,1,-1,ot,1);
    drawLimitedText8x8(72,71,10,gLapTimeLabel,5,-1,ot,1);
    sprintf(text,gLapTimeFormat,((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][0].minutes,((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][0].seconds,((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][0].centiseconds >> 8);
    drawLimitedText8x8(72,-88,20,text,7,-1,ot,1);
    drawLimitedText8x8(72,-97,10,gBestLapLabel,7,-1,ot,1);
    icon = (s8)gRacePlayers[0].unk936[11];
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
    drawMenuKindSprite(-32, -96, (s8)player->unk936[8], (s8)player->unk936[10], ot, 1);
    drawPaletteMappedIcon(0, -96, (s8)player->unk936[11], ot, 1);
    frame = (s16)gFrameCounter % 12;
    func_8006C998(120, 80, (s16)(frame / 2) + 472, 7, -1, ot, 1);
    palette = player->unk01C < 100 ? 2 : 3;
    func_8006DFB4(78, 80, 5, 0, player->unk01C, palette, -1, ot, 1);
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

INCLUDE_ASM("asm/nonmatchings/race/ui/race_hud", func_80043C28);

INCLUDE_ASM("asm/nonmatchings/race/ui/race_hud", func_80043C5C);

void resetControllersAndDeleteCurrentTask(void) {
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetControllerEffects();
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_hud", func_80043FBC);

s16 isFirstPlayerUsingCallback26D10(void) {
    return gRacePlayers[0].task->callback == updateCurrentPlayerTaskCountdown;
}
