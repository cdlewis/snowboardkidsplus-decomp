#include "../race/race_presentation_internal.h"
#include "../race/race_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/controller.h"
#include "game/menu/selected_records.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_time.h"
#include "game/save/save_data.h"
#include "psx/libc.h"
#include "psx/libgs.h"

extern u8 D_8013AFE1 __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 D_8013AFCE __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));

const char gHitCountTimeOutLabel[12] __attribute__((section(".rodata.hit_count_time_out"), aligned(4))) = "TIME OUT";
char gHitCountTotalFormat[4] __attribute__((section(".data.hit_count_labels"), aligned(4))) = "/%d";
char gHitCountMarker[4] __attribute__((section(".data.hit_count_labels"), aligned(4))) = "*";
char gStatsTrickLabel[8] __attribute__((section(".data.stats_value_labels"), aligned(4))) = "-TRICK-";
char gStatsMakeLabel[8] __attribute__((section(".data.stats_value_labels"), aligned(4))) = "-MAKE-";

const char gHitPrizeOneGoldLabel[20] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "-HIT PRIZE(*1G)-";

const char gHitPrizeFiftyGoldLabel[20] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "-HIT PRIZE(*50G)-";

const char gPerfectHitLabel[16] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "  -PERFECT HIT-";

const char gRaceResultsTargetLabel[12] __attribute__((section(".rodata.race_results_labels"), aligned(4))) = "-TARGET-";
const char gRaceResultsTargetTimeFormat[20] __attribute__((section(".rodata.race_results_labels"), aligned(4))) = "%2.2d'%2.2d\"%2.2d";
const char gRaceResultsTimeDifferenceFormat[20] __attribute__((section(".rodata.race_results_labels"), aligned(4))) = "-%2.2d'%2.2d\"%2.2d";
const char gRaceResultsTimePrizeLabel[16] __attribute__((section(".rodata.race_results_labels"), aligned(4))) = "  -TIME PRIZE-";
const char gRaceResultsSpeedFanGetLabel[20] __attribute__((section(".rodata.race_results_labels"), aligned(4))) = " -SPEED FAN GET-";
const char gRaceResultsPerfectGetLabel[16] __attribute__((section(".rodata.race_results_labels"), aligned(4))) = "  -PERFECT GET-";
const char gRaceComparisonLapTimeLabel[12] __attribute__((section(".rodata.race_comparison_labels"), aligned(4))) = "LAP TIME";
const char gRaceComparisonSpeedFanLabel[16] __attribute__((section(".rodata.race_comparison_labels"), aligned(4))) = "SPEEDFAN GET";
const char gRaceComparisonHitCountLabel[12] __attribute__((section(".rodata.race_comparison_labels"), aligned(4))) = "HIT COUNT";
const char gRaceComparisonTrickPointLabel[12] __attribute__((section(".rodata.race_comparison_labels"), aligned(4))) = "TRICK POINT";

extern char gRaceResultsFastLabel[];

void drawRaceTime(const RaceTime *value, s32 x, s32 y, s16 mode, GsOT *ot, u16 depth) {
    drawHudDecimalNumber8x16(x, y, 2, 1, value->minutes, mode, -1, ot, depth);
    drawHudDecimalNumber8x16(x + 24, y, 2, 1, value->seconds, mode, -1, ot, depth);
    drawHudDecimalNumber8x16(x + 48, y, 2, 1, value->centiseconds >> 8, mode, -1, ot, depth);
    drawSpriteByIndexWithBlend(x + 16, y, 25, mode, -1, ot, depth);
    drawSpriteByIndexWithBlend(x + 40, y, 24, mode, -1, ot, depth);
}

void drawTimeRecordRows(MenuTimeResource *resource) {
    s32 y, i, mode;
    setSpriteColor(128, 128, 128);
    y = -80;
    for (i = 0; i < 5; i++) {
        if (resource->selection == i && (gFrameCounter & 1)) {
            mode = 2;
        } else if (i < 3) {
            mode = 1;
        } else {
            mode = 4;
        }
        drawSpriteByIndexWithBlend(-8, y, i + 0x136, mode, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&gPlayerSaveRecords[0].lapRecords[D_8013AFCE][i],
                     16, y, mode, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(88, y,
                     (gPlayerSaveRecords[0].lapRecordIcons[D_8013AFCE][i] & 15) * 3 + 0x14A,
                     D_80124284[gPlayerSaveRecords[0].lapRecordIcons[D_8013AFCE][i] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(108, y,
                     (gPlayerSaveRecords[0].lapRecordIcons[D_8013AFCE][i] >> 4) + 0x13B,
                     7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
}

void drawMenuTimeWithBrightness(MenuTimeResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawRaceTime(&gRaceElapsedTime, -108, -48, 1, &D_800EBFF0[D_8013AF78], 1);
}

void drawLapTimeLabel(TaskMenuFadeResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness,
                       (u8)resource->brightness,
                       (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawLimitedText8x8(-116, -60, 20, D_80010A94, 5, -1,
                 &D_800EBFF0[D_8013AF78], 1);
}

void drawCurrentTaskTimeMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    drawMenuTimeWithBrightness((MenuTimeResource *)resource);
    drawTimeRecordRows((MenuTimeResource *)resource);
    drawLapTimeLabel((TaskMenuFadeResource *)resource);
}

void drawTaskTimeMenuAndSetFlag10OnInput(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(drawCurrentTaskTimeMenu);
    }
    drawMenuTimeWithBrightness((MenuTimeResource *)resource);
    drawTimeRecordRows((MenuTimeResource *)resource);
    drawLapTimeLabel((TaskMenuFadeResource *)resource);
}

void waitTimeMenuDisplayTask(void) {
    TaskMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(drawTaskTimeMenuAndSetFlag10OnInput);
    }
    drawMenuTimeWithBrightness((MenuTimeResource *)resource);
    drawLapTimeLabel(resource);
}

void fadeInTimeMenuTask(void) {
    TaskMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->secondaryBrightness += 8;
    if (resource->secondaryBrightness > 128) {
        resource->timer = 30;
        resource->secondaryBrightness = 128;
        setCurrentTaskCallback(waitTimeMenuDisplayTask);
    }
    drawMenuTimeWithBrightness((MenuTimeResource *)resource);
    drawLapTimeLabel(resource);
}

void fadeInLapTimeLabelTask(void) {
    TaskMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->brightness = 128;
        setCurrentTaskCallback(fadeInTimeMenuTask);
    }
    drawLapTimeLabel(resource);
}

void initializeTaskMenuFade(void) {
    TaskMenuFadeResource *resource;

    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk00 = gRacePlayers[0].unk100[0];
    resource->brightness = 0;
    resource->secondaryBrightness = 0;
    setCurrentTaskCallback(fadeInLapTimeLabelTask);
}

void drawHitCountPanel(TaskResourceRecord *record) {
    char text[32];
    MenuFadeInResource *resource = (MenuFadeInResource *)record;
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawHudDecimalNumber8x16(-76, -28, 2, 0, gRacePlayers[0].scoreAttackPointTotal, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    sprintf(text, gHitCountTotalFormat, gRacePlayers[0].scoreAttackPointTarget);
    drawLimitedText8x8(-84, -20, 1, gHitCountMarker, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-60, -20, 10, text, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    if (D_8013B18D) {
        drawLimitedText8x8(-104, -4, 10, gHitCountTimeOutLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
    }
    drawSpriteByIndexWithBlend(-108, -36, 432, 0, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawHitPrizeMoneyValues(TaskResourceRecord *record) {
    MenuFadeInResource *resource = (MenuFadeInResource *)record;
    u8 padding[0x20];
    s32 y;
    s32 stage;

    setSpriteColor(128, 128, 128);
    y = resource->unk0E == 0 ? -51 : -71;
    stage = resource->unk06;
    if (stage < 0) {
        goto draw_completion;
    }
    drawHudDecimalNumber8x16(32, y, 5, 0, resource->unk08, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    y += 40;
    stage = resource->unk06;
    if (stage <= 0) {
        goto draw_completion;
    }
    if (resource->unk0E != 0) {
        drawHudDecimalNumber8x16(32, y, 5, 0, resource->unk0A, 1, -1,
                      &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 40;
    }
    stage = resource->unk06;
draw_completion:
    if (stage >= 2) {
        drawHudDecimalNumber8x16(32, y, 5, 0, resource->unk0C, 1, -1,
                      &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 40;
    }
    if (resource->unk06 >= 3) {
        drawHudDecimalNumber8x16(24, y, 6, 0, gRacePlayers[0].money, 1, -1,
                      &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    }
}

void drawCountRecordRows(TaskResourceRecord *resource) {
    s32 y, i, mode, textMode;
    setSpriteColor(128, 128, 128);
    y = -80;
    for (i = 0; i < 5; i++) {
        if (((TaskMenuFadeResource *)resource)->unk00 == i && (gFrameCounter & 1)) {
            mode = 2;
        } else if (i < 3) {
            mode = 1;
            textMode = 2;
        } else {
            mode = 4;
            textMode = 4;
        }
        drawSpriteByIndexWithBlend(-8, y, i + 0x136, mode, -1, &D_800EBFF0[D_8013AF78], 1);
        drawHudDecimalNumber8x16(16, y, 2, 0, gPlayerSaveRecords[0].countRecordValues[D_8013AFCE][i], mode, -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(70, y, (gPlayerSaveRecords[0].countRecordIcons[D_8013AFCE][i] & 15) * 3 + 0x14A,
            D_80124284[gPlayerSaveRecords[0].countRecordIcons[D_8013AFCE][i] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(88, y, (gPlayerSaveRecords[0].countRecordIcons[D_8013AFCE][i] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(32, i * 32 - 72, 3, D_8013B138, textMode, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
}

void drawHitPrizeMoneyLabels(MenuFadeInResource *resource) {
    s32 y;
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    y = -80;
    if (resource->unk0E == 0) y = -60;
    if (D_8013B18D) {
        drawLimitedText8x8(-8, y, 20, gHitPrizeOneGoldLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    } else {
        drawLimitedText8x8(-12, y, 20, gHitPrizeFiftyGoldLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    }
    y += 40;
    if (resource->unk0E != 0) {
        drawLimitedText8x8(-12, y, 20, gPerfectHitLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 40;
    }
    drawLimitedText8x8(-8, y, 20, gCompleteBonusLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    y += 40;
    drawLimitedText8x8(4, y, 20, gResultTotalMoneyLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawCurrentTaskMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    drawHitCountPanel(resource);
    drawCountRecordRows(resource);
}

void drawTaskMenuAndSetFlag10OnInput(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(drawCurrentTaskMenu);
    }
    drawHitCountPanel(resource);
    drawCountRecordRows(resource);
}

void drawCurrentTaskHitPrizeMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    drawHitCountPanel(resource);
    drawHitPrizeMoneyValues(resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void updateMenuExitInputTask(void) {
    TaskMenuFadeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        if (resource->unk00 < 5) {
            setCurrentTaskCallback(drawTaskMenuAndSetFlag10OnInput);
        } else {
            D_8013AFE4 |= 0x10;
            setCurrentTaskCallback(drawCurrentTaskHitPrizeMenu);
        }
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void transferHitPrizeMoneyTask(void) {
    MenuFadeInResource *resource;
    s32 amount;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer == 0) {
        resource->timer = 2;
        playSoundEffect(0, 0x700, 0x20);
    } else {
        resource->timer--;
    }
    amount = resource->unk08;
    if (amount >= 8) {
        if (!(D_8013AEE8[0] & 0xB0)) {
            amount = 8;
        }
    }
    gRacePlayers[0].money += amount;
    resource->unk08 -= amount;
    amount = resource->unk0A;
    if (amount >= 8) {
        if (!(D_8013AEE8[0] & 0xB0)) {
            amount = 8;
        }
    }
    gRacePlayers[0].money += amount;
    resource->unk0A -= amount;
    amount = resource->unk0C;
    if (amount >= 8) {
        if (!(D_8013AEE8[0] & 0xB0)) {
            amount = 8;
        }
    }
    gRacePlayers[0].money += amount;
    resource->unk0C -= amount;
    if (gRacePlayers[0].money > 999999) {
        gRacePlayers[0].money = 999999;
    }
    gPlayerSaveRecords[0].money = gRacePlayers[0].money;
    if (resource->unk08 == 0 && resource->unk0A == 0 && resource->unk0C == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateMenuExitInputTask);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels(resource);
}

void advanceTaskMenuCountdown(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 3;
    if (--resource->timer == 0) {
        setCurrentTaskCallback(transferHitPrizeMoneyTask);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void resetTaskMenuCountdown(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 3;
    if (--resource->timer == 0) {
        resource->timer = 10;
        setCurrentTaskCallback(advanceTaskMenuCountdown);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void startTaskMenuCountdownAndPlaySound(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 2;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(resetTaskMenuCountdown);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void tickMenuSoundDelayStage1Task(void) {
    MenuSoundDelayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuSoundDelayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 1;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(startTaskMenuCountdownAndPlaySound);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void tickMenuSoundDelayStage0Task(void) {
    MenuSoundDelayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuSoundDelayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 0;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        if (resource->unk0E != 0) {
            setCurrentTaskCallback(tickMenuSoundDelayStage1Task);
        } else {
            setCurrentTaskCallback(startTaskMenuCountdownAndPlaySound);
        }
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyValues((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

void waitTaskMenuCountdownAndPlaySound(void) {
    MenuFadeInResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(tickMenuSoundDelayStage0Task);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels(resource);
}

void updateTaskMenuFadeIn(void) {
    MenuFadeInResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness != 128) {
        resource->timer = 20;
        resource->brightness = 128;
        setCurrentTaskCallback(waitTaskMenuCountdownAndPlaySound);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels(resource);
}

void initializeTaskMenuFadeIn(void) {
    MenuFadeInResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0C = 300;
    resource->brightness = 0;
    resource->unk0E = 0;
    resource->unk0A = 0;
    if (D_8013AFCE == 9) {
        resource->unk0C = 100;
    }
    if (D_8013B18D) {
        resource->unk08 = gRacePlayers[0].scoreAttackPointTotal;
    } else {
        resource->unk08 = gRacePlayers[0].scoreAttackPointTotal * 50;
        if (gRacePlayers[0].scoreAttackPointTotal == gRacePlayers[0].scoreAttackPointTarget) {
            resource->unk0E = 1;
            resource->unk0A = 1000;
        }
    }
    resource->unk00 = gRacePlayers[0].unk100[0];
    gRacePlayers[0].money = gPlayerSaveRecords[0].money;
    setCurrentTaskCallback(updateTaskMenuFadeIn);
}

void drawTimeRecordHeader(TimeRecordHeaderResource *resource) {
    u8 padding[0x20];
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawLimitedText8x8(-112, -64, 20, D_80010A94, 5, -1,
                 &D_800EBFF0[D_8013AF78], 1);
    drawRaceTime(&gRaceElapsedTime, -104, -55, 1,
                 &D_800EBFF0[D_8013AF78], 1);
}


void drawRaceResultsMoneyValues(TaskResourceRecord *record) {
    RaceResultsFadeResource *resource = (RaceResultsFadeResource *)record;
    u8 padding[0x20];
    s32 y;
    s32 stage;

    setSpriteColor(128, 128, 128);
    y = -71;
    stage = resource->unk0A;
    if (stage >= 0) {
        drawHudDecimalNumber8x16(32, y, 5, 0, resource->timeBonus, 1, -1,
                                &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
        stage = resource->unk0A;
        if (stage >= 1) {
            drawHudDecimalNumber8x16(32, y, 5, 0, resource->scoreBonus, 1, -1,
                                    &D_800EBFF0[D_8013AF78], 1);
            drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
            y += 32;
            stage = resource->unk0A;
        }
    }
    if (stage >= 2 && resource->hasBonus) {
        drawHudDecimalNumber8x16(32, y, 5, 0, resource->bonusTimer, 1, -1,
                                &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
    if (resource->unk0A >= 3) {
        drawHudDecimalNumber8x16(32, y, 5, 0, resource->displayTimer, 1, -1,
                                &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
    if (resource->unk0A >= 4) {
        drawHudDecimalNumber8x16(24, y, 6, 0, gRacePlayers[0].money, 1, -1,
                                &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    }
}

void drawTimeRecordListRows(TimeRecordListResource *resource) {
    u8 padding[0x20];
    s32 y;
    s32 row;
    s32 palette;
    setSpriteColor(128, 128, 128);
    y = -80;
    for (row = 0; row < 5; row++) {
        if (resource->selectedRow == row && (gFrameCounter & 1)) {
            palette = 2;
        } else if (row < 3) {
            palette = 1;
        } else {
            palette = 4;
        }
        drawSpriteByIndexWithBlend(-8, y, row + 0x136, palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&gPlayerSaveRecords[0].timeRecords[D_8013AFCE][row], 16, y, palette, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(88, y, (gPlayerSaveRecords[0].timeRecordCharacters[D_8013AFCE][row] & 15) * 3 + 0x14A,
            D_80124284[gPlayerSaveRecords[0].timeRecordCharacters[D_8013AFCE][row] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(108, y, (gPlayerSaveRecords[0].timeRecordCharacters[D_8013AFCE][row] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
}

void drawRaceResultsLabels(RaceResultsFadeResource *resource) {
    char text[32];
    s32 y;

    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }

    if (!D_8013B18D) {
        drawLimitedText8x8(-104, -31, 10, gRaceResultsTargetLabel, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        sprintf(text, gRaceResultsTargetTimeFormat, D_80124220[D_8013AFCE].minutes,
                D_80124220[D_8013AFCE].seconds, (s8)(D_80124220[D_8013AFCE].centiseconds >> 8));
        drawLimitedText8x8(-104, -22, 20, text, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(-104, -8, 10, gRaceResultsFastLabel, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        sprintf(text, gRaceResultsTimeDifferenceFormat, resource->difference.minutes, resource->difference.seconds,
                (s8)(resource->difference.centiseconds >> 8));
        drawLimitedText8x8(-108, 1, 20, text, 7, -1, &D_800EBFF0[D_8013AF78], 1);
    } else {
        drawLimitedText8x8(-104, -36, 10, gHitCountTimeOutLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
    }

    drawLimitedText8x8(-8, -80, 20, gRaceResultsTimePrizeLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-12, -48, 20, gRaceResultsSpeedFanGetLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    y = -16;
    if (resource->hasBonus) {
        drawLimitedText8x8(-12, y, 20, gRaceResultsPerfectGetLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
    drawLimitedText8x8(-8, y, 20, gCompleteBonusLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    y += 32;
    drawLimitedText8x8(-12, y, 20, gTotalMoneyLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
}

/* Leave the size unknown above so CCPSX schedules an absolute address load. */
char gRaceResultsFastLabel[8] __attribute__((section(".data.race_results_fast_label"), aligned(4))) = " -FAST-";

void drawCurrentTaskTimeRecords(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        deleteCurrentTaskSubtree();
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawTimeRecordListRows((TimeRecordListResource *)resource);
}

void drawTaskTimeRecordsAndSetFlag10OnInput(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(drawCurrentTaskTimeRecords);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawTimeRecordListRows((TimeRecordListResource *)resource);
}

void drawCurrentTaskTimeRecordDetails(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        deleteCurrentTaskSubtree();
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues(resource);
    drawRaceResultsLabels((RaceResultsFadeResource *)resource);
}

void updateRaceResultsExitInputTask(void) {
    RaceResultsFadeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        if (resource->unk00 < 5) {
            setCurrentTaskCallback(drawTaskTimeRecordsAndSetFlag10OnInput);
        } else {
            D_8013AFE4 |= 0x10;
            setCurrentTaskCallback(drawCurrentTaskTimeRecordDetails);
        }
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels(resource);
}

void countRaceResultsMoneyTask(void) {
    RaceResultsFadeResource *resource;
    s32 amount;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer == 0) {
        resource->timer = 2;
        playSoundEffect(0, 0x700, 0x20);
    } else {
        resource->timer--;
    }
    amount = resource->timeBonus;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->timeBonus -= amount;
    amount = resource->scoreBonus;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->scoreBonus -= amount;
    amount = resource->bonusTimer;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->bonusTimer -= amount;
    amount = resource->displayTimer;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->displayTimer -= amount;
    if (gRacePlayers[0].money > 999999) {
        gRacePlayers[0].money = 999999;
    }
    gPlayerSaveRecords[0].money = gRacePlayers[0].money;
    if (resource->scoreBonus == 0 && resource->timeBonus == 0 && resource->bonusTimer == 0 && resource->displayTimer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateRaceResultsExitInputTask);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels(resource);
}

void tickRaceResultsDelayTask(void) {
    RaceResultsFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 4;
    if (--resource->timer == 0) {
        setCurrentTaskCallback(countRaceResultsMoneyTask);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels(resource);
}

void updateTaskTimeRecordDetailCountdown(void) {
    TimeRecordDetailCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimeRecordDetailCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 4;
    if (--resource->timer == 0) {
        resource->timer = 10;
        setCurrentTaskCallback(tickRaceResultsDelayTask);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels((RaceResultsFadeResource *)resource);
}

void updateTaskTimeRecordCountdownAndPlaySound(void) {
    TimeRecordDetailCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimeRecordDetailCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 3;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateTaskTimeRecordDetailCountdown);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels((RaceResultsFadeResource *)resource);
}

void updateTaskTimeRecordCountdownPhase2AndPlaySound(void) {
    TimeRecordDetailCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimeRecordDetailCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 2;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateTaskTimeRecordCountdownAndPlaySound);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels((RaceResultsFadeResource *)resource);
}

void updateTaskTimeRecordCountdownPhase1AndPlaySound(void) {
    TimeRecordDetailCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimeRecordDetailCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 1;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        if (resource->unk14 != 0) {
            setCurrentTaskCallback(updateTaskTimeRecordCountdownPhase2AndPlaySound);
        } else {
            setCurrentTaskCallback(updateTaskTimeRecordCountdownAndPlaySound);
        }
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels((RaceResultsFadeResource *)resource);
}

void tickRaceResultsSoundDelayTask(void) {
    RaceResultsFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 0;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateTaskTimeRecordCountdownPhase1AndPlaySound);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsMoneyValues((TaskResourceRecord *)resource);
    drawRaceResultsLabels(resource);
}

void waitTaskTimeRecordCountdownAndPlaySound(void) {
    TimeRecordDetailCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimeRecordDetailCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(tickRaceResultsSoundDelayTask);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsLabels((RaceResultsFadeResource *)resource);
}

void fadeInRaceResultsTask(void) {
    RaceResultsFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 20;
        resource->brightness = 128;
        setCurrentTaskCallback(waitTaskTimeRecordCountdownAndPlaySound);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    drawRaceResultsLabels(resource);
}

void initializeRaceResultsTask(void) {
    u8 padding[8];
    RaceResultsFadeResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    resource->hasBonus = 0;
    resource->bonusTimer = 0;
    resource->displayTimer = 300;
    if (D_8013AFCE == 9) {
        resource->displayTimer = 100;
    }
    calculateRaceTimeDifference(&D_80124220[D_8013AFCE], &gRaceElapsedTime, &resource->difference);
    if (D_8013B18D) {
        resource->timeBonus = 0;
    } else {
        resource->timeBonus = (resource->difference.minutes * 60 + resource->difference.seconds) * 20;
        if (gRacePlayers[0].collectedCourseCoinMarkerCount == gRacePlayers[0].totalCourseCoinMarkerCount) {
            resource->hasBonus = 1;
            resource->bonusTimer = 300;
        }
    }
    resource->scoreBonus = gRacePlayers[0].collectedCourseCoinMarkerCount * 10;
    resource->unk00 = gRacePlayers[0].unk100[0];
    gRacePlayers[0].money = gPlayerSaveRecords[0].money;
    setCurrentTaskCallback(fadeInRaceResultsTask);
}

void drawStatsMenuValues(TaskResourceRecord *record) {
    char text[32];
    s16 length;
    s32 x;
    StatsMenuFadeResource *resource = (StatsMenuFadeResource *)record;
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawLimitedText8x8(-104, -64, 10, gStatsTrickLabel, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-100, -28, 10, gStatsMakeLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    if (D_8013B18D) {
        drawLimitedText8x8(-108, 8, 10, gHitCountTimeOutLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
    }
    sprintf(text, gResultValueFormat, gRacePlayers[0].trickAttackPointTotal);
    length = strlen(text);
    x = -80;
    if (gRacePlayers[0].trickAttackPointTotal >= 10) x = -84;
    if (gRacePlayers[0].trickAttackPointTotal >= 100) x = -88;
    if (gRacePlayers[0].trickAttackPointTotal >= 1000) x = -92;
    drawHudDecimalNumber8x16(x, -55, length, 0, gRacePlayers[0].trickAttackPointTotal, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    sprintf(text, gResultValueFormat, gRacePlayers[0].trickAttackMakeCount);
    length = strlen(text);
    x = -80;
    if (gRacePlayers[0].trickAttackMakeCount >= 10) x = -84;
    drawHudDecimalNumber8x16(x, -19, length, 0, gRacePlayers[0].trickAttackMakeCount, 3, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawStatsMenuMoneyValues(TaskResourceRecord *record) {
    u8 padding[0x20];
    s32 y;
    s32 stage;
    setSpriteColor(128, 128, 128);
    y = -71;
    stage = ((StatsMenuFadeResource *)record)->unk06;
    if (stage >= 0) {
        drawHudDecimalNumber8x16(32, y, 5, 0, ((StatsMenuFadeResource *)record)->trickPrize, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 40;
        stage = ((StatsMenuFadeResource *)record)->unk06;
        if (stage >= 1) {
            drawHudDecimalNumber8x16(32, y, 5, 0, ((StatsMenuFadeResource *)record)->makeBonus, 1, -1, &D_800EBFF0[D_8013AF78], 1);
            drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
            y += 40;
            stage = ((StatsMenuFadeResource *)record)->unk06;
        }
    }
    if (stage >= 2) {
        drawHudDecimalNumber8x16(32, y, 5, 0, ((StatsMenuFadeResource *)record)->completeBonus, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 40;
    }
    if (((StatsMenuFadeResource *)record)->unk06 >= 3) {
        drawHudDecimalNumber8x16(24, y, 6, 0, gRacePlayers[0].money, 1, -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(72, y, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    }
}

void drawStatsRecordRows(TaskResourceRecord *resource) {
    u8 padding[0x20];
    s32 y, i, mode, textMode;
    y = -80;
    for (i = 0; i < 5; i++) {
        if (((StatsMenuFadeResource *)resource)->unk00 == i && (gFrameCounter & 1)) {
            mode = 2;
        } else if (i < 3) {
            mode = 1;
            textMode = 2;
        } else {
            mode = 4;
            textMode = 4;
        }
        drawSpriteByIndexWithBlend(24, y, i + 0x136, mode, -1, &D_800EBFF0[D_8013AF78], 1);
        drawHudDecimalNumber8x16(44, y, 5, 0, gPlayerSaveRecords[0].statsRecordValues[D_8013AFCE][i], mode, -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(102, y, (gPlayerSaveRecords[0].statsRecordIcons[D_8013AFCE][i] & 15) * 3 + 0x14A,
            D_80124284[gPlayerSaveRecords[0].statsRecordIcons[D_8013AFCE][i] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        drawSpriteByIndexWithBlend(120, y, (gPlayerSaveRecords[0].statsRecordIcons[D_8013AFCE][i] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(84, y + 8, 1, gPlayerNumberSuffixLabel, textMode, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
}

void drawStatsMenuMoneyLabels(StatsMenuFadeResource *resource) {
    u8 padding[0x10];
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawLimitedText8x8(-12, -80, 20, gTrickPrizeLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-8, -40, 20, gMakeBonusLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-8, 0, 20, gCompleteBonusLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-12, 40, 20, gTotalMoneyLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawCurrentTaskStatsSelection(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        deleteCurrentTaskSubtree();
    }
    drawStatsMenuValues(resource);
    drawStatsRecordRows(resource);
}

void updateStatsMenuExitInputTask(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(drawCurrentTaskStatsSelection);
    }
    drawStatsMenuValues(resource);
    drawStatsRecordRows(resource);
}

void drawCurrentTaskStatsMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    drawStatsMenuValues(resource);
    drawStatsMenuMoneyValues(resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
}

void updateStatsMenuPostCountInputTask(void) {
    StatsMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        if (resource->unk00 < 5) {
            setCurrentTaskCallback(updateStatsMenuExitInputTask);
        } else {
            D_8013AFE4 |= 0x10;
            setCurrentTaskCallback(drawCurrentTaskStatsMenu);
        }
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels(resource);
}

void countStatsMenuMoneyTask(void) {
    StatsMenuFadeResource *resource;
    s32 amount;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer == 0) {
        resource->timer = 2;
        playSoundEffect(0, 0x700, 0x20);
    } else {
        resource->timer--;
    }
    amount = resource->trickPrize;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->trickPrize -= amount;
    amount = resource->makeBonus;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->makeBonus -= amount;
    amount = resource->completeBonus;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    gRacePlayers[0].money += amount;
    resource->completeBonus -= amount;
    if (gRacePlayers[0].money > 999999) {
        gRacePlayers[0].money = 999999;
    }
    gPlayerSaveRecords[0].money = gRacePlayers[0].money;
    if (resource->trickPrize == 0 && resource->makeBonus == 0 && resource->completeBonus == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateStatsMenuPostCountInputTask);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels(resource);
}

void waitForStatsScoreCountTask(void) {
    StatsMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 3;
    if (--resource->timer == 0) {
        setCurrentTaskCallback(countStatsMenuMoneyTask);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels(resource);
}

void waitForStatsScoreCountStartTask(void) {
    StatsMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 3;
    if (--resource->timer == 0) {
        resource->timer = 10;
        setCurrentTaskCallback(waitForStatsScoreCountTask);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels(resource);
}

void startTaskStatsCountdownAndPlaySound(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 2;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(waitForStatsScoreCountStartTask);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
}

void advanceTaskStatsCountdownAndPlaySound(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 1;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(startTaskStatsCountdownAndPlaySound);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
}

void resetTaskStatsCountdown(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 0;
    if (--resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(advanceTaskStatsCountdownAndPlaySound);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
}

void waitTaskStatsCountdownAndPlaySound(void) {
    MenuFadeInResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(resetTaskStatsCountdown);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
}

void fadeInStatsMenuTask(void) {
    StatsMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 20;
        resource->brightness = 128;
        setCurrentTaskCallback(waitTaskStatsCountdownAndPlaySound);
    }
    drawStatsMenuValues((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels(resource);
}

void initializeStatsMenuTask(void) {
    StatsMenuFadeResource *resource;

    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    resource->makeBonus = 0;
    resource->completeBonus = 300;
    if (D_8013B18D) {
        resource->trickPrize = 0;
        resource->makeBonus = 0;
    } else {
        resource->trickPrize = gRacePlayers[0].trickAttackPointTotal * 3;
        resource->makeBonus = gRacePlayers[0].trickAttackMakeCount * 10;
    }
    resource->unk00 = gRacePlayers[0].unk100[0];
    gRacePlayers[0].money = gPlayerSaveRecords[0].money;
    setCurrentTaskCallback(fadeInStatsMenuTask);
}

void drawRaceTimeComparisonPanel(PanelResource *resource) {
    char text[32];
    s16 i;

    for (i = 0; i < 2; i++) {
        if (resource->brightness != 128) {
            setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
        } else {
            setSpriteColor(128, 128, 128);
        }
        drawSpriteByIndexWithBlend(-124, i * 104 - 88, i + 490, (s16)(i + 13), 0, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(-80, i * 104 - 56, 20, gRaceComparisonLapTimeLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&D_8013B190[i], -80, i * 104 - 48, 1, &D_800EBFF0[D_8013AF78], 1);
        if (D_8013B1A0[i] == 0) {
            drawLimitedText8x8(-80, i * 104 - 24, 20, gRaceResultsFastLabel, 7, -1, &D_800EBFF0[D_8013AF78], 1);
            sprintf(text, gRaceResultsTimeDifferenceFormat, resource->time[i].minutes,
                    resource->time[i].seconds, resource->time[i].centiseconds >> 8);
            drawLimitedText8x8(-80, i * 104 - 16, 20, text, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        } else {
            drawLimitedText8x8(-80, i * 104 - 24, 20, gHitCountTimeOutLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
        }
        if (resource->displayStage > 0) {
            switch (D_8013AFE0) {
            case 0:
                drawUppercaseProportionalText(8, i * 104 - 56, 20, gRaceComparisonSpeedFanLabel, 3, -1, &D_800EBFF0[D_8013AF78], 1);
                drawHudDecimalNumber8x16(64, i * 104 - 40, 2, 0, gRacePlayers[i].collectedCourseCoinMarkerCount,
                                        1, -1, &D_800EBFF0[D_8013AF78], 1);
                drawSpriteByIndexWithBlend(80, i * 104 - 40, 27, 1, -1, &D_800EBFF0[D_8013AF78], 1);
                drawHudDecimalNumber8x16(88, i * 104 - 40, 2, 0, gRacePlayers[i].totalCourseCoinMarkerCount,
                                        1, -1, &D_800EBFF0[D_8013AF78], 1);
                break;
            case 1:
                drawUppercaseProportionalText(8, i * 104 - 56, 20, gRaceComparisonHitCountLabel, 3, -1, &D_800EBFF0[D_8013AF78], 1);
                drawHudDecimalNumber8x16(64, i * 104 - 40, 2, 0, gRacePlayers[i].scoreAttackPointTotal,
                                        1, -1, &D_800EBFF0[D_8013AF78], 1);
                drawSpriteByIndexWithBlend(80, i * 104 - 40, 27, 1, -1, &D_800EBFF0[D_8013AF78], 1);
                drawHudDecimalNumber8x16(88, i * 104 - 40, 2, 0, gRacePlayers[i].scoreAttackPointTarget,
                                        1, -1, &D_800EBFF0[D_8013AF78], 1);
                break;
            case 2:
                drawUppercaseProportionalText(8, i * 104 - 56, 20, gRaceComparisonTrickPointLabel, 3, -1, &D_800EBFF0[D_8013AF78], 1);
                drawHudDecimalNumber8x16(88, i * 104 - 40, 5, 0, gRacePlayers[i].trickAttackPointTotal,
                                        1, -1, &D_800EBFF0[D_8013AF78], 1);
                break;
            }
        }
        if (resource->displayStage >= 2) {
            s16 palette;
            if (resource->resultIcon[i] == 0 && (gFrameCounter & 1)) {
                setSpriteColor(192, 192, 192);
                palette = resource->resultIcon[i] + 9;
            } else {
                setSpriteColor(128, 128, 128);
                palette = resource->resultIcon[i] + 9;
            }
            drawSpriteByIndexWithBlend(-80, i * 104 - 84, resource->resultIcon[i] + 567, palette,
                          -1, &D_800EBFF0[D_8013AF78], 1);
        }
    }
}

void drawCurrentTaskPanel(void) {
    PanelResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        deleteCurrentTaskSubtree();
    }
    drawRaceTimeComparisonPanel(resource);
}

void updatePanelExitInputTask(void) {
    PanelResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer > 0) {
        resource->timer--;
    } else if ((D_8013AEE8[0] | D_8013AEE8[1]) & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(drawCurrentTaskPanel);
    }
    drawRaceTimeComparisonPanel(resource);
}

void advanceCurrentTaskPanelTimer(void) {
    PanelResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 20) {
        resource->displayStage = 1;
    }
    if (resource->timer == 0) {
        resource->timer = 20;
        resource->displayStage = 2;
        setCurrentTaskCallback(updatePanelExitInputTask);
    }
    drawRaceTimeComparisonPanel(resource);
}

void fadeInCurrentTaskPanel(void) {
    PanelResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 40;
        resource->brightness = 128;
        setCurrentTaskCallback(advanceCurrentTaskPanelTimer);
    }
    drawRaceTimeComparisonPanel(resource);
}

void initializeRaceTimeComparisonPanel(void) {
    PanelResource *resource;
    s16 i;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    resource->displayStage = 0;
    if (D_8013AFE1 == 1) {
        resource->resultIcon[0] = 0;
        resource->resultIcon[1] = 1;
    } else if (D_8013AFE1 == 2) {
        resource->resultIcon[0] = 1;
        resource->resultIcon[1] = 0;
    } else {
        resource->resultIcon[0] = 2;
        resource->resultIcon[1] = 2;
    }
    for (i = 0; i < 2; i++) {
        switch (D_8013AFE0) {
        case 0:
            calculateRaceTimeDifference(&D_80124220[D_8013AFCE], &D_8013B190[i], &resource->time[i]);
            break;
        case 1:
            calculateRaceTimeDifference(&D_80124248[D_8013AFCE], &D_8013B190[i], &resource->time[i]);
            break;
        case 2:
            calculateRaceTimeDifference(&D_8013B154, &D_8013B190[i], &resource->time[i]);
            break;
        }
    }
    setCurrentTaskCallback(fadeInCurrentTaskPanel);
}
