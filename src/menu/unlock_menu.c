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

extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));

const u16 gUnlockOptionLabelSprites[8] __attribute__((section(".data.unlock_option_labels"))) = {
    0x225, 0x226, 0x227, 0x234, 0x228, 0x229, 0x22A, 0
};

const u16 gUnlockOptionChoiceSprites[10] __attribute__((section(".data.unlock_option_choices"))) = {
    0x223, 0x224,
    0x22B, 0x22C,
    0x22B, 0x22C,
    0x22B, 0x22C,
    0x22D, 0x22E
};

s16 gUnlockNotificationSprites[14] __attribute__((section(".data.unlock_notification_sprites"))) = {
    0x216, 0x217, 0x218, 0x219, 0x21A, 0x21B,
    0x21D, 0x21E, 0x21F, 0x220, 0x221, 0x222,
    0x21C, 0
};

const u8 gUnlockOptionBitIndices[6] __attribute__((section(".data.unlock_option_bits"))) = {
    0, 1, 2, 4, 3, 0
};

void initializeUnlockMenuResourceTask(void) {
    UnlockMenuState *state;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {
        }
    }
    drawUnlockOptionsMenu();
}

void updateUnlockOptionsMenuTask(void) {
    UnlockMenuState *state;
    s16 exitRequested;
    s16 optionChanged;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    exitRequested = 0;
    optionChanged = 0;
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];

    if (gControllerPressedButtons[0] & 0x1000) {
        if (state->selection > 0) {
            state->selection--;
        }
    }
    if (gControllerPressedButtons[0] & 0x4000) {
        if (state->selection < 6) {
            state->selection++;
        }
    }
    if (gControllerPressedButtons[0] & 0x40) {
        exitRequested = 1;
    }
    if (state->selection != state->previousSelection) {
        playSoundEffect(0, 0x100, 0x20);
        state->previousSelection = state->selection;
    }

    switch (state->selection) {
    case 0:
        if ((gControllerPressedButtons[0] & 0x8000) && !(gGameOptionFlags & 0x1)) {
            gGameOptionFlags |= 0x1;
            optionChanged = 1;
        }
        if ((gControllerPressedButtons[0] & 0x2000) && (gGameOptionFlags & 0x1)) {
            gGameOptionFlags &= ~0x1;
            optionChanged = 1;
        }
        if (gControllerPressedButtons[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x1) ?
                gGameOptionFlags | 0x1 : gGameOptionFlags & ~0x1;
            optionChanged = 1;
        }
        break;
    case 1:
        if ((gControllerPressedButtons[0] & 0x8000) && !(gGameOptionFlags & 0x2)) {
            gGameOptionFlags |= 0x2;
            optionChanged = 1;
        }
        if ((gControllerPressedButtons[0] & 0x2000) && (gGameOptionFlags & 0x2)) {
            gGameOptionFlags &= ~0x2;
            optionChanged = 1;
        }
        if (gControllerPressedButtons[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x2) ?
                gGameOptionFlags | 0x2 : gGameOptionFlags & ~0x2;
            optionChanged = 1;
        }
        break;
    case 2:
        if ((gControllerPressedButtons[0] & 0x8000) && !(gGameOptionFlags & 0x4)) {
            gGameOptionFlags |= 0x4;
            optionChanged = 1;
        }
        if ((gControllerPressedButtons[0] & 0x2000) && (gGameOptionFlags & 0x4)) {
            gGameOptionFlags &= ~0x4;
            optionChanged = 1;
        }
        if (gControllerPressedButtons[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x4) ?
                gGameOptionFlags | 0x4 : gGameOptionFlags & ~0x4;
            optionChanged = 1;
        }
        break;
    case 3:
        if ((gControllerPressedButtons[0] & 0x8000) && !(gGameOptionFlags & 0x10)) {
            gGameOptionFlags |= 0x30;
            optionChanged = 1;
        }
        if ((gControllerPressedButtons[0] & 0x2000) && (gGameOptionFlags & 0x10)) {
            gGameOptionFlags &= ~0x30;
            optionChanged = 1;
        }
        if (gControllerPressedButtons[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x10) ?
                gGameOptionFlags | 0x30 : gGameOptionFlags & ~0x30;
            optionChanged = 1;
        }
        break;
    case 4:
        if ((gControllerPressedButtons[0] & 0x8000) && !(gGameOptionFlags & 0x8)) {
            gGameOptionFlags |= 0x8;
            optionChanged = 1;
        }
        if ((gControllerPressedButtons[0] & 0x2000) && (gGameOptionFlags & 0x8)) {
            gGameOptionFlags &= ~0x8;
            optionChanged = 1;
        }
        if (gControllerPressedButtons[0] & 0xB0) {
            gGameOptionFlags = !(gGameOptionFlags & 0x8) ?
                gGameOptionFlags | 0x8 : gGameOptionFlags & ~0x8;
            optionChanged = 1;
        }
        break;
    case 5:
        if (gControllerPressedButtons[0] & 0xB0) {
            setCurrentTaskCallback(initializeUnlockMenuTask);
            optionChanged = 1;
        }
        break;
    case 6:
        if (gControllerPressedButtons[0] & 0xB0) {
            exitRequested = 1;
        }
        break;
    }

    drawUnlockOptionsMenu();
    if (exitRequested) {
        playSoundEffect(0, 0, 0x20);
        gCurrentTask->data18 = 0;
        setCurrentTaskCallback(finishUnlockMenuTask);
    } else if (optionChanged) {
        playSoundEffect(0, 0, 0x20);
        if (state->selection == 4) {
            applyStereoSoundOption();
        }
    }
}

void drawUnlockOptionsMenu(void) {
    UnlockMenuState *state;
    s16 row;
    s16 offset;
    s16 palette;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {
        }
    }
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    setSpriteColor(128, 128, 128);
    offset = state->unk3C[0] / 4096;
    drawSpriteByIndexWithBlend(-56, -96 - offset, 0x230, 2, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);

    for (row = 0; row < 5; row++) {
        offset = state->unk04[row] / 4096;
        palette = state->selection == row ? 3 : 1;
        drawSpriteByIndexWithBlend(-128 - offset, row * 24 - 56, gUnlockOptionLabelSprites[row], palette,
                                   -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        if (gGameOptionFlags & (1U << gUnlockOptionBitIndices[row])) {
            palette = state->selection == row ? 0 : 4;
        } else {
            palette = 1;
        }
        drawSpriteByIndexWithBlend(offset + 32, row * 24 - 56, gUnlockOptionChoiceSprites[row * 2], palette,
                                   -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        if (palette == 1) {
            palette = state->selection == row ? 0 : 4;
        } else {
            palette = 1;
        }
        drawSpriteByIndexWithBlend(offset + 92, row * 24 - 56, gUnlockOptionChoiceSprites[row * 2 + 1], palette,
                                   -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }

    for (row = 5; row < 7; row++) {
        offset = state->unk3C[row - 5] / 4096;
        palette = state->selection != row;
        drawSpriteByIndexWithBlend(-56, row * 24 - 56 + offset, gUnlockOptionLabelSprites[row],
                                   palette, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }

    addTilePacketWithBlend(-160, -120, 320, 240, 192, 192, 192, 2, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    drawFullScreenTexturedBackground();
}

void finishUnlockMenuTask(void) {
    UnlockMenuState *state;
    s16 i;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    drawUnlockOptionsMenu();
    switch (gCurrentTask->data18) {
    case 0:
        for (i = 0; i < 5; i++) {
            state->unk58[i] = 0;
            state->unk20[i] = 0;
        }
        gCurrentTask->data18 = 1;
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
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        if (isCdCommandQueueIdle() == 1) {
            configureTaskStateTransition(2, 0, 10);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (isTaskStateTransitionBelowMaximum(0) == 0) { deleteCurrentTaskSubtree(); }
        break;
    }
}

void initializeUnlockMenuTask(void) {
    UnlockMenuState *state;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuOffsets(state);
    drawUnlockNotificationMenu();
    setCurrentTaskCallback(updateUnlockNotificationMenuTask);
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

void updateUnlockNotificationMenuTask(void) {
    UnlockMenuState *state;
    u16 resource;
    s16 leave;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    /* The single-pass block preserves instruction ordering for the task-state lookup. */
    do {
        resource = gCurrentTask->resource14;
        state = (UnlockMenuState *)&gTaskMemoryPool[resource];
    } while (0);
    leave = 0;

    if (gControllerPressedButtons[0] & 0x1000) {
        u16 selection = state->selection;
        if ((u16)(selection - 1) < 5 || (u32)(selection - 7) < 6) {
            state->selection = selection - 1;
        }
    }
    if (gControllerPressedButtons[0] & 0x4000) {
        u32 selection = (u16)state->selection;
        if (selection < 5 || (u32)(selection - 6) < 6) {
            state->selection = selection + 1;
        } else if ((s16)selection == 5) {
            state->selection = 12;
        }
    }
    if (gControllerPressedButtons[0] & 0x8000) {
        u16 selection = state->selection;
        if ((u16)(selection - 6) < 6) {
            state->selection = selection - 6;
        }
    }
    if (gControllerPressedButtons[0] & 0x2000) {
        u16 selection = state->selection;
        if (selection < 6) {
            state->selection = selection + 6;
        }
    }
    if (gControllerPressedButtons[0] & 0x40) {
        leave = 1;
    }
    if (state->selection != state->previousSelection) {
        playSoundEffect(0, 0x100, 0x20);
        state->previousSelection = state->selection;
    }
    if (state->selection == 12) {
        if (gControllerPressedButtons[0] & 0xB0) {
            leave = 1;
        }
    } else if ((gControllerPressedButtons[0] & 0xB0) &&
               (s16)hasUnlockNotificationFlag(state->selection)) {
        void (*callback)(void);
        u8 selection;

        playSoundEffect(0, 0, 0x20);
        callback = startUnlockActionAfterCountdown;
        selection = state->selection;
        state->unk8C = 1;
        D_8013AF3A = selection;
        gCurrentTask->data18 = 15;
        setCurrentTaskCallback(callback);
    }

    drawUnlockNotificationMenu();
    if (leave) {
        playSoundEffect(0, 0, 0x20);
        deleteCurrentTaskDescendants();
        setCurrentTaskCallback(animateUnlockMenuTransition);
        gCurrentTask->data18 = 0;
    }
}

void drawUnlockNotificationMenu(void) {
    UnlockMenuState *state;
    s16 row;
    s16 verticalOffset;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    setSpriteColor(128, 128, 128);
    verticalOffset = state->unk3C[0] / 4096;
    drawSpriteByIndexWithBlend(-68, -96 - verticalOffset, 0x22F, 2, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    for (row = 0; row < 6; row++) {
        s16 sprite;
        s16 rightIndex;
        s16 horizontalOffset = state->unk04[row] / 4096;

        if ((s16)hasUnlockNotificationFlag(row)) {
            sprite = gUnlockNotificationSprites[row];
        } else {
            sprite = 0x233;
        }
        drawSpriteByIndexWithBlend(-136 - horizontalOffset, row * 20 - 56, sprite,
                      state->selection != row, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        rightIndex = row + 6;
        if ((s16)hasUnlockNotificationFlag(rightIndex)) {
            sprite = gUnlockNotificationSprites[rightIndex];
        } else {
            sprite = 0x233;
        }
        drawSpriteByIndexWithBlend(horizontalOffset + 8, row * 20 - 56, sprite,
                      state->selection != rightIndex, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }
    drawSpriteByIndexWithBlend(-64, verticalOffset + 72, 0x21C, state->selection != 12, -1,
                  &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    addTilePacketWithBlend(-160, -120, 320, 240, 192, 192, 192, 2, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    drawFullScreenTexturedBackground();
}

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

void startUnlockActionAfterCountdown(void) {
    if (--gCurrentTask->data18 == 0) {
        if (D_8013AF3A < 11) {
            createChildTaskWithReserve(initializeScenePlaybackResourcesTask);
        } else {
            createChildTaskWithReserve(startUnlockResourceLoad);
        }
        setCurrentTaskCallback(prepareUnlockSelectionResume);
        gCurrentTask->unk10 |= 1;
    }
}

void animateUnlockMenuTransition(void) {
    UnlockMenuState *state;
    s16 i;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    drawUnlockNotificationMenu();
    switch (gCurrentTask->data18) {
    case 0:
        for (i = 0; i < 6; i++) {
            state->unk58[i] = 0;
            state->unk20[i] = 0;
        }
        gCurrentTask->data18 = 1;
        break;
    case 1:
        for (i = 0; i < 6; i++) {
            state->unk20[i] += 0x4000;
            state->unk04[i] += state->unk20[i];
            state->unk58[i] += 0x4000;
            state->unk3C[i] += state->unk58[i];
        }
        if (state->unk04[0] > 0x168000) {
            gCurrentTask->data18 = 2;
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
    setCurrentTaskCallback(updateUnlockNotificationMenuTask);
}

void prepareUnlockSelectionResume(void) {
    setSecondaryDisplayTaskMode(3);
    D_8013B10C = -1;
    queueMenuResourceReads(2);
    setCurrentTaskCallback(resumeUnlockSelectionTask);
    gCurrentTask->unk10 |= 2;
}

void finishUnlockPresentationTask(void) {
    if (gTaskTransitionFlags & 0x100) {
        deleteCurrentTaskSubtree();
    }
}

void startUnlockResourceLoad(void) {
    resetPendingCdRequests();
    gTaskTransitionFlags &= ~0x100;
    createResourceLoadTask();
    setCurrentTaskCallback(finishUnlockPresentationTask);
}
