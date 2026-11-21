#define GAME_OPTIONS_GP_ADDRESSING
#include "selection_tasks_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/save/save_data.h"
#include "game/scene/scene_events.h"
#include "unlock_menu_types.h"

extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));

s16 gUnlockNotificationSprites[14] __attribute__((section(".data.unlock_notification_sprites"))) = {
    0x216, 0x217, 0x218, 0x219, 0x21A, 0x21B,
    0x21D, 0x21E, 0x21F, 0x220, 0x221, 0x222,
    0x21C, 0
};

void initializeUnlockMenuResourceTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuAnimation(state);
    state->unk8C = 0;
    configureTaskStateTransition(1, 0, 30);
    applySaveRecordUnlockOverrides(0);
    setCurrentTaskCallback(updateUnlockOptionsMenuTask);
}

void resetUnlockMenuAnimation(UnlockMenuState *state) {
    s16 i;
    state->unk04[0] = state->unk20[0] = 0;
    for (i = 1; i < 7; i++) {
        state->unk04[i] = state->unk3C[i] = state->unk04[0];
        state->unk20[i] = state->unk58[i] = state->unk20[0];
    }
    state->unk3C[0] = state->unk04[0];
    state->unk58[0] = state->unk20[0];
    state->unk74[0] = 0;
    state->unk74[1] = 1;
    state->unk74[2] = 2;
    state->unk74[3] = 3;
}

void runTaskResource14Update(void) {
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {
        }
    }
    func_800A42CC();
}

void updateUnlockOptionsMenuTask(void) {
    UnlockMenuState *state;
    s16 exitRequested;
    s16 optionChanged;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    exitRequested = 0;
    optionChanged = 0;
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];

    if (D_8013AEE8[0] & 0x1000) {
        if (state->selection > 0) {
            state->selection--;
        }
    }
    if (D_8013AEE8[0] & 0x4000) {
        if (state->selection < 6) {
            state->selection++;
        }
    }
    if (D_8013AEE8[0] & 0x40) {
        exitRequested = 1;
    }
    if (state->selection != state->previousSelection) {
        playSoundEffect(0, 0x100, 0x20);
        state->previousSelection = state->selection;
    }

    switch (state->selection) {
    case 0:
        if ((D_8013AEE8[0] & 0x8000) && !(gGameOptionFlags & 0x1)) {
            gGameOptionFlags |= 0x1;
            optionChanged = 1;
        }
        if ((D_8013AEE8[0] & 0x2000) && (gGameOptionFlags & 0x1)) {
            gGameOptionFlags &= ~0x1;
            optionChanged = 1;
        }
        if (D_8013AEE8[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x1) ?
                gGameOptionFlags | 0x1 : gGameOptionFlags & ~0x1;
            optionChanged = 1;
        }
        break;
    case 1:
        if ((D_8013AEE8[0] & 0x8000) && !(gGameOptionFlags & 0x2)) {
            gGameOptionFlags |= 0x2;
            optionChanged = 1;
        }
        if ((D_8013AEE8[0] & 0x2000) && (gGameOptionFlags & 0x2)) {
            gGameOptionFlags &= ~0x2;
            optionChanged = 1;
        }
        if (D_8013AEE8[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x2) ?
                gGameOptionFlags | 0x2 : gGameOptionFlags & ~0x2;
            optionChanged = 1;
        }
        break;
    case 2:
        if ((D_8013AEE8[0] & 0x8000) && !(gGameOptionFlags & 0x4)) {
            gGameOptionFlags |= 0x4;
            optionChanged = 1;
        }
        if ((D_8013AEE8[0] & 0x2000) && (gGameOptionFlags & 0x4)) {
            gGameOptionFlags &= ~0x4;
            optionChanged = 1;
        }
        if (D_8013AEE8[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x4) ?
                gGameOptionFlags | 0x4 : gGameOptionFlags & ~0x4;
            optionChanged = 1;
        }
        break;
    case 3:
        if ((D_8013AEE8[0] & 0x8000) && !(gGameOptionFlags & 0x10)) {
            gGameOptionFlags |= 0x30;
            optionChanged = 1;
        }
        if ((D_8013AEE8[0] & 0x2000) && (gGameOptionFlags & 0x10)) {
            gGameOptionFlags &= ~0x30;
            optionChanged = 1;
        }
        if (D_8013AEE8[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x10) ?
                gGameOptionFlags | 0x30 : gGameOptionFlags & ~0x30;
            optionChanged = 1;
        }
        break;
    case 4:
        if ((D_8013AEE8[0] & 0x8000) && !(gGameOptionFlags & 0x8)) {
            gGameOptionFlags |= 0x8;
            optionChanged = 1;
        }
        if ((D_8013AEE8[0] & 0x2000) && (gGameOptionFlags & 0x8)) {
            gGameOptionFlags &= ~0x8;
            optionChanged = 1;
        }
        if (D_8013AEE8[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x8) ?
                gGameOptionFlags | 0x8 : gGameOptionFlags & ~0x8;
            optionChanged = 1;
        }
        break;
    case 5:
        if (D_8013AEE8[0] & 0xB0) {
            setCurrentTaskCallback(initializeUnlockMenuTask);
            optionChanged = 1;
        }
        break;
    case 6:
        if (D_8013AEE8[0] & 0xB0) {
            exitRequested = 1;
        }
        break;
    }

    func_800A42CC();
    if (exitRequested) {
        playSoundEffect(0, 0, 0x20);
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishUnlockMenuTask);
    } else if (optionChanged) {
        playSoundEffect(0, 0, 0x20);
        if (state->selection == 4) {
            applyStereoSoundOption();
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A42CC);

void finishUnlockMenuTask(void) {
    UnlockMenuState *state;
    s16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    func_800A42CC();
    switch (D_8013AE88->data18) {
    case 0:
        for (i = 0; i < 5; i++) {
            state->unk58[i] = 0;
            state->unk20[i] = 0;
        }
        D_8013AE88->data18 = 1;
        break;
    case 1:
        for (i = 0; i < 5; i++) {
            state->unk20[i] += 0x4000;
            state->unk04[i] += state->unk20[i];
            state->unk58[i] += 0x4000;
            state->unk3C[i] += state->unk58[i];
        }
        if (state->unk04[0] > 0x168000) {
            if ((u16)state->unk8C != 0) {
                setSecondaryDisplayTaskMode(3);
                queueMode11TransitionResources();
                queueBackgroundTextureSectorCommands();
                D_8013B10C = -1;
                queueMenuResourceReads(2);
            }
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if (isCdCommandQueueIdle() == 1) {
            configureTaskStateTransition(2, 0, 10);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (isTaskStateTransitionBelowMaximum(0) == 0) { deleteCurrentTaskSubtree(); }
        break;
    }
}

void initializeUnlockMenuTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuOffsets(state);
    drawUnlockNotificationMenu();
    setCurrentTaskCallback(func_800A499C);
}

void resetUnlockMenuOffsets(UnlockMenuState *state) {
    s16 i;
    for (i = 0; i < 6; i++) {
        i[state->unk3C] = 0;
        i[state->unk04] = 0;
        i[state->unk58] = 0;
        i[state->unk20] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A499C);

void drawUnlockNotificationMenu(void) {
    UnlockMenuState *state;
    s16 row;
    s16 verticalOffset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    setSpriteColor(128, 128, 128);
    verticalOffset = state->unk3C[0] / 4096;
    func_8006C998(-68, -96 - verticalOffset, 0x22F, 2, -1, &D_800EBFF0[D_8013AF78], 4);
    for (row = 0; row < 6; row++) {
        s16 sprite;
        s16 rightIndex;
        s16 horizontalOffset = state->unk04[row] / 4096;

        if ((s16)hasUnlockNotificationFlag(row)) {
            sprite = gUnlockNotificationSprites[row];
        } else {
            sprite = 0x233;
        }
        func_8006C998(-136 - horizontalOffset, row * 20 - 56, sprite,
                      state->selection != row, -1, &D_800EBFF0[D_8013AF78], 4);
        rightIndex = row + 6;
        if ((s16)hasUnlockNotificationFlag(rightIndex)) {
            sprite = gUnlockNotificationSprites[rightIndex];
        } else {
            sprite = 0x233;
        }
        func_8006C998(horizontalOffset + 8, row * 20 - 56, sprite,
                      state->selection != rightIndex, -1, &D_800EBFF0[D_8013AF78], 4);
    }
    func_8006C998(-64, verticalOffset + 72, 0x21C, state->selection != 12, -1,
                  &D_800EBFF0[D_8013AF78], 4);
    func_8007240C(-160, -120, 320, 240, 192, 192, 192, 2, &D_800EBFF0[D_8013AF78], 4);
    drawFullScreenTexturedBackground();
}

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

void startUnlockActionAfterCountdown(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013AF3A < 11) {
            createChildTaskWithReserve(initializeScenePlaybackResourcesTask);
        } else {
            createChildTaskWithReserve(startUnlockResourceLoad);
        }
        setCurrentTaskCallback(prepareUnlockSelectionResume);
        D_8013AE88->unk10 |= 1;
    }
}

void animateUnlockMenuTransition(void) {
    UnlockMenuState *state;
    s16 i;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    drawUnlockNotificationMenu();
    switch (D_8013AE88->data18) {
    case 0:
        for (i = 0; i < 6; i++) {
            state->unk58[i] = 0;
            state->unk20[i] = 0;
        }
        D_8013AE88->data18 = 1;
        break;
    case 1:
        for (i = 0; i < 6; i++) {
            state->unk20[i] += 0x4000;
            state->unk04[i] += state->unk20[i];
            state->unk58[i] += 0x4000;
            state->unk3C[i] += state->unk58[i];
        }
        if (state->unk04[0] > 0x168000) {
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        state->selection = 5;
        state->previousSelection = 5;
        resetUnlockMenuAnimation(state);
        setCurrentTaskCallback(updateUnlockOptionsMenuTask);
        break;
    }
}

void resumeUnlockSelectionTask(void) {
    setSecondaryDisplayTaskMode(0);
    setCurrentTaskCallback(func_800A499C);
}

void prepareUnlockSelectionResume(void) {
    setSecondaryDisplayTaskMode(3);
    D_8013B10C = -1;
    queueMenuResourceReads(2);
    setCurrentTaskCallback(resumeUnlockSelectionTask);
    D_8013AE88->unk10 |= 2;
}

void finishUnlockPresentationTask(void) {
    if (D_8013AFE4 & 0x100) {
        deleteCurrentTaskSubtree();
    }
}

void startUnlockResourceLoad(void) {
    resetPendingCdRequests();
    D_8013AFE4 &= ~0x100;
    createResourceLoadTask();
    setCurrentTaskCallback(finishUnlockPresentationTask);
}
