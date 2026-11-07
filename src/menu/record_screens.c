#include "../race/race_presentation_internal.h"

const char gHitCountTimeOutLabel[12] __attribute__((section(".rodata.hit_count_time_out"), aligned(4))) = "TIME OUT";
char gHitCountTotalFormat[4] __attribute__((section(".data.hit_count_labels"), aligned(4))) = "/%d";
char gHitCountMarker[4] __attribute__((section(".data.hit_count_labels"), aligned(4))) = "*";

const char gHitPrizeOneGoldLabel[20] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "-HIT PRIZE(*1G)-";

const char gHitPrizeFiftyGoldLabel[20] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "-HIT PRIZE(*50G)-";

const char gPerfectHitLabel[16] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "  -PERFECT HIT-";

void drawRaceTime(const RaceTime *value, s32 x, s32 y, s16 mode, GsOT *ot, u16 depth) {
    func_8006DFB4(x, y, 2, 1, value->minutes, mode, -1, ot, depth);
    func_8006DFB4(x + 24, y, 2, 1, value->seconds, mode, -1, ot, depth);
    func_8006DFB4(x + 48, y, 2, 1, value->centiseconds >> 8, mode, -1, ot, depth);
    func_8006C998(x + 16, y, 25, mode, -1, ot, depth);
    func_8006C998(x + 40, y, 24, mode, -1, ot, depth);
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
        func_8006C998(-8, y, i + 0x136, mode, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][i],
                     16, y, mode, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(88, y,
                     (((SaveRecord *)&D_8013080C)->lapRecordIcons[D_8013AFCE][i] & 15) * 3 + 0x14A,
                     D_80124284[((SaveRecord *)&D_8013080C)->lapRecordIcons[D_8013AFCE][i] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(108, y,
                     (((SaveRecord *)&D_8013080C)->lapRecordIcons[D_8013AFCE][i] >> 4) + 0x13B,
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
    resource->unk00 = D_80120F90[0];
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
    func_8006DFB4(-76, -28, 2, 0, D_80120E90[0].unk95E, 1, -1, &D_800EBFF0[D_8013AF78], 1);
    sprintf(text, gHitCountTotalFormat, D_80120E90[0].unk960);
    drawLimitedText8x8(-84, -20, 1, gHitCountMarker, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(-60, -20, 10, text, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    if (D_8013B18D) {
        drawLimitedText8x8(-104, -4, 10, gHitCountTimeOutLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
    }
    func_8006C998(-108, -36, 432, 0, -1, &D_800EBFF0[D_8013AF78], 1);
}

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_80038270);

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
        func_8006C998(-8, y, i + 0x136, mode, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006DFB4(16, y, 2, 0, ((SaveRecord *)&D_8013080C)->countRecordValues[D_8013AFCE][i], mode, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(70, y, (((SaveRecord *)&D_8013080C)->countRecordIcons[D_8013AFCE][i] & 15) * 3 + 0x14A,
            D_80124284[((SaveRecord *)&D_8013080C)->countRecordIcons[D_8013AFCE][i] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(88, y, (((SaveRecord *)&D_8013080C)->countRecordIcons[D_8013AFCE][i] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
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

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_80038B34);

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
            setCurrentTaskCallback(func_80038B34);
        }
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    func_80038270((TaskResourceRecord *)resource);
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
    D_80120E90[0].unk010 += amount;
    resource->unk08 -= amount;
    amount = resource->unk0A;
    if (amount >= 8) {
        if (!(D_8013AEE8[0] & 0xB0)) {
            amount = 8;
        }
    }
    D_80120E90[0].unk010 += amount;
    resource->unk0A -= amount;
    amount = resource->unk0C;
    if (amount >= 8) {
        if (!(D_8013AEE8[0] & 0xB0)) {
            amount = 8;
        }
    }
    D_80120E90[0].unk010 += amount;
    resource->unk0C -= amount;
    if (D_80120E90[0].unk010 > 999999) {
        D_80120E90[0].unk010 = 999999;
    }
    D_80130BF0[0] = D_80120E90[0].unk010;
    if (resource->unk08 == 0 && resource->unk0A == 0 && resource->unk0C == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateMenuExitInputTask);
    }
    drawHitCountPanel((TaskResourceRecord *)resource);
    func_80038270((TaskResourceRecord *)resource);
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
    func_80038270((TaskResourceRecord *)resource);
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
    func_80038270((TaskResourceRecord *)resource);
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
    func_80038270((TaskResourceRecord *)resource);
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
    func_80038270((TaskResourceRecord *)resource);
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
    func_80038270((TaskResourceRecord *)resource);
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
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
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
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
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
        resource->unk08 = D_80120E90[0].unk95E;
    } else {
        resource->unk08 = D_80120E90[0].unk95E * 50;
        if (D_80120E90[0].unk95E == D_80120E90[0].unk960) {
            resource->unk0E = 1;
            resource->unk0A = 1000;
        }
    }
    resource->unk00 = D_80120E90[0].unk100[0];
    D_80120E90[0].unk010 = D_80130BF0[0];
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

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_80039564);

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
        func_8006C998(-8, y, row + 0x136, palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&((SaveRecord *)&D_8013080C)->timeRecords[D_8013AFCE][row], 16, y, palette, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(88, y, (((SaveRecord *)&D_8013080C)->timeRecordCharacters[D_8013AFCE][row] & 15) * 3 + 0x14A,
            D_80124284[((SaveRecord *)&D_8013080C)->timeRecordCharacters[D_8013AFCE][row] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(108, y, (((SaveRecord *)&D_8013080C)->timeRecordCharacters[D_8013AFCE][row] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 32;
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_80039B34);

void drawCurrentTaskTimeRecords(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        func_800149A8();
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
        func_800149A8();
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
}

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_8003A158);

void tickRaceResultsDelayTask(void) {
    RaceResultsFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RaceResultsFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk0A = 4;
    if (--resource->timer == 0) {
        setCurrentTaskCallback(func_8003A158);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039B34((TaskResourceRecord *)resource);
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
    func_80039B34((TaskResourceRecord *)resource);
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
        if (D_80120E90[0].unk95A == D_80120E90[0].unk95C) {
            resource->hasBonus = 1;
            resource->bonusTimer = 300;
        }
    }
    resource->scoreBonus = D_80120E90[0].unk95A * 10;
    resource->unk00 = D_80120E90[0].unk100[0];
    D_80120E90[0].unk010 = D_80130BF0[0];
    setCurrentTaskCallback(fadeInRaceResultsTask);
}

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_8003AAE0);

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_8003AD48);

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
        func_8006C998(24, y, i + 0x136, mode, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006DFB4(44, y, 5, 0, ((SaveRecord *)&D_8013080C)->statsRecordValues[D_8013AFCE][i], mode, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(102, y, (((SaveRecord *)&D_8013080C)->statsRecordIcons[D_8013AFCE][i] & 15) * 3 + 0x14A,
            D_80124284[((SaveRecord *)&D_8013080C)->statsRecordIcons[D_8013AFCE][i] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(120, y, (((SaveRecord *)&D_8013080C)->statsRecordIcons[D_8013AFCE][i] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
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
        func_800149A8();
    }
    func_8003AAE0(resource);
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
    func_8003AAE0(resource);
    drawStatsRecordRows(resource);
}

void drawCurrentTaskStatsMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    func_8003AAE0(resource);
    func_8003AD48(resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
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
    D_80120E90[0].unk010 += amount;
    resource->trickPrize -= amount;
    amount = resource->makeBonus;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    D_80120E90[0].unk010 += amount;
    resource->makeBonus -= amount;
    amount = resource->completeBonus;
    if (amount >= 8 && !(D_8013AEE8[0] & 0xB0)) {
        amount = 8;
    }
    D_80120E90[0].unk010 += amount;
    resource->completeBonus -= amount;
    if (D_80120E90[0].unk010 > 999999) {
        D_80120E90[0].unk010 = 999999;
    }
    D_80130BF0[0] = D_80120E90[0].unk010;
    if (resource->trickPrize == 0 && resource->makeBonus == 0 && resource->completeBonus == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateStatsMenuPostCountInputTask);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
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
    func_8003AAE0((TaskResourceRecord *)resource);
    drawStatsMenuMoneyLabels((StatsMenuFadeResource *)resource);
}

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_8003BD70);

INCLUDE_ASM("asm/nonmatchings/menu/record_screens", func_8003BE58);

void drawCurrentTaskPanel(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        func_800149A8();
    }
    func_8003BE58(resource);
}

void updatePanelExitInputTask(void) {
    PanelInputResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PanelInputResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer > 0) {
        resource->timer--;
    } else if ((D_8013AEE8[0] | D_8013AEE8[1]) & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(drawCurrentTaskPanel);
    }
    func_8003BE58((TaskResourceRecord *)resource);
}

void advanceCurrentTaskPanelTimer(void) {
    PanelResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 20) {
        resource->unk0C = 1;
    }
    if (resource->timer == 0) {
        resource->timer = 20;
        resource->unk0C = 2;
        setCurrentTaskCallback(updatePanelExitInputTask);
    }
    func_8003BE58((TaskResourceRecord *)resource);
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
    func_8003BE58((TaskResourceRecord *)resource);
}

void initializeRaceTimeComparisonPanel(void) {
    PanelResource *resource;
    s16 i;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PanelResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    resource->unk0C = 0;
    if (D_8013AFE1 == 1) {
        resource->unk0E[0] = 0;
        resource->unk0E[1] = 1;
    } else if (D_8013AFE1 == 2) {
        resource->unk0E[0] = 1;
        resource->unk0E[1] = 0;
    } else {
        resource->unk0E[0] = 2;
        resource->unk0E[1] = 2;
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
