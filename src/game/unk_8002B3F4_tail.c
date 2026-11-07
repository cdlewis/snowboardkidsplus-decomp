#include "unk_8002B3F4.h"
#include "../main_remaining.h"
#include "game/global_flags.h"
#include "psx/libc.h"


/* Verified time-menu fields in the shared save-state prefix. */
typedef struct TimeMenuSaveStateView {
    u8 unk000[0x44];
    RaceTime times[10][5];
    u8 unk10C[0x1C1];
    u8 icons[10][5];
} TimeMenuSaveStateView;

typedef struct StatsRecordSaveView {
    u8 unk000[0x228];
    u16 scores[10][5];
    u8 unk28C[10];
    u8 icons[10][5];
} StatsRecordSaveView;

typedef struct CountRecordSaveView {
    u8 unk000[0x304];
    u8 scores[10][5];
    u8 unk336[5];
    u8 icons[10][5];
} CountRecordSaveView;

const char gHitPrizeOneGoldLabel[20] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "-HIT PRIZE(*1G)-";
const char gHitPrizeFiftyGoldLabel[20] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "-HIT PRIZE(*50G)-";
const char gPerfectHitLabel[16] __attribute__((section(".rodata.hit_prize_labels"), aligned(4))) = "  -PERFECT HIT-";

void dispatchMode11TransitionTask(void) {
    if (D_8013B109 != 1) {
        func_800149A8();
        return;
    }
    switch (D_8013B108) {
    case 2:
        func_80014874(func_80030108);
        D_8013AE88->unk10 |= 1;
        break;
    case 25:
        func_80014874(initializeTaskResourcePairWithOneTickTransition);
        D_8013AE88->unk10 |= 1;
        break;
    case 27:
        func_80014874(initializeMode1Task);
        D_8013AE88->unk10 |= 1;
        D_8013B109 = 3;
        D_8013AFD7 = 0;
        break;
    case 28:
        func_80014874(initializeSelectionMenuTask);
        D_8013AE88->unk10 |= 1;
        D_8013B109 = 3;
        D_8013AFD7 = 0;
        break;
    case 26:
        func_80014874(initializeUnlockMenuResourceTask);
        D_8013AE88->unk10 |= 1;
        D_8013B108 = 2;
        break;
    default:
        func_80014874(initializeBackgroundCameraTask);
        D_8013AE8C->data18 = D_8013B108;
        D_8013AE88->unk10 |= 1;
        D_8013B0F0 = 0;
        break;
    }
}

void initializeMode11TransitionTask(void) {
    configureTaskStateTransition(11, 0, 30);
    setSecondaryDisplayTaskMode(1);
    queueMode11TransitionResources();
    setCurrentTaskCallback(dispatchMode11TransitionTask);
    D_8013AE88->unk10 |= 2;
}

void dispatchGameOptionTransitionTask(void) {
    D_8013AF84 = 0x100;
    initPlayerCamera(0);
    func_8003DEE0();
    switch (D_8013B109) {
    case 0:
        func_80019D3C(2);
        resetPendingCdRequests();
        D_8013B109 = 1;
        D_8013B0F0 = 0;
        break;
    case 1:
        func_80014874(initializeMode11TransitionTask);
        D_8013AE88->unk10 |= 1;
        break;
    case 2:
        func_80070274(1, 0, 30);
        setSecondaryDisplayTaskMode(1);
        queueCdSectorCommand(0, 1, 5, (void *)0x8017D000, 0);
        queueCdSectorCommand(0, 6, 18, (void *)0x801F0000, 0);
        D_8013B109 = 1;
        D_8013B0F0 = 0;
        D_8013AE88->callback = func_80014B58;
        break;
    case 3:
        D_8013B109 = 1;
        D_8013B0F0 = 0;
        func_800149A8();
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80030108);

void initializeMenuTransitionResourcesTask(void) {
    MenuTransitionResource *resource;
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (MenuTransitionResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    resource->unk0E = 0;
    setTaskBackgroundMode(0, 0, 0);
    func_8002EE0C();
    if (D_8013B0F0 == 0) {
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(3);
        queueCdSectorCommand(2, 0x162, 0x8D, (void *)0x80189800, 0);
        D_8013B0F0 = 1;
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(waitMode2TransitionTask);
        D_8013AE88->unk10 |= 2;
    } else {
        setCurrentTaskCallback(func_800302F8);
    }
}

void waitMode2TransitionTask(void) {
    switch (D_8013AE88->data18) {
    case 0:
        configureTaskStateTransition(2, 0, 30);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (func_800707F0(0) == 0) {
            D_8013AE88->data18 = 0;
            setCurrentTaskCallback(func_800302F8);
        }
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800302F8);

void drawFullScreenTexturedBackground(void) {
    setSpriteColor(128, 128, 128);
    drawPagedTexturedRectangle(-160, -120, 320, 240, 0x85, 0x39F0,
                  &D_800EBFF0[D_8013AF78], 0xFFF);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800309EC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80030B50);

void drawSelectedTaskSideMarkerSprite(void) {
    SelectedTaskSpriteResource *resource;
    SelectedTaskEntry *entries;
    Movement2D *position;
    s32 offset;
    s32 selection;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SelectedTaskSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (SelectedTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    if ((s16)resource->state > 0) {
        setSpriteColor(128, 128, 128);
        selection = resource->selection;
        switch (selection) {
        case 0:
        case 1:
            offset = 16;
            break;
        case 2:
            offset = 8;
            break;
        }
        position = (Movement2D *)&D_800D9504[entries[selection].task->resource14];
        func_8006C998(position->x / 4096 + (s16)(offset - 32), position->y / 4096 - 4,
                      0x44, 0, -1, &D_800EBFF0[D_8013AF78], (u16)(resource->depth - 1));
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800318C4);

void initializePlayerModeSelectionTask(void) {
    PlayerModeSelectionResource *resource;

    D_8013AE88->data18 = 1;
    D_8013AE88->data1A = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PlayerModeSelectionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk00 = 0;
    if (gameOptionPlayerModeAbsolute == 2) {
        resource->selection = gameOptionPlayerModeAbsolute - 1;
    } else {
        resource->selection = 0;
    }
    resource->initialSelection = resource->selection;
    resource->unk06 = 0;
    resource->unk08 = 0;
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_80031C98);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80031C98);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80032280);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003240C);

void drawSelectedTaskMarkerSprite(void) {
    SelectedTaskSpriteResource *resource;
    SelectedTaskEntry *entries;
    Movement2D *position;
    u16 state;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SelectedTaskSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (SelectedTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    state = resource->state;
    if ((s16)resource->state > 0) {
        if ((u32)(state - 1) < 2) {
            setSpriteColor(resource->r, resource->g, resource->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&D_800D9504[entries[resource->selection].task->resource14];
        func_8006C998(position->x / 4096 - 30, position->y / 4096 + 20,
                      0x44, 0, -1, &D_800EBFF0[D_8013AF78], (u16)(resource->depth - 1));
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003288C);

void createRepeatedSoundTask(s32 sound, s32 delay, s32 interval, s32 count) {
    RepeatedSoundResource *resource;

    func_80014874(updateRepeatedSoundTask);
    D_8013AE8C->data18 = delay;
    D_8013AE8C->data1A = interval;
    D_8013AE8C->data1C = 0;
    D_8013AE8C->data1E = count;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RepeatedSoundResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->sound = sound;
}

void updateRepeatedSoundTask(void) {
    RepeatedSoundResource *resource;
    s16 sound;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RepeatedSoundResource *)&D_800D9504[D_8013AE88->resource14];
    sound = resource->sound;
    if (D_8013AE88->data1E == 0) {
        func_800149A8();
    } else if (D_8013AE88->data18 != 0) {
        D_8013AE88->data18--;
    } else {
        if (D_8013AE88->data1C == 0) {
            switch (sound) {
            case 0:
                playSoundEffect(0, 0x200, 0x20);
                break;
            case 1:
                playSoundEffect(0, 0x500, 0x20);
                break;
            case 2:
                playSoundEffect(0, 0x300, 0x20);
                break;
            }
            D_8013AE88->data1E--;
            D_8013AE88->data1C = D_8013AE88->data1A;
            if (D_8013AE88->data1C == 0) {
                return;
            }
        }
        D_8013AE88->data1C--;
    }
}

void drawPlayerModeMessagePanelTask(void) {
    if (D_8013AE88->data18 == 99) {
        func_800149A8();
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30(-112, -24, 14, gPlayerModeMessageTexts[0], 8, -1,
                  &D_800EBFF0[D_8013AF78], 3);
    drawDefaultTexturedPanel(-120, -32, 240, 64, 5, 0,
                            &D_800EBFF0[D_8013AF78], 3);
}

void startUnlockNotification0Task(void) {
    GameTask *task = D_8013AE88;
    GameTask *parent = D_8013AE98;

    task->data18 = 1;
    parent->data1C++;
    startUnlockNotification(0);
    setCurrentTaskCallback(waitUnlockNotification0TransitionTask);
}

void waitUnlockNotification0TransitionTask(void) {
    if (D_8013AFE4 & 0x200) {
        D_8013AE98->data18 = 5;
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(1);
        D_8013B10C = -1;
        queueMode11TransitionResources();
        queueBackgroundTextureSectorCommands();
        func_8002FA58((s16)D_8013AE98->data18);
        setCurrentTaskCallback(func_80032F3C);
        D_8013AE88->unk10 |= 2;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80032F3C);

void drawSound601SequenceSpriteTask(void) {
    if (gameOptionPlayerModeAbsolute == 1) {
        func_8006C998(-48, -12, 0x1E0, 6, -1, &D_800EC018[D_8013AF78], 1);
    } else if (D_8013AE88->data18 == 0) {
        if (func_8003D904(0)) {
            func_8006C998(-40, -12, 0x1E0, 6, -1, &D_800EC018[D_8013AF78], 1);
        }
    } else {
        if (func_8003D904(1)) {
            func_8006C998(-40, -12, 0x1E0, 6, -1, &D_800EC0B8[D_8013AF78], 1);
        }
    }
    if (D_8013AFE4 & 8) {
        func_800149A8();
    }
}

void beginSound601TaskSequence(void) {
    playSoundEffect(0, 0x601, 0x20);
    setCurrentTaskCallback(drawSound601SequenceSpriteTask);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80033100);

void initializePlayerPromptTask(void) {
    if (D_8013AFCD == 1) {
        D_8013AE88->data1C = 0;
        D_8013AE88->data1E = 0;
    } else {
        D_8013AE88->data1C = -32;
        D_8013AE88->data1E = D_8013AE88->data18 * 112 - 48;
    }
    setCurrentTaskCallback(func_80033100);
}


void drawGrowingSprite179Task(void) {
    GameTask *task = D_8013AE88;
    if (task->data1C < 0x1000) {
        task->data1C += task->data1E;
        if (task->data1C >= 0x1000) {
            task->data1C = 0x1000;
        }
    }
    if (--D_8013AE88->data1A == 0) {
        func_800149A8();
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)D_8013AE88->data18, 0, 0x179, 6, -1,
        0x7FFF, 0x7FFF, (s16)D_8013AE88->data1C,
        (s16)D_8013AE88->data1C, 0, &D_800EBFF0[D_8013AF78], 1);
}

void drawGrowingSprite178Task(void) {
    GameTask *task = D_8013AE88;
    if (task->data1C < 0x1000) {
        task->data1C += task->data1E;
        if (task->data1C >= 0x1000) {
            task->data1C = 0x1000;
        }
    }
    if (--D_8013AE88->data1A == 0) {
        D_8013AE88->data1E = 0x400;
        D_8013AE88->data1C = 0;
        D_8013AE88->data1A = 30;
        setCurrentTaskCallback(drawGrowingSprite179Task);
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)D_8013AE88->data18, 0, 0x178, 6, -1,
        0x7FFF, 0x7FFF, (s16)D_8013AE88->data1C,
        (s16)D_8013AE88->data1C, 0, &D_800EBFF0[D_8013AF78], 1);
}

void waitThenStartSpriteScaleTransition(void) {
    GameTask *task = D_8013AE88;

    if (--task->data1A == 0) {
        task->data1E = 0x200;
        task->data1C = 0;
        task->data1A = 60;
        setCurrentTaskCallback(drawGrowingSprite178Task);
    }
}

void beginSpriteScaleTransitionDelay(void) {
    if (D_8013AFCD == 1) {
        D_8013AE88->data18 = 0;
    } else {
        D_8013AE88->data18 = -30;
    }
    D_8013AE88->data1A = 45;
    setCurrentTaskCallback(waitThenStartSpriteScaleTransition);
}

void drawSlidingRaceTimeDisplay(SlidingTimeResource *resource) {
    u8 padding[0x20];
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    setSpriteColor(128, 128, 128);
    if (resource->hasExtraTime) {
        func_8006C998(resource->x - 102, resource->y, 0x1AE, 0, -1, ot, 1);
        drawRaceTime(&resource->time, resource->x + 38, resource->y, 1, ot, 1);
    } else {
        func_8006C998(resource->x - 98, resource->y, 0x1AD, 4, -1, ot, 1);
        drawRaceTime(&resource->time, resource->x + 34, resource->y, 2, ot, 1);
    }
}

void slideRaceTimeDisplayOut(void) {
    SlidingTimeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed += 4;
    if (resource->speed == 56) {
        func_800149A8();
    } else {
        drawSlidingRaceTimeDisplay(resource);
    }
}

void waitSpriteMotionTask(void) {
    SlidingTimeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideRaceTimeDisplayOut);
    }
    drawSlidingRaceTimeDisplay(resource);
}

void slideRaceTimeDisplayIn(void) {
    SlidingTimeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed -= 4;
    if (resource->speed == 0) {
        resource->timer = 90;
        setCurrentTaskCallback(waitSpriteMotionTask);
    }
    drawSlidingRaceTimeDisplay(resource);
}

void initializeRaceTimeDifferenceDisplay(void) {
    SlidingTimeResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->y = -44;
    resource->x = 420;
    resource->speed = 56;
    resource->hasExtraTime = calculateRaceTimeDifference(&D_8013B188, &D_80124050[D_8013AFCE], &resource->time);
    setCurrentTaskCallback(slideRaceTimeDisplayIn);
}

void drawFallingSpriteEffect(FallingSpriteEffect *effect) {
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex(effect->x >> 4, effect->y >> 4, effect->frame + 378,
                  effect->palette, -1, 0x7FFF, 0x7FFF, effect->scale,
                  effect->scale, 0, &D_800EBFF0[D_8013AF78], 2);
}

void updateDriftingFallingSpriteEffect(FallingSpriteEffect *effect) {
    effect->frame = (effect->frame + 1) & 3;
    if (effect->unk0E >= 0) {
        if (effect->unk10 != 0) {
            effect->velocityX += 4;
            effect->unk10--;
        } else {
            if ((effect->velocityX -= 4) == 0) {
                effect->unk10 = (randomNextMain() & 31) + 16;
                effect->unk0E = -1;
            }
        }
    } else {
        if (effect->unk10 != 0) {
            effect->velocityX -= 4;
            effect->unk10--;
        } else {
            if ((effect->velocityX += 4) == 0) {
                effect->unk10 = (randomNextMain() & 31) + 16;
                effect->unk0E = 1;
            }
        }
    }
    effect->x += effect->velocityX;
    effect->y += effect->velocityY;
    if (effect->y > 1760) {
        effect->active = 0;
    } else {
        drawFallingSpriteEffect(effect);
    }
}

void updateSwayingFallingSpriteEffect(FallingSpriteEffect *effect) {
    effect->frame = (effect->frame + 1) & 3;
    effect->y += effect->velocityY;
    effect->x += rsin(effect->unk0E = (effect->unk0E + 32) & 0xFFF) * (effect->unk08 + 48) / 4096;
    if (effect->unk0E == 0 || effect->unk0E == 0x800) {
        effect->unk08 = randomNextMain() & 15;
    }
    if (effect->y > 1760) {
        effect->active = 0;
    } else {
        drawFallingSpriteEffect(effect);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80033C2C);

void initializeFallingSpriteEffectsTask(void) {
    FallingSpriteEffect *effects;
    s16 i;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    effects = (FallingSpriteEffect *)&D_800D9504[D_8013AE88->resource14];
    for (i = 0; i < 10; i++) {
        effects[i].active = 0;
    }
    setCurrentTaskCallback(func_80033C2C);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80033FAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80033FE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80034028);

void drawSlidingDisplaySprite(SlidingDisplayResource *resource) {
    s32 x = (resource->position >> 1) - 56;
    if (gameOptionPlayerModeAbsolute == 1) {
        func_8006C998(x, 8, 0x1DF, 6, -1, &D_800EC018[D_8013AF78], 1);
    } else if (D_8013AE88->data18 == 0) {
        if (func_8003D904(0)) {
            func_8006C998(x, 8, 0x1DF, 6, -1, &D_800EC018[D_8013AF78], 1);
        }
    } else {
        if (func_8003D904(1)) {
            func_8006C998(x, 8, 0x1DF, 6, -1, &D_800EC0B8[D_8013AF78], 1);
        }
    }
}

void slideOutSlidingDisplayTask(void) {
    SlidingDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->position += resource->velocity;
    resource->velocity += 4;
    if (resource->velocity == 56) {
        func_800149A8();
    } else {
        drawSlidingDisplaySprite(resource);
    }
}

void waitSlidingDisplayTask(void) {
    SlidingDisplayResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutSlidingDisplayTask);
    }
    drawSlidingDisplaySprite(resource);
}

void slideInSlidingDisplayTask(void) {
    SlidingDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer != 0) {
        resource->timer--;
    } else {
        resource->position += resource->velocity;
        resource->velocity -= 4;
        if (resource->velocity == 0) {
            resource->timer = 30;
            setCurrentTaskCallback(waitSlidingDisplayTask);
        }
    }
    drawSlidingDisplaySprite(resource);
}

void initializeTaskMotionResource(void) {
    TaskMotionResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskMotionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->velocity = 60;
    resource->position = -480;
    resource->delay = 10;
    setCurrentTaskCallback(slideInSlidingDisplayTask);
}

void drawPlayerSpriteAnimation(SpriteAnimationResource *resource) {
    GsOT *ot;
    s32 palette;
    if (resource->checkPlayer == 1) {
        if (!func_8003D904(resource->player)) return;
    }
    if (resource->player == 0) {
        ot = &D_800EC018[D_8013AF78];
    } else {
        ot = &D_800EC0B8[D_8013AF78];
    }
    setSpriteColor(128, 128, 128);
    palette = 3;
    if (!resource->alternateLayer) palette = 2;
    func_8006C998(resource->x, resource->y, (resource->frame >> 1) + 0x1E2,
                 palette, -1, ot, 0);
}

void advancePlayerSpriteAnimationTask(void) {
    SpriteAnimationResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteAnimationResource *)&D_800D9504[D_8013AE88->resource14];
    if (++resource->frame >= 16) {
        func_800149A8();
    } else {
        drawPlayerSpriteAnimation(resource);
    }
}

void createPlayerSpriteAnimationTask(s32 x, s32 y, s32 player, s32 checkPlayer, u16 alternateLayer) {
    SpriteAnimationResource *resource;
    func_80014874(advancePlayerSpriteAnimationTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (SpriteAnimationResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = x - 8;
    resource->y = y - 8;
    resource->player = player;
    resource->checkPlayer = checkPlayer;
    resource->alternateLayer = alternateLayer;
    resource->frame = 0xFFFF;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80034664);

void slideOutPlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x += resource->speed;
    resource->speed += 4;
    if (resource->speed >= 56) {
        func_800149A8();
    } else {
        func_80034664(resource);
    }
}

void waitPlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutPlayerNumberDisplayTask);
    }
    func_80034664(resource);
}

void slideInPlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed -= 4;
    if (resource->speed <= 0) {
        resource->speed = 0;
        resource->timer = 45;
        setCurrentTaskCallback(waitPlayerNumberDisplayTask);
    }
    func_80034664(resource);
}

void initializePlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->y = -44;
    resource->speed = 56;
    if (D_8013AFE2 == 0 && D_8013AFCD == 2) {
        resource->x = 452;
    } else if (D_8013AFE2 == 1 && D_8013AFE0 == 2 && D_8013AFCD == 2) {
        resource->x = 460;
    } else {
        resource->x = 420;
    }
    setCurrentTaskCallback(slideInPlayerNumberDisplayTask);
}

void createPlayerNumberDisplayTask(s32 value, s32 showExtraValue, s32 player) {
    PlayerNumberResource *resource;
    func_80014874(initializePlayerNumberDisplayTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->value = value;
    resource->showExtraValue = showExtraValue;
    resource->player = player;
}

void drawSpriteMotionDisplay(SpriteMotionResource *resource) {
    s16 draw = 0;
    GsOT *ot;
    s16 depth = 1;
    setSpriteColor(128, 128, 128);
    if (D_8013AFCD == 1) {
        draw = 1;
        ot = &D_800EC018[D_8013AF78];
    } else if (resource->variant == 0) {
        if (func_8003D904(0)) {
            draw = 1;
            ot = &D_800EC018[D_8013AF78];
        }
    } else {
        if (func_8003D904(1)) {
            draw = 1;
            ot = &D_800EC0B8[D_8013AF78];
        }
    }
    if (draw) {
        if (D_8013AFCD == 1) {
            if (D_8013AFE2 == 1 && D_8013AFE0 == 2) {
                func_8006C998(-60, resource->unk08, 0x1AB, 8, -1, ot, depth);
            } else {
                func_8006C998(-36, resource->unk08, 0x1AB, 8, -1, ot, depth);
            }
        } else {
            func_8006C998(-30, (s16)(resource->unk08 + 32), 0x1AB, 8, -1, ot, depth);
        }
    }
}

void slideOutSpriteMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk04 += resource->unk14;
    resource->unk14 += 4;
    if (resource->unk14 >= 56) {
        func_800149A8();
    } else {
        drawSpriteMotionDisplay(resource);
    }
}

void waitSpriteExitMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutSpriteMotionTask);
    }
    drawSpriteMotionDisplay(resource);
}

void slideInSpriteMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk04 -= resource->unk14;
    resource->unk14 -= 4;
    if (resource->unk14 <= 0) {
        resource->unk14 = 0;
        resource->timer = 45;
        setCurrentTaskCallback(waitSpriteExitMotionTask);
    }
    drawSpriteMotionDisplay(resource);
}

void initializeSpriteMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AFCD == 1) {
        resource->unk08 = 8;
    } else {
        resource->unk08 = -8;
    }
    resource->unk14 = 0x38;
    resource->unk04 = 0x1A4;
    setCurrentTaskCallback(slideInSpriteMotionTask);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003504C);

void drawSlidingNumberDisplay(SlidingNumberResource *resource) {
    s16 visible = 0;
    GsOT *ot;
    setSpriteColor(128, 128, 128);
    if (D_8013AFCD == 1) {
        visible = 1;
        ot = &D_800EC018[D_8013AF78];
    } else if (resource->player == 0) {
        if (func_8003D904(0)) {
            visible = 1;
            ot = &D_800EC018[D_8013AF78];
        }
    } else {
        if (func_8003D904(1)) {
            visible = 1;
            ot = &D_800EC0B8[D_8013AF78];
        }
    }
    if (visible) {
        func_8006DFB4((s16)(resource->x - 8), (s16)(resource->y - 8), 5, 0,
                     resource->value, 3, -1, ot, 1);
        func_8006C998((s16)(resource->x - 60), (s16)(resource->y - 4), 0x1B4, 4, -1, ot, 1);
        func_8006C998((s16)(resource->x + 44), (s16)(resource->y - 4), 0x1B5, 4, -1, ot, 1);
    }
}

void slideOutSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x += resource->speed;
    resource->speed += 4;
    if (resource->speed >= 56) {
        func_800149A8();
    } else {
        drawSlidingNumberDisplay(resource);
    }
}

void waitSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutSlidingNumberDisplayTask);
    }
    drawSlidingNumberDisplay(resource);
}

void slideInSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed -= 4;
    if (resource->speed <= 0) {
        resource->speed = 0;
        resource->timer = 45;
        setCurrentTaskCallback(waitSlidingNumberDisplayTask);
    }
    drawSlidingNumberDisplay(resource);
}

void initializeSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AFCD == 1) {
        resource->y = -36;
    } else {
        resource->y = -22;
    }
    resource->speed = 56;
    if (D_8013AFE2 == 0 && D_8013AFCD == 2) {
        resource->x = 452;
    } else if (D_8013AFE2 == 1 && D_8013AFE0 == 2 && D_8013AFCD == 2) {
        resource->x = 460;
    } else {
        resource->x = 420;
    }
    setCurrentTaskCallback(slideInSlidingNumberDisplayTask);
}

void createActivePlayerSlidingNumberTask(s32 value, s16 player) {
    SlidingNumberResource *resource;
    if (D_80120E90[player].unk024 != 0) {
        func_80014874(initializeSlidingNumberDisplayTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SlidingNumberResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->value = value;
        resource->player = player;
    }
}

void drawFallingPairedVisualTask(void) {
    FallingVisualResource *resource;
    GsCOORDINATE2 *base;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    base = (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource16];
    resource->coordinate.coord.t[0] = base->coord.t[0];
    resource->coordinate.coord.t[1] = base->coord.t[1] + (resource->height >> 8);
    resource->coordinate.coord.t[2] = base->coord.t[2];
    resource->framePosition = 0;
    setTaskVisualFrame((TaskVisualResource *)resource, gFallingPairedVisualFrames, 0x1E00);
    func_8005AC4C((TaskVisualResource *)resource, 0);
    resource->coordinate.coord.t[1] += 0x49;
    resource->framePosition = gFallingPairedVisualAnimation[resource->animationTick >> 1] << 8;
    setTaskVisualFrame((TaskVisualResource *)resource, gFallingPairedVisualFrames, 0x1E00);
    func_8005AC4C((TaskVisualResource *)resource, 0);
}

void updateFallingPairedVisualTask(void) {
    FallingVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (resource->delay > 0) {
            resource->delay--;
        } else {
            resource->height += resource->velocity;
            resource->velocity -= 30;
            if (resource->height <= -48000 && resource->velocity < 0) {
                if (!resource->triggered) {
                    playSoundEffect(1, 0x600, 0x20);
                    resource->triggered = 1;
                }
            }
            if (resource->triggered) {
                if (resource->animationTick < 11) {
                    resource->animationTick++;
                }
            }
        }
    }
    drawFallingPairedVisualTask();
}

void initializeBouncingVisualTask(void) {
    BouncingVisualResource *resource;
    GsCOORDINATE2 *position;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (BouncingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    position = (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource16];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 128;
    resource->code = POLY_FT4_CODE;
    resource->timer = 0;
    resource->framePosition = 0;
    if (D_8013AFD2 == 1) {
        position->coord.t[0] = D_80120E90[0].worldCoordinate.coord.t[0];
    } else {
        position->coord.t[0] = 0;
    }
    position->coord.t[1] = D_80120E90[0].worldCoordinate.coord.t[1];
    position->coord.t[2] = D_80120E90[0].worldCoordinate.coord.t[2] + 50;
    resource->offsetY = -37120;
    resource->velocity = 0;
    resource->frame = 0;
    do {
        resource->velocity += 30;
        resource->offsetY -= resource->velocity;
    } while (resource->offsetY > -89600);
    resource->soundPlayed = 0;
    resource->delay = 15;
    setCurrentTaskCallback(updateFallingPairedVisualTask);
}

void drawFixedSpriteAndBottomBarTask(void) {
    addTexturedSprite(D_8013AF6C, 0x1C, 0x3DFE, 0x28, 0x52,
                      0x68, 0x18, 8, 0xD8, 0x80, 0x80, 0x80);
    addFlatRectangle(D_8013AF6C + 1, -(D_8013AF7A >> 1),
                     (s16)((D_8013AF7C >> 1) - 0x20),
                     (s16)D_8013AF7A, 0x20, 0, 0, 0);
}

void drawDemonstrationOverlay(void) {
    setSpriteColor(128, 128, 128);
    drawUppercaseProportionalText(-52, -8, 20, gDemonstrationText, 1, -1, &D_800EBFF0[D_8013AF78], 3);
    if (gFrameCounter & 8) {
        drawUppercaseProportionalText(-40, 28, 20, gDemonstrationStartText, 2, -1, &D_800EBFF0[D_8013AF78], 3);
    }
}

void drawAutoPilotLabelTask(void) {
    s32 frame;
    if (!(D_8013B0D8->unk128 & 0x40) && !D_8013AFD6) {
        setSpriteColor(128, 128, 128);
        if (D_8013AFCD == 1) {
            frame = gFrameCounter & 2;
            drawUppercaseProportionalText(-40, -18, 20, gAutoPilotLabel,
                         (frame / 2) + 1, -1,
                         &D_800EBFF0[D_8013AF78], 3);
        } else if (D_8013AE88->data18 == 0) {
            frame = gFrameCounter & 2;
            drawUppercaseProportionalText(-72, -66, 20, gAutoPilotLabel,
                         (frame / 2) + 1, -1,
                         &D_800EBFF0[D_8013AF78], 3);
        } else {
            frame = gFrameCounter & 2;
            drawUppercaseProportionalText(-72, 46, 20, gAutoPilotLabel,
                         (frame / 2) + 1, -1,
                         &D_800EBFF0[D_8013AF78], 3);
        }
    }
}

void checkControllerInputSequenceTask(void) {
    if (D_8013AEE8[0] != 0) {
        if (D_8013AEE8[0] == gControllerInputSequences[D_8013AE88->data18][D_8013AE88->data1A]) {
            D_8013AE88->data1A++;
            if (gControllerInputSequences[D_8013AE88->data18][D_8013AE88->data1A] == 0xFFFF) {
                D_8013B11A |= 1 << D_8013AE88->data18;
                playSoundEffect(0, 0, 0x20);
                func_800149A8();
            }
        } else {
            D_8013AE88->data1A = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80035CF8);

void drawResultDisplayLabels(ResultDisplayResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    func_8006C998(-104, -44, 392, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    func_8006C998((s16)(-104 - gResultLabelXOffsets[resource->unk08]), 6,
                  resource->unk08 + 382, gResultLabelPalettes[resource->unk08], -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_800401DC(-66, -12, resource->unk06, &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(-34, 4, resource->unk06 + 393, gResultIconPalettes[resource->unk06], -1,
                  &D_800EBFF0[D_8013AF78], 1);
}

void drawResultNumberAndIcon(ResultDisplayResource *resource) {
    setSpriteColor(128, 128, 128);
    func_8006DFB4(32, -63, 5, 0, resource->unk04, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, -63, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawFirstPlayerResultCounterAndIcon(ResultDisplayResource *resource) {
    s32 palette = 1;
    if (resource->unk06 == 3) {
        palette = 4;
    }
    setSpriteColor(128, 128, 128);
    if (resource->unk06 == 3) {
        func_8006C998(24, -11, 397, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    }
    func_8006DFB4(32, -15, 5, 0, D_80120E90[0].unk01C, palette, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, -15, 26, palette, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawFirstPlayerResultNumberAndIcon(ResultDisplayResource *resource) {
    setSpriteColor(128, 128, 128);
    func_8006DFB4(24, 33, 6, 0, D_80120E90[0].unk010, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, 33, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawResultPrizeLabels(ResultDisplayResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness,
                       (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawLimitedText8x8(8, -72, 12, gResultRankPrizeLabel, 3, -1,
                      &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(0, -24, 12, gResultGetCoinLabel, 3, -1,
                      &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(4, 24, 13, gResultTotalMoneyLabel, 3, -1,
                      &D_800EBFF0[D_8013AF78], 1);
}

void waitResultDisplayTaskFlagsTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if ((D_8013AFE4 & 0x50) != 0x10) {
        func_800149A8();
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawFirstPlayerResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultFlagsInputTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(waitResultDisplayTaskFlagsTask);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawFirstPlayerResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80036454);

void waitResultDisplayInputTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        resource->timer = 0;
        setCurrentTaskCallback(func_80036454);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawFirstPlayerResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultDisplayConfirmationTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(waitResultDisplayInputTask);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultNumberDisplayTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(waitResultDisplayConfirmationTask);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultDisplayTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(waitResultNumberDisplayTask);
    }
    drawResultDisplayLabels(resource);
    drawResultPrizeLabels(resource);
}

void fadeInResultDisplayTask(void) {
    ResultDisplayResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 20;
        resource->brightness = 128;
        setCurrentTaskCallback(waitResultDisplayTask);
    }
    drawResultDisplayLabels(resource);
    drawResultPrizeLabels(resource);
}

void initializeResultDisplayTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    resource->unk06 = (s8)D_80120E90[0].unk935;
    resource->unk04 = 0;
    if (resource->unk06 < 3) {
        resource->unk04 = D_8012FC6C[D_8013AFCE][resource->unk06];
    }
    D_80120E90[0].unk010 = D_80130BF0[0];
    resource->unk08 = D_80120E90[0].unk014;
    setCurrentTaskCallback(fadeInResultDisplayTask);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80036BA4);

void drawPlayerResultRowValues(s16 row, s16 x, s16 y, TaskFadeResource *resource) {
    char text[32];
    s32 length;
    s32 mode;
    if (row != 0 || (gFrameCounter & 1)) {
        mode = 6;
    } else {
        mode = 0;
    }
    sprintf(text, gResultValueFormat, D_80120E90[resource->playerOrder[0]].unk02A);
    length = strlen(text);
    drawLimitedText8x8(x + 104, y + 24, 1, gResultValueMultiplierLabel, mode, -1, &D_800EBFF0[D_8013AF78], 1);
    func_8006D66C(x + 112, y + 24, length, 0, D_80120E90[resource->playerOrder[row]].unk02A, mode, -1, &D_800EBFF0[D_8013AF78], 1);
    if (D_80120E90[resource->playerOrder[row]].unk008 == 0) {
        func_8006D66C(x + 88, y + 6, 1, 0, D_80120E90[resource->playerOrder[row]].playerIndex + 1, 5, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(x + 96, y + 6, 1, gPlayerNumberSuffixLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    } else {
        drawLimitedText8x8(x + 88, y + 6, 3, gComputerPlayerLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
    }
}

void drawPlayerResultRows(TaskFadeResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    if (resource->playerOrder[0] != -1) {
        drawPlayerResultRowValues(0, -136, -80, resource);
    }
    if (resource->playerOrder[1] != -1) {
        drawPlayerResultRowValues(1, 8, -48, resource);
    }
    if (resource->playerOrder[2] != -1) {
        drawPlayerResultRowValues(2, -136, -16, resource);
    }
    if (resource->playerOrder[3] != -1) {
        drawPlayerResultRowValues(3, 8, 16, resource);
    }
}

void updatePlayerResultOrder(TaskFadeResource *resource) {
    s32 i, j;
    s32 temp;
    for (i = 0; i < D_8013AFD2; i++) {
        resource->playerOrder[i] = i;
    }
    for (i = 0; i < D_8013AFD2 - 1; i++) {
        for (j = i + 1; j < D_8013AFD2; j++) {
            if (D_80120E90[resource->playerOrder[i]].unk935 >
                D_80120E90[resource->playerOrder[j]].unk935) {
                temp = resource->playerOrder[i];
                resource->playerOrder[i] = resource->playerOrder[j];
                resource->playerOrder[j] = temp;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (!(D_80120E90[resource->playerOrder[i]].unk128 & 0x40)) {
            resource->playerOrder[i] = -1;
        }
    }
}

void drawCurrentTaskPlayerResults(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        func_800149A8();
    }
    updatePlayerResultOrder((TaskFadeResource *)resource);
    func_80036BA4(resource);
    drawPlayerResultRows((TaskFadeResource *)resource);
}

void waitPlayerResultsInputTask(void) {
    s32 player;
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    for (player = 0; player < gameOptionPlayerModeAbsolute; player++) {
        if (D_8013AEE8[player] & 0xB0) {
            D_8013AFE4 |= 0x10;
            setCurrentTaskCallback(drawCurrentTaskPlayerResults);
        }
    }
    updatePlayerResultOrder((TaskFadeResource *)resource);
    func_80036BA4(resource);
    drawPlayerResultRows((TaskFadeResource *)resource);
}

void fadeInPlayerResultsTask(void) {
    TaskFadeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 150;
        resource->brightness = 128;
        setCurrentTaskCallback(waitPlayerResultsInputTask);
    }
    updatePlayerResultOrder((TaskFadeResource *)resource);
    func_80036BA4((TaskResourceRecord *)resource);
    drawPlayerResultRows(resource);
}

void initializeTaskFade(void) {
    TaskFadeResource *resource;

    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    setCurrentTaskCallback(fadeInPlayerResultsTask);
}

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
        drawRaceTime(&((TimeMenuSaveStateView *)&D_8013080C)->times[D_8013AFCE][i],
                     16, y, mode, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(88, y,
                     (((TimeMenuSaveStateView *)&D_8013080C)->icons[D_8013AFCE][i] & 15) * 3 + 0x14A,
                     D_80124284[((TimeMenuSaveStateView *)&D_8013080C)->icons[D_8013AFCE][i] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(108, y,
                     (((TimeMenuSaveStateView *)&D_8013080C)->icons[D_8013AFCE][i] >> 4) + 0x13B,
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80038090);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80038270);

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
        func_8006DFB4(16, y, 2, 0, ((CountRecordSaveView *)&D_8013080C)->scores[D_8013AFCE][i], mode, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(70, y, (((CountRecordSaveView *)&D_8013080C)->icons[D_8013AFCE][i] & 15) * 3 + 0x14A,
            D_80124284[((CountRecordSaveView *)&D_8013080C)->icons[D_8013AFCE][i] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(88, y, (((CountRecordSaveView *)&D_8013080C)->icons[D_8013AFCE][i] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
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
    func_80038090(resource);
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
    func_80038090(resource);
    drawCountRecordRows(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80038B34);

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
    func_80038090((TaskResourceRecord *)resource);
    func_80038270((TaskResourceRecord *)resource);
    drawHitPrizeMoneyLabels((MenuFadeInResource *)resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80038C70);

void advanceTaskMenuCountdown(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 3;
    if (--resource->timer == 0) {
        setCurrentTaskCallback(func_80038C70);
    }
    func_80038090((TaskResourceRecord *)resource);
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
    func_80038090((TaskResourceRecord *)resource);
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
    func_80038090((TaskResourceRecord *)resource);
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
    func_80038090((TaskResourceRecord *)resource);
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
    func_80038090((TaskResourceRecord *)resource);
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
    func_80038090((TaskResourceRecord *)resource);
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
    func_80038090((TaskResourceRecord *)resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80039564);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800398F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80039B34);

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
    func_800398F4(resource);
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
    func_800398F4(resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003A158);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003AAE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003AD48);

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
        func_8006DFB4(44, y, 5, 0, ((StatsRecordSaveView *)&D_8013080C)->scores[D_8013AFCE][i], mode, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(102, y, (((StatsRecordSaveView *)&D_8013080C)->icons[D_8013AFCE][i] & 15) * 3 + 0x14A,
            D_80124284[((StatsRecordSaveView *)&D_8013080C)->icons[D_8013AFCE][i] & 15], -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(120, y, (((StatsRecordSaveView *)&D_8013080C)->icons[D_8013AFCE][i] >> 4) + 0x13B, 7, -1, &D_800EBFF0[D_8013AF78], 1);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003BD70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003BE58);

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

s16 compareRaceTimes(const RaceTime *a, const RaceTime *b) {
    s32 first = (a->centiseconds & 0xFF00) + a->seconds * 25600 + a->minutes * 1536000;
    s32 second = (b->centiseconds & 0xFF00) + b->seconds * 25600 + b->minutes * 1536000;
    s32 result;

    if (first > second) {
        result = 1;
    } else if (first < second) {
        result = -1;
    } else {
        result = 0;
    }
    return result;
}

void assignTwoPlayerResultSlots(s16 result) {
    if (result == 1) {
        D_80120E90[0].unk100[0] = 0;
        D_80120E90[1].unk100[0] = 1;
    } else if (result == 2) {
        D_80120E90[0].unk100[0] = 1;
        D_80120E90[1].unk100[0] = 0;
    } else if ((rand() & 1) == 0) {
        D_80120E90[0].unk100[0] = 2;
        D_80120E90[1].unk100[0] = 3;
    } else {
        D_80120E90[0].unk100[0] = 3;
        D_80120E90[1].unk100[0] = 2;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003CAF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003D904);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003D940);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003DB94);

void stopPlayerSound(u16 playerIndex) {
    RacePlayer *players = D_80120E90;
    RacePlayer *player = &players[playerIndex];

    player->unk96C = 0;
    if (player->soundVoice != 0xFF) {
        func_800187B8(player->soundVoice);
        player->soundVoice = 0xFF;
        player->unk96E = 0xFFFF;
        player->unk970 = 0xFFFF;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003DEE0);

void initPlayerCamera(u16 index) {
    GsRVIEW2 *view;
    GsRVIEW2 *initialView;
    PlayerCamera *camera;
    GsRVIEW2 *views;
    GsRVIEW2 *initialViews;
    PlayerCamera *cameras;
    RacePlayer *players;
    GsSetProjection(D_8013AF84);
    initialViews = gPlayerCameraViews;
    initialView = &initialViews[index];
    initialView->vpx = 0;
    initialView->vpy = 0;
    initialView->vpz = 0;
    initialView->vrx = 0;
    initialView->vry = 0;
    if (!(D_8013AFAA & 0xC00)) initialView->vrz = 320;
    else initialView->vrz = 576;
    views = gPlayerCameraViews;
    view = &views[index];
    cameras = D_801241E8;
    camera = &cameras[index];
    players = D_80120E90;
    view->rz = 0;
    view->super = 0;
    camera->yaw = players[index].unk0B6 + 0x400;
    camera->pitch = -16;
    camera->unk10 = view->vpz - view->vrz;
    camera->unk12 = camera->unk14 = 0;
    camera->unk16 = 1;
    camera->unk1A = 0;
    GsSetRefView2L(view);
}

void updatePlayerCameraPosition(u16 index) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[index];
    RacePlayer *players;
    RacePlayer *player;
    GsRVIEW2 *views;
    GsRVIEW2 *view;
    RotMatrixYXZ((D_8013AF90->vx = camera->pitch & 0xFFF,
                  D_8013AF90->vy = camera->yaw & 0xFFF,
                  D_8013AF90->vz = 0, D_8013AF90), D_8013AF98);
    D_8013AF94->vx = 0;
    D_8013AF94->vy = 0;
    D_8013AF94->vz = camera->unk10 - camera->unk12 + camera->unk14;
    ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
    players = D_80120E90;
    player = &players[index];
    camera->x = player->x + (D_8013AF88->vx << 8);
    camera->y = player->y + (D_8013AF88->vy << 8);
    camera->z = player->z + (D_8013AF88->vz << 8);
    views = gPlayerCameraViews;
    view = &views[index];
    view->vpx = camera->x / 256;
    view->vpy = camera->y / 256 - 120;
    view->vpz = camera->z / 256;
}

void updatePlayerCameraPitch(u16 playerIndex) {
    RacePlayer *players = D_80120E90;
    RacePlayer *player = &players[playerIndex];
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    s32 x = (player->x - camera->x) / 8;
    s32 y = (player->y - camera->y) / 8;
    s32 z = (player->z - camera->z) / 8;
    s32 scale = 1;
    while (1) {
        if ((u32)(x + 32767) <= 65534 && (u32)(z + 32767) <= 65534) {
            break;
        }
        scale *= 2;
        x /= 2;
        z /= 2;
    }
    D_801241E8[playerIndex].pitch = func_8004A124(0, 0, SquareRoot0(x * x + z * z), y / scale) - 0x400;
}

void updatePlayerCameraAngles(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    RacePlayer *players = D_80120E90;
    RacePlayer *player = &players[playerIndex];
    camera->yaw = func_8004A124(camera->x, camera->z, player->x, player->z);
    updatePlayerCameraPitch(playerIndex);
}

void clampPlayerCameraDistance(u16 playerIndex) {
    GsRVIEW2 *views = gPlayerCameraViews;
    GsRVIEW2 *view = &views[playerIndex];
    s32 dx = view->vpx - view->vrx;
    s32 dz = view->vpz - view->vrz;
    s32 dy = view->vpy - view->vry;
    s32 distance = SquareRoot0(dx * dx + dz * dz);
    PlayerCamera *cameras;
    PlayerCamera *camera;
    distance = SquareRoot0(distance * distance + dy * dy);
    if (distance < 320) {
        cameras = D_801241E8;
        camera = &cameras[playerIndex];
        D_8013AF90->vx = camera->pitch & 0xFFF;
        D_8013AF90->vy = camera->yaw & 0xFFF;
        D_8013AF90->vz = 0;
        RotMatrixYXZ(D_8013AF90, D_8013AF98);
        D_8013AF94->vx = 0;
        D_8013AF94->vy = 0;
        D_8013AF94->vz = -320;
        ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
        view->vpx = view->vrx + D_8013AF88->vx;
        view->vpy = view->vry + D_8013AF88->vy;
        view->vpz = view->vrz + D_8013AF88->vz;
    }
}

void clampPlayerCameraHeight(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    GsRVIEW2 *views = gPlayerCameraViews;
    GsRVIEW2 *view = &views[playerIndex];
    s32 height;
    camera->unk18 = getAdjacentGeometryRegionForPoint(camera->unk18, view->vpx, view->vpz);
    height = func_8004A764(camera->unk18, view->vpx, view->vpz);
    if (height - 120 < view->vpy) {
        view->vpy = height - 120;
        camera->y = height << 8;
        updatePlayerCameraPitch(playerIndex);
    }
}

void initPlayerCameraAndTarget(u16 index) {
    GsRVIEW2 *targets;
    RacePlayer *players;
    PlayerCamera *cameras;
    initPlayerCamera(index);
    cameras = D_801241E8;
    cameras[index].unk18 = 0;
    updatePlayerCameraPosition(index);
    clampPlayerCameraHeight(index);
    targets = gPlayerCameraViews;
    players = D_80120E90;
    targets[D_8013AE88->data18].vrx = players[D_8013AE88->data18].worldCoordinate.coord.t[0] + players[D_8013AE88->data18].unk084;
    targets[D_8013AE88->data18].vry = players[D_8013AE88->data18].worldCoordinate.coord.t[1] + (s16)(cameras[D_8013AE88->data18].unk14 / 8 - 120) + players[D_8013AE88->data18].unk086;
    targets[D_8013AE88->data18].vrz = players[D_8013AE88->data18].worldCoordinate.coord.t[2] + players[D_8013AE88->data18].unk088;
}

void initPlayerZeroTask(void) {
    initPlayerCameraAndTarget(0);
    setCurrentTaskCallback(func_8003EE70);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003E794);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003EA20);

void updateCurrentPlayerCameraTarget(void) {
    GsRVIEW2 *targets = gPlayerCameraViews;
    RacePlayer *players = D_80120E90;
    targets[D_8013AE88->data18].vrx = players[D_8013AE88->data18].worldCoordinate.coord.t[0] + players[D_8013AE88->data18].unk084;
    targets[D_8013AE88->data18].vry = players[D_8013AE88->data18].worldCoordinate.coord.t[1] + (s16)(D_801241E8[D_8013AE88->data18].unk14 / 8 - 120) + players[D_8013AE88->data18].unk086;
    targets[D_8013AE88->data18].vrz = players[D_8013AE88->data18].worldCoordinate.coord.t[2] + players[D_8013AE88->data18].unk088;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8003EE70);

void addFourWordGpuPacket(GpuPacket4 *source, GsOT *ot, u16 depth) {
    GpuPacket4 *packet = (GpuPacket4 *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800400C4);

void addTexturedSpritePacket(TexturedSpritePacket *source, GsOT *ot, u16 depth) {
    TexturedSpritePacket *packet = (TexturedSpritePacket *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800401DC);

void drawMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, GsOT *ot, u16 depth) {
    s32 frameIndex = frame;
    s32 sprite;
    if (kind == 0) {
        frame = 5;
        sprite = 0x1BE;
    } else {
        s32 baseSprite = 0x1BE;
        u8 *palettes = gMenuSpritePaletteGroups.kind;
        frame = palettes[kind];
        sprite = (kind - 1) * 3 + (frameIndex + baseSprite);
    }
    func_8006C998(x, y, (s16)sprite, frame, -1, ot, depth);
}

void drawScaledMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, s16 scaleX, s16 scaleY, GsOT *ot, u16 depth) {
    s32 frameIndex = frame;
    s32 sprite;
    if (kind == 0) {
        frame = 5;
        sprite = 0x1BE;
    } else {
        s32 baseSprite = 0x1BE;
        u8 *palettes = gMenuSpritePaletteGroups.kind;
        frame = palettes[kind];
        sprite = (kind - 1) * 3 + (frameIndex + baseSprite);
    }
    if (scaleX > 0 && scaleY > 0) {
        drawTransformedSpriteByIndex((s16)(x + 16), (s16)(y + 16), (s16)sprite, frame, -1,
                     0x7FFF, 0x7FFF, scaleX, scaleY, 0, ot, depth);
    } else {
        func_8006C998(x, y, (s16)sprite, frame, -1, ot, depth);
    }
}

void drawPaletteMappedIcon(s16 x, s16 y, s16 index, struct GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        u8 *palettes = gMenuSpritePaletteGroups.mappedIconPalettes;
        palette = palettes[index];
        sprite = index + 461;
    }
    func_8006C998(x, y, sprite, palette, -1, ot, depth);
}

void drawScaledPaletteMappedIcon(s16 x, s16 y, s16 index, s16 scaleX, s16 scaleY, GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        u8 *palettes = gMenuSpritePaletteGroups.mappedIconPalettes;
        palette = palettes[index];
        sprite = index + 461;
    }
    if (scaleX > 0 && scaleY > 0) {
        drawTransformedSpriteByIndex((s16)(x + 16), (s16)(y + 16), sprite, palette, -1,
                       0x7FFF, 0x7FFF, scaleX, scaleY, 0, ot, depth);
    } else {
        func_8006C998(x, y, sprite, palette, -1, ot, depth);
    }
}

void unpackUpperWordBytes(u32 packed, UnpackedWordBytes *out) {
    u32 byte;

    packed >>= 8;
    byte = packed & 0xFF;
    packed >>= 8;
    out->unk1 = packed;
    packed >>= 8;
    out->unk2 = byte;
    out->unk0 = packed;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8004054C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80040A60);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80040D5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80041554);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80041890);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800418F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80041DC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8004213C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80042558);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80042894);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80042DC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80043120);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800435AC);

void drawFirstPlayerStatusHud(void) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    RacePlayer *player;
    s16 frame;
    s32 palette;
    setSpriteColor(128, 128, 128);
    player = D_80120E90;
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80043A4C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80043C28);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80043C5C);

void resetControllersAndDeleteCurrentTask(void) {
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetControllerEffects();
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80043FBC);

s16 isFirstPlayerUsingCallback26D10(void) {
    return D_80120E90[0].task->callback == updateCurrentPlayerTaskCountdown;
}

void drawPlayerFlagTaskVisual(TaskVisualResource *resource) {
    PlayerCamera *cameras = D_801241E8;
    D_800EB22C.vx = rcos(-cameras[func_80059254()].yaw) * 40 / 4096;
    D_800EB22C.vz = rsin(-cameras[func_80059254()].yaw) * 40 / 4096;
    resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[0] + D_800EB22C.vx;
    resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[1] - 80;
    resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[2] + D_800EB22C.vz;
    setTaskVisualFrame(resource, gPlayerFlagVisualFrames, 0x1800);
    func_8005AC4C(resource, 1);
    func_80059290(resource);
}

void initializeTaskCoordinateResource(void) {
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    GsInitCoordinate2(0, (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource14]);
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 1;
    setCurrentTaskCallback(updatePlayerFlagTaskVisual);
}

void updatePlayerFlagTaskVisual(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_80120E90[D_8013AE88->data18].unk946) {
        if (!D_8013AE88->data1A) {
            resource->unkC6 = 0x600;
            resource->unkC8 = 0x100;
            resource->r0 = resource->g0 = resource->b0 = 0x80;
            resource->code = POLY_FT4_CODE;
            resource->framePosition = 0;
            D_8013AE88->data1A = 1;
            D_8013AE88->data1C = 7;
        }
        drawPlayerFlagTaskVisual(resource);
    } else if (D_8013AE88->data1A) {
        D_8013AE88->data1C--;
        if (D_8013AE88->data1C == 0) {
            D_8013AE88->data1A = 0;
        } else {
            drawPlayerFlagTaskVisual(resource);
        }
    }
}

void initializePlayerSideVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (OrbitingPlayerVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->frameLimit = 0x400;
    resource->frameStep = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->framePosition = 0;
    func_800183FC(D_8013AE88->data18, 1, 0x600, 0x20);
    func_8002E5A0(D_8013AE88->data18, 12);
    if (D_80120E90[D_8013AE88->data18].unk128 & 0x400) {
        resource->radius = -52;
    } else {
        resource->radius = 52;
    }
    setCurrentTaskCallback(updatePlayerSideVisualTask);
}

void updatePlayerSideVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (OrbitingPlayerVisualResource *)&D_800D9504[D_8013AE88->resource14];
    D_800EB22C.vx = rsin((s16)D_80120E90[D_8013AE88->data18].unk110) * resource->radius / 4096;
    D_800EB22C.vz = rcos((s16)D_80120E90[D_8013AE88->data18].unk110) * resource->radius / 4096;
    resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[0] + D_800EB22C.vx;
    resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[2] + D_800EB22C.vz;
    setTaskVisualFrame((TaskVisualResource *)resource, gPlayerSideVisualFrames, 0x2000);
    func_8005AC4C((TaskVisualResource *)resource, 1);
    if (!D_8013AFD3) {
        if (!func_80059290((TaskVisualResource *)resource)) {
            func_800149A8();
        }
    }
}

void updateOneShotTaskVisual(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    setTaskVisualFrame(resource, D_80124304, 0x2000);
    func_8005AC4C(resource, 1);
    if (!D_8013AFD3) {
        if (!func_80059290(resource)) {
            func_800149A8();
        }
    }
}

void initializeExpandingCoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    ExpandingCoordinateEffectResource *initial;
    s32 x, y, z;
    s16 index;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    initial = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    index = initial->rotation.vz;
    x = initial->coordinate.coord.t[0];
    y = initial->coordinate.coord.t[1];
    z = initial->coordinate.coord.t[2];
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->parent);
    GsInitCoordinate2(&resource->parent, &resource->coordinate);
    resource->position.x = x << 8;
    resource->position.y = y << 8;
    resource->position.z = z << 8;
    resource->selectedIndex = index;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->parent.coord.t[0] = x;
    resource->parent.coord.t[1] = y;
    resource->parent.coord.t[2] = z;
    playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0x101, 0x20);
    func_8004A764(index, x, z);
    computeSelectedTriangleAngles(&resource->pitch, &resource->yaw);
    resource->unkCA = 0;
    D_8013AE88->data1A = 45;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    setCurrentTaskCallback(func_80044BC0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80044BC0);

void resetTaskVisualState(TaskVisualResource *resource) {
    resource->unkC6 = 0x400;
    resource->unkC8 = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->framePosition = 0;
    playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z,
                  1, 0x700, 0x20);
    setCurrentTaskCallback(updateOneShotTaskVisual);
}

void createNineIndexedChildTasks(u16 value, u16 mode) {
    u16 i;
    for (i = 0; i < 9; i++) {
        func_800148B4(func_8004531C);
        D_8013AE8C->data18 = D_8013AE88->data18;
        D_8013AE8C->data1A = i;
        D_8013AE8C->data1C = value;
        D_8013AE8C->data1E = mode;
    }
}

void initializePlayerState23CoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->parent);
    GsInitCoordinate2(&resource->parent, &resource->coordinate);
    players = D_80120E90;
    resource->selectedIndex = players[D_8013AE88->data18].unk92E;
    resource->rotation.vx = 0;
    resource->rotation.vy = players[D_8013AE88->data18].unk110;
    resource->rotation.vz = 0;
    resource->position.x = players[D_8013AE88->data18].x;
    resource->position.y = players[D_8013AE88->data18].y;
    resource->position.z = players[D_8013AE88->data18].z;
    func_8004A764(resource->selectedIndex, resource->position.x >> 8, resource->position.z >> 8);
    computeSelectedTriangleAngles(&resource->pitch, &resource->yaw);
    resource->unkCA = 0;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    players[D_8013AE88->data18].unk024 = 0;
    setCurrentTaskCallback(func_800450BC);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_800450BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_8004531C);

void updateFallingRotatingObjectTask(void) {
    FallingObjectResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (FallingObjectResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (D_8013AE88->data1A & 1) {
            resource->rotation.vx += 0x100;
            resource->rotation.vy += 0x100;
            resource->rotation.vz += 0x100;
        } else {
            resource->rotation.vx -= 0x100;
            resource->rotation.vy -= 0x100;
            resource->rotation.vz -= 0x100;
        }
        resource->coordinate.coord.t[0] += resource->velocity.x;
        resource->coordinate.coord.t[1] += resource->velocity.y;
        resource->coordinate.coord.t[2] += resource->velocity.z;
        resource->velocity.y += 6;
        if (resource->coordinate.coord.t[1] > resource->heightLimit) {
            func_800149A8();
            return;
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->parentRotation, &resource->parent);
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
}

void initializeScaledTaskCoordinateResource(void) {
    ScaledCoordinateResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ScaledCoordinateResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0xC00;
    setCurrentTaskCallback(updatePlayerScaledObjectTask);
}

void updatePlayerScaledObjectTask(void) {
    PlayerEffectResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    players = D_80120E90;
    resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1];
    resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2];
    resource->rotation.vx = players[D_8013AE88->data18].unk0B4;
    resource->rotation.vy = players[D_8013AE88->data18].unk0B6;
    resource->rotation.vz = players[D_8013AE88->data18].unk0B8;
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[10].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(10);
    }
    if (players[D_8013AE88->data18].unk12C != 25) {
        func_800183FC(D_8013AE88->data18, 1, 0xF00, 0x20);
        createNineIndexedChildTasks(0x15, 0);
        D_8013AE88->callback = deleteCurrentTask;
        D_8013AE88->unk10 |= 1;
    }
}

void initializePlayerParticleHistoryTask(void) {
    TaskVisualResource *resource;
    PlayerParticleResource *trail;
    u16 i;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    trail = (PlayerParticleResource *)&D_800D9504[D_8013AE88->resource16];
    GsInitCoordinate2(0, &resource->coordinate);
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 0;
    D_8013AE88->data1E = 0;
    for (i = 0; i < 16; i++) {
        trail->entries[i].framePosition = 0xFFFF;
    }
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->code = POLY_FT4_CODE;
    setCurrentTaskCallback(func_80045C7C);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80045C7C);

void initializePlayerCoordinateEffectTask(void) {
    PlayerEffectResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    D_8013AE88->data1A = 0;
    resource->rotation.vx = resource->rotation.vy = resource->rotation.vz = 0;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    func_800183FC(D_8013AE88->data18, 1, 0x800, 0x20);
    setCurrentTaskCallback(func_80046044);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80046044);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80046228);

void initializePlayerVisualHistory(TaskVisualResource *resource, VECTOR *vectors, s32 value, s32 scale) {
    RacePlayer *players;
    RacePlayer *player;
    VECTOR *entry;
    u16 i;
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    if (D_80120E90[D_8013AE88->data18].unk128 & 0x400) {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110 + 0x800;
    } else {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110;
    }
    players = D_80120E90;
    resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2];
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(players[D_8013AE88->data18].unk92E, players[D_8013AE88->data18].x >> 8, players[D_8013AE88->data18].z >> 8);
    resource->unkDC = value;
    resource->groundY = scale;
    player = &players[D_8013AE88->data18];
    if (player->unk0BC != 0) {
        resource->velocityY = (player->unk062 * ((scale << 8) / player->unk0BC)) / 256;
    } else {
        resource->velocityY = player->unk062;
    }
    for (i = 0; i < 4; i++) {
        entry = vectors;
        entry += i;
        entry->vx = entry->vy = entry->vz = 0;
        entry->pad = 0;
    }
    D_8013AE88->data1A = 0;
    func_8001496C(&gGameTasks[4], initializePlayerSideVisualTask);
    D_8013AE8C->data18 = D_8013AE88->data18;
}

void drawTaskVisualPositionHistory(TaskVisualResource *resource, VECTOR *history, TaskVisualFrame *frames) {
    u16 i;
    s32 phase;
    VECTOR *entry;
    resource->code = POLY_FT4_CODE;
    resource->scaleX = 0x1A00;
    resource->scaleY = 0x1A00;
    resource->scaleZ = 0x1A00;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    prepareTaskVisualQuad(resource, frames, 1);
    func_8005A788(resource, frames, 0);
    resource->code = POLY_FT4_CODE | 2;
    for (i = 0; i < 4; i++) {
        entry = history;
        entry += i;
        if (entry->pad != 0) {
            resource->coordinate.coord.t[0] = entry->vx >> 8;
            resource->coordinate.coord.t[1] = entry->vy >> 8;
            resource->coordinate.coord.t[2] = entry->vz >> 8;
            phase = (D_8013AE88->data1A - i) & 3;
            resource->r0 = resource->g0 = resource->b0 = 0x80 - (phase << 3);
            resource->scaleX = 0x1A00 + (phase << 9);
            resource->scaleY = 0x1A00 + (phase << 9);
            resource->scaleZ = 0x1A00 + (phase << 9);
            prepareTaskVisualQuad(resource, frames, 1);
            func_8005A788(resource, frames, 0);
        }
    }
    if (D_8013AFD3 == 0) {
        D_8013AE88->data1A = (D_8013AE88->data1A + 1) & 3;
    }
}

void updateHomingTaskVisualMovement(TaskVisualResource *resource, VECTOR *history, s32 distance, s32 maxTurn) {
    VECTOR *entry;
    s16 angle;
    s32 player;
    s32 oldY;
    s32 ground;
    entry = history;
    entry += D_8013AE88->data1A;
    entry->vx = resource->position.x;
    entry = history;
    entry += D_8013AE88->data1A;
    entry->vy = resource->position.y;
    entry = history;
    entry += D_8013AE88->data1A;
    entry->vz = resource->position.z;
    history += D_8013AE88->data1A;
    history->pad = 1;
    player = func_80046228(&resource->position, distance, resource->unkD8, D_8013AE88->data18, &angle);
    resource->unkE0 = player;
    if (player != -1) {
        D_80120E90[player].unk946 = 1;
        angle = (angle - resource->unkD8) & 0xFFF;
        if (angle > 0x800) angle -= 0x1000;
        if (angle > (s16)maxTurn) angle = maxTurn;
        if (angle < -(s16)maxTurn) angle = -maxTurn;
        resource->unkD8 += angle;
    }
    D_800EB22C.vx = rsin(resource->unkD8) * resource->groundY / 4096;
    D_800EB22C.vz = rcos(resource->unkD8) * resource->groundY / 4096;
    oldY = resource->position.y;
    resource->position.x += D_800EB22C.vx;
    resource->position.y += resource->velocityY;
    resource->position.z += D_800EB22C.vz;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
    ground = func_8004A764(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    if (resource->position.y > ground) resource->position.y = ground;
    resource->velocityY = resource->position.y - oldY + 0x1900;
}

void checkTaskVisualOtherPlayerCollision(TaskVisualResource *resource, s16 flags) {
    u16 i;
    RacePlayer *player;
    for (i = 0; i < D_8013AFD2; i++) {
        if (i != D_8013AE88->data18) {
            player = &D_80120E90[i];
            if (player->unk946 != 0) {
                if (addPlayerFlagsIfWithinRadius(&resource->position, 0xF00, flags, i) != 0) {
                    player->unk0F4 = D_8013AE88->data18;
                    resource->unkDC = 0;
                    break;
                }
            }
        }
    }
}

void initializePlayerDualResourceEffectTask(void) {
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5580);
    setCurrentTaskCallback(updatePlayerDualResourceEffectTask);
}

void updatePlayerDualResourceEffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x63000, 0x1C);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerDualResourceEffectFrame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 8);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializePlayerVisualPairTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory(resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5580);
    setCurrentTaskCallback(updatePlayerVisualPairTask);
}

void updatePlayerVisualPairTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x51000, 0x12);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPairFrame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x100);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializePlayerVisualPairTask5E80(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory(resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5E80);
    setCurrentTaskCallback(updatePlayerVisualPairTask5E80);
}

void updatePlayerVisualPairTask5E80(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x3F000, 0x18);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPair5E80Frame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x10);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializePlayerVisualPairTask6780(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory(resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 180, 0x6780);
    setCurrentTaskCallback(updatePlayerVisualPairTask6780);
}

void updatePlayerVisualPairTask6780(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x1B000, 0x1C);
        func_8004BC3C(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz, &D_800EB22C.vx, &D_800EB22C.vz);
        if (dx != 0 || dz != 0) {
            resource->position.x += dx;
            resource->position.z += dz;
            resource->unkD8 = func_8004A124(0, 0, D_800EB22C.vx, D_800EB22C.vz);
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPair6780Frame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x40);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializeDualResource4C80EffectTask(void) {
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x4C80);
    setCurrentTaskCallback(updateDualResource4C80EffectTask);
}

void updateDualResource4C80EffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x2D000, 0x10);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, D_80124354);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x80);
        if (resource->unkDC == 0) {
            initializeExpandingCoordinateEffectTask();
        } else {
            resource->unkDC--;
        }
    }
}

void initializeBouncingTaskVisual(void) {
    TaskVisualResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x200;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    if (D_80120E90[D_8013AE88->data18].unk128 & 0x400) {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110;
    } else {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110 + 0x800;
    }
    D_800EB22C.vx = rsin(resource->unkD8) / 64;
    D_800EB22C.vz = rcos(resource->unkD8) / 64;
    players = D_80120E90;
    resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0] + D_800EB22C.vx;
    resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2] + D_800EB22C.vz;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(players[D_8013AE88->data18].unk92E, players[D_8013AE88->data18].x >> 8, players[D_8013AE88->data18].z >> 8);
    resource->velocityY = -0xA00;
    resource->groundY = func_8004A764(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    resource->bounceCount = 0;
    func_800183FC(D_8013AE88->data18, 1, 2, 0x20);
    setCurrentTaskCallback(updateBouncingTaskVisual);
}

void updateBouncingTaskVisual(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->velocityY += 0x180;
        resource->position.y += resource->velocityY;
        if (resource->velocityY > 0 && resource->position.y > resource->groundY) {
            if (resource->bounceCount == 0) {
                resource->velocityY = -0xA00;
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xB01, 12);
            } else {
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xB01, 8);
            }
            resource->position.y = resource->groundY;
            resource->bounceCount++;
        }
        resource->coordinate.coord.t[0] = resource->position.x >> 8;
        resource->coordinate.coord.t[1] = resource->position.y >> 8;
        resource->coordinate.coord.t[2] = resource->position.z >> 8;
    }
    setTaskVisualFrame(resource, D_80124354, 0x1A00);
    func_8005AC4C(resource, 1);
    if (!D_8013AFD3 && resource->bounceCount == 2) {
        resource->unkDC = 30;
        setCurrentTaskCallback(updateAcceleratingTaskVisualCountdown);
    }
}

void updateAcceleratingTaskVisualCountdown(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    setTaskVisualFrame(resource, D_80124354, 0x1A00);
    func_8005AC4C(resource, 1);
    if (!D_8013AFD3) {
        if (resource->unkDC >= 5) {
            resource->unkC8 += 8;
            if (resource->unkC8 > 0x80) {
                resource->unkC8 = 0x80;
            }
            resource->framePosition += resource->unkC8;
            if (resource->framePosition >= resource->unkC6) {
                resource->framePosition &= 0xFF;
            }
        } else {
            resource->framePosition = 0x100;
        }
        if (--resource->unkDC == 0) {
            initializeExpandingCoordinateEffectTask();
        }
    }
}

void initializeDualResource5100EffectTask(void) {
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5100);
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(func_80047D94);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80047D94);

void initializeVisualTrailTask(void) {
    TrailVisualResource *resource;
    VECTOR *vectors;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (TrailVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    vectors = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    D_800EB22C.vx = rsin(resource->angle) / 64;
    D_800EB22C.vz = rcos(resource->angle) / 64;
    resource->lifetime = 60;
    resource->scale = 0x6780;
    resource->unkD4 = 0;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    for (i = 0; i < 4; i++) {
        VECTOR *vector = &vectors[i];
        vector->vx = vector->vy = vector->vz = 0;
        vector->pad = 0;
    }
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(func_80048340);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80048340);

void initPlayerTimerTask(void) {
    PlayerTimerState *timer = &D_80120E90[D_8013AE88->data18].timer;

    GsInitCoordinate2(0, &timer->coordinate);
    timer->unk60 = 0;
    D_80120E90[D_8013AE88->data18].unk0FE = 1;
    setCurrentTaskCallback(updatePlayerTimerTask);
}

void updatePlayerTimerTask(void) {
    if (!D_8013AFD3) {
        if (D_80120E90[D_8013AE88->data18].unk128 & 0x3040) {
            D_80120E90[D_8013AE88->data18].unk0FC = 0;
        }
        if (D_80120E90[D_8013AE88->data18].unk0FC != 0) {
            D_80120E90[D_8013AE88->data18].unk0FC--;
        }
        if (D_80120E90[D_8013AE88->data18].unk0FC == 0) {
            if (!D_80120E90[D_8013AE88->data18].unk008 &&
                gControllerEffectStates[D_8013AE88->data18].effect == 13 &&
                !(D_80120E90[D_8013AE88->data18].unk128 & 0x2000)) {
                D_8013AF08[D_8013AE88->data18] = 0;
            }
            setCurrentTaskCallback(fadeOutPlayerTimerTask);
        } else if (D_80120E90[D_8013AE88->data18].timer.unk60 < 0x1000) {
            D_80120E90[D_8013AE88->data18].timer.unk60 += 0x100;
        }
    }
}

void fadeOutPlayerTimerTask(void) {
    if (!D_8013AFD3) {
        if (D_80120E90[D_8013AE88->data18].unk0FC == 0) {
            D_80120E90[D_8013AE88->data18].timer.unk60 -= 0x200;
            if (D_80120E90[D_8013AE88->data18].timer.unk60 > 0) {
                return;
            }
            D_80120E90[D_8013AE88->data18].timer.unk60 = 0;
            D_80120E90[D_8013AE88->data18].unk0FE = 0;
        }
        func_800149A8();
    }
}

void initializeOrbitingPlayerVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (OrbitingPlayerVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->frameLimit = 0x400;
    resource->frameStep = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE | 2;
    resource->offset.y = -32;
    resource->radius = 8;
    resource->r0 = resource->g0 = resource->b0 = 0x40;
    resource->framePosition = 0;
    resource->scale = 0x1800;
    resource->angle = 0;
    func_800183FC(D_8013AE88->data18, 1, 0xE00, 0x20);
    resource->countdown = 0x10;
    D_8013AE88->data1C = 0;
    setCurrentTaskCallback(func_80048AA4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80048AA4);

void initializeDelayedPlayerRiseEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (D_8013AE88->data1C == 0) {
        if (D_8013AE88->resource14 != 0xFFFF) {
            while (1) {}
        }
        D_8013AE88->resource14 = allocateTaskResource();
        resource = (RisingPlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->offset.x = 0;
        resource->offset.y = -64;
        resource->offset.z = 0;
        resource->velocity = 0;
        resource->rotation.vx = resource->rotation.vy = resource->rotation.vz = 0;
        resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
        playSoundEffect(1, 0xA00, 0x30);
        setCurrentTaskCallback(updateRisingPlayerEffectTask);
    } else {
        D_8013AE88->data1C--;
    }
}

void updateRisingPlayerEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RisingPlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->velocity -= 2;
        resource->offset.y += resource->velocity;
        if (resource->offset.y < -320) {
            resource->velocity = 2;
            resource->unkD8 = 1;
            D_8013AE88->data1C = 0;
            setCurrentTaskCallback(func_80049404);
            return;
        }
        resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data1A].position8E4[0];
        resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data1A].position8E4[1] + resource->offset.y;
        resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data1A].position8E4[2];
        if (resource->scale.vx < 0x1000) {
            resource->scale.vx += 0x100;
            resource->scale.vy += 0x100;
            resource->scale.vz += 0x100;
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[12].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(12);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4_tail", func_80049404);

void initializeSlidingVisualSequenceTask(void) {
    SlidingVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (SlidingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 128;
    resource->code = POLY_FT4_CODE;
    resource->step = 0;
    resource->timer = 0;
    resource->framePosition = 0;
    resource->unk74 = -352;
    resource->velocity = 0;
    playSoundEffect(1, 0xD01, 0x20);
    D_8013AE88->data1C = 0;
    setCurrentTaskCallback(func_800497B8);
}
