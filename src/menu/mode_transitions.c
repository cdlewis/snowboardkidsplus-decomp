#include "../race/race_presentation_internal.h"
#include "game/menu/selected_sprite.h"

extern TexturedQuadListEntry D_80123850[];
extern TexturedQuadListEntry D_801239D0[];
extern TexturedQuadListEntry D_80123910[];
extern TexturedQuadListEntry D_80123A90[];

TexturedQuadListEntry *gRegisteredTaskQuadLists[4]
    __attribute__((section(".data.registered_task_quad_lists"))) = {
    D_80123850, D_801239D0, D_80123910, D_80123A90
};

const u16 gRegisteredPlayerModePanelText0[28]
    __attribute__((section(".data.registered_player_mode_panel_text"), aligned(4))) = {
    0xC00F, 0x006C, 0x002C, 0x004F, 0x00A8, 0x00DA, 0x00DB, 0x009F,
    0x00BB, 0x0087, 0x006C, 0x0044, 0x0066, 0x002C, 0x00AB, 0x00DC,
    0x00DD, 0x00BF, 0x00DE, 0x00B4, 0x00A5, 0x009A, 0x00CB, 0x009D,
    0x0094, 0x0033, 0x0000, 0x0000,
};

const u16 gRegisteredPlayerModePanelText1[38]
    __attribute__((section(".data.registered_player_mode_panel_text"), aligned(4))) = {
    0xC00E, 0x0080, 0x0056, 0x006B, 0x006C, 0x002C, 0x004F, 0xC00F,
    0x0039, 0xC00E, 0x0052, 0x0044, 0x0063, 0x0043, 0x0052, 0x003F,
    0x004A, 0xC00F, 0x0039, 0xC00E, 0x004F, 0x0049, 0x006B, 0x0074,
    0x002C, 0x0063, 0xC00F, 0x00C1, 0x0087, 0x006C, 0x0044, 0x00CE,
    0x0099, 0x00B1, 0x009F, 0x0033, 0x0000, 0x0000,
};

const u16 gRegisteredPlayerModePanelText2[28]
    __attribute__((section(".data.registered_player_mode_panel_text"), aligned(4))) = {
    0xC00E, 0x0080, 0x0056, 0x006B, 0x006C, 0x002C, 0x004F, 0xC00F,
    0x00A6, 0xC00E, 0x004F, 0x0049, 0x006B, 0x0074, 0x002C, 0x0063,
    0xC00F, 0x00C1, 0x0087, 0x006C, 0x0044, 0x00CE, 0x0099, 0x00B1,
    0x009F, 0x0033, 0x0000, 0x0000,
};

const u16 *gRegisteredPlayerModePanelTexts[3]
    __attribute__((section(".data.registered_player_mode_panel_texts"))) = {
    gRegisteredPlayerModePanelText0, gRegisteredPlayerModePanelText1, gRegisteredPlayerModePanelText2
};

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
        configureTaskStateTransition(1, 0, 30);
        setSecondaryDisplayTaskMode(1);
        queueCdSectorCommand(0, DAT_STARTUP_MODELS_SECTOR, DAT_STARTUP_MODELS_SECTOR_COUNT, DAT_STARTUP_MODELS, 0);
        queueCdSectorCommand(0, DAT_STARTUP_TEXTURES_SECTOR, DAT_STARTUP_TEXTURES_SECTOR_COUNT, DAT_STARTUP_TEXTURES, 0);
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

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_80030108);

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
        queueCdSectorCommand(2, DAT_MODE2_TEXTURES_SECTOR, DAT_MODE2_TEXTURES_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
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

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_800302F8);

void drawFullScreenTexturedBackground(void) {
    setSpriteColor(128, 128, 128);
    drawPagedTexturedRectangle(-160, -120, 320, 240, 0x85, 0x39F0,
                  &D_800EBFF0[D_8013AF78], 0xFFF);
}

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_800309EC);

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_80030B50);

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

void drawRegisteredTaskGridSpriteWithDefaultBlend(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            func_800149A8();
        } else {
            if (parent->mode == 1 && parent->selectedIndex != D_8013AE88->data1A) {
                sprite = func_800736D0(parent->sprites, parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                sprite = func_800736D0(parent->sprites, parent->mapping, (s16)D_8013AE88->data1A);
            }
            setSpriteColor(128, 128, 128);
            func_8006C998((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}

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

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_80031C98);

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_80032280);

void updateRegisteredPlayerModePanelTask(void) {
    Movement2D *position;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    u16 remainder;
    u16 resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    remainder = D_8013AE88->data1E % 10U;
    resource = D_8013AE98->resource14;
    if (remainder == 0) {
        if (resource == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (entry->command == 99) {
        entry->flags = 0;
        func_800149A8();
    } else {
        entry->flags = updateMovement2D(position);
        setSpriteColor(128, 128, 128);
        setTexturedQuadColor(128, 128, 128);
        D_8013AE88->data1A = D_8013AE98->data1A;
        func_8006EF30((s16)(position->x / 4096 + 8), (s16)(position->y / 4096 + 8),
            14, gRegisteredPlayerModePanelTexts[D_8013AE88->data1A], 8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDefaultTexturedPanel((s16)(position->x / 4096), (s16)(position->y / 4096),
            240, 64, 5, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

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

void drawRegisteredTaskTexturedQuadList(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    TextureColor *color;
    s16 texture;
    TexturedQuadListEntry *list;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    color = (TextureColor *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            func_800149A8();
        } else {
            setTexturedQuadColor(color->r, color->g, color->b);
            if (parent->mode == 1 && parent->selectedIndex != D_8013AE88->data1A) {
                texture = func_8007373C(parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                texture = func_8007373C(parent->mapping, (s16)D_8013AE88->data1A);
            }
            list = gRegisteredTaskQuadLists[texture];
            drawTexturedQuadList((s16)(position->x / 4096), (s16)(position->y / 4096),
                list, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}

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

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_80032F3C);

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

INCLUDE_ASM("asm/nonmatchings/menu/mode_transitions", func_80033100);

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
