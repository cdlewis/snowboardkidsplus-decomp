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

typedef struct MenuModelSelectionPair {
    s16 selection;
    s16 previousSelection;
} MenuModelSelectionPair;

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

extern s16 D_8013AFCE __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));
extern s16 D_8013B104 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));
extern u8 D_8013B3A4 __attribute__((section(".data")));
extern u8 D_8013B3A5 __attribute__((section(".data")));
extern u8 D_8013B3A6 __attribute__((section(".data")));

extern u16 D_8013AFD0 __attribute__((section(".data")));

extern s16 D_8013AF3E __attribute__((section(".data")));

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

IndexedHalfwordRecord gRecordFlagMenuSpriteRecords[]
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

extern u8 D_8013B420[], D_8013B424[], D_8013B42C[];
extern u8 D_8013B430[], D_8013B438[];
extern u8 D_8013B43C[], D_8013B440[], D_8013B448[], D_8013B44C[];
extern u8 D_8013B450[], D_8013B454[];

GroupedSelectionMenuDescriptor gGroupedCourseMenuDescriptors[]
    __attribute__((section(".data.grouped_course_menu_descriptors"))) = {
    {D_8013B420, 0, 24, 4, 0, D_8013B43C, NULL, D_8013B450},
    {D_8013B424, 0, 20, 5, 0, D_8013B440, NULL, D_8013B454},
    {D_8013B42C, 0, 24, 4, 0, D_8013B43C, NULL, D_8013B450},
    {D_8013B434, 0, 20, 3, 0, D_8013B44C, NULL, NULL},
    {D_8013B438, 0, 16, 3, 0, D_8013B448, NULL, NULL},
    {D_8013B430, 0, 24, 4, 0, D_8013B43C, NULL, D_8013B450},
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
    {D_8013B420, 0, 16, 3, 0, NULL, NULL, D_8013B450},
    {D_8013B424, 0, 14, 4, 0, NULL, NULL, D_8013B454},
    {D_8013B42C, 0, 16, 3, 0, NULL, NULL, D_8013B450},
    {D_8013B434, 0, 14, 3, 0, NULL, NULL, NULL},
    {D_8013B438, 0, 14, 3, 0, NULL, NULL, NULL},
    {D_8013B430, 0, 16, 3, 0, NULL, NULL, D_8013B450},
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

StateValueHistory D_8013BE30 __attribute__((section(".bss.state_value_history")));

extern const u16 *D_8012FC2C[10];
extern MenuModelSelectionPair D_8013B468[];
extern u8 gPlayerGroupedSelectionStates[2];
void drawClippedRegisteredTaskGridSprite(void);
extern u16 gSaveRecordProgressCurrencyLabel[2];
extern char gSelectionPrizeCurrencyLabel[] __attribute__((section(".data")));
extern const char gRegisteredTripletUnavailableLabel[];

void updateRegisteredPlayerModelTask(void);
void transitionRegisteredPlayerModels(void);
void transitionRegisteredPlayerModelsUp(void);
void func_80083C3C(void);
void createPaletteEntryCopyTask(s32 x, s32 y, s32 destination, s32 source, s32 index);
extern const u16 D_80130308[], D_80130348[], D_801303CC[], D_8013052C[];
extern const u16 D_8013066C[], D_801306EC[], D_80130780[];

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

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        if (D_8013AFCD == 1) {
            mode = 1;
        } else {
            mode = 3;
        }
        group = &D_8012FD10[D_8013AE88->data1A];
        selection->mode = 0;
        selectedIndex = findIdentityMappedIndexOrZero(group, D_8013B104);
        selection->selectedIndex = selectedIndex;
        selection->previousIndex = selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)D_8012F798,
                                         group->mapping, NULL, 0, 0, 20,
                                         -312, -96, group->spacingX, group->spacingY,
                                         group->count, 149, drawSelectedTaskSprite,
                                         drawRegisteredSelectionItemStatus, 4, 1);
        createRegisteredCallbackTask(0, 172, -72, drawSelectionItemDetails, 4, 11);
        createRegisteredTexturePairTask(172, -72, group->textureMapping,
                                        group->count, selection->selectedIndex, 1, 5, 21);
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            selectionMenuFadeStartedGp = 0;
            D_8013AE88->data18 = 5;
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
                GameTask *parent = D_8013AE98;
                D_8013B104 = 0;
                parent->data18 = 6;
                D_8013AE88->data18 = 7;
            } else {
                D_8013AE88->data18 = 50;
            }
        }
        break;
    case 50: {
        GameTask *task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 51;
        break;
    }
    case 51:
        if (entries[0].flags == 0) {
            D_8013AE88->data18 = 52;
        }
        break;
    case 52: {
        GameTask *task = D_8013AE88;
        group = &D_8012FD10[task->data1A];
        switch (group->mapping[selection->selectedIndex]) {
        case 10: {
            GameTask *parent = D_8013AE98;
            D_8013B104 = 0;
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
            selectedIndex = D_8013B104 = mapMenuByteIndex(group->mapping, selection->selectedIndex);
            course = D_8012D46C[selectedIndex];
            courseParameter = D_80123D80[course].unk02;
            D_8013AFCE = course;
            D_8013AE88->data18 = 30;
            D_8013AFD0 = courseParameter;
            break;
        }
        break;
    }
    case 30:
        createChildTaskWithReserve(initializeModeDependentDualResourceTask);
        D_8013AE8C->data18 = 1;
        D_8013AE88->data18 = 31;
        break;
    case 32: {
        GameTask *parent = D_8013AE98;
        GameTask *task = D_8013AE88;
        parent->data18 = 8;
        task->data18 = 99;
        queueMenuResourceReads((s16)parent->data18);
        break;
    }
    case 40:
        moveTaskChildrenWithSpacing(0, entries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
        D_8013AE88->data18 = 41;
        break;
    case 41:
        if (entries[0].flags == 0) {
            GameTask *task = D_8013AE88;
            entries[0].command = 99;
            task->data18 = 42;
        }
        break;
    case 42:
        if (entries[0].task == NULL) {
            D_8013AE88->data18 = 2;
        }
        break;
    case 7:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            queueMenuResourceReads((s16)D_8013AE98->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x138000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        if (entries[0].flags == 0 && entries[1].flags == 0 && entries[2].flags == 0) {
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        D_8013AE88->data18 = 99;
        break;
    case 31:
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 getCourseSelectionGroupIndex(void) {
    s16 result;

    if (D_8013AFCD == 1) {
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
        u32 mask = gMergedSaveRecord.combinedMask;

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
    D_8013AE88->data18 = 1;
    if (D_8013AFCD == 1) {
        D_8013AE88->data1A = 1;
    } else {
        D_8013AE88->data1A = 3;
    }
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
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

    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = D_8013AE98;
    parentSelection = (CourseSelectionResource *)&gTaskMemoryPool[parent->resource14];
    if (parent->resource16 == 0xFFFF) {
        for (;;) {}
    }
    parentEntries = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource16];
    task = D_8013AE88;
    if (task->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[task->resource14];
    if (task->resource16 == 0xFFFF) {
        for (;;) {}
    }
    task = D_8013AE88;
    entries = (RegisteredPanelSlot *)&gTaskMemoryPool[task->resource16];
    selection->selectedIndex = parentSelection->selectedIndex;

    switch (task->data18) {
    case 1:
        moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x138000, -0x60000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (parentEntries[0].flags == 0) {
            parentEntries[0].command = 23;
            configureTaskGridMovement(1, parentEntries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (parentEntries[0].flags == 0) {
            createRegisteredCallbackTask(0, -308, -76, drawSelectionPrizeAndPlayerCounters, 3, 1);
            createRegisteredTextPanelTask(-320, -24, D_8012FC2C, NULL, 4, 11);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if (entries[0].flags == 0 || entries[1].flags == 0) {
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[0].task->resource14],
                                -0x80000, -0x4C000, 0x1E000, 0x1E000, 0);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8C000, -0x18000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        if (entries[0].flags == 0 || entries[1].flags == 0) {
            createPaletteRotationTask(816, 245, 1, 12, 3, 5, 91);
            createInputTimedSpriteTask(-40, 44, D_80123840, 2, D_8013AE88->data1A, 176, 64, 3, 21);
            entries[0].command = 20;
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = D_8013AE88;
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
                createRegisteredMovingPanelTask(D_8013AE88->data1A, panelValues, -120, 32, 0, 16, -120, 64, 5, 31);
                D_8013AE88->data18 = 8;
            } else {
                goto dismiss;
            }
        }
        break;
    case 8:
        if (entries[3].flags >= 0) {
            if (entries[3].flags == 0) {
                D_8013AE88->data18 = 9;
            } else {
            dismiss:
                D_8013AE88->data18 = 30;
            }
        }
        break;
    case 30:
        entries[9].command = 99;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[0].task->resource14],
                            -0x134000, -0x4C000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                            -0x140000, -0x18000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 31;
        break;
    case 31:
        if (entries[0].flags == 0) {
            moveTaskChildrenWithSpacing(1, parentEntries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 32;
        }
        break;
    case 32:
        if (parentEntries[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x84000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 33;
        }
        break;
    case 33:
        if (parentEntries[0].flags == 0) {
            parentEntries[0].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = D_8013AE88;
            selectionMenuFadeStartedGp = 0;
            transitionTask->data18 = 34;
        }
        break;
    case 34:
        resumeTask = D_8013AE98;
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
            D_8013AE88->data18 = 10;
        }
        break;
    case 10:
        if (entries[0].flags == 0 && entries[1].flags == 0 && parentEntries[0].flags == 0 &&
            parentEntries[1].flags == 0 && parentEntries[2].flags == 0) {
            D_8013AE88->data18 = 11;
        }
        break;
    case 11:
        resumeTask = D_8013AE98;
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
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (PaletteRotationResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
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
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(D_8013AE88->resource16, index);
    }
    slot->state = 1;
    slot->unk06 = 0;
    slot->task = D_8013AE8C;
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
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteRotationResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource16, D_8013AE88->data1E);
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    color = (ByteTriplet *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = (GroupedIndexResource *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->values != 0) {
        sprite = D_8012F798;
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            sprite = getMappedGridSpriteEntry(sprite, parent->values, (s16)D_8013AE88->data1A);
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096, sprite->sprite,
                sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            setSpriteColor(128, 128, 128);
            character = gRacePlayers[0].unk009;
            index = mapCourseMenuByteIndex(parent->values, (s16)D_8013AE88->data1A);
            if ((u16)index < 10) {
                if (D_8013AFCD == 1) {
                    if ((gMergedSaveRecord.masks[character] & (1U << (index * 2))) > 0) {
                        if (!(gMergedSaveRecord.masks[character] & (2U << (index * 2)))) {
                            goto draw_base_status;
                        }
                        drawSpriteByIndexWithBlend(position->x / 4096 - 17, position->y / 4096 - 1,
                            index + 70, 9, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                    } else if (gMergedSaveRecord.masks[character] & (2U << (index * 2))) {
                        drawSpriteByIndexWithBlend(position->x / 4096 - 17, position->y / 4096 - 1,
                            index + 70, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                    } else {
draw_base_status:
                        if ((gMergedSaveRecord.masks[character] & (1U << (index * 2))) > 0) {
                            drawSpriteByIndexWithBlend(position->x / 4096 - 17, position->y / 4096 - 1,
                                index + 70, 11, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                        } else {
                            drawMenuDecimalNumber8x16(position->x / 4096 - 20, position->y / 4096 + 1,
                                2, 0, index + 1, 3, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                        }
                    }
                } else {
                    drawMenuDecimalNumber8x16(position->x / 4096 - 20, position->y / 4096 + 1,
                        2, 0, index + 1, 3, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (HalfwordPair *)&gTaskMemoryPool[D_8013AE98->resource14];
    if (D_8013AE88->data1A == 0) {
        return;
    }
    drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                 0xB3, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    group = &D_8012FD10[D_8013AE98->data1A];
    index = mapMenuByteIndex(group->mapping, selection->unk_02);
    if ((u16)index >= 10) {
        return;
    }
    stats = &D_8012FCE4[index];
    if (stats->price && stats->rating) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 84, position->y / 4096 + 128,
                     0xBC, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 88, position->y / 4096 + 136,
                             4, 0, stats->price, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 122, position->y / 4096 + 138,
                     0xBF, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 84, position->y / 4096 + 148,
                     0xBE, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawHalfStepSpriteBar(position->x / 4096 + 88, position->y / 4096 + 156,
                              stats->rating, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (D_8013AE88->data1A == 1) {
        addTilePacketWithBlend(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        addTilePacketWithBlend(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void drawSelectionPrizeAndPlayerCounters(void) {
    Movement2D *position;
    s16 row;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        if (D_8013AFCD == 1) {
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                          0xC1, 14, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            for (row = 0; row < 3; row++) {
                s32 *prizes = D_8012FC6C[D_8013AFCE];
                drawDecimalNumber8x8(position->x / 4096 + 48, row * 8 + (s16)(position->y / 4096 + 9),
                                     5, 0, prizes[row], 0, -1,
                                     &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                drawWrappedText8x8(position->x / 4096 + 88, row * 8 + (s16)(position->y / 4096 + 9),
                                   2, gSelectionPrizeCurrencyLabel, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
            drawSpriteByIndexWithBlend(position->x / 4096 + 6, position->y / 4096 + 35,
                          0xC2, 14, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawDecimalNumber8x8(position->x / 4096 + 26, position->y / 4096 + 42,
                                 5, 0, gPlayerSaveRecords[0].bestScore[D_8013AFCE],
                                 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawWrappedText8x8(position->x / 4096 + 66, position->y / 4096 + 42,
                               2, gSelectionPrizeCurrencyLabel, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                          0xC0, 14, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawMenuDecimalNumber8x16(position->x / 4096 + 64, position->y / 4096 + 19,
                                      2, 0, gRacePlayers[0].unk02A, 5, -1,
                                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawMenuDecimalNumber8x16(position->x / 4096 + 64, position->y / 4096 + 35,
                                      2, 0, gRacePlayers[1].unk02A, 5, -1,
                                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }
}

char gSelectionPrizeCurrencyLabel[3]
    __attribute__((section(".data.selection_prize_currency_label"), aligned(4))) = "\x82" "f";
u8 D_8013B41F __attribute__((section(".data.selection_prize_currency_label"))) = 0;

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
        table = D_8012FC6C;
        row = table[D_8013AFCE];
        result = row[place];
    } else {
        result = 0;
    }
    return result;
}

void initializeCountedPlayerGroupedSelectionBoundsTask(void) {
    RECT *bounds;
    D_8013AE98->data1C++;
    if (D_8013AFCD == 2) {
        setCurrentTaskCallback(initializeDualResourceTaskAndResetByteState);
    } else {
        D_8013AE88->data18 = 0;
        D_8013AE88->data1A = getPlayerRecordFlagMode(0);
        if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
        D_8013AE88->resource14 = allocateTaskMemoryBlock();
        if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
        D_8013AE88->resource16 = allocateTaskMemoryBlock();
        adjustGroupedIndexForMode(D_8013AE88->data1A, &gRacePlayers[0].unk00A[1]);
        D_801300AC[0].selection = getGroupedIndexBase(gRacePlayers[0].unk00A[1]);
        if (D_801300AC[0].selection == 9) {
            D_801300AC[0].selection += getSaveRecordFlagGroupIndex(0, 0);
        }
        D_801300B8[0].x = 0;
        D_801300B8[0].y = 0;
        D_801300B8[0].w = -1;
        D_801300B8[0].h = -1;
        D_801300B8[1].x = 0;
        bounds = &D_801300B8[1];
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
        D_801300AC[0].previousSelection = D_801300AC[0].selection;
        setCurrentTaskCallback(updatePlayerGroupedCourseSelectionTask);
    }
}

void updatePlayerGroupedCourseSelectionTask(void) {
    CourseSelectionResource *selection;
    RegisteredPanelSlot *panels;
    MenuSelectionState *groupedSelection;
    GroupedSelectionMenuDescriptor *descriptor;
    s32 complete;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];
    groupedSelection = &D_801300AC[0];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        descriptor = &gGroupedCourseMenuDescriptors[D_8013AE88->data1A];
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gRecordFlagMenuSpriteRecords,
                                         descriptor->mapping, NULL, 0, 0, 20, -304, -96,
                                         descriptor->spacingX, descriptor->spacingY, descriptor->count, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-316, 8, D_8012FFD4, descriptor->textMapping, 4, 11);
        createRegisteredPanelTask(0, groupedSelection->selection, 244, 0, 384, 21);
        createRegisteredCallbackTask(0, 172, -72, drawTaskOffsetSpriteB8, 4, 31);
        createRegisteredTripletTask(-296, 48, 0, 16, D_8012FFFC, descriptor->tripletIndices, 3, 41);
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
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x88000, 0x8000, 0x1E000, 0x1E000, 5);
            configureRegisteredChildMovement(0, panels[2].task, 0x40000, 0, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14], -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[4].task->resource14], -0x74000, 0x30000, 0x1E000, 0x1E000, 5);
            D_8013AE88->data18 = 3;
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
            D_8013B41F = 0;
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            panels[3].command = 22;
            panels[4].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
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
            if (D_8013B41F == 0) {
                panels[0].command = 22;
                panels[1].command = 22;
                panels[3].command = 22;
                panels[4].command = 22;
                D_8013B41F = 1;
            }
            if (selection->choiceResult < 0) {
                D_8013AE88->data18 = 14;
            } else {
                descriptor = &gGroupedCourseMenuDescriptors[D_8013AE88->data1A];
                if (descriptor->mapping[selection->selectedIndex] == 0) {
                    selection->choiceResult = -1;
                    D_8013AE88->data18 = 14;
                } else {
                    panels[0].command = 10;
                    D_8013AE88->data18 = 12;
                }
            }
        }
        break;
    case 12:
        panels[0].command = 10;
        D_8013AE88->data18 = 13;
        break;
    case 13:
        if (panels[0].flags == 0) {
            D_8013AE88->data18 = 9;
        }
        break;
    case 14:
        switch (D_8013AFE2) {
        case 0:
            D_8013AE98->data18 = 7;
            break;
        case 2:
            D_8013AE98->data18 = 10;
            break;
        case 1:
            D_8013AE98->data18 = 20;
            break;
        }
        D_8013AE88->data18 = 5;
        break;
    case 9:
        createChildTaskWithReserve(initializeDualResourceTaskAndClearState);
        D_8013AE8C->data18 = 1;
        descriptor = &gGroupedCourseMenuDescriptors[D_8013AE88->data1A];
        D_8013AE8C->data1A = mapCourseMenuByteIndex(descriptor->mapping, selection->selectedIndex);
        D_8013AE88->data18 = 10;
        break;
    case 10:
        break;
    case 11:
        beginMode2SequenceAndFadeMusic();
        D_8013AE98->data18 = 23;
        goto finishExit;
    case 5:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
            queueMenuResourceReads(D_8013AE98->data18);
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            panels[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            configureRegisteredChildMovement(0, panels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, panels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14], 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[4].task->resource14], -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 6;
        }
        break;
    case 6:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            panels[3].flags == 0 && panels[4].flags == 0) {
finishExit:
            D_8013AE88->data18 = 7;
        }
        break;
    case 7:
        if (selection->choiceResult <= 0 || isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013AE88->data18 = 8;
        }
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void initializeDualResourceTaskAndClearState(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
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

    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parentSelection = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE98->resource14];
    if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
    parentPanels = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];
    groupedSelection = &D_801300AC[0];

    switch (D_8013AE88->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (parentPanels[0].flags == 0) {
            parentPanels[0].command = 23;
            parentPanels[1].command = 23;
            parentPanels[4].command = 23;
            configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if (parentPanels[0].flags == 0) {
            entries = &entryStorage;
            buildRecordFlagMenuEntries(0, D_8013AE98->data1A, parentSelection->selectedIndex, 0, 20, entries);
            createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gRecordFlagMenuSpriteRecords,
                                             entries->entries, NULL, 0, 0, 20, -304, -96,
                                             entries->unk_04, entries->unk_05, entries->count, 222,
                                             drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 5, 1);
            createRegisteredTextPanelTask(-136, 8, D_8012FFD4, entries->textIndices, 4, 11);
            selection->mode = 0;
            value = getGroupedIndexOffsetPlus216(groupedSelection->selection);
            selection->selectedIndex = value;
            value = findIndexedHalfwordRecord(gRecordFlagMenuSpriteRecords, entries->entries, entries->count, value);
            selection->selectedIndex = value;
            selection->previousIndex = value;
            selection->choiceResult = 0;
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x74000, -0x60000, 0, 0);
            configureVerticalTaskGridMovement(0, panels[0].task, -0x74000, -0x4C000, 0x8000, 0x8000, 2);
            initMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14], -0x13C000, 0x8000);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if (panels[0].flags == 0 && panels[1].flags == 0) {
            panels[0].command = 20;
            panels[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (parentPanels[2].flags == 0) {
            if (selection->choiceResult > 0) {
                parentPanels[2].command = 3;
            }
            if (selection->choiceResult != 0) {
                D_8013AE88->data18 = 6;
            }
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(D_8013AE88->data1A, panels[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else {
                parentPanels[2].command = 13;
            }
            selection->previousIndex = selection->selectedIndex;
        } else if (D_8013AE88->data18 != 5 && D_8013B41F == 0) {
            panels[0].command = 22;
            panels[1].command = 22;
            D_8013B41F = 1;
        }
        break;
    case 6:
        if (parentPanels[2].flags == 0 && selection->choiceResult != 0 && panels[0].flags == 0) {
            if (selection->choiceResult < 0) {
                D_8013AE88->data18 = 10;
            } else {
                gRacePlayers[0].unk00A[1] = getTaskResourceAdjustedGroupedIndex(D_8013AE88->data1A, panels[0].task, selection->selectedIndex);
                D_8013AE88->data18 = 7;
            }
        }
        break;
    case 10:
        configureTaskGridMovement(0, panels[0].task, -0x74000, -0x60000, 0x1E000, 0x1E000, 0);
        panels[0].command = 23;
        D_8013AE88->data18 = 11;
        break;
    case 11:
        if (panels[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x128000, -0x60000, 0, 0);
            moveTaskChildrenWithSpacing(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 12;
        }
        break;
    case 12:
        if (parentPanels[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 13;
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
            D_8013B41F = 0;
            D_8013AE88->data18 = 14;
        }
        break;
    case 14:
        parentSelection->choiceResult = 0;
        D_8013AE98->data18 = 4;
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
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            parentPanels[0].flags == 0 && parentPanels[1].flags == 0 &&
            parentPanels[2].flags == 0 && parentPanels[3].flags == 0 && parentPanels[4].flags == 0) {
            D_8013AE98->data18 = 11;
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
        out->entries = D_80130094[index];
        out->entries[0] = 8;
        out->count = 1;
        out->textIndices = D_8013B44C;
        out->unk_10 = 0;
        return;
    case 1:
        if (group != 3) {
            goto standardGroups;
        }
        out->entries = D_80130094[index];
        out->count = 0;
        out->textIndices = D_801300A0[index];
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
            out->entries = D_80130094[index];
            out->count = 0;
            out->textIndices = D_801300A0[index];
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
    D_8013AE8C->data18 = mode;
    D_8013AE8C->data1A = value;
    D_8013AE8C->data1C = 0;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14];
    initMovement2D(movement, x << 12, y << 12);
    movement->unk08 = depth;
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource16, index);
    }
    slot->command = 0;
    slot->flags = 1;
    slot->task = D_8013AE8C;
}

void initRegisteredPlayerModels(void) {
    MenuPlayerModelPosition *position;
    s16 index;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    } else {
        position = (MenuPlayerModelPosition *)&gTaskMemoryPool[D_8013AE88->resource14];
        index = D_8013AE88->data18;
        createRegisteredPlayerModelTask(index * 2, index, D_8013AE88->data1A,
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

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = &D_801300AC[(s16)D_8013AE88->data18];
    playerIndex = D_8013AE88->data18;
    alternateSelection = &D_8013B468[(s16)D_8013AE88->data18];

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
        selection->unk04 = 20;
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
            u16 slot = D_8013AE88->data1C;
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
            setCurrentTaskCallback(func_80083C3C);
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
    D_8013AE8C->data1A = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], (s16)x << 12, (s16)y << 12);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    data = (RegisteredTripletData *)&gTaskMemoryPool[D_8013AE8C->resource16];
    data->records = records;
    data->indices = indices;
    data->offsetX = offsetX;
    data->offsetY = offsetY;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTripletEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredTripletEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = D_8013AE8C;
}

void updateRegisteredTripletTask(void) {
    Movement2D *position;
    RegisteredTripletData *data;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    HalfwordPair *selection;
    ByteTriplet *values;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    data = (RegisteredTripletData *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (HalfwordPair *)&gTaskMemoryPool[D_8013AE98->resource14];

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
    D_8013AE88->data1A = 1;
    goto clearCommand;
modeTwo:
    D_8013AE88->data1A = 2;
    goto clearCommand;
modeThree:
    D_8013AE88->data1A = 3;
    goto clearCommand;
hide:
    D_8013AE88->data1A = 0;
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
    if (D_8013AE88->data1A == 0) {
        return;
    }
    if ((u32)(D_8013AE88->data1A - 1) < 2) {
        setSpriteColor(D_8013B3A4, D_8013B3A5, D_8013B3A6);
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
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawSpriteByIndexWithBlend(position->x / 4096 + data->offsetX, position->y / 4096 + data->offsetY,
                  31, 1, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawSpriteByIndexWithBlend(data->offsetX * 2 + position->x / 4096, data->offsetY * 2 + position->y / 4096,
                  32, 1, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);

    if (values->bytes[0] == 255) {
        drawWrappedText8x8(position->x / 4096 + 56, position->y / 4096, 2, gRegisteredTripletUnavailableLabel,
                           6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        drawHalfStepSpriteBar(position->x / 4096 + 56, position->y / 4096, values->bytes[0],
                              5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (values->bytes[1] == 255) {
        drawWrappedText8x8((s16)(position->x / 4096 + 56) + data->offsetX, position->y / 4096 + data->offsetY,
                           2, gRegisteredTripletUnavailableLabel, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        drawHalfStepSpriteBar((s16)(position->x / 4096 + 56) + data->offsetX, position->y / 4096 + data->offsetY,
                              values->bytes[1], 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (values->bytes[2] == 255) {
        drawWrappedText8x8(data->offsetX * 2 + (s16)(position->x / 4096 + 56), data->offsetY * 2 + position->y / 4096,
                           2, gRegisteredTripletUnavailableLabel, 6, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        drawHalfStepSpriteBar(data->offsetX * 2 + (s16)(position->x / 4096 + 56), data->offsetY * 2 + position->y / 4096,
                              values->bytes[2], 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
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
    D_8013AE8C->data18 = selection;
    D_8013AE8C->data1A = modelIndex;
    D_8013AE8C->data1C = slot;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    offsets = DAT_STARTUP_MODELS->offsets;
    archive = (u8 *)DAT_STARTUP_MODELS;
    buffers = DAT_PLAYER_MODEL_BLOCKS;
    i = 0;
    resource = (MenuModelResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    entry = &offsets[D_80123640[D_8013AE8C->data1A]];
    model = buffers[D_8013AE8C->data1C];
    source = (u32 *)(*entry + archive);
    dest = model;
    for (; i < 512; i++) {
        *dest++ = *source++;
    }
    entry = (u32 *)((u32)model | 4);
    GsMapModelingData(entry);
    entry += 2;
    if (D_8013AE8C->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)entry, &gPlayerRootTextureOffsets[D_8013AE8C->data1C]);
    }
    GsLinkObject4(entry, &resource->object, 0);
    GsInitCoordinate2(0, &resource->world);
    GsInitCoordinate2(&resource->world, &resource->local);
    resource->object.coord2 = &resource->local;
    resource->object.attribute = 0;
    textureOffsets = DAT_STARTUP_TEXTURES->offsets;
    entry = &textureOffsets[gPlayerResourceTextureSelections.entries[D_8013AE8C->data18][D_8013AE8C->data1A]];
    uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *entry),
                             gPlayerResourceTexturePositions[D_8013AE8C->data1C]);
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
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource16], x << 12, y << 12);
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        registration = getRegisteredTripletEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        registration = getRegisteredTripletEntry(D_8013AE88->resource16, index);
    }
    registration->command = 0;
    registration->flags = 1;
    registration->task = D_8013AE8C;
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
    entry = &offsets[D_80123640[task->data1A]];
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

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    model = (MenuModelResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    movement = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource16];

    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
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
    if (D_8013AE88->data18 == 0) {
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
    gMenuPlayerModelYaw[D_8013AE88->data18] = model->worldRotation.vy;
    setCoordinateRotation(&model->worldRotation, &model->world);
    setCoordinateRotation(&model->localRotation, &model->local);
    if (model->world.coord.t[2] >= 512) {
        sortLitObjectWithDrawArea(&model->object, D_8013AE88->data18 + 2, &D_801300B8[D_8013AE88->data18]);
    } else {
        u16 index = D_8013AE88->data1C;
        sortLitObjectWithDrawArea(&model->object, index & 1, &D_801300B8[index]);
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
        GsSortObject4(object, &D_800EBFF0[D_8013AF78], (depthIndex & 1) + 1,
                      (u32 *)getScratchAddr(0));
        if (bounds->w >= 0 && bounds->h >= 0) {
            addDrawAreaPacket(bounds->x, bounds->y, bounds->w, bounds->h,
                              &D_800EBFF0[D_8013AF78], litObjectDrawAreaDepths[depthIndex]);
            addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], 14);
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

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = &D_801300AC[(s16)D_8013AE88->data18];
    playerIndex = D_8013AE88->data18;
    command = entry->command;
    if (command == 1) {
        s16 incomingSlot;
        s32 outgoingSlot;
        Movement2D *movement;
        if (D_8013AE88->data1C == 0) {
            outgoingSlot = 0;
            incomingSlot = 1;
            D_8013AE88->data1C = command;
        } else {
            outgoingSlot = 1;
            incomingSlot = 0;
            D_8013AE88->data1C = 0;
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

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    selection = &D_801300AC[(s16)D_8013AE88->data18];
    playerIndex = D_8013AE88->data18;
    command = entry->command;
    if (command == 1) {
        s16 incomingSlot;
        s32 outgoingSlot;
        Movement2D *movement;
        if (D_8013AE88->data1C == 0) {
            outgoingSlot = 0;
            incomingSlot = 1;
            D_8013AE88->data1C = command;
        } else {
            outgoingSlot = 1;
            incomingSlot = 0;
            D_8013AE88->data1C = 0;
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
    SelectionTransitionEntry *selection;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    selection = &((SelectionTransitionEntry *)&D_801300AC)[(s16)D_8013AE88->data18];
    if (entry->command == 1) {
        if (selection->timer == 0) {
            initMovement2D((Movement2D *)&gTaskMemoryPool[children[D_8013AE88->data1C].task->resource16], position->x, position->y);
            entry->command = 0;
            entry->flags = 0;
        } else if (selection->timer > 0) {
            if (selection->timer % 2 == 0) {
                initMovement2D((Movement2D *)&gTaskMemoryPool[children[D_8013AE88->data1C].task->resource16], position->x, position->y);
            } else {
                initMovement2D((Movement2D *)&gTaskMemoryPool[children[D_8013AE88->data1C].task->resource16], position->x + 0x140000, position->y);
            }
            selection->timer--;
            entry->flags = 1;
        }
    } else {
        setCurrentTaskCallback(updateRegisteredPlayerModels);
        entry->command = 0;
        entry->flags = 0;
        selection->previousSelection = selection->selection;
    }
}

/* DECOMP_ATTEMPT: inline 301/390 */
#ifdef NON_MATCHING
extern u8 D_80130084[];

void func_80083C3C(void) {
    MenuPlayerModelPosition *position;
    RegisteredTaskEntry *children;
    RegisteredTaskEntry *entry;
    MenuSelectionState *selection;
    MenuModelSelectionPair *texture;
    GameTask *task;
    s16 playerIndex;
    s32 outgoingSlot;
    s32 incomingSlot;
    RECT *incomingBounds;
    s16 height;
    s16 i;
    s32 x;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (MenuPlayerModelPosition *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    children = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entry = getRegisteredMenuTaskEntry(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    task = D_8013AE88;
    playerIndex = task->data18;
    selection = &D_801300AC[playerIndex];
    texture = &D_8013B468[playerIndex];

    if (task->data1C == 0) {
        outgoingSlot = 0;
        incomingSlot = 1;
    } else {
        outgoingSlot = 1;
        incomingSlot = 0;
    }
    incomingBounds = &D_801300B8[incomingSlot];
    height = incomingBounds->h;

    switch (entry->command) {
    case 1: {
        RECT *outgoingBounds = &D_801300B8[outgoingSlot];
        s16 width = 160;
        s16 top = 60;
        outgoingBounds->y = top;
        outgoingBounds->h = top * 2;
        outgoingBounds->x = width;
        outgoingBounds->w = width;
        incomingBounds->y = top;
        incomingBounds->x = width;
        incomingBounds->w = width;
        incomingBounds->h = 0;
        gPlayerResourceTextureSelections.entries[playerIndex][selection->selection] =
            D_80130084[texture->selection];
        reloadTaskPlayerModel(children[incomingSlot].task, (s16)(incomingSlot + playerIndex * 2),
                              playerIndex, selection->selection,
                              position->x / 4096, position->y / 4096, position->z);
        playSoundEffect(1, 0x102, 0x20);
        entry->command++;
        break;
    }
    case 2: {
        RECT *outgoingBounds = &D_801300B8[outgoingSlot];
        s16 width = 160;
        height += 4;
        if (height <= 120) {
            outgoingBounds->y = height + 60;
            outgoingBounds->h = 120 - height;
            outgoingBounds->w = outgoingBounds->x = width;
            incomingBounds->y = 60;
            incomingBounds->w = incomingBounds->x = width;
            incomingBounds->h = height;
            if ((u16)(height - 9) < 103) {
                i = 0;
                for (;;) {
                    s16 frame = rand() % 5;
                    s16 firstY = height - 68;
                    x = i;
                    x *= 9;
                    x -= height - 82;
                    x += height / 4;
                    drawAnimatedSprite16(frame, x, firstY, 10, &D_800EBFF0[D_8013AF78], 4);
                    {
                        s16 secondFrame = rand() % 5;
                        s16 secondX = x + 3;
                        s16 secondY = height - 71;
                        drawAnimatedSprite16(secondFrame, secondX, secondY, 10, &D_800EBFF0[D_8013AF78], 4);
                    }
                    if (++i >= 6) {
                        break;
                    }
                }
            }
        } else {
            entry->command++;
        }
        break;
    }
    case 3:
        if (D_8013AE88->data1C == 0) {
            D_8013AE88->data1C = 1;
        } else {
            D_8013AE88->data1C = 0;
        }
        initMovement2D((Movement2D *)&gTaskMemoryPool[children[outgoingSlot].task->resource16],
                       position->x + 0x140000, position->y);
        D_801300B8[outgoingSlot].x = 0;
        D_801300B8[outgoingSlot].y = 0;
        D_801300B8[outgoingSlot].w = -1;
        D_801300B8[outgoingSlot].h = -1;
        D_801300B8[incomingSlot].x = 0;
        D_801300B8[incomingSlot].y = 0;
        D_801300B8[incomingSlot].w = -1;
        D_801300B8[incomingSlot].h = -1;
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
#else
INCLUDE_ASM("asm/nonmatchings/menu/course_selection_tasks", func_80083C3C);
#endif

void drawTaskOffsetSpriteB8(void) {
    Movement2D *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        drawSpriteByIndexWithBlend(resource->x / 4096 + 8, resource->y / 4096 - 22,
                      0xB8, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void initializeDualResourceTaskAndResetByteState(void) {
    D_8013AE88->data18 = 1;
    gPlayerGroupedSelectionStates[0] = 0;
    gPlayerGroupedSelectionStates[1] = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updatePlayerGroupedSelectionMenu);
    D_8013AE88->unk10 |= 2;
}

void updatePlayerGroupedSelectionMenu(void) {
    RegisteredPanelSlot *slots;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        createPlayerGroupedSelectionTask(0, 1);
        createPlayerGroupedSelectionTask(1, 11);
        createPaletteEntryCopyTask(816, 240, 15, 0, 21);
        createPaletteEntryCopyTask(816, 241, 15, 0, 31);
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (gPlayerGroupedSelectionStates[0] == 4 || gPlayerGroupedSelectionStates[1] == 4) {
            D_8013AE88->data18 = 40;
        } else if (gPlayerGroupedSelectionStates[0] == 6 || gPlayerGroupedSelectionStates[1] == 6) {
            if (D_8013AFE2 == 0 && (s16)getGameOptionFlag0() != 0) {
                D_8013AE98->data18 = 29;
            } else {
                D_8013AE98->data18 = 23;
                beginMode2SequenceAndFadeMusic();
            }
            D_8013AE88->data18 = 9;
        }
        break;
    case 40:
        if (gPlayerGroupedSelectionStates[0] == 4 && gPlayerGroupedSelectionStates[1] == 4) {
            gPlayerGroupedSelectionStates[0] = 5;
            gPlayerGroupedSelectionStates[1] = 5;
            D_8013AE88->data18 = 41;
        }
        break;
    case 41:
        if (gPlayerGroupedSelectionStates[0] == 6 || gPlayerGroupedSelectionStates[1] == 6) {
            switch (D_8013AFE2) {
            case 0:
            case 2:
                D_8013AE98->data18 = 7;
                break;
            case 1:
                D_8013AE98->data18 = 20;
                break;
            }
            queueMenuResourceReads((s16)D_8013AE98->data18);
            slots[2].command = 99;
            slots[3].command = 99;
            D_8013AE88->data18 = 99;
        }
        break;
    case 9:
        if (D_8013AE98->data18 != 23 || isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013AE88->data18 = 99;
        }
        break;
    default:
        queueMenuResourceReads((s16)D_8013AE98->data18);
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void createPlayerGroupedSelectionTask(s32 player, s32 index) {
    MenuSelectionState *selection;
    RegisteredPanelSlot *slot;
    RECT *bounds;
    createChildTaskWithReserve(updatePlayerGroupedSelectionTask);
    D_8013AE8C->data18 = 1;
    D_8013AE8C->data1A = getPlayerRecordFlagMode(player + 10);
    D_8013AE8C->data1C = player;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    selection = &D_801300AC[(s16)player];
    adjustGroupedIndexForMode((s16)D_8013AE8C->data1A, &((PlayerSelectionRecord *)gRacePlayers)[(s16)player].selectionBytes[3]);
    selection->selection = getGroupedIndexBase(((PlayerSelectionRecord *)gRacePlayers)[(s16)player].selectionBytes[3]);
    if (selection->selection == 9) {
        selection->selection += getSaveRecordFlagGroupIndex(player + 10, 0);
    }
    selection->previousSelection = selection->selection;
    D_801300B8[0].x = 25;
    D_801300B8[0].y = 32;
    D_801300B8[0].w = 270;
    D_801300B8[0].h = 87;
    D_801300B8[1].x = 25;
    bounds = &D_801300B8[1];
    bounds->y = 136;
    bounds->w = 270;
    bounds->h = 87;
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPanelSlot(D_8013AE88->resource16, index);
    }
    slot->command = 0;
    slot->flags = 1;
    slot->task = D_8013AE8C;
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

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parentStatus = (PlayerGroupedSelectionStatus *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        slotIndex = D_8013AE88->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource14];
        parentSlot = &slots[slotIndex];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        slotIndex = D_8013AE88->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource16];
        parentSlot = &slots[slotIndex];
    }

    player = D_8013AE88->data1C;
    if ((gPlayerGroupedSelectionStates[0] == 4 || gPlayerGroupedSelectionStates[1] == 4) &&
        (gPlayerGroupedSelectionStates[player] < 2 || gPlayerGroupedSelectionStates[player] > 4)) {
        gPlayerGroupedSelectionStates[player] = 4;
        D_8013AE88->data18 = 7;
    }
    groupedSelection = &D_801300AC[player];

    switch (D_8013AE88->data18) {
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
        descriptor = &gPlayerGroupedSelectionDescriptors[D_8013AE88->data1A];
        createRegisteredSpacedChildTasks(player == 0 ? 1 : 2, 0xB0, 0x40, (const u8 *)gPlayerGroupedSelectionSprites,
                                         descriptor->mapping, NULL, 0, 0, 20, -308, player * 104 - 72,
                                         descriptor->spacingX, descriptor->spacingY, descriptor->count, 230,
                                         drawSelectedTaskSprite, drawClippedRegisteredTaskGridSprite, 4, 11);
        createRegisteredPanelTask(player, groupedSelection->selection, 284, player * 142 - 60, 544, 21);
        createRegisteredTripletTask(-64, (s16)(player * 104 - 60), 0, 16, D_8012FFFC, descriptor->tripletIndices, 3, 31);
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            panels[2].flags == 0 && panels[3].flags == 0) {
            configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[0].task->resource14],
                                -0x88000, (player * 104 - 96) * 4096, 0x5A000, 0x1E000, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if (panels[0].flags == 0) {
            moveTaskChildrenWithSpacing(0, panels[1].task, -0x80000, (player * 104 - 72) * 4096, 0x1E000, 0x1E000, 1);
            configureRegisteredChildMovement(0, panels[2].task, 0x68000, (player * 142 - 60) * 4096, 0x1E000, 0x1E000, 5);
            D_8013AE88->data18 = 5;
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
                D_8013AE88->data18 = 6;
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
                D_8013AE88->data18 = 7;
                parentSlot->flags = 1;
            } else {
                D_8013AE88->data18 = 71;
            }
        }
        break;
    case 71:
        panels[1].command = 10;
        D_8013AE88->data18 = 72;
        break;
    case 72:
        if (panels[1].flags == 0) {
            D_8013AE88->data18 = 30;
        }
        break;
    case 30:
        createChildTaskWithReserve(initializeDualResourceTask);
        D_8013AE8C->data18 = 1;
        descriptor = &gPlayerGroupedSelectionDescriptors[D_8013AE88->data1A];
        D_8013AE8C->data1A = mapCourseMenuByteIndex(descriptor->mapping, selection->selectedIndex);
        D_8013AE8C->data1E = 41;
        D_8013AE8C->data1C = player;
        panels[4].task = D_8013AE8C;
        panels[4].command = 0;
        panels[4].flags = 1;
        gPlayerGroupedSelectionStates[player] = 2;
        D_8013AE88->data18 = 31;
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
            D_8013AE88->data18 = 8;
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
                D_8013AE88->data18 = 10;
            }
        }
        break;
    case 10:
        if (panels[0].flags == 0) {
finish:
            D_8013AE88->data18 = 99;
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
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
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

    if (D_8013AE98->resource14 == 0xFFFF) {
        while (1) {}
    }
    parentStatus = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE98->resource14];
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {}
    }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    selection = (CourseSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = D_8013AE88->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource14];
        parentSlot = &slots[slotIndex];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = D_8013AE88->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource16];
        parentSlot = &slots[slotIndex];
    }

    player = D_8013AE88->data1C;
    groupedSelection = &D_801300AC[player];
    if ((gPlayerGroupedSelectionStates[0] == 4 || gPlayerGroupedSelectionStates[1] == 4) &&
        gPlayerGroupedSelectionStates[player] != 4) {
        gPlayerGroupedSelectionStates[player] = 4;
        D_8013AE88->data18 = 19;
    }

    switch (D_8013AE88->data18) {
    case 1:
        moveTaskChildrenWithSpacing(2, panels[1].task, -0x134000, (player * 104 - 72) * 4096,
                                    0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (panels[1].flags == 0) {
            panels[1].command = 23;
            configureTaskGridMovement(1, panels[1].task, -0x80000, (player * 104 - 72) * 4096,
                                      0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3: {
        if (panels[1].flags != 0) {
            break;
        }
        mode = player == 0 ? 1 : 2;
        menuEntries = &entries;
        buildRecordFlagMenuEntries(player + 10, D_8013AE98->data1A, parentStatus->selectedIndex,
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
        D_8013AE88->data18 = 4;
        break;
    }
    case 4:
        if (children[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, children[0].task, -0x7C000, (player * 104 - 72) * 4096, 0, 0);
            configureVerticalTaskGridMovement(0, children[0].task, -0x7C000, (player * 104 - 58) * 4096,
                                              0x8000, 0x8000, 2);
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        if (children[0].flags == 0) {
            children[0].command = 21;
            D_8013AE88->data18 = 6;
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
                D_8013AE88->data18 = 8;
            }
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(
                D_8013AE88->data1A, children[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else {
                panels[2].command = 13;
            }
            selection->previousIndex = selection->selectedIndex;
        } else if ((u32)(D_8013AE88->data18 - 6) >= 2) {
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
                D_8013AE88->data18 = 30;
            } else {
                gRacePlayers[player].unk00A[1] = getTaskResourceAdjustedGroupedIndex(
                    D_8013AE88->data1A, children[0].task, selection->selectedIndex);
                parentSlot->flags = 0;
                gPlayerGroupedSelectionStates[player] = 3;
                D_8013AE88->data18 = 9;
            }
        }
        break;
    case 30:
        configureTaskGridMovement(0, children[0].task, -0x7C000, (player * 104 - 72) * 4096,
                                  0x1E000, 0x1E000, 0);
        children[0].command = 23;
        D_8013AE88->data18 = 31;
        break;
    case 31:
        if (children[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, children[0].task, -0x130000, (player * 104 - 72) * 4096, 0, 0);
            moveTaskChildrenWithSpacing(1, panels[1].task, -0x80000, (player * 104 - 72) * 4096, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 32;
        }
        break;
    case 32:
        if (panels[1].flags == 0) {
            moveTaskChildrenWithSpacing(2, panels[1].task, -0x80000, (player * 104 - 72) * 4096,
                                        0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 33;
        }
        break;
    case 33:
        if (panels[1].flags == 0) {
            panels[1].command = 21;
            D_8013AE88->data18 = 34;
        }
        break;
    case 34:
        selection = parentStatus;
        selection->choiceResult = 0;
        D_8013AE98->data18 = 60;
        gPlayerGroupedSelectionStates[player] = 1;
        goto deleteTask;
    case 9:
        createRegisteredStaticTextPanelTask(-76, player * 104 - 74, -56, player * 104 - 62,
                                            gPlayerCourseVariantConfirmationText, -1, 3, 11);
        if (gPlayerGroupedSelectionStates[0] != 3 || gPlayerGroupedSelectionStates[1] != 3) {
            if (!(D_8013AEE8[player] & 0x40)) {
                break;
            }
            children[0].command = 21;
            panels[2].command = 1;
            selection->choiceResult = 0;
            D_8013AE88->data18 = 7;
            D_8013AEE8[player] = 0;
            gPlayerGroupedSelectionStates[player] = 2;
            children[1].command = 99;
            playSoundEffect(0, 0, 32);
        } else {
            parentSlot->command = 20;
            D_8013AE88->data18 = 18;
        }
        break;
    case 18:
        if (--parentSlot->command <= 0) {
            D_8013AE88->data18 = 19;
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
            D_8013AE88->data18 = 11;
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
                D_8013AE88->data18 = 12;
            }
        }
        break;
    }
    case 12:
        if (panels[0].flags == 0) {
            D_8013AE88->data18 = 13;
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

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    color = (ByteTriplet *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (ClippedGridSpriteParent *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            row = (parent->mode & 1) ^ 1;
            if (parent->alternateSprites == 1) {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)D_8013AE88->data1A);
            }
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            addDrawAreaPacket(25, row * 104 + 40, 270, 94,
                &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}

void drawTaskIndexPanel(void) {
    Movement2D *resource;
    s16 index;
    TexturedQuadListEntry *list;

    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    index = D_8013AE88->data18;
    if (D_8013AE88->data1A != 0) {
        drawSpriteByIndexWithBlend(resource->x / 4096 + 8, resource->y / 4096 + 80,
                      0x133, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawSpriteByIndexWithBlend(resource->x / 4096 + 24, resource->y / 4096 + 80,
                      0x31, 2, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawDecimalNumber8x8(resource->x / 4096 + 54, resource->y / 4096 + 4,
                  1, 0, index + 1, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (index == 0) list = taskIndexPanelQuadsPalette0;
    else list = taskIndexPanelQuadsPalette1;
    drawTexturedQuadList(resource->x / 4096, resource->y / 4096, list,
                        -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
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
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (PaletteEntryResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->rect.w = 16;
    resource->rect.x = x;
    resource->rect.y = y;
    resource->rect.h = 1;
    StoreImage(&resource->rect, (u32 *)resource->original);
    resource->destination = destination;
    resource->source = source;

    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteRotationSlot(D_8013AE88->resource16, index);
    }
    slot->state = 1;
    slot->unk06 = 0;
    slot->task = D_8013AE8C;
}

void updatePaletteEntryTask(void) {
    PaletteEntryResource *resource;
    PaletteRotationSlot *slot;
    s32 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteEntryResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        slot = getPaletteEntrySlot(D_8013AE98->resource16, D_8013AE88->data1E);
    }
    switch (slot->state) {
    case 1:
        if (++D_8013AE88->data18 >= 2) {
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
    return D_8013BE30.current[index] != D_8013BE30.previous[index];
}

void updateMemoryCardSlotTask(void) {
    switch (D_8013AE88->data18) {
    case 0:
        resetMemoryCardTaskState();
        if (D_8013BE30.unk_00 == 0) {
            D_8013BE30.activeIndex = isValue16(D_8013BE30.unk_00 = 16);
        } else {
            D_8013BE30.activeIndex = isValue16(D_8013BE30.unk_00 = 0);
        }
        setMemoryCardChannel(D_8013BE30.unk_00);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (D_8013AF3E != 7) {
            D_8013BE30.unk_16[D_8013BE30.activeIndex] = 1;
        } else {
            D_8013BE30.unk_16[D_8013BE30.activeIndex] = 0;
        }
        switch (D_8013AF3E) {
        case 0:
            if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
            }
            if (D_8013BE30.unk_0E[D_8013BE30.activeIndex] == 2) {
                D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 3;
                D_8013AE88->data18 = 0;
            }
            break;
        case 2:
        case 3:
            if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
            }
            break;
        case 6:
            D_8013AF3E = 5;
            /* Fall through. */
        case 5:
            if (D_8013BE30.unk_0E[D_8013BE30.activeIndex] == 3) {
                D_8013AE88->data18 = 0;
            } else if (D_8013BE30.unk_12[D_8013BE30.activeIndex] == 1) {
                D_8013BE30.unk_12[D_8013BE30.activeIndex] = 0;
                D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 2;
                startMemoryCardReadTask();
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
                return;
            } else {
                if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                    D_8013BE30.previous[D_8013BE30.activeIndex] =
                        D_8013BE30.current[D_8013BE30.activeIndex];
                    D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
                }
                D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 1;
                D_8013AE88->data18 = 0;
            }
            break;
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            if (D_8013BE30.current[D_8013BE30.activeIndex] != D_8013AF3E) {
                D_8013BE30.previous[D_8013BE30.activeIndex] =
                    D_8013BE30.current[D_8013BE30.activeIndex];
                D_8013BE30.current[D_8013BE30.activeIndex] = D_8013AF3E;
            }
            D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 1;
            D_8013AE88->data18 = 0;
            break;
        }
        if ((s16)startMemoryCardReadStatusTask(MEMORY_CARD_SLOT_BUFFER_BASE +
                                              D_8013BE30.activeIndex * MEMORY_CARD_SLOT_BUFFER_STRIDE) == 1) {
            D_8013BE30.unk_0E[D_8013BE30.activeIndex] = 0;
        }
        break;
    }
}

void startClearedStateValueHistoryTask(void) {
    s16 i;

    createChildTaskWithReserve(updateMemoryCardSlotTask);
    D_8013AE8C->data18 = 0;
    D_8013BE30.unk_00 = 16;
    D_8013BE30.activeIndex = 1;
    for (i = 0; i < 2; i++) {
        D_8013BE30.current[i] = 0;
        D_8013BE30.previous[i] = 0;
        D_8013BE30.unk_0E[i] = 0;
        D_8013BE30.unk_12[i] = 0;
        D_8013BE30.unk_16[i] = 0;
    }
}

void createSaveRecordSlotPanelTask(s16 slot, s32 x, s32 y, s16 depth, s32 index);

void initializeSaveRecordTask(void) {
    s16 i;

    D_8013AE88->data18 = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
    initializeSaveRecordDefaults(gPlayerSaveRecords);
    initializeSaveRecordDefaults(&gPlayerSaveRecords[1]);
    copyGlobalFlagState(&gPlayerSaveRecords[0], &gMergedSaveRecord);
    mergeSaveRecordFlags(&gPlayerSaveRecords[1], &gMergedSaveRecord);
    D_8013B484[0] = &SAVE_RECORD_WORKSPACE[0];
    D_8013B484[1] = &SAVE_RECORD_WORKSPACE[1];
    initializeSaveRecordDefaults(D_8013B484[0]);
    initializeSaveRecordDefaults(D_8013B484[1]);
    for (i = 0; i < 15; i++) {
        gPlayerResourceTextureSelections.entries[0][i] = D_80123650[i];
        gPlayerResourceTextureSelections.entries[1][i] = D_80123650[i];
    }
    startClearedStateValueHistoryTask();
    setCurrentTaskCallback(updateSaveRecordTask);
}

void updateSaveRecordTask(void) {
    SaveRecordTaskStatus *status;
    RegisteredPanelSlot *panels;
    s16 firstResult;
    s16 secondResult;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    status = (SaveRecordTaskStatus *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        status->first.flags = 0;
        status->secondResult = 0;
        uploadBufferedTexturePair();
        createSaveRecordSlotPanelTask(0, -136, -96, 6, 1);
        createSaveRecordSlotPanelTask(1, -136, 8, 6, 11);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (panels[0].flags || panels[1].flags) {
            break;
        }
        markSecondaryTaskAndClearReference();
        panels[0].command = 3;
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (panels[0].flags) {
            break;
        }
        if (status->first.flags < 0) {
            goto finish_panels;
        }
        panels[1].command = 3;
        D_8013AE88->data18 = 4;
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
        D_8013AE88->data18 = 5;
        break;
    case 5:
        if ((firstResult = status->first.flags) < 0 ||
            (secondResult = status->secondResult) < 0) {
            D_8013AE98->data18 = 3;
            resetUnlockNotificationStateForMode2();
        } else {
            if (firstResult == 0 || secondResult == 0 || panels[0].flags || panels[1].flags) {
                break;
            }
            if (D_8013AFCD == 1 && D_8013B47C == 0) {
                D_8013AE98->data18 = 30;
            } else {
                D_8013AE98->data18 = 5;
                queueMenuResourceReads(5);
            }
            applyStereoSoundOption();
        }
        D_8013AE88->data18 = 6;
        break;
    case 6:
        panels[0].command = 27;
        panels[1].command = 27;
        D_8013AE88->data18 = 7;
        break;
    case 7:
        if (panels[0].flags || panels[1].flags) {
            break;
        }
        if (D_8013AE98->data18 == 30) {
            configureTaskStateTransition(2, 0, 30);
            D_8013AE88->data18 = 8;
            break;
        }
        goto finish_transition;
    case 8:
        if (D_8013AFCD == 2) {
            D_8013AE98->data18 = 5;
            queueMenuResourceReads(5);
        } else {
            if (isTaskStateTransitionBelowMaximum(0) != 0) {
                break;
            }
            setMusicSequenceEnabled(0);
            setTaskBackgroundMode(0, 0, 0);
        }
    finish_transition:
        D_8013AE88->data18 = 9;
        break;
    case 9:
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void resetUnlockNotificationStateForMode2(void) {
    if (D_8013AFCD == 2) {
        gUnlockNotificationFlags = 0;
        D_8013B47C = 0;
    }
}

void createSaveRecordSlotPanelTask(s16 slot, s32 x, s32 y, s16 depth, s32 index) {
    RegisteredPanelSlot *panel;

    createChildTaskWithReserve(updateSaveRecordLoadPanelTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = slot;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], (s16)x * 4096, (s16)y * 4096);
    D_8013AE8C->resource16 = allocateTaskMemoryBlock();
    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        panel = getPanelSlot(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        panel = getPanelSlot(D_8013AE88->resource16, index);
    }
    panel->command = 0;
    panel->flags = 1;
    panel->task = D_8013AE8C;
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    menu = (SavePanelPosition *)position;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    resources = (SavePanelResources *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) {
        while (1) {}
    }
    selections = (PlayerGroupedSelectionStatus *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = D_8013AE88->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource14];
        parentPanel = &slots[slotIndex];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            while (1) {}
        }
        slotIndex = D_8013AE88->data1E / 10U;
        slots = (RegisteredPanelSlot *)&gTaskMemoryPool[D_8013AE98->resource16];
        parentPanel = &slots[slotIndex];
    }
    player = D_8013AE88->data1A;
    selection = &selections[player].selection;
    color = 2;
    if (player == 0)
        color = 1;
    x = position->x / 4096 + 60;
    y = position->y / 4096 + 22;
    if (parentPanel->command > 0) {
        D_8013AE88->data18 = parentPanel->command;
        parentPanel->command = 0;
    }

    switch (D_8013AE88->data18) {
    case 0:
        x = position->x / 4096;
        y = position->y / 4096;
        if (player == 0)
            y -= 180;
        else
            y += 180;
        createRegisteredCallbackTask(player, x, y, drawSaveRecordProgressPanel, D_8013AE88->data1C, 1);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (resources->panels[0].flags)
            break;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[resources->panels[0].task->resource14],
                            position->x, position->y, 0x1E000, 0x1E000, 0);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (resources->panels[0].flags)
            break;
    case 14:
    case 26:
        parentPanel->flags = 0;
        break;
    case 3:
        if (player >= D_8013AFCD) {
            selection->choiceResult = 1;
            D_8013AE88->data18 = 26;
            break;
        }
        selection->choiceResult = 0;
        parentPanel->flags = 1;
        D_8013AE88->data18 = 4;
        break;
    case 4:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_801306EC, -1, color,
                                              1, D_801307AC, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 5;
        break;
    case 5:
        accept = 1;
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == accept) {
            selection->choiceResult = -1;
            D_8013AE88->data18 = 14;
        } else if (result == 2) {
            D_8013AE88->data18 = 30;
        } else if (result == 3) {
            goto resetRecord;
        } else {
            goto showCardOptions;
        }
        break;
    case 30: {
        s32 otherPlayer = player != 0;
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_8013066C, -1,
                                              (otherPlayer << 8) | color, 1, D_80130800,
                                              D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 31;
        break;
    }
    case 31:
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == 1) {
        showCardOptions:
            D_8013AE88->data18 = 4;
            break;
        } else if (result == 2) {
            menu->cardSlot = 0;
            goto inspectCard;
        } else if (result == 3) {
            goto selectSecondCard;
        }
        D_8013AE88->data18 = 30;
        break;
    selectSecondCard:
        menu->cardSlot = 1;
        goto inspectCard;
    case 33:
        switch (D_8013BE30.current[menu->cardSlot]) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 6:
            break;
        case 3:
            createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, D_801302BC, menu->cardSlot, D_8013AE88->data1C - 3, 11);
            break;
        case 5:
            D_8013AE88->data18 = 6;
            break;
        case 7:
            D_8013BE30.previous[menu->cardSlot] = D_8013BE30.current[menu->cardSlot];
            D_8013AE88->data18 = 34;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            D_8013AE88->data18 = 36;
            break;
        case 12:
            D_8013BE30.previous[menu->cardSlot] = D_8013BE30.current[menu->cardSlot];
            D_8013AE88->data18 = 42;
            break;
        default:
            break;
        }
        break;
    case 6:
        D_8013BE30.unk_12[menu->cardSlot] = 1;
        createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, D_801302BC, menu->cardSlot, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 7;
    case 7:
        if (D_8013BE30.current[menu->cardSlot] == 7)
            goto inspectCard;
        if (D_8013BE30.unk_0E[menu->cardSlot] != 2)
            break;
        D_8013AE88->data18 = 8;
    case 8:
        createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, D_8013052C, -1, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 9;
    case 9:
        result = D_8013BE30.current[menu->cardSlot];
        if (result == 3)
            break;
        if (result == 0)
            D_8013AE88->data18 = 13;
        else
            D_8013AE88->data18 = 10;
        break;
    case 10:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_80130770, -1, color,
                                              1, NULL, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 11;
        break;
    case 11:
        if (resources->panels[1].flags)
            D_8013AE88->data18 = 40;
        break;
    case 13:
        card = (SaveRecord *)(MEMORY_CARD_SLOT_BUFFER_BASE + menu->cardSlot * MEMORY_CARD_SLOT_BUFFER_STRIDE);
        *D_8013B484[player] = *card;
        recordTask = D_8013AE88;
        resources->panels[1].command = 99;
        goto showRecord;
    case 34:
        resources->panels[0].command = 23;
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_80130308, menu->cardSlot, color,
                                              1, D_801307AC, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 35;
        break;
    case 35:
        if ((u32)((u16)D_8013BE30.current[menu->cardSlot] - 5) < 2) {
            D_8013AE88->data18 = 3;
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
        D_8013AE88->data18 = 30;
        break;
    case 36:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_80130348, menu->cardSlot, color,
                                              1, D_801307AC, D_8013AE88->data1C - 3, 11);
        resources->panels[0].command = 23;
        D_8013AE88->data18 = 37;
        break;
    case 37:
        if (D_8013BE30.current[menu->cardSlot] == 7)
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
        D_8013AE88->data18 = 30;
        break;
    case 42:
        card = (SaveRecord *)(MEMORY_CARD_SLOT_BUFFER_BASE + menu->cardSlot * MEMORY_CARD_SLOT_BUFFER_STRIDE);
        if (calculateByteSum((const u8 *)card, sizeof(*card) - sizeof(card->checksum)) == card->checksum) {
            *D_8013B484[player] = *card;
        } else {
            card++;
            if (calculateByteSum((const u8 *)card, sizeof(*card) - sizeof(card->checksum)) != card->checksum) {
                goto corruptCard;
            }
            *D_8013B484[player] = *card;
        }
        recordTask = D_8013AE88;
    showRecord:
        recordTask->data18 = 15;
        break;
    corruptCard:
        D_8013AE88->data18 = 38;
        break;
    case 38:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_80130390, menu->cardSlot, color,
                                              1, NULL, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 39;
        break;
    case 39:
        if (hasStateValueChanged(menu->cardSlot))
            goto inspectCard;
        if (resources->panels[1].flags == 0)
            break;
    case 40:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_801303CC, -1, color,
                                              1, D_801307AC, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 41;
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
        D_8013AE88->data18 = 22;
        break;
    retryCardSelection:
        D_8013AE88->data18 = 30;
        break;
    case 15:
        parentPanel->flags = 1;
        resources->panels[0].command = 20;
        resources->panels[1].command = 99;
        configureColorCycleTask(1, 0, 128, 8, player);
        gPlayerGroupedSelectionTransitionFlags[player] = 0;
        D_8013AE88->data18 = 16;
        break;
    case 16:
        if (isTaskResourceTransitionComplete(player) != 1)
            break;
        resources->panels[0].command = 22;
    case 17:
        configureColorCycleTask(2, 64, 128, 4, player);
        createRegisteredCallbackTask(player, position->x / 4096, position->y / 4096,
                                     drawSaveRecordPushAnyButtonPrompt, D_8013AE88->data1C - 3, 21);
        D_8013AE88->data18 = 18;
        break;
    case 18:
        if (resources->panels[2].flags == 0) {
            resources->panels[2].command = 21;
            D_8013AE88->data18 = 19;
        }
        break;
    case 19:
        if (D_8013BE30.current[menu->cardSlot] == 7) {
            resources->panels[2].command = 99;
            D_8013AE88->data18 = 33;
            break;
        }
        if (D_8013AEE8[player]) {
            selection->choiceResult = 1;
            resources->panels[2].command = 99;
            D_8013AE88->data18 = 20;
        }
        break;
    case 20:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, D_80130780, -1, color,
                                              1, D_801307AC, D_8013AE88->data1C - 3, 11);
        D_8013AE88->data18 = 21;
        break;
    case 21:
        if (D_8013BE30.current[menu->cardSlot] == 7) {
        inspectCard:
            D_8013AE88->data18 = 33;
            break;
        }
        result = resources->panels[1].flags;
        if (result == 0)
            break;
        if (result == 1)
            goto drawRecord;
        if (result == 2) {
            gPlayerSaveRecords[player] = *D_8013B484[player];
            copySaveRecordAndMergeFlags();
            applyLoadedSaveRecordSettings(&gPlayerSaveRecords[player], player);
            D_8013B47C = 1;
            D_8013AE88->data18 = 24;
        } else if (result == 3) {
            task = D_8013AE88;
            resources->panels[0].command = 23;
            task->data18 = 30;
        } else {
        drawRecord:
            resources->panels[1].command = 99;
            D_8013AE88->data18 = 17;
        }
        break;
    case 22:
        copyGlobalFlagState(&gPlayerSaveRecords[player], D_8013B484[player]);
        gUnlockNotificationFlags = 0;
        resources->panels[0].command = 20;
        configureColorCycleTask(1, 0, 128, 8, player);
        gPlayerGroupedSelectionTransitionFlags[player] = 0;
        D_8013AE88->data18 = 23;
        break;
    case 23:
        if (isTaskResourceTransitionComplete(player) == 1) {
            resources->panels[0].command = 22;
            D_8013AE88->data18 = 26;
        }
        break;
    case 24:
        resources->countdown = createCountdownTask(15);
        D_8013AE88->data18 = 25;
        break;
    case 25:
        countdownStatus = resources->countdown->data1A;
        if (countdownStatus != 1)
            break;
        resources->countdown->data18 = countdownStatus;
        D_8013AE88->data18 = 26;
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
        D_8013AE88->data18 = 28;
        break;
    case 28:
        if (resources->panels[0].flags == 0)
            D_8013AE88->data18 = 29;
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    slot = D_8013AE88->data18;
    if (slot < D_8013AFCD) {
        if (D_8013AE88->data1A != 0) {
            if (D_8013AE88->data1A == 1 || D_8013AE88->data1A == 2) {
                copyColorCycleTaskColor(slot, &D_8013B3A4);
                r = D_8013B3A4;
                g = D_8013B3A5;
                b = D_8013B3A6;
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
                                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                }
                drawSpriteByIndexWithBlend(i * 32 + (s16)(x + 8), y + 60, 0x70 + i * 3, 0, -1,
                              &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
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
                                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                }
                if (D_8013B484[slot]->flags & (1u << (i - 8))) {
                    drawSpriteByIndexWithBlend((i - 8) * 32 + (s16)(x + 200), y + 19,
                                  0x70 + D_8012D460[i] * 3, 0, -1,
                                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
                }
            }
            drawSpriteByIndexWithBlend(x + 72, y + 19, 0x5A, 3, -1,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            drawDecimalNumber16(x + 72, y + 30, 6, 0, D_8013B484[slot]->money,
                                8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            func_8006EF30((s16)(x + 168), (s16)(y + 30), 1, gSaveRecordProgressCurrencyLabel, 8, -1,
                          &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    } else {
        setSpriteColor(128, 128, 128);
        drawSpriteByIndexWithBlend(x + 64, y + 32, 0xB4, 1, -1,
                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }

    if (slot < D_8013AFCD) {
        setSpriteColor(128, 128, 128);
        setTexturedQuadColor(128, 128, 128);
    } else {
        setSpriteColor(64, 64, 64);
        setTexturedQuadColor(64, 64, 64);
    }
    drawDecimalNumber8x8(x + 54, y + 4, 1, 0, slot + 1, 0, -1,
                         &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (slot == 0) {
        list = taskIndexPanelQuadsPalette0;
    } else {
        list = taskIndexPanelQuadsPalette1;
    }
    drawTexturedQuadList(x, y, list, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

u16 gSaveRecordProgressCurrencyLabel[2]
    __attribute__((section(".data.save_record_progress_currency_label"), aligned(4))) = {0x11, 0};

void drawSaveRecordPushAnyButtonPrompt(void) {
    Movement2D *position;
    s32 x, y;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    func_8006EC84(x + 134, y - 6, 20, gSaveRecordPushAnyButtonText, 0, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    setSpriteColor(0, 0, 0);
    func_8006EC84(x + 135, y - 5, 20, gSaveRecordPushAnyButtonText, 0, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006EC84(x + 136, y - 5, 20, gSaveRecordPushAnyButtonText, 0, -1,
                  &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}
