#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#include "game/controller_effects.h"
#include "game/global_flags.h"
#define SELECTED_RECORDS_ABSOLUTE_ADDRESSING
#include "game/selected_records.h"
#include "game/cd_queue.h"
#include "game/game_options.h"
#include "game/graphics.h"
#define MUSIC_ABSOLUTE_ADDRESSING
#include "game/music.h"
#include "psx/libsnd.h"
#include "psx/libsn.h"
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

typedef struct CdAudioOwner {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0x10];
    GameTask *task;
} CdAudioOwner;
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
typedef struct CdResourceLocation {
    s16 sector;
    s16 count;
} CdResourceLocation;

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;


s16 gTextEntryCdCommands[230] __attribute__((section(".data.text_entry_cd_commands"))) = {
    0x0000, 0x0010, 0x0011, 0x0077, 0x00BE, 0x0047, 0x00A5, 0x0000, 0x00E1, 0x008E,
    0x0046, 0x00D0, 0x00C7, 0x00BD, 0x001B, 0x00D9, 0x00E0, 0x008D, 0x003B, 0x005E,
    0x0031, 0x005D, 0x00BC, 0x00AC, 0x009E, 0x00CF, 0x0012, 0x00C6, 0x00B8, 0x009D,
    0x003F, 0x008C, 0x00B7, 0x00CE, 0x00D6, 0x00E7, 0x0094, 0x005F, 0x00B6, 0x00D2,
    0x0030, 0x0013, 0x0045, 0x00B5, 0x003E, 0x0052, 0x001D, 0x00AB, 0x00C5, 0x004E,
    0x00B4, 0x004D, 0x0068, 0x0076, 0x0016, 0x0026, 0x00D8, 0x0020, 0x0082, 0x0062,
    0x003A, 0x00B3, 0x0051, 0x00D5, 0x008B, 0x00A4, 0x0093, 0x0043, 0x0092, 0x00C4,
    0x001C, 0x0018, 0x00B2, 0x00DF, 0x002B, 0x009C, 0x0067, 0x00ED, 0x00E6, 0x00DE,
    0x00E5, 0x00E4, 0x00CD, 0x00A3, 0x0017, 0x0063, 0x0024, 0x00A0, 0x0059, 0x0095,
    0x004A, 0x0058, 0x009F, 0x0049, 0x0048, 0x0021, 0x0083, 0x0041, 0x0057, 0x0056,
    0x0080, 0x0066, 0x00E3, 0x00EA, 0x0055, 0x00DC, 0x0089, 0x005B, 0x0098, 0x007F,
    0x002E, 0x0035, 0x007A, 0x007E, 0x00E9, 0x0061, 0x0091, 0x00EC, 0x0081, 0x002A,
    0x0042, 0x001F, 0x0050, 0x002F, 0x00D4, 0x0039, 0x006D, 0x0075, 0x00D1, 0x0000,
    0x00EE, 0x001E, 0x0029, 0x0014, 0x0038, 0x0015, 0x00BB, 0x0033, 0x004C, 0x00A8,
    0x0037, 0x0036, 0x006B, 0x00A2, 0x008A, 0x00A1, 0x00F0, 0x00B1, 0x006F, 0x0028,
    0x009A, 0x0099, 0x00AA, 0x00EB, 0x005C, 0x009B, 0x006E, 0x00DD, 0x00C3, 0x003C,
    0x0022, 0x0025, 0x00DB, 0x0073, 0x0027, 0x00E2, 0x002D, 0x0019, 0x0088, 0x002C,
    0x0079, 0x0032, 0x005A, 0x0040, 0x00B0, 0x0072, 0x0097, 0x0065, 0x0078, 0x0060,
    0x00BA, 0x00CC, 0x004F, 0x00D7, 0x0034, 0x0090, 0x008F, 0x0087, 0x00CB, 0x00AF,
    0x001A, 0x00D3, 0x006A, 0x00CA, 0x00A7, 0x00C0, 0x0044, 0x0071, 0x0054, 0x0086,
    0x0000, 0x007D, 0x004B, 0x00AE, 0x0023, 0x00E8, 0x0070, 0x0069, 0x0085, 0x00AD,
    0x00C9, 0x0084, 0x00BF, 0x00B9, 0x00A6, 0x00C8, 0x0096, 0x0064, 0x0053, 0x00DA,
    0x00A9, 0x007C, 0x00C2, 0x007B, 0x006C, 0x0074, 0x00C1, 0x003D, 0x0000, -1,
};

CdResourceLocation gSecondaryResourceLocations[108] __attribute__((section(".data.secondary_resource_tables"))) = {
    {0x1B49, 0x0026},
    {0x136B, 0x0026},
    {0x1391, 0x0026},
    {0x13B7, 0x0026},
    {0x13DD, 0x0026},
    {0x1403, 0x0026},
    {0x1429, 0x0026},
    {0x144F, 0x0026},
    {0x1475, 0x0026},
    {0x149B, 0x0026},
    {0x14C1, 0x0026},
    {0x14E7, 0x0026},
    {0x150D, 0x0026},
    {0x1533, 0x0026},
    {0x1559, 0x0026},
    {0x157F, 0x0026},
    {0x15A5, 0x0026},
    {0x15CB, 0x0026},
    {0x15F1, 0x0026},
    {0x1617, 0x0026},
    {0x163D, 0x0026},
    {0x1663, 0x0026},
    {0x1689, 0x0026},
    {0x16AF, 0x0026},
    {0x16D5, 0x0026},
    {0x16FB, 0x0026},
    {0x1721, 0x0026},
    {0x1747, 0x0026},
    {0x176D, 0x0026},
    {0x1793, 0x0026},
    {0x17B9, 0x0026},
    {0x17DF, 0x0026},
    {0x1805, 0x0026},
    {0x182B, 0x0026},
    {0x1851, 0x0026},
    {0x1877, 0x0026},
    {0x189D, 0x0026},
    {0x18C3, 0x0026},
    {0x18E9, 0x0026},
    {0x190F, 0x0026},
    {0x1935, 0x0026},
    {0x195B, 0x0026},
    {0x1981, 0x0026},
    {0x19A7, 0x0026},
    {0x19CD, 0x0026},
    {0x19F3, 0x0026},
    {0x1A19, 0x0026},
    {0x1A3F, 0x0026},
    {0x1A65, 0x0026},
    {0x1A8B, 0x0026},
    {0x1AB1, 0x0026},
    {0x1AD7, 0x0026},
    {0x1AFD, 0x0026},
    {0x1B23, 0x0026},
    {0x1B49, 0x0026},
    {0x1B6F, 0x0026},
    {0x1B95, 0x0026},
    {0x1BBB, 0x0026},
    {0x1BE1, 0x0026},
    {0x1C07, 0x0026},
    {0x1C2D, 0x0026},
    {0x1C53, 0x0026},
    {0x1C79, 0x0026},
    {0x1C9F, 0x0026},
    {0x1CC5, 0x0026},
    {0x1CEB, 0x0026},
    {0x1D11, 0x0026},
    {0x1D37, 0x0026},
    {0x1D5D, 0x0026},
    {0x1D83, 0x0026},
    {0x1DA9, 0x0026},
    {0x1DCF, 0x0026},
    {0x1DF5, 0x0026},
    {0x1E1B, 0x0026},
    {0x1E41, 0x0026},
    {0x1E67, 0x0026},
    {0x1E8D, 0x0026},
    {0x1EB3, 0x0026},
    {0x1ED9, 0x0026},
    {0x1EFF, 0x0026},
    {0x1F25, 0x0026},
    {0x1F4B, 0x0026},
    {0x1F71, 0x0026},
    {0x1F97, 0x0026},
    {0x1FBD, 0x0026},
    {0x1FE3, 0x0026},
    {0x2009, 0x0026},
    {0x202F, 0x0026},
    {0x2055, 0x0026},
    {0x207B, 0x0026},
    {0x20A1, 0x0026},
    {0x20C7, 0x0026},
    {0x20ED, 0x0026},
    {0x2113, 0x0026},
    {0x2139, 0x0026},
    {0x215F, 0x0026},
    {0x2185, 0x0026},
    {0x21AB, 0x0026},
    {0x21D1, 0x0026},
    {0x21F7, 0x0026},
    {0x221D, 0x0026},
    {0x2243, 0x0026},
    {0x2269, 0x0026},
    {0x228F, 0x0026},
    {0x22B5, 0x0026},
    {0x22DB, 0x0026},
    {0x2301, 0x0026},
    {0x2327, 0x0026},
};
u8 gSecondaryResourceModeOffsets[12] __attribute__((section(".data.secondary_resource_tables"))) = {
    0, 31, 40, 46, 53, 62, 67, 78, 85, 93, 101, 108
};

GameTask *gSecondaryResourceDisplayTask __attribute__((section(".sdata.secondary_resource_display_task"))) = NULL;

s16 gControllerPlaybackDurations[12] __attribute__((section(".data.controller_playback_durations"))) = {
    900, 384, 560, 592, 613, 606, 400, 288, 288, 286, 205, 272
};

extern u8 D_8013AF3A __attribute__((section(".data")));
extern s16 D_8013B10C __attribute__((section(".data")));
extern u8 D_8013B55E __attribute__((section(".data")));

extern u8 D_8013B564;
extern s16 D_8013B59C;
extern u8 D_8013B59E;
extern u8 D_8013B59F;

extern s16 D_8013B560;
extern s16 selectionMenuRecordAbsolute __attribute__((section(".data")));
extern u8 D_8013B562;
extern char D_80012310[];
extern char D_8001232C[];


void func_800187B8(u8 voice);
void func_8001814C(void);
void signalFlag9WhenTaskCountdownExpires(void);
int printf(const char *format, ...);

void func_80040D5C(void);
void func_800A2324(void);
void func_800A2D68(void);
void func_800A2C40(void);
void func_800A25F8(void);
void func_800A3750(void);
void func_800A5AFC(void);
void initializeTextPlaybackTask(void);
void func_800A6684(void);
void func_800A624C(void);

void func_80040D5C(void);
void func_800A3AA4(void);
void func_8002FA58(s16 resource);
void func_800A42CC(void);
void func_800A499C(void);
void func_8001B858(void);
void func_800A1F50(void);
void func_800A4C28(void);
void resetUnlockMenuOffsets(UnlockMenuState *state);
void resetUnlockMenuAnimation(UnlockMenuState *state);
void func_8007BBAC(s16 index);
void func_800A3D9C(void);
void func_800A525C(void);
void initializeSecondaryResourceTask(void);
void func_800A6A8C(void);
void drawSecondaryTextBackgroundTask(void);
void func_800A6E88(void);

typedef struct AudioTransitionState {
    u8 unk00[0x2C];
    s32 unk2C[5];
} AudioTransitionState;

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
void func_800A77B0(void);
void func_800A53BC(void);
void func_800A5630(void);
void beginMode2SequenceAndFadeMusic(void);
void func_800A624C(void);
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
        func_80014874(func_800A2510);
        setCurrentTaskCallback(func_800A2324);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2324);

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
    func_80070274(1, 0, 30);
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
    func_80070274(1, 0, 30);
    if (D_8013B564 == 0) {
        queueCdCommand(3, 0);
        D_8013B564 = 1;
    }
    setCurrentTaskCallback(func_800A2D68);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A2D68);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3028);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A31C0);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3750);

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
        func_80070274(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(func_800A3AA4);
    } else {
        playbackControllerInput(0);
        playbackControllerInput(1);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A3AA4);

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
    func_80070274(1, 0, 30);
    func_8007BBAC(0);
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

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A46E0);

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

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A5418);

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

void initializeLoadedTextEntries(void) {
    TextEntryListHeader *header = (TextEntryListHeader *)0x80180000;
    TextEntry *freeEntries = header->freeEntries;
    TextEntry *activeEntries = header->activeEntries;
    D_8013B5D0 = freeEntries;
    D_8013B5D4 = activeEntries;
    func_800A5AFC();
    setCurrentTaskCallback(initializeTextPlaybackTask);
}

void initializeTextPlaybackTask(void) {
    TextPlaybackState *state;
    s16 i;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (TextPlaybackState *)&D_800D9504[D_8013AE88->resource16];
    state->currentEntry = findKeyedListNode(0, 0);
    if (state->currentEntry == NULL) {
        state->currentEntry = findTextEntryInsertionPoint(0, 0);
    }
    for (i = 0; i < 5; i++) {
        state->unk04[i] = 0;
        state->unk18[i] = 0;
    }
    state->tick = -1;
    state->flags = 0;
    state->unk2A = 0xFFFF;
    state->unk40 = 0;
    for (i = 0; i < 8; i++) {
        gTextSoundSlots[i].voice = 0xFF;
        gTextSoundSlots[i].soundKey = 0;
    }
    setSecondaryDisplayTaskMode(0);
    func_80070274(2, 5, 30);
    D_8013B59C = 0;
    D_8013B59E = 0;
    D_8013B59F = 0;
    setCurrentTaskCallback(func_800A6684);
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6684);

void setSecondaryResourceEntryTask(TextEntry *entry) {
    SecondaryOwner *owner;
    SecondaryResource *resource;
    s16 index;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (SecondaryOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL || entry->unk06 == 0) {
        owner->task = NULL;
        gSecondaryResourceDisplayTask = NULL;
    } else {
        func_80014874(initializeSecondaryResourceTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SecondaryResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        owner->task = D_8013AE8C;
        if (entry->unk08 == 0) resource->deadline = 0;
        else resource->deadline = owner->tick + entry->unk0A;
        owner->flags |= 1;
        if (entry->unk06 != owner->resource) {
            owner->resource = entry->unk06;
            index = gSecondaryResourceModeOffsets[D_8013AF3A] + entry->unk06 - 1;
            resetPendingCdRequests();
            queueCdSectorCommand(0, gSecondaryResourceLocations[index].sector, gSecondaryResourceLocations[index].count, (void *)0x80184060, 0);
        }
    }
}


void initializeSecondaryResourceTask(void) {
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {
        }
    }
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(func_800A6A8C);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6A8C);

void drawSecondaryTextBackgroundTask(void) {
    if (gSecondaryResourceDisplayTask != D_8013AE88) {
        func_800149A8();
    } else {
        if (D_8013B59F) {
            setSpriteColor(128, 128, 128);
            func_8007240C(-140, 44, 280, 56, 128, 128, 128, 2, &D_800EBFF0[D_8013AF78], 8);
        }
        setSpriteColor(128, 128, 128);
        func_800A7D8C(-160, -120, 320, 240, 149, 0x3AB0, &D_800EBFF0[D_8013AF78], 8);
    }
}

void setSecondaryTextEntryTask(TextEntry *entry) {
    TextTaskResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextTaskResource *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL) {
        resource->task = NULL;
    } else {
        func_80014874(func_800A6E88);
        D_8013AE8C->resource14 = allocateTaskResource();
        ((TextTaskResource *)&D_800D9504[D_8013AE8C->resource14])->entry = entry;
        D_8013AE8C->data18 = 0;
        resource->task = D_8013AE8C;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A6E88);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A73BC);

void finishMusicTaskAtDeadline(void) {
    MusicOwnerState *owner;
    TaskResourceRecord *state;
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {}
    }
    owner = (MusicOwnerState *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = &D_800D9504[D_8013AE88->resource14];
    if (owner->musicTask == D_8013AE88) {
        if (owner->tick < state->unk04) {
            return;
        }
        owner->musicTask = NULL;
        if (owner->completionTick < state->unk04) {
            owner->completionTick = state->unk04;
        }
    }
    func_800149A8();
}

void setTextEntryCdAudioTask(TextEntry *entry) {
    CdAudioOwner *owner;
    CdAudioResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (CdAudioOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL || entry->unk06 == 0) {
        SsSetSerialVol(0, 0, 0);
        resetPendingCdRequests();
        owner->task = NULL;
    } else {
        resetPendingCdRequests();
        if (entry->unk14 == 0) SsSetSerialVol(0, 103, 103);
        else SsSetSerialVol(0, entry->unk14, entry->unk14);
        queueCdCommand(gTextEntryCdCommands[entry->unk06], 1);
        func_80014874(func_800A77B0);
        owner->task = D_8013AE8C;
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (CdAudioResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        resource->deadline = owner->tick + entry->unk0A;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A77B0);

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7928);

void stopTextSoundVoices(void) {
    s16 i;
    for (i = 0; i < 8; i++) {
        if (gTextSoundSlots[i].voice != 0xFF) {
            func_800187B8(gTextSoundSlots[i].voice);
            gTextSoundSlots[i].voice = 0xFF;
            gTextSoundSlots[i].soundKey = 0;
        }
    }
}

void beginAudioFadeCountdownTask(void) {
    AudioTransitionState *state;
    s16 i;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (AudioTransitionState *)&D_800D9504[D_8013AE88->resource16];
    for (i = 0; i < 5; i++) {
        state->unk2C[i] = 0;
    }
    gSecondaryResourceDisplayTask = NULL;
    func_8001814C();
    SsSeqSetVol(gMusicSequenceId, 64, 64);
    fadeOutMusicSequence(0x200);
    resetPendingCdRequests();
    SsSetSerialVol(0, 103, 103);
    func_80070274(0, 5, 0);
    D_8013AE88->data18 = 10;
    setCurrentTaskCallback(signalFlag9WhenTaskCountdownExpires);
}

void signalFlag9WhenTaskCountdownExpires(void) {
    if (--D_8013AE88->data18 == 0) {
        D_8013AFE4 |= 0x200;
        func_800149A8();
    }
}

void startUnlockNotification(s16 index) {
    if ((u16)index < 11 && (s16)getUnlockNotificationFlag(index) == 0) {
        D_8013AF3A = index;
        D_8013AFE4 &= ~0x200;
        setUnlockNotificationFlag(index);
        func_80014874(func_800A624C);
    } else {
        D_8013AFE4 |= 0x200;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A7D8C);

int readHostFile(char *path, char *buffer) {
    int length;
    int fd;

    fd = PCopen(path, 0, 0);
    if (fd == -1) {
        printf(D_80012310, path);
    } else {
        length = PClseek(fd, 0, 2);
        PClseek(fd, 0, 0);
        if (PCread(fd, buffer, length) == length) {
            PCclose(fd);
            return 0;
        }
        printf(D_8001232C, path);
    }
    return -1;
}

int writePcHostFile(char *name, char *buffer, int length) {
    int fd;
    fd = PCopen(name, 1, 0);
    if (fd == -1) {
        fd = PCcreat(name, 0);
        if (fd == -1) {
            printf(D_80012310, name);
            return -1;
        }
    }
    if (PCwrite(fd, buffer, length) != length) {
        printf(D_8001232C, name);
        return -1;
    }
    PCclose(fd);
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/main_tail_ui", func_800A8104);
