#define RACE_FINISH_ABSOLUTE_ADDRESSING
#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#include "game/controller_effects.h"
#include "game/global_flags.h"
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/selected_records.h"
#include "game/cd_queue.h"
#define GAME_OPTION_TRANSITION_ABSOLUTE_ADDRESSING
#include "game/game_options.h"
#include "game/graphics.h"
#define MUSIC_STATE_ABSOLUTE_ADDRESSING
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/music.h"
#include "psx/libsnd.h"
#include "psx/libsn.h"
#include "psx/libc.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/text_entries.h"
#include "game/task.h"
#include "game/selected_sprite.h"
#include "game/movement.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libsn.h"
#include "psx/libc.h"

#define DRAW_MODE_COMMAND_HIGH 0xE100
#define TEXTURED_SPRITE_CODE 0x64

typedef struct DrawModeSpritePacket {
    u32 tag;
    u16 tpage, drawModeCode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} DrawModeSpritePacket;

typedef struct CdAudioOwner {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0x10];
    GameTask *task;
} CdAudioOwner;
typedef struct CdAudioPlaybackOwner {
    u8 unk00[0x24];
    u32 tick;
    u16 flags;
    u8 unk2A[0xE];
    GameTask *task;
    u8 unk3C[4];
    u32 completionTick;
} CdAudioPlaybackOwner;
typedef struct CdAudioResource {
    TextEntry *entry;
    u32 deadline;
} CdAudioResource;

typedef struct SelectionMenuState {
    s16 selection, previousSelection;
    s16 unk04, unk06, unk08, unk0A;
    u8 unk0C[0xF4];
} SelectionMenuState;

typedef struct UnlockMenuState {
    s16 selection, previousSelection;
    s32 unk04[7];
    s32 unk20[7];
    s32 unk3C[7];
    s32 unk58[7];
    s16 unk74[4];
    u8 unk7C[0x10];
    s16 unk8C;
    u8 unk8E[0x72];
} UnlockMenuState;

typedef struct TextControlState {
    s16 unk00;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u8 unk10[8];
    s16 unk18[5];
} TextControlState;
typedef struct TextResourceState {
    TextEntry *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
} TextResourceState;
typedef struct TextPlaybackState {
    TextEntry *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
    u8 unk22[2];
    s32 tick;
    u16 flags, unk2A;
    GameTask *tasks[5];
    u32 unk40;
    u8 unk44[0xBC];
} TextPlaybackState;

typedef struct TextTaskResource {
    TextEntry *entry;
    u8 unk04[0x2C];
    GameTask *task;
    u8 unk34[0xCC];
} TextTaskResource;

typedef struct SecondaryOwner {
    u8 unk00[0x24];
    u32 tick;
    u16 flags;
    u16 resource;
    GameTask *task;
} SecondaryOwner;
typedef struct SecondaryResource {
    TextEntry *entry;
    u32 deadline;
} SecondaryResource;
typedef struct ControllerPlaybackEntry {
    u8 course;
    u8 pad;
    s16 sector;
    s16 count;
} ControllerPlaybackEntry;

typedef struct CdResourceLocation {
    s16 sector;
    s16 count;
} CdResourceLocation;

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;


u16 gTextOneShotSoundKey __attribute__((section(".sdata.text_sound_state"))) = 0;
u8 gTextOneShotVoice __attribute__((section(".sdata.text_sound_state"))) = 0;

ControllerPlaybackEntry gControllerPlaybackEntries[12] __attribute__((section(".data.controller_playback_entries"))) = {
    {0, 0, 0, 0},
    {0, 0, 9037, 2},
    {0, 0, 9039, 2},
    {9, 0, 9041, 2},
    {0, 0, 9043, 2},
    {0, 0, 9045, 2},
    {4, 0, 9047, 2},
    {1, 0, 9049, 2},
    {3, 0, 9051, 2},
    {2, 0, 9053, 2},
    {9, 0, 9055, 2},
    {0, 0, 9057, 2},
};

s16 gControllerPlaybackDurations[12] __attribute__((section(".data.controller_playback_durations"))) = {
    900, 384, 560, 592, 613, 606, 400, 288, 288, 286, 205, 272
};

extern u8 D_8013AF3A __attribute__((section(".data")));
extern u16 D_8013AFAA __attribute__((section(".data")));
extern s16 D_8013B10E __attribute__((section(".data")));
extern s16 D_8013B10C __attribute__((section(".data")));
extern u8 D_8013B55E __attribute__((section(".data")));
/* This transition uses $gp; the playback transition below uses absolute access. */
extern u8 modeTransitionReadyGp;

extern u8 D_8013B564;

extern s16 D_8013B560;
extern s16 selectionMenuRecordAbsolute __attribute__((section(".data")));
extern u8 D_8013B562;
/* Playback cleanup addresses this shared transition byte absolutely. */
extern u8 playbackTransitionByteAbsolute __attribute__((section(".data")));
extern char D_80012310[];
extern char D_8001232C[];
extern char D_8013B5C8[];

void func_800187B8(u8 voice);
void func_8001814C(void);
int printf(const char *format, ...);

void func_80040D5C(void);
void initializeNumberSelectionTask(void);
void completeModeSelectionTransitionWhenReady(void);
void func_800A2D68(void);
void finishSelectionMenuTask(void);
void func_800A31C0(SelectionMenuState *state);
void initializeModeSectorReadResourcesTask(void);
void func_800A24EC(void);
void selectTaskBySelectedRecordMatch(void);
void SetFogNearFar(s32 near, s32 far, s32 h);
void func_800A2C40(void);
void func_800A25F8(void);
void initializeControllerPlaybackRaceTask(void);
void initializeRaceTaskState(void);
void func_800A5AFC(void);
void initializeTextPlaybackTask(void);

void func_80040D5C(void);
void initializeNumberSelectionTask(void);
void completeModeSelectionTransitionWhenReady(void);
void finishControllerPlaybackTransition(void);
void func_8002FA58(s16 resource);
void func_800A42CC(void);
void func_800A499C(void);
void func_8001B858(void);
void func_800A1F50(void);
void func_800A4C28(void);
void resetUnlockMenuOffsets(UnlockMenuState *state);
void resetUnlockMenuAnimation(UnlockMenuState *state);
void func_800A3D9C(void);
void func_800A525C(void);

typedef struct NumberPanelState {
    u16 unk00[5];
    u16 number;
} NumberPanelState;

typedef struct NumberAdjustmentState {
    s16 value;
    s16 countdown;
    s16 active;
    u8 unk06[0x1A];
    s32 unk20[3];
    s32 unk2C[3];
    u8 unk38[0xC8];
} NumberAdjustmentState;
typedef struct MusicOwnerState {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0xC];
    GameTask *musicTask;
    u8 unk38[8];
    u32 completionTick;
} MusicOwnerState;

typedef struct MusicResource {
    TextEntry *entry;
    u32 deadline;
} MusicResource;
void updateTextEntryCdAudioTask(void);
void func_800A53BC(void);
void finishNumberAdjustmentTask(void);
void func_8002F394(void);
void func_800A5630(void);
void beginMode2SequenceAndFadeMusic(void);
void startUnlockResourceLoad(void);
void prepareUnlockSelectionResume(void);
extern u8 D_8013AF3A __attribute__((section(".data")));
INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A1F50);

void advanceTaskAtModeCoursePosition(void) {
    s16 reached = 0;

    switch (D_8013AFD8) {
    case 1:
        if (D_80120E90[0].unk92E == 80) reached = 1;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        if (D_80120E90[0].unk92E == 156) reached = 1;
        break;
    case 6:
        if (D_80120E90[0].unk92E == 180) reached = 1;
        break;
    case 7:
        if (D_80120E90[0].unk92E == 22) reached = 1;
        break;
    case 8:
        if (D_80120E90[0].unk92E == 29) reached = 1;
        break;
    case 9:
        if (D_80120E90[0].unk92E == 54) reached = 1;
        break;
    }
    func_80040D5C();
    if (reached) {
        stopPlayerSound(0);
        stopPlayerSound(1);
        startCdVolumeFade(103, 20, 0, 1);
        taskTransitionFlagGp = 0;
        func_80014874(initializeNumberSelectionTask);
        setCurrentTaskCallback(completeModeSelectionTransitionWhenReady);
    }
}

void completeModeSelectionTransitionWhenReady(void) {
    if (modeTransitionReadyGp != 0) {
        D_8013AFAA = (D_8013AFAA | 0x1090) & ~0x2000;
        resetPendingCdRequests();
        resetPaletteRecords();
        func_800149E0();
        D_8013AFD3 = 0;
        D_8013AFAA |= 0x48;
        switch (D_8013AFD8) {
        case 10:
            modeTransitionReadyGp = 0;
            func_80014874(initializeModeSectorReadResourcesTask);
            D_8013AE88->unk10 |= 1;
            break;
        case 11:
            D_8013AFAA |= 0x48;
            D_8013B10C = -1;
            D_8013B10E = -1;
            D_8013B106 = 0;
            D_8013AFD8 = 0;
            D_8013B562 = 0;
            SetFogNearFar(5000, 6000, 400);
            setCurrentTaskCallback(func_800A24EC);
            D_8013B108 = 2;
            return;
        default:
            func_80014838(initializeMode1RacePlayers);
            setCurrentTaskCallback(selectTaskBySelectedRecordMatch);
            D_8013AE88->unk10 |= 1;
            break;
        }
    }
}

void selectTaskBySelectedRecordMatch(void) {
    if (D_8013B560 != D_8013AFCE) {
        D_8013B562 = 0;
        setCurrentTaskCallback(func_8001B858);
    } else {
        D_8013B562 = 1;
        setCurrentTaskCallback(func_800A1F50);
    }
}

void startMode1TaskTransition(void) {
    setSecondaryDisplayTaskMode(1);
    configureTaskStateTransition(1, 0, 30);
    setCurrentTaskCallback(initializeGameOptionViewTask);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A24EC);

void initializeNumberSelectionTask(void) {
    NumberPanelState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (NumberPanelState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    D_8013AFD3 = 1;
    state->number = D_8013AFD8 - 1;
    D_8013AE88->data18 = 0;
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetPendingCdRequests();
    func_80014874(func_800A2C40);
    setCurrentTaskCallback(func_800A25F8);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A25F8);

void drawNumberSelectionPanel(NumberPanelState *state) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006D984(23, -39, 1, 0, state->number + 1, 5, -1, ot, 2);
    func_8006C998(-40, -36, 404, 8, -1, ot, 2);
    drawDefaultTexturedPanel(-56, -48, 112, 96, 5, 0, ot, 2);
    func_80070C5C(128, ot, 2);
}

void drawOffsetSelectedTaskSprite(void) {
    SelectedSpriteResource *state;
    SelectedSpriteChild *children;
    Movement2D *position;
    u16 mode;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = (SelectedSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (SelectedSpriteChild *)&D_800D9504[D_8013AE88->resource16];
    mode = state->mode;
    if ((s16)mode > 0) {
        if ((u32)(mode - 1) < 2) {
            setSpriteColor(state->r, state->g, state->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&D_800D9504[children[state->selectedIndex].task->resource14];
        func_8006C998(position->x / 4096 - 8, position->y / 4096 + 2, state->sprite, 0, -1,
                     &D_800EBFF0[D_8013AF78], state->depth - 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2C40);

void initializeSelectionMenuTask(void) {
    SelectionMenuState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (SelectionMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->previousSelection = state->selection = D_8013AFD9 - 1;
    state->unk04 = 0;
    state->unk06 = 16;
    state->unk08 = 0;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013AFCE = -1;
    selectionMenuRecordAbsolute = 99;
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(1, 0, 30);
    if (D_8013B564 == 0) {
        queueCdCommand(3, 0);
        D_8013B564 = 1;
    }
    setCurrentTaskCallback(func_800A2D68);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2D68);

void finishSelectionMenuTask(void) {
    SelectionMenuState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (SelectionMenuState *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 0:
        if (state->unk08 == 0) {
            configureTaskStateTransition(2, 0, 30);
            startCdVolumeFade(103, 20, 0, 1);
            D_8013AE88->data18 = 1;
        }
        break;
    case 1:
        if (func_800707F0(0) == 0) {
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        configureColorCycleTask(0, 0, 0, 0, 0);
        configureColorCycleTask(0, 0, 0, 0, 1);
        if (state->unk0A != 0) {
            D_8013B108 = 2;
            D_8013B109 = 1;
            func_800149A8();
        } else {
            queueStartupSectorReads();
            setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
            D_8013AE88->unk10 |= 2;
        }
        return;
    }
    func_800A31C0(state);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A31C0);

void initializeControllerPlaybackRaceTask(void) {
    s32 i = 0;
    RacePlayer *player;
    selectionMenuRecordAbsolute = D_8013AFCE;
    D_8013AFCE = gControllerPlaybackEntries[D_8013AFD9].course;
    player = D_80120E90;
    D_8013AFD3 = 0;
    D_8013AFD6 = 0;
    D_8013AFE1 = 0;
    D_8013AFD8 = 0;
    D_8013AFE2 = 0;
    D_8013AFE0 = 0;
    player[0].unk008 = 0;
    player[1].unk008 = 0;
    player[2].unk008 = 1;
    player[3].unk008 = 1;
    player[0].unk014 = 0;
    player[1].unk014 = 1;
    player[2].unk014 = 2;
    player[3].unk014 = 3;
    player[0].unk015[1] = 0;
    player[1].unk015[1] = 0;
    player[2].unk015[1] = 0;
    player[3].unk015[1] = 0;
    player[0].unk015[2] = 3;
    player[1].unk015[2] = 5;
    player[2].unk015[2] = 6;
    player[3].unk015[2] = 7;
    player[0].unk025[3] = 0;
    player[1].unk025[3] = 1;
    player[2].unk025[3] = 2;
    player[3].unk025[3] = 3;
    player[0].unk024 = 1;
    player[1].unk024 = 1;
    player[2].unk024 = 1;
    player[3].unk024 = 1;
    D_8013AFD0 = 5;
    player[0].unk01C = 0;
    player[1].unk01C = 0;
    player[2].unk01C = 0;
    player[3].unk01C = 0;
    D_8013AFD2 = 3;
    D_8013AFCD = 2;
    D_8013AFDE = 100;
    for (; i < 4; i++) {
        player[i].unk025[1] = 0;
        player[i].unk025[2] = 0;
    }
    resetGameplayRng();
    configureTaskStateTransition(1, 0, 30);
    setSecondaryDisplayTaskMode(1);
    setCurrentTaskCallback(initializeRaceTaskState);
    queueCdSectorCommand(0, gControllerPlaybackEntries[D_8013AFD9].sector, gControllerPlaybackEntries[D_8013AFD9].count, D_800E959C, 0);
    if (selectionMenuRecordAbsolute != D_8013AFCE) {
        func_800180F4(2);
        func_800180F4(3);
        func_800180F4(4);
        func_800180F4(5);
        queuePlayerResourceReadsAndUploadTextures();
    }
    D_8013AE88->unk10 |= 2;
}


void advanceTaskWhenFlagB55ESet(void) {
    if (D_8013B55E != 0) {
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(updateControllerPlaybackUntilFade);
    }
}

void updateControllerPlaybackUntilFade(void) {
    func_80040D5C();
    if (D_8013AEE8[0] & 0x800) {
        D_8013AE88->data18 = gControllerPlaybackDurations[D_8013AFD9];
    }
    D_8013AE88->data18++;
    if (gControllerPlaybackDurations[D_8013AFD9] <= D_8013AE88->data18) {
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishControllerPlaybackTransition);
    } else {
        playbackControllerInput(0);
        playbackControllerInput(1);
    }
}

void finishControllerPlaybackTransition(void) {
    switch (D_8013AE88->data18) {
    case 0:
        if (func_800707F0(0)) {
            playbackControllerInput(0);
            playbackControllerInput(1);
        } else {
            stopPlayerSound(0);
            D_8013AFAA = (D_8013AFAA | 0x1090) & ~0x2000;
            resetPendingCdRequests();
            resetPaletteRecords();
            func_800149E0();
            D_8013B10C = -1;
            D_8013AFAA |= 0x48;
            func_8002FA58(2);
            D_8013B106 = 0;
            playbackTransitionByteAbsolute = 0;
            setControllerFlag5(0);
            setControllerFlag5(1);
            SetFogNearFar(5000, 6000, 400);
            D_8013B109 = 1;
            D_8013B108 = 28;
            D_8013AE88->data18 = 1;
        }
        break;
    case 1:
        setSecondaryDisplayTaskMode(1);
        configureTaskStateTransition(1, 0, 30);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        D_8013B564 = 0;
        setCurrentTaskCallback(initializeGameOptionViewTask);
        D_8013AE88->unk10 |= 2;
        break;
    }
}

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
    setCurrentTaskCallback(func_800A3D9C);
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

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3D9C);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A42CC);

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
                func_8002FA58(2);
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
        if (func_800707F0(0) == 0) { func_800149A8(); }
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
    func_800A4C28();
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

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A499C);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A4C28);

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

void startUnlockActionAfterCountdown(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013AF3A < 11) {
            func_80014874(func_800A624C);
        } else {
            func_80014874(startUnlockResourceLoad);
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
    func_800A4C28();
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
        setCurrentTaskCallback(func_800A3D9C);
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
    func_8002FA58(2);
    setCurrentTaskCallback(resumeUnlockSelectionTask);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A525C);

void startUnlockResourceLoad(void) {
    resetPendingCdRequests();
    D_8013AFE4 &= ~0x100;
    createResourceLoadTask();
    setCurrentTaskCallback(func_800A525C);
}

void initializeNumberAdjustmentTask(void) {
    NumberAdjustmentState *state;
    s16 i;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    state->value = D_8013AFD0;
    for (i = 0; i < 3; i++) {
        state->unk20[i] = 0;
        state->unk2C[i] = 0;
    }
    state->active = 1;
    func_8002FA58(0x17);
    setCurrentTaskCallback(func_800A53BC);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A53BC);

void updateNumberAdjustmentTask(void) {
    NumberAdjustmentState *state;
    s16 previous;
    u16 buttons;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    buttons = D_8013AEE8[0] | D_8013AEE8[1];
    previous = state->value;
    if ((buttons & 0x1000) && state->value > 1) {
        state->value--;
    }
    if ((buttons & 0x4000) && state->value < 9) {
        state->value++;
    }
    if (previous != state->value) {
        playSoundEffect(0, 0x100, 0x20);
    }
    if (buttons & 0xB0) {
        playSoundEffect(0, 0, 0x20);
        D_8013AFD0 = state->value;
        state->countdown = 20;
        state->active = 0;
        D_8013AE98->data18 = 0x17;
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishNumberAdjustmentTask);
    }
    if (buttons & 0x40) {
        playSoundEffect(0, 0, 0x20);
        D_8013AE98->data18 = 8;
        func_8002F394();
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishNumberAdjustmentTask);
    }
    func_800A5630();
}

void drawNumberAdjustmentArrow(s16 direction, GsOT *ot, u16 depth) {
    POLY_F3 *triangle = &gNumberAdjustmentArrow;
    setRGB0(triangle, 0xC0, 0x20, 0);
    if (direction == 0) {
        setXY3(triangle, -46, -32, -40, -44, -34, -32);
    } else {
        setXY3(triangle, -46, -8, -40, 4, -34, -8);
    }
    GsSortPoly(triangle, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5630);

void finishNumberAdjustmentTask(void) {
    NumberAdjustmentState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberAdjustmentState *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 0:
        if (state->countdown == 0) {
            if (D_8013AE98->data18 == 0x17) {
                beginMode2SequenceAndFadeMusic();
                D_8013AE88->data18 = 1;
            } else {
                D_8013AE88->data18 = 2;
            }
        }
        break;
    case 1:
        if (func_800707F0(0) != 0) break;
    case 2:
        D_8013AE98->data1C--;
        func_800149A8();
        return;
    }
    func_800A5630();
}

void func_800A5AD4(void) {
}

void func_800A5ADC(void) {
}

void func_800A5AE4(void) {
}

void func_800A5AEC(void) {
}

void func_800A5AF4(void) {
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5AFC);

s16 parseDecimalS16(const u8 *text, s16 *result) {
    s16 value = 0;
    s16 count = 0;
    u16 digit;
    while ((u32)(*text - '0') < 10) {
        count++;
        digit = *text++;
        value = value * 10 + (digit - (u16)'0');
    }
    *result = value;
    return count;
}

void resetTextEntryStates(TextControlState *control, TextResourceState *resource) {
    s16 i;
    if (control != NULL) {
        control->unk00 = 0;
        control->unk02 = 1;
        control->unk04 = 0;
        control->unk08 = 0;
        control->unk0C = 0;
        for (i = 0; i < 5; i++) {
            control->unk18[i] = 0;
        }
    }
    if (resource != NULL) {
        for (i = 0; i < 5; i++) {
            resource->unk04[i] = 0;
            resource->unk18[i] = 0;
        }
    }
}

void resetTextEntryPool(void) {
    TextEntry *previous = NULL;
    TextEntry *entry = (TextEntry *)0x80180008;
    u16 i;
    D_8013B5D0 = entry;
    for (i = 0; i < 295; i++) {
        entry->previous = previous;
        previous = entry;
        previous->unk04 = 0;
        previous->unk06 = 0;
        previous->unk08 = 0;
        previous->unk0A = 0;
        previous->next = previous + 1;
        entry++;
    }
    entry[-1].next = NULL;
    D_8013B5D4 = NULL;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5F00);

TextEntry *findKeyedListNode(u32 key, u32 value) {
    TextEntry *node = D_8013B5D4;
    TextEntry *result = NULL;
    while (node != NULL) {
        if (key == (u32)node->text && value == node->unk04) {
            result = node;
            break;
        }
        node = node->next;
        result = NULL;
    }
    return result;
}

TextEntry *findTextEntryInsertionPoint(u32 key, u32 value) {
    TextEntry *node = D_8013B5D4;
    while (node != NULL) {
        if (key == (u32)node->text && value < node->unk04) {
            break;
        }
        if (key < (u32)node->text) {
            break;
        }
        node = node->next;
    }
    return node;
}

s16 countWrappedTextLines(const s8 *text, s16 width, s16 limit) {
    s16 count = 0;
    s16 column = 0;
    s16 lines = 1;
    while (*text != 0) {
        if (count >= limit) break;
        if (column >= width) {
            if (*text == '\r' && text[1] == '\n') text += 2;
            lines++;
            column = 0;
        }
        if (*text == '\r' && text[1] == '\n') {
            lines++;
            column = 0;
            text += 2;
        }
        if (*text & 0x80) {
            column++;
            count++;
            text += 2;
        } else if (*text == ' ') {
            column++;
            text++;
            count++;
            if (*text != ' ') continue;
            text++;
            continue;
        } else {
            text += 2;
        }
    }
    return lines;
}

s16 advanceTextLine(const s8 **text, s16 limit) {
    const s8 *p = *text;
    s16 count = 0;
    while (*p != 0) {
        if (count >= limit) {
            if (*p == '\r' && p[1] == '\n') {
                p += 2;
            }
            break;
        }
        if (*p == '\r' && p[1] == '\n') {
            p += 2;
            break;
        }
        if (*p & 0x80) {
            count++;
            p += 2;
        } else if (*p == ' ') {
            p++;
            count++;
            if (*p != ' ') {
                continue;
            }
            p++;
            continue;
        } else {
            p += 2;
        }
    }
    *text = p;
    return count;
}

void playMusicSequence(s16 count, u8 left, u8 right) {
    if (gMusicSequenceActive) {
        D_8013AF30 = left << 8;
        SsSeqSetVol(gMusicSequenceId, left, right);
        SsSeqPlay(gMusicSequenceId, 1, count);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A624C);
