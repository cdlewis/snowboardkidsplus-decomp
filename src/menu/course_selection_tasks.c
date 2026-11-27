#include "game/menu/save_record_tasks.h"
#include "psx/libetc.h"
#include "save_record_tasks_internal.h"
#include "grouped_selection_internal.h"
#include "game/engine/byte_search.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/menu/selected_records.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/dat_resources.h"
#include "game/io/memory_card.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/race/player/player_resource_textures.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

#define MEMORY_CARD_SLOT_BUFFER_BASE ((u8 *)0x801E4000)
#define MEMORY_CARD_SLOT_BUFFER_STRIDE 0x2000

typedef struct CourseSelectionResource {
    s16 mode;
    s16 selectedIndex;
    s16 previousIndex;
    s16 choiceResult;
} CourseSelectionResource;

typedef struct PlayerGroupedSelectionStatus {
    CourseSelectionResource selection;
    s16 unk08;
} PlayerGroupedSelectionStatus;

typedef struct SavePanelPosition {
    Movement2D movement;
    s16 cardSlot;
} SavePanelPosition;

typedef struct SavePanelResources {
    RegisteredPanelSlot panels[5];
    GameTask *countdown;
    s16 command2C;
} SavePanelResources;

typedef struct GroupedSelectionMenuDescriptor {
    const u8 *mapping;
    u8 spacingX;
    u8 spacingY;
    u8 count;
    u8 unk07;
    const u8 *textMapping;
    const u8 *unk0C;
    u8 *tripletIndices;
} GroupedSelectionMenuDescriptor;

typedef struct MenuModelResource {
    GsDOBJ2 object;
    GsCOORDINATE2 local;
    SVECTOR localRotation;
    GsCOORDINATE2 world;
    SVECTOR worldRotation;
    s16 unkC0, unkC2, unkC4, unkC6;
} MenuModelResource;

typedef struct MenuPlayerModelPosition {
    s32 x;
    s32 y;
    s16 z;
} MenuPlayerModelPosition;

typedef struct ClippedGridSpriteParent {
    u8 unk00[0x44];
    const GridSpriteEntry *sprites;
    const u8 *mapping;
    u8 unk4C[8];
    u16 mode;
    u8 unk56[4];
    u16 count;
    s8 alternateSprites;
} ClippedGridSpriteParent;

typedef struct SaveRecordTaskStatus {
    RegisteredPanelSlot first;
    u8 unk08[8];
    s16 secondResult;
} SaveRecordTaskStatus;

extern MenuRgbColor gMenuColorCycleColor __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 gMainMenuSelectionIndex __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));


extern s16 gMemoryCardTaskState __attribute__((section(".data")));

u16 litObjectDrawAreaDepths[4] __attribute__((section(".data.lit_object_draw_area_depths"))) = {
    0x30, 0x20, 0x50, 0x3E
};

HalfwordPair D_8012F13C[16] __attribute__((section(".data"))) = {0};

TexturedQuadListEntry taskIndexPanelQuadsPalette0[10] __attribute__((section(".data.task_index_panel_quads"))) = {
    {33, 0, 0, 0, 0, 0},
    {34, 0, 72, 0, 176, 0},
    {35, 0, 248, 0, 0, 0},
    {36, 0, 0, 16, 0, 72},
    {37, 0, 72, 16, 176, 72},
    {38, 0, 248, 16, 0, 72},
    {39, 0, 0, 88, 0, 0},
    {40, 0, 72, 88, 176, 0},
    {41, 0, 248, 88, 0, 0},
    {0, 255, 0, 0, 0, 0},
};

TexturedQuadListEntry taskIndexPanelQuadsPalette1[10] __attribute__((section(".data.task_index_panel_quads"))) = {
    {33, 1, 0, 0, 0, 0},
    {34, 1, 72, 0, 176, 0},
    {35, 1, 248, 0, 0, 0},
    {36, 1, 0, 16, 0, 72},
    {37, 1, 72, 16, 176, 72},
    {38, 1, 248, 16, 0, 72},
    {39, 1, 0, 88, 0, 0},
    {40, 1, 72, 88, 176, 0},
    {41, 1, 248, 88, 0, 0},
    {0, 255, 0, 0, 0, 0},
};

const u16 gSaveRecordPushAnyButtonText[16]
    __attribute__((section(".data.save_record_push_any_button_text"), aligned(4))) = {
    0x001A, 0x001F, 0x001D, 0x0012, 0x8020, 0x000B, 0x0018, 0x0023,
    0x8020, 0x000C, 0x001F, 0x001E, 0x001E, 0x0019, 0x0018, 0,
};

s16 gMenuPlayerModelYaw[2] __attribute__((section(".sdata"))) = {0};

IndexedHalfwordRecord gRecordFlagMenuSpriteRecords[18]
    __attribute__((section(".data.record_flag_menu_sprites"), aligned(4))) = {
    {205, 0, 0},
    {206, 0x100, 0x100},
    {207, 0x100, 0x200},
    {208, 0x100, 0},
    {209, 0x100, 0x300},
    {210, 0x100, 0x700},
    {211, 0x100, 0x700},
    {212, 0x100, 0x700},
    {216, 0, 0},
    {217, 0, 0},
    {218, 0, 0},
    {219, 0x100, 0x600},
    {220, 0x100, 0x500},
    {221, 0x100, 0x400},
    {210, 0x100, 0x800},
    {211, 0x100, 0x800},
    {212, 0x100, 0x800},
    {199, 0, 0},
};

extern u8 gGroupedSelectionMode0Items[4];
extern u8 gGroupedSelectionMode1Items[5];
extern u8 gGroupedSelectionMode2Items[4];
extern u8 gGroupedSelectionMode5Items[4];
extern u8 gGroupedSelectionMode4Items[4];
extern u8 gGroupedCourseMenuTextIndices[4];
extern u8 D_8013B440[], D_8013B448[];
extern u8 gDefaultBoardMenuTextIndices[4];
extern u8 gSelectionPanelTripletIndices[4];
extern u8 gExtendedSelectionPanelTripletIndices[5];

GroupedSelectionMenuDescriptor gGroupedCourseMenuDescriptors[]
    __attribute__((section(".data.grouped_course_menu_descriptors"))) = {
    {gGroupedSelectionMode0Items, 0, 24, 4, 0, gGroupedCourseMenuTextIndices, NULL, gSelectionPanelTripletIndices},
    {gGroupedSelectionMode1Items, 0, 20, 5, 0, D_8013B440, NULL, gExtendedSelectionPanelTripletIndices},
    {gGroupedSelectionMode2Items, 0, 24, 4, 0, gGroupedCourseMenuTextIndices, NULL, gSelectionPanelTripletIndices},
    {gDefaultBoardMenuItems, 0, 20, 3, 0, gDefaultBoardMenuTextIndices, NULL, NULL},
    {gGroupedSelectionMode4Items, 0, 16, 3, 0, D_8013B448, NULL, NULL},
    {gGroupedSelectionMode5Items, 0, 24, 4, 0, gGroupedCourseMenuTextIndices, NULL, gSelectionPanelTripletIndices},
};

const u8 gMenuPlayerTextureIds[16]
    __attribute__((section(".data.menu_player_texture_ids"), aligned(4))) = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 21, 20,
};

IndexedHalfwordRecord gPlayerGroupedSelectionSprites[17]
    __attribute__((section(".data.player_grouped_selection_sprites"), aligned(4))) = {
    {205, 0x0, 0x0},
    {223, 0x100, 0x100},
    {224, 0x100, 0x200},
    {225, 0x100, 0x0},
    {226, 0x100, 0x300},
    {238, 0x100, 0x700},
    {239, 0x100, 0x700},
    {240, 0x100, 0x700},
    {231, 0x0, 0x0},
    {232, 0x0, 0x0},
    {233, 0x0, 0x0},
    {234, 0x100, 0x600},
    {235, 0x100, 0x500},
    {236, 0x100, 0x400},
    {238, 0x100, 0x800},
    {239, 0x100, 0x800},
    {240, 0x100, 0x800},
};

const u16 gPlayerGroupedSelectionSpritesPadding
    __attribute__((section(".data.player_grouped_selection_sprites_padding"))) = 0;

GroupedSelectionMenuDescriptor gPlayerGroupedSelectionDescriptors[6]
    __attribute__((section(".data.player_grouped_selection_descriptors"))) = {
    {gGroupedSelectionMode0Items, 0, 16, 3, 0, NULL, NULL, gSelectionPanelTripletIndices},
    {gGroupedSelectionMode1Items, 0, 14, 4, 0, NULL, NULL, gExtendedSelectionPanelTripletIndices},
    {gGroupedSelectionMode2Items, 0, 16, 3, 0, NULL, NULL, gSelectionPanelTripletIndices},
    {gDefaultBoardMenuItems, 0, 14, 3, 0, NULL, NULL, NULL},
    {gGroupedSelectionMode4Items, 0, 14, 3, 0, NULL, NULL, NULL},
    {gGroupedSelectionMode5Items, 0, 16, 3, 0, NULL, NULL, gSelectionPanelTripletIndices},
};

const u16 gPlayerCourseVariantConfirmationText[10]
    __attribute__((section(".data.player_course_variant_confirmation_text"))) = {
    0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x009D, 0x00B0, 0x009D, 0x00A1, 0x0033, 0,
};

u8 gPlayerGroupedSelectionTransitionFlags[4]
    __attribute__((section(".data.player_grouped_selection_transition_flags"))) = {0};

const u32 gGroupedCourseMenuTablePadding
    __attribute__((section(".rodata.grouped_course_menu_padding"))) = 0;

const u32 gRecordFlagSelectionMenuTablePadding
    __attribute__((section(".rodata.record_flag_selection_menu_padding"))) = 0;

const u32 gGroupedSelectionMenuTablePadding
    __attribute__((section(".rodata.grouped_selection_menu_padding"))) = 0;

const u32 gMemoryCardSlotTaskTablePadding
    __attribute__((section(".rodata.memory_card_slot_task_padding"))) = 0;

const u32 gSaveRecordLoadPanelTablePadding
    __attribute__((section(".rodata.save_record_load_panel_padding"))) = 0;

StateValueHistory gMemoryCardSlotHistory __attribute__((section(".bss.state_value_history")));

extern const u16 *gSelectionPrizeDescriptionTexts[10];
extern u8 gPlayerGroupedSelectionStates[2];
void drawClippedRegisteredTaskGridSprite(void);
extern u16 gSaveRecordProgressCurrencyLabel[2];
extern char gSelectionPrizeCurrencyLabel[] __attribute__((section(".data")));
extern const char gRegisteredTripletUnavailableLabel[];

void updateRegisteredPlayerModelTask(void);
void transitionRegisteredPlayerModels(void);
void transitionRegisteredPlayerModelsUp(void);
void transitionRegisteredPlayerModelTexture(void);
void createPaletteEntryCopyTask(s32 x, s32 y, s32 destination, s32 source, s32 index);
extern const u16 gMemoryCardMissingRetryText[32];
extern const u16 gMemoryCardLoadErrorRetryText[36];
extern const u16 gMemoryCardLoadRetryConfirmationText[16];
extern const u16 gMemoryCardLoadInProgressText[38];
extern const u16 gMemoryCardLoadSlotChoiceText[20];
extern const u16 D_801306EC[], D_80130780[];

void updateSaveRecordLoadPanelTask(void);
void drawSaveRecordPushAnyButtonPrompt(void);

void updateCdAwareCourseSelectionTask(void) {
    s32 mode;
    ByteMappingDescriptor *group;
    CourseSelectionResource *selection;
    RegisteredPanelSlot *entries;
    s32 complete;
    s16 selectedIndex;
    u16 course;
    u16 courseParameter;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        if (playerCountAbsolute == 1) {
            mode = 1;
        } else {
            mode = 3;
        }
        group = &gCourseMenuMappingGroups[gCurrentTask->data1A];
        selection->mode = 0;
        selectedIndex = findIdentityMappedIndexOrZero(group, selectedCourseMenuItemAbsolute);
        selection->selectedIndex = selectedIndex;
        selection->previousIndex = selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)gCourseMenuSpriteEntries,
                                         group->mapping, NULL, 0, 0, 20,
                                         -312, -96, group->spacingX, group->spacingY,
                                         group->count, 149, drawSelectedTaskSprite,
                                         drawRegisteredSelectionItemStatus, 4, 1);
        createRegisteredCallbackTask(0, 172, -72, drawSelectionItemDetails, 4, 11);
        createRegisteredTexturePairTask(172, -72, group->textureMapping,
                                        group->count, selection->selectedIndex, 1, 5, 21);
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            selectionMenuFadeStartedGp = 0;
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
    case 6:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && selectionMenuFadeStartedGp == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            selectionMenuFadeStartedGp = complete;
        }
        if (selection->choiceResult != 0 && entries[0].flags == 0) {
            if (selectionMenuFadeStartedGp == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                selectionMenuFadeStartedGp = 1;
            }
            if (selection->choiceResult < 0) {
                GameTask *parent = gCurrentTaskParent;
                selectedCourseMenuItemAbsolute = 0;
                parent->data18 = 6;
                gCurrentTask->data18 = 7;
            } else {
                gCurrentTask->data18 = 50;
            }
        }
        break;
    case 50: {
        GameTask *task = gCurrentTask;
        entries[0].command = 10;
        task->data18 = 51;
        break;
    }
    case 51:
        if (entries[0].flags == 0) {
            gCurrentTask->data18 = 52;
        }
        break;
    case 52: {
        GameTask *task = gCurrentTask;
        group = &gCourseMenuMappingGroups[task->data1A];
        switch (group->mapping[selection->selectedIndex]) {
        case 10: {
            GameTask *parent = gCurrentTaskParent;
            selectedCourseMenuItemAbsolute = 0;
            parent->data18 = 6;
            task->data18 = 7;
            break;
        }
        case 12:
            task->data1A = 0;
            task->data18 = 40;
            break;
        case 11:
            task->data1A = 8;
            task->data18 = 40;
            break;
        default:
            selectedIndex = selectedCourseMenuItemAbsolute = mapMenuByteIndex(group->mapping, selection->selectedIndex);
            course = gCourseMenuCourseIndices[selectedIndex];
            courseParameter = gSelectedCourseRecords[course].unk02;
            gSelectedCourseIndex = course;
            gCurrentTask->data18 = 30;
            selectedLapCountAbsolute = courseParameter;
            break;
        }
        break;
    }
    case 30:
        createChildTaskWithReserve(initializeModeDependentDualResourceTask);
        gCreatedTask->data18 = 1;
        gCurrentTask->data18 = 31;
        break;
    case 32: {
        GameTask *parent = gCurrentTaskParent;
        GameTask *task = gCurrentTask;
        parent->data18 = 8;
        task->data18 = 99;
        queueMenuResourceReads((s16)parent->data18);
        break;
    }
    case 40:
        moveTaskChildrenWithSpacing(0, entries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
        gCurrentTask->data18 = 41;
        break;
    case 41:
        if (entries[0].flags == 0) {
            GameTask *task = gCurrentTask;
            entries[0].command = 99;
            task->data18 = 42;
        }
        break;
    case 42:
        if (entries[0].task == NULL) {
            gCurrentTask->data18 = 2;
        }
        break;
    case 7:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x138000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            gCurrentTask->data18 = 9;
        }
        break;
    case 9:
        gCurrentTask->data18 = 99;
        break;
    case 31:
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 getCourseSelectionGroupIndex(void) {
    s16 result;

    if (playerCountAbsolute == 1) {
        u8 character = gRacePlayers[0].unk009;
        u32 mask = gPlayerSaveRecords[0].masks[character];

        if (mask & 0x04000000) {
            result = 3;
        } else if (mask & 0x02000000) {
            result = 2;
        } else if (mask & 0x01000000) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        u32 mask = gMergedSaveRecord.masks[SAVE_RECORD_COMBINED_MASK_INDEX];

        if (mask & 0x04000000) {
            result = 3;
        } else if (mask & 0x02000000) {
            result = 2;
        } else if (mask & 0x01000000) {
            result = 1;
        } else {
            result = 0;
        }
    }
    return result;
}

void initializeModeDependentDualResourceTask(void) {
    gCurrentTask->data18 = 1;
    if (playerCountAbsolute == 1) {
        gCurrentTask->data1A = 1;
    } else {
        gCurrentTask->data1A = 3;
    }
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateSelectionPrizeMenuTask);
}

void updateSelectionPrizeMenuTask(void) {
    CourseSelectionResource *parentSelection;
    CourseSelectionResource *selection;
    RegisteredPanelSlot *parentEntries;
    RegisteredPanelSlot *entries;
    GameTask *parent;
    GameTask *task;
    GameTask *transitionTask;
    GameTask *resumeTask;
    u16 parentNextState;
    s32 complete;

    if (gCurrentTaskParent->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = gCurrentTaskParent;
    parentSelection = (CourseSelectionResource *)&gTaskMemoryPool[parent->resource14];
    if (parent->resource16 == 0xFFFF) {
        for (;;) {}
    }
    parentEntries = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
    task = gCurrentTask;
    if (task->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[task->resource14];
    if (task->resource16 == 0xFFFF) {
        for (;;) {}
    }
    task = gCurrentTask;
    entries = (RegisteredPanelSlot *)&gTaskMemoryPool[task->resource16];
    selection->selectedIndex = parentSelection->selectedIndex;

    switch (task->data18) {
    case 1:
        moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x138000, -0x60000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (parentEntries[0].flags == 0) {
            parentEntries[0].command = 23;
            configureTaskGridMovement(1, parentEntries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (parentEntries[0].flags == 0) {
            createRegisteredCallbackTask(0, -308, -76, drawSelectionPrizeAndPlayerCounters, 3, 1);
            createRegisteredTextPanelTask(-320, -24, gSelectionPrizeDescriptionTexts, NULL, 4, 11);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (entries[0].flags == 0 || entries[1].flags == 0) {
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[0].task->resource14],
                                -0x80000, -0x4C000, 0x1E000, 0x1E000, 0);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8C000, -0x18000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        if (entries[0].flags == 0 || entries[1].flags == 0) {
            createPaletteRotationTask(816, 245, 1, 12, 3, 5, 91);
            createInputTimedSpriteTask(-40, 44, gMenuInputPromptSequence, 2, gCurrentTask->data1A, 176, 64, 3, 21);
            entries[0].command = 20;
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = gCurrentTask;
            selectionMenuFadeStartedGp = 0;
            transitionTask->data18 = 6;
        }
        break;
    case 6:
    case 7:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && selectionMenuFadeStartedGp == 0) {
            entries[0].command = 22;
            entries[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            selectionMenuFadeStartedGp = complete;
        }
        if (entries[2].flags <= 0) {
            if (selectionMenuFadeStartedGp == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                selectionMenuFadeStartedGp = 1;
            }
            if (entries[2].flags >= 0) {
                s16 panelValues[6];

                panelValues[0] = 21;
                panelValues[1] = 5;
                panelValues[2] = 22;
                panelValues[3] = 5;
                panelValues[4] = 23;
                panelValues[5] = 5;
                createRegisteredMovingPanelTask(gCurrentTask->data1A, panelValues, -120, 32, 0, 16, -120, 64, 5, 31);
                gCurrentTask->data18 = 8;
            } else {
                goto dismiss;
            }
        }
        break;
    case 8:
        if (entries[3].flags >= 0) {
            if (entries[3].flags == 0) {
                gCurrentTask->data18 = 9;
            } else {
            dismiss:
                gCurrentTask->data18 = 30;
            }
        }
        break;
    case 30:
        entries[9].command = 99;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[0].task->resource14],
                            -0x134000, -0x4C000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                            -0x140000, -0x18000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 31;
        break;
    case 31:
        if (entries[0].flags == 0) {
            moveTaskChildrenWithSpacing(1, parentEntries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 32;
        }
        break;
    case 32:
        if (parentEntries[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 33;
        }
        break;
    case 33:
        if (parentEntries[0].flags == 0) {
            parentEntries[0].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = gCurrentTask;
            selectionMenuFadeStartedGp = 0;
            transitionTask->data18 = 34;
        }
        break;
    case 34:
        resumeTask = gCurrentTaskParent;
        parentNextState = 6;
        parentSelection->choiceResult = 0;
        goto finish;
    case 9:
        if (entries[0].flags == 0 && entries[1].flags == 0 && parentEntries[0].flags == 0 &&
            parentEntries[1].flags == 0 && parentEntries[2].flags == 0) {
            entries[9].command = 99;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[0].task->resource14],
                                -0x134000, -0x4C000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x140000, -0x18000, 0x1E000, 0x1E000, -1000);
            configureTaskGridMovement(1, parentEntries[0].task,
                                      -0x138000, -0x60000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[parentEntries[1].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[parentEntries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 10;
        }
        break;
    case 10:
        if (entries[0].flags == 0 && entries[1].flags == 0 && parentEntries[0].flags == 0 &&
            parentEntries[1].flags == 0 && parentEntries[2].flags == 0) {
            gCurrentTask->data18 = 11;
        }
        break;
    case 11:
        resumeTask = gCurrentTaskParent;
        parentNextState = 32;
    finish:
        resumeTask->data18 = parentNextState;
        /* fall through */
    default:
        deleteCurrentTaskSubtree();
        break;
    }
}

static inline PaletteRotationSlot *getPaletteRotationSlot(u16 resourceIndex, s32 index) {
    PaletteRotationSlot *slots = (PaletteRotationSlot *)&gTaskMemoryPool[resourceIndex];
    return &slots[index / 10];
}

void createPaletteRotationTask(s32 x, s32 y, s32 first, s32 last, s32 steps, s32 interval, s32 index) {
    PaletteRotationResource *resource;
    PaletteRotationSlot *slot;
    createChildTaskWithReserve(updatePaletteRotationTask);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (PaletteRotationResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->rect.w = 16;
    resource->rect.x = x;
    resource->rect.y = y;
    resource->rect.h = 1;
    StoreImage(&resource->rect, (u32 *)resource->original);
    resource->interval = interval;
    resource->timer = 0;
    resource->first = first;
    resource->last = last;
    resource->steps = steps;
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(gCurrentTask->resource16, index);
    }
    slot->state = 1;
    slot->unk06 = 0;
    slot->task = gCreatedTask;
}

static inline PaletteRotationSlot *getPaletteEntrySlot(u16 resourceIndex, u16 index) {
    PaletteRotationSlot *slots = (PaletteRotationSlot *)&gTaskMemoryPool[resourceIndex];
    return &slots[(u16)(index / 10)];
}

void updatePaletteRotationTask(void) {
    PaletteRotationResource *resource;
    PaletteRotationSlot *slot;
    s32 i, j, step;
    u16 color;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteRotationResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    switch (slot->state) {
    case 1:
        if (++resource->timer < 2) return;
        for (i = 0; i < 16; i++) resource->rotated[i] = resource->original[i];
        resource->timer = 0;
        slot->state = 0;
        break;
    case 99:
        LoadImage(&resource->rect, (u32 *)resource->original);
        slot->task = 0;
        slot->unk06 = 0;
        deleteCurrentTaskSubtree();
        return;
    }
    if (++resource->timer > resource->interval) {
        resource->timer = 0;
        for (step = 0; step < resource->steps; step++) {
            color = resource->rotated[resource->last];
            for (j = resource->last; j > resource->first; j--) resource->rotated[j] = resource->rotated[j - 1];
            resource->rotated[resource->first] = color;
        }
        LoadImage(&resource->rect, (u32 *)resource->rotated);
    }
}

void drawRegisteredSelectionItemStatus(void) {
    Movement2D *position;
    ByteTriplet *color;
    GroupedIndexResource *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    s16 index;
    u8 character;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    color = (ByteTriplet *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gCurrentTaskParent->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = (GroupedIndexResource *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (parent->values != 0) {
        sprite = gCourseMenuSpriteEntries;
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            sprite = getMappedGridSpriteEntry(sprite, parent->values, (s16)gCurrentTask->data1A);
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096, sprite->sprite,
                sprite->color, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            setSpriteColor(128, 128, 128);
            character = gRacePlayers[0].unk009;
            index = mapCourseMenuByteIndex(parent->values, (s16)gCurrentTask->data1A);
            if ((u16)index < 10) {
                if (playerCountAbsolute == 1) {
                    if ((gMergedSaveRecord.masks[character] & (1U << (index * 2))) > 0) {
                        if (!(gMergedSaveRecord.masks[character] & (2U << (index * 2)))) {
                            goto draw_base_status;
                        }
                        drawSpriteByIndexWithBlend(position->x / 4096 - 17, position->y / 4096 - 1,
                            index + 70, 9, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                    } else if (gMergedSaveRecord.masks[character] & (2U << (index * 2))) {
                        drawSpriteByIndexWithBlend(position->x / 4096 - 17, position->y / 4096 - 1,
                            index + 70, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                    } else {
draw_base_status:
                        if ((gMergedSaveRecord.masks[character] & (1U << (index * 2))) > 0) {
                            drawSpriteByIndexWithBlend(position->x / 4096 - 17, position->y / 4096 - 1,
                                index + 70, 11, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                        } else {
                            drawMenuDecimalNumber8x16(position->x / 4096 - 20, position->y / 4096 + 1,
                                2, 0, index + 1, 3, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                        }
                    }
                } else {
                    drawMenuDecimalNumber8x16(position->x / 4096 - 20, position->y / 4096 + 1,
                        2, 0, index + 1, 3, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                }
            }
            entry->flags = 0;
        }
    }
}

s16 mapCourseMenuByteIndex(const u8 *table, s16 index) {
    s32 value;
    switch (table == NULL) {
    case 1:
        value = index;
        break;
    default:
        value = table[index];
        break;
    }
    return value;
}

void drawSelectionItemDetails(void) {
    Movement2D *position;
    HalfwordPair *selection;
    s16 index;
    ByteMappingDescriptor *group;
    SelectionItemStats *stats;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTaskParent->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (HalfwordPair *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if (gCurrentTask->data1A == 0) {
        return;
    }
    drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                 0xB3, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    group = &gCourseMenuMappingGroups[gCurrentTaskParent->data1A];
    index = mapMenuByteIndex(group->mapping, selection->unk_02);
    if ((u16)index >= 10) {
        return;
    }
    stats = &D_8012FCE4[index];
    if (stats->price && stats->rating) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 84, position->y / 4096 + 128,
                     0xBC, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 88, position->y / 4096 + 136,
                             4, 0, stats->price, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 122, position->y / 4096 + 138,
                     0xBF, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 84, position->y / 4096 + 148,
                     0xBE, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawHalfStepSpriteBar(position->x / 4096 + 88, position->y / 4096 + 156,
                              stats->rating, 5, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
    if (gCurrentTask->data1A == 1) {
        addTilePacketWithBlend(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    } else {
        addTilePacketWithBlend(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

void drawSelectionPrizeAndPlayerCounters(void) {
    Movement2D *position;
    s16 row;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        if (playerCountAbsolute == 1) {
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                          0xC1, 14, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            for (row = 0; row < 3; row++) {
                s32 *prizes = gCoursePlacementPrizes[gSelectedCourseIndex];
                drawDecimalNumber8x8(position->x / 4096 + 48, row * 8 + (s16)(position->y / 4096 + 9),
                                     5, 0, prizes[row], 0, -1,
                                     &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                drawWrappedText8x8(position->x / 4096 + 88, row * 8 + (s16)(position->y / 4096 + 9),
                                   2, gSelectionPrizeCurrencyLabel, 6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
            drawSpriteByIndexWithBlend(position->x / 4096 + 6, position->y / 4096 + 35,
                          0xC2, 14, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawDecimalNumber8x8(position->x / 4096 + 26, position->y / 4096 + 42,
                                 5, 0, gPlayerSaveRecords[0].bestScore[gSelectedCourseIndex],
                                 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawWrappedText8x8(position->x / 4096 + 66, position->y / 4096 + 42,
                               2, gSelectionPrizeCurrencyLabel, 6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                          0xC0, 14, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawMenuDecimalNumber8x16(position->x / 4096 + 64, position->y / 4096 + 19,
                                      2, 0, gRacePlayers[0].unk02A, 5, -1,
                                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawMenuDecimalNumber8x16(position->x / 4096 + 64, position->y / 4096 + 35,
                                      2, 0, gRacePlayers[1].unk02A, 5, -1,
                                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }
}

char gSelectionPrizeCurrencyLabel[3]
    __attribute__((section(".data.selection_prize_currency_label"), aligned(4))) = "\x82" "f";
u8 gSelectionMenuFadeStarted __attribute__((section(".data.selection_prize_currency_label"))) = 0;

void clearHalfwordPairTable(void) {
    s16 i;

    for (i = 0; i < 16; i++) {
        D_8012F13C[i].unk_00 = 0;
        D_8012F13C[i].unk_02 = 0;
    }
}

u32 getSelectedCoursePrize(s16 unusedCourse, s16 place) {
    u32 result;
    s32 (*table)[3];
    s32 *row;

    if (place >= 0 && place < 3) {
        table = gCoursePlacementPrizes;
        row = table[gSelectedCourseIndex];
        result = row[place];
    } else {
        result = 0;
    }
    return result;
}

void initializeCountedPlayerGroupedSelectionBoundsTask(void) {
    RECT *bounds;
    gCurrentTaskParent->data1C++;
    if (playerCountAbsolute == 2) {
        setCurrentTaskCallback(initializeDualResourceTaskAndResetByteState);
    } else {
        gCurrentTask->data18 = 0;
        gCurrentTask->data1A = getPlayerRecordFlagMode(0);
        if (gCurrentTask->resource14 != 0xFFFF) { for (;;) {} }
        gCurrentTask->resource14 = allocateTaskMemoryBlock();
        if (gCurrentTask->resource16 != 0xFFFF) { for (;;) {} }
        gCurrentTask->resource16 = allocateTaskMemoryBlock();
        adjustGroupedIndexForMode(gCurrentTask->data1A, &gRacePlayers[0].unk00A[1]);
        gMenuSelectionStates[0].selection = getGroupedIndexBase(gRacePlayers[0].unk00A[1]);
        if (gMenuSelectionStates[0].selection == 9) {
            gMenuSelectionStates[0].selection += getSaveRecordFlagGroupIndex(0, 0);
        }
        gMenuModelDrawAreas[0].x = 0;
        gMenuModelDrawAreas[0].y = 0;
        gMenuModelDrawAreas[0].w = -1;
        gMenuModelDrawAreas[0].h = -1;
        gMenuModelDrawAreas[1].x = 0;
        bounds = &gMenuModelDrawAreas[1];
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
        gMenuSelectionStates[0].previousSelection = gMenuSelectionStates[0].selection;
        setCurrentTaskCallback(updatePlayerGroupedCourseSelectionTask);
    }
}

void updatePlayerGroupedCourseSelectionTask(void) {
    CourseSelectionResource *selection;
    RegisteredPanelSlot *panels;
    MenuSelectionState *groupedSelection;
    GroupedSelectionMenuDescriptor *descriptor;
    s32 complete;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    groupedSelection = &gMenuSelectionStates[0];

    switch (gCurrentTask->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        descriptor = &gGroupedCourseMenuDescriptors[gCurrentTask->data1A];
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gRecordFlagMenuSpriteRecords,
                                         descriptor->mapping, NULL, 0, 0, 20, -304, -96,
                                         descriptor->spacingX, descriptor->spacingY, descriptor->count, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-316, 8, gSelectionPanelTexts, descriptor->textMapping, 4, 11);
        createRegisteredPanelTask(0, groupedSelection->selection, 244, 0, 384, 21);
        createRegisteredCallbackTask(0, 172, -72, drawTaskOffsetSpriteB8, 4, 31);
        createRegisteredTripletTask(-296, 48, 0, 16, gSelectionPanelStatTriplets, descriptor->tripletIndices, 3, 41);
        selection->mode = 0;
        if (gRacePlayers[0].unk00A[1] < 9 && (u8)(gRacePlayers[0].unk00A[1] % 3) == 0) {
            selection->selectedIndex = 0;
        }
        if (gRacePlayers[0].unk00A[1] < 9 && (u8)(gRacePlayers[0].unk00A[1] % 3) == 1) {
            selection->selectedIndex = 1;
        }
        if (gRacePlayers[0].unk00A[1] < 9 && (u8)(gRacePlayers[0].unk00A[1] % 3) == 2) {
            selection->selectedIndex = 2;
        }
        if (gRacePlayers[0].unk00A[1] >= 9 && gRacePlayers[0].unk00A[1] < 12) {
            selection->selectedIndex = 3;
        }
        if (gRacePlayers[0].unk00A[1] >= 12) {
            selection->selectedIndex = gRacePlayers[0].unk00A[1] - 12;
        }
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x88000, 0x8000, 0x1E000, 0x1E000, 5);
            configureRegisteredChildMovement(0, panels[2].task, 0x40000, 0, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14], -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[4].task->resource14], -0x74000, 0x30000, 0x1E000, 0x1E000, 5);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
            panels[0].command = 20;
            panels[1].command = 20;
            panels[2].command = 1;
            panels[3].command = 20;
            panels[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            panels[3].command = 22;
            panels[4].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceGroupedIndex(panels[0].task, selection->selectedIndex);
            if (groupedSelection->selection == 9) {
                groupedSelection->selection += getSaveRecordFlagGroupIndex(0, 0);
            }
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else if (groupedSelection->selection != groupedSelection->previousSelection) {
                if (selection->previousIndex >= selection->selectedIndex) {
                    panels[2].command = 12;
                } else {
                    panels[2].command = 11;
                }
            }
            selection->previousIndex = selection->selectedIndex;
        } else if (selection->choiceResult != 0 && panels[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                panels[0].command = 22;
                panels[1].command = 22;
                panels[3].command = 22;
                panels[4].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (selection->choiceResult < 0) {
                gCurrentTask->data18 = 14;
            } else {
                descriptor = &gGroupedCourseMenuDescriptors[gCurrentTask->data1A];
                if (descriptor->mapping[selection->selectedIndex] == 0) {
                    selection->choiceResult = -1;
                    gCurrentTask->data18 = 14;
                } else {
                    panels[0].command = 10;
                    gCurrentTask->data18 = 12;
                }
            }
        }
        break;
    case 12:
        panels[0].command = 10;
        gCurrentTask->data18 = 13;
        break;
    case 13:
        if (panels[0].flags == 0) {
            gCurrentTask->data18 = 9;
        }
        break;
    case 14:
        switch (gMainMenuSelectionIndex) {
        case 0:
            gCurrentTaskParent->data18 = 7;
            break;
        case 2:
            gCurrentTaskParent->data18 = 10;
            break;
        case 1:
            gCurrentTaskParent->data18 = 20;
            break;
        }
        gCurrentTask->data18 = 5;
        break;
    case 9:
        createChildTaskWithReserve(initializeDualResourceTaskAndClearState);
        gCreatedTask->data18 = 1;
        descriptor = &gGroupedCourseMenuDescriptors[gCurrentTask->data1A];
        gCreatedTask->data1A = mapCourseMenuByteIndex(descriptor->mapping, selection->selectedIndex);
        gCurrentTask->data18 = 10;
        break;
    case 10:
        break;
    case 11:
        beginMode2SequenceAndFadeMusic();
        gCurrentTaskParent->data18 = 23;
        goto finishExit;
    case 5:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
            queueMenuResourceReads(gCurrentTaskParent->data18);
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            panels[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            configureRegisteredChildMovement(0, panels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, panels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14], 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[4].task->resource14], -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 6;
        }
        break;
    case 6:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
finishExit:
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        if (selection->choiceResult <= 0 || isTaskStateTransitionBelowMaximum(0) == 0) {
            gCurrentTask->data18 = 8;
        }
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void initializeDualResourceTaskAndClearState(void) {
    gCurrentTask->data18 = 0;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateRecordFlagSelectionMenuTask);
}

void updateRecordFlagSelectionMenuTask(void) {
    CourseSelectionResource *parentSelection;
    RegisteredPanelSlot *parentPanels;
    CourseSelectionResource *selection;
    RegisteredPanelSlot *panels;
    MenuSelectionState *groupedSelection;
    RecordFlagMenuEntries entryStorage;
    RecordFlagMenuEntries *entries;
    s32 complete;
    s16 value;

    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parentSelection = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
    parentPanels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    groupedSelection = &gMenuSelectionStates[0];

    switch (gCurrentTask->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (parentPanels[0].flags == 0) {
            parentPanels[0].command = 23;
            parentPanels[1].command = 23;
            parentPanels[4].command = 23;
            configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        if (parentPanels[0].flags == 0) {
            entries = &entryStorage;
            buildRecordFlagMenuEntries(0, gCurrentTaskParent->data1A, parentSelection->selectedIndex, 0, 20, entries);
            createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gRecordFlagMenuSpriteRecords,
                                             entries->entries, NULL, 0, 0, 20, -304, -96,
                                             entries->unk_04, entries->unk_05, entries->count, 222,
                                             drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 5, 1);
            createRegisteredTextPanelTask(-136, 8, gSelectionPanelTexts, entries->textIndices, 4, 11);
            selection->mode = 0;
            value = getGroupedIndexOffsetPlus216(groupedSelection->selection);
            selection->selectedIndex = value;
            value = findIndexedHalfwordRecord(gRecordFlagMenuSpriteRecords, entries->entries, entries->count, value);
            selection->selectedIndex = value;
            selection->previousIndex = value;
            selection->choiceResult = 0;
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x74000, -0x60000, 0, 0);
            configureVerticalTaskGridMovement(0, panels[0].task, -0x74000, -0x4C000, 0x8000, 0x8000, 2);
            initMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14], -0x13C000, 0x8000);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (panels[0].flags == 0 && panels[1].flags == 0) {
            panels[0].command = 20;
            panels[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (parentPanels[2].flags == 0) {
            if (selection->choiceResult > 0) {
                parentPanels[2].command = 3;
            }
            if (selection->choiceResult != 0) {
                gCurrentTask->data18 = 6;
            }
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(gCurrentTask->data1A, panels[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else {
                parentPanels[2].command = 13;
            }
            selection->previousIndex = selection->selectedIndex;
        } else if (gCurrentTask->data18 != 5 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 22;
            panels[1].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        break;
    case 6:
        if (parentPanels[2].flags == 0 && selection->choiceResult != 0 && panels[0].flags == 0) {
            if (selection->choiceResult < 0) {
                gCurrentTask->data18 = 10;
            } else {
                gRacePlayers[0].unk00A[1] = getTaskResourceAdjustedGroupedIndex(gCurrentTask->data1A, panels[0].task, selection->selectedIndex);
                gCurrentTask->data18 = 7;
            }
        }
        break;
    case 10:
        configureTaskGridMovement(0, panels[0].task, -0x74000, -0x60000, 0x1E000, 0x1E000, 0);
        panels[0].command = 23;
        gCurrentTask->data18 = 11;
        break;
    case 11:
        if (panels[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x128000, -0x60000, 0, 0);
            moveTaskChildrenWithSpacing(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 12;
        }
        break;
    case 12:
        if (parentPanels[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 13;
        }
        break;
    case 13:
        if (parentPanels[0].flags == 0) {
            initMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x13C000, 0x8000);
            initMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14], -0x88000, 0x8000);
            parentPanels[2].command = 1;
            parentPanels[0].command = 20;
            parentPanels[1].command = 20;
            parentPanels[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 14;
        }
        break;
    case 14:
        parentSelection->choiceResult = 0;
        gCurrentTaskParent->data18 = 4;
        goto finish;
    case 7:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            parentPanels[0].flags == 0 && parentPanels[1].flags == 0 &&
            parentPanels[2].flags == 0 && parentPanels[3].flags == 0 && parentPanels[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x128000, -0x4C000, 0x1E000, 0x1E000, -1000);
            panels[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            configureTaskGridMovement(0, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14], -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            configureRegisteredChildMovement(0, parentPanels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, parentPanels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[3].task->resource14], 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[4].task->resource14], -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            parentPanels[0].flags == 0 && parentPanels[1].flags == 0 &&
            parentPanels[2].flags == 0 && parentPanels[3].flags == 0 && parentPanels[4].flags == 0) {
            gCurrentTaskParent->data18 = 11;
            goto finish;
        }
        break;
    default:
finish:
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 getPlayerRecordFlagMode(s16 index) {
    s32 result;
    SaveRecord *record;
    s16 playerIndex = index;
    if (index >= 10) {
        record = &gMergedSaveRecord;
        playerIndex = index - 10;
    } else {
        record = &gPlayerSaveRecords[index];
    }
    if (gRacePlayers[playerIndex].unk009 == 8) {
        result = 2;
    } else if (gRacePlayers[playerIndex].unk009 == 9) {
        result = 5;
    } else {
        result = (record->flags & 0x3800) != 0;
    }
    return result;
}

void buildRecordFlagMenuEntries(s16 index, s16 mode, s16 group, u16 arg4, u16 arg5,
                                RecordFlagMenuEntries *out) {
    SaveRecord *record;
    s32 i;

    out->unk_05 = arg5;
    out->unk_04 = arg4;
    if (index >= 10) {
        record = &gMergedSaveRecord;
        index -= 10;
    } else {
        record = &gPlayerSaveRecords[index];
    }

    switch (mode) {
    case 2:
    case 5:
        out->entries = gRecordFlagMenuItemBuffers[index];
        out->entries[0] = 8;
        out->count = 1;
        out->textIndices = gDefaultBoardMenuTextIndices;
        out->unk_10 = 0;
        return;
    case 1:
        if (group != 3) {
            goto standardGroups;
        }
        out->entries = gRecordFlagMenuItemBuffers[index];
        out->count = 0;
        out->textIndices = gRecordFlagMenuTextIndexBuffers[index];
        out->unk_10 = 0;
        for (i = 0; i < 3; i++) {
            if (record->flags & (0x800 << i)) {
                out->entries[out->count] = i + 11;
                out->textIndices[out->count] = i + 5;
                out->count++;
            }
        }
        return;
    case 0:
standardGroups:
        if (group < 3) {
            out->entries = gRecordFlagMenuItemBuffers[index];
            out->count = 0;
            out->textIndices = gRecordFlagMenuTextIndexBuffers[index];
            out->unk_10 = 0;
            if (group == 0) {
                for (i = 0; i < 3; i++) {
                    if (record->flags & (4 << i)) {
                        out->entries[out->count] = i + 8;
                        out->textIndices[out->count] = 8;
                        out->count++;
                    }
                }
            } else if (group == 1) {
                for (i = 0; i < 3; i++) {
                    if (record->flags & (0x20 << i)) {
                        out->entries[out->count] = i + 8;
                        out->textIndices[out->count] = 8;
                        out->count++;
                    }
                }
            } else if (group == 2) {
                for (i = 0; i < 3; i++) {
                    if (record->flags & (0x100 << i)) {
                        out->entries[out->count] = i + 8;
                        out->textIndices[out->count] = 8;
                        out->count++;
                    }
                }
            }
            return;
        }
        /* fall through */
    default:
        out->entries = NULL;
        out->textIndices = NULL;
        out->unk_10 = 0;
        return;
    }
}

s32 getSaveRecordFlagGroupIndex(s16 recordIndex, s16 groupIndex) {
    SaveRecord *record;
    s16 i;
    if (recordIndex >= 10) {
        record = &gMergedSaveRecord;
    } else {
        record = &gPlayerSaveRecords[recordIndex];
    }
    for (i = 0; i < 3; i++) {
        if (record->flags & (0x800 << (groupIndex * 3 + i))) {
            return i;
        }
    }
    return 0;
}

s16 getTaskResourceGroupedIndex(GameTask *task, s16 index) {
    s16 result;
    s32 base = 0;
    GroupedIndexResource *resource = (GroupedIndexResource *)&gTaskMemoryPool[task->resource14];
    switch (resource->values[index]) {
    case 1: result = base; break;
    case 2: result = base + 1; break;
    case 3: result = base + 2; break;
    case 4: result = base + 9; break;
    case 5:
    case 14: result = 12; break;
    case 6:
    case 15: result = 13; break;
    case 7:
    case 16: result = 14; break;
    default: result = -1; break;
    }
    return result;
}

s16 getTaskResourceAdjustedGroupedIndex(s16 value, GameTask *task, s16 index) {
    s16 result;
    s32 base;
    GroupedIndexResource *resource = (GroupedIndexResource *)&gTaskMemoryPool[task->resource14];
    switch (resource->values[index]) {
    case 8: base = 0; break;
    case 9: base = 3; break;
    case 10: base = 6; break;
    case 11: base = 0; break;
    case 12: base = 1; break;
    case 13: base = 2; break;
    default: base = 0; break;
    }
    switch (value) {
    case 1: result = base; break;
    case 2: result = base + 1; break;
    case 3: result = base + 2; break;
    case 4: result = base + 9; break;
    case 5:
    case 14: result = 12; break;
    case 6:
    case 15: result = 13; break;
    case 7:
    case 16: result = 14; break;
    }
    return result;
}

void adjustGroupedIndexForMode(s16 mode, u8 *index) {
    s32 value = *index;
    switch (mode) {
    case 2:
    case 5:
        if (value < 12) {
            value = 12;
        }
        break;
    case 0:
    case 1:
        if (value >= 12) {
            value = 0;
        }
        break;
    }
    *index = value;
}

s16 getGroupedIndexBase(s16 index) {
    s16 result;
    if (index < 9) {
        result = index % 3;
    } else if ((u16)(index - 9) < 3) {
        result = 9;
    } else if (index >= 12) {
        result = index;
    }
    return result;
}

s16 getGroupedIndexOffset(s16 index) {
    s16 result;
    if (index < 9) {
        result = index / 3;
    } else if ((u16)(index - 9) < 3) {
        result = index - 9;
    } else if (index >= 12) {
        result = index - 12;
    }
    return result;
}

s16 getGroupedIndexOffsetPlus216(s16 index) {
    s16 result;
    if (index < 9) {
        result = index / 3 + 216;
    } else if ((u16)(index - 9) < 3) {
        result = index + 210;
    } else if (index >= 12) {
        result = index + 204;
    }
    return result;
}

s16 findIndexedHalfwordRecord(IndexedHalfwordRecord *records, u8 *indices, s16 count, s16 value) {
    s16 result = 0;
    s16 i;
    for (i = 0; i < count; i++) {
        if (records[indices[i]].value == value) {
            result = i;
            break;
        }
    }
    return result;
}

static inline RegisteredPanelSlot *getPanelSlot(u32 resource, s32 index) {
    RegisteredPanelSlot *slots = (RegisteredPanelSlot *)&gTaskMemoryPool[resource];
    return &slots[index / 10];
}

void createRegisteredPanelTask(s32 mode, s32 value, s16 x, s16 y, s16 depth, s32 index) {
    Movement2D *movement;
    RegisteredPanelSlot *slot;
    createChildTaskWithReserve(initRegisteredPlayerModels);
    gCreatedTask->data18 = mode;
    gCreatedTask->data1A = value;
    gCreatedTask->data1C = 0;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    movement = (Movement2D *)&gTaskMemoryPool[gCreatedTask->resource14];
    initMovement2D(movement, x << 12, y << 12);
    movement->unk08 = depth;
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(gCurrentTask->resource16, index);
    }
    slot->command = 0;
    slot->flags = 1;
    slot->task = gCreatedTask;
}

void initRegisteredPlayerModels(void) {
    MenuPlayerModelPosition *position;
    s16 index;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    } else {
        position = (MenuPlayerModelPosition *)&gTaskMemoryPool[gCurrentTask->resource14];
        index = gCurrentTask->data18;
        createRegisteredPlayerModelTask(index * 2, index, gCurrentTask->data1A,
                                        position->x / 4096, position->y / 4096, position->z, 1);
        createRegisteredPlayerModelTask(index * 2 + 1, index, 0, 320, 240, position->z, 11);
        setCurrentTaskCallback(updateRegisteredPlayerModels);
    }
}

static inline RegisteredTaskEntry *getRegisteredMenuTaskEntry(u16 resource, u32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&gTaskMemoryPool[resource];
    return &entries[(u16)(index / 10)];
}

void updateRegisteredPlayerModels(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    MenuModelSelectionPair *alternateSelection;
    u16 playerIndex;
    s16 childCommand;

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
    selection = &gMenuSelectionStates[(s16)gCurrentTask->data18];
    playerIndex = gCurrentTask->data18;
    alternateSelection = &gMenuPlayerTextureSelections[(s16)gCurrentTask->data18];

    switch (entry->command) {
    case 0:
        entry->flags = 0;
        break;
    case 1:
        childCommand = 1;
        goto commandChildren;
    case 2:
        childCommand = 3;
commandChildren:
        children[0].command = childCommand;
        children[1].command = childCommand;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 3:
        children[0].command = 3;
        children[1].command = 3;
        entry->command = 10;
        entry->flags = 1;
        break;
    case 10:
        setCurrentTaskCallback(updateRegisteredChildBlinkTransition);
        entry->command = 1;
        entry->flags = 1;
        selection->blinkTimer = 20;
        break;
    case 11:
        if (selection->selection != selection->previousSelection) {
            setCurrentTaskCallback(transitionRegisteredPlayerModels);
            entry->command = 1;
            entry->flags = 1;
        }
        break;
    case 12:
        if (selection->selection != selection->previousSelection) {
            setCurrentTaskCallback(transitionRegisteredPlayerModelsUp);
            entry->command = 1;
            entry->flags = 1;
        }
        break;
    case 13:
        if (selection->selection != selection->previousSelection) {
            u16 slot = gCurrentTask->data1C;
            Movement2D *movement = (Movement2D *)&gTaskMemoryPool[children[slot].task->resource16];
            reloadTaskPlayerModel(children[slot].task, (s16)(slot + (s16)playerIndex * 2),
                                  playerIndex, selection->selection,
                                  movement->x / 4096, movement->y / 4096, position->z);
            entry->command = 0;
            entry->flags = 0;
            selection->previousSelection = selection->selection;
        }
        break;
    case 20:
        if (alternateSelection->selection != alternateSelection->previousSelection) {
            setCurrentTaskCallback(transitionRegisteredPlayerModelTexture);
            entry->command = 1;
            entry->flags = 1;
        }
        break;
    case 99:
    default:
        entry->task = 0;
        deleteCurrentTaskSubtree();
        return;
    }
    entry->flags |= children[0].flags;
    entry->flags |= children[1].flags;
}

void configureRegisteredChildMovement(s32 index, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    RegisteredTaskEntry *entries;
    Movement2D *movement;

    initMovement2D((Movement2D *)&gTaskMemoryPool[task->resource14], x, y);
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[task->resource16];
    movement = (Movement2D *)&gTaskMemoryPool[entries[index].task->resource16];
    x = stepX ? x : movement->x;
    y = stepY ? y : movement->y;
    configureMovement2D(movement, x, y, stepX, stepY, delay);
}

void initRegisteredChildMovement(s32 index, GameTask *task, s32 x, s32 y) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&gTaskMemoryPool[task->resource16];
    GameTask *child = entries[index].task;
    Movement2D *movement = (Movement2D *)&gTaskMemoryPool[child->resource16];
    initMovement2D(movement, x, y);
}

static inline RegisteredTaskEntry *getRegisteredTripletEntry(u16 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&gTaskMemoryPool[resource];
    return &entries[index / 10];
}

void createRegisteredTripletTask(s32 x, s32 y, s32 offsetX, s32 offsetY, ByteTriplet *records, u8 *indices, s32 depth, s32 index) {
    RegisteredTripletData *data;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(updateRegisteredTripletTask);
    gCreatedTask->data1A = 0;
    gCreatedTask->data1C = depth;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[gCreatedTask->resource14], (s16)x << 12, (s16)y << 12);
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    data = (RegisteredTripletData *)&gTaskMemoryPool[gCreatedTask->resource16];
    data->records = records;
    data->indices = indices;
    data->offsetX = offsetX;
    data->offsetY = offsetY;
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTripletEntry(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTripletEntry(gCurrentTask->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = gCreatedTask;
}

void updateRegisteredTripletTask(void) {
    Movement2D *position;
    RegisteredTripletData *data;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    HalfwordPair *selection;
    ByteTriplet *values;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    data = (RegisteredTripletData *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (gCurrentTaskParent->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (HalfwordPair *)&gTaskMemoryPool[gCurrentTaskParent->resource14];

    {
        s16 command = entry->command;
        if (command == 21) goto modeTwo;
        if (command < 22) {
            if (command == 0) goto clearFlags;
            if (command == 20) goto modeOne;
            goto remove;
        }
        if (command == 23) goto hide;
        if (command < 23) goto modeThree;
        goto remove;
    }

modeOne:
    gCurrentTask->data1A = 1;
    goto clearCommand;
modeTwo:
    gCurrentTask->data1A = 2;
    goto clearCommand;
modeThree:
    gCurrentTask->data1A = 3;
    goto clearCommand;
hide:
    gCurrentTask->data1A = 0;
clearCommand:
    entry->command = 0;
clearFlags:
    entry->flags = 0;
    goto update;

remove:
    entry->task = 0;
    deleteCurrentTaskSubtree();
    return;

update:
    entry->flags |= updateMovement2D(position);
    if (gCurrentTask->data1A == 0) {
        return;
    }
    if ((u32)(gCurrentTask->data1A - 1) < 2) {
        setSpriteColor(gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b);
    } else {
        setSpriteColor(128, 128, 128);
    }
    if (data->records == 0 || data->indices == 0) {
        return;
    }
    values = lookupByteTriplet(data->records, data->indices, selection->unk_02);
    if (!values->bytes[0] && !values->bytes[1] && !values->bytes[2]) {
        return;
    }

    drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096, 30, 1, -1,
                  &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawSpriteByIndexWithBlend(position->x / 4096 + data->offsetX, position->y / 4096 + data->offsetY,
                  31, 1, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawSpriteByIndexWithBlend(data->offsetX * 2 + position->x / 4096, data->offsetY * 2 + position->y / 4096,
                  32, 1, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);

    if (values->bytes[0] == 255) {
        drawWrappedText8x8(position->x / 4096 + 56, position->y / 4096, 2, gRegisteredTripletUnavailableLabel,
                           6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    } else {
        drawHalfStepSpriteBar(position->x / 4096 + 56, position->y / 4096, values->bytes[0],
                              5, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
    if (values->bytes[1] == 255) {
        drawWrappedText8x8((s16)(position->x / 4096 + 56) + data->offsetX, position->y / 4096 + data->offsetY,
                           2, gRegisteredTripletUnavailableLabel, 6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    } else {
        drawHalfStepSpriteBar((s16)(position->x / 4096 + 56) + data->offsetX, position->y / 4096 + data->offsetY,
                              values->bytes[1], 5, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
    if (values->bytes[2] == 255) {
        drawWrappedText8x8(data->offsetX * 2 + (s16)(position->x / 4096 + 56), data->offsetY * 2 + position->y / 4096,
                           2, gRegisteredTripletUnavailableLabel, 6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    } else {
        drawHalfStepSpriteBar(data->offsetX * 2 + (s16)(position->x / 4096 + 56), data->offsetY * 2 + position->y / 4096,
                              values->bytes[2], 5, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

const char gRegisteredTripletUnavailableLabel[4]
    __attribute__((section(".data.registered_triplet_unavailable_label"), aligned(4))) = "\x81\x48";

void createRegisteredPlayerModelTask(s16 slot, s16 selection, s16 modelIndex,
                                     s16 x, s16 y, s16 z, s32 index) {
    MenuModelResource *resource;
    u32 *offsets;
    u32 *textureOffsets;
    u32 *entry;
    u8 *archive;
    u32 (*buffers)[512];
    u32 *dest;
    u32 *source;
    u32 *model;
    u32 i;
    RegisteredTaskEntry *registration;

    createChildTaskWithReserve(updateRegisteredPlayerModelTask);
    gCreatedTask->data18 = selection;
    gCreatedTask->data1A = modelIndex;
    gCreatedTask->data1C = slot;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    offsets = DAT_STARTUP_MODELS->offsets;
    archive = (u8 *)DAT_STARTUP_MODELS;
    buffers = DAT_PLAYER_MODEL_BLOCKS;
    i = 0;
    resource = (MenuModelResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    entry = &offsets[gPlayerModelArchiveIndices[gCreatedTask->data1A]];
    model = buffers[gCreatedTask->data1C];
    source = (u32 *)(*entry + archive);
    dest = model;
    for (; i < 512; i++) {
        *dest++ = *source++;
    }
    entry = (u32 *)((u32)model | 4);
    GsMapModelingData(entry);
    entry += 2;
    if (gCreatedTask->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)entry, &gPlayerRootTextureOffsets[gCreatedTask->data1C]);
    }
    GsLinkObject4(entry, &resource->object, 0);
    GsInitCoordinate2(0, &resource->world);
    GsInitCoordinate2(&resource->world, &resource->local);
    resource->object.coord2 = &resource->local;
    resource->object.attribute = 0;
    textureOffsets = DAT_STARTUP_TEXTURES->offsets;
    entry = &textureOffsets[gPlayerResourceTextureSelections.entries[gCreatedTask->data18][gCreatedTask->data1A]];
    uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *entry),
                             gPlayerResourceTexturePositions[gCreatedTask->data1C]);
    resource->world.coord.t[0] = x;
    resource->world.coord.t[1] = y;
    resource->world.coord.t[2] = z;
    resource->worldRotation.vx = 0;
    resource->worldRotation.vy = 0;
    resource->worldRotation.vz = 0;
    gMenuPlayerModelYaw[selection] = resource->worldRotation.vy;
    resource->local.coord.t[0] = 0;
    resource->local.coord.t[1] = 0;
    resource->local.coord.t[2] = 0;
    resource->localRotation.vx = 0x400;
    resource->localRotation.vy = -0x249;
    resource->localRotation.vz = 0;
    resource->unkC2 = 0;
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[gCreatedTask->resource16], x << 12, y << 12);
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
        registration = getRegisteredTripletEntry(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
        registration = getRegisteredTripletEntry(gCurrentTask->resource16, index);
    }
    registration->command = 0;
    registration->flags = 1;
    registration->task = gCreatedTask;
}

void reloadTaskPlayerModel(GameTask *task, s16 slot, s16 selection, s16 modelIndex, s16 x, s16 y, s16 z) {
    MenuModelResource saved;
    MenuModelResource *resource;
    u32 *offsets;
    u32 *textureOffsets;
    u32 *entry;
    u8 *archive;
    u32 (*buffers)[512];
    u32 *dest;
    u32 *source;
    u32 *model;
    u32 i;
    task->data18 = selection;
    task->data1A = modelIndex;
    task->data1C = slot;
    resource = (MenuModelResource *)&gTaskMemoryPool[task->resource14];
    saved = *resource;
    offsets = DAT_STARTUP_MODELS->offsets;
    archive = (u8 *)DAT_STARTUP_MODELS;
    buffers = DAT_PLAYER_MODEL_BLOCKS;
    i = 0;
    entry = &offsets[gPlayerModelArchiveIndices[task->data1A]];
    source = (u32 *)(*entry + archive);
    model = buffers[task->data1C];
    dest = model;
    for (; i < 512; i++) { *dest++ = *source++; }
    entry = (u32 *)((u32)model | 4);
    GsMapModelingData(entry);
    entry += 2;
    if (task->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)entry, &gPlayerRootTextureOffsets[task->data1C]);
    }
    GsLinkObject4(entry, &resource->object, 0);
    GsInitCoordinate2(0, &resource->world);
    GsInitCoordinate2(&resource->world, &resource->local);
    resource->object.coord2 = &resource->local;
    resource->object.attribute = 0;
    textureOffsets = DAT_STARTUP_TEXTURES->offsets;
    entry = &textureOffsets[gPlayerResourceTextureSelections.entries[task->data18][task->data1A]];
    uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *entry), gPlayerResourceTexturePositions[task->data1C]);
    resource->world.coord.t[0] = x;
    resource->world.coord.t[1] = y;
    resource->world.coord.t[2] = z;
    resource->worldRotation.vx = saved.worldRotation.vx;
    resource->worldRotation.vy = saved.worldRotation.vy;
    resource->worldRotation.vz = saved.worldRotation.vz;
    resource->local.coord.t[0] = saved.local.coord.t[0];
    resource->local.coord.t[1] = saved.local.coord.t[1];
    resource->local.coord.t[2] = saved.local.coord.t[2];
    resource->localRotation.vx = saved.localRotation.vx;
    resource->localRotation.vy = saved.localRotation.vy;
    resource->localRotation.vz = saved.localRotation.vz;
    resource->unkC2 = saved.unkC2;
    initMovement2D((Movement2D *)&gTaskMemoryPool[task->resource16], x << 12, y << 12);
}

void updateRegisteredPlayerModelTask(void) {
    MenuModelResource *model;
    Movement2D *movement;
    RegisteredTaskEntry *entry;
    u8 padding[16];

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    model = (MenuModelResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource16];

    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }

    {
        s16 command = entry->command;
        if (command == 2) goto keepRotating;
        if (command < 3) {
            if (command == 0) goto clearFlags;
            if (command == 1) goto startRotating;
            goto remove;
        }
        if (command == 3) goto resetRotation;
        goto remove;
    }

startRotating:
    model->unkC2 = -32;
    entry->command = 0;
    entry->flags = 0;
    if (gCurrentTask->data18 == 0) {
        model->worldRotation.vy = gMenuPlayerModelYaw[1];
    } else {
        model->worldRotation.vy = gMenuPlayerModelYaw[0] - model->unkC2;
    }
    goto update;

keepRotating:
    model->unkC2 = -32;
    goto clearCommand;

resetRotation:
    model->worldRotation.vy = 0;
    model->unkC2 = 0;
clearCommand:
    entry->command = 0;
clearFlags:
    entry->flags = 0;
    goto update;

remove:
    entry->task = 0;
    entry->flags = 0;
    deleteCurrentTaskSubtree();
    return;

update:
    entry->flags |= updateMovement2D(movement);
    model->world.coord.t[0] = movement->x / 4096;
    model->world.coord.t[1] = movement->y / 4096;
    model->worldRotation.vy += model->unkC2;
    gMenuPlayerModelYaw[gCurrentTask->data18] = model->worldRotation.vy;
    setCoordinateRotation(&model->worldRotation, &model->world);
    setCoordinateRotation(&model->localRotation, &model->local);
    if (model->world.coord.t[2] >= 512) {
        sortLitObjectWithDrawArea(&model->object, gCurrentTask->data18 + 2, &gMenuModelDrawAreas[gCurrentTask->data18]);
    } else {
        u16 index = gCurrentTask->data1C;
        sortLitObjectWithDrawArea(&model->object, index & 1, &gMenuModelDrawAreas[index]);
    }
}

void sortLitObjectWithDrawArea(GsDOBJ2 *object, u16 index, RECT *bounds) {
    MATRIX matrix;
    GsGetLw(object->coord2, &matrix);
    GsSetLightMatrix(&matrix);
    GsGetLs(object->coord2, &matrix);
    GsSetLsMatrix(&matrix);
    {
        u32 depthIndex = index;
        GsSortObject4(object, &gMainOrderingTables[gActiveGraphicsBuffer], (depthIndex & 1) + 1,
                      (u32 *)getScratchAddr(0));
        if (bounds->w >= 0 && bounds->h >= 0) {
            addDrawAreaPacket(bounds->x, bounds->y, bounds->w, bounds->h,
                              &gMainOrderingTables[gActiveGraphicsBuffer], litObjectDrawAreaDepths[depthIndex]);
            addDrawAreaPacket(0, 0, 320, 240, &gMainOrderingTables[gActiveGraphicsBuffer], 14);
        }
    }
}

ByteTriplet *lookupByteTriplet(ByteTriplet *records, u8 *indices, s16 index) {
    return &records[indices[index]];
}

void transitionRegisteredPlayerModels(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    s16 player;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    u16 playerIndex;
    s32 command;

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
    selection = &gMenuSelectionStates[(s16)gCurrentTask->data18];
    playerIndex = gCurrentTask->data18;
    command = entry->command;
    if (command == 1) {
        s16 incomingSlot;
        s32 outgoingSlot;
        Movement2D *movement;
        if (gCurrentTask->data1C == 0) {
            outgoingSlot = 0;
            incomingSlot = 1;
            gCurrentTask->data1C = command;
        } else {
            outgoingSlot = 1;
            incomingSlot = 0;
            gCurrentTask->data1C = 0;
        }
        movement = (Movement2D *)&gTaskMemoryPool[children[incomingSlot].task->resource16];
        player = playerIndex;
        reloadTaskPlayerModel(children[incomingSlot].task, (s16)(incomingSlot + player * 2), player,
                              selection->selection, position->x / 4096, position->y / 4096 - 180, position->z);
        configureMovement2D(movement, position->x, position->y, 0x4B000, 0x4B000, 0);

        movement = (Movement2D *)&gTaskMemoryPool[children[outgoingSlot].task->resource16];
        reloadTaskPlayerModel(children[outgoingSlot].task, (s16)(outgoingSlot + player * 2), player,
                              selection->previousSelection, position->x / 4096, movement->y / 4096, position->z);
        configureMovement2D(movement, position->x, position->y + 0xB4000, 0x4B000, 0x4B000, 0);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
        setCurrentTaskCallback(updateRegisteredPlayerModels);
    } else {
        setCurrentTaskCallback(updateRegisteredPlayerModels);
        entry->command = 0;
        entry->flags = 0;
    }
}

void transitionRegisteredPlayerModelsUp(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    s16 player;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    u16 playerIndex;
    s32 command;

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
    selection = &gMenuSelectionStates[(s16)gCurrentTask->data18];
    playerIndex = gCurrentTask->data18;
    command = entry->command;
    if (command == 1) {
        s16 incomingSlot;
        s32 outgoingSlot;
        Movement2D *movement;
        if (gCurrentTask->data1C == 0) {
            outgoingSlot = 0;
            incomingSlot = 1;
            gCurrentTask->data1C = command;
        } else {
            outgoingSlot = 1;
            incomingSlot = 0;
            gCurrentTask->data1C = 0;
        }
        movement = (Movement2D *)&gTaskMemoryPool[children[incomingSlot].task->resource16];
        player = playerIndex;
        reloadTaskPlayerModel(children[incomingSlot].task, (s16)(incomingSlot + player * 2), player,
                              selection->selection, position->x / 4096, position->y / 4096 + 180, position->z);
        configureMovement2D(movement, position->x, position->y, 0x4B000, 0x4B000, 0);

        movement = (Movement2D *)&gTaskMemoryPool[children[outgoingSlot].task->resource16];
        reloadTaskPlayerModel(children[outgoingSlot].task, (s16)(outgoingSlot + player * 2), player,
                              selection->previousSelection, position->x / 4096, movement->y / 4096, position->z);
        configureMovement2D(movement, position->x, position->y - 0xB4000, 0x4B000, 0x4B000, 0);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
        setCurrentTaskCallback(updateRegisteredPlayerModels);
    } else {
        setCurrentTaskCallback(updateRegisteredPlayerModels);
        entry->command = 0;
        entry->flags = 0;
    }
}

void updateRegisteredChildBlinkTransition(void) {
    Movement2D *position;
    RegisteredTaskEntry *children;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    selection = &gMenuSelectionStates[(s16)gCurrentTask->data18];
    if (entry->command == 1) {
        if (selection->blinkTimer == 0) {
            initMovement2D((Movement2D *)&gTaskMemoryPool[children[gCurrentTask->data1C].task->resource16], position->x, position->y);
            entry->command = 0;
            entry->flags = 0;
        } else if (selection->blinkTimer > 0) {
            if (selection->blinkTimer % 2 == 0) {
                initMovement2D((Movement2D *)&gTaskMemoryPool[children[gCurrentTask->data1C].task->resource16], position->x, position->y);
            } else {
                initMovement2D((Movement2D *)&gTaskMemoryPool[children[gCurrentTask->data1C].task->resource16], position->x + 0x140000, position->y);
            }
            selection->blinkTimer--;
            entry->flags = 1;
        }
    } else {
        setCurrentTaskCallback(updateRegisteredPlayerModels);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
    }
}

void transitionRegisteredPlayerModelTexture(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    MenuModelSelectionPair *texture;
    GameTask *task;
    s16 playerIndex;
    s32 outgoingSlot;
    s32 incomingSlot;
    RECT *bounds;
    RECT *incomingBounds;
    s16 height;
    s16 i;

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
    task = gCurrentTask;
    playerIndex = task->data18;
    selection = &gMenuSelectionStates[playerIndex];
    texture = &gMenuPlayerTextureSelections[playerIndex];

    if (task->data1C == 0) {
        outgoingSlot = 0;
        incomingSlot = 1;
    } else {
        outgoingSlot = 1;
        incomingSlot = 0;
    }
    bounds = gMenuModelDrawAreas;
    incomingBounds = &bounds[incomingSlot];
    height = incomingBounds->h;

    switch (entry->command) {
    case 1: {
        RECT *outgoingBounds = &bounds[outgoingSlot];
        s16 width = 160;
        s16 top = 60;
        s32 player;
        u8 (*rows)[15];
        u8 *textures;
        outgoingBounds->x = width;
        outgoingBounds->y = top;
        outgoingBounds->w = width;
        outgoingBounds->h = top * 2;
        incomingBounds->x = width;
        incomingBounds->y = top;
        incomingBounds->w = width;
        incomingBounds->h = 0;
        player = playerIndex;
        rows = gPlayerResourceTextureSelections.entries;
        textures = rows[player];
        textures[selection->selection] =
            gMenuPlayerTextureIds[texture->selection];
        reloadTaskPlayerModel(children[incomingSlot].task, (s16)(incomingSlot + playerIndex * 2),
                              playerIndex, selection->selection,
                              position->x / 4096, position->y / 4096, position->z);
        playSoundEffect(1, 0x102, 0x20);
        entry->command++;
        break;
    }
    case 2: {
        height += 4;
        if (height <= 120) {
            RECT *outgoingBounds = &bounds[outgoingSlot];
            s16 width = 160;
            outgoingBounds->x = width;
            outgoingBounds->y = height + 60;
            outgoingBounds->w = width;
            outgoingBounds->h = 120 - height;
            incomingBounds->x = width;
            incomingBounds->y = 60;
            incomingBounds->w = width;
            incomingBounds->h = height;
            if ((u16)(height - 9) < 103) {
                for (i = 0; i < 6; i++) {
                    drawAnimatedSprite16(rand() % 5, i * 9 - (s16)(height - 82) + height / 4, height - 68,
                                         10, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
                    drawAnimatedSprite16(rand() % 5, i * 9 - (s16)(height - 82) + height / 4 + 3, height - 71,
                                         10, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
                }
            }
        } else {
            entry->command++;
        }
        break;
    }
    case 3:
        if (gCurrentTask->data1C == 0) {
            gCurrentTask->data1C = 1;
        } else {
            gCurrentTask->data1C = 0;
        }
        initMovement2D((Movement2D *)&gTaskMemoryPool[children[outgoingSlot].task->resource16],
                       position->x + 0x140000, position->y);
        gMenuModelDrawAreas[outgoingSlot].x = 0;
        gMenuModelDrawAreas[outgoingSlot].y = 0;
        gMenuModelDrawAreas[outgoingSlot].w = -1;
        gMenuModelDrawAreas[outgoingSlot].h = -1;
        gMenuModelDrawAreas[incomingSlot].x = 0;
        gMenuModelDrawAreas[incomingSlot].y = 0;
        gMenuModelDrawAreas[incomingSlot].w = -1;
        gMenuModelDrawAreas[incomingSlot].h = -1;
        entry->command = 0;
        entry->flags = 0;
        texture->previousSelection = texture->selection;
        setCurrentTaskCallback(updateRegisteredPlayerModels);
        break;
    default:
        setCurrentTaskCallback(updateRegisteredPlayerModels);
        entry->command = 0;
        entry->flags = 0;
        break;
    }
}

void drawTaskOffsetSpriteB8(void) {
    Movement2D *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(resource->x / 4096 + 8, resource->y / 4096 - 22,
                      0xB8, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

void initializeDualResourceTaskAndResetByteState(void) {
    gCurrentTask->data18 = 1;
    gPlayerGroupedSelectionStates[0] = 0;
    gPlayerGroupedSelectionStates[1] = 0;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updatePlayerGroupedSelectionMenu);
    gCurrentTask->unk10 |= 2;
}

void updatePlayerGroupedSelectionMenu(void) {
    RegisteredPanelSlot *slots;

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        createPlayerGroupedSelectionTask(0, 1);
        createPlayerGroupedSelectionTask(1, 11);
        createPaletteEntryCopyTask(816, 240, 15, 0, 21);
        createPaletteEntryCopyTask(816, 241, 15, 0, 31);
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if (gPlayerGroupedSelectionStates[0] == 4 || gPlayerGroupedSelectionStates[1] == 4) {
            gCurrentTask->data18 = 40;
        } else if (gPlayerGroupedSelectionStates[0] == 6 || gPlayerGroupedSelectionStates[1] == 6) {
            if (gMainMenuSelectionIndex == 0 && (s16)getGameOptionFlag0() != 0) {
                gCurrentTaskParent->data18 = 29;
            } else {
                gCurrentTaskParent->data18 = 23;
                beginMode2SequenceAndFadeMusic();
            }
            gCurrentTask->data18 = 9;
        }
        break;
    case 40:
        if (gPlayerGroupedSelectionStates[0] == 4 && gPlayerGroupedSelectionStates[1] == 4) {
            gPlayerGroupedSelectionStates[0] = 5;
            gPlayerGroupedSelectionStates[1] = 5;
            gCurrentTask->data18 = 41;
        }
        break;
    case 41:
        if (gPlayerGroupedSelectionStates[0] == 6 || gPlayerGroupedSelectionStates[1] == 6) {
            switch (gMainMenuSelectionIndex) {
            case 0:
            case 2:
                gCurrentTaskParent->data18 = 7;
                break;
            case 1:
                gCurrentTaskParent->data18 = 20;
                break;
            }
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            slots[2].command = 99;
            slots[3].command = 99;
            gCurrentTask->data18 = 99;
        }
        break;
    case 9:
        if (gCurrentTaskParent->data18 != 23 || isTaskStateTransitionBelowMaximum(0) == 0) {
            gCurrentTask->data18 = 99;
        }
        break;
    default:
        queueMenuResourceReads((s16)gCurrentTaskParent->data18);
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void createPlayerGroupedSelectionTask(s32 player, s32 index) {
    MenuSelectionState *selection;
    RegisteredPanelSlot *slot;
    RECT *bounds;
    createChildTaskWithReserve(updatePlayerGroupedSelectionTask);
    gCreatedTask->data18 = 1;
    gCreatedTask->data1A = getPlayerRecordFlagMode(player + 10);
    gCreatedTask->data1C = player;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    selection = &gMenuSelectionStates[(s16)player];
    adjustGroupedIndexForMode((s16)gCreatedTask->data1A, &((PlayerSelectionRecord *)gRacePlayers)[(s16)player].selectionBytes[3]);
    selection->selection = getGroupedIndexBase(((PlayerSelectionRecord *)gRacePlayers)[(s16)player].selectionBytes[3]);
    if (selection->selection == 9) {
        selection->selection += getSaveRecordFlagGroupIndex(player + 10, 0);
    }
    selection->previousSelection = selection->selection;
    gMenuModelDrawAreas[0].x = 25;
    gMenuModelDrawAreas[0].y = 32;
    gMenuModelDrawAreas[0].w = 270;
    gMenuModelDrawAreas[0].h = 87;
    gMenuModelDrawAreas[1].x = 25;
    bounds = &gMenuModelDrawAreas[1];
    bounds->y = 136;
    bounds->w = 270;
    bounds->h = 87;
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(gCurrentTask->resource16, index);
    }
    slot->command = 0;
    slot->flags = 1;
    slot->task = gCreatedTask;
}

void updatePlayerGroupedSelectionTask(void) {
    CourseSelectionResource *selection;
    RegisteredPanelSlot *panels;
    PlayerGroupedSelectionStatus *parentStatus;
    RegisteredPanelSlot *parentSlot;
    MenuSelectionState *groupedSelection;
    GroupedSelectionMenuDescriptor *descriptor;
    s16 player;
    u8 *transitionFlag;
    s32 complete;
    u16 slotIndex;
    RegisteredPanelSlot *slots;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parentStatus = (PlayerGroupedSelectionStatus *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        slotIndex = gCurrentTask->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        parentSlot = &slots[slotIndex];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        slotIndex = gCurrentTask->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        parentSlot = &slots[slotIndex];
    }

    player = gCurrentTask->data1C;
    if ((gPlayerGroupedSelectionStates[0] == 4 || gPlayerGroupedSelectionStates[1] == 4) &&
        (gPlayerGroupedSelectionStates[player] < 2 || gPlayerGroupedSelectionStates[player] > 4)) {
        gPlayerGroupedSelectionStates[player] = 4;
        gCurrentTask->data18 = 7;
    }
    groupedSelection = &gMenuSelectionStates[player];

    switch (gCurrentTask->data18) {
    case 1:
        selection->mode = 0;
        if (gRacePlayers[player].unk00A[1] < 9 && (u8)(gRacePlayers[player].unk00A[1] % 3U) == 0) {
            selection->selectedIndex = 0;
        }
        if (gRacePlayers[player].unk00A[1] < 9 && (u8)(gRacePlayers[player].unk00A[1] % 3U) == 1) {
            selection->selectedIndex = 1;
        }
        if (gRacePlayers[player].unk00A[1] < 9 && (u8)(gRacePlayers[player].unk00A[1] % 3U) == 2) {
            selection->selectedIndex = 2;
        }
        if (gRacePlayers[player].unk00A[1] >= 9 && gRacePlayers[player].unk00A[1] < 12) {
            selection->selectedIndex = 3;
        }
        if (gRacePlayers[player].unk00A[1] >= 12) {
            selection->selectedIndex = gRacePlayers[player].unk00A[1] - 12;
        }
        selection->choiceResult = 0;
        selection->previousIndex = selection->selectedIndex;
        if (player == 0) {
            createRegisteredCallbackTask(player, 44, -216, drawTaskIndexPanel, 6, 1);
        } else {
            createRegisteredCallbackTask(player, -316, player * 104 + 24, drawTaskIndexPanel, 6, 1);
        }
        descriptor = &gPlayerGroupedSelectionDescriptors[gCurrentTask->data1A];
        createRegisteredSpacedChildTasks(player == 0 ? 1 : 2, 0xB0, 0x40, (const u8 *)gPlayerGroupedSelectionSprites,
                                         descriptor->mapping, NULL, 0, 0, 20, -308, player * 104 - 72,
                                         descriptor->spacingX, descriptor->spacingY, descriptor->count, 230,
                                         drawSelectedTaskSprite, drawClippedRegisteredTaskGridSprite, 4, 11);
        createRegisteredPanelTask(player, groupedSelection->selection, 284, player * 142 - 60, 544, 21);
        createRegisteredTripletTask(-64, (s16)(player * 104 - 60), 0, 16, gSelectionPanelStatTriplets, descriptor->tripletIndices, 3, 31);
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            panels[2].flags == 0 && panels[3].flags == 0) {
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[0].task->resource14],
                                -0x88000, (player * 104 - 96) * 4096, 0x5A000, 0x1E000, 0);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (panels[0].flags == 0) {
            moveTaskChildrenWithSpacing(0, panels[1].task, -0x80000, (player * 104 - 72) * 4096, 0x1E000, 0x1E000, 1);
            configureRegisteredChildMovement(0, panels[2].task, 0x68000, (player * 142 - 60) * 4096, 0x1E000, 0x1E000, 5);
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        if (panels[1].flags == 0 && panels[2].flags == 0) {
            gPlayerGroupedSelectionStates[player] = 1;
            if (gPlayerGroupedSelectionStates[0] == 1 && gPlayerGroupedSelectionStates[1] == 1) {
                panels[2].command = 1;
                panels[0].command = 20;
                panels[1].command = 20;
                panels[3].command = 20;
                configureColorCycleTask(1, 0, 128, 8, player);
                gPlayerGroupedSelectionTransitionFlags[player] = 0;
                parentSlot->flags = 1;
                gCurrentTask->data18 = 6;
            }
        }
        break;
    case 6:
    case 60:
        complete = isTaskResourceTransitionComplete(player);
        if (complete == 1) {
            transitionFlag = &gPlayerGroupedSelectionTransitionFlags[player];
            if (*transitionFlag == 0) {
                panels[1].command = 21;
                panels[3].command = 22;
                panels[0].command = 22;
                configureColorCycleTask(2, 64, 128, 4, player);
                *transitionFlag = complete;
            }
        }
        if (selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceGroupedIndex(panels[1].task, selection->selectedIndex);
            if (groupedSelection->selection == 9) {
                groupedSelection->selection += getSaveRecordFlagGroupIndex(player + 10, 0);
            }
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else if (selection->previousIndex < selection->selectedIndex) {
                panels[2].command = 11;
            } else {
                panels[2].command = 12;
            }
            selection->previousIndex = selection->selectedIndex;
        } else if (selection->choiceResult != 0 && panels[0].flags == 0) {
            if (gPlayerGroupedSelectionTransitionFlags[player] == 0) {
                panels[2].command = 1;
                panels[1].command = 22;
                panels[3].command = 22;
                panels[0].command = 22;
                gPlayerGroupedSelectionTransitionFlags[player] = 1;
            }
            if (selection->choiceResult < 0) {
                gPlayerGroupedSelectionStates[player] = 4;
                gCurrentTask->data18 = 7;
                parentSlot->flags = 1;
            } else {
                gCurrentTask->data18 = 71;
            }
        }
        break;
    case 71:
        panels[1].command = 10;
        gCurrentTask->data18 = 72;
        break;
    case 72:
        if (panels[1].flags == 0) {
            gCurrentTask->data18 = 30;
        }
        break;
    case 30:
        createChildTaskWithReserve(initializeDualResourceTask);
        gCreatedTask->data18 = 1;
        descriptor = &gPlayerGroupedSelectionDescriptors[gCurrentTask->data1A];
        gCreatedTask->data1A = mapCourseMenuByteIndex(descriptor->mapping, selection->selectedIndex);
        gCreatedTask->data1E = 41;
        gCreatedTask->data1C = player;
        panels[4].task = gCreatedTask;
        panels[4].command = 0;
        panels[4].flags = 1;
        gPlayerGroupedSelectionStates[player] = 2;
        gCurrentTask->data18 = 31;
        break;
    case 31:
        if (gPlayerGroupedSelectionStates[player] != 6) {
            break;
        }
        goto finish;
    case 7:
        if (gPlayerGroupedSelectionStates[player] == 5) {
            moveTaskChildrenWithSpacing(0, panels[1].task, -0x134000, (player * 104 - 72) * 4096, 0x1E000, 0x1E000, -1000);
            panels[1].command = 23;
            configureRegisteredChildMovement(0, panels[2].task, 0x11C000, (player * 142 - 60) * 4096, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, panels[2].task, 0x11C000, (player * 142 - 60) * 4096, 0x1E000, 0, -1000);
            panels[3].command = 20;
            panels[0].command = 20;
            configureColorCycleTask(2, 0, 128, 8, player);
            gPlayerGroupedSelectionTransitionFlags[player] = 0;
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
    case 9:
        complete = isTaskResourceTransitionComplete(player);
        if (complete == 1) {
            transitionFlag = &gPlayerGroupedSelectionTransitionFlags[player];
            if (*transitionFlag == 0) {
                initMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14],
                               -0x13C000, (player * 104 - 96) * 4096);
                panels[0].command = 23;
                *transitionFlag = complete;
            }
            if (panels[1].flags == 0 && panels[2].flags == 0) {
                Movement2D *movement = (Movement2D *)&gTaskMemoryPool[panels[0].task->resource14];
                if (player == 0) {
                    configureMovement2D(movement, 0x2C000, -0xD8000, 0x5A000, 0x1E000, -1000);
                } else {
                    configureMovement2D(movement, -0x13C000, (player * 104 + 24) * 4096, 0x5A000, 0x1E000, -1000);
                }
                gCurrentTask->data18 = 10;
            }
        }
        break;
    case 10:
        if (panels[0].flags == 0) {
finish:
            gCurrentTask->data18 = 99;
        }
        break;
    case 61:
        break;
    default:
        parentSlot->flags = 0;
        gPlayerGroupedSelectionStates[player] = 6;
        deleteCurrentTaskSubtree();
        return;
    }

    parentStatus[player].selection.mode = selection->mode;
    parentStatus[player].selection.selectedIndex = selection->selectedIndex;
    parentStatus[player].selection.previousIndex = selection->previousIndex;
    parentStatus[player].selection.choiceResult = selection->choiceResult;
}

void initializeDualResourceTask(void) {
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updatePlayerCourseVariantSelectionTask);
}

void updatePlayerCourseVariantSelectionTask(void) {
    CourseSelectionResource *parentStatus;
    RegisteredPanelSlot *panels;
    CourseSelectionResource *selection;
    RegisteredPanelSlot *children;
    RegisteredPanelSlot *parentSlot;
    RegisteredPanelSlot *slots;
    u16 slotIndex;
    MenuSelectionState *groupedSelection;
    s16 player;
    RecordFlagMenuEntries entries;
    RecordFlagMenuEntries *menuEntries;
    s16 mode;
    Movement2D *movement;

    if (gCurrentTaskParent->resource14 == 0xFFFF) {
        while (1) {}
    }
    parentStatus = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if (gCurrentTaskParent->resource16 == 0xFFFF) {
        while (1) {}
    }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = gCurrentTask->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        parentSlot = &slots[slotIndex];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = gCurrentTask->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        parentSlot = &slots[slotIndex];
    }

    player = gCurrentTask->data1C;
    groupedSelection = &gMenuSelectionStates[player];
    if ((gPlayerGroupedSelectionStates[0] == 4 || gPlayerGroupedSelectionStates[1] == 4) &&
        gPlayerGroupedSelectionStates[player] != 4) {
        gPlayerGroupedSelectionStates[player] = 4;
        gCurrentTask->data18 = 19;
    }

    switch (gCurrentTask->data18) {
    case 1:
        moveTaskChildrenWithSpacing(2, panels[1].task, -0x134000, (player * 104 - 72) * 4096,
                                    0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (panels[1].flags == 0) {
            panels[1].command = 23;
            configureTaskGridMovement(1, panels[1].task, -0x80000, (player * 104 - 72) * 4096,
                                      0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3: {
        if (panels[1].flags != 0) {
            break;
        }
        mode = player == 0 ? 1 : 2;
        menuEntries = &entries;
        buildRecordFlagMenuEntries(player + 10, gCurrentTaskParent->data1A, parentStatus->selectedIndex,
                                   0, 14, menuEntries);
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)gPlayerGroupedSelectionSprites,
                                         entries.entries, NULL, 0, 0, 20, -444, player * 104 - 72,
                                         menuEntries->unk_04, menuEntries->unk_05, menuEntries->count, 237,
                                         drawSelectedTaskSprite, drawClippedRegisteredTaskGridSprite, 5, 1);
        selection->mode = 0;
        selection->selectedIndex = mapGroupedValueToTableIndex(groupedSelection->selection);
        selection->selectedIndex = findIndexedHalfwordRecord(gPlayerGroupedSelectionSprites,
                                                            entries.entries, menuEntries->count,
                                                            selection->selectedIndex);
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        gCurrentTask->data18 = 4;
        break;
    }
    case 4:
        if (children[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, children[0].task, -0x7C000, (player * 104 - 72) * 4096, 0, 0);
            configureVerticalTaskGridMovement(0, children[0].task, -0x7C000, (player * 104 - 58) * 4096,
                                              0x8000, 0x8000, 2);
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        if (children[0].flags == 0) {
            children[0].command = 21;
            gCurrentTask->data18 = 6;
        }
        break;
    case 6:
    case 7: {
        s32 complete = isTaskResourceTransitionComplete(player);
        if (complete == 1) {
            if (gPlayerGroupedSelectionTransitionFlags[player] == 0) {
                children[0].command = 21;
                configureColorCycleTask(2, 64, 128, 4, player);
                gPlayerGroupedSelectionTransitionFlags[player] = complete;
            }
        }
        if (panels[2].flags == 0) {
            if (selection->choiceResult > 0) {
                panels[2].command = 3;
            }
            if (selection->choiceResult != 0) {
                gCurrentTask->data18 = 8;
            }
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(
                gCurrentTask->data1A, children[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else {
                panels[2].command = 13;
            }
            selection->previousIndex = selection->selectedIndex;
        } else if ((u32)(gCurrentTask->data18 - 6) >= 2) {
            u8 *transitionFlag = &gPlayerGroupedSelectionTransitionFlags[player];
            if (*transitionFlag == 0) {
                children[0].command = 22;
                *transitionFlag = 1;
            }
        }
        break;
    }
    case 8:
        if (panels[2].flags == 0 && selection->choiceResult != 0 && children[0].flags == 0) {
            if (selection->choiceResult < 0) {
                gCurrentTask->data18 = 30;
            } else {
                gRacePlayers[player].unk00A[1] = getTaskResourceAdjustedGroupedIndex(
                    gCurrentTask->data1A, children[0].task, selection->selectedIndex);
                parentSlot->flags = 0;
                gPlayerGroupedSelectionStates[player] = 3;
                gCurrentTask->data18 = 9;
            }
        }
        break;
    case 30:
        configureTaskGridMovement(0, children[0].task, -0x7C000, (player * 104 - 72) * 4096,
                                  0x1E000, 0x1E000, 0);
        children[0].command = 23;
        gCurrentTask->data18 = 31;
        break;
    case 31:
        if (children[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, children[0].task, -0x130000, (player * 104 - 72) * 4096, 0, 0);
            moveTaskChildrenWithSpacing(1, panels[1].task, -0x80000, (player * 104 - 72) * 4096, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 32;
        }
        break;
    case 32:
        if (panels[1].flags == 0) {
            moveTaskChildrenWithSpacing(2, panels[1].task, -0x80000, (player * 104 - 72) * 4096,
                                        0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 33;
        }
        break;
    case 33:
        if (panels[1].flags == 0) {
            panels[1].command = 21;
            gCurrentTask->data18 = 34;
        }
        break;
    case 34:
        selection = parentStatus;
        selection->choiceResult = 0;
        gCurrentTaskParent->data18 = 60;
        gPlayerGroupedSelectionStates[player] = 1;
        goto deleteTask;
    case 9:
        createRegisteredStaticTextPanelTask(-76, player * 104 - 74, -56, player * 104 - 62,
                                            gPlayerCourseVariantConfirmationText, -1, 3, 11);
        if (gPlayerGroupedSelectionStates[0] != 3 || gPlayerGroupedSelectionStates[1] != 3) {
            if (!(gControllerPressedButtons[player] & 0x40)) {
                break;
            }
            children[0].command = 21;
            panels[2].command = 1;
            selection->choiceResult = 0;
            gCurrentTask->data18 = 7;
            gControllerPressedButtons[player] = 0;
            gPlayerGroupedSelectionStates[player] = 2;
            children[1].command = 99;
            playSoundEffect(0, 0, 32);
        } else {
            parentSlot->command = 20;
            gCurrentTask->data18 = 18;
        }
        break;
    case 18:
        if (--parentSlot->command <= 0) {
            gCurrentTask->data18 = 19;
        }
        break;
    case 10:
    case 19:
        if (gPlayerGroupedSelectionStates[player] == 3 || gPlayerGroupedSelectionStates[player] == 5) {
            moveTaskChildrenWithSpacing(0, children[0].task, -0x130000, (player * 104 - 58) * 4096,
                                        0x1E000, 0x1E000, -1000);
            children[0].command = 23;
            children[1].command = 99;
            moveTaskChildrenWithSpacing(0, panels[1].task, -0x134000, (player * 104 - 72) * 4096,
                                        0x1E000, 0x1E000, -1000);
            configureRegisteredChildMovement(0, panels[2].task, 0x11C000, (player * 142 - 60) * 4096, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, panels[2].task, 0x11C000, (player * 142 - 60) * 4096, 0x1E000, 0, -1000);
            panels[3].command = 20;
            panels[0].command = 20;
            configureColorCycleTask(2, 0, 128, 8, player);
            gPlayerGroupedSelectionTransitionFlags[player] = 0;
            gCurrentTask->data18 = 11;
        }
        break;
    case 11: {
        s32 complete = isTaskResourceTransitionComplete(player);
        if (complete == 1) {
            u8 *transitionFlag = &gPlayerGroupedSelectionTransitionFlags[player];
            if (*transitionFlag == 0) {
                initMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14],
                               -0x13C000, (player * 104 - 96) * 4096);
                panels[0].command = 23;
                children[0].command = 23;
                *transitionFlag = complete;
            }
            if (children[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0) {
                movement = (Movement2D *)&gTaskMemoryPool[panels[0].task->resource14];
                if (player == 0) {
                    configureMovement2D(movement, 0x2C000, -0xD8000, 0x5A000, 0x1E000, -1000);
                } else {
                    configureMovement2D(movement, -0x13C000, (player * 104 + 24) * 4096, 0x5A000, 0x1E000, -1000);
                }
                gCurrentTask->data18 = 12;
            }
        }
        break;
    }
    case 12:
        if (panels[0].flags == 0) {
            gCurrentTask->data18 = 13;
        }
        break;
    case 13:
        gPlayerGroupedSelectionStates[player] = 6;
        parentSlot->flags = 0;
        /* fall through */
    default:
deleteTask:
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawClippedRegisteredTaskGridSprite(void) {
    Movement2D *position;
    ByteTriplet *color;
    ClippedGridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    s16 row;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    color = (ByteTriplet *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parent = (ClippedGridSpriteParent *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            row = (parent->mode & 1) ^ 1;
            if (parent->alternateSprites == 1) {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)(gCurrentTask->data1A + parent->count));
            } else {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)gCurrentTask->data1A);
            }
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            addDrawAreaPacket(0, 0, 320, 240, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            addDrawAreaPacket(25, row * 104 + 40, 270, 94,
                &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            entry->flags = 0;
        }
    }
}

void drawTaskIndexPanel(void) {
    Movement2D *resource;
    s16 index;
    TexturedQuadListEntry *list;

    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    index = gCurrentTask->data18;
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(resource->x / 4096 + 8, resource->y / 4096 + 80,
                      0x133, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(resource->x / 4096 + 24, resource->y / 4096 + 80,
                      0x31, 2, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawDecimalNumber8x8(resource->x / 4096 + 54, resource->y / 4096 + 4,
                  1, 0, index + 1, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    if (index == 0) list = taskIndexPanelQuadsPalette0;
    else list = taskIndexPanelQuadsPalette1;
    drawTexturedQuadList(resource->x / 4096, resource->y / 4096, list,
                        -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

s16 mapGroupedValueToTableIndex(s16 value) {
    s16 result;

    if (value < 9) {
        result = value / 3 + 231;
    } else if ((u16)(value - 9) < 3) {
        result = value + 225;
    } else if (value >= 12) {
        result = value + 219;
    }
    return result;
}

void createPaletteEntryCopyTask(s32 x, s32 y, s32 destination, s32 source, s32 index) {
    PaletteEntryResource *resource;
    PaletteRotationSlot *slot;

    createChildTaskWithReserve(updatePaletteEntryTask);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (PaletteEntryResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->rect.w = 16;
    resource->rect.x = x;
    resource->rect.y = y;
    resource->rect.h = 1;
    StoreImage(&resource->rect, (u32 *)resource->original);
    resource->destination = destination;
    resource->source = source;

    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(gCurrentTask->resource16, index);
    }
    slot->state = 1;
    slot->unk06 = 0;
    slot->task = gCreatedTask;
}

void updatePaletteEntryTask(void) {
    PaletteEntryResource *resource;
    PaletteRotationSlot *slot;
    s32 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteEntryResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    switch (slot->state) {
    case 1:
        if (++gCurrentTask->data18 >= 2) {
            for (i = 0; i < 16; i++) resource->modified[i] = resource->original[i];
            resource->modified[resource->destination] = resource->original[resource->source];
            LoadImage(&resource->rect, (u32 *)resource->modified);
            slot->state = 0;
        }
        break;
    case 99:
        LoadImage(&resource->rect, (u32 *)resource->original);
        slot->task = 0;
        slot->unk06 = 0;
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 isValue16(s32 value) {
    return value == 16;
}

s16 hasStateValueChanged(s16 index) {
    return gMemoryCardSlotHistory.current[index] != gMemoryCardSlotHistory.previous[index];
}

void updateMemoryCardSlotTask(void) {
    switch (gCurrentTask->data18) {
    case 0:
        resetMemoryCardTaskState();
        if (gMemoryCardSlotHistory.channel == 0) {
            gMemoryCardSlotHistory.activeIndex = isValue16(gMemoryCardSlotHistory.channel = 16);
        } else {
            gMemoryCardSlotHistory.activeIndex = isValue16(gMemoryCardSlotHistory.channel = 0);
        }
        setMemoryCardChannel(gMemoryCardSlotHistory.channel);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (gMemoryCardTaskState != 7) {
            gMemoryCardSlotHistory.unk_16[gMemoryCardSlotHistory.activeIndex] = 1;
        } else {
            gMemoryCardSlotHistory.unk_16[gMemoryCardSlotHistory.activeIndex] = 0;
        }
        switch (gMemoryCardTaskState) {
        case 0:
            if (gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] != gMemoryCardTaskState) {
                gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                    gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
            }
            if (gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] == 2) {
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 3;
                gCurrentTask->data18 = 0;
            }
            break;
        case 2:
        case 3:
            if (gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] != gMemoryCardTaskState) {
                gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                    gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
            }
            break;
        case 6:
            gMemoryCardTaskState = 5;
            /* Fall through. */
        case 5:
            if (gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] == 3) {
                gCurrentTask->data18 = 0;
            } else if (gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] == 1) {
                gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] = 0;
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 2;
                startMemoryCardReadTask();
                gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                    gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
                return;
            } else {
                if (gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] != gMemoryCardTaskState) {
                    gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                        gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
                    gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
                }
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 1;
                gCurrentTask->data18 = 0;
            }
            break;
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            if (gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] != gMemoryCardTaskState) {
                gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                    gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
            }
            gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 1;
            gCurrentTask->data18 = 0;
            break;
        }
        if ((s16)startMemoryCardReadStatusTask(MEMORY_CARD_SLOT_BUFFER_BASE +
                                              gMemoryCardSlotHistory.activeIndex * MEMORY_CARD_SLOT_BUFFER_STRIDE) == 1) {
            gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 0;
        }
        break;
    }
}

void startClearedStateValueHistoryTask(void) {
    s16 i;

    createChildTaskWithReserve(updateMemoryCardSlotTask);
    gCreatedTask->data18 = 0;
    gMemoryCardSlotHistory.channel = 16;
    gMemoryCardSlotHistory.activeIndex = 1;
    for (i = 0; i < 2; i++) {
        gMemoryCardSlotHistory.current[i] = 0;
        gMemoryCardSlotHistory.previous[i] = 0;
        gMemoryCardSlotHistory.unk_0E[i] = 0;
        gMemoryCardSlotHistory.unk_12[i] = 0;
        gMemoryCardSlotHistory.unk_16[i] = 0;
    }
}

void createSaveRecordSlotPanelTask(s16 slot, s32 x, s32 y, s16 depth, s32 index);

void initializeSaveRecordTask(void) {
    s16 i;

    gCurrentTask->data18 = 0;
    gCurrentTaskParent->data1C++;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializeSaveRecordDefaults(gPlayerSaveRecords);
    initializeSaveRecordDefaults(&gPlayerSaveRecords[1]);
    copyGlobalFlagState(&gPlayerSaveRecords[0], &gMergedSaveRecord);
    mergeSaveRecordFlags(&gPlayerSaveRecords[1], &gMergedSaveRecord);
    gWorkingSaveRecords[0] = &SAVE_RECORD_WORKSPACE[0];
    gWorkingSaveRecords[1] = &SAVE_RECORD_WORKSPACE[1];
    initializeSaveRecordDefaults(gWorkingSaveRecords[0]);
    initializeSaveRecordDefaults(gWorkingSaveRecords[1]);
    for (i = 0; i < 15; i++) {
        gPlayerResourceTextureSelections.entries[0][i] = gDefaultPlayerResourceTextureSelections[i];
        gPlayerResourceTextureSelections.entries[1][i] = gDefaultPlayerResourceTextureSelections[i];
    }
    startClearedStateValueHistoryTask();
    setCurrentTaskCallback(updateSaveRecordTask);
}

void updateSaveRecordTask(void) {
    SaveRecordTaskStatus *status;
    RegisteredPanelSlot *panels;
    s16 firstResult;
    s16 secondResult;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    status = (SaveRecordTaskStatus *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        status->first.flags = 0;
        status->secondResult = 0;
        uploadBufferedTexturePair();
        createSaveRecordSlotPanelTask(0, -136, -96, 6, 1);
        createSaveRecordSlotPanelTask(1, -136, 8, 6, 11);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (panels[0].flags || panels[1].flags) {
            break;
        }
        markSecondaryTaskAndClearReference();
        panels[0].command = 3;
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if (panels[0].flags) {
            break;
        }
        if (status->first.flags < 0) {
            goto finish_panels;
        }
        panels[1].command = 3;
        gCurrentTask->data18 = 4;
        break;
    case 4:
        if (panels[1].flags) {
            break;
        }
        if (status->secondResult >= 0) {
            if (panels[0].task->data18 != 26) {
                panels[0].command = 15;
            }
            if (panels[1].task->data18 != 26) {
                panels[1].command = 15;
            }
        }
    finish_panels:
        gCurrentTask->data18 = 5;
        break;
    case 5:
        if ((firstResult = status->first.flags) < 0 ||
            (secondResult = status->secondResult) < 0) {
            gCurrentTaskParent->data18 = 3;
            resetUnlockNotificationStateForMode2();
        } else {
            if (firstResult == 0 || secondResult == 0 || panels[0].flags || panels[1].flags) {
                break;
            }
            if (playerCountAbsolute == 1 && gSaveRecordSavedOrLoaded == 0) {
                gCurrentTaskParent->data18 = 30;
            } else {
                gCurrentTaskParent->data18 = 5;
                queueMenuResourceReads(5);
            }
            applyStereoSoundOption();
        }
        gCurrentTask->data18 = 6;
        break;
    case 6:
        panels[0].command = 27;
        panels[1].command = 27;
        gCurrentTask->data18 = 7;
        break;
    case 7:
        if (panels[0].flags || panels[1].flags) {
            break;
        }
        if (gCurrentTaskParent->data18 == 30) {
            configureTaskStateTransition(2, 0, 30);
            gCurrentTask->data18 = 8;
            break;
        }
        goto finish_transition;
    case 8:
        if (playerCountAbsolute == 2) {
            gCurrentTaskParent->data18 = 5;
            queueMenuResourceReads(5);
        } else {
            if (isTaskStateTransitionBelowMaximum(0) != 0) {
                break;
            }
            setMusicSequenceEnabled(0);
            setTaskBackgroundMode(0, 0, 0);
        }
    finish_transition:
        gCurrentTask->data18 = 9;
        break;
    case 9:
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void resetUnlockNotificationStateForMode2(void) {
    if (playerCountAbsolute == 2) {
        gUnlockNotificationFlags = 0;
        gSaveRecordSavedOrLoaded = 0;
    }
}

void createSaveRecordSlotPanelTask(s16 slot, s32 x, s32 y, s16 depth, s32 index) {
    RegisteredPanelSlot *panel;

    createChildTaskWithReserve(updateSaveRecordLoadPanelTask);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1A = slot;
    gCreatedTask->data1C = depth;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[gCreatedTask->resource14], (s16)x * 4096, (s16)y * 4096);
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) {
            while (1) {}
        }
        panel = getPanelSlot(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) {
            while (1) {}
        }
        panel = getPanelSlot(gCurrentTask->resource16, index);
    }
    panel->command = 0;
    panel->flags = 1;
    panel->task = gCreatedTask;
}

void updateSaveRecordLoadPanelTask(void) {
    Movement2D *position;
    SavePanelPosition *menu;
    SavePanelResources *resources;
    RegisteredPanelSlot *parentPanel;
    CourseSelectionResource *selection;
    PlayerGroupedSelectionStatus *selections;
    s16 player, x, y, result, offset;
    s32 accept;
    GameTask *task;
    u16 countdownStatus;
    GameTask *recordTask;
    s32 color;
    u8 padding[0x10];
    SaveRecord *card;
    u16 slotIndex;
    RegisteredPanelSlot *slots;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    menu = (SavePanelPosition *)position;
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    resources = (SavePanelResources *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gCurrentTaskParent->resource14 == 0xFFFF) {
        while (1) {}
    }
    selections = (PlayerGroupedSelectionStatus *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = gCurrentTask->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        parentPanel = &slots[slotIndex];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = gCurrentTask->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        parentPanel = &slots[slotIndex];
    }
    player = gCurrentTask->data1A;
    selection = &selections[player].selection;
    color = 2;
    if (player == 0)
        color = 1;
    x = position->x / 4096 + 60;
    y = position->y / 4096 + 22;
    if (parentPanel->command > 0) {
        gCurrentTask->data18 = parentPanel->command;
        parentPanel->command = 0;
    }

    switch (gCurrentTask->data18) {
    case 0:
        x = position->x / 4096;
        y = position->y / 4096;
        if (player == 0)
            y -= 180;
        else
            y += 180;
        createRegisteredCallbackTask(player, x, y, drawSaveRecordProgressPanel, gCurrentTask->data1C, 1);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (resources->panels[0].flags)
            break;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[resources->panels[0].task->resource14],
                            position->x, position->y, 0x1E000, 0x1E000, 0);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (resources->panels[0].flags)
            break;
    case 14:
    case 26:
        parentPanel->flags = 0;
        break;
    case 3:
        if (player >= playerCountAbsolute) {
            selection->choiceResult = 1;
            gCurrentTask->data18 = 26;
            break;
        }
        selection->choiceResult = 0;
        parentPanel->flags = 1;
        gCurrentTask->data18 = 4;
        break;
    case 4:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_801306EC, -1, color,
                                              1, gSaveRecordConfirmationSprites, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 5;
        break;
    case 5:
        accept = 1;
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == accept) {
            selection->choiceResult = -1;
            gCurrentTask->data18 = 14;
        } else if (result == 2) {
            gCurrentTask->data18 = 30;
        } else if (result == 3) {
            goto resetRecord;
        } else {
            goto showCardOptions;
        }
        break;
    case 30: {
        s32 otherPlayer = player != 0;
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardLoadSlotChoiceText, -1,
                                              (otherPlayer << 8) | color, 1, gMemoryCardSlotChoiceSprites,
                                              gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 31;
        break;
    }
    case 31:
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == 1) {
        showCardOptions:
            gCurrentTask->data18 = 4;
            break;
        } else if (result == 2) {
            menu->cardSlot = 0;
            goto inspectCard;
        } else if (result == 3) {
            goto selectSecondCard;
        }
        gCurrentTask->data18 = 30;
        break;
    selectSecondCard:
        menu->cardSlot = 1;
        goto inspectCard;
    case 33:
        switch (gMemoryCardSlotHistory.current[menu->cardSlot]) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 6:
            break;
        case 3:
            createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardAccessInProgressText, menu->cardSlot, gCurrentTask->data1C - 3, 11);
            break;
        case 5:
            gCurrentTask->data18 = 6;
            break;
        case 7:
            gMemoryCardSlotHistory.previous[menu->cardSlot] = gMemoryCardSlotHistory.current[menu->cardSlot];
            gCurrentTask->data18 = 34;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            gCurrentTask->data18 = 36;
            break;
        case 12:
            gMemoryCardSlotHistory.previous[menu->cardSlot] = gMemoryCardSlotHistory.current[menu->cardSlot];
            gCurrentTask->data18 = 42;
            break;
        default:
            break;
        }
        break;
    case 6:
        gMemoryCardSlotHistory.unk_12[menu->cardSlot] = 1;
        createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardAccessInProgressText, menu->cardSlot, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 7;
    case 7:
        if (gMemoryCardSlotHistory.current[menu->cardSlot] == 7)
            goto inspectCard;
        if (gMemoryCardSlotHistory.unk_0E[menu->cardSlot] != 2)
            break;
        gCurrentTask->data18 = 8;
    case 8:
        createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardLoadInProgressText, -1, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 9;
    case 9:
        result = gMemoryCardSlotHistory.current[menu->cardSlot];
        if (result == 3)
            break;
        if (result == 0)
            gCurrentTask->data18 = 13;
        else
            gCurrentTask->data18 = 10;
        break;
    case 10:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardOperationFailedText, -1, color,
                                              1, NULL, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 11;
        break;
    case 11:
        if (resources->panels[1].flags)
            gCurrentTask->data18 = 40;
        break;
    case 13:
        card = (SaveRecord *)(MEMORY_CARD_SLOT_BUFFER_BASE + menu->cardSlot * MEMORY_CARD_SLOT_BUFFER_STRIDE);
        *gWorkingSaveRecords[player] = *card;
        recordTask = gCurrentTask;
        resources->panels[1].command = 99;
        goto showRecord;
    case 34:
        resources->panels[0].command = 23;
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardMissingRetryText, menu->cardSlot, color,
                                              1, gSaveRecordConfirmationSprites, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 35;
        break;
    case 35:
        if ((u32)((u16)gMemoryCardSlotHistory.current[menu->cardSlot] - 5) < 2) {
            gCurrentTask->data18 = 3;
            break;
        }
        accept = 1;
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == accept)
            goto retryUnformattedCard;
        if (result == 2)
            goto resetRecord;
    retryUnformattedCard:
        gCurrentTask->data18 = 30;
        break;
    case 36:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardLoadErrorRetryText, menu->cardSlot, color,
                                              1, gSaveRecordConfirmationSprites, gCurrentTask->data1C - 3, 11);
        resources->panels[0].command = 23;
        gCurrentTask->data18 = 37;
        break;
    case 37:
        if (gMemoryCardSlotHistory.current[menu->cardSlot] == 7)
            goto inspectCard;
        accept = 1;
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == accept)
            goto retryUnreadableCard;
        if (result == 2)
            goto resetRecord;
    retryUnreadableCard:
        gCurrentTask->data18 = 30;
        break;
    case 42:
        card = (SaveRecord *)(MEMORY_CARD_SLOT_BUFFER_BASE + menu->cardSlot * MEMORY_CARD_SLOT_BUFFER_STRIDE);
        if (calculateByteSum((const u8 *)card, sizeof(*card) - sizeof(card->checksum)) == card->checksum) {
            *gWorkingSaveRecords[player] = *card;
        } else {
            card++;
            if (calculateByteSum((const u8 *)card, sizeof(*card) - sizeof(card->checksum)) != card->checksum) {
                goto corruptCard;
            }
            *gWorkingSaveRecords[player] = *card;
        }
        recordTask = gCurrentTask;
    showRecord:
        recordTask->data18 = 15;
        break;
    corruptCard:
        gCurrentTask->data18 = 38;
        break;
    case 38:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardCorruptDataText, menu->cardSlot, color,
                                              1, NULL, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 39;
        break;
    case 39:
        if (hasStateValueChanged(menu->cardSlot))
            goto inspectCard;
        if (resources->panels[1].flags == 0)
            break;
    case 40:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardLoadRetryConfirmationText, -1, color,
                                              1, gSaveRecordConfirmationSprites, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 41;
        break;
    case 41:
        accept = 1;
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == accept)
            goto retryCardSelection;
        if (result != 2)
            goto retryCardSelection;
    resetRecord:
        selection->choiceResult = accept;
        gCurrentTask->data18 = 22;
        break;
    retryCardSelection:
        gCurrentTask->data18 = 30;
        break;
    case 15:
        parentPanel->flags = 1;
        resources->panels[0].command = 20;
        resources->panels[1].command = 99;
        configureColorCycleTask(1, 0, 128, 8, player);
        gPlayerGroupedSelectionTransitionFlags[player] = 0;
        gCurrentTask->data18 = 16;
        break;
    case 16:
        if (isTaskResourceTransitionComplete(player) != 1)
            break;
        resources->panels[0].command = 22;
    case 17:
        configureColorCycleTask(2, 64, 128, 4, player);
        createRegisteredCallbackTask(player, position->x / 4096, position->y / 4096,
                                     drawSaveRecordPushAnyButtonPrompt, gCurrentTask->data1C - 3, 21);
        gCurrentTask->data18 = 18;
        break;
    case 18:
        if (resources->panels[2].flags == 0) {
            resources->panels[2].command = 21;
            gCurrentTask->data18 = 19;
        }
        break;
    case 19:
        if (gMemoryCardSlotHistory.current[menu->cardSlot] == 7) {
            resources->panels[2].command = 99;
            gCurrentTask->data18 = 33;
            break;
        }
        if (gControllerPressedButtons[player]) {
            selection->choiceResult = 1;
            resources->panels[2].command = 99;
            gCurrentTask->data18 = 20;
        }
        break;
    case 20:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_80130780, -1, color,
                                              1, gSaveRecordConfirmationSprites, gCurrentTask->data1C - 3, 11);
        gCurrentTask->data18 = 21;
        break;
    case 21:
        if (gMemoryCardSlotHistory.current[menu->cardSlot] == 7) {
        inspectCard:
            gCurrentTask->data18 = 33;
            break;
        }
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == 1)
            goto drawRecord;
        if (result == 2) {
            gPlayerSaveRecords[player] = *gWorkingSaveRecords[player];
            copySaveRecordAndMergeFlags();
            applyLoadedSaveRecordSettings(&gPlayerSaveRecords[player], player);
            gSaveRecordSavedOrLoaded = 1;
            gCurrentTask->data18 = 24;
        } else if (result == 3) {
            task = gCurrentTask;
            resources->panels[0].command = 23;
            task->data18 = 30;
        } else {
        drawRecord:
            resources->panels[1].command = 99;
            gCurrentTask->data18 = 17;
        }
        break;
    case 22:
        copyGlobalFlagState(&gPlayerSaveRecords[player], gWorkingSaveRecords[player]);
        gUnlockNotificationFlags = 0;
        resources->panels[0].command = 20;
        configureColorCycleTask(1, 0, 128, 8, player);
        gPlayerGroupedSelectionTransitionFlags[player] = 0;
        gCurrentTask->data18 = 23;
        break;
    case 23:
        if (isTaskResourceTransitionComplete(player) == 1) {
            resources->panels[0].command = 22;
            gCurrentTask->data18 = 26;
        }
        break;
    case 24:
        resources->countdown = createCountdownTask(15);
        gCurrentTask->data18 = 25;
        break;
    case 25:
        countdownStatus = resources->countdown->data1A;
        if (countdownStatus != 1)
            break;
        resources->countdown->data18 = countdownStatus;
        gCurrentTask->data18 = 26;
        break;
    case 27:
        offset = 180;
        if (player == 0)
            offset = -180;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[resources->panels[0].task->resource14],
                            position->x, position->y + offset * 4096, 0x1E000, 0x1E000, -1000);
        resources->panels[1].command = 99;
        resources->panels[2].command = 99;
        resources->panels[3].command = 99;
        resources->panels[4].command = 99;
        resources->command2C = 99;
        parentPanel->flags = 1;
        gCurrentTask->data18 = 28;
        break;
    case 28:
        if (resources->panels[0].flags == 0)
            gCurrentTask->data18 = 29;
        break;
    default:
        parentPanel->task = NULL;
        parentPanel->flags = 0;
        deleteCurrentTaskSubtree();
        break;
    }
    return;
}

void drawSaveRecordProgressPanel(void) {
    Movement2D *position;
    s16 x, y;
    s16 slot;
    s16 i;
    s16 tier, sprite, palette;
    u8 r, g, b;
    TexturedQuadListEntry *list;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    slot = gCurrentTask->data18;
    if (slot < playerCountAbsolute) {
        if (gCurrentTask->data1A != 0) {
            if (gCurrentTask->data1A == 1 || gCurrentTask->data1A == 2) {
                copyColorCycleTaskColor(slot, &gMenuColorCycleColor);
                r = gMenuColorCycleColor.r;
                g = gMenuColorCycleColor.g;
                b = gMenuColorCycleColor.b;
            } else {
                b = g = r = 128;
            }
            setSpriteColor(r, g, b);
            for (i = 0; i < 8; i++) {
                tier = getStateMaskTier(slot, i);
                if (tier == 4) {
                    sprite = 0x34;
                    palette = 12;
                } else if (tier == 3) {
                    sprite = 0x10A;
                    palette = 9;
                } else if (tier == 2) {
                    sprite = 0x109;
                    palette = 10;
                } else if (tier == 1) {
                    sprite = 0x108;
                    palette = 11;
                } else {
                    sprite = -1;
                    palette = 0;
                }
                if (sprite >= 0) {
                    drawSpriteByIndexWithBlend(i * 32 + (s16)(x + 9), y + 54, sprite, palette, -1,
                                  &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                }
                drawSpriteByIndexWithBlend(i * 32 + (s16)(x + 8), y + 60, 0x70 + i * 3, 0, -1,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
            for (i = 8; i < 10; i++) {
                tier = getStateMaskTier(slot, i);
                if (tier == 4) {
                    sprite = 0x34;
                    palette = 12;
                } else if (tier == 3) {
                    sprite = 0x10A;
                    palette = 9;
                } else if (tier == 2) {
                    sprite = 0x109;
                    palette = 10;
                } else if (tier == 1) {
                    sprite = 0x108;
                    palette = 11;
                } else {
                    sprite = -1;
                    palette = 0;
                }
                if (sprite >= 0) {
                    drawSpriteByIndexWithBlend(x + i * 32 - 55, y + 13, sprite, palette, -1,
                                  &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                }
                if (gWorkingSaveRecords[slot]->flags & (1u << (i - 8))) {
                    drawSpriteByIndexWithBlend((i - 8) * 32 + (s16)(x + 200), y + 19,
                                  0x70 + gCharacterMenuCharacterIndices[i] * 3, 0, -1,
                                  &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
                }
            }
            drawSpriteByIndexWithBlend(x + 72, y + 19, 0x5A, 3, -1,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawDecimalNumber16(x + 72, y + 30, 6, 0, gWorkingSaveRecords[slot]->money,
                                8, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawWrappedGlyphText16x16((s16)(x + 168), (s16)(y + 30), 1, gSaveRecordProgressCurrencyLabel, 8, -1,
                                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    } else {
        setSpriteColor(128, 128, 128);
        drawSpriteByIndexWithBlend(x + 64, y + 32, 0xB4, 1, -1,
                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }

    if (slot < playerCountAbsolute) {
        setSpriteColor(128, 128, 128);
        setTexturedQuadColor(128, 128, 128);
    } else {
        setSpriteColor(64, 64, 64);
        setTexturedQuadColor(64, 64, 64);
    }
    drawDecimalNumber8x8(x + 54, y + 4, 1, 0, slot + 1, 0, -1,
                         &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    if (slot == 0) {
        list = taskIndexPanelQuadsPalette0;
    } else {
        list = taskIndexPanelQuadsPalette1;
    }
    drawTexturedQuadList(x, y, list, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

u16 gSaveRecordProgressCurrencyLabel[2]
    __attribute__((section(".data.save_record_progress_currency_label"), aligned(4))) = {0x11, 0};

void drawSaveRecordPushAnyButtonPrompt(void) {
    Movement2D *position;
    s32 x, y;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    drawWrappedGlyphText8x16((s16)(x + 134), (s16)(y - 6), 20, gSaveRecordPushAnyButtonText, 0, -1,
                             &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    setSpriteColor(0, 0, 0);
    drawWrappedGlyphText8x16((s16)(x + 135), (s16)(y - 5), 20, gSaveRecordPushAnyButtonText, 0, -1,
                             &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawWrappedGlyphText8x16((s16)(x + 136), (s16)(y - 5), 20, gSaveRecordPushAnyButtonText, 0, -1,
                             &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}
