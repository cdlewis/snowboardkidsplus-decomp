#include "common.h"
#include "game/race/player/race_player.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_geometry.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_offset.h"
#include "game/graphics/quad_animation.h"
#include "game/engine/angles.h"
#include "game/engine/random.h"
#include "game/audio/music.h"
#include "game/save/save_data.h"
#include "game/engine/game_options.h"
#include "game/engine/game_loop.h"
#include "game/menu/selected_records.h"
#include "psx/libapi.h"
#include "psx/libetc.h"
#include "psx/scratchpad.h"
#include "player_update_internal.h"
#include "game/io/dat_resources.h"
#include "game/io/controller.h"
#include "../race_presentation_internal.h"
#include "../race_internal.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "psx/libgs.h"
#include "psx/libgte.h"


typedef struct PlayerState2Trick {
    s16 sound;
    s16 state;
    s16 trickIndex;
} PlayerState2Trick;

typedef struct PlayerStartPlacement {
    VECTOR position;
    s16 recordIndex;
    s16 heading;
} PlayerStartPlacement;

typedef struct PlayerVariantAttributeTable {
    u16 values[15][9];
    u16 padding;
} PlayerVariantAttributeTable;

extern SelectedRecord *D_8013B1F0 __attribute__((section(".data")));
extern s16 D_8013B1FE __attribute__((section(".data")));
extern s16 *D_8013B200 __attribute__((section(".data")));
extern s16 *D_8013B204 __attribute__((section(".data")));
extern s16 *D_8013B208 __attribute__((section(".data")));
extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFD9 __attribute__((section(".data")));
extern u8 D_8013AFE1 __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 D_8013AFCE __attribute__((section(".data")));
extern u8 D_8013AFDB __attribute__((section(".data")));
extern s16 D_8013AFDE __attribute__((section(".data")));
extern VECTOR *D_8013AF88 __attribute__((section(".data")));
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern MATRIX *D_8013AF98 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));
extern u8 D_8013AFD7 __attribute__((section(".data")));
extern u8 D_8013AFD6 __attribute__((section(".data")));

extern VECTOR *D_8013AF8C __attribute__((section(".data")));

extern s16 D_8013B0E4 __attribute__((section(".data")));
extern u16 D_8013B0E6 __attribute__((section(".data")));
extern s32 D_8013B0E8 __attribute__((section(".data")));

PlayerVariantAttributeTable gPlayerVariantAttributes
    __attribute__((section(".data.player_variant_attributes"), aligned(4))) = {
    {
        {0x2B60, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2A00, 0x001F, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2C00, 0x001F, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
        {0x2A60, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2C00, 0x001F, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2D00, 0x001F, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
        {0x2E00, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2E00, 0x001F, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2F00, 0x001F, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
        {0x2E80, 0x03E8, 0x00C0, 0x0FB0, 0x0200, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2B00, 0x0000, 0x0000, 0x0BF0, 0x0200, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2F00, 0x001F, 0x00D0, 0x0CD0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x2CF0, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2EA9, 0x002E, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2F2A, 0x003E, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
    },
    0,
};

u16 gPlayerCharacterAttributes[10][7]
    __attribute__((section(".data.player_character_attributes"), aligned(4))) = {
    {0x02F0, 0x0700, 0x00F9, 0x0B90, 0x00BF, 0x0008, 0x0020},
    {0x00D0, 0x0500, 0x00CF, 0x0CA0, 0x00FF, 0x0007, 0x0016},
    {0x0100, 0x0600, 0x00DC, 0x0C50, 0x00F0, 0x0008, 0x001B},
    {0x02FD, 0x0650, 0x00DE, 0x0A50, 0x0090, 0x0008, 0x0025},
    {0x0340, 0x0800, 0x011C, 0x0420, 0x0073, 0x0009, 0x002A},
    {0x02FF, 0x0650, 0x00DE, 0x0A50, 0x0090, 0x0008, 0x0025},
    {0x0200, 0x0550, 0x00CF, 0x0B90, 0x00C4, 0x0009, 0x0020},
    {0x0320, 0x0770, 0x010D, 0x06CD, 0x0086, 0x0009, 0x0027},
    {0x0310, 0x0650, 0x00DA, 0x0B90, 0x00F0, 0x0008, 0x0020},
    {0x02F0, 0x0520, 0x00CF, 0x06A0, 0x00B8, 0x0008, 0x0020},
};

PlayerStartPlacement gPlaybackPlayerStartPlacements[44]
    __attribute__((section(".data.playback_player_start_placements"), aligned(4))) = {
    {{-0x4C973C0C, -0x34B949DC, -0x15F75C52, 0}, 134, 0x219},
    {{-0x4C233D40, -0x34893439, -0x153B862E, 0}, 133, 0x219},
    {{-0x4D1F5EE8, -0x34E406CC, -0x168907F2, 0}, 135, 0x219},
    {{-0x4BA775CC, -0x34844240, -0x156F80BA, 0}, 133, 0x219},

    {{-0x2F9C0015, -0x1C325481, -0x9EDD4B9, 0}, 71, 0x39D},
    {{-0x2DEC593B, -0x1BB801E7, -0xAC9EB63, 0}, 69, 0x57C},
    {{-0x320CEF39, -0x1CE69457, -0xA540867, 0}, 74, 0xCB},
    {{-0x2D08138B, -0x1ABC260A, -0xE81A693, 0}, 66, 0x7A6},

    {{-0xFF93AD, -0x1FE7D8B, -0x92606A7, 0}, 14, 0x1BB},
    {{0x312A27, -0x19DDD18, -0x81EFF0B, 0}, 12, 0x1BB},
    {{0x9D334B, -0x1568446, -0x6B45767, 0}, 8, 0x14F},
    {{0x122D61D, -0x10F40E8, -0x4F3D899, 0}, 5, 0x7},

    {{-0xCEB926A, -0x9A02784, -0x1A6B4D59, 0}, 28, 0x112},
    {{-0xC21FEB2, -0x90B721B, -0x19078555, 0}, 27, 0x112},
    {{-0xAF3C2D8, -0x86213B9, -0x17A3F597, 0}, 27, 0x18C},
    {{-0xB02DF46, -0x840DB1F, -0x172998F9, 0}, 27, 0x18C},

    {{-0x9439096, -0x6D7D2D1, -0x153658E2, 0}, 25, 0x185},
    {{-0xAE3C86E, -0x856BECC, -0x17892B48, 0}, 27, 0x185},
    {{-0xAE40770, -0x84AED3E, -0x1761443E, 0}, 27, 0x185},
    {{-0xB1072EE, -0x8571E92, -0x176AD568, 0}, 27, 0x185},

    {{0x3658EEB, -0x24FF70B, -0xCB8263D, 0}, 14, 0x7A},
    {{0x32C7FE7, -0x22D4578, -0xC4E4AC5, 0}, 14, 0xA6},
    {{0x39BF323, -0x245FEC4, -0xC0546C9, 0}, 14, 0xA6},
    {{0x3561369, -0x22E2CE3, -0xBAA033F, 0}, 14, 0xA6},

    {{0x152F51DA, -0x24A8EB76, -0x263A2166, 0}, 61, -0x26},
    {{0x15945DC0, -0x2500F6D9, -0x26C116B0, 0}, 61, -0x26},
    {{0x1574E1FA, -0x256FB2C0, -0x27898422, 0}, 62, -0x26},
    {{0x15223A28, -0x242DFF79, -0x2564392C, 0}, 60, -0x12E},

    {{-0x22D1172B, -0x1D707228, -0x4F4169D2, 0}, 123, 0x225},
    {{-0x24D5F811, -0x1E29BE70, -0x50710F56, 0}, 125, 0x137},
    {{-0x248C797D, -0x1E14C635, -0x505C8526, 0}, 125, 0x137},
    {{-0x2420E879, -0x1DE480DE, -0x504DE972, 0}, 124, 0x1CA},

    {{-0x8DAAB59, -0x106A5C0A, -0x22507AC2, 0}, 51, 0x319},
    {{-0xBCF2339, -0x10C4CA36, -0x23D26A52, 0}, 55, 0x319},
    {{-0x7EBD42D, -0x103DD94A, -0x2224AF9C, 0}, 50, 0x39D},
    {{-0x6974D27, -0xFE59AD3, -0x22121DD4, 0}, 49, 0x39D},

    {{-0x3360774, -0x3FF2382, -0xDE980FF, 0}, 20, -0x1A9},
    {{-0x1F85334, -0x53FB8EE, -0x101E2A2D, 0}, 22, -0x1A9},
    {{-0x48138, -0x6EEF536, -0x11DA22E1, 0}, 24, -0x1A9},
    {{0x10B8D34, -0x7B17937, -0x14F3BDC3, 0}, 28, -0x265},

    {{-0x4D56854F, -0x4A8EB15C, -0x397927D5, 0}, 160, -0x490},
    {{-0x4EB2B06F, -0x49C902FA, -0x39111D15, 0}, 159, -0x380},
    {{-0x4D7494C3, -0x4A736BB8, -0x38FECA8F, 0}, 160, -0x380},
    {{-0x508EA3AF, -0x48BAC9F9, -0x373B87C9, 0}, 157, -0x16B},
};

s32 gCoursePlayerStartZ[10]
    __attribute__((section(".data.course_player_start_z"), aligned(4))) = {
    0xA000, 0x2F000, 0x14800, 0xA000, 0x14000,
    0x17800, 0x37000, 0x16000, 0xA000, 0x4A000,
};

u8 D_8012089C[28] = {
    0, 3, 1, 3, 0, 0, 0, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0
};

u8 D_801208B8[28] = {
    0, 3, 1, 3, 0, 0, 0, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
};

u8 D_801208D4[28] = {
    2, 3, 1, 3, 0, 0, 2, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0
};

u8 D_801208F0[28] = {
    3, 3, 1, 3, 0, 0, 3, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0
};

u8 *D_8012090C[10] = {
    D_8012089C, D_8012089C, D_8012089C, D_801208D4, D_801208B8, D_801208F0, D_8012089C, D_8012089C, D_801208F0, D_8012089C
};


s32 gPlayerTimedEffectPlacementOffsets[4]
    __attribute__((section(".data.player_timed_effect_placement_offsets"), aligned(4))) = {
    -0x800, 0x800, -0x1800, 0x1800,
};

u8 gPlayerState30Animation20Countdowns[12]
    __attribute__((section(".data.player_state30_animation_countdowns"), aligned(4))) = {
    20, 0, 20, 0, 50, 0, 0, 0, 0, 30, 0, 0
};

u8 gPlayerState30Animation22Countdowns[12]
    __attribute__((section(".data.player_state30_animation_countdowns"), aligned(4))) = {
    30, 30, 30, 0, 0, 0, 30, 0, 0, 30, 0, 0
};

/* Each pair contains a directional button pattern and a completed trick ID. */
u8 gPlayerTrick9Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {2, 0}, {8, 0}, {2, 0}, {8, 9},
};

u8 gPlayerTrick18Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {1, 18}, {0, 0},
};

u8 gPlayerTrick19Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 19},
};

u8 gPlayerTrick20Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {1, 0}, {2, 20}, {0, 0},
};

u8 gPlayerTrick10Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {8, 0}, {2, 10},
};

u8 gPlayerTrick11Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {8, 11}, {0, 0},
};

u8 gPlayerTrick12Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {1, 0}, {4, 12},
};

u8 gPlayerTrick21Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 21},
};

u8 gPlayerTrick22Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {9, 0}, {3, 22},
};

u8 gPlayerTrick23Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {1, 0}, {2, 0}, {4, 23},
};

u8 gPlayerTrick13Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {8, 0}, {2, 13},
};

u8 gPlayerTrick14Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {4, 0}, {1, 0}, {8, 14}, {0, 0},
};

u8 gPlayerTrick24Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 24},
};

u8 gPlayerTrick25Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {1, 0}, {4, 25},
};

u8 gPlayerTrick27Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 27},
};

u8 gPlayerTrick28Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {1, 0}, {4, 28},
};

u8 gPlayerTrick30Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {2, 0}, {8, 0}, {2, 30}, {0, 0},
};

u8 gPlayerTrick31Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {8, 0}, {2, 31},
};

u8 gPlayerTrick38Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 38},
};

u8 gPlayerTrick39Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {8, 0}, {2, 39},
};

u8 gPlayerTrick40Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {1, 0}, {4, 40},
};

u8 gPlayerTrick41Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {8, 41}, {0, 0},
};

u8 gPlayerTrick42Input[2][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 42},
};

u8 gPlayerTrick43Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {1, 0}, {2, 0}, {4, 43},
};

u8 gPlayerTrick35Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {2, 0}, {8, 0}, {2, 35},
};

u8 gPlayerTrick36Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {2, 0}, {4, 0}, {8, 36},
};

u8 gPlayerTrick37Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {2, 0}, {8, 0}, {2, 37}, {0, 0},
};

u8 gPlayerTrick17Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {8, 0}, {1, 0}, {2, 0}, {4, 17},
};

u8 gPlayerTrick32Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {2, 0}, {8, 0}, {2, 32}, {0, 0},
};

u8 gPlayerTrick46Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {2, 0}, {8, 0}, {2, 46}, {0, 0},
};

u8 gPlayerTrick48Input[4][2]
    __attribute__((section(".data.player_trick_short_inputs"), aligned(4))) = {
    {1, 0}, {8, 0}, {4, 0}, {2, 48},
};

u8 gPlayerTrick26Input[18][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {4, 0}, {12, 0}, {8, 0}, {9, 0}, {1, 0}, {3, 0}, {2, 0}, {6, 0},
    {4, 0}, {12, 0}, {8, 0}, {9, 0}, {1, 0}, {3, 0}, {2, 0}, {6, 0},
    {4, 26}, {0, 0},
};

u8 gPlayerTrick15Input[6][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {1, 0}, {4, 0}, {8, 15}, {0, 0},
};

u8 gPlayerTrick29Input[6][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {8, 0}, {2, 0}, {1, 0}, {4, 29},
};

u8 gPlayerTrick16Input[10][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {1, 0}, {3, 0}, {2, 0}, {6, 0}, {4, 0}, {12, 0}, {8, 0}, {9, 0},
    {1, 16}, {0, 0},
};

u8 gPlayerTrick44Input[6][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {1, 0}, {4, 0}, {8, 44}, {0, 0},
};

u8 gPlayerTrick45Input[6][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {1, 0}, {4, 0}, {8, 0}, {2, 0}, {1, 0}, {4, 45},
};

u8 gPlayerTrick33Input[18][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {2, 0}, {6, 0}, {4, 0}, {12, 0}, {8, 0}, {9, 0}, {1, 0}, {3, 0},
    {2, 0}, {6, 0}, {4, 0}, {12, 0}, {8, 0}, {9, 0}, {1, 0}, {3, 0},
    {2, 33}, {0, 0},
};

u8 gPlayerTrick34Input[18][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {1, 0}, {3, 0}, {2, 0}, {6, 0}, {4, 0}, {12, 0}, {8, 0}, {9, 0},
    {1, 0}, {3, 0}, {2, 0}, {6, 0}, {4, 0}, {12, 0}, {8, 0}, {9, 0},
    {1, 34}, {0, 0},
};

u8 gPlayerTrick47Input[6][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {4, 0}, {8, 0}, {2, 0}, {8, 0}, {2, 47}, {0, 0},
};

u8 gPlayerTrick49Input[8][2]
    __attribute__((section(".data.player_trick_long_inputs"), aligned(4))) = {
    {8, 0}, {1, 0}, {2, 0}, {4, 0}, {8, 0}, {1, 0}, {2, 0}, {4, 49},
};

static s16 gPlayerUnlockSceneYawOffsets[4]
    __attribute__((section(".data.player_unlock_scene_yaw_offsets"), aligned(4))) = {
    -0x1C0, -0x40, 0x140, 0, /* Trailing alignment halfword. */
};

static s32 gPlayerWallProbeOffsets[2]
    __attribute__((section(".data.player_wall_probe_offsets"), aligned(4))) = {
    0x2000, -0x2000,
};

const PlayerState2Trick gPlayerLaunchTricks[50]
    __attribute__((section(".data.player_launch_tricks"), aligned(4))) = {
    {0, 0, 0}, {0, 13, 1}, {0, 15, 2}, {0, 16, 3}, {0, 17, 4},
    {1, 18, 5}, {1, 19, 6}, {1, 20, 7}, {1, 21, 8}, {1, 31, 11},
    {1, 32, 11}, {1, 33, 12}, {1, 34, 13}, {1, 35, 10}, {1, 36, 12},
    {1, 37, 11}, {1, 38, 13}, {1, 39, 14}, {1, 41, 9}, {1, 42, 10},
    {1, 43, 12}, {1, 44, 9}, {1, 45, 10}, {1, 46, 14}, {1, 47, 9},
    {1, 48, 11}, {1, 49, 13}, {1, 50, 9}, {1, 51, 10}, {1, 52, 12},
    {1, 53, 10}, {1, 54, 12}, {1, 55, 9}, {1, 56, 10}, {1, 57, 13},
    {1, 58, 11}, {1, 59, 12}, {1, 60, 9}, {1, 61, 9}, {1, 62, 10},
    {1, 63, 11}, {1, 64, 12}, {1, 65, 9}, {1, 66, 10}, {1, 67, 11},
    {1, 68, 12}, {1, 69, 9}, {1, 70, 10}, {1, 71, 11}, {1, 72, 12},
};

u16 gPlayerTrickRewards[16]
    __attribute__((section(".data.player_trick_rewards"), aligned(4))) = {
    0, 100, 100, 120, 120, 150, 150, 150, 150, 220, 250, 300, 350, 400, 500, 0,
};

u16 gPlayerDirectionalTrickRewards[10]
    __attribute__((section(".data.player_trick_rewards"), aligned(4))) = {
    0, 20, 20, 30, 30, 20, 20, 30, 30, 0,
};

u16 gPlayerCombinedTrickBonuses[10]
    __attribute__((section(".data.player_trick_rewards"), aligned(4))) = {
    0, 10, 20, 20, 30, 10, 20, 20, 30, 0,
};

u32 gPlayerTerrainContactSavedStackPointer
    __attribute__((section(".sdata.player_terrain_contact_saved_stack"))) = 0;

u32 gPlayerGroundContactSavedStackPointer
    __attribute__((section(".sdata.player_ground_contact_saved_stack"))) = 0;

void updateCurrentPlayerState24MotionAndScale(void);
void updateCurrentPlayerState3MotionAndSteering(void);
void updateCurrentPlayerTimedEffect(void);
void updateCurrentPlayerRiding(void);
s16 tryStartPlayerSelectedRecordTransition(s16 playerIndex);
void updateCurrentPlayerAirborneDragUntilLanding(void);
s32 snapCurrentPlayerToTerrain(void);
void moveCurrentPlayerToTaskTarget(void);
void advanceCurrentPlayerTaskOnAnimationFlag(void);
void updateCurrentPlayerTaskDecelerationSteps(void);
void advanceCurrentPlayerTaskAndSetSpeed(void);
void moveCurrentPlayerForwardUntilState22(void);
void updateCurrentPlayerState30TaskUntilStopped(void);
s32 updateCurrentPlayerGroundContact(void);
void updateCurrentPlayerState12MotionAndTimer(void);
s16 resolveCurrentPlayerTerrainContact(void);
s16 markPlayerNearSelectedRecord(s16 playerIndex);
void awardCurrentPlayerTrickRewards(void);

TaskCallback gCurrentPlayerTaskPhaseCallbacks[8]
    __attribute__((section(".data.player_task_phases"))) = {
    moveCurrentPlayerToTaskTarget, advanceCurrentPlayerTaskOnAnimationFlag, updateCurrentPlayerTaskJumpAndSpawnCourseOffset, updateCurrentPlayerTaskDecelerationSteps,
    advanceCurrentPlayerTaskAndSetSpeed, advanceCurrentPlayerTaskPhaseMovement, updateCurrentPlayerTaskLapTransition, moveCurrentPlayerForwardUntilState22
};


TaskCallback gCurrentPlayerState30TaskPhaseCallbacks[3]
    __attribute__((section(".data.player_state30_task_phases"))) = {
    updateCurrentPlayerState30TaskUntilStopped, advanceCurrentPlayerState30TaskOnResult, updateCurrentPlayerState30AnimationAndHeight
};


void (*currentPlayerStateCallbacks[74])(void) = {
    resetCurrentPlayerForCallback205B8,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState2,
    initializeCurrentPlayerState3,
    initializeCurrentPlayerState4,
    initializeCurrentPlayerState5,
    initializeCurrentPlayerState6,
    initializeCurrentPlayerState7,
    resetCurrentPlayerForCallback26DAC,
    initializeCurrentPlayerState9,
    initializeCurrentPlayerState10,
    initializeCurrentPlayerState11,
    initializeCurrentPlayerState12,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState14,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState22,
    initializeCurrentPlayerState23,
    initializeCurrentPlayerState24,
    initializeCurrentPlayerState25,
    resetCurrentPlayerForCallback25508,
    resetCurrentPlayerForCallback255C4,
    NULL,
    resetCurrentPlayerForCallback240D0,
    initializeCurrentPlayerState30,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState40,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerUnlockScene,
};

void clearCurrentPlayerSequenceState(void) {
    s32 i;
    for (i = 0; i < 6; i++) {
        D_8013B0D8->unk0CA[i] = 0;
        D_8013B0D8->unk0D6[i] = 0;
    }
}



void initializeCurrentRacePlayer(void);

u16 readCurrentPlayerTrickInput(void) {
    u16 direction = 0;
    u16 sequenceResult;
    u16 buttons;

    if (D_8013B0D8->unk008 != 0) {
        direction = (u8)randomNextMain() & (s8)D_8013B0D8->unk952[1];
    } else {
        buttons = D_8013AEEC[D_8013AE88->data18];
        if (buttons & 0x1000) {
            direction = 1;
        }
        if (buttons & 0x4000) {
            direction = 2;
        }
        if (buttons & 0x2000) {
            direction = 3;
        }
        if (buttons & 0x8000) {
            direction = 4;
        }
        if ((buttons & 0x3000) == 0x3000) {
            direction = 5;
        }
        if ((buttons & 0x9000) == 0x9000) {
            direction = 6;
        }
        if ((buttons & 0x6000) == 0x6000) {
            direction = 7;
        }
        if ((buttons & 0xC000) == 0xC000) {
            direction = 8;
        }

        switch (D_8013B0D8->characterId) {
        case 0:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick9Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick18Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick19Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick20Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 1:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick21Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick10Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick11Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick12Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick22Input, 4);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick23Input, 5);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 2:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick24Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick13Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick14Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick25Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick26Input, 4);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 3:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick15Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick27Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick28Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick29Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 4:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick16Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick30Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick31Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 5:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick38Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick39Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick40Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick41Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 6:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick42Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick43Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick44Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick45Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 7:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick37Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick35Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick36Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 8:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick17Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick32Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick33Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick34Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        case 9:
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick46Input, 0);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick47Input, 1);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick48Input, 2);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            sequenceResult = advanceCurrentPlayerInputSequence(gPlayerTrick49Input, 3);
            if (sequenceResult != 0) {
                direction = sequenceResult;
            }
            break;
        }
    }

    return direction;
}

u8 readSelectedByteTable(s32 index) {
    return D_8012090C[D_8013AFCE][index];
}

void updateCurrentPlayerMotionEffects(void) {
    s32 difference;
    s16 angle;
    s16 turn;
    RacePlayer *player;

    difference = (s16)(getXZHeading(0, 0,
        D_8013B0D8->x - D_8013B0D8->unk050,
        D_8013B0D8->z - D_8013B0D8->unk058) - D_8013B0D8->unk110) & 0xFFF;
    angle = difference;
    if (difference > 0x800) {
        angle = 0x1000 - difference;
    }
    if (angle > 0x400) {
        angle = 0x800 - angle;
    }
    player = D_8013B0D8;
    player->unk972 = angle * 3 / 2;
    player->unk96C = player->unk0BC / 768;
    if (player->unk96C != 0) {
        player->unk96C += angle >> 6;
        turn = player->unk126;
        if (turn < 0) {
            turn = -turn;
        }
        player->unk96C += turn >> 4;
        if (player->unk96C > 28) {
            player->unk96C = 28;
        }
    }
    switch (readSelectedByteTable(D_8013B0D8->surfaceState.fields.unk136)) {
    case 0:
        D_8013B0D8->unk96E = 0;
        break;
    case 1:
        D_8013B0D8->unk96E = 0x801;
        D_8013B0D8->unk972 = 0;
        D_8013B0D8->unk96C = 32;
        break;
    case 2:
        D_8013B0D8->unk96E = 0x100;
        break;
    case 3:
        D_8013B0D8->unk96E = 0x200;
        break;
    case 4:
        D_8013B0D8->unk96E = 0x100;
        if (D_8013B0D8->surfaceState.fields.unk136 == 13) {
            D_8013B0D8->unk96E = 0x300;
            if (D_8013B0D8->unk96C > 10 && (gFrameCounter & 3) == 0) {
                queuePlayerControllerEffect(D_8013AE88->data18, 15);
            }
        }
        D_8013B0D8->unk972 = 0;
        break;
    }
    if (D_8013B0D8->unk128 & 0x2000000) {
        D_8013B0D8->unk96E = 0x100;
    }
}


void updateCurrentPlayerPartTransforms(void) {
    u16 i;
    u8 parent;
    for (i = 0; i < 12; i++) {
        parent = D_8013B0D8->unk13E[i];
        RotMatrix((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i].coord);
        if (i == 0) {
            D_8013AF90->vx = 0;
            D_8013AF90->vy = 0;
            D_8013AF90->vz = 0;
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[0].coord, D_8013AF90, D_8013AF88);
            D_8013B0D8->model.modelCoordinates[0].coord.t[0] = D_8013AF88->vx + D_8013B0D8->unk62C[0].x;
            D_8013B0D8->model.modelCoordinates[0].coord.t[1] = D_8013AF88->vy + D_8013B0D8->unk62C[0].y;
            D_8013B0D8->model.modelCoordinates[0].coord.t[2] = D_8013AF88->vz + D_8013B0D8->unk62C[0].z;
        } else {
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[parent].coord, (SVECTOR *)&D_8013B0D8->unk62C[i], D_8013AF88);
            if (D_8013B0D8->unk128 & 0x200000) {
                D_8013AF88->vy = D_8013AF88->vy * D_8013B0D8->unk0F8 / 8;
            }
            D_8013B0D8->model.modelCoordinates[i].coord.t[0] = D_8013AF88->vx + D_8013B0D8->model.modelCoordinates[parent].coord.t[0];
            D_8013B0D8->model.modelCoordinates[i].coord.t[1] = D_8013AF88->vy + D_8013B0D8->model.modelCoordinates[parent].coord.t[1];
            D_8013B0D8->model.modelCoordinates[i].coord.t[2] = D_8013AF88->vz + D_8013B0D8->model.modelCoordinates[parent].coord.t[2];
        }
        setCoordinateRotation((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i]);
    }
}

void resetCurrentPlayerForCallback205B8(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk954 = 0;
    player->unk126 = 0;
    player->unk128 &= ~0x200;
    D_8013B0D8->unk096 = 0;
    if (D_8013B0D8->unk965 != 0) {
        D_8013B0D8->unk965 = 0;
        playCurrentPlayerSoundEffect(7);
        startCurrentPlayerAnimation(0x12, 2);
    }
    setCurrentTaskCallback(updateCurrentPlayerRiding);
}

void initializeCurrentPlayerDefaultState(void) {
    if (D_8013B0D8->animationIndex != D_800EB09C[D_8013B0D8->characterId][4]) {
        startCurrentPlayerAnimation(4, 2);
    }
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk08C = 0;
    D_8013B0D8->unk128 |= 0x200;
    clearCurrentPlayerTaskState();
    setCurrentTaskCallback(updateCurrentPlayerAirborneTricks);
}

void initializeCurrentPlayerState22(void) {
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk128 |= 0x200;
    clearCurrentPlayerTaskState();
    D_8013B0D8->unk0C8 = 0;
    if (D_8013B0D8->animationIndex == D_800EB09C[D_8013B0D8->characterId][6] ||
        D_8013B0D8->animationIndex == D_800EB09C[D_8013B0D8->characterId][5]) {
        startCurrentPlayerAnimation(1, 5);
    } else {
        D_8013B0D8->animationFlags |= 4;
    }
    setCurrentTaskCallback(updateCurrentPlayerAirborneTricks);
}

void initializeCurrentPlayerState2(void) {
    D_8013B0D8->unk08C = 0xB84;
    D_8013B0D8->unk128 |= 0x10;
    if (D_8013AE88->data1A) {
        startCurrentPlayerAnimation(3, 6);
    } else {
        startCurrentPlayerAnimation(3, 2);
    }
    clearCurrentPlayerSequenceState();
    D_8013B0D8->unk0C8 = 0;
    D_8013B0D8->unk13C = 0;
    setCurrentTaskCallback(updateCurrentPlayerChargeAndLaunch);
}

void initializeCurrentPlayerState3(void) {
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x2000;
    if (((D_8013B0D8->unk102 - (s16)D_8013B0D8->unk110) & 0xFFF) > 0x800) {
        startCurrentPlayerAnimation(0x18, 2);
    } else {
        startCurrentPlayerAnimation(0x17, 2);
    }

    D_8013B0D8->unk060 = D_8013B0D8->unk104.half[0];
    D_8013B0D8->unk064 = D_8013B0D8->unk108.half[0];
    D_8013B0D8->unk050 = D_8013B0D8->x - D_8013B0D8->unk060;
    D_8013B0D8->unk058 = D_8013B0D8->z - D_8013B0D8->unk064;
    queuePlayerControllerEffect(D_8013AE88->data18, 8);
    setCurrentTaskCallback(updateCurrentPlayerState3MotionAndSteering);
}

void initializeCurrentPlayerState4(void) {
    startCurrentPlayerAnimation(12, 2);
    D_8013B0D8->unk0B8 = 0;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x12204;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk060 = D_8013B0D8->unk104.half[0];
    D_8013B0D8->unk062 = -0x600;
    D_8013B0D8->unk064 = D_8013B0D8->unk108.half[0];
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk050 = D_8013B0D8->x - D_8013B0D8->unk060;
    D_8013B0D8->unk058 = D_8013B0D8->z - D_8013B0D8->unk064;
    queuePlayerControllerEffect(D_8013AE88->data18, 4);
    setCurrentTaskCallback(updateCurrentPlayerState4MotionAndLanding);
}

void initializeCurrentPlayerState5(void) {
    D_8013B0D8->unk08C = 30;
    if (D_8013B0D8->unk128 & 0x800000) {
        D_8013B0D8->unk08C = 90;
    }
    startCurrentPlayerAnimation(13, 2);
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk128 &= 0xFF0C1FFB;
    D_8013B0D8->unk128 &= ~0x200;
    D_8013B0D8->unk128 |= 0x12000;
    D_8013B0D8->unk08C -= D_8013B0D8->unk08C * D_8013B0D8->unk935 / 8;
    playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB00, 0x20);
    queuePlayerControllerEffect(D_8013AE88->data18, 9);
    setCurrentTaskCallback(updateCurrentPlayerState5MotionAndTimer);
}

void initializeCurrentPlayerTimedEffectState(s32 value) {
    RacePlayer *player = D_8013B0D8;
    player->unk08C = 60;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x42000;
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = value;
    player->unk078 = 0;
    player->unk10C = player->unk92E;
    switch (player->surfaceState.fields.unk136) {
    case 6:
        createPlayerRingEffectTask(D_8013AE88->data18);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xF00, 0x30);
        break;
    case 7:
    case 12:
        startPositionEffectTask((const PositionEffectOrigin *)gRacePlayers[D_8013AE88->data18].worldCoordinate.coord.t);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 1, 0x30);
        break;
    case 14:
        createPlayerRingEffectTask(D_8013AE88->data18);
        break;
    }
    D_8013B0D8->unk08C -= D_8013B0D8->unk08C * D_8013B0D8->unk935 / 8;
    startCurrentPlayerAnimation(16, 2);
    queuePlayerControllerEffect(D_8013AE88->data18, 1);
    D_8013B0D8->unk064 = 0;
    D_8013B0D8->unk062 = 0;
    D_8013B0D8->unk060 = 0;
    D_8013B0D8->unk0BC = 0;
    D_8013B0D8->unk094 = 0;
    setCurrentTaskCallback(updateCurrentPlayerTimedEffect);
}

void initializeCurrentPlayerState6(void) {
    initializeCurrentPlayerTimedEffectState(0);
}

void initializeCurrentPlayerState9(void) {
    initializeCurrentPlayerTimedEffectState(1);
}

void resetCurrentPlayerForCallback26DAC(void) {
    RacePlayer *player;
    if (D_8013B0D8->unk128 & 0x10000) {
        startCurrentPlayerAnimation(8, 2);
    } else {
        startCurrentPlayerAnimation(7, 2);
    }
    player = D_8013B0D8;
    player->unk078 = 0x400;
    player->unk0BC = 0;
    player->unk094 = 0;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 &= ~0x200;
    player->unk128 |= 0xA000;
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(updateCurrentPlayerState8Steering);
}

void initializeCurrentPlayerState10(void) {
    RacePlayer *player;
    D_801241E8[D_8013AE88->data18].unk16 = 4;
    player = D_8013B0D8;
    player->unk128 &= ~0x200;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x6000;
    if (player->animationIndex != D_800EB09C[player->characterId][15]) {
        startCurrentPlayerAnimation(15, 1);
    }
    D_8013B0D8->unk08C = 30;
    D_8013B0D8->unk078 = 0;
    queuePlayerControllerEffect(D_8013AE88->data18, 2);
    setCurrentTaskCallback(updateCurrentPlayerState10Movement);
}

void initializeCurrentPlayerState14(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 15;
    player->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    queuePlayerControllerEffect(D_8013AE88->data18, 5);
    setCurrentTaskCallback(updateCurrentPlayerTimedMotion);
}

void initializeCurrentPlayerState11(void) {
    RacePlayer *player;
    startCurrentPlayerAnimation(11, 2);
    player = D_8013B0D8;
    player->unk0B8 = 0;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x22201;
    player->unk062 -= 0x800;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk078 = 0;
    queuePlayerControllerEffect(D_8013AE88->data18, 9);
    setCurrentTaskCallback(updateCurrentPlayerState11Motion);
}

void initializeCurrentPlayerState12(void) {
    startCurrentPlayerAnimation(14, 2);
    D_8013B0D8->unk08C = 30;
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk090 = 0;
    D_8013B0D8->unk128 &= 0xFF0C1FFB;
    D_8013B0D8->unk128 &= ~0x200;
    D_8013B0D8->unk128 |= 0x22000;
    D_8013B0D8->unk08C -= D_8013B0D8->unk08C * D_8013B0D8->unk935 / 8;
    D_8013AE88->data1A = 0;
    playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB00, 0x20);
    queuePlayerControllerEffect(D_8013AE88->data18, 4);
    setCurrentTaskCallback(updateCurrentPlayerState12MotionAndTimer);
}

void initializeCurrentPlayerState40(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 30;
    D_8013B0D8->unk08C += ((u32)randomNextMain() & 0xFF) >> 4;
    D_8013B0D8->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    queuePlayerControllerEffect(D_8013AE88->data18, 5);
    setCurrentTaskCallback(updateCurrentPlayerTimedMotion);
}

void initializeCurrentPlayerState24(void) {
    RacePlayer *player;
    u16 i;
    D_8013B0D8->unk0F8 = 1;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x202000;
    startCurrentPlayerAnimation(1, 1);
    for (i = 0; i < 12; i++) {
        D_8013B0D8->unk5BC[i].z = 0;
        D_8013B0D8->unk5BC[i].x = 0;
    }
    updateCurrentPlayerPartTransforms();
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 0;
    player = D_8013B0D8;
    player->unk08C = 90;
    player->unk078 = 0;
    player->unk064 = 0;
    player->unk062 = 0;
    player->unk060 = 0;
    player->unk0BC = 0;
    player->unk094 = 0;
    player->unk08C = 90 - (player->unk08C * player->unk935) / 8;
    queuePlayerControllerEffect(D_8013AE88->data18, 7);
    D_8013AF88->vx = D_8013B0D8->worldCoordinate.coord.t[0];
    D_8013AF88->vy = D_8013B0D8->worldCoordinate.coord.t[1] - 32;
    D_8013AF88->vz = D_8013B0D8->worldCoordinate.coord.t[2];
    createQuadAnimationTask(D_8013AE88->data18, D_8013AF88);
    setCurrentTaskCallback(updateCurrentPlayerState24MotionAndScale);
}

void initializeCurrentPlayerState25(void) {
    RacePlayer *player = D_8013B0D8;
    s32 reduction;
    player->unk08C = 240;
    reduction = player->unk935 * player->unk08C;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x400000;
    player->unk08C = 240 - reduction / 8;
    startCurrentPlayerAnimation(1, 1);
    createChildTaskForParentWithReserve(&gGameTasks[4], initializeScaledTaskCoordinateResource);
    D_8013AE8C->data18 = D_8013AE88->data18;
    queuePlayerControllerEffect(D_8013AE88->data18, 6);
    setCurrentTaskCallback(updateCurrentPlayerState25MotionAndTimer);
}

void resetCurrentPlayerForCallback25508(void) {
    startCurrentPlayerAnimation(15, 1);
    D_8013B0D8->unk0B8 = 0;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x6204;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk062 = -0x2000;
    D_8013B0D8->unk064 = 0;
    D_8013B0D8->unk060 = 0;
    D_8013B0D8->unk0BC = 0;
    D_8013B0D8->unk094 = 0;
    queuePlayerControllerEffect(D_8013AE88->data18, 3);
    createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerParticleHistoryTask);
    D_8013AE8C->data18 = D_8013AE88->data18;
    setCurrentTaskCallback(updateCurrentPlayerVerticalMotionUntilLanding);
}

void resetCurrentPlayerForCallback255C4(void) {
    RacePlayer *player;
    startCurrentPlayerAnimation(0xB, 2);
    player = D_8013B0D8;
    player->unk0B8 = 0;
    player->unk078 = 0;
    player->unk062 = -0x1800;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x822204;
    D_8013AE88->data1C = 0;
    queuePlayerControllerEffect(D_8013AE88->data18, 6);
    setCurrentTaskCallback(updateCurrentPlayerAirborneDragUntilLanding);
}

void initializeCurrentPlayerState23(void) {
    s32 reduction;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x102000;
    startCurrentPlayerAnimation(12, 2);
    D_8013B0D8->unk08C = 90;
    reduction = D_8013B0D8->unk935 * D_8013B0D8->unk08C;
    D_8013B0D8->unk08C = 90 - reduction / 8;
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk064 = 0;
    D_8013B0D8->unk062 = 0;
    D_8013B0D8->unk060 = 0;
    D_8013B0D8->unk0BC = 0;
    D_8013B0D8->unk094 = 0;
    createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerState23CoordinateEffectTask);
    D_8013AE8C->data18 = D_8013AE88->data18;
    queuePlayerControllerEffect(D_8013AE88->data18, 6);
    playPlayerSoundEffect(D_8013AE88->data18, 1, 0xD00, 0x20);
    setCurrentTaskCallback(updateCurrentPlayerState23MotionAndTimer);
}

void initializeCurrentPlayerState7(void) {
    RacePlayer *player = D_8013B0D8;
    D_8013AE88->data1A = 0;
    player->unk08C = 20;
    player->unk090 = 10;
    player->unk126 = D_80123D80[D_8013AFCE].unk16;
    if (player->unk128 & 0x400) {
        player->unk126 += 0x800;
    }
    D_8013B0D8->surfaceState.fields.unk134 = 0;
    D_8013AFE4 |= 4;
    startCurrentPlayerAnimation(1, 1);
    playPlayerSoundEffect(D_8013AE88->data18, 1, 0x401, 0x20);
    setCurrentTaskCallback(updateCurrentPlayerFromTaskPhase);
}

void resetCurrentPlayerForCallback240D0(void) {
    RacePlayer *player = D_8013B0D8;
    GameTask *task = D_8013AE88;

    player->unk08C = 0;
    player->unk090 = 0;
    player->unk128 &= ~0x201;
    task->data1A = 0;
    task->data1C = 0;
    player->unk954 = 0;
    player->unk126 = 0;
    D_8013B0D8->unk0BE = D_8013B0D8->unk138;
    startCurrentPlayerAnimation(0x22, 2);
    setCurrentTaskCallback(updateCurrentPlayerState29MotionAndAnimation);
}

void initializeCurrentPlayerState30(void) {
    u8 padding[0x10];
    GameTask *task = D_8013AE88;
    RacePlayer *player = D_8013B0D8;

    task->data1A = 0;
    if (0x800 < player->unk0BC) {
        task->data1E = 1;
        if (player->unk128 & 0x400) {
            player->unk0BE += 0x800;
        }
        if (D_8013B0D8->unk128 & 0x400) {
            D_8013AE88->data1C = 0;
        } else {
            D_8013AE88->data1C = 1;
        }
        {
            RacePlayer *volatile *currentPlayer = &D_8013B0D8;

            D_8013B0D8->unk0BE += 0x400;
            /* The single-iteration loop preserves the register allocation for the heading update. */
            do {
                (*currentPlayer)->unk08C = 0;
            } while (0);
        }
    } else {
        task->data1E = 0;
        startCurrentPlayerAnimation(1, 1);
    }
    setCurrentTaskCallback(updateCurrentPlayerAndRequestState30);
}

void initializeCurrentPlayerUnlockScene(void) {
    u16 angle;
    D_8013B0D8 = &gRacePlayers[D_8013AE88->data18];
    startCurrentPlayerAnimation(D_8013B0D8->unk008 == 0 ? 9 : 10, 1);
    if (D_8013AE88->data18 == 0) {
        D_8013B0D8->x = D_80129C60[26].coord.t[0] << 8;
        D_8013B0D8->y = (D_80129C60[26].coord.t[1] - 74) << 8;
        D_8013B0D8->z = D_80129C60[26].coord.t[2] << 8;
        D_8013B0D8->unk0B8 = 0;
        D_8013B0D8->unk0B4 = 0;
        D_8013B0D8->unk0B6 = D_8012A5C0[26].vy;
        D_8013AE88->data1A = 30;
    } else {
        D_8013B0D8->x = (D_80129C60[26].coord.t[0] -
            rsin(D_8012A5C0[26].vy + gPlayerUnlockSceneYawOffsets[D_8013AE88->data18 - 1]) / 32) << 8;
        D_8013B0D8->y = D_80129C60[26].coord.t[1] << 8;
        D_8013B0D8->z = (D_80129C60[26].coord.t[2] -
            rcos(D_8012A5C0[26].vy + gPlayerUnlockSceneYawOffsets[D_8013AE88->data18 - 1]) / 32) << 8;
        D_8013B0D8->unk0B8 = 0;
        D_8013B0D8->unk0B4 = 0;
        angle = gPlayerUnlockSceneYawOffsets[D_8013AE88->data18 - 1] + 0x800;
        D_8013B0D8->unk0B6 = D_8012A5C0[26].vy + angle;
    }
    D_8013B0D8->unk92E = D_80123D80[D_8013AFCE].unk00;
    D_8013B0D8->surfaceState.fields.unk136 = 0;
    D_8013B0D8->unk128 = 0;
    D_8013B0D8->unk12C = 73;
    setCurrentTaskCallback(updateCurrentPlayerTaskCountdown);
    updateCurrentPlayerTaskCountdown();
}

s32 callCurrentPlayerStateCallbackIfDifferent(u16 state) {
    s32 result;
    if (D_8013B0D8->unk12C == state) {
        result = 0;
    } else {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        result = 1;
    }
    return result;
}

void initializeCurrentPlayerModelAndHeight(void) {
    RacePlayer *player;
    if (D_8013AFE2 == 0) {
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeTaskCoordinateResource);
        D_8013AE8C->data18 = D_8013AE88->data18;
    }
    player = &gRacePlayers[D_8013AE88->data18];
    player->unk0B6 = -0x400;
    player->unk0B4 = 0;
    player->unk0B8 = 0;
    player->unk962 = -1;
    D_8013B0D8 = player;
    initializeCurrentPlayerModelObjects((DAT_PLAYER_MODEL_BUFFERS)[D_8013AE88->data18]);
    if (!D_8013AFD8 && !D_8013AFD9) {
        D_8013B0D8->unk92E = 0;
    } else {
        D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    }
    D_8013B0D8->y = D_8013B0D8->unk054 = getCourseHeightAtPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    updateCurrentPlayerGroundContact();
    startCurrentPlayerAnimation(1, 1);
    setCurrentTaskCallback(updateCurrentPlayerAndResetState);
}

void updateCurrentPlayerAndResetState(void) {
    prepareCurrentPlayerFrame();
    updateCurrentPlayerAnimation();
    if (D_8013AFE4 == 0) {
        if (D_8013B0D8->unk008 == 0) {
            if (D_8013AFD7 || D_8013AFD9) {
                initializeControllerInputPlayback(D_8013AE88->data18);
            } else if (D_8013AFE2 == 2 && D_8013AFD6 == 0) {
                initializeControllerInputRecording(0);
                resetSelectedRecordStates();
            }
        }
        D_8013B0D8->unk12C = 0;
        resetCurrentPlayerForCallback205B8();
    }
}

void updateCurrentPlayerRiding(void) {
    RacePlayer *framePlayer;
    RacePlayer *autoPlayer;
    RacePlayer *headingPlayer;
    RacePlayer *filteredPlayer;
    RacePlayer *manualPlayer;
    RacePlayer *stancePlayer;
    RacePlayer *speedPlayer;
    RacePlayer *steeringPlayer;
    RacePlayer *centeringPlayer;
    RacePlayer *turnPlayer;
    RacePlayer *boostPlayer;
    RacePlayer *movingPlayer;
    RacePlayer *itemPlayer;
    RacePlayer *autoItemPlayer;
    RacePlayer *jumpPlayer;
    RacePlayer *lowSpeedPlayer;
    RacePlayer *manualItemPlayer;
    RacePlayer *landingPlayer;
    const SelectedCourseRecord *course;
    s16 steer;
    s16 angle;
    s16 headingStep;
    s32 speed;
    s32 magnitude;
    s32 difference;
    s32 boost;
    u8 character;
    u8 padding[0x10];
    s32 targetX, targetZ;

    prepareCurrentPlayerFrame();
    framePlayer = D_8013B0D8;
    framePlayer->unk09E = framePlayer->unk09A;
    speed = framePlayer->unk094;
    if (speed >= 0) {
        if (speed < 0x40) {
            speed = 0x40;
        }
    } else {
        if (speed > -0x40) {
            speed = -0x40;
        }
    }

    if (D_8013B0D8->unk008 ||
        (D_8013B0D8->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 &&
         (s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18))) {
        autoPlayer = D_8013B0D8;
        getCourseSteeringTarget(autoPlayer->unk92E, autoPlayer->x >> 8, autoPlayer->z >> 8,
                      &targetX, &targetZ, (s8)autoPlayer->unk025[3], D_8013AE88->data18);
        angle = getXZHeading(D_8013B0D8->x, D_8013B0D8->z, targetX << 8, targetZ << 8);
        headingPlayer = D_8013B0D8;
        if (headingPlayer->unk128 & 0x400) {
            angle = (angle + 0x800) & 0xFFF;
        }
        steer = angle - headingPlayer->unk110;
        difference = steer & 0xFFF;
        steer = difference;
        if (difference > 0x800) {
            steer = difference | 0xF000;
        }
        if (steer > 0x100) {
            steer = 0x100;
        }
        if (steer < -0x100) {
            steer = -0x100;
        }
        headingStep = steer >> 2;
        steer = (steer + 1) >> 1;
        if (steer > 31) {
            steer = 31;
        }
        if (steer < -31) {
            steer = -31;
        }
        {
            u8 previous = headingPlayer->unk954;
            steer -= (s8)previous;
            difference = steer & 0xFFF;
            steer = difference;
            if (difference > 0x800) {
                steer = difference | 0xF000;
            }
            if (steer > 2) {
                steer = 2;
            }
            if (steer < -2) {
                steer = -2;
            }
            headingPlayer->unk954 = previous + steer;
        }
        filteredPlayer = D_8013B0D8;
        steer = -(s8)filteredPlayer->unk954;
        if (steer == 1) {
            steer = 0;
        }
        if (steer == -1) {
            steer = 0;
        }
        filteredPlayer->unk124 = 63;
        if (!filteredPlayer->unk008) {
            drawAutoPilotLabelTask();
        }
    } else {
        s16 throttle;
        steer = -D_8013AEF8[D_8013AE88->data18];
        throttle = D_8013AEFC[D_8013AE88->data18];
        if (throttle == 31) {
            steer *= 3;
            if (steer > 31) {
                steer = 31;
            } else if (steer < -31) {
                steer = -31;
            }
        }
        throttle *= 3;
        if (throttle > 31) {
            throttle = 31;
        }
        manualPlayer = D_8013B0D8;
        manualPlayer->unk124 = throttle + 32;
        if (manualPlayer->unk124 < 31) {
            manualPlayer->unk124 = 31;
        }
    }

    stancePlayer = D_8013B0D8;
    magnitude = speed;
    if (stancePlayer->unk094 < 0) {
        magnitude = -speed;
        steer = -steer;
    }
    if (stancePlayer->animationIndex != D_800EB09C[stancePlayer->characterId][2]) {
        const SelectedCourseRecord *records = D_80123D80;
        s16 recordIndex = D_8013AFCE;
        course = &records[recordIndex];
        if (stancePlayer->unk92E == (s16)course->unk00 && !(stancePlayer->unk128 & 0x40)) {
            s32 offset;
            angle = getXZHeading(stancePlayer->x, stancePlayer->z, course->unk40[0], course->unk40[1]);
            offset = (s16)(angle - D_8013B0D8->unk110 + 0x400) & 0xFFF;
            if (offset < 0x800) {
                if (D_8013B0D8->unk128 & 0x400) {
                    goto reloadStance;
                }
                goto applySteering;
            }
            if (D_8013B0D8->unk128 & 0x400) {
                goto applySteering;
            }
            goto reloadStance;
        }
        speedPlayer = D_8013B0D8;
        if (speedPlayer->unk094 > 0x80 && (speedPlayer->unk128 & 0x400)) {
            goto reverseStance;
        }
        if (speedPlayer->unk094 >= -0x80 || (speedPlayer->unk128 & 0x400)) {
            goto applySteering;
        }
reloadStance:
        speedPlayer = D_8013B0D8;
reverseStance:
        speedPlayer->unk128 ^= 0x400;
        startCurrentPlayerAnimation(2, 2);
    }

applySteering:
    angle = smoothAndScaleCurrentPlayerInput(magnitude, steer);
    steeringPlayer = D_8013B0D8;
    if (steeringPlayer->animationIndex != D_800EB09C[steeringPlayer->characterId][2] &&
        steeringPlayer->animationIndex != D_800EB09C[steeringPlayer->characterId][0]) {
        angle -= steeringPlayer->unk126;
        if (angle > 0x30) {
            angle = 0x30;
        }
        if (angle < -0x30) {
            angle = -0x30;
        }
        steeringPlayer->unk126 += angle;
        if ((s16)steeringPlayer->unk126 == 0) {
            if (steeringPlayer->animationIndex != D_800EB09C[steeringPlayer->characterId][0x12] &&
                steeringPlayer->animationIndex != D_800EB09C[steeringPlayer->characterId][1]) {
                startCurrentPlayerAnimation(1, 1);
            }
        } else {
            if ((s16)steeringPlayer->unk126 > 0) {
                blendCurrentPlayerSteeringPose(6, (s16)steeringPlayer->unk126, 0x118);
            }
            if ((s16)D_8013B0D8->unk126 < 0) {
                blendCurrentPlayerSteeringPose(5, -(s16)D_8013B0D8->unk126, 0x118);
            }
        }
    } else {
        centeringPlayer = D_8013B0D8;
        angle = -centeringPlayer->unk126;
        if (angle > 0x30) {
            angle = 0x30;
        }
        if (angle < -0x30) {
            angle = -0x30;
        }
        centeringPlayer->unk126 += angle;
    }
    if (D_8013B0D8->animationFlags & 0x10) {
        startCurrentPlayerAnimation(1, 1);
    }

    if (D_8013B0D8->unk008 ||
        (D_8013B0D8->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 &&
         (s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18))) {
        /* The same autopilot condition above supplies this correction. */
        D_8013B0D8->unk0B6 += headingStep;
    } else {
        s16 span;
        s16 baseScale;
        s32 turnScale;
        s32 radius;
        s32 turnRate;
        turnPlayer = D_8013B0D8;
        turnRate = (-(steer << 4) * (s16)turnPlayer->unk0A0) / 256;
        span = turnPlayer->unk0A4;
        baseScale = turnPlayer->unk0A2;
        turnScale = ((span - baseScale) * (63 - turnPlayer->unk124)) / 32 + baseScale;
        angle = 0;
        if (turnRate != 0) {
            radius = (f32)turnScale * (f32)magnitude * (f32)magnitude / (f32)turnRate;
            if (radius != 0) {
                if (radius > 0) {
                    radius += 0xC00;
                }
                if (radius < 0) {
                    radius -= 0xC00;
                }
                if (radius > 0x100000) {
                    radius = 0x100000;
                }
                if (radius < -0x100000) {
                    radius = -0x100000;
                }
                angle = getXZHeading(radius, 0, 0, speed);
                if (radius > 0) {
                    angle += 0x400;
                }
                if (radius < 0) {
                    angle += 0xC00;
                }
            }
            angle &= 0xFFF;
        }
        if (angle > 0x80 && angle <= 0x800) {
            angle = 0x80;
        }
        if (angle > 0x800 && angle < 0xF80) {
            angle = -0x80;
        }
        D_8013B0D8->unk0B6 = (D_8013B0D8->unk0B6 + angle) & 0xFFF;
    }

    boostPlayer = D_8013B0D8;
    boost = 0;
    if (boostPlayer->unk0FC != 0) {
        boost = 0x280;
        if (boostPlayer->unk0FF) {
            boost = -0x280;
        }
    }
    if (D_8013AFD9 && D_8013B0D8->unk945 == 8) {
        boost = 0x1800;
    }
    updateCurrentPlayerSlopeVelocity(0, boost, (s16)D_8013B0D8->unk0A6,
                                    (s16)D_8013B0D8->unk0A8, (s16)D_8013B0D8->unk0AA);
    movingPlayer = D_8013B0D8;
    movingPlayer->x += movingPlayer->unk060;
    movingPlayer->y += movingPlayer->unk062;
    movingPlayer->z += movingPlayer->unk064;
    updateCurrentPlayerMotionEffects();

    itemPlayer = D_8013B0D8;
    if (itemPlayer->animationIndex != D_800EB09C[itemPlayer->characterId][0]) {
        if (itemPlayer->unk008 ||
            (itemPlayer->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 &&
             (s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18))) {
            autoItemPlayer = D_8013B0D8;
            if (autoItemPlayer->unk950) {
                autoItemPlayer->unk951 = 1;
                jumpPlayer = D_8013B0D8;
                character = jumpPlayer->characterId;
                jumpPlayer->unk12C = 2;
                if (jumpPlayer->animationIndex == D_800EB09C[character][6] ||
                    jumpPlayer->animationIndex == D_800EB09C[character][5]) {
                    D_8013AE88->data1A = 1;
                } else {
                    D_8013AE88->data1A = 0;
                }
            } else if (autoItemPlayer->unk0BC < 0xC00) {
                autoItemPlayer->unk951 = 0;
                lowSpeedPlayer = D_8013B0D8;
                character = lowSpeedPlayer->characterId;
                lowSpeedPlayer->unk12C = 2;
                if (lowSpeedPlayer->animationIndex == D_800EB09C[character][6] ||
                    lowSpeedPlayer->animationIndex == D_800EB09C[character][5]) {
                    D_8013AE88->data1A = 1;
                } else {
                    D_8013AE88->data1A = 0;
                }
            }
        } else {
            GameTask *task = D_8013AE88;
            if (D_8013AEEC[task->data18] & 0x40) {
                manualItemPlayer = D_8013B0D8;
                manualItemPlayer->unk12C = 2;
                if (manualItemPlayer->animationIndex == D_800EB09C[manualItemPlayer->characterId][6] ||
                    manualItemPlayer->animationIndex == D_800EB09C[manualItemPlayer->characterId][5]) {
                    task->data1A = 1;
                } else {
                    task->data1A = 0;
                }
            }
        }
        updateCurrentPlayerProjectileUse(D_8013AE88->data18);
        updateCurrentPlayerItemUse(D_8013AE88->data18);
    }

    if (D_8013B0D8->unk943) {
        playCurrentPlayerSoundEffect(5);
    }
    steer = updateCurrentPlayerGroundContact();
    landingPlayer = D_8013B0D8;
    if (landingPlayer->animationIndex != D_800EB09C[landingPlayer->characterId][6] &&
        landingPlayer->animationIndex != D_800EB09C[landingPlayer->characterId][5]) {
        updateCurrentPlayerAnimation();
    }
    if ((u16)callCurrentPlayerStateCallbackIfDifferent(0) == 0) {
        if (steer != 0 && D_8013B0D8->unk12C == 0) {
            D_8013B0D8->unk12C = 0x16;
        } else {
            if (D_8013B0D8->unk128 & 0x40) {
                D_8013B0D8->unk12C = 0x1E;
            }
            if (tryStartPlayerSelectedRecordTransition(D_8013AE88->data18)) {
                D_8013B0D8->unk12C = 7;
            }
        }
        callCurrentPlayerStateCallbackIfDifferent(0);
    }
}

void updateCurrentPlayerChargeAndLaunch(void) {
    RacePlayer *player;
    RacePlayer *chargePlayer;
    RacePlayer *positionPlayer;
    RacePlayer *velocityPlayer;
    RacePlayer *trickPlayer;
    RacePlayer *launchPlayer;
    RacePlayer *motionPlayer;
    RacePlayer *inputPlayer;
    s32 boost;
    s32 push;
    u16 trick;
    s16 contact;
    const PlayerState2Trick *reward;
    const PlayerState2Trick *rewardTable;

    prepareCurrentPlayerFrame();
    push = 0;
    player = D_8013B0D8;
    if (!player->unk008 && player->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 &&
        (s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18)) {
        drawAutoPilotLabelTask();
    }
    player = D_8013B0D8;
    if (player->animationIndex != D_800EB09C[player->characterId][2]) {
        u32 reversed;
        RacePlayer *counterPlayer;
        if (player->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 && !(player->unk128 & 0x40)) {
            s16 heading = getXZHeading(player->x, player->z, D_80123D80[D_8013AFCE].unk40[0],
                                       D_80123D80[D_8013AFCE].unk40[1]);
            s16 delta;
            counterPlayer = D_8013B0D8;
            delta = heading - counterPlayer->unk110 + 0x400;
            if ((delta & 0xFFF) < 0x800) {
                if (!(counterPlayer->unk128 & 0x400)) {
                    goto advanceCharge;
                }
                goto turn;
            }
            reversed = counterPlayer->unk128 & 0x400;
            goto checkReversed;
        }
        if (D_8013B0D8->unk094 > 0x80 && (D_8013B0D8->unk128 & 0x400)) {
            goto turn;
        }
        if (D_8013B0D8->unk094 < -0x80) {
            reversed = D_8013B0D8->unk128 & 0x400;
checkReversed:
            if (!reversed) {
turn:
                D_8013B0D8->unk128 ^= 0x400;
                startCurrentPlayerAnimation(2, 2);
            }
        }
        counterPlayer = D_8013B0D8;
advanceCharge:
        if (counterPlayer->unk13C < 90) {
            counterPlayer->unk13C++;
        }
    }
    if (D_8013B0D8->animationIndex != D_800EB09C[D_8013B0D8->characterId][3] &&
        (D_8013B0D8->animationFlags & 0x10)) {
        startCurrentPlayerAnimation(3, 2);
    }
    updateCurrentPlayerProjectileUse(D_8013AE88->data18);
    if (D_8013B0D8->unk13C < 6) {
        if (!D_8013B0D8->unk008 &&
            (D_8013B0D8->unk92E != (s16)D_80123D80[D_8013AFCE].unk00 ||
             !(s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18))) {
            if (!(D_8013AEEC[D_8013AE88->data18] & 0x40)) {
                D_8013B0D8->unk128 &= ~0x10;
            }
        } else if (!D_8013B0D8->unk951) {
            D_8013B0D8->unk128 &= ~0x10;
        }
    } else if (D_8013B0D8->unk128 & 0x10) {
        if (!D_8013B0D8->unk008 &&
            (D_8013B0D8->unk92E != (s16)D_80123D80[D_8013AFCE].unk00 ||
             !(s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18))) {
            if (!(D_8013AEEC[D_8013AE88->data18] & 0x40)) {
                D_8013B0D8->unk128 &= ~0x10;
            }
        } else {
            chargePlayer = D_8013B0D8;
            chargePlayer->unk128 &= ~0x10;
            if (chargePlayer->unk951) {
                chargePlayer->unk08C = 0xD00;
            }
        }
        if ((D_8013B0D8->unk128 & 0x10) && D_8013B0D8->unk08C < 0xD00) {
            D_8013B0D8->unk08C += 0x4C;
        }
    }
    smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk094, 0);
    inputPlayer = D_8013B0D8;
    inputPlayer->unk09E = inputPlayer->unk09A;
    boost = 0;
    if (inputPlayer->unk0FC) {
        boost = inputPlayer->unk0FF ? -0x280 : 0x280;
    }
    motionPlayer = D_8013B0D8;
    if (motionPlayer->unk13C < 6) {
        updateCurrentPlayerSlopeVelocity(0, boost, (s16)motionPlayer->unk0A6 * 2, (s16)motionPlayer->unk0A8, (s16)motionPlayer->unk0AA);
        readCurrentPlayerTrickInput();
    } else {
        if (!(motionPlayer->unk128 & 0x10)) {
            if (motionPlayer->unk128 & 0x400) {
                push = -0xF00;
                if (motionPlayer->unk945) {
                    boost = -0x4000;
                }
                if (motionPlayer->unk13C >= 60) {
                    boost -= 0x1400;
                    motionPlayer->unk12E += 0x1400;
                    motionPlayer->unk130 += 0x1400;
                    playCurrentPlayerSoundEffect(0);
                }
                if (D_8013AFCE == 7 && D_8013B0D8->unk92E == 2) {
                    push = -0x1100;
                }
            } else {
                push = 0xF00;
                if (motionPlayer->unk945) {
                    boost = 0x1000;
                }
                if (motionPlayer->unk13C >= 60) {
                    boost += 0x1400;
                    motionPlayer->unk12E += 0x1400;
                    motionPlayer->unk130 += 0x1400;
                    playCurrentPlayerSoundEffect(0);
                }
                if (D_8013AFCE == 7 && D_8013B0D8->unk92E == 2) {
                    push = 0x1100;
                }
            }
        }
        updateCurrentPlayerSlopeVelocity(push, boost, (s16)D_8013B0D8->unk0A6 * 2,
                                         (s16)D_8013B0D8->unk0A8, (s16)D_8013B0D8->unk0AA);
        trick = readCurrentPlayerTrickInput();
        launchPlayer = D_8013B0D8;
        if (!(launchPlayer->unk128 & 0x10)) {
            if (launchPlayer->unk08C < 0xD00) {
                trick = 0;
            }
            if (trick >= 9) {
                launchPlayer->unk08C = 0xE00;
            }
            D_8013B0D8->unk0C4 = 0;
            if (trick) {
                playPlayerSoundEffect(D_8013AE88->data18, 1, 0x501, 0x30);
                rewardTable = gPlayerLaunchTricks;
                reward = &rewardTable[trick];
                playCurrentPlayerSoundEffect(reward->sound);
                trickPlayer = D_8013B0D8;
                trickPlayer->unk12C = reward->state;
                trickPlayer->unk0C8 = reward->trickIndex;
                trickPlayer->unk128 |= 0x800;
                if (!trickPlayer->unk008) {
                    if (trickPlayer->unk0C8 < 9) {
                        startIndexedRenderTask(D_8013AE88->data18);
                    } else {
                        startIndexedRenderTaskPair(D_8013AE88->data18);
                    }
                }
            } else {
                D_8013B0D8->unk12C = 1;
                D_8013B0D8->unk0C8 = 0;
            }
            velocityPlayer = D_8013B0D8;
            velocityPlayer->unk062 -= velocityPlayer->unk08C;
            if (D_8013AFCE == 7 && velocityPlayer->unk92E == 2) {
                velocityPlayer->unk062 -= 0x400;
            }
            D_8013B0D8->y -= 0x1800;
            D_8013B0D8->unk128 |= 0x200;
            startCurrentPlayerAnimation(4, 2);
        }
    }
    positionPlayer = D_8013B0D8;
    positionPlayer->x += positionPlayer->unk060;
    positionPlayer->y += positionPlayer->unk062;
    positionPlayer->z += positionPlayer->unk064;
    updateCurrentPlayerMotionEffects();
    if (D_8013B0D8->unk943) {
        playCurrentPlayerSoundEffect(5);
    }
    contact = updateCurrentPlayerGroundContact();
    updateCurrentPlayerAnimation();
    if (!(u16)callCurrentPlayerStateCallbackIfDifferent(2)) {
        if ((u16)contact && D_8013B0D8->unk12C == 2) {
            D_8013B0D8->unk12C = 22;
        } else {
            if (D_8013B0D8->unk128 & 0x40) {
                D_8013B0D8->unk12C = 30;
            }
            if (tryStartPlayerSelectedRecordTransition((s16)D_8013AE88->data18)) {
                D_8013B0D8->unk12C = 7;
            }
        }
        callCurrentPlayerStateCallbackIfDifferent(2);
    }
}

void updateCurrentPlayerAirborneTricks(void) {
    RacePlayer *rotationPlayer;
    RacePlayer *rollPlayer;
    s32 rotation;
    RacePlayer *motionPlayer;
    RacePlayer *pitchPlayer;
    struct {
        s32 x;
        s32 z;
    } steeringTarget;
    RacePlayer *steeringPlayer;
    s16 steeringRegion;
    s32 *targetZ;
    s16 heading;
    s32 temp_a0;
    s16 temp_a0_5;
    s16 terrainContact;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 landingSpeed;
    s16 temp_v1_9;
    RacePlayer *spinPlayer;
    s32 temp_a0_3;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 temp_s0_4;
    s32 temp_v0_33;
    s32 temp_v0_34;
    s32 landingSurface;
    s32 var_v0_10;
    s32 var_v0_11;
    RacePlayer *phasePlayer;
    GameTask *phaseTask;
    s32 nextPhase;
    RacePlayer *dispatchPlayer;
    u32 landingState;
    s32 var_v0_13;
    s32 var_v0_19;
    s32 var_v0_20;
    s32 var_v0_21;
    s32 var_v0_24;
    s32 var_v0_27;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_9;
    u16 temp_v0_11;
    u16 temp_v0_12;
    u16 temp_v0_14;
    u16 temp_v0_15;
    u16 temp_v0_16;
    u16 temp_v0_18;
    u16 temp_v0_19;
    u16 temp_v0_20;
    u16 temp_v0_21;
    u16 temp_v0_22;
    u16 temp_v0_23;
    u16 temp_v0_24;
    u16 temp_v0_25;
    u16 temp_v0_26;
    u16 temp_v0_27;
    u16 temp_v0_28;
    u16 temp_v0_29;
    u16 temp_v0_30;
    u16 temp_v0_31;
    u16 temp_v0_32;
    u16 temp_v0_36;
    s16 temp_v0_37;
    u16 trickState;
    u16 temp_v0_4;
    u16 temp_v0_5;
    u16 temp_v0_7;
    u16 temp_v0_8;
    u16 temp_v0_9;
    u16 temp_v1;
    u16 temp_v1_2;
    u16 temp_v1_3;
    u16 temp_v1_4;
    u16 temp_v1_5;
    u16 temp_v1_6;
    u16 temp_v1_7;
    u16 temp_v1_8;
    u32 temp_a1;
    u32 temp_v1_11;
    TaskCallback nextStateCallback;

    prepareCurrentPlayerFrame();
    if (D_8013B0D8->unk008 != 0) {
        goto block_4;
    }
    if (D_8013B0D8->unk92E != (s16) D_80123D80[D_8013AFCE].unk00) {
        goto block_4;
    }
    if ((s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18) == 0) {
        goto block_4;
    }
    drawAutoPilotLabelTask();
block_4:
    if (D_8013AE88->data1A != 0) {
        goto block_20;
    }
    if (D_8013B0D8->unk0C8 != 0) {
        goto block_20;
    }
    updateCurrentPlayerProjectileUse(D_8013AE88->data18);
    if (D_8013B0D8->unk008 != 0) {
        goto block_13;
    }
    if (D_8013B0D8->unk92E != (s16) D_80123D80[D_8013AFCE].unk00) {
        goto block_9;
    }
    if ((s16)getModeFilteredAutoPilotFlag(D_8013AE88->data18) != 0) {
        goto block_13;
    }
block_9:
    if (!(D_8013AEEC[D_8013AE88->data18] & 0x8000)) {
        goto block_11;
    }
    D_8013B0D8->unk0B6 = (u16) D_8013B0D8->unk0B6 - 0x18;
block_11:
    if (!(D_8013AEEC[D_8013AE88->data18] & 0x2000)) {
        goto block_20;
    }
    D_8013B0D8->unk0B6 = (u16) D_8013B0D8->unk0B6 + 0x18;
    goto block_20;
block_13:
    steeringPlayer = D_8013B0D8;
    targetZ = &steeringTarget.z;
    steeringRegion = steeringPlayer->unk92E;
    getCourseSteeringTarget(steeringRegion, steeringPlayer->x >> 8, steeringPlayer->z >> 8, &steeringTarget.x, targetZ, (s8)steeringPlayer->unk025[3], (u16) (s32) D_8013AE88->data18);
    heading = getXZHeading(D_8013B0D8->x, D_8013B0D8->z, steeringTarget.x << 8, steeringTarget.z << 8);
    heading -= D_8013B0D8->unk110;
    temp_a0 = heading & 0xFFF;
    heading = temp_a0;
    if (temp_a0 < 0x801) {
        goto block_15;
    }
    heading = temp_a0 | 0xF000;
block_15:
    if (heading < 0x21) {
        goto block_17;
    }
    heading = 0x20;
block_17:
    if (heading >= -0x20) {
        goto block_19;
    }
    heading = -0x20;
block_19:
    D_8013B0D8->unk0B6 = (u16) D_8013B0D8->unk0B6 + heading;
block_20:
    if (D_8013B0D8->unk0C8 >= 9) {
        goto block_22;
    }
    dispatchCurrentPlayerDirectionalAnimation();
block_22:
    smoothAndScaleCurrentPlayerInput((s32) D_8013B0D8->unk094, 0);
    pitchPlayer = D_8013B0D8;
    temp_v0 = pitchPlayer->unk0B8 / 24;
    if (temp_v0 > 0) {
        if (temp_v0 < 4) {
            temp_v0 = 4;
        }
        pitchPlayer->unk0B8 -= temp_v0;
        if (pitchPlayer->unk0B8 < 0) {
            pitchPlayer->unk0B8 = 0;
        }
    } else {
        if (temp_v0 > -4) {
            temp_v0 = -4;
        }
        pitchPlayer->unk0B8 -= temp_v0;
        if (pitchPlayer->unk0B8 > 0) {
            pitchPlayer->unk0B8 = 0;
        }
    }
    temp_v0_2 = (u16) D_8013B0D8->unk062 + D_8013B0D8->unk09C;
    D_8013B0D8->unk062 = temp_v0_2;
    if (temp_v0_2 < 0x7801) {
        goto block_33;
    }
    D_8013B0D8->unk062 = 0x7800;
block_33:
    D_800EB22C.vx = (s32) D_8013B0D8->unk060;
    D_800EB22C.vz = (s32) D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    motionPlayer = D_8013B0D8;
    motionPlayer->unk060 = D_800EB22C.vx;
    motionPlayer->unk064 = D_800EB22C.vz;
    motionPlayer->x += motionPlayer->unk060;
    motionPlayer->y += motionPlayer->unk062;
    motionPlayer->z += motionPlayer->unk064;
    if (motionPlayer->unk943 == 0) {
        goto block_35;
    }
    playCurrentPlayerSoundEffect(5);
block_35:
    trickState = D_8013B0D8->unk12C;
    switch (trickState) {
    case 0xD:
        rollPlayer = D_8013B0D8;
        if (rollPlayer->unk128 & 0x400) {
            goto block_41;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C);
        goto block_42;
    case 0xF:
        rollPlayer = D_8013B0D8;
        if (!(rollPlayer->unk128 & 0x400)) {
            goto block_41;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C);
        goto block_42;
block_41:
        D_8013B0D8->modelRotation.vz = -rsin(rollPlayer->unk08C);
block_42:
        D_8013B0D8->unk08C += 0x20;
        goto block_317;
    case 0x10:
        rotation = rsin(D_8013B0D8->unk08C);
        rotationPlayer = D_8013B0D8;
        rotationPlayer->modelRotation.vy = rotation;
        rotationPlayer->unk08C += 30;
        break;
    case 0x11:
        rotation = -rsin(D_8013B0D8->unk08C);
        rotationPlayer = D_8013B0D8;
        rotationPlayer->modelRotation.vy = rotation;
        rotationPlayer->unk08C += 30;
        break;
    case 0x12:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_48;
        }
        var_v0_4 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_4;
        goto block_49;
block_48:
        var_v0_4 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_4;
block_49:
        rotation = rsin(D_8013B0D8->unk08C);
        rotationPlayer = D_8013B0D8;
        rotationPlayer->modelRotation.vy = rotation;
        rotationPlayer->unk08C += 26;
        break;
    case 0x13:
        rollPlayer = D_8013B0D8;
        if (rollPlayer->unk128 & 0x400) {
            goto block_58;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C);
        goto block_59;
    case 0x14:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_54;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C);
        goto block_55;
block_54:
        D_8013B0D8->modelRotation.vz = -rsin(D_8013B0D8->unk08C);
block_55:
        rotation = rsin(D_8013B0D8->unk08C);
        rotationPlayer = D_8013B0D8;
        rotationPlayer->modelRotation.vy = rotation;
        rotationPlayer->unk08C += 26;
        break;
    case 0x15:
        rollPlayer = D_8013B0D8;
        if (!(rollPlayer->unk128 & 0x400)) {
            goto block_58;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C);
        goto block_59;
block_58:
        D_8013B0D8->modelRotation.vz = -rsin(rollPlayer->unk08C);
block_59:
        rotation = -rsin(D_8013B0D8->unk08C);
        rotationPlayer = D_8013B0D8;
        rotationPlayer->modelRotation.vy = rotation;
        rotationPlayer->unk08C += 26;
        break;
    case 0x1F:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_4 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_4;
        if ((temp_v0_4 & 0xFFFF) != 1) {
            goto block_63;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_63:
        if (D_8013AE88->data1C != 8) {
            goto block_65;
        }
        startCurrentPlayerAnimation(0x1BU, 2U);
block_65:
        if (D_8013AE88->data1C != 0xF) {
            goto block_67;
        }
        startCurrentPlayerAnimation(0x20U, 2U);
block_67:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x21, 2U);
        goto block_317;
    case 0x29:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_71;
        }
        var_v0_5 = -rsin(D_8013B0D8->unk08C);
        goto block_72;
block_71:
        var_v0_5 = rsin(D_8013B0D8->unk08C);
block_72:
        D_8013B0D8->modelRotation.vz = var_v0_5 * 2;
        goto block_286;
    case 0x2A:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_75;
        }
        var_v0_6 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vx = var_v0_6 * 2;
        goto block_76;
block_75:
        var_v0_6 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vx = var_v0_6 * 2;
block_76:
        goto block_286;
    case 0x2B:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 4;
        D_8013B0D8->unk08C += 0x14;
        if (D_8013AE88->data1C < 8) {
            D_8013B0D8->modelRotation.vz = D_8013AE88->data1C * 128;
        } else if (D_8013AE88->data1C >= 0x23) {
            D_8013B0D8->modelRotation.vz = (0x32 - D_8013AE88->data1C) * 64;
        } else {
            D_8013B0D8->modelRotation.vz = 0x380;
        }
        if (D_8013B0D8->unk128 & 0x400) {
            D_8013B0D8->modelRotation.vz = -D_8013B0D8->modelRotation.vz;
        }
        temp_v1 = D_8013AE88->data1C;
        if (temp_v1 >= 0x32U) {
            goto block_317;
        }
        D_8013AE88->data1C = temp_v1 + 1;
        goto block_317;
    case 0x20:
        D_8013B0D8->unk08C += 0x13;
        temp_v0_5 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_5;
        if ((temp_v0_5 & 0xFFFF) != 1) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x24, 2U);
        goto block_317;
    case 0x21:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_90;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C) * 3;
        goto block_91;
block_90:
        D_8013B0D8->modelRotation.vz = -rsin(D_8013B0D8->unk08C) * 3;
block_91:
        D_8013B0D8->modelRotation.vy = -(s16) D_8013AE88->data1C;
        temp_v1_2 = D_8013AE88->data1C;
        if (temp_v1_2 == 0x1000) {
            goto block_93;
        }
        D_8013AE88->data1C = temp_v1_2 + 0x80;
block_93:
        D_8013B0D8->unk08C += 0x11;
        goto block_317;
    case 0x22:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_96;
        }
        var_v0_9 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_9;
        goto block_97;
block_96:
        var_v0_9 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_9;
block_97:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 4;
        D_8013B0D8->unk08C += 0x15;
        goto block_317;
    case 0x2C:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_7 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_7;
        if ((temp_v0_7 & 0xFFFF) != 1) {
            goto block_100;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_100:
        if (D_8013AE88->data1C == 8) {
            startCurrentPlayerAnimation(0x1D, 2);
        }
        goto block_305;
    case 0x2D:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_104;
        }
        var_v0_10 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = var_v0_10 * 2;
        goto block_105;
block_104:
        var_v0_10 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = var_v0_10 * 2;
block_105:
        if ((u16) D_8013AE88->data1C >= 0x19U) {
            goto block_107;
        }
        var_v0_11 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vy = var_v0_11 * 2;
        goto block_108;
block_107:
        var_v0_11 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vy = var_v0_11 * 2;
block_108:
        phasePlayer = D_8013B0D8;
        nextPhase = phasePlayer->unk08C + 0x14;
        phaseTask = D_8013AE88;
        goto block_316;
    case 0x2E:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 5;
        D_8013B0D8->unk08C += 0x14;
        temp_v0_8 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_8;
        if ((temp_v0_8 & 0xFFFF) != 1) {
            goto block_111;
        }
        startCurrentPlayerAnimation(0x20U, 2U);
block_111:
        if (D_8013AE88->data1C != 0x28) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x21, 2U);
        goto block_317;
    case 0x23:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_115;
        }
        var_v0_13 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vx = var_v0_13 * 2;
        goto block_116;
block_115:
        var_v0_13 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vx = var_v0_13 * 2;
block_116:
        D_8013B0D8->unk08C += 0x16;
        temp_v0_9 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_9;
        if ((temp_v0_9 & 0xFFFF) != 1) {
            goto block_118;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_118:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x24:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_122;
        }
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 3;
        goto block_123;
block_122:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 3;
block_123:
        D_8013B0D8->unk08C += 0x15;
        temp_v0_11 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_11;
        if ((temp_v0_11 & 0xFFFF) != 1) {
            goto block_125;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_125:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x2F:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_12 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_12;
        if ((temp_v0_12 & 0xFFFF) != 1) {
            goto block_129;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_129:
        if (D_8013AE88->data1C != 8) {
            goto block_131;
        }
        startCurrentPlayerAnimation(0x1DU, 2U);
block_131:
        if (D_8013AE88->data1C != 0xF) {
            goto block_133;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_133:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x30:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_137;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C) * 3;
        goto block_138;
block_137:
        D_8013B0D8->modelRotation.vz = -rsin(D_8013B0D8->unk08C) * 3;
block_138:
        D_8013B0D8->unk08C += 0x16;
        temp_v1_3 = D_8013AE88->data1C;
        if (temp_v1_3 >= 0x2DU) {
            goto block_140;
        }
        D_8013AE88->data1C = temp_v1_3 + 1;
block_140:
        D_8013B0D8->modelRotation.vy = (D_8013AE88->data1C << 0xC) / 45;
        if (D_8013AE88->data1C != 1) {
            goto block_142;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_142:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x31:
        if (D_8013B0D8->unk128 & 0x400) {
            D_8013B0D8->modelRotation.vz = -rsin(D_8013B0D8->unk08C) * 3;
        } else {
            D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C) * 3;
        }
        D_8013B0D8->unk08C += 0x12;
        break;
    case 0x25:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_150;
        }
        D_8013B0D8->modelRotation.vx = rsin(D_8013B0D8->unk08C) * 2;
        goto block_151;
block_150:
        D_8013B0D8->modelRotation.vx = -rsin(D_8013B0D8->unk08C) * 2;
block_151:
        D_8013B0D8->unk08C += 0x16;
        temp_v0_14 = D_8013AE88->data1C;
        D_8013AE88->data1C = temp_v0_14 + 1;
        if ((u32) (temp_v0_14 - 8) >= 0xBU) {
            goto block_154;
        }
        temp_v1_4 = D_8013AE88->data1E;
        if (temp_v1_4 == 0xFC00) {
            goto block_154;
        }
        D_8013AE88->data1E = temp_v1_4 - 0x80;
block_154:
        if ((u16) D_8013AE88->data1C < 0x1FU) {
            goto block_157;
        }
        temp_v0_15 = D_8013AE88->data1E;
        if (temp_v0_15 == 0) {
            goto block_157;
        }
        D_8013AE88->data1E = temp_v0_15 + 0x80;
block_157:
        D_8013B0D8->modelRotation.vy = -(s16) D_8013AE88->data1E;
        if (D_8013AE88->data1C != 1) {
            goto block_159;
        }
        startCurrentPlayerAnimation(0x24U, 2U);
block_159:
        if (D_8013AE88->data1C != 0xC) {
            goto block_161;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_161:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x32:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_16 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_16;
        if ((temp_v0_16 & 0xFFFF) != 1) {
            goto block_165;
        }
        startCurrentPlayerAnimation(0x1EU, 2U);
block_165:
        if (D_8013AE88->data1C != 8) {
            goto block_167;
        }
        startCurrentPlayerAnimation(0x1FU, 2U);
block_167:
        if (D_8013AE88->data1C != 0xF) {
            goto block_169;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_169:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1D, 2U);
        goto block_317;
    case 0x33:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_173;
        }
        D_8013B0D8->modelRotation.vx = rsin(D_8013B0D8->unk08C) * 3;
        goto block_174;
block_173:
        D_8013B0D8->modelRotation.vx = -rsin(D_8013B0D8->unk08C) * 3;
block_174:
        D_8013B0D8->unk08C += 0x14;
        temp_v0_18 = D_8013AE88->data1C;
        D_8013AE88->data1C = temp_v0_18 + 1;
        if ((u32) (temp_v0_18 - 8) >= 0xBU) {
            goto block_177;
        }
        temp_v1_5 = D_8013AE88->data1E;
        if (temp_v1_5 == 0x400) {
            goto block_177;
        }
        D_8013AE88->data1E = temp_v1_5 + 0x80;
block_177:
        if ((u16) D_8013AE88->data1C < 0x1FU) {
            goto block_180;
        }
        temp_v0_19 = D_8013AE88->data1E;
        if (temp_v0_19 == 0) {
            goto block_180;
        }
        D_8013AE88->data1E = temp_v0_19 - 0x80;
block_180:
        D_8013B0D8->modelRotation.vy = -(s16) D_8013AE88->data1E;
        if (D_8013AE88->data1C != 1) {
            goto block_182;
        }
        startCurrentPlayerAnimation(0x24U, 2U);
block_182:
        if (D_8013AE88->data1C != 0xC) {
            goto block_184;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_184:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x34:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 3;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_20 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_20;
        if ((temp_v0_20 & 0xFFFF) != 1) {
            goto block_188;
        }
        startCurrentPlayerAnimation(0x20U, 2U);
block_188:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x21, 2U);
        goto block_317;
    case 0x26:
        D_8013B0D8->unk08C += 0x16;
        temp_v0_21 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_21;
        if ((temp_v0_21 & 0xFFFF) != 1) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x25, 2U);
        goto block_317;
    case 0x35:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_22 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_22;
        if ((temp_v0_22 & 0xFFFF) != 1) {
            goto block_194;
        }
        startCurrentPlayerAnimation(0x1EU, 2U);
block_194:
        if (D_8013AE88->data1C != 8) {
            goto block_196;
        }
        startCurrentPlayerAnimation(0x1FU, 2U);
block_196:
        if (D_8013AE88->data1C != 0xF) {
            goto block_198;
        }
        startCurrentPlayerAnimation(0x20U, 2U);
block_198:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x21, 2U);
        goto block_317;
    case 0x36:
        D_8013B0D8->unk08C += 0x14;
        temp_v0_23 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_23;
        if ((temp_v0_23 & 0xFFFF) != 1) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x24, 2U);
        goto block_317;
    case 0x3D:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_24 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_24;
        if ((temp_v0_24 & 0xFFFF) != 1) {
            goto block_204;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_204:
        if (D_8013AE88->data1C != 8) {
            goto block_206;
        }
        startCurrentPlayerAnimation(0x1BU, 2U);
block_206:
        if (D_8013AE88->data1C != 0xF) {
            goto block_208;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_208:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1D, 2U);
        goto block_317;
    case 0x3E:
        temp_a0_3 = D_8013B0D8->unk08C;
        if (temp_a0_3 >= 0x200) {
            goto block_212;
        }
        var_v0_19 = -rsin(temp_a0_3);
        D_8013B0D8->modelRotation.vy = var_v0_19 * 2;
        goto block_213;
block_212:
        var_v0_19 = rsin(temp_a0_3);
        D_8013B0D8->modelRotation.vy = var_v0_19 * 2;
block_213:
        D_8013B0D8->unk08C += 0x14;
        temp_v0_25 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_25;
        if ((temp_v0_25 & 0xFFFF) != 1) {
            goto block_215;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_215:
        if (D_8013AE88->data1C != 8) {
            goto block_217;
        }
        startCurrentPlayerAnimation(0x1DU, 2U);
block_217:
        if (D_8013AE88->data1C != 0xF) {
            goto block_219;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_219:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x3F:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_223;
        }
        var_v0_20 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_20;
        goto block_224;
block_223:
        var_v0_20 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_20;
block_224:
        D_8013B0D8->unk08C += 0x12;
        goto block_317;
    case 0x40:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_228;
        }
        var_v0_21 = -rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_21;
        goto block_229;
block_228:
        var_v0_21 = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_21;
block_229:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 5;
        D_8013B0D8->unk08C += 0x11;
        goto block_317;
    case 0x41:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_26 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_26;
        if ((temp_v0_26 & 0xFFFF) != 1) {
            goto block_232;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_232:
        if (D_8013AE88->data1C != 8) {
            goto block_234;
        }
        startCurrentPlayerAnimation(0x1DU, 2U);
block_234:
        if (D_8013AE88->data1C != 0xF) {
            goto block_236;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_236:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1B, 2U);
        goto block_317;
    case 0x42:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C);
        D_8013B0D8->unk08C += 0x16;
        temp_v0_27 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_27;
        if ((temp_v0_27 & 0xFFFF) != 1) {
            goto block_240;
        }
        startCurrentPlayerAnimation(0x24U, 2U);
block_240:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x25, 2U);
        goto block_317;
    case 0x43:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 5;
        D_8013B0D8->unk08C += 0x14;
        temp_v0_28 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_28;
        if ((temp_v0_28 & 0xFFFF) != 1) {
            goto block_244;
        }
        startCurrentPlayerAnimation(0x1EU, 2U);
block_244:
        if (D_8013AE88->data1C == 0x23) {
            startCurrentPlayerAnimation(0x1F, 2);
        }
        goto block_317;
    case 0x44:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_247;
        }
        D_8013B0D8->modelRotation.vx = rsin(D_8013B0D8->unk08C) * 4;
        goto block_248;
block_247:
        D_8013B0D8->modelRotation.vx = -rsin(D_8013B0D8->unk08C) * 4;
block_248:
        D_8013B0D8->unk08C += 0x12;
        temp_v0_29 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_29;
        if ((temp_v0_29 & 0xFFFF) != 1) {
            goto block_250;
        }
        startCurrentPlayerAnimation(0x26U, 2U);
block_250:
        if (D_8013AE88->data1C != 0x2D) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x27, 2U);
        goto block_317;
    case 0x3A:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x14;
        temp_v0_30 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_30;
        if ((temp_v0_30 & 0xFFFF) != 1) {
            goto block_254;
        }
        startCurrentPlayerAnimation(0x24U, 2U);
block_254:
        if (D_8013AE88->data1C != 0x28) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x25, 2U);
        goto block_317;
    case 0x3B:
        D_8013B0D8->unk08C += 0x12;
        temp_v0_31 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_31;
        if ((temp_v0_31 & 0xFFFF) != 1) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x26, 2U);
        goto block_317;
    case 0x3C:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        temp_v0_32 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_32;
        if ((temp_v0_32 & 0xFFFF) != 1) {
            goto block_260;
        }
        startCurrentPlayerAnimation(0x1AU, 2U);
block_260:
        if (D_8013AE88->data1C != 8) {
            goto block_262;
        }
        startCurrentPlayerAnimation(0x1BU, 2U);
block_262:
        if (D_8013AE88->data1C != 0xF) {
            goto block_264;
        }
        startCurrentPlayerAnimation(0x1CU, 2U);
block_264:
        if (D_8013AE88->data1C != 0x1E) {
            goto block_317;
        }
        startCurrentPlayerAnimation(0x1D, 2U);
        goto block_317;
    case 0x27:
        temp_v1_6 = D_8013AE88->data1C;
        if (temp_v1_6 == 0) {
            goto block_269;
        }
        if (temp_v1_6 == 1) {
            goto block_274;
        }
        goto block_317;
block_269:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_271;
        }
        var_v0_24 = -rsin(D_8013B0D8->unk08C * 2);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_24;
        goto block_272;
block_271:
        var_v0_24 = rsin(D_8013B0D8->unk08C * 2);
        D_8013B0D8->modelRotation.vz = (s16) var_v0_24;
block_272:
        temp_v0_33 = D_8013B0D8->unk08C + 0x19;
        D_8013B0D8->unk08C = temp_v0_33;
        if (temp_v0_33 < 0x201) {
            goto block_317;
        }
        D_8013B0D8->unk08C = 0x200;
        D_8013AE88->data1E = 0;
        D_8013AE88->data1C += 1;
        goto block_317;
block_274:
        D_8013B0D8->modelRotation.vy = -rsin((D_8013B0D8->unk08C - 0x200) * 2) * 2;
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_276;
        }
        D_8013B0D8->modelRotation.vz = (s16) D_8013AE88->data1E;
        goto block_277;
block_276:
        D_8013B0D8->modelRotation.vz = -(s16) D_8013AE88->data1E;
block_277:
        temp_v0_34 = D_8013B0D8->unk08C + 0x10;
        D_8013B0D8->unk08C = temp_v0_34;
        if (temp_v0_34 < 0x401) {
            goto block_279;
        }
        D_8013B0D8->unk08C = 0x400;
block_279:
        temp_v1_7 = D_8013AE88->data1E;
        if (temp_v1_7 == 0x1000) {
            goto block_317;
        }
        D_8013AE88->data1E = temp_v1_7 + 0x80;
        goto block_317;
    case 0x38:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 5;
        D_8013B0D8->unk08C += 0x14;
        goto block_317;
    case 0x39:
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_284;
        }
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C) * 5;
        goto block_285;
block_284:
        D_8013B0D8->modelRotation.vz = -rsin(D_8013B0D8->unk08C) * 5;
block_285:
block_286:
        D_8013B0D8->unk08C += 0x14;
        goto block_317;
    case 0x37:
    case 0x45:
        D_8013B0D8->modelRotation.vy = -rsin(D_8013B0D8->unk08C) * 2;
        D_8013B0D8->unk08C += 0x16;
        goto block_317;
    case 0x46:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 2;
        if (D_8013AE88->data1C < 8) {
            D_8013B0D8->modelRotation.vz = D_8013AE88->data1C * 256;
        } else if (D_8013AE88->data1C >= 0x22) {
            D_8013B0D8->modelRotation.vz = D_8013AE88->data1C * 128 - 0x880;
        } else {
            D_8013B0D8->modelRotation.vz = 0x800;
        }
        D_8013B0D8->unk08C += 0x14;
        temp_v1_8 = D_8013AE88->data1C;
        if (temp_v1_8 >= 0x32U) {
            goto block_317;
        }
        D_8013AE88->data1C = temp_v1_8 + 1;
        goto block_317;
    case 0x47:
        var_v0_27 = rsin(D_8013B0D8->unk08C) * 4;
        spinPlayer = D_8013B0D8;
        spinPlayer->modelRotation.vy = var_v0_27;
        if ((u16) D_8013AE88->data1C >= 0x14U) {
            goto block_298;
        }
        if (spinPlayer->unk128 & 0x400) {
            goto block_300;
        }
        D_8013B0D8->modelRotation.vz = rsin(spinPlayer->unk08C) * 2;
        goto block_301;
block_298:
        if (!(spinPlayer->unk128 & 0x400)) {
            goto block_300;
        }
        D_8013B0D8->modelRotation.vz = rsin(spinPlayer->unk08C) * 2;
        goto block_301;
block_300:
        D_8013B0D8->modelRotation.vz = -rsin(spinPlayer->unk08C) * 2;
block_301:
        D_8013B0D8->unk08C += 0x14;
        temp_v0_36 = D_8013AE88->data1C + 1;
        D_8013AE88->data1C = temp_v0_36;
        if ((temp_v0_36 & 0xFFFF) != 1) {
            goto block_303;
        }
        startCurrentPlayerAnimation(0x20U, 2U);
block_303:
        if (D_8013AE88->data1C == 8) {
            startCurrentPlayerAnimation(0x21, 2);
        }
block_305:
        if (D_8013AE88->data1C != 0xF) {
            goto block_307;
        }
        startCurrentPlayerAnimation(0x1EU, 2U);
block_307:
        if (D_8013AE88->data1C == 0x1E) {
            startCurrentPlayerAnimation(0x1F, 2);
        }
        goto block_317;
    case 0x48:
        D_8013B0D8->modelRotation.vy = rsin(D_8013B0D8->unk08C) * 6;
        if (!(D_8013B0D8->unk128 & 0x400)) {
            goto block_312;
        }
        D_8013B0D8->modelRotation.vz = -rsin(D_8013B0D8->unk08C) * 6;
        goto block_313;
block_312:
        D_8013B0D8->modelRotation.vz = rsin(D_8013B0D8->unk08C) * 6;
block_313:
        if (!((u16) D_8013B0D8->modelRotation.vz & 0x1000)) {
            goto block_315;
        }
        D_8013B0D8->modelRotation.vy = -(s16) (u16) D_8013B0D8->modelRotation.vy;
block_315:
        phasePlayer = D_8013B0D8;
        nextPhase = phasePlayer->unk08C + 0x11;
        phaseTask = D_8013AE88;
block_316:
        phasePlayer->unk08C = nextPhase;
        phaseTask->data1C += 1;
    }
block_317:
    temp_v0_37 = D_8013B0D8->modelRotation.vz;
    if (temp_v0_37 == 0) {
        goto block_321;
    }
    temp_v1_9 = rsin(temp_v0_37 / 2) / 48;
    D_8013B0D8->unk086 = temp_v1_9;
    if (temp_v1_9 < 0) {
        D_8013B0D8->unk086 = -temp_v1_9;
    }
    temp_s0 = rsin((s32) D_8013B0D8->modelRotation.vz);
    D_8013B0D8->unk088 = (temp_s0 * rcos((s32) (s16) D_8013B0D8->unk110)) / 393216;
    temp_s0_2 = rsin((s32) D_8013B0D8->modelRotation.vz);
    D_8013B0D8->unk084 = (temp_s0_2 * rsin((s32) (s16) D_8013B0D8->unk110)) / 393216;
block_321:
    temp_a0_5 = D_8013B0D8->modelRotation.vx;
    if (temp_a0_5 == 0) {
        goto block_323;
    }
    D_8013B0D8->unk086 = (0x1000 - rcos((s32) temp_a0_5)) / 96;
    temp_s0_3 = rsin((s32) D_8013B0D8->modelRotation.vx);
    D_8013B0D8->unk084 = (temp_s0_3 * rcos((s32) (s16) D_8013B0D8->unk110)) / 393216;
    temp_s0_4 = rsin((s32) D_8013B0D8->modelRotation.vx);
    D_8013B0D8->unk088 = (-temp_s0_4 * rsin((s32) (s16) D_8013B0D8->unk110)) / 393216;
block_323:
    if (D_8013B0D8->unk0C8 == 0) {
        goto block_333;
    }
    if (D_8013B0D8->unk08C < 0x401) {
        goto block_326;
    }
    D_8013B0D8->unk08C = 0x400;
block_326:
    temp_a1 = D_8013B0D8->unk128;
    D_8013B0D8->unk128 = temp_a1 | 2;
    if (D_8013B0D8->unk08C >= 0x3D0) {
        goto block_333;
    }
    D_8013B0D8->unk128 = temp_a1 | 0x802;
    if (D_8013B0D8->unk0C8 >= 9) {
        goto block_330;
    }
    if ((u16)gFrameCounter & 1) {
        createPlayerPairedQuadEffectTask(D_8013AE88->data18, 0);
    }
    goto block_333;
block_330:
    if ((u16)gFrameCounter & 1) {
        createPlayerPairedQuadEffectTask(D_8013AE88->data18, 1);
    }
block_333:
    markPlayerNearSelectedRecord((s16) D_8013AE88->data18);
    terrainContact = resolveCurrentPlayerTerrainContact();
    updateCurrentPlayerAnimation();
    if (D_8013B0E4 < 2) {
        goto block_335;
    }
    D_8013B0D8->unk965 = 0;
    dispatchPlayer = D_8013B0D8;
    dispatchPlayer->unk0C8 = 0;
    nextStateCallback = currentPlayerStateCallbacks[dispatchPlayer->unk12C];
    goto block_359;
block_335:
    landingState = D_8013B0D8->unk12C;
    if ((u16)landingState != 0x1D) {
        goto block_337;
    }
    currentPlayerStateCallbacks[landingState]();
    return;
block_337:
    if (terrainContact == 0) {
        goto block_360;
    }
    temp_v1_11 = D_8013B0D8->unk128;
    D_8013B0D8->unk128 = temp_v1_11 & ~0x200;
    if (!(temp_v1_11 & 0x800)) {
        goto block_340;
    }
    D_8013B0D8->trickAttackMakeCount = 0;
    D_8013B0D8->unk965 = 0;
    D_8013B0D8->unk0C8 = 0;
    D_8013B0D8->unk12C = 0xE;
    playCurrentPlayerSoundEffect(2);
    goto block_357;
block_340:
    landingSpeed = D_8013B0D8->unk062;
    if (landingSpeed < 0x1000) {
        queuePlayerControllerEffect(D_8013AE88->data18, 0xB);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x10);
    } else if (landingSpeed < 0x3000) {
        queuePlayerControllerEffect(D_8013AE88->data18, 0xA);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x18);
    } else {
        queuePlayerControllerEffect(D_8013AE88->data18, 9);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x20);
    }
    startCurrentPlayerAnimation(0U, 2U);
    landingSurface = getCourseSurfaceAtPoint(D_8013B0D8->unk92E, (s32) D_8013B0D8->x >> 8, (s32) D_8013B0D8->z >> 8) & 0xFF;
    if (landingSurface == 3) {
        goto block_354;
    }
    if (landingSurface == 6) {
        goto block_354;
    }
    if (landingSurface == 7) {
        goto block_354;
    }
    if (landingSurface == 8) {
        goto block_354;
    }
    if (landingSurface == 0xC) {
        goto block_354;
    }
    if (landingSurface == 0xE) {
        goto block_354;
    }
    if (landingSurface == 0xF) {
        goto block_354;
    }
    if (landingSurface == 0x10) {
        goto block_354;
    }
    createPlayerTripleQuadEffectTask((s32) D_8013AE88->data18);
    awardCurrentPlayerTrickRewards();
block_354:
    if ((s16)tryStartPlayerSelectedRecordTransition((s16) D_8013AE88->data18) == 0) {
        goto block_356;
    }
    D_8013B0D8->unk12C = 7;
    goto block_357;
block_356:
    D_8013B0D8->unk12C = 0;
block_357:
    dispatchPlayer = D_8013B0D8;
    nextStateCallback = currentPlayerStateCallbacks[dispatchPlayer->unk12C];
block_359:
    nextStateCallback();
block_360:
    return;
}

void updateCurrentPlayerState29MotionAndAnimation(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    player = D_8013B0D8;
    player->unk09E = player->unk09A;
    player->unk110 = player->unk138;
    player->unk112 = player->unk13A;
    if (player->unk128 & 0x400) {
        player->unk110 += 0x800;
        player->unk112 = -(player->unk112 & 0xFFF);
    }
    D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    D_8013B0D8->unk0B8 = D_8013B0D8->unk112;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_800EB22C.vx = D_8013B0D8->unk060;
    D_800EB22C.vz = D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    D_8013B0D8->unk060 = D_800EB22C.vx;
    D_8013B0D8->unk064 = D_800EB22C.vz;
    updateCurrentPlayerSlopeVelocityWithoutBoost(0, (s16)D_8013B0D8->unk0A6, (s16)D_8013B0D8->unk0A8, (s16)D_8013B0D8->unk0AA);
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    updateCurrentPlayerGroundContact();
    updateCurrentPlayerAnimation();
    if (D_8013B0D8->animationFlags & 0x10) {
        startCurrentPlayerAnimation(0x23, 1);
    }
    D_8013B0D8->unk128 &= ~0x2000000;
    if ((u16)callCurrentPlayerStateCallbackIfDifferent(0x1D)) {
        if (D_8013B0D8->unk12C == 0x16) {
            if (D_8013B0D8->unk062 < 0) {
                D_8013B0D8->unk062 = -D_8013B0D8->unk062;
            }
            D_8013B0D8->unk92C = 0;
            startCurrentPlayerAnimation(1, 1);
        }
    }
}

void updateCurrentPlayerState3MotionAndSteering(void) {
    RacePlayer *player;
    RacePlayer *airbornePlayer;
    RacePlayer *movingPlayer;
    RacePlayer *steeringPlayer;
    RacePlayer *animatedPlayer;
    s32 velocity;
    s16 angle;
    s16 delta;
    s32 difference;
    u32 flags;

    prepareCurrentPlayerFrame();
    smoothAndScaleCurrentPlayerInput(0x400, 0);
    player = D_8013B0D8;
    player->unk130 = 0x1800;
    if (!(player->unk128 & 1)) {
        player->unk128 &= ~0x200;
        player->unk09E = 0x280;
        updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0x80, 0x80, 0x80);
    } else {
        player->unk128 |= 0x200;
        player->unk062 += 0x1C0;
        D_800EB22C.vx = player->unk060;
        D_800EB22C.vz = player->unk064;
        clampVectorToCurrentPlayerLimit(&D_800EB22C);
        airbornePlayer = D_8013B0D8;
        airbornePlayer->unk060 = D_800EB22C.vx;
        airbornePlayer->unk064 = D_800EB22C.vz;
        velocity = airbornePlayer->unk060;
        if (velocity > 0x80) {
            velocity = 0x80;
        }
        if (velocity < -0x80) {
            velocity = -0x80;
        }
        airbornePlayer->unk060 -= velocity;
        velocity = airbornePlayer->unk064;
        if (velocity > 0x80) {
            velocity = 0x80;
        }
        if (velocity < -0x80) {
            velocity = -0x80;
        }
        airbornePlayer->unk064 -= velocity;
    }

    movingPlayer = D_8013B0D8;
    movingPlayer->x += movingPlayer->unk060;
    movingPlayer->y += movingPlayer->unk062;
    movingPlayer->z += movingPlayer->unk064;
    if (movingPlayer->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 && !(movingPlayer->unk128 & 0x40)) {
        angle = getXZHeading(movingPlayer->x, movingPlayer->z,
                             D_80123D80[D_8013AFCE].unk40[0], D_80123D80[D_8013AFCE].unk40[1]);
        steeringPlayer = D_8013B0D8;
        if (steeringPlayer->unk128 & 0x400) {
            angle += 0x800;
        }
        delta = angle - steeringPlayer->unk110;
        difference = delta & 0xFFF;
        angle = difference;
        if (difference > 0x800) {
            angle = difference | 0xF000;
        }
        if ((s16)angle > 0x96) {
            angle = 0x96;
        }
        if ((s16)angle < -0x96) {
            angle = -0x96;
        }
        steeringPlayer->unk0B6 += angle;
    }

    if (!(D_8013B0D8->unk128 & 1)) {
        updateCurrentPlayerMotionEffects();
        updateCurrentPlayerGroundContact();
    } else if (resolveCurrentPlayerTerrainContact()) {
        queuePlayerControllerEffect(D_8013AE88->data18, 0xA);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x18);
        createPlayerTripleQuadEffectTask(D_8013AE88->data18);
    }

    updateCurrentPlayerAnimation();
    animatedPlayer = D_8013B0D8;
    if (animatedPlayer->animationFlags & 0x10) {
        flags = animatedPlayer->unk128;
        animatedPlayer->unk128 = flags & 0xFF0C1FFB;
        if (animatedPlayer->unk12C != 0x18) {
            if (flags & 1) {
                animatedPlayer->unk12C = 0x16;
            } else {
                animatedPlayer->unk12C = 0;
            }
        }
    }
    callCurrentPlayerStateCallbackIfDifferent(3);
}

void updateCurrentPlayerState4MotionAndLanding(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    player = D_8013B0D8;
    player->unk130 = 0x1800;
    if (player->unk128 & 1) {
        D_800EB22C.vx = player->unk060;
        D_800EB22C.vz = player->unk064;
        clampVectorToCurrentPlayerLimit(&D_800EB22C);
        D_8013B0D8->unk060 = D_800EB22C.vx;
        D_8013B0D8->unk064 = D_800EB22C.vz;
        D_8013B0D8->unk062 += 0x280;
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 15;
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 15;
    } else {
        player->unk09E = player->unk09A;
        updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0xC0, 0xC0, 0xC0);
    }
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    if (!(D_8013B0D8->unk128 & 1)) {
        updateCurrentPlayerGroundContact();
    } else if (resolveCurrentPlayerTerrainContact()) {
        queuePlayerControllerEffect(D_8013AE88->data18, 9);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x20);
        createPlayerTripleQuadEffectTask(D_8013AE88->data18);
    }
    updateCurrentPlayerAnimation();
    if (D_8013B0D8->animationFlags & 0x10) {
        if (!(D_8013B0D8->unk128 & 1)) {
            if (!processCurrentPlayerActionRequests(0)) {
                D_8013B0D8->unk12C = 5;
            }
            currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        }
    }
}

void updateCurrentPlayerState5MotionAndTimer(void) {
    RacePlayer *player;
    RacePlayer *motion;
    RacePlayer *grounded;
    s16 input;
    prepareCurrentPlayerFrame();
    smoothAndScaleCurrentPlayerInput(0, 0);
    if (!(D_8013B0D8->unk128 & 1)) {
        updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0x100, 0x100, 0x100);
    }
    motion = D_8013B0D8;
    motion->unk09E = 0x280;
    if (motion->unk062 > 0x7800) {
        motion->unk062 = 0x7800;
    }
    D_8013B0D8->y += D_8013B0D8->unk062;
    updateCurrentPlayerGroundContact();
    updateCurrentPlayerAnimation();
    player = D_8013B0D8;
    if (!(player->unk128 & 1)) {
        input = countCurrentPlayerInputGroups();
        D_8013B0D8->unk08C--;
        D_8013B0D8->unk08C -= input;
        if (D_8013B0D8->unk08C < 0) {
            D_8013B0D8->unk08C = 0;
        }
        grounded = D_8013B0D8;
        if (grounded->animationFlags & 0x10) {
            grounded->unk128 &= ~0x200;
            if (grounded->unk08C == 0) {
                grounded->unk12C = 8;
                currentPlayerStateCallbacks[8]();
                return;
            }
        }
    } else {
        player->unk128 |= 0x200;
        startCurrentPlayerAnimation(13, 2);
        if (D_8013B0D8->unk08C < 30) {
            D_8013B0D8->unk08C = 30;
        }
    }
    callCurrentPlayerStateCallbackIfDifferent(5);
}

void updateCurrentPlayerTimedEffect(void) {
    GameTask *task;
    s32 nextX, nextY, nextZ;
    u16 angle;
    s32 xOffset, zOffset;

    prepareCurrentPlayerFrame();
    updateCurrentPlayerGroundContact();
    updateCurrentPlayerAnimation();
    if (D_8013B0D8->animationFlags & 0x10) {
        startCurrentPlayerAnimation(17, 1);
    }

    task = D_8013AE88;
    switch (task->data1A) {
    case 0:
        if (--D_8013B0D8->unk08C != 0) {
            break;
        }
        task->data1A++;
        D_8013B0D8->unk08C = 30;
        if (D_8013B0D8->unk008 == 0) {
            if (task->data18 == 0) {
                configureTaskStateTransition(2, 0x1000, D_8013B0D8->unk08C);
            } else {
                configureTaskStateTransition(2, 0x2000, D_8013B0D8->unk08C);
            }
        }
        break;
    case 1:
        if (--D_8013B0D8->unk08C != 0) {
            break;
        }
        task->data1A++;
        D_8013B0D8->unk08C = 30;
        if (D_8013B0D8->unk008 == 0) {
            if (task->data18 == 0) {
                configureTaskStateTransition(1, 0x1000, D_8013B0D8->unk08C);
            } else {
                configureTaskStateTransition(1, 0x2000, D_8013B0D8->unk08C);
            }
        }
        if (D_8013AE88->data1C == 0) {
            for (;;) {
                getGeometryPlacementTransform(D_8013B0D8->unk92E, &D_8013B0D8->x, &D_8013B0D8->y,
                                              &D_8013B0D8->z, &D_8013B0D8->unk110);
                if (checkCurrentPlayerSurfaceType()) {
                    break;
                }
                D_8013B0D8->unk92E--;
            }
        } else {
            /* Reuse the probe counter as the output angle below. */
            angle = 0;
            do {
                if (getCourseSurfaceAtPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) == 8) {
                    D_8013B0D8->unk92E++;
                    break;
                }
                D_8013B0D8->unk92E++;
                angle++;
            } while ((s16)angle != 3);
            getGeometryPlacementTransform(D_8013B0D8->unk92E, &D_8013B0D8->x, &D_8013B0D8->y,
                                              &D_8013B0D8->z, &D_8013B0D8->unk110);
            getGeometryPlacementTransform(D_8013B0D8->unk92E + 1, &nextX, &nextY, &nextZ, &angle);
            xOffset = rcos((s16)angle) * gPlayerTimedEffectPlacementOffsets[D_8013AE88->data18];
            D_8013B0D8->x += (nextX - D_8013B0D8->x) / 2 + xOffset / 4096;
            zOffset = rsin((s16)angle) * gPlayerTimedEffectPlacementOffsets[D_8013AE88->data18];
            D_8013B0D8->z += (nextZ - D_8013B0D8->z) / 2 - zOffset / 4096;
        }

        D_8013B0D8->unk0B8 = 0;
        D_8013B0D8->unk0B4 = 0;
        D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
        D_8013B0D8->y = getCourseHeightAtPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
        /* Preserve the fourth word of the position snapshot. */
        *(VECTOR *)&D_8013B0D8->unk050 = *(VECTOR *)&D_8013B0D8->x;
        D_8013B0D8->unk128 &= 0xFFFBDBFF;
        smoothAndScaleCurrentPlayerInput(0, 0);
        startCurrentPlayerAnimation(1, 1);
        if (D_8013B0D8->unk008 == 0) {
            D_801241E8[D_8013AE88->data18].unk16 = 1;
            D_801241E8[D_8013AE88->data18].yaw = D_8013B0D8->unk110;
            D_801241E8[D_8013AE88->data18].pitch = -0x40;
            updatePlayerCameraPosition(D_8013AE88->data18);
            clampPlayerCameraHeight(D_8013AE88->data18);
        }
        D_8013B0D8->unk078 = 0x400;
        break;
    case 2:
        if (--D_8013B0D8->unk08C != 0) {
            break;
        }
        if (D_8013B0D8->unk12C != 24) {
            D_8013B0D8->unk12C = 0;
        }
        D_8013B0D8->unk128 &= ~8;
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        return;
    }
    if (D_8013B0D8->unk12C != 6 && D_8013B0D8->unk12C != 9) {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    }
}

void updateCurrentPlayerState23MotionAndTimer(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    player = D_8013B0D8;
    if (!(player->unk128 & 1)) {
        player->unk128 &= ~0x200;
        updateCurrentPlayerGroundContact();
    } else {
        player->unk128 |= 0x200;
        player->unk062 += 0x240;
        if (player->unk062 > 0x7800) {
            player->unk062 = 0x7800;
        }
        D_8013B0D8->y += D_8013B0D8->unk062;
        resolveCurrentPlayerTerrainContact();
    }
    D_8013B0D8->unk08C -= 1 + countCurrentPlayerInputGroups();
    if (D_8013B0D8->unk08C < 0) {
        D_8013B0D8->unk08C = 0;
    }
    if (D_8013B0D8->unk08C == 0 && D_8013B0D8->unk12C != 24) {
        D_8013B0D8->unk12C = 4;
        D_8013B0D8->unk108.word = 0;
        D_8013B0D8->unk104.word = 0;
    }
    callCurrentPlayerStateCallbackIfDifferent(23);
}

void updateCurrentPlayerState24MotionAndScale(void) {
    RacePlayer *player;
    u16 timerStep;
    s16 landed;

    prepareCurrentPlayerFrame();
    smoothAndScaleCurrentPlayerInput(0, 0);
    if (D_8013B0D8->unk128 & 1) {
        D_8013B0D8->unk062 += 0x280;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->unk128 |= 0x200;
    } else {
        D_8013B0D8->unk128 &= ~0x200;
    }
    if (!(D_8013B0D8->unk128 & 1)) {
        updateCurrentPlayerGroundContact();
    } else if ((landed = resolveCurrentPlayerTerrainContact()) != 0) {
        queuePlayerControllerEffect(D_8013AE88->data18, 9);
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x20);
        createPlayerTripleQuadEffectTask(D_8013AE88->data18);
    }

    switch (D_8013AE88->data1A) {
    case 0:
        if (D_8013B0D8->unk0F8 != 1) {
            D_8013B0D8->unk0F8--;
        }
        timerStep = 1 + countCurrentPlayerInputGroups();
        if (timerStep != 1) {
            D_8013B0D8->unk0F8 = 3;
        }
        D_8013B0D8->unk08C -= timerStep;
        if (D_8013B0D8->unk08C < 0) {
            D_8013B0D8->unk08C = 0;
        }
        if (D_8013B0D8->unk08C == 0) {
            D_8013AE88->data1A++;
            D_8013B0D8->unk0F8 = 1;
        }
        break;
    case 1:
        if (++D_8013B0D8->unk0F8 == 7) {
            D_8013B0D8->unk0F8 = 2;
            D_8013AE88->data1A++;
        }
        break;
    case 2:
        if (++D_8013B0D8->unk0F8 == 8) {
            player = D_8013B0D8;
            player->unk12C = 0;
            player->unk128 &= 0xFF0C1FFB;
            currentPlayerStateCallbacks[0]();
            D_8013B0D8->unk0F6 = 60;
            return;
        }
        break;
    }
    updateCurrentPlayerPartTransforms();
}

void updateCurrentPlayerState25MotionAndTimer(void) {
    prepareCurrentPlayerFrame();
    if (!(D_8013B0D8->unk128 & 1)) {
        D_8013B0D8->unk128 &= ~0x200;
        smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk094, 0);
        updateCurrentPlayerSlopeVelocity(0, 0, (s16)D_8013B0D8->unk0A6, (s16)D_8013B0D8->unk0A8, (s16)D_8013B0D8->unk0AA);
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        updateCurrentPlayerGroundContact();
    } else {
        D_8013B0D8->unk128 |= 0x200;
        D_800EB22C.vx = D_8013B0D8->unk060;
        D_800EB22C.vz = D_8013B0D8->unk064;
        clampVectorToCurrentPlayerLimit(&D_800EB22C);
        D_8013B0D8->unk060 = D_800EB22C.vx;
        D_8013B0D8->unk064 = D_800EB22C.vz;
        D_8013B0D8->unk062 += D_8013B0D8->unk09C;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        if (resolveCurrentPlayerTerrainContact()) {
            queuePlayerControllerEffect(D_8013AE88->data18, 10);
            playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x18);
            createPlayerTripleQuadEffectTask(D_8013AE88->data18);
        }
    }
    D_8013B0D8->unk08C -= 1 + countCurrentPlayerInputGroups();
    if (D_8013B0D8->unk08C < 0) {
        D_8013B0D8->unk08C = 0;
    }
    if (D_8013B0D8->unk08C == 0) {
        D_8013B0D8->unk128 &= 0xFF0C1FFB;
        if (D_8013B0D8->unk12C != 24) {
            if (!(D_8013B0D8->unk128 & 0x200)) {
                D_8013B0D8->unk12C = 0;
            } else {
                D_8013B0D8->unk12C = 1;
            }
        }
    }
    callCurrentPlayerStateCallbackIfDifferent(25);
}

void updateCurrentPlayerVerticalMotionUntilLanding(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    player = D_8013B0D8;
    player->unk062 += 0x1C0;
    player->y += player->unk062;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
    if (player->unk062 > 0) {
        if (resolveCurrentPlayerTerrainContact() != 0) {
            D_8013B0D8->unk12C = 12;
            currentPlayerStateCallbacks[12]();
            return;
        }
    } else {
        player->unk946 = 0;
    }
    updateCurrentPlayerAnimation();
}

void updateCurrentPlayerAirborneDragUntilLanding(void) {
    prepareCurrentPlayerFrame();
    smoothAndScaleCurrentPlayerInput(0, 0);
    if (D_8013B0D8->unk062 < 0) {
        D_8013B0D8->unk062 += 0x150;
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 64;
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 64;
    } else {
        if (D_8013AE88->data1C == 0) {
            D_8013AE88->data1C++;
            createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerCoordinateEffectTask);
            D_8013AE8C->data18 = D_8013AE88->data18;
        }
        D_8013B0D8->unk0B6 += 0x40;
        D_8013B0D8->unk062 += 0x18;
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 48;
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 48;
    }
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
    D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
    D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
    if (resolveCurrentPlayerTerrainContact() != 0) {
        D_8013B0D8->unk12C = 8;
        currentPlayerStateCallbacks[8]();
    } else {
        updateCurrentPlayerAnimation();
    }
}

void updateCurrentPlayerFromTaskPhase(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    updateCurrentPlayerAnimation();
    gCurrentPlayerTaskPhaseCallbacks[D_8013AE88->data1A]();
    D_8013B0D8->unk946 = 0;
    player = D_8013B0D8;
    player->unk110 = player->unk0B6 + 0x400;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
}

void moveCurrentPlayerToTaskTarget(void) {
    s16 angle;
    s32 difference;
    RacePlayer *player;
    difference = (D_8013B0D8->unk126 - (u16)D_8013B0D8->unk0B6) & 0xFFF;
    angle = difference;
    if (difference > 0x800) {
        angle = difference | 0xF000;
    }
    smoothAndScaleCurrentPlayerInput(0x100, 0);
    player = D_8013B0D8;
    if (player->unk090 != 0) {
        player->unk0B6 += angle / player->unk090;
        player->x += ((s32)D_80123D80[D_8013AFCE].unk18[0] - player->x) / player->unk090;
        player->z += ((s32)D_80123D80[D_8013AFCE].unk18[2] - player->z) / player->unk090;
    }
    player = D_8013B0D8;
    if (--player->unk08C == 0) {
        player->x = D_80123D80[D_8013AFCE].unk18[0];
        player->z = D_80123D80[D_8013AFCE].unk18[2];
        D_8013AE88->data1A++;
        player->unk08C = 0;
        startCurrentPlayerAnimation(3, 2);
        D_801241E8[D_8013AE88->data18].unk16 = 3;
    }
}

void advanceCurrentPlayerTaskOnAnimationFlag(void) {
    RacePlayer *player;
    if (D_8013B0D8->animationFlags & 0x10) {
        D_8013AE88->data1A++;
        playPlayerSoundEffect(D_8013AE88->data18, 1, 0x601, 0x20);
        player = D_8013B0D8;
        player->unk0B6 = player->unk126;
        player->x = D_80123D80[D_8013AFCE].unk18[0];
        player->z = D_80123D80[D_8013AFCE].unk18[2];
        player->unk062 = -0xC00;
        player->unk08C = 30;
        startCurrentPlayerAnimation(4, 2);
        if (D_8013B0D8->unk128 & 0x400) {
            D_8013B0D8->unk090 = -0x800;
        } else {
            D_8013B0D8->unk090 = 0x800;
        }
    }
}

void updateCurrentPlayerTaskJumpAndSpawnCourseOffset(void) {
    RacePlayer *player;
    RacePlayer *landingPlayer;
    CourseOffsetResource *resource;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    player = D_8013B0D8;
    player->x += D_8013AF88->vx;
    player->z += D_8013AF88->vz;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = player->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    landingPlayer = D_8013B0D8;
    landingPlayer->x += D_8013AF88->vx;
    landingPlayer->z += D_8013AF88->vz;
    landingPlayer->unk062 += 0x140;
    landingPlayer->y += landingPlayer->unk062;
    if (landingPlayer->unk062 > 0 && landingPlayer->y > (s32)D_80123D80[D_8013AFCE].unk18[1]) {
        landingPlayer->y = D_80123D80[D_8013AFCE].unk18[1];
        if (landingPlayer->animationIndex != D_800EB09C[landingPlayer->characterId][1]) {
            startCurrentPlayerAnimation(1, 1);
            playPlayerSoundEffect(D_8013AE88->data18, 1, 0x701, 0x20);
            D_8013AFE4 &= ~4;
        }
        updateCurrentPlayerMotionEffects();
        D_8013B0D8->unk128 &= ~0x200;
    } else {
        D_8013B0D8->unk128 |= 0x200;
    }
    player = D_8013B0D8;
    if (--player->unk08C == 0) {
        D_8013AE88->data1A++;
        player->unk08C = 0x20;
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeSelectedCourseOffsetTask);
        D_8013AE8C->data18 = D_8013AE88->data18;
        D_8013AE8C->resource14 = allocateTaskMemoryBlock();
        resource = (CourseOffsetResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
        resource->linkedTask = D_8013AE88;
        D_8013B0D8->unk128 &= ~0x20;
    }
}

void updateCurrentPlayerTaskDecelerationSteps(void) {
    RacePlayer *player = D_8013B0D8;
    RacePlayer *firstStep;
    RacePlayer *second;
    RacePlayer *last;
    player->unk0B6 -= 0x10;
    if (player->unk090 > 0) {
        player->unk090 -= 0x20;
    } else {
        player->unk090 += 0x20;
    }
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    firstStep = D_8013B0D8;
    firstStep->x += D_8013AF88->vx;
    firstStep->z += D_8013AF88->vz;
    firstStep->unk0B6 -= 0x10;
    if (firstStep->unk090 > 0) {
        firstStep->unk090 -= 0x20;
    } else {
        firstStep->unk090 += 0x20;
    }
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    second = D_8013B0D8;
    second->x += D_8013AF88->vx;
    second->z += D_8013AF88->vz;
    updateCurrentPlayerMotionEffects();
    if (--D_8013B0D8->unk08C == 0) {
        D_8013AE88->data1A++;
        startCurrentPlayerAnimation(25, 2);
        last = D_8013B0D8;
        last->unk08C = 19;
        last->unk128 |= 0x200;
    }
}

void advanceCurrentPlayerTaskAndSetSpeed(void) {
    if (--D_8013B0D8->unk08C == 0) {
        D_8013AE88->data1A++;
        D_8013B0D8->unk08C = 19;
        if (D_8013B0D8->unk128 & 0x400) {
            D_8013B0D8->unk090 = -0x1000;
        } else {
            D_8013B0D8->unk090 = 0x1000;
        }
    }
}

void advanceCurrentPlayerTaskPhaseMovement(void) {
    RacePlayer *player;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    player = D_8013B0D8;
    player->x += D_8013AF88->vx;
    player->z += D_8013AF88->vz;
    if (--player->unk08C == 0) {
        D_8013AE88->data1A++;
        player->unk08C = 100;
    }
}

void updateCurrentPlayerTaskLapTransition(void) {
    RacePlayer *player;
    RacePlayer *movingPlayer;
    RacePlayer *placedPlayer;
    RacePlayer *phasePlayer;
    const SelectedCourseRecord *course;
    GameTask *task;
    s16 segment;

    if (D_8013B0D8->unk090 > 0) {
        if (D_8013B0D8->unk090 < 0x2800) {
            D_8013B0D8->unk090 += 0x80;
        }
        D_8013AF90->vx = 0x100;
        D_8013AF90->vy = D_8013B0D8->unk110;
        D_8013AF90->vz = 0;
    } else {
        if (D_8013B0D8->unk090 >= -0x27FF) {
            D_8013B0D8->unk090 -= 0x80;
        }
        D_8013AF90->vx = -0x100;
        D_8013AF90->vy = D_8013B0D8->unk110;
        D_8013AF90->vz = 0;
    }
    RotMatrixYXZ(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    movingPlayer = D_8013B0D8;
    movingPlayer->x += D_8013AF88->vx;
    movingPlayer->y += D_8013AF88->vy;
    movingPlayer->z += D_8013AF88->vz;
    if (--movingPlayer->unk08C == 0) {
        D_8013AE88->data1A++;
        movingPlayer->unk0B6 = D_80123D80[D_8013AFCE].unk14;
        movingPlayer->unk110 = movingPlayer->unk0BE = movingPlayer->unk0B6 + 0x400;
        D_8013AF90->vx = 0;
        D_8013AF90->vy = movingPlayer->unk110;
        D_8013AF90->vz = 0;
        RotMatrix(D_8013AF90, D_8013AF98);
        D_8013AF88->vx = 0;
        D_8013AF88->vy = 0;
        D_8013AF88->vz = -0x28000;
        ApplyMatrixLV(D_8013AF98, D_8013AF88, (VECTOR *)&D_8013B0D8->x);
        course = &D_80123D80[D_8013AFCE];
        placedPlayer = D_8013B0D8;
        placedPlayer->x += course->unk08;
        placedPlayer->y += course->unk0C - 0x2000;
        placedPlayer->z += course->unk10;
        segment = course->unk04;
        placedPlayer->unk934++;
        placedPlayer->unk92E = segment;
        /* Preserve the complete four-word position vector. */
        *(VECTOR *)&D_8013B0D8->unk050 = *(VECTOR *)&D_8013B0D8->x;
        phasePlayer = D_8013B0D8;
        phasePlayer->unk08C = 40;
        phasePlayer->animationFlags &= ~0x10;
        phasePlayer->unk128 &= ~0x04000400;
        if (D_8013B0D8->unk008 == 0) {
            PlayerCamera *cameras = D_801241E8;
            cameras[D_8013AE88->data18].unk16 = 7;
            task = D_8013AE88;
            cameras[task->data18].yaw = D_8013B0D8->unk110;
            cameras[task->data18].pitch = 0;
            cameras[task->data18].unk18 = D_8013B0D8->unk92E;
            updatePlayerCameraPosition(task->data18);
            clampPlayerCameraHeight(D_8013AE88->data18);
            if (D_8013AE88->data18 == 0) {
                configureTaskStateTransition(1, 0x1000, 60);
            } else {
                configureTaskStateTransition(1, 0x2000, 60);
            }
            if (D_8013B0D8->unk934 >= selectedFinishLapCountAbsolute - 1) {
                createChildTaskWithReserve(initializeTaskMotionResource);
                D_8013AE8C->data18 = D_8013AE88->data18;
            }
        }
    }
    player = D_8013B0D8;
    if (player->unk08C == 60 && player->unk008 == 0) {
        if (D_8013AE88->data18 == 0) {
            configureTaskStateTransition(2, 0x1000, player->unk08C);
        } else {
            configureTaskStateTransition(2, 0x2000, player->unk08C);
        }
    }
}

void moveCurrentPlayerForwardUntilState22(void) {
    RacePlayer *player;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = 0x1000;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    player = D_8013B0D8;
    player->x += D_8013AF88->vx;
    player->z += D_8013AF88->vz;
    if (--player->unk08C == 0) {
        player->unk12C = 22;
        player->unk128 = 0;
        player->unk0E8 = 0;
        switch (player->unk935) {
        case 1:
            player->x += player->unk060;
            player->z += player->unk064;
            break;
        case 2:
            player->x += player->unk060 * 2;
            player->z += player->unk064 * 2;
            break;
        case 3:
            player->x += player->unk060 * 4;
            player->z += player->unk064 * 4;
            break;
        }
        D_8013B0D8->unk062 = 0;
        D_8013B0D8->unk094 = 0;
        D_8013B0D8->unk943 = 0;
        startCurrentPlayerAnimation(4, 2);
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    }
}

void updateCurrentPlayerAndRequestState30(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    gCurrentPlayerState30TaskPhaseCallbacks[D_8013AE88->data1A]();
    if (D_8013B0D8->unk0AC < 0x3C00) {
        D_8013B0D8->unk0AC += 0x200;
    }
    player = D_8013B0D8;
    player->unk110 = player->unk0B6 + 0x400;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
    callCurrentPlayerStateCallbackIfDifferent(30);
}

void updateCurrentPlayerState30TaskUntilStopped(void) {
    RacePlayer *player;
    RacePlayer *steeringPlayer;
    GameTask *task = D_8013AE88;
    RacePlayer *animationPlayer;
    RacePlayer *movingPlayer;
    RacePlayer *stoppedPlayer;
    RacePlayer *finishedPlayer;
    s16 turn;
    s32 difference;

    if (task->data1E != 0) {
        player = D_8013B0D8;
        difference = (player->unk0BE - player->unk110) & 0xFFF;
        turn = difference;
        if (difference > 0x800) {
            turn = difference | 0xF000;
        }
        if (turn > 0x40) {
            turn = 0x40;
        }
        if (turn < -0x40) {
            turn = -0x40;
        }
        player->unk0B6 -= turn;
        if (task->data1C) {
            player->unk08C = 31;
        } else {
            player->unk08C = -31;
        }
        D_8013B0D8->unk124 = 63;
        turn = smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk0BC, (s16)D_8013B0D8->unk08C);
        steeringPlayer = D_8013B0D8;
        turn -= steeringPlayer->unk126;
        if (turn > 0x30) {
            turn = 0x30;
        }
        if (turn < -0x30) {
            turn = -0x30;
        }
        steeringPlayer->unk126 += turn;
        if ((s16)steeringPlayer->unk126 == 0) {
            if (steeringPlayer->animationIndex != D_800EB09C[steeringPlayer->characterId][1]) {
                startCurrentPlayerAnimation(1, 1);
            }
        } else {
            if ((s16)steeringPlayer->unk126 >= 0) {
                blendCurrentPlayerSteeringPose(6, (s16)steeringPlayer->unk126, 0x118);
                if ((s16)D_8013B0D8->unk126 >= 0) {
                    goto updateAnimation;
                }
            }
            blendCurrentPlayerSteeringPose(5, -(s16)D_8013B0D8->unk126, 0x118);
        }
    } else {
        smoothAndScaleCurrentPlayerInput(0, 0);
    }

updateAnimation:
    animationPlayer = D_8013B0D8;
    if (animationPlayer->animationIndex != D_800EB09C[animationPlayer->characterId][6] &&
        animationPlayer->animationIndex != D_800EB09C[animationPlayer->characterId][5]) {
        updateCurrentPlayerAnimation();
    }
    updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0x200, 0x180, 0x180);
    movingPlayer = D_8013B0D8;
    movingPlayer->x += movingPlayer->unk060;
    movingPlayer->y += movingPlayer->unk062;
    movingPlayer->z += movingPlayer->unk064;
    updateCurrentPlayerGroundContact();
    updateCurrentPlayerMotionEffects();

    stoppedPlayer = D_8013B0D8;
    if ((stoppedPlayer->unk060 | stoppedPlayer->unk064) == 0 && !(stoppedPlayer->unk128 & 1)) {
        if (stoppedPlayer->unk008 == 0) {
            PlayerCamera *cameras = D_801241E8;
            PlayerCamera *camera = &cameras[D_8013AE88->data18];
            if (camera->unk16 != 6 && D_8013AFD6 == 0) {
                camera->unk16 = 5;
            }
        }
        finishedPlayer = D_8013B0D8;
        finishedPlayer->unk128 |= 0x100;
        D_8013AE88->data1A++;
        if (finishedPlayer->animationIndex != D_800EB09C[finishedPlayer->characterId][1]) {
            startCurrentPlayerAnimation(1, 1);
        }
    }
}

void advanceCurrentPlayerState30TaskOnResult(void) {
    if (D_8013AFE1 != 0) {
        if (D_8013AFE2 == 0) {
            if (D_8013B0D8->unk935 == 0) {
                D_8013AE88->data1C = 0;
                startCurrentPlayerAnimation(0x13, 2);
            } else {
                D_8013AE88->data1C = 2;
                startCurrentPlayerAnimation(0x15, 2);
            }
        } else {
            if (D_8013AE88->data18 + 1 == D_8013AFE1) {
                D_8013AE88->data1C = 0;
                startCurrentPlayerAnimation(0x13, 2);
            } else {
                D_8013AE88->data1C = 2;
                startCurrentPlayerAnimation(0x15, 2);
            }
        }
        D_8013AE88->data1A++;
        D_8013B0D8->unk08C = 0;
    } else {
        smoothAndScaleCurrentPlayerInput(0, 0);
        updateCurrentPlayerGroundContact();
        updateCurrentPlayerAnimation();
    }
}

void updateCurrentPlayerState30AnimationAndHeight(void) {
    RacePlayer *player = D_8013B0D8;
    if (player->unk08C == 0) {
        if (player->animationFlags & 0x10) {
            if (!(D_8013AE88->data1C & 1)) {
                D_8013AE88->data1C++;
                if (!(D_8013AE88->data1C & 2)) {
                    if (!D_8013AFD6) playCurrentPlayerSoundEffect(6);
                    startCurrentPlayerAnimation(0x14, 2);
                } else {
                    startCurrentPlayerAnimation(0x16, 2);
                }
            } else {
                if (!(D_8013AE88->data1C & 2)) {
                    player->unk08C = gPlayerState30Animation20Countdowns[player->characterId];
                    startCurrentPlayerAnimation(0x14, 2);
                } else {
                    player->unk08C = gPlayerState30Animation22Countdowns[player->characterId];
                    startCurrentPlayerAnimation(0x16, 2);
                }
            }
        }
        updateCurrentPlayerAnimation();
    } else {
        player->unk08C--;
    }
    D_8013B0D8->unk946 = 0;
    if (processCurrentPlayerActionRequests(0)) {
        D_8013B0D8->unk92E = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
        D_8013B0D8->y = getCourseHeightAtPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    }
}

void updateCurrentPlayerTaskCountdown(void) {
    RacePlayer *player;
    prepareCurrentPlayerFrame();
    D_8013B0D8->unk946 = 0;
    player = D_8013B0D8;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
    updateCurrentPlayerAnimation();
    if (D_8013AE88->data18 == 0) {
        if (--D_8013AE88->data1A == 0) {
            playCurrentPlayerSoundEffect(6);
        }
    }
}

void updateCurrentPlayerState8Steering(void) {
    s16 angle;
    RacePlayer *player;
    s32 difference;
    s16 delta;
    prepareCurrentPlayerFrame();
    if (D_8013B0D8->unk128 & 1) {
        D_8013B0D8->unk128 |= 0x200;
    } else {
        D_8013B0D8->unk128 &= ~0x200;
    }
    player = D_8013B0D8;
    if (player->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 && !(player->unk128 & 0x40)) {
        angle = getXZHeading(player->x, player->z, D_80123D80[D_8013AFCE].unk40[0], D_80123D80[D_8013AFCE].unk40[1]);
        player = D_8013B0D8;
        if (player->unk128 & 0x400) {
            angle += 0x800;
        }
        delta = angle - player->unk110;
        difference = delta & 0xFFF;
        angle = difference;
        if (difference > 0x800) {
            angle = difference | 0xF000;
        }
        if (angle > 0x66) {
            angle = 0x66;
        }
        if (angle < -0x66) {
            angle = -0x66;
        }
        player->unk0B6 += angle;
    } else {
        if (player->unk008 == 0 || player->unk92E != player->unk962) {
            goto movement;
        }
        angle = ((GeometryPlacement *)geometryPlacementDataAbsolute)[player->unk92E].angle;
        if (player->unk128 & 0x400) {
            angle += 0x800;
        }
        delta = angle - player->unk110;
        difference = delta & 0xFFF;
        angle = difference;
        if (difference > 0x800) {
            angle = difference | 0xF000;
        }
        if (angle > 0x66) {
            angle = 0x66;
        }
        if (angle < -0x66) {
            angle = -0x66;
        }
        player->unk0B6 += angle;
    }
movement:
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk09E = 0x280;
    updateCurrentPlayerGroundContact();
    updateCurrentPlayerAnimation();
    if (++D_8013AE88->data1A < 15) {
        D_8013B0D8->unk128 |= 8;
    } else {
        D_8013B0D8->unk128 &= ~8;
    }
    if (D_8013B0D8->animationFlags & 0x10) {
        D_8013B0D8->unk962 = D_8013B0D8->unk92E;
        D_8013B0D8->unk128 &= 0xFF0C1FFB;
        if (D_8013B0D8->unk12C != 24) {
            D_8013B0D8->unk12C = 0;
        }
    }
    if ((u16)callCurrentPlayerStateCallbackIfDifferent(8)) {
        D_8013B0D8->unk128 &= ~8;
    }
}

void updateCurrentPlayerState11Motion(void) {
    prepareCurrentPlayerFrame();
    D_8013B0D8->unk130 = 0x1800;
    D_800EB22C.vx = D_8013B0D8->unk060;
    D_800EB22C.vz = D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    D_8013B0D8->unk060 = D_800EB22C.vx;
    D_8013B0D8->unk064 = D_800EB22C.vz;
    if (!(D_8013B0D8->unk128 & 1)) {
        D_8013B0D8->unk09E = D_8013B0D8->unk09C;
        updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0x100, 0x100, 0x100);
    } else {
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 16;
        D_8013B0D8->unk062 += D_8013B0D8->unk09C;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 16;
    }
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    if (!(D_8013B0D8->unk128 & 1)) {
        updateCurrentPlayerGroundContact();
    } else {
        resolveCurrentPlayerTerrainContact();
    }
    updateCurrentPlayerAnimation();
    if (!(D_8013B0D8->unk128 & 1) && (D_8013B0D8->animationFlags & 0x10)) {
        D_8013B0D8->unk12C = 12;
        currentPlayerStateCallbacks[12]();
    }
}

void updateCurrentPlayerState10Movement(void) {
    prepareCurrentPlayerFrame();
    smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk094, 0);
    if (D_8013B0D8->unk062 < 0) {
        D_8013B0D8->unk060 = 0;
        D_8013B0D8->unk062 = 0;
        D_8013B0D8->unk064 = 0;
    }
    D_8013B0D8->unk130 = 0x2000;
    D_8013B0D8->unk09E = D_8013B0D8->unk09A;
    updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0, 0, 0);
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    D_8013B0D8->unk110 = getXZHeading(0, 0, D_8013B0D8->unk060, D_8013B0D8->unk064);
    if (D_8013B0D8->unk128 & 0x400) {
        D_8013B0D8->unk110 += 0x800;
    }
    D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    snapCurrentPlayerToTerrain();
    updateCurrentPlayerAnimation();
    if (D_8013B0E4 != 0) {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    } else if (!(D_8013B0D8->unk128 & 0x201)) {
        if (D_8013B0D8->surfaceState.fields.unk136 != 3 && D_8013B0D8->surfaceState.fields.unk136 != 6 &&
            D_8013B0D8->surfaceState.fields.unk136 != 12 && D_8013B0D8->surfaceState.fields.unk136 != 16 &&
            D_8013B0D8->surfaceState.fields.unk136 != 8 && D_8013B0D8->surfaceState.fields.unk136 != 7 &&
            D_8013B0D8->surfaceState.fields.unk136 != 14 && D_8013B0D8->surfaceState.fields.unk136 != 15) {
            D_801241E8[D_8013AE88->data18].unk16 = 1;
            initializeCurrentPlayerState14();
        } else {
            D_8013B0D8->unk96E = 0x500;
            D_8013B0D8->unk972 = 0;
            D_8013B0D8->unk96C = 0x20;
        }
    }
}

void updateCurrentPlayerTimedMotion(void) {
    prepareCurrentPlayerFrame();
    if (!(D_8013B0D8->unk128 & 1)) {
        smoothAndScaleCurrentPlayerInput(0x400, 0);
        D_8013B0D8->unk09E = 0x280;
        D_8013B0D8->unk128 &= ~0x200;
        updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0, 0, 0);
    } else {
        D_8013B0D8->unk062 += D_8013B0D8->unk09C;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->unk128 |= 0x200;
    }
    D_800EB22C.vx = D_8013B0D8->unk060;
    D_800EB22C.vz = D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    D_8013B0D8->unk060 = D_800EB22C.vx;
    D_8013B0D8->unk064 = D_800EB22C.vz;
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    if (!(D_8013B0D8->unk128 & 1)) {
        updateCurrentPlayerGroundContact();
        D_8013B0D8->unk96E = 0x500;
        D_8013B0D8->unk972 = 0;
        D_8013B0D8->unk96C = 0x20;
    } else {
        resolveCurrentPlayerTerrainContact();
    }
    D_8013B0D8->unk110 = getXZHeading(0, 0, D_8013B0D8->unk060, D_8013B0D8->unk064);
    if (D_8013B0D8->unk128 & 0x400) {
        D_8013B0D8->unk110 += 0x800;
    }
    D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    updateCurrentPlayerAnimation();
    if (D_8013B0E4) {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    } else if (D_8013B0D8->unk08C == 0) {
        D_8013B0D8->unk12C = 12;
        currentPlayerStateCallbacks[12]();
    } else {
        D_8013B0D8->unk08C--;
    }
}

void updateCurrentPlayerState12MotionAndTimer(void) {
    RacePlayer *player;
    RacePlayer *grounded;
    s16 input;

    prepareCurrentPlayerFrame();
    smoothAndScaleCurrentPlayerInput(0, 0);
    player = D_8013B0D8;
    if (!(player->unk128 & 1)) {
        player->unk09E = 0x280;
        updateCurrentPlayerSlopeVelocityWithoutBoost(0, 0x180, 0x180, 0x180);
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        updateCurrentPlayerGroundContact();
        updateCurrentPlayerAnimation();
        if (++D_8013AE88->data1A >= 8) {
            D_8013B0D8->unk128 |= 8;
        } else {
            D_8013B0D8->unk128 &= ~8;
        }
        updateCurrentPlayerMotionEffects();
        if (D_8013B0D8->unk96E != 0x300 && D_8013B0D8->unk96E != 0x801) {
            D_8013B0D8->unk972 = 0x400;
        }
        if (D_8013B0E4) {
            D_8013B0D8->unk128 &= ~8;
            currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        } else {
            input = countCurrentPlayerInputGroups();
            D_8013B0D8->unk08C--;
            D_8013B0D8->unk08C -= input;
            if (D_8013B0D8->unk08C < 0) {
                D_8013B0D8->unk08C = 0;
            }
            grounded = D_8013B0D8;
            if (grounded->animationFlags & 0x10) {
                grounded->unk128 &= ~0x200;
                if (grounded->unk08C == 0) {
                    grounded->unk12C = 8;
                    grounded->unk128 &= ~8;
                    currentPlayerStateCallbacks[grounded->unk12C]();
                }
            }
        }
    } else {
        player->unk062 += player->unk09C;
        if (player->unk062 > 0x7800) {
            player->unk062 = 0x7800;
        }
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        if (resolveCurrentPlayerTerrainContact()) {
            queuePlayerControllerEffect(D_8013AE88->data18, 9);
            playPlayerSoundEffect(D_8013AE88->data18, 1, 0xB01, 0x20);
            createPlayerTripleQuadEffectTask(D_8013AE88->data18);
        }
        D_8013B0D8->unk08C = 30;
        D_8013B0D8->unk128 |= 0x200;
        D_8013B0D8->unk090 = 7;
        D_8013B0D8->unk128 &= ~8;
    }
}

void resolveCurrentPlayerWallContact(void) {
    VECTOR probes[2];
    s32 dx, dz;
    s32 responseX, responseZ;
    s32 region;
    s32 radius;
    s8 adjusted;
    s16 i;
    s32 sine, cosine;
    s32 velocityX, velocityZ;

    D_8013B0D8->unk943 = 0;
    D_8013B0D8->unk965 = 0;
    D_8013B0D8->unk946 = 0;
    D_8013B0D8->unk0EC = D_8013B0D8->unk060;
    D_8013B0D8->unk0F0 = D_8013B0D8->unk064;
    D_8013B0D8->unk074 = D_8013B0D8->y + 0x1800;
    if (D_8013B0D8->unk0F6) {
        D_8013B0D8->unk0F6--;
    }
    if (D_8013B0D8->unk0FC) {
        D_8013B0D8->unk96E = 0x400;
        D_8013B0D8->unk972 = 0;
        D_8013B0D8->unk96C = 0x20;
    }
    if (D_8013B0D8->unk128 & 0x403040) {
        D_8013B0D8->surfaceState.fields.unk134 = 0;
    } else if (D_8013B0D8->surfaceState.fields.unk134) {
        D_8013B0D8->surfaceState.fields.unk134--;
    }
    if (!(D_8013AFE4 & 1) && D_8013B0D8->unk945) {
        D_8013B0D8->unk945--;
    }
    if (D_8013B0D8->unk0E8) {
        return;
    }

    region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    resolveGeometryBoundaryCollision(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8, 18, 2, &dx, &dz, &responseX, &responseZ);
    if (dx || dz) {
        D_8013B0D8->x += dx << 8;
        D_8013B0D8->z += dz << 8;
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    }

    radius = (D_8013B0D8->unk128 & 0x7C000) ? 36 : 22;
    resolveGeometryBoundaryCollision(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8, radius, 1, &dx, &dz,
                  &D_8013B0D8->unk0EC, &D_8013B0D8->unk0F0);
    if (dx || dz) {
        D_8013B0D8->x += dx << 8;
        D_8013B0D8->z += dz << 8;
        D_8013B0D8->unk0E8 |= 1;
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    }
    resolveGeometryBoundaryCollision(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8, radius, 3, &dx, &dz,
                  &D_8013B0D8->unk0EC, &D_8013B0D8->unk0F0);
    if (dx || dz) {
        D_8013B0D8->x += dx << 8;
        D_8013B0D8->z += dz << 8;
        D_8013B0D8->unk0E8 |= 0x200;
    }

    adjusted = 0;
    /* Check both ends of the board, then retry from the corrected pose. */
    if (radius == 22) {
        D_8013B0D8->unk10E = D_8013B0D8->unk0B4;
        D_8013B0D8->unk110 = D_8013B0D8->unk0B6 + 0x400;
        sine = rsin((s16)D_8013B0D8->unk110);
        cosine = rcos((s16)D_8013B0D8->unk110);
        for (i = 0; i < 2; i++) {
            probes[i].vx = D_8013B0D8->x + gPlayerWallProbeOffsets[i] * sine / 4096;
            probes[i].vz = D_8013B0D8->z + gPlayerWallProbeOffsets[i] * cosine / 4096;
            region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, probes[i].vx >> 8, probes[i].vz >> 8);
            resolveGeometryBoundaryCollision(region, probes[i].vx >> 8, probes[i].vz >> 8, 14, 1, &dx, &dz,
                          &D_8013B0D8->unk0EC, &D_8013B0D8->unk0F0);
            if (dx || dz) {
                adjusted = 1;
                probes[i].vx += dx << 8;
                probes[i].vz += dz << 8;
                D_8013B0D8->unk0E8 |= 1;
                region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, probes[i].vx >> 8, probes[i].vz >> 8);
            }
            resolveGeometryBoundaryCollision(region, probes[i].vx >> 8, probes[i].vz >> 8, 14, 3, &dx, &dz,
                          &D_8013B0D8->unk0EC, &D_8013B0D8->unk0F0);
            if (dx || dz) {
                probes[i].vx += dx << 8;
                probes[i].vz += dz << 8;
                adjusted = 1;
                D_8013B0D8->unk0E8 |= 0x200;
            }
        }

        if (adjusted) {
            D_8013B0D8->unk110 = getXZHeading(probes[1].vx, probes[1].vz, probes[0].vx, probes[0].vz);
            D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
            D_8013B0D8->x = (probes[0].vx + probes[1].vx) / 2;
            D_8013B0D8->z = (probes[0].vz + probes[1].vz) / 2;
            sine = rsin((s16)D_8013B0D8->unk110);
            cosine = rcos((s16)D_8013B0D8->unk110);
            for (i = 0; i < 2; i++) {
                probes[i].vx = D_8013B0D8->x + gPlayerWallProbeOffsets[i] * sine / 4096;
                probes[i].vz = D_8013B0D8->z + gPlayerWallProbeOffsets[i] * cosine / 4096;
                region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, probes[i].vx >> 8, probes[i].vz >> 8);
                resolveGeometryBoundaryCollision(region, probes[i].vx >> 8, probes[i].vz >> 8, 14, 1, &dx, &dz,
                              &D_8013B0D8->unk0EC, &D_8013B0D8->unk0F0);
                if (dx || dz) {
                    D_8013B0D8->x += dx << 8;
                    D_8013B0D8->z += dz << 8;
                    region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, probes[i].vx >> 8, probes[i].vz >> 8);
                }
                resolveGeometryBoundaryCollision(region, probes[i].vx >> 8, probes[i].vz >> 8, 14, 3, &dx, &dz,
                              &D_8013B0D8->unk0EC, &D_8013B0D8->unk0F0);
                if (dx || dz) {
                    D_8013B0D8->x += dx << 8;
                    D_8013B0D8->z += dz << 8;
                }
            }
        }
    }

    if (D_8013B0D8->unk0E8) {
        velocityX = D_8013B0D8->unk0EC - D_8013B0D8->unk060;
        velocityZ = D_8013B0D8->unk0F0 - D_8013B0D8->unk064;
        D_8013B0E8 = SquareRoot0(velocityX * velocityX + velocityZ * velocityZ) / 2;
        D_8013B0E6 = getXZHeading(0, 0, velocityX, velocityZ);
    }
}

s16 resolveCurrentPlayerTerrainContact(void) {
    s32 groundY;
    s32 probeDistance;
    s32 dx, dz;
    s32 forwardY, rearY;
    s32 y;
    s16 region;
    s32 sinYaw, cosYaw;

    gPlayerTerrainContactSavedStackPointer = SetSp((u32)getScratchAddr((sizeof(scratchpad) - sizeof(u32)) / sizeof(u32)));
    resolveCurrentPlayerWallContact();
    if (processCurrentPlayerActionRequests(1)) {
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
        D_8013B0D8->unk92E = region;
        D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
        D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
        D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        SetSp(gPlayerTerrainContactSavedStackPointer);
        return 0;
    }

    region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    D_8013B0D8->unk92E = region;
    groundY = getCourseHeightAtPoint(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    if (groundY < D_8013B0D8->y) {
        copySelectedRecordToCurrentPlayer();
        D_8013B0D8->unk11E = D_8013B0D8->unk118 = D_8013B0D8->unk116;
        D_8013B0D8->y = groundY;
        D_8013B0D8->unk120 = D_8013B0D8->unk11C = D_8013B0D8->unk11A;
        D_8013B0D8->unk128 &= ~1;
        D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
        D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
        D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        SetSp(gPlayerTerrainContactSavedStackPointer);
        return 1;
    }

    D_8013B0D8->unk128 |= 1;
    D_8013B0D8->worldCoordinate.coord.t[0] = (D_8013B0D8->x >> 8) - D_8013B0D8->unk084;
    D_8013B0D8->worldCoordinate.coord.t[1] = (D_8013B0D8->y >> 8) - D_8013B0D8->unk086;
    D_8013B0D8->worldCoordinate.coord.t[2] = (D_8013B0D8->z >> 8) - D_8013B0D8->unk088;
    probeDistance = (rcos((s16)D_8013B0D8->unk112) * 20) / 4096;
    if (groundY < D_8013B0D8->y + (probeDistance << 8)) {
        sinYaw = rsin((s16)D_8013B0D8->unk110);
        cosYaw = rcos((s16)D_8013B0D8->unk110);
        dx = (sinYaw * probeDistance) / 4096;
        dz = (cosYaw * probeDistance) / 4096;
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) + dx, (D_8013B0D8->z >> 8) + dz);
        forwardY = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) + dx, (D_8013B0D8->z >> 8) + dz) << 8;
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) - dx, (D_8013B0D8->z >> 8) - dz);
        rearY = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) - dx, (D_8013B0D8->z >> 8) - dz) << 8;
        y = D_8013B0D8->y;
        if (forwardY < y || rearY < y) {
            if (forwardY < y && rearY < y) {
                copySelectedRecordToCurrentPlayer();
                D_8013B0D8->unk11E = D_8013B0D8->unk118 = D_8013B0D8->unk116;
                D_8013B0D8->y = y;
                D_8013B0D8->unk120 = D_8013B0D8->unk11C = D_8013B0D8->unk11A;
                D_8013B0D8->unk128 &= ~1;
                SetSp(gPlayerTerrainContactSavedStackPointer);
                return 1;
            }
            if (!(D_8013B0D8->unk128 & 0x2000000)) {
                s16 pitch;
                if (forwardY < y) {
                    pitch = getXZHeading(0, 0, probeDistance, (y - forwardY) >> 8);
                } else {
                    pitch = getXZHeading(0, 0, probeDistance, (rearY - y) >> 8);
                }
                D_8013B0D8->unk112 = D_8013B0D8->unk0B8 = pitch - 0x400;
            }
        }
    }

    SetSp(gPlayerTerrainContactSavedStackPointer);
    return 0;
}

/* Average angles on the 4096-unit circle, taking the shorter arc. */
s32 averageWrappedAngle(s16 first, u16 second) {
    s16 difference = first - second;
    s16 midpoint;
    if (difference > -0x800 && difference < 0x800) {
        return first - difference / 2;
    }
    midpoint = (s16)(first + second) / 2 - 0x800;
    if (midpoint & 0x800) {
        return midpoint | 0xF000;
    }
    return midpoint & 0x7FF;
}

s32 updateCurrentPlayerGroundContact(void) {
    u8 padding[0x28];
    s16 result = 0;
    s16 region;
    s32 groundY;

    gPlayerGroundContactSavedStackPointer = SetSp((u32)getScratchAddr((sizeof(scratchpad) - sizeof(u32)) / sizeof(u32)));
    resolveCurrentPlayerWallContact();
    if (processCurrentPlayerActionRequests(0) >= 2) {
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
        D_8013B0D8->unk92E = region;
        D_8013B0D8->y = getCourseHeightAtPoint(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
        D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
        D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
        D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        SetSp(gPlayerGroundContactSavedStackPointer);
        return 0;
    }

    region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    D_8013B0D8->unk92E = region;
    groundY = getCourseHeightAtPoint(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    copySelectedRecordToCurrentPlayer();

    if (groundY > D_8013B0D8->y + 0x400) {
        s16 yawDifference = D_8013B0D8->unk116 - D_8013B0D8->unk118;
        if ((yawDifference > 0x400 && yawDifference < 0xC00) ||
            (yawDifference < -0x400 && yawDifference > -0xC00)) {
            s16 previousPitch;
            if (D_8013B0D8->unk11C > 0) {
                previousPitch = D_8013B0D8->unk11C - 0x800;
            } else {
                previousPitch = D_8013B0D8->unk11C + 0x800;
            }
            if (D_8013B0D8->unk11A - previousPitch > 0xA0 &&
                D_8013B0D8->unk0BC / 16 < groundY - D_8013B0D8->y) {
                result = 2;
            }
        } else if (D_8013B0D8->unk062 > 0 &&
                   D_8013B0D8->unk11A - D_8013B0D8->unk11C > 0xA0 &&
                   D_8013B0D8->unk0BC / 16 < groundY - D_8013B0D8->y) {
            result = 1;
        }
    }

    if (result == 0) {
        s32 sinYaw, cosYaw, probeDistance, dx, dz;
        s32 forwardY, rearY;
        s16 forwardRecord;

        sinYaw = rsin((s16)D_8013B0D8->unk110);
        cosYaw = rcos((s16)D_8013B0D8->unk110);
        probeDistance = rcos((s16)D_8013B0D8->unk112) * 20 / 4096;
        dx = sinYaw * probeDistance / 4096;
        dz = cosYaw * probeDistance / 4096;

        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) + dx, (D_8013B0D8->z >> 8) + dz);
        forwardY = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) + dx, (D_8013B0D8->z >> 8) + dz) << 8;
        forwardRecord = D_8013B1FE;
        computeSelectedTriangleNormal(D_8013AF88);

        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) - dx, (D_8013B0D8->z >> 8) - dz);
        rearY = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) - dx, (D_8013B0D8->z >> 8) - dz) << 8;
        computeSelectedTriangleNormal(D_8013AF8C);

        if (forwardRecord == D_8013B1FE) {
            s32 y;
            D_8013B0D8->y = groundY;
            D_8013B0D8->unk118 = D_8013B0D8->unk11E;
            D_8013B0D8->unk11C = D_8013B0D8->unk120;
            D_8013B0D8->unk11E = D_8013B0D8->unk116;
            D_8013B0D8->unk120 = D_8013B0D8->unk11A;
            D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
            y = D_8013B0D8->y;
            D_8013B0D8->worldCoordinate.coord.t[1] = y >> 8;
            D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        } else {
            D_8013B0D8->unk118 = D_8013B0D8->unk11E;
            D_8013B0D8->unk11C = D_8013B0D8->unk120;
            D_8013AF88->vx = (D_8013AF88->vx + D_8013AF8C->vx) / 2;
            D_8013AF88->vy = (D_8013AF88->vy + D_8013AF8C->vy) / 2;
            D_8013AF88->vz = (D_8013AF88->vz + D_8013AF8C->vz) / 2;
            computeVectorYawAndPitch(D_8013AF88, &D_8013B0D8->unk116, &D_8013B0D8->unk11A);
            D_8013B0D8->unk11E = averageWrappedAngle(D_8013B0D8->unk116, D_8013B0D8->unk11E);
            D_8013B0D8->y = groundY;
            D_8013B0D8->worldCoordinate.coord.t[1] = (((forwardY + rearY) >> 1) + groundY) >> 9;
            D_8013B0D8->unk120 = (D_8013B0D8->unk11A + D_8013B0D8->unk11C) / 2;
            D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
            D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        }

        if (!(D_8013B0D8->unk128 & 0x2000000)) {
            D_8013B0D8->unk10E = D_8013B0D8->unk0B4;
            D_8013B0D8->unk110 = D_8013B0D8->unk0B6 + 0x400;
            D_8013B0D8->unk112 = D_8013B0D8->unk0B8 = getXZHeading(0, 0, probeDistance * 2, (rearY - forwardY) >> 8) - 0x400;
        }
        D_8013B0D8->unk128 &= ~1;
    } else {
        if (result == 2) {
            D_8013B0D8->unk062 = 0;
        }
        D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
        D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
        D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        D_8013B0D8->unk128 |= 1;
    }

    SetSp(gPlayerGroundContactSavedStackPointer);
    return result;
}

s32 snapCurrentPlayerToTerrain(void) {
    u8 padding[0x20];
    s32 heights[4];
    s16 records[4];
    s32 firstSin, firstCos, secondSin, secondCos;
    s32 firstX, firstZ, secondX, secondZ;
    s16 region;
    s16 firstAngle, secondAngle;

    resolveCurrentPlayerWallContact();
    if (processCurrentPlayerActionRequests(0)) {
        region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
        D_8013B0D8->unk92E = region;
        D_8013B0D8->y = getCourseHeightAtPoint(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
        D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
        D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
        D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
        return 0;
    }

    region = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    D_8013B0D8->unk92E = region;
    D_8013B0D8->y = getCourseHeightAtPoint(region, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    copySelectedRecordToCurrentPlayer();

    /* Probe both ends of two axes angled 45 degrees from the heading. */
    firstSin = rsin((s16)D_8013B0D8->unk110 + 0x200);
    firstCos = rcos((s16)D_8013B0D8->unk110 + 0x200);
    secondSin = rsin((s16)D_8013B0D8->unk110 - 0x200);
    secondCos = rcos((s16)D_8013B0D8->unk110 - 0x200);
    firstX = firstSin * 20 / 4096;
    firstZ = firstCos * 20 / 4096;
    secondX = secondSin * 20 / 4096;
    secondZ = secondCos * 20 / 4096;

    region = getAdjacentGeometryRegionForPoint(
        D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) + firstX, (D_8013B0D8->z >> 8) + firstZ);
    heights[0] = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) + firstX, (D_8013B0D8->z >> 8) + firstZ) << 8;
    records[0] = D_8013B1FE;

    region = getAdjacentGeometryRegionForPoint(
        D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) - firstX, (D_8013B0D8->z >> 8) - firstZ);
    heights[1] = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) - firstX, (D_8013B0D8->z >> 8) - firstZ) << 8;
    records[1] = D_8013B1FE;

    region = getAdjacentGeometryRegionForPoint(
        D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) + secondX, (D_8013B0D8->z >> 8) + secondZ);
    heights[2] = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) + secondX, (D_8013B0D8->z >> 8) + secondZ) << 8;
    records[2] = D_8013B1FE;

    region = getAdjacentGeometryRegionForPoint(
        D_8013B0D8->unk92E, (D_8013B0D8->x >> 8) - secondX, (D_8013B0D8->z >> 8) - secondZ);
    heights[3] = getCourseHeightAtPoint(region, (D_8013B0D8->x >> 8) - secondX, (D_8013B0D8->z >> 8) - secondZ) << 8;
    records[3] = D_8013B1FE;

    D_8013B0D8->unk118 = D_8013B0D8->unk11E;
    D_8013B0D8->unk11C = D_8013B0D8->unk120;
    firstAngle = averageWrappedAngle(D_8013B1F0[records[0]].unk8, D_8013B1F0[records[1]].unk8);
    secondAngle = averageWrappedAngle(D_8013B1F0[records[2]].unk8, D_8013B1F0[records[3]].unk8);
    D_8013B0D8->unk11E = averageWrappedAngle(firstAngle, secondAngle);
    /* The original average counts the third record twice. */
    D_8013B0D8->unk120 = (D_8013B1F0[records[0]].unkA + D_8013B1F0[records[1]].unkA +
                         D_8013B1F0[records[2]].unkA + D_8013B1F0[records[2]].unkA) / 4;
    D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
    D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
    D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
    if (!(D_8013B0D8->unk128 & 0x2000000)) {
        D_8013B0D8->unk0B8 = 0;
        D_8013B0D8->unk112 = 0;
        D_8013B0D8->unk10E = D_8013B0D8->unk0B4;
        D_8013B0D8->unk110 = D_8013B0D8->unk0B6 + 0x400;
    }
    D_8013B0D8->unk128 &= ~1;
    /* The main path falls through. The caller discards the return value. */
}

void initializeActiveRacePlayers(void) {
    u16 i;
    for (i = 0; i < D_8013AFD2; i++) {
        RacePlayer *player = &gRacePlayers[i];
        player->playerIndex = i;
        if (player->isActive) {
            currentPlayerAbsolute = player;
            initializeCurrentRacePlayer();
        }
    }
}

void initializeCurrentPlayerAttributes(void) {
    u16 (*variant)[9] = gPlayerVariantAttributes.values;
    u16 (*character)[7] = gPlayerCharacterAttributes;
    RacePlayer *player = currentPlayerAbsolute;
    player->unk130 = player->unk098 = variant[player->unk015[1]][0] + character[player->characterId][0];
    player->unk09A = variant[player->unk015[1]][1] + character[player->characterId][1];
    player->unk09C = variant[player->unk015[1]][2] + character[player->characterId][2];
    player->unk0A0 = variant[player->unk015[1]][3] + character[player->characterId][3];
    player->unk0A6 = variant[player->unk015[1]][4] + character[player->characterId][4];
    player->unk0A2 = variant[player->unk015[1]][5] + character[player->characterId][5];
    player->unk0A4 = variant[player->unk015[1]][6] + character[player->characterId][6];
    player->unk0A8 = variant[player->unk015[1]][7];
    player->unk0AA = variant[player->unk015[1]][8];
}

void initializeCurrentRacePlayer(void) {
    u32 i = 0;
    RacePlayer *player = currentPlayerAbsolute;
    u8 *state = (u8 *)&player->x;
    u32 size = (u8 *)(player + 1) - state;

    /* The fields before x contain the player setup supplied by the caller. */
    while (i < size) {
        state[i++] = 0;
    }

    currentPlayerAbsolute->unk96C = 0;
    currentPlayerAbsolute->unk972 = 0;
    currentPlayerAbsolute->soundVoice = 0xFF;
    initCurrentPlayerModelParts();

    switch (currentPlayerAbsolute->playerIndex) {
        case 0: currentPlayerAbsolute->x = -0x3C00; break;
        case 1: currentPlayerAbsolute->x = 0x3C00; break;
        case 2: currentPlayerAbsolute->x = -0xB400; break;
        case 3: currentPlayerAbsolute->x = 0xB400; break;
    }

    currentPlayerAbsolute->z = gCoursePlayerStartZ[D_8013AFCE];
    /* Copy the four-word position block, including its fourth word. */
    __builtin_memcpy(&currentPlayerAbsolute->unk050, &currentPlayerAbsolute->x, sizeof(VECTOR));
    currentPlayerAbsolute->unk078 = 0x400;
    currentPlayerAbsolute->unk0AC = 0x2000;
    currentPlayerAbsolute->unk0B0 = 0x3800;
    initializeCurrentPlayerAttributes();

    if (currentPlayerAbsolute->unk008) {
        if (currentPlayerAbsolute->characterId >= 8) {
            currentPlayerAbsolute->unk0A6 = 0x6000;
        } else {
            currentPlayerAbsolute->unk0A6 = 0x800;
        }
    }
    if (D_8013AFE2 == 2) {
        currentPlayerAbsolute->heldItem = 1;
    }
    if (!currentPlayerAbsolute->unk008) {
        currentPlayerAbsolute->unk945 = 8;
    } else if (D_8013AFD9) {
        currentPlayerAbsolute->unk945 = 8;
    }
    if (D_8013AFE2 == 1 && D_8013AFE0 == 1) {
        if (D_8013AFCD == 1) {
            currentPlayerAbsolute->unk93C = 10;
        } else {
            currentPlayerAbsolute->unk93C = 5;
        }
    }
    if (D_8013AFCE == 7 && D_8013AFCD == 1) {
        currentPlayerAbsolute->x = 0;
    }
    if (!currentPlayerAbsolute->unk008 && !D_8013AFD6 &&
        (s16)currentPlayerAbsolute->unk0A8 != (s16)currentPlayerAbsolute->unk0AA) {
        createChildTaskForParentWithReserve(&gGameTasks[1], initializePlayerPromptTask);
        D_8013AE8C->data18 = currentPlayerAbsolute->playerIndex;
    }

    switch (D_8013AFD8) {
        case 1:
            *(s16 *)&currentPlayerAbsolute->unk110 = -0x57E;
            currentPlayerAbsolute->x = 0xFF6E1C30;
            currentPlayerAbsolute->z = 0x634F64;
            currentPlayerAbsolute->unk92E = 0x34;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            *(s16 *)&currentPlayerAbsolute->unk110 = -0xFD5;
            currentPlayerAbsolute->x = 0xFEB8AC08;
            currentPlayerAbsolute->z = 0xA4ECB8;
            currentPlayerAbsolute->unk92E = 0x96;
            break;
        case 6:
            *(s16 *)&currentPlayerAbsolute->unk110 = -0x993;
            currentPlayerAbsolute->x = 0xFF060F18;
            currentPlayerAbsolute->z = 0xC6B91C;
            currentPlayerAbsolute->unk92E = 0xAE;
            break;
        case 7:
            switch (currentPlayerAbsolute->playerIndex) {
                case 0:
                    *(s16 *)&currentPlayerAbsolute->unk110 = 0x10C0;
                    currentPlayerAbsolute->x = 0xD6AA;
                    currentPlayerAbsolute->z = 0xC4AA5;
                    currentPlayerAbsolute->unk92E = 3;
                    break;
                case 2:
                    *(s16 *)&currentPlayerAbsolute->unk110 = -0x102;
                    currentPlayerAbsolute->x = 0x4BEA0;
                    currentPlayerAbsolute->z = 0x19EBC0;
                    currentPlayerAbsolute->unk92E = 8;
                    break;
                case 3:
                    *(s16 *)&currentPlayerAbsolute->unk110 = -0x8F;
                    currentPlayerAbsolute->x = 0x30B18;
                    currentPlayerAbsolute->z = 0x17AA90;
                    currentPlayerAbsolute->unk92E = 7;
                    break;
                case 1:
                    *(s16 *)&currentPlayerAbsolute->unk110 = -0x8F;
                    currentPlayerAbsolute->x = 0x5F0BC;
                    currentPlayerAbsolute->z = 0x179368;
                    currentPlayerAbsolute->unk92E = 7;
                    break;
            }
            break;
        case 8:
            switch (currentPlayerAbsolute->playerIndex) {
                case 0:
                    *(s16 *)&currentPlayerAbsolute->unk110 = 0xE23;
                    currentPlayerAbsolute->x = 0x7CD;
                    currentPlayerAbsolute->z = 0x208502;
                    currentPlayerAbsolute->unk92E = 12;
                    break;
                case 2:
                    *(s16 *)&currentPlayerAbsolute->unk110 = 0xDA8;
                    currentPlayerAbsolute->x = 0xFFF9A98C;
                    currentPlayerAbsolute->z = 0x26DEA6;
                    currentPlayerAbsolute->unk92E = 14;
                    break;
                case 3:
                    *(s16 *)&currentPlayerAbsolute->unk110 = 0xE75;
                    currentPlayerAbsolute->x = 0xFFFA9EC0;
                    currentPlayerAbsolute->z = 0x28D821;
                    currentPlayerAbsolute->unk92E = 14;
                    break;
                case 1:
                    *(s16 *)&currentPlayerAbsolute->unk110 = 0xE43;
                    currentPlayerAbsolute->x = 0xFFFAB685;
                    currentPlayerAbsolute->z = 0x281B92;
                    currentPlayerAbsolute->unk92E = 14;
                    break;
            }
            break;
        case 9:
            *(s16 *)&currentPlayerAbsolute->unk110 = -0x235;
            currentPlayerAbsolute->x = 0xFFFABFE0;
            currentPlayerAbsolute->z = 0x274DD8;
            currentPlayerAbsolute->unk92E = 14;
            break;
    }

    switch (D_8013AFD9) {
        case 1:
            currentPlayerAbsolute->unk93E = 1;
            currentPlayerAbsolute->unk940 = 3;
            break;
        case 2:
            currentPlayerAbsolute->unk93E = 2;
            currentPlayerAbsolute->unk940 = 3;
            break;
        case 3:
            currentPlayerAbsolute->unk93E = 3;
            currentPlayerAbsolute->unk940 = 3;
            break;
        case 4:
            currentPlayerAbsolute->unk93E = 4;
            currentPlayerAbsolute->unk940 = 3;
            break;
        case 5:
            currentPlayerAbsolute->unk93E = 5;
            currentPlayerAbsolute->unk940 = 3;
            break;
        case 6:
            currentPlayerAbsolute->heldItem = 1;
            break;
        case 7:
            currentPlayerAbsolute->heldItem = 2;
            break;
        case 8:
            currentPlayerAbsolute->heldItem = 3;
            break;
        case 9:
            currentPlayerAbsolute->heldItem = 4;
            break;
        case 10:
            currentPlayerAbsolute->heldItem = 5;
            if (currentPlayerAbsolute->playerIndex != 0) {
                currentPlayerAbsolute->unk01C = 5000;
            }
            break;
        case 11:
            currentPlayerAbsolute->heldItem = 6;
            if (currentPlayerAbsolute->playerIndex == 1) {
                currentPlayerAbsolute->heldItem = 3;
            }
            currentPlayerAbsolute->unk93E = 1;
            currentPlayerAbsolute->unk940 = 3;
            break;
    }

    if (D_8013AFD9) {
        if (currentPlayerAbsolute->playerIndex >= 2) {
            currentPlayerAbsolute->heldItem = 0;
            currentPlayerAbsolute->unk93E = 0;
            currentPlayerAbsolute->unk940 = 0;
        }
        currentPlayerAbsolute->x = (gPlaybackPlayerStartPlacements[(D_8013AFD9 - 1) * 4 +
                                                  currentPlayerAbsolute->playerIndex].position.vx / 256) * 4;
        currentPlayerAbsolute->z = (-gPlaybackPlayerStartPlacements[(D_8013AFD9 - 1) * 4 +
                                                  currentPlayerAbsolute->playerIndex].position.vz / 256) * 4;
        currentPlayerAbsolute->unk110 = -gPlaybackPlayerStartPlacements[(D_8013AFD9 - 1) * 4 +
                                                  currentPlayerAbsolute->playerIndex].heading;
        currentPlayerAbsolute->unk92E = gPlaybackPlayerStartPlacements[(D_8013AFD9 - 1) * 4 +
                                                  currentPlayerAbsolute->playerIndex].recordIndex;
    }

    currentPlayerAbsolute->y += 0x40000;
    __builtin_memcpy(&currentPlayerAbsolute->unk050, &currentPlayerAbsolute->x, sizeof(VECTOR));
    currentPlayerAbsolute->randomIndex = randomNextMain();
}

s16 isPlayerNearSelectedRecord(s16 index) {
    s16 i;
    s32 distance;
    if (gRacePlayers[index].unk92E != (s16)D_80123D80[D_8013AFCE].unk00) return 0;
    if (gRacePlayers[index].unk128 & 0x40) return 0;
    for (i = 0; i < 4; i++) {
        if (gRacePlayers[i].isActive && gRacePlayers[i].playerIndex != index &&
            (gRacePlayers[i].unk128 & 0x20)) return 0;
    }
    distance = gRacePlayers[index].x - D_80123D80[D_8013AFCE].unk18[0];
    if (distance < 0) distance = -distance;
    if (distance > 0x6000) return 0;
    distance = gRacePlayers[index].z - D_80123D80[D_8013AFCE].unk18[2];
    if (distance < 0) distance = -distance;
    return distance <= 0x6000;
}

s16 tryStartPlayerSelectedRecordTransition(s16 playerIndex) {
    RacePlayer *player;
    if (isPlayerNearSelectedRecord(playerIndex)) {
        player = gRacePlayers;
        player[playerIndex].unk128 |= 0x04001020;
        player[playerIndex].unk966 = D_8013AFDE;
        if (D_8013AFDE != 0) {
            D_8013AFDE--;
        }
        return 1;
    }
    return 0;
}

s16 markPlayerNearSelectedRecord(s16 playerIndex) {
    if (isPlayerNearSelectedRecord(playerIndex)) {
        gRacePlayers[playerIndex].unk128 |= 0x1000;
    } else {
        return 0;
    }
    return 1;
}

void assignCurrentPlayerFinishSlot(s32 value) {
    RacePlayer *player = currentPlayerAbsolute;
    u32 flags = player->unk128;
    if (!(flags & 0x40) && player->unk934 >= selectedFinishLapCountAbsolute - 1 &&
        player->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 && !(flags & 0x1000)) {
        player->unk128 = flags | 0x40;
        if (!D_8013AFD6) {
            if (!player->unk008) {
                createChildTaskWithReserve(beginSound601TaskSequence);
                D_8013AE8C->data18 = value;
            }
            currentPlayerAbsolute->unk100[0] = D_8013AFDB;
            D_8013AFDB++;
        }
    }
}

void awardCurrentPlayerTrickRewards(void) {
    s32 reward = 0;
    s32 points = 0;
    s32 sound;
    s16 trick;
    RacePlayer *player;

    if (currentPlayerAbsolute->unk0C8 >= 9 ||
        (currentPlayerAbsolute->unk0C8 != 0 && currentPlayerAbsolute->unk0C4 != 0)) {
        currentPlayerAbsolute->unk965 = 1;
    }

    if (currentPlayerAbsolute->unk0C8 != 0) {
        reward += gPlayerTrickRewards[currentPlayerAbsolute->unk0C8];
        if (currentPlayerAbsolute->unk0C8 < 9) {
            points += 10;
        } else {
            points += 40;
        }
        trick = currentPlayerAbsolute->unk0C8 < 10 ? currentPlayerAbsolute->unk0C8 : 9;
        currentPlayerAbsolute->unk0E6 |= 1 << trick;
    }

    if (currentPlayerAbsolute->unk0C4 != 0) {
        if (currentPlayerAbsolute->unk0C8 != 0) {
            reward += gPlayerCombinedTrickBonuses[currentPlayerAbsolute->unk0C4];
            if (currentPlayerAbsolute->unk0C4 & 1) {
                points += 5;
            } else {
                points += 10;
            }
        } else {
            reward += gPlayerDirectionalTrickRewards[currentPlayerAbsolute->unk0C4];
            points++;
        }
        reward += currentPlayerAbsolute->unk0C6;
    }

    if (reward != 0 && D_8013AFE2 == 0) {
        addPlayerCappedCounters(D_8013AE88->data18, reward);
        if (currentPlayerAbsolute->unk008 == 0 && (D_8013AFD9 == 0 || D_8013AE88->data18 == 0)) {
            createActivePlayerSlidingNumberTask(reward, currentPlayerAbsolute->playerIndex);
            sound = 0x201;
            if (reward >= 100) {
                sound = 0x301;
            }
            if (reward >= 120) {
                sound = 0x401;
            }
            if (reward >= 200) {
                sound = 0x501;
            }
            playSoundEffect(0, sound, 0x40);
        }
    }

    if (D_8013AFE2 == 1 && D_8013AFE0 == 2 && points != 0) {
        if (points > 9999) {
            points = 9999;
        }
        player = currentPlayerAbsolute;
        player->trickAttackMakeCount++;
        player->trickAttackPointTotal += points;
        if (currentPlayerAbsolute->trickAttackMakeCount > 99) {
            currentPlayerAbsolute->trickAttackMakeCount = 99;
        }
        if (currentPlayerAbsolute->unk0E4 < currentPlayerAbsolute->trickAttackMakeCount) {
            currentPlayerAbsolute->unk0E4 = currentPlayerAbsolute->trickAttackMakeCount;
        }
        sound = 0x201;
        if (points >= 10) {
            sound = 0x301;
        }
        if (points >= 15) {
            sound = 0x401;
        }
        if (points >= 20) {
            sound = 0x501;
        }

        if (currentPlayerAbsolute->unk0E6 == 0x3FE) {
            currentPlayerAbsolute->unk0E6 = 0x3FF;
            currentPlayerAbsolute->trickAttackPointTotal += 300;
            createPlayerNumberDisplayTask(points, 1, (s16)D_8013AE88->data18);
            sound = 0x501;
        } else {
            createPlayerNumberDisplayTask(points, 0, (s16)D_8013AE88->data18);
        }
        playSoundEffect(0, sound, 0x40);
        if (currentPlayerAbsolute->trickAttackPointTotal > 9999) {
            currentPlayerAbsolute->trickAttackPointTotal = 9999;
        }
    }
}

void computeSelectedTriangleNormal(VECTOR *normal) {
    VECTOR first, second;

    first.vx = D_8013B204[0] - D_8013B200[0];
    first.vy = D_8013B204[1] - D_8013B200[1];
    first.vz = D_8013B204[2] - D_8013B200[2];
    second.vx = D_8013B208[0] - D_8013B204[0];
    second.vy = D_8013B208[1] - D_8013B204[1];
    second.vz = D_8013B208[2] - D_8013B204[2];
    OuterProduct0(&first, &second, normal);
}

void computeVectorYawAndPitch(VECTOR *vector, s16 *yaw, s16 *pitch) {
    s32 x, y, z;
    s16 angle;
    x = vector->vx;
    y = vector->vy;
    z = vector->vz;
    angle = getXZHeading(0, 0, x, z);
    if (angle >= 0x800) angle -= 0x1000;
    *yaw = angle;
    x /= 16;
    y /= 16;
    z /= 16;
    angle = getXZHeading(0, 0, SquareRoot0(x*x + z*z), y);
    if (angle >= 0x800) angle -= 0x1000;
    *pitch = angle;
}
