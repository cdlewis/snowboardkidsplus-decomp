#include "common.h"
#include "game/angles.h"
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/game_task_scheduler.h"
#include "game/race_player.h"
#include "game/race_camera.h"
#include "game/race_time.h"
#include "game/selected_records.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/game_options.h"
#include "game/controller.h"
#include "game/cd_queue.h"
#include "game/controller_effects.h"
#include "game/controller.h"
#include "psx/libsnd.h"

typedef struct RisingPlayerEffectResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    Position3D offset;
    u8 unkAC[4];
    VECTOR scale;
    SVECTOR rotation;
    u8 unkC8[0x14];
    s32 velocity;
    u8 unkE0[0x20];
} RisingPlayerEffectResource;

typedef struct PlayerEffectResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x60];
    VECTOR scale;
    SVECTOR rotation;
    u8 unkC8[0x38];
} PlayerEffectResource;

typedef struct ScaledCoordinateResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x60];
    VECTOR scale;
    u8 unkC0[0x40];
} ScaledCoordinateResource;

typedef struct FallingSpriteEffect {
    s16 active;
    s16 x, y;
    s16 frame;
    s16 unk08;
    s16 velocityX, velocityY;
    s16 unk0E, unk10;
    s16 scale;
    s16 palette;
} FallingSpriteEffect;

typedef struct TimeRecordHeaderResource {
    u8 unk00[6];
    s16 brightness;
    u8 unk08[0xF8];
} TimeRecordHeaderResource;

typedef struct TimeRecordDetailCountdownResource {
    u8 unk00[8];
    s16 timer;
    s16 unk0A;
    u8 unk0C[0xF4];
} TimeRecordDetailCountdownResource;

typedef struct MenuTimeResource {
    s16 selection;
    s16 unk02;
    s16 brightness;
    u8 unk06[0xFA];
} MenuTimeResource;

typedef struct RepeatedSoundResource {
    s16 sound;
    u8 unk02[0xFE];
} RepeatedSoundResource;

typedef struct SpriteAnimationResource {
    s16 x;
    s16 y;
    u16 frame;
    u16 unk06;
    u8 player;
    u8 checkPlayer;
    u8 alternateLayer;
    u8 unk0B[0xF5];
} SpriteAnimationResource;

typedef struct SlidingTimeResource {
    u16 timer;
    u16 unk02;
    s32 x;
    s32 y;
    u8 unk0C[8];
    s32 speed;
    RaceTime time;
    s8 hasExtraTime;
    u8 unk1D[0xE3];
} SlidingTimeResource;

typedef struct SlidingNumberResource {
    u16 timer;
    u16 unk02;
    s32 x;
    s32 y;
    u8 unk0C[8];
    s32 speed;
    s32 value;
    s16 player;
    u8 unk1E[0xE2];
} SlidingNumberResource;

typedef struct UnpackedWordBytes {
    u8 unk0;
    u8 unk1;
    u16 unk2;
} UnpackedWordBytes;


typedef struct MenuSpritePaletteGroups {
    u8 unk00[3];
    u8 kind[8];
    u8 unk0B[9];
} MenuSpritePaletteGroups;

typedef struct PlayerNumberResource {
    u16 timer;
    u16 unk02;
    s32 x;
    s32 y;
    u8 unk0C[8];
    s32 speed;
    s32 value;
    s16 showExtraValue;
    s16 player;
    u8 unk20[0xE0];
} PlayerNumberResource;

typedef struct PanelResource {
    RaceTime time[2];
    s16 brightness;
    s16 timer;
    s16 unk0C;
    s16 unk0E[2];
    u8 unk12[0xEE];
} PanelResource;

/* The fade callbacks use a full task resource record. */
typedef struct TaskFadeResource {
    s16 brightness;
    s16 timer;
    s8 playerOrder[4];
    u8 unk08[0xF8];
} TaskFadeResource;

typedef struct ResultDisplayResource {
    s16 brightness;
    s16 timer;
    s16 unk04;
    s16 unk06;
    s16 unk08;
    u8 unk0A[0xF6];
} ResultDisplayResource;

typedef struct PanelInputResource {
    u8 unk00[0xA];
    s16 timer;
    u8 unk0C[0xF4];
} PanelInputResource;

typedef struct StatsMenuFadeResource {
    s16 unk00;
    s16 brightness;
    s16 timer;
    s16 unk06;
    u8 unk08[0xF8];
} StatsMenuFadeResource;

typedef struct RaceResultsFadeResource {
    u8 unk00[6];
    s16 brightness;
    s16 timer;
    s16 unk0A;
    u8 unk0C[0xF4];
} RaceResultsFadeResource;

typedef struct TaskMenuFadeResource {
    s16 unk00;
    s16 brightness;
    s16 secondaryBrightness;
    s16 timer;
    u8 unk08[0xF8];
} TaskMenuFadeResource;

typedef struct MenuCountdownResource {
    s16 unk00, unk02;
    s16 timer;
    s16 unk06;
    u8 unk08[0xF8];
} MenuCountdownResource;

typedef struct MenuFadeInResource {
    s16 unk00;
    s16 brightness;
    s16 timer;
    u8 unk06[0xFA];
} MenuFadeInResource;

typedef struct SlidingDisplayResource {
    s32 position;
    u8 unk04[0xC];
    s32 velocity;
    u16 timer;
    u8 unk16[0xEA];
} SlidingDisplayResource;


typedef struct TaskMotionResource {
    s32 position;
    u8 unk04[12];
    s32 velocity;
    u16 delay;
    u8 unk16[0xEA];
} TaskMotionResource;

typedef struct SpriteMotionResource {
    u16 timer;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    s32 unk10;
    s32 unk14;
    s16 variant;
    u8 padding[0xE6];
} SpriteMotionResource;

typedef struct SlidingVisualResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x24];
    s32 unk74;
    u8 unk78[0x50];
    s16 timer;
    s16 framePosition;
    u8 r0, g0, b0, code;
    s32 step;
    s32 velocity;
    u8 unkD8[0x28];
} SlidingVisualResource;

typedef struct PlayerCameraView {
    s32 x, y, z;
    s32 targetX, targetY, targetZ;
    s32 unk18, unk1C;
} PlayerCameraView;

const char gDemonstrationText[16]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4))) = "DEMONSTRATION";
const char gDemonstrationStartText[12]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4))) = "PUSH START";

PlayerCameraView gPlayerCameraViews[2]
    __attribute__((section(".data.player_camera_views"))) = {0};

MenuSpritePaletteGroups gMenuSpritePaletteGroups
    __attribute__((section(".data.menu_sprite_palettes"))) = {
        {2, 2, 3},
        {3, 5, 6, 5, 5, 4, 0, 0},
        {0, 5, 0, 1, 4, 0, 5, 0, 0}
    };

/* Each input sequence ends with 0xFFFF. */
u16 gControllerInputSequences[3][26]
    __attribute__((section(".data.controller_input_sequences"))) = {
    {0x0004, 0x0008, 0x0002, 0x0001, 0x0004, 0x0008, 0x0002, 0x0001, 0x0100, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x1000, 0x4000, 0x8000, 0x2000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0100, 0xFFFF},
    {0x8000, 0x1000, 0x2000, 0x4000, 0x1000, 0x1000, 0x1000, 0x4000, 0x4000, 0x4000, 0x0100, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
};

extern u8 D_8013B11A;
/* Assembly and C consumers address this palette subtable directly. */
extern u8 gMappedIconPalettes[9];

extern s16 D_8013B10C;
extern u8 D_80123640[];

extern s16 D_8013B0E4;
extern u16 D_8013B0E6;
extern s32 D_8013B0E8;
extern s32 D_8013AF84 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));

void func_800335E4(SlidingTimeResource *resource);
void func_800497B8(void);
s32 func_80043ACC(const RaceTime *a, const RaceTime *b, RaceTime *difference);

void func_80037850(const RaceTime *value, s16 x, s16 y, s16 mode, GsOT *ot, u16 depth);

/* Only the first byte of this shared state is used by this task flow. */
extern u8 D_80120F90[];
extern const char D_80010A94[];

RaceTime gRaceElapsedTime = {0, 0, 0};

extern int rand(void);

void uploadTimImageAt(s16 x, s16 y, s16 clutX, s16 clutY, u32 *tim);
s16 func_8004A124(s32 x1, s32 z1, s32 x2, s32 z2);
s32 func_8004A4C4(s16 index, s32 x, s32 z);
s32 func_8004A764(s16 index, s32 x, s32 z);
u8 func_8004AAA0(s16 index, s32 x, s32 z);
void func_8003E1D0(u16 playerIndex);
void func_8003E60C(u16 index);
void func_8003EE70(void);
void func_8002F5F8(void);
void func_8003DF3C(u16 index);
void func_8003DEE0(void);
void func_80032F84(void);
void func_80033404(void);
void func_80033C2C(void);
void func_80044990(void);
void updateOneShotTaskVisual(void);
void func_80034C40(SpriteMotionResource *resource);
void slideInSpriteMotionTask(void);
void waitSpriteExitMotionTask(void);
void slideInSlidingDisplayTask(void);
void func_8002FCF8(void);
void func_8002FE18(void);
void func_800302F8(void);
void func_80032C14(void);
void waitUnlockNotification0TransitionTask(void);
void func_80032F3C(void);
void func_8002F1C8(void);
void func_8002FA58(s16 resource);
void initializePlayerNumberDisplayTask(void);
void slideOutPlayerNumberDisplayTask(void);
void func_80034664(PlayerNumberResource *resource);
void slideOutSlidingNumberDisplayTask(void);
void func_8003543C(void);
void func_800350AC(SlidingNumberResource *resource);
void advancePlayerSpriteAnimationTask(void);
void func_80034434(SpriteAnimationResource *resource);
void slideOutSlidingDisplayTask(void);
void func_80034090(SlidingDisplayResource *resource);
void slideRaceTimeDisplayOut(void);
void func_80033100(void);
void func_80048714(void);
void func_80049240(void);
void func_800458F8(void);
void func_8004531C(void);
void func_80044524(void);
void func_80046044(void);
void func_80046404(TaskResourceRecord *resource, TaskResourceRecord *secondary, s32 value, s32 scale);
void func_80047600(void);
void func_80047D94(void);
void func_80046CB4(void);
void func_80037424(TaskResourceRecord *resource);
void func_80036BA4(TaskResourceRecord *resource);
void func_80037354(TaskResourceRecord *resource);
void func_8003763C(void);
void fadeInPlayerResultsTask(void);
void fadeInLapTimeLabelTask(void);
void func_80037EC8(void);
void drawTaskTimeMenuAndSetFlag10OnInput(void);
void func_8003A814(void);
void func_8003A158(void);
void func_80039484(RaceResultsFadeResource *resource);
void func_80039564(RaceResultsFadeResource *resource);
void func_80039B34(RaceResultsFadeResource *resource);
void func_80036940(void);
void func_80035D5C(ResultDisplayResource *resource);
void func_800361C4(ResultDisplayResource *resource);
void func_8003772C(void);
void func_80037F74(void);
void func_80037D90(void);
void drawMenuTimeWithBrightness(MenuTimeResource *resource);
void func_800379B8(TaskResourceRecord *resource);
void drawLapTimeLabel(TaskMenuFadeResource *resource);
void func_80038090(TaskResourceRecord *resource);
void func_80038270(TaskResourceRecord *resource);
void func_80038C70(void);
void func_80038574(TaskResourceRecord *resource);
void func_8003881C(TaskResourceRecord *resource);
void func_80039144(void);
void waitTaskMenuCountdownAndPlaySound(void);
void func_8003BE58(TaskResourceRecord *resource);
void func_8003C5D8(void);
void advanceCurrentTaskPanelTimer(void);
void func_8003A3D8(void);
void func_8003A760(void);
void drawTimeRecordHeader(TimeRecordHeaderResource *resource);
void func_80039564(TaskResourceRecord *resource);
void func_80039B34(TaskResourceRecord *resource);
void func_800398F4(TaskResourceRecord *resource);
void func_8003AAE0(TaskResourceRecord *resource);
void func_8003AD48(TaskResourceRecord *resource);
void func_8003B020(TaskResourceRecord *resource);
void func_8003B2BC(TaskResourceRecord *resource);
void func_8003B94C(void);
void advanceTaskStatsCountdownAndPlaySound(void);
void resetTaskStatsCountdown(void);
void func_8003B020(TaskResourceRecord *resource);
void func_8003B690(void);
void func_8003B434(void);
void func_8003BC18(void);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002B3F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002B820);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002BC2C);

s16 arePositionsWithinXZRange(const Position3D *a, const Position3D *b, s32 threshold) {
    s32 dx = a->x - b->x;
    s32 dz = a->z - b->z;

    if (dx < 0) {
        dx = -dx;
    }
    if (dz < 0) {
        dz = -dz;
    }
    if (dx < threshold && dz < threshold) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002BF88);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002C2D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002C520);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002C738);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002C8BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CA48);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CB8C);

s32 requestCurrentPlayerAction6(void) {
    RacePlayer *player = D_8013B0D8;
    s16 angle;
    s32 difference;

    if (player->unk128 & 0x379000) {
        return 1;
    }
    if (!(player->unk128 & 0x400000)) {
        difference = D_8013B0E6 - player->unk110;
        angle = difference;
        if (!(player->unk128 & 0x400)) {
            angle += 0x800;
        }
        angle = (angle + 0x200) & 0xFFF;
        if (D_8013B0E8 < 0xC00 || angle > 0x400) {
            return 1;
        }
    }
    if (D_8013B0E4 < 6) {
        D_8013B0E4 = 6;
    }
    return 0;
}

s32 requestCurrentPlayerAction6NarrowAngle(void) {
    RacePlayer *player = D_8013B0D8;
    s16 angle;
    s32 difference;

    if (player->unk128 & 0x379000) {
        return 1;
    }
    if (!(player->unk128 & 0x400000)) {
        difference = D_8013B0E6 - player->unk110;
        angle = difference;
        if (!(player->unk128 & 0x400)) {
            angle += 0x800;
        }
        angle = (angle + 0x180) & 0xFFF;
        if (D_8013B0E8 < 0xC00 || angle > 0x300) {
            return 1;
        }
    }
    if (D_8013B0E4 < 6) {
        D_8013B0E4 = 6;
    }
    return 0;
}

s32 requestCurrentPlayerAction7(void) {
    RacePlayer *player = D_8013B0D8;
    s16 angle;
    s32 difference;

    if (player->unk128 & 0x379000) {
        return 1;
    }
    if (!(player->unk128 & 0x400000)) {
        difference = D_8013B0E6 - player->unk110;
        angle = difference;
        if (!(player->unk128 & 0x400)) {
            angle += 0x800;
        }
        angle = (angle + 0x200) & 0xFFF;
        if (D_8013B0E8 < 0xC00 || angle > 0x400) {
            return 1;
        }
    }
    if (D_8013B0E4 < 7) {
        D_8013B0E4 = 7;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CE58);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CE9C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CEE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CF24);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CF68);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CFAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CFF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D040);

s32 checkCurrentPlayerFlags43040OrAngleRange(void) {
    RacePlayer *player = D_8013B0D8;
    s32 difference;
    s32 angle;
    if (player->unk128 & 0x43040) {
        return 1;
    }
    difference = D_8013B0E6 - player->unk110;
    if (player->unk128 & 0x400) {
        angle = difference;
    } else {
        angle = 0x800;
        angle += difference;
    }
    angle = (angle + 0x400) & 0xFFF;
    if ((D_8013B0E8 < 0x500) || (angle > 0x800)) {
        return 1;
    }
    if (D_8013B0E4 < 2) {
        D_8013B0E4 = 2;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D120);

s32 checkCurrentPlayerFlags345001(void) {
    RacePlayer *player = D_8013B0D8;
    if (player->unk12C != 14 && (player->unk128 & 0x345001)) {
        return 1;
    }
    if (D_8013B0E4 < 15) {
        D_8013B0E4 = 15;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D1B8);

s32 requestCurrentPlayerAction17(void) {
    RacePlayer *player = D_8013B0D8;

    if (player->unk128 & 0x275000) {
        return 1;
    }
    if (player->unk0F6 != 0) {
        return 1;
    }
    if (player->unk134 != 0) {
        return 1;
    }
    if (D_8013B0E4 < 17) {
        D_8013B0E4 = 17;
    }
    return 0;
}

s32 checkCurrentPlayerFlags275000OrField134(void) {
    RacePlayer *player = D_8013B0D8;
    if (player->unk128 & 0x275000) {
        return 1;
    }
    if (player->unk134) {
        return 1;
    }
    if (D_8013B0E4 < 18) {
        D_8013B0E4 = 18;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D2B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D2F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D33C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D380);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D3C4);

s16 checkCurrentPlayerSurfaceType(void) {
    RacePlayer *player = D_8013B0D8;
    u8 surface = func_8004AAA0(player->unk92E, player->x >> 8, player->z >> 8);
    if (surface == 3 || surface == 6 || surface == 7 || surface == 8 ||
        surface == 12 || surface == 14 || surface == 15 || surface == 16) {
        return 0;
    }
    return 1;
}

s16 countCurrentPlayerInputGroups(void) {
    s32 count = 0;
    u16 buttons;
    if (D_8013B0D8->unk008) {
        return (randomNextMain() & 3) == 0;
    }
    buttons = D_8013AEE8[D_8013AE88->data18];
    if (buttons & 0xF000) {
        count = 1;
    }
    if (buttons & 0x40) {
        count++;
    }
    return count;
}

void clearCurrentPlayerTaskState(void) {
    GameTask *task = D_8013AE88;
    RacePlayer *player = D_8013B0D8;

    task->data1A = 0;
    task->data1C = 0;
    task->data1E = 0;
    player->unk0C2 = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002DEA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002DEE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E1B0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E368);

void func_8002E3C0(void) {
}

void addPlayerCappedCounters(u16 playerIndex, s32 amount) {
    RacePlayer *players = D_80120E90;
    RacePlayer *player;
    players += playerIndex;
    player = players;
    if (player->unk024) {
        players = player;
        players->unk01C += amount;
        if (players->unk01C > 99999) {
            player->unk01C = 99999;
        }
        player->unk020 += amount;
        if (players->unk020 > 99999) {
            player->unk020 = 99999;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E44C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E5A0);

void startPlayerTimerTask(RacePlayer *player) {
    player->unk0FC = 240;
    if (player->unk128 & 0x400) {
        player->unk0FF = 1;
    } else {
        player->unk0FF = 0;
    }
    func_80014930(player->task, initPlayerTimerTask);
    D_8013AE8C->data18 = player->playerIndex;
}


INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E644);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002EA0C);

void loadCurrentPlayerModelBlock(void) {
    u32 *offsets = (u32 *)0x8017D004;
    u8 *archive = (u8 *)0x8017D000;
    u32 (*buffers)[512] = (u32 (*)[512])0x8017B000;
    u32 i = 0;
    u16 player = D_8013AE88->data18;
    u32 *dest;
    u32 offset;
    u32 *source;
    u32 *model;
    offset = offsets[D_80123640[D_80120E90[player].unk015[1]]];
    model = buffers[player];
    source = (u32 *)(offset + archive);
    dest = model;
    for (; i < 512; i++) {
        *dest++ = *source++;
    }
    GsMapModelingData((u32 *)((u32)model | 4));
    {
        u32 (*largeBuffers)[4096] = (u32 (*)[4096])0x8016B000;
        i = 3;
        dest = model;
        model = largeBuffers[D_8013AE88->data18];
        {
            u32 *secondSource = dest + 3;
            dest = model + 3;
            for (; i < 10; i++) {
                *dest++ = *secondSource++;
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002EE0C);

void resetGameOptionState(void) {
    D_8013B108 = 2;
    D_8013B109 = 0;
    D_8013B10A = 0;
    gGameOptionFlags = 0x3E;
}

void queueBackgroundTextureSectorCommands(void) {
    queueCdSectorCommand(2, 0x480, 0x4A, (u8 *)0x80180000, 0);
    queueCdSectorCommand(0, 0xC9B, 1, (u8 *)0x80180000, 0);
    queueCdSectorCommand(0, 0xC9C, 1, (u8 *)0x80180800, 0);
    queueCdSectorCommand(0, 0xC78, 13, (u8 *)0x80181000, 0);
    queueCdSectorCommand(0, 0xC85, 4, (u8 *)0x80187800, 0);
}

void uploadBufferedTexturePair(void) {
    uploadTimImageWithOffset((u32 *)0x80187800, NULL);
    uploadTimImageWithOffset((u32 *)0x80181000, NULL);
}

void uploadTextureArchiveEntry(s16 x, s16 y, s16 clutX, s16 clutY, s32 index) {
    OffsetArchive *archive = (OffsetArchive *)0x80189800;
    s32 offset;
    /* The archive word index includes the count and wraps as a signed halfword. */
    index = (s16)index + 1;
    index = (s16)index;
    offset = (s32)archive->offsets[index - 1];
    uploadTimImageAt(x, y, clutX, clutY,
                  (u32 *)((s16 *)archive + (offset >> 1)));
}

void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map) {
    func_80071300(mode, unused, background, map,
                  (u8 *)0x80180000, (u8 *)0x80180800);
}

void queueStartupSectorReads(void) {
    queueCdSectorCommand(0, 1, 5, (u8 *)0x8017D000, 0);
    queueCdSectorCommand(0, 6, 18, (u8 *)0x801F0000, 0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F394);

void uploadTimImageAt(s16 x, s16 y, s16 clutX, s16 clutY, u32 *tim) {
    GsIMAGE image;
    if (*tim == 0x10) {
        GsGetTimInfo(tim + 1, &image);
        image.px = x;
        image.py = y;
        LoadImage((RECT *)&image.px, image.pixel);
        if ((image.pmode >> 3) & 1) {
            image.cx = clutX;
            image.cy = clutY;
            LoadImage((RECT *)&image.cx, image.clut);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F468);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F548);

void initializeViewAndSelectTaskCallback(void) {
    if (D_8013AE88->data1E != 0) {
        D_8013AF84 = 0x100;
        func_8003DF3C(0);
        func_8003DEE0();
        if (D_8013AFD7 != 0) {
            goto restore;
        }
    } else {
restore:
        func_800149A8();
        return;
    }
    setCurrentTaskCallback(func_8001DB7C);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F5F8);

void beginMode2TaskSequence(void) {
    func_80070274(2, 0, 30);
    setCurrentTaskCallback(func_8002F5F8);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F65C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F698);

void beginMode2SequenceAndFadeMusic(void) {
    func_80070274(2, 0, 30);
    if (D_8013B106 == 1) {
        fadeOutMusicSequence(0x200);
        D_8013B106 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002FA58);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002FCF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002FE18);

void initializeMode11TransitionTask(void) {
    func_80070274(11, 0, 30);
    func_800719A8(1);
    func_8002FCF8();
    setCurrentTaskCallback(func_8002FE18);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002FFC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80030108);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003012C);

void waitMode2TransitionTask(void) {
    switch (D_8013AE88->data18) {
    case 0:
        func_80070274(2, 0, 30);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800302F8);

void drawFullScreenTexturedBackground(void) {
    setSpriteColor(128, 128, 128);
    func_800A7D8C(-160, -120, 320, 240, 0x85, 0x39F0,
                  &D_800EBFF0[D_8013AF78], 0xFFF);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800309EC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80030B50);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80031728);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800318C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80031B90);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80031C98);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032280);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003240C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800326FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003288C);

void createRepeatedSoundTask(s32 sound, s32 delay, s32 interval, s32 count) {
    RepeatedSoundResource *resource;

    func_80014874(func_80032C14);
    D_8013AE8C->data18 = delay;
    D_8013AE8C->data1A = interval;
    D_8013AE8C->data1C = 0;
    D_8013AE8C->data1E = count;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RepeatedSoundResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->sound = sound;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032C14);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032D48);

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
        func_80070274(11, 0, 30);
        func_800719A8(1);
        D_8013B10C = -1;
        func_8002FCF8();
        func_8002F1C8();
        func_8002FA58((s16)D_8013AE98->data18);
        setCurrentTaskCallback(func_80032F3C);
        D_8013AE88->unk10 |= 2;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032F3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032F84);

void beginSound601TaskSequence(void) {
    playSoundEffect(0, 0x601, 0x20);
    setCurrentTaskCallback(func_80032F84);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033100);

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


INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800332F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033404);

void waitThenStartSpriteScaleTransition(void) {
    GameTask *task = D_8013AE88;

    if (--task->data1A == 0) {
        task->data1E = 0x200;
        task->data1C = 0;
        task->data1A = 60;
        setCurrentTaskCallback(func_80033404);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800335E4);

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
        func_800335E4(resource);
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
    func_800335E4(resource);
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
    func_800335E4(resource);
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
    resource->hasExtraTime = func_80043ACC(&D_8013B188, &D_80124050[D_8013AFCE], &resource->time);
    setCurrentTaskCallback(slideRaceTimeDisplayIn);
}

void drawFallingSpriteEffect(FallingSpriteEffect *effect) {
    setSpriteColor(128, 128, 128);
    func_8006CF38(effect->x >> 4, effect->y >> 4, effect->frame + 378,
                  effect->palette, -1, 0x7FFF, 0x7FFF, effect->scale,
                  effect->scale, 0, &D_800EBFF0[D_8013AF78], 2);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033A24);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033B5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033C2C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033FAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033FE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034028);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034090);

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
        func_80034090(resource);
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
    func_80034090(resource);
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
    func_80034090(resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034434);

void advancePlayerSpriteAnimationTask(void) {
    SpriteAnimationResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteAnimationResource *)&D_800D9504[D_8013AE88->resource14];
    if (++resource->frame >= 16) {
        func_800149A8();
    } else {
        func_80034434(resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034664);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034C40);

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
        func_80034C40(resource);
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
    func_80034C40(resource);
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
    func_80034C40(resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003504C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800350AC);

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
        func_800350AC(resource);
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
    func_800350AC(resource);
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
    func_800350AC(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003543C);

void createActivePlayerSlidingNumberTask(s32 value, s16 player) {
    SlidingNumberResource *resource;
    if (D_80120E90[player].unk024 != 0) {
        func_80014874(func_8003543C);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SlidingNumberResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->value = value;
        resource->player = player;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800355D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800356FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035808);

void drawFixedSpriteAndBottomBarTask(void) {
    addTexturedSprite(D_8013AF6C, 0x1C, 0x3DFE, 0x28, 0x52,
                      0x68, 0x18, 8, 0xD8, 0x80, 0x80, 0x80);
    addFlatRectangle(D_8013AF6C + 1, -(D_8013AF7A >> 1),
                     (s16)((D_8013AF7C >> 1) - 0x20),
                     (s16)D_8013AF7A, 0x20, 0, 0, 0);
}

void drawDemonstrationOverlay(void) {
    setSpriteColor(128, 128, 128);
    func_8006F57C(-52, -8, 20, gDemonstrationText, 1, -1, &D_800EBFF0[D_8013AF78], 3);
    if (gFrameCounter & 8) {
        func_8006F57C(-40, 28, 20, gDemonstrationStartText, 2, -1, &D_800EBFF0[D_8013AF78], 3);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035B24);

void checkControllerInputSequenceTask(void) {
    if (D_8013AEE8[0] != 0) {
        if (D_8013AEE8[0] == gControllerInputSequences[D_8013AE88->data18][D_8013AE88->data1A]) {
            D_8013AE88->data1A++;
            if (gControllerInputSequences[D_8013AE88->data18][D_8013AE88->data1A] == 0xFFFF) {
                D_8013B11A |= 1 << D_8013AE88->data18;
                func_800181B0(0, 0, 0x20);
                func_800149A8();
            }
        } else {
            D_8013AE88->data1A = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035CF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035D5C);

void drawResultNumberAndIcon(ResultDisplayResource *resource) {
    setSpriteColor(128, 128, 128);
    func_8006DFB4(32, -63, 5, 0, resource->unk04, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, -63, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035FAC);

void drawFirstPlayerResultNumberAndIcon(void) {
    setSpriteColor(128, 128, 128);
    func_8006DFB4(24, 33, 6, 0, D_80120E90[0].unk010, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, 33, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800361C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800362FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800363A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036454);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800366C0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036790);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003686C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036940);

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
        setCurrentTaskCallback(func_80036940);
    }
    func_80035D5C(resource);
    func_800361C4(resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036BA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037074);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037354);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037424);

void drawCurrentTaskPlayerResults(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        func_800149A8();
    }
    func_80037424(resource);
    func_80036BA4(resource);
    func_80037354(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003763C);

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
        setCurrentTaskCallback(func_8003763C);
    }
    func_80037424((TaskResourceRecord *)resource);
    func_80036BA4((TaskResourceRecord *)resource);
    func_80037354((TaskResourceRecord *)resource);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037850);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800379B8);

void drawMenuTimeWithBrightness(MenuTimeResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    func_80037850(&gRaceElapsedTime, -108, -48, 1, &D_800EBFF0[D_8013AF78], 1);
}

void drawLapTimeLabel(TaskMenuFadeResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness,
                       (u8)resource->brightness,
                       (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    func_8006D41C(-116, -60, 20, D_80010A94, 5, -1,
                 &D_800EBFF0[D_8013AF78], 1);
}

void drawCurrentTaskTimeMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    drawMenuTimeWithBrightness((MenuTimeResource *)resource);
    func_800379B8(resource);
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
    func_800379B8(resource);
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
        setCurrentTaskCallback(func_80037EC8);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038090);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038270);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038574);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003881C);

void drawCurrentTaskMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    func_80038090(resource);
    func_80038574(resource);
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
    func_80038574(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038B34);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038C70);

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
    func_8003881C((TaskResourceRecord *)resource);
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
    func_8003881C((TaskResourceRecord *)resource);
}

void startTaskMenuCountdownAndPlaySound(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 2;
    if (--resource->timer == 0) {
        func_800181B0(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(resetTaskMenuCountdown);
    }
    func_80038090((TaskResourceRecord *)resource);
    func_80038270((TaskResourceRecord *)resource);
    func_8003881C((TaskResourceRecord *)resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003908C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039144);

void waitTaskMenuCountdownAndPlaySound(void) {
    MenuFadeInResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        resource->timer = 20;
        func_800181B0(0, 0x700, 0x20);
        setCurrentTaskCallback(func_80039144);
    }
    func_80038090((TaskResourceRecord *)resource);
    func_8003881C((TaskResourceRecord *)resource);
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
    func_8003881C((TaskResourceRecord *)resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039360);

void drawTimeRecordHeader(TimeRecordHeaderResource *resource) {
    u8 padding[0x20];
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    func_8006D41C(-112, -64, 20, D_80010A94, 5, -1,
                 &D_800EBFF0[D_8013AF78], 1);
    func_80037850(&gRaceElapsedTime, -104, -55, 1,
                 &D_800EBFF0[D_8013AF78], 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039564);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800398F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039B34);

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
    func_80039564(resource);
    func_80039B34(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A094);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A158);

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
    func_80039484(resource);
    func_80039564(resource);
    func_80039B34(resource);
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
        setCurrentTaskCallback(func_8003A3D8);
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
        func_800181B0(0, 0x700, 0x20);
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
        func_800181B0(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(updateTaskTimeRecordCountdownAndPlaySound);
    }
    drawTimeRecordHeader((TimeRecordHeaderResource *)resource);
    func_80039564((TaskResourceRecord *)resource);
    func_80039B34((TaskResourceRecord *)resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A690);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A760);

void waitTaskTimeRecordCountdownAndPlaySound(void) {
    TimeRecordDetailCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimeRecordDetailCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        resource->timer = 20;
        func_800181B0(0, 0x700, 0x20);
        setCurrentTaskCallback(func_8003A760);
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
        setCurrentTaskCallback(func_8003A814);
    }
    func_80039484(resource);
    func_80039B34(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A96C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003AAE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003AD48);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B020);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B2BC);

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
    func_8003B020(resource);
}

void updateStatsMenuExitInputTask(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(func_8003B434);
    }
    func_8003AAE0(resource);
    func_8003B020(resource);
}

void drawCurrentTaskStatsMenu(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    func_8003AAE0(resource);
    func_8003AD48(resource);
    func_8003B2BC(resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B5CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B690);

void waitForStatsScoreCountTask(void) {
    StatsMenuFadeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (StatsMenuFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 3;
    if (--resource->timer == 0) {
        setCurrentTaskCallback(func_8003B690);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    func_8003B2BC((TaskResourceRecord *)resource);
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
    func_8003B2BC((TaskResourceRecord *)resource);
}

void startTaskStatsCountdownAndPlaySound(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 2;
    if (--resource->timer == 0) {
        func_800181B0(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(func_8003B94C);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    func_8003B2BC((TaskResourceRecord *)resource);
}

void advanceTaskStatsCountdownAndPlaySound(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 1;
    if (--resource->timer == 0) {
        func_800181B0(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(startTaskStatsCountdownAndPlaySound);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    func_8003B2BC((TaskResourceRecord *)resource);
}

void resetTaskStatsCountdown(void) {
    MenuCountdownResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuCountdownResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk06 = 0;
    if (--resource->timer == 0) {
        func_800181B0(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(advanceTaskStatsCountdownAndPlaySound);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003AD48((TaskResourceRecord *)resource);
    func_8003B2BC((TaskResourceRecord *)resource);
}

void waitTaskStatsCountdownAndPlaySound(void) {
    MenuFadeInResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (MenuFadeInResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        resource->timer = 20;
        func_800181B0(0, 0x700, 0x20);
        setCurrentTaskCallback(resetTaskStatsCountdown);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003B2BC((TaskResourceRecord *)resource);
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
        setCurrentTaskCallback(func_8003BC18);
    }
    func_8003AAE0((TaskResourceRecord *)resource);
    func_8003B2BC((TaskResourceRecord *)resource);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BD70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BE58);

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
        setCurrentTaskCallback(func_8003C5D8);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C7F0);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003CAF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003D904);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003D940);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DB94);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DEE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DF3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E050);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E1D0);

void updatePlayerCameraAngles(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    RacePlayer *players = D_80120E90;
    RacePlayer *player = &players[playerIndex];
    camera->yaw = func_8004A124(camera->x, camera->z, player->x, player->z);
    func_8003E1D0(playerIndex);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E3F0);

void clampPlayerCameraHeight(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    PlayerCameraView *views = gPlayerCameraViews;
    PlayerCameraView *view = &views[playerIndex];
    s32 height;
    camera->unk18 = func_8004A4C4(camera->unk18, view->x, view->z);
    height = func_8004A764(camera->unk18, view->x, view->z);
    if (height - 120 < view->y) {
        view->y = height - 120;
        camera->y = height << 8;
        func_8003E1D0(playerIndex);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E60C);

void initPlayerZeroTask(void) {
    func_8003E60C(0);
    setCurrentTaskCallback(func_8003EE70);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E794);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003EA20);

void updateCurrentPlayerCameraTarget(void) {
    PlayerPlayerCameraView *targets = gPlayerCameraViews;
    RacePlayer *players = D_80120E90;
    targets[D_8013AE88->data18].targetX = players[D_8013AE88->data18].unk8E4 + players[D_8013AE88->data18].unk084;
    targets[D_8013AE88->data18].targetY = players[D_8013AE88->data18].unk8E8 + (s16)(D_801241E8[D_8013AE88->data18].unk14 / 8 - 120) + players[D_8013AE88->data18].unk086;
    targets[D_8013AE88->data18].targetZ = players[D_8013AE88->data18].unk8EC + players[D_8013AE88->data18].unk088;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003EE70);

void addFourWordGpuPacket(GpuPacket4 *source, GsOT *ot, u16 depth) {
    GpuPacket4 *packet = (GpuPacket4 *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800400C4);

void addTexturedSpritePacket(TexturedSpritePacket *source, GsOT *ot, u16 depth) {
    TexturedSpritePacket *packet = (TexturedSpritePacket *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800401DC);

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
    func_8006C998(x, y, sprite, frame, -1, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800402C8);

void drawPaletteMappedIcon(s16 x, s16 y, s16 index, struct GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        palette = gMappedIconPalettes[index];
        sprite = index + 461;
    }
    func_8006C998(x, y, sprite, palette, -1, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80040440);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004054C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80040A60);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80040D5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80041554);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80041890);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800418F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80041DC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004213C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80042558);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80042894);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80042DC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043120);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800435AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043810);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043A4C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043ACC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043C28);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043C5C);

void resetControllersAndDeleteCurrentTask(void) {
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetControllerEffects();
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043FBC);

s16 isFirstPlayerUsingCallback26D10(void) {
    return D_80120E90[0].task->callback == updateCurrentPlayerTaskCountdown;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800442F4);

void initializeTaskCoordinateResource(void) {
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    GsInitCoordinate2(0, (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource14]);
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 1;
    setCurrentTaskCallback(func_80044524);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044524);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004463C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044768);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044A38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044BC0);

void resetTaskVisualState(TaskVisualResource *resource) {
    resource->unkC6 = 0x400;
    resource->unkC8 = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->framePosition = 0;
    func_80018538(resource->position.x, resource->position.y, resource->position.z,
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044EC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800450BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004531C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800456DC);

void initializeScaledTaskCoordinateResource(void) {
    ScaledCoordinateResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ScaledCoordinateResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0xC00;
    setCurrentTaskCallback(func_800458F8);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800458F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80045B4C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80045C7C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046044);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046228);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046404);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046708);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046894);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046AEC);

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
    func_80046404(resource, &D_800D9504[D_8013AE88->resource16], 300, 0x5580);
    setCurrentTaskCallback(func_80046CB4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046CB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046E30);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046EF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047074);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004713C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800472B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047380);

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
    func_80046404(resource, &D_800D9504[D_8013AE88->resource16], 300, 0x4C80);
    setCurrentTaskCallback(func_80047600);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047600);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004777C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047A3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047BAC);

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
    func_80046404(resource, &D_800D9504[D_8013AE88->resource16], 300, 0x5100);
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(func_80047D94);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047D94);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800481A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048340);

void initPlayerTimerTask(void) {
    PlayerTimerState *timer = &D_80120E90[D_8013AE88->data18].timer;

    GsInitCoordinate2(0, &timer->coordinate);
    timer->unk60 = 0;
    D_80120E90[D_8013AE88->data18].unk0FE = 1;
    setCurrentTaskCallback(func_80048714);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048714);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800489A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048AA4);

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
        func_800181B0(1, 0xA00, 0x30);
        setCurrentTaskCallback(func_80049240);
    } else {
        D_8013AE88->data1C--;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80049240);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80049404);

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
    func_800181B0(1, 0xD01, 0x20);
    D_8013AE88->data1C = 0;
    setCurrentTaskCallback(func_800497B8);
}
