#include "game/engine/game_options.h"
#include "player_actions_internal.h"
#include "game/engine/random.h"
#include "game/graphics/quad_animation.h"
#include "../race_internal.h"
#include "game/audio/music.h"
#include "game/engine/angles.h"
#include "game/engine/task.h"
#include "game/engine/system_runtime.h"
#include "game/menu/save_record_tasks.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/io/controller.h"
#include "game/io/dat_resources.h"
#include "game/menu/selected_records.h"
#include "game/menu/menu_selection_state.h"
#include "game/race/course/course_geometry.h"
#include "game/race/player/player_resource_textures.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_time.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern VECTOR *gScratchVector0 __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));

extern u16 D_8013AF7A __attribute__((section(".data")));
extern u16 D_8013AF7C __attribute__((section(".data")));
extern u8 D_8013AFD6 __attribute__((section(".data")));
extern u8 gDemoRaceIndex __attribute__((section(".data")));
extern volatile u8 gShopMenuRememberedSelection __attribute__((section(".data")));

extern u8 D_8013B47C __attribute__((section(".data")));
extern u8 D_8013B527 __attribute__((section(".data")));
extern u8 D_8013B110;

RaceTime gDefaultRaceRecordTimes[3][5]
    __attribute__((section(".data.default_race_record_times"), aligned(4))) = {
    {{0, 36, 0}, {0, 42, 0}, {0, 44, 0}, {0, 48, 0}, {0, 53, 0}},
    {{1, 58, 0}, {2, 3, 0}, {2, 11, 0}, {2, 13, 0}, {2, 21, 0}},
    {{2, 1, 0}, {2, 6, 0}, {2, 13, 0}, {2, 18, 0}, {2, 22, 0}},
};

RaceTime gDefaultLapRecordTimes[10][5]
    __attribute__((section(".data.default_lap_record_times"), aligned(4))) = {
    {{2, 15, 0}, {2, 20, 0}, {2, 30, 0}, {2, 40, 0}, {2, 50, 0}},
    {{2, 12, 0}, {2, 15, 0}, {2, 20, 0}, {2, 30, 0}, {2, 38, 0}},
    {{1, 47, 0}, {1, 50, 0}, {1, 56, 0}, {2, 0, 0}, {2, 8, 0}},
    {{1, 56, 0}, {2, 0, 0}, {2, 12, 0}, {2, 21, 0}, {2, 25, 0}},
    {{1, 52, 0}, {2, 0, 0}, {2, 5, 0}, {2, 9, 0}, {2, 13, 0}},
    {{1, 53, 0}, {1, 56, 0}, {2, 0, 0}, {2, 4, 0}, {2, 10, 0}},
    {{1, 59, 0}, {2, 2, 0}, {2, 11, 0}, {2, 17, 0}, {2, 20, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 28, 0}, {0, 33, 0}, {0, 36, 0}, {0, 40, 0}, {0, 42, 0}},
    {{0, 36, 0}, {0, 40, 0}, {0, 45, 0}, {0, 50, 0}, {0, 55, 0}},
};

u16 gPlayerSoundVariantsA[2] __attribute__((section(".data.player_sound_variants"))) = {0x100, 0x200};

u16 gPlayerSoundVariantsB[2] __attribute__((section(".data.player_sound_variants"))) = {0x300, 0x400};

u8 gRaceOrderPlayerIds[4] __attribute__((section(".sdata.race_order_player_ids"))) = {0};

const char gDemonstrationText[16]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4))) = "DEMONSTRATION";

const char gDemonstrationStartText[12]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4))) = "PUSH START";

TaskVisualFrame gPlayerFlagVisualFrames[6]
    __attribute__((section(".data.player_flag_visual_frames"), aligned(4))) = {
        {0x30, 0x48, 24, 24, 0xD, 0x3CB9},
        {0x48, 0x48, 24, 24, 0xD, 0x3CB9},
        {0x60, 0x48, 24, 24, 0xD, 0x3CB9},
        {0x78, 0x48, 24, 24, 0xD, 0x3CB9},
        {0x90, 0x48, 24, 24, 0xD, 0x3CB9},
        {0xA8, 0x48, 24, 24, 0xD, 0x3CB9}
    };

TaskVisualFrame gPlayerSideVisualFrames[4]
    __attribute__((section(".data.player_side_visual_frames"), aligned(4))) = {
        {0x50, 0x28, 32, 32, 0xD, 0x3D39},
        {0x70, 0x28, 32, 32, 0xD, 0x3D39},
        {0x90, 0x28, 32, 32, 0xD, 0x3D39},
        {0xB0, 0x28, 32, 32, 0xD, 0x3D39}
    };

GsRVIEW2 gPlayerCameraViews[2]
    __attribute__((section(".data.player_camera_views"))) = {0};

RaceTime D_8013B154
    __attribute__((section(".data.menu_sprite_palettes_prefix"), aligned(4))) = {3, 0, 0};

MenuSpritePaletteGroups gMenuSpritePaletteGroups
    __attribute__((section(".data.menu_sprite_palettes"), aligned(4))) = {
        {2, 2, 3},
        {3, 5, 6, 5, 5, 4, 0, 0},
        {0, 5, 0, 1, 4, 0, 5, 0, 0}
    };

RaceTime D_80124248[10]
    __attribute__((section(".data.course_hit_time_limits"), aligned(4))) = {
        {3, 0, 0}, {3, 0, 0}, {3, 0, 0}, {3, 0, 0}, {3, 0, 0},
        {3, 0, 0}, {3, 0, 0}, {3, 0, 0}, {1, 0, 0}, {1, 30, 0}
    };

u8 D_8012429C[12]
    __attribute__((section(".data.course_count_targets"), aligned(4))) = {
        20, 20, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0
    };

u8 D_801242A8[12]
    __attribute__((section(".data.course_hit_targets"), aligned(4))) = {
        60, 60, 0, 0, 0, 0, 0, 0, 0, 30, 0, 0
    };

u16 gControllerInputSequences[3][26]
    __attribute__((section(".data.controller_input_sequences"))) = {
    {0x0004, 0x0008, 0x0002, 0x0001, 0x0004, 0x0008, 0x0002, 0x0001, 0x0100, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x1000, 0x4000, 0x8000, 0x2000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0100, 0xFFFF},
    {0x8000, 0x1000, 0x2000, 0x4000, 0x1000, 0x1000, 0x1000, 0x4000, 0x4000, 0x4000, 0x0100, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
};

u8 gResultLabelPalettes[12]
    __attribute__((section(".data.result_label_tables"))) = {
        0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0
    };

u8 gResultLabelXOffsets[12]
    __attribute__((section(".data.result_label_tables"))) = {
        0, 0, 0, 0, 8, 0, 16, 0, 8, 0, 0, 0
    };

u8 gResultIconPalettes[4]
    __attribute__((section(".data.result_icon_palettes"), aligned(4))) = {2, 2, 3, 3};

const u16 gPlayerModeMessageText[] __attribute__((section(".data.player_mode_messages"))) = {
    0xC00F, 0x0064, 0x0065, 0x006A, 0x002C, 0x0048, 0x002C, 0x007F, 0x00BF, 0x0053, 0x003D, 0x003F, 0x004A, 0x009E, 0x00B1, 0x009F, 0x0033, 0x0000
};

const u16 gPlayerModeAlternateMessageText[] __attribute__((section(".data.player_mode_messages"))) = {
    0xC00F, 0x0049, 0x0040, 0x0069, 0x004A, 0x0052, 0x0158, 0x00BF, 0x0096, 0x00B9, 0x00C0, 0x00CE, 0x009A, 0x00CB, 0x009D, 0x0094, 0x0033, 0x0000
};

const u16 *gPlayerModeMessageTexts[] __attribute__((section(".data.player_mode_message_texts"))) = {
    gPlayerModeMessageText, gPlayerModeAlternateMessageText
};

TaskVisualFrame gFallingPairedVisualFrames[7]
    __attribute__((section(".data.falling_paired_visual_frames"), aligned(4))) = {
    {0, 0, 48, 40, 13, 0x3C79},
    {0, 40, 48, 40, 13, 0x3C39},
    {48, 0, 48, 40, 13, 0x3C39},
    {96, 0, 48, 40, 13, 0x3C39},
    {144, 0, 48, 40, 13, 0x3C39},
    {192, 0, 48, 40, 13, 0x3C39},
    {0, 80, 48, 40, 13, 0x3C39}
};

u8 gFallingPairedVisualAnimation[6]
    __attribute__((section(".data.falling_paired_visual_animation"), aligned(2))) = {
    1, 2, 3, 4, 5, 6
};

TaskVisualFrame gPlayerDualResourceEffectFrame
    __attribute__((section(".data.player_dual_resource_effect_frame"), aligned(4))) = {
        48, 96, 24, 24, 13, 0x3DF9
    };

TaskVisualFrame gPlayerVisualPairFrame
    __attribute__((section(".data.player_visual_pair_frame"), aligned(4))) = {
        72, 96, 24, 24, 13, 0x3E39
    };

TaskVisualFrame gPlayerVisualPair6780Frame
    __attribute__((section(".data.player_visual_pair_6780_frame"), aligned(4))) = {
        120, 96, 24, 24, 13, 0x3EB9
    };

TaskVisualFrame gPlayerVisualPair5E80Frame
    __attribute__((section(".data.player_visual_pair_5e80_frame"), aligned(4))) = {
        96, 96, 24, 24, 13, 0x3E79
    };

TaskVisualFrame gPlayerDualResource5100EffectFrame
    __attribute__((section(".data.player_dual_resource_5100_effect_frame"), aligned(4))) = {
        168, 96, 24, 24, 13, 0x3F39
    };

s16 D_8013B1D4 __attribute__((section(".data.finish_slot_state"))) = 0;

const char gAutoPilotLabel[] __attribute__((section(".rodata.auto_pilot_label"), aligned(4))) = "AUTO PILOT";

const char gCompleteBonusLabel[20] __attribute__((section(".rodata.complete_bonus_label"), aligned(4))) = "-COMPLETE BONUS-";

const char gTotalMoneyLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4))) = "  -TOTAL MONEY-";

const char gTrickPrizeLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4))) = "  -TRICK PRIZE-";

const char gMakeBonusLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4))) = "  -MAKE BONUS-";

const char gResultRankPrizeLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4))) = "-RANK PRIZE-";

const char gResultGetCoinLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4))) = "  -GET COIN-";

const char gResultTotalMoneyLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4))) = "-TOTAL MONEY-";

const char gResultValueFormat[4] __attribute__((section(".data.result_row_labels"), aligned(4))) = "%d";

const char gResultValueMultiplierLabel[4] __attribute__((section(".data.result_row_labels"), aligned(4))) = "X";

const char gPlayerNumberSuffixLabel[4] __attribute__((section(".data.result_row_labels"), aligned(4))) = "P";

const char gComputerPlayerLabel[4] __attribute__((section(".data.result_row_labels"), aligned(4))) = "COM";

const u32 gPlayerCourseProgressTablePadding
    __attribute__((section(".rodata.player_course_progress_padding"))) = 0;

TaskCallback gCurrentPlayerDirectionalAnimationCallbacks[4]
    __attribute__((section(".data.player_directional_animation_callbacks"), aligned(4))) = {
        startCurrentPlayerDirectionalAnimation,
        updateCurrentPlayerDirectionalAnimation,
        waitForCurrentPlayerDirectionalAnimationEnd,
        func_8002E3C0
    };

u8 gPlayerProjectileBounceChoices[4][8]
    __attribute__((section(".data.player_projectile_bounce_choices"), aligned(4))) = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {1, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

const u32 gPlayerProjectileUseTablePadding
    __attribute__((section(".rodata.player_projectile_use_padding"))) = 0;

RaceTime gRaceElapsedTime = {0, 0, 0};

s32 requestPlayerAction8(void);
s32 requestPlayerAction12(void);
s32 requestPlayerAction11(void);
s32 requestPlayerAction13(void);
s32 requestPlayerAction14(void);
s32 requestPlayerAction3(void);
s32 requestPlayerAction9(void);
s32 requestPlayerAction10(void);
s32 requestPlayerAction1(void);
s32 requestPlayerAction16(void);
s32 requestPlayerAction4(void);
s32 requestPlayerAction5(void);
s32 requestPlayerAction19(void);
s32 requestPlayerAction20(void);

void getPlayerCourseProgress(u16 playerIndex, s32 *progress, s32 *distance) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];
    s32 position;

    *progress = player->unk92E * 8;
    *distance = player->coursePlacementDistance;

    /* Remap course segments onto a common progress scale for race placement. */
    switch (gSelectedCourseIndex) {
    case 0:
        if (*progress >= 0x580) {
            *progress = 0x598 - *progress;
            *distance = -*distance;
        }
        break;
    case 1:
        if (*progress >= 0x4A8) {
            *progress = 0x4A8 - *progress;
            *distance = -*distance;
        }
        break;
    case 2: {
        s32 segmentPosition;
        s32 offset;
        segmentPosition = *progress;
        if (segmentPosition >= 0x490 && segmentPosition <= 0x4B8) {
            *progress = 0x490 - segmentPosition;
            *distance = -*distance;
            break;
        }
        offset = segmentPosition - 0x4C0;
        if ((u32)offset <= 0x10) {
            *progress = 0x80 + offset * 56 / 16;
        } else if (segmentPosition >= 0x4D8 && segmentPosition <= 0x5C0) {
            *progress = 0x188 + (segmentPosition - 0x4D8) * 288 / 232;
        } else if (segmentPosition >= 0x5C8 && segmentPosition <= 0x620) {
            *progress = segmentPosition - 0x338;
        }
        break;
    }
    case 3:
        position = *progress;
        if (position >= 0x628 && position <= 0x648) {
            *progress = 0x628 - position;
            *distance = -*distance;
        } else if (position >= 0x650 && position <= 0x7D0) {
            *progress = 0x60 + (position - 0x650) * 312 / 384;
        } else if (position >= 0x7D8 && position <= 0x8E0) {
            *progress = 0x2F0 + (position - 0x7D8) * 184 / 264;
        } else if (position >= 0x8E8 && position <= 0x920) {
            *progress = 0x520 + (position - 0x8E8) * 104 / 56;
        }
        break;
    case 4: {
        s32 offset;
        position = *progress;
        if (position >= 0x618 && position <= 0x640) {
            *progress = 0x618 - position;
            *distance = -*distance;
        }
        offset = *progress - 0x648;
        if ((u32)offset <= 0x50) {
            *progress = 0x380 + offset * 112 / 80;
        }
        break;
    }
    case 5:
        position = *progress;
        if (position >= 0x6D0 && position <= 0x6F8) {
            *progress = 0x6F8 - position;
            *distance = -*distance;
        }
        position = *progress;
        if (position >= 0x700 && position <= 0x7E8) {
            *progress = 0x1A0 + (position - 0x700) * 208 / 232;
        } else if (position >= 0x7F0 && position <= 0xA00) {
            *progress = 0x2C8 + (position - 0x7F0) * 640 / 528;
        } else if (position >= 0xA08 && position <= 0xA38) {
            *progress = 0x550 + (position - 0xA08) * 72 / 48;
        }
        break;
    case 6:
        if (*progress >= 0x7B8 && *progress <= 0x7E0) {
            *progress = 0x7F8 - *progress;
            *distance = -*distance;
        }
        break;
    case 8:
        if (*progress >= 0x1F0 && *progress <= 0x218) {
            *progress = 0x1F0 - *progress;
            *distance = -*distance;
        }
        break;
    }
}

void updateRacePlayerRankings(void) {
    s8 order[4];
    s32 progress[4];
    s32 distance[4];
    s32 i, j;
    RacePlayer *player;

    if (gMainMenuSelectionIndex == 2) {
        return;
    }
    if (gTaskTransitionFlags & 1) {
        gRaceOrderPlayerIds[0] = 0;
        gRaceOrderPlayerIds[1] = 1;
        gRaceOrderPlayerIds[2] = 2;
        gRaceOrderPlayerIds[3] = 3;
        return;
    }

    order[0] = 0;
    order[1] = 1;
    order[2] = 2;
    order[3] = 3;
    for (i = 0; i < D_8013AFD2; i++) {
        player = &gRacePlayers[i];
        player->coursePlacementDistance =
            -getGeometryPlacementSignedDistance(player->unk92E, player->x >> 8, player->z >> 8);
        getPlayerCourseProgress(i, &progress[i], &distance[i]);
        if (player->unk128 & 0x4000000) {
            progress[i] += player->unk966;
        }
    }

    for (i = 0; i < D_8013AFD2 - 1; i++) {
        for (j = i + 1; j < D_8013AFD2; j++) {
            if (gRacePlayers[order[i]].unk935 > gRacePlayers[order[j]].unk935) {
                s32 slot = order[i];
                order[i] = order[j];
                order[j] = slot;
            }
        }
    }

    for (i = 0; i < D_8013AFD2 - 1; i++) {
        RacePlayer *players = gRacePlayers;
        for (j = i + 1; j < D_8013AFD2; j++) {
            s32 firstIndex = order[i];
            RacePlayer *first = &gRacePlayers[firstIndex];
            s32 secondIndex;
            RacePlayer *second;
            u8 secondSlot;

            if (players[firstIndex].unk128 & 0x40) {
                continue;
            }
            secondIndex = order[j];
            second = &gRacePlayers[secondIndex];
            secondSlot = (u8)order[j];
            if (!(second->unk128 & 0x40)) {
                if (first->unk934 < second->unk934) {
                    order[i] = secondSlot;
                    order[j] = firstIndex;
                } else if (first->unk934 == second->unk934) {
                    if (progress[firstIndex] < progress[secondIndex]) {
                        order[i] = secondSlot;
                        order[j] = firstIndex;
                    } else if (progress[firstIndex] == progress[secondIndex] &&
                               distance[firstIndex] < distance[secondIndex]) {
                        order[i] = secondSlot;
                        order[j] = firstIndex;
                    }
                }
            }
        }
    }

    for (i = 0; i < D_8013AFD2; i++) {
        gRacePlayers[order[i]].unk935 = i;
        gRaceOrderPlayerIds[i] = gRacePlayers[order[i]].playerIndex;
    }
}

void resolveRacePlayerOverlaps(void) {
    RacePlayer *second;
    s16 heading;
    s32 shiftX;
    s32 halfZ;
    s32 halfX;
    s32 shiftZ;
    s32 radius;
    s32 sinAngle;
    s32 cosAngle;
    s32 separation;
    s32 dx;
    s32 j;
    s32 i;
    s32 heightLimit;
    s32 dz;

    for (i = 0; i < D_8013AFD2 - 1; i++) {
        for (j = i + 1; j < D_8013AFD2; j++) {
            second = &gRacePlayers[j];
            if (!gRacePlayers[i].isActive || !second->isActive ||
                (gRacePlayers[i].unk128 & 0x200000) || (second->unk128 & 0x200000)) {
                continue;
            }
            separation = gRacePlayers[i].unk074 - second->unk074;
            if (separation < 0) {
                heightLimit = gRacePlayers[i].unk0B0;
                separation = -separation;
            } else {
                heightLimit = second->unk0B0;
            }
            if (separation > heightLimit * 2) {
                continue;
            }

            radius = gRacePlayers[i].unk0AC + second->unk0AC;
            dx = gRacePlayers[i].x - second->x;
            if (dx < 0) {
                dx = -dx;
            }
            if (dx >= radius) {
                continue;
            }

            dz = gRacePlayers[i].z - second->z;
            if (dz < 0) {
                dz = -dz;
            }
            if (dz >= radius) {
                continue;
            }

            separation = SquareRoot0(dx * dx + dz * dz);
            if (separation >= radius) {
                continue;
            }

            separation = (separation - radius) / 2;
            heading = getXZHeading(gRacePlayers[i].x, gRacePlayers[i].z, second->x, second->z);
            sinAngle = rsin(heading);
            cosAngle = rcos(heading);
            shiftX = (-sinAngle * separation) / 4096;
            shiftZ = (cosAngle * separation) / 4096;

            if (gRacePlayers[i].unk128 & 0x1000) {
                if (second->unk128 & 0x1000) {
                    continue;
                }
                second->x += shiftX;
                second->z -= shiftZ;
            } else if (second->unk128 & 0x1000) {
                gRacePlayers[i].x -= shiftX;
                gRacePlayers[i].z += shiftZ;
                continue;
            } else {
                halfX = shiftX / 2;
                gRacePlayers[i].x -= halfX;
                halfZ = shiftZ / 2;
                gRacePlayers[i].z += halfZ;
                second->x += halfX;
                second->z -= halfZ;
            }
        }
    }
}

u16 arePositionsWithinXZRange(const Position3D *a, const Position3D *b, s32 threshold) {
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

void resolvePlayerCollisionsWithCylinder(const Position3D *position, s32 radius, s32 height, s32 flags) {
    u16 i;
    u16 hitFlags = flags;

    for (i = 0; i < D_8013AFD2; i++) {
        RacePlayer *player = &gRacePlayers[i];
        s32 distance, verticalRange, x, z, range;
        s32 sine, cosine, tangent, normal;
        s32 correctionX, correctionZ;

        if (!player->isActive || (player->unk128 & 0x1000) ||
            !arePositionsWithinXZRange(position, (const Position3D *)&gRacePlayers[i].x, 0x18000)) {
            continue;
        }
        distance = position->y - player->unk074;
        if (distance < 0) {
            distance = -distance;
            verticalRange = player->unk0B0 * 2;
        } else {
            verticalRange = height;
        }
        if (distance > verticalRange) {
            continue;
        }

        player = &gRacePlayers[i];
        x = position->x - player->x;
        range = radius + player->unk0AC;
        if (x < 0) {
            x = -x;
        }
        if (x >= range) {
            continue;
        }

        z = position->z - player->z;
        if (z < 0) {
            z = -z;
        }
        if (z >= range) {
            continue;
        }

        distance = SquareRoot0(x * x + z * z);
        if (distance >= range) {
            continue;
        }
        distance -= range;
        D_8013B0E6 = getXZHeading(position->x, position->z, player->x, player->z);
        sine = rsin((s16)D_8013B0E6);
        cosine = rcos((s16)D_8013B0E6);

        correctionX = (-sine * distance) / 4096;
        correctionZ = (cosine * distance) / 4096;
        player->x += correctionX;
        player->z -= correctionZ;

        tangent = (cosine * player->unk0EC - sine * player->unk0F0) / 4096;
        normal = (sine * player->unk0EC + cosine * player->unk0F0) / 4096;
        if (normal > 0) {
            normal = -normal;
        }
        player->unk0EC = -(cosine * tangent + sine * normal) / 4096;
        player->unk0F0 = -(-sine * tangent + cosine * normal) / 4096;
        D_8013B0E8 = player->unk0BC / 2;
        player->unk0E8 |= hitFlags;
    }
}

s32 resolvePlayerCylinderCollisionsBySpeed(const Position3D *position, s32 radius, s32 height,
    s32 speedThreshold, u16 flags) {
    s16 hit = 0;
    s32 i;
    RacePlayer *player;
    s32 x, distance, z;
    RacePlayer *collisionPlayer;
    s32 range, verticalRange;
    s16 heading;
    s32 sine, cosine;

    for (i = 0; i < D_8013AFD2; i++) {
        player = &gRacePlayers[i];
        if (player->isActive &&
            arePositionsWithinXZRange(position, (const Position3D *)&gRacePlayers[i].x, 0x18000)) {
            distance = position->y - player->unk074;
            if (distance < 0) {
                distance = -distance;
                verticalRange = player->unk0B0 * 2;
            } else {
                verticalRange = height;
            }
            if (distance <= verticalRange) {
                x = position->x - player->x;
                range = radius + player->unk0AC;
                if (x < 0) {
                    x = -x;
                }
                if (x < range) {
                    z = position->z - player->z;
                    if (z < 0) {
                        z = -z;
                    }
                    if (z < range) {
                        distance = SquareRoot0(x * x + z * z);
                        collisionPlayer = player;
                        if (distance < range) {
                            if (player->unk0BC < speedThreshold) {
                                distance -= range;
                                heading = getXZHeading(position->x, position->z,
                                    collisionPlayer->x, collisionPlayer->z);
                                sine = rsin(heading);
                                cosine = rcos(heading);
                                range = (-sine * distance) / 4096;
                                z = cosine * distance;
                                /* Round the signed fixed-point correction toward zero. */
                                if (z < 0) {
                                    z += 4095;
                                }
                                collisionPlayer->x += range;
                                collisionPlayer->z -= z >> 12;
                            } else {
                                collisionPlayer->unk0E8 |= flags;
                                hit = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return hit;
}

u16 pushPlayerOutOfCylinder(const Position3D *position, s32 radius, s32 height, s16 playerIndex) {
    RacePlayer *player;
    RacePlayer *players;
    s32 x, distance, z, range, verticalRange;

    players = gRacePlayers;
    player = &players[playerIndex];
    if (!player->isActive) {
        return 0;
    }
    if (arePositionsWithinXZRange(position, (const Position3D *)&gRacePlayers[playerIndex].x, 0x10000)) {
        distance = position->y - player->unk074;
        if (distance < 0) {
            distance = -distance;
            verticalRange = height;
        } else {
            verticalRange = player->unk0B0;
        }
        if (distance < verticalRange) {
            RacePlayer *horizontalPlayer;
            RacePlayer *horizontalPlayers = gRacePlayers;

            horizontalPlayer = &horizontalPlayers[playerIndex];
            x = position->x - horizontalPlayer->x;
            range = radius + horizontalPlayer->unk0AC;
            if (x < 0) {
                x = -x;
            }
            if (x < range) {
                z = position->z - horizontalPlayer->z;
                if (z < 0) {
                    z = -z;
                }
                if (z < range) {
                    distance = SquareRoot0(x * x + z * z);
                    if (distance < range) {
                        s16 heading;
                        s32 sine, cosine;
                        s32 correctionX, correctionZ;

                        distance -= range;
                        heading = getXZHeading(position->x, position->z, horizontalPlayer->x, horizontalPlayer->z);
                        sine = rsin(heading);
                        cosine = rcos(heading);
                        correctionX = (-sine * distance) / 4096;
                        correctionZ = (cosine * distance) / 4096;
                        horizontalPlayer->x += correctionX;
                        horizontalPlayer->z -= correctionZ;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

u16 isPositionWithinPlayerCollisionRange(const Position3D *position, s32 radius, s32 height, s16 playerIndex) {
    RacePlayer *player;
    RacePlayer *players;
    s32 x, y, z, range, verticalRange;
    players = gRacePlayers;
    player = &players[playerIndex];
    if (player->isActive) {
        if (arePositionsWithinXZRange(position, (Position3D *)&gRacePlayers[playerIndex].x, 0x10000)) {
            y = position->y - player->unk074;
            verticalRange = height;
            if (y < 0) {
                y = -y;
            } else {
                verticalRange = player->unk0B0;
            }
            if (y < verticalRange) {
                RacePlayer *horizontalPlayer;
                RacePlayer *horizontalPlayers = gRacePlayers;
                horizontalPlayer = &horizontalPlayers[playerIndex];
                x = position->x - horizontalPlayer->x;
                range = radius + horizontalPlayer->unk0AC;
                if (x < 0) {
                    x = -x;
                }
                if (x < range) {
                    z = position->z - horizontalPlayer->z;
                    if (z < 0) {
                        z = -z;
                    }
                    if (z < range) {
                        if (SquareRoot0(x * x + z * z) < range) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

u16 addPlayerFlagsIfWithinRadius(Position3D *position, s32 radius, s16 flags, u16 playerIndex) {
    RacePlayer *player;
    RacePlayer *players;
    s32 x, y, z, range;
    if (gRacePlayers[playerIndex].isActive) {
        if (flags != 4 && gRacePlayers[playerIndex].surfaceState.fields.unk134 != 0) {
            return 0;
        }
        players = gRacePlayers;
        player = &players[playerIndex];
        x = player->x - position->x;
        range = radius + player->unk0AC;
        if (x < 0) {
            x = -x;
        }
        if (x < range) {
            y = player->unk074 - player->unk0AC - position->y;
            if (y < 0) {
                y = -y;
            }
            if (y < range) {
                z = player->z - position->z;
                if (z < 0) {
                    z = -z;
                }
                if (z < range) {
                    if (SquareRoot0(x * x + y * y + z * z) < range) {
                        player->unk0E8 |= flags;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void addPlayerFlagsWithinRadius(const Position3D *position, s32 radius, u16 flags) {
    RacePlayer *player;
    s32 i;
    s32 x, y, z, range;

    for (i = 0; i < 4; i++) {
        player = &gRacePlayers[i];
        if (player->isActive) {
            x = player->x - position->x;
            range = radius + player->unk0AC;
            if (x < 0) {
                x = -x;
            }
            if (x < range) {
                y = player->unk074 - player->unk0AC - position->y;
                if (y < 0) {
                    y = -y;
                }
                if (y < range) {
                    z = player->z - position->z;
                    if (z < 0) {
                        z = -z;
                    }
                    if (z < range) {
                        if (SquareRoot0(x * x + y * y + z * z) < range) {
                            player->unk0E8 |= flags;
                        }
                    }
                }
            }
        }
    }
}

void updateRacePlayerFinishSlotsTask(void) {
    u16 i;
    if (gTaskTransitionFlags & 0x40) {
        deleteCurrentTaskSubtree();
    } else {
        if (gMainMenuSelectionIndex == 1 && D_8013AFE0 == 1) {
            D_8013B1D4 = 0;
        }
        resolveRacePlayerOverlaps();
        updateRacePlayerRankings();
        for (i = 0; i < D_8013AFD2; i++) {
            currentPlayerAbsolute = &gRacePlayers[i];
            assignCurrentPlayerFinishSlot(i);
        }
    }
}

s32 requestCurrentPlayerAction6(void) {
    RacePlayer *player = currentPlayerAbsolute;
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
    RacePlayer *player = currentPlayerAbsolute;
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
    RacePlayer *player = currentPlayerAbsolute;
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

s32 requestPlayerAction8(void) {
    s32 result;
    if (currentPlayerAbsolute->unk128 & 0x37D000) {
        result = 1;
    } else {
        if (D_8013B0E4 < 8) {
            D_8013B0E4 = 8;
        }
        result = 0;
    }
    return result;
}

s32 requestPlayerAction12(void) {
    s16 result;
    if (currentPlayerAbsolute->unk128 & 0x77D000) {
        result = 1;
    } else {
        if (D_8013B0E4 < 12) {
            D_8013B0E4 = 12;
        }
        result = 0;
    }
    return result;
}

s32 requestPlayerAction11(void) {
    if (currentPlayerAbsolute->unk128 & 0x77D000) {
        return 1;
    }
    if (D_8013B0E4 < 11) {
        D_8013B0E4 = 11;
    }
    return 0;
}

s32 requestPlayerAction13(void) {
    if (currentPlayerAbsolute->unk128 & 0x77D000) return 1;
    if (D_8013B0E4 < 13) D_8013B0E4 = 13;
    return 0;
}

s32 requestPlayerAction14(void) {
    s32 result;
    if (currentPlayerAbsolute->unk128 & 0x77D000) {
        result = 1;
    } else {
        if (D_8013B0E4 < 14) {
            D_8013B0E4 = 14;
        }
        result = 0;
    }
    return (s16)result;
}

s32 requestPlayerAction3(void) {
    if (currentPlayerAbsolute->unk128 & 0x77D000) {
        return 1;
    }
    if (D_8013B0E4 < 3) {
        D_8013B0E4 = 3;
    }
    return 0;
}

s32 requestPlayerAction9(void) {
    u32 flags = currentPlayerAbsolute->unk128;
    if (!(flags & 0x43000)) {
        if (flags & 0x800) {
            if (D_8013B0E4 < 9) {
                D_8013B0E4 = 9;
            }
            return 0;
        }
    }
    return 1;
}

s32 requestPlayerAction10(void) {
    u32 flags = currentPlayerAbsolute->unk128;
    if (!(flags & 0x43000)) {
        if (flags & 0x800) {
            if (D_8013B0E4 < 10) {
                D_8013B0E4 = 10;
            }
            return 0;
        }
    }
    return 1;
}

s32 checkCurrentPlayerFlags43040OrAngleRange(void) {
    RacePlayer *player = currentPlayerAbsolute;
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

s32 requestPlayerAction1(void) {
    u8 result;
    if (currentPlayerAbsolute->unk128 & 0x43000) {
        result = 1;
    } else {
        if (D_8013B0E4 <= 0) {
            D_8013B0E4 = 1;
        }
        result = 0;
    }
    return result;
}

s32 checkCurrentPlayerFlags345001(void) {
    RacePlayer *player = currentPlayerAbsolute;
    if (player->unk12C != 14 && (player->unk128 & 0x345001)) {
        return 1;
    }
    if (D_8013B0E4 < 15) {
        D_8013B0E4 = 15;
    }
    return 0;
}

s32 requestPlayerAction16(void) {
    if (currentPlayerAbsolute->unk128 & 0x775000) {
        return 1;
    } else {
        if (D_8013B0E4 < 16) {
            D_8013B0E4 = 16;
        }
        return 0;
    }
}

s32 requestCurrentPlayerAction17(void) {
    RacePlayer *player = currentPlayerAbsolute;

    if (player->unk128 & 0x275000) {
        return 1;
    }
    if (player->unk0F6 != 0) {
        return 1;
    }
    if (player->surfaceState.fields.unk134 != 0) {
        return 1;
    }
    if (D_8013B0E4 < 17) {
        D_8013B0E4 = 17;
    }
    return 0;
}

s32 checkCurrentPlayerFlags275000OrField134(void) {
    RacePlayer *player = currentPlayerAbsolute;
    if (player->unk128 & 0x275000) {
        return 1;
    }
    if (player->surfaceState.fields.unk134) {
        return 1;
    }
    if (D_8013B0E4 < 18) {
        D_8013B0E4 = 18;
    }
    return 0;
}

s32 requestPlayerAction4(void) {
    if (currentPlayerAbsolute->unk128 & 0x775000) {
        return 1;
    }
    if (D_8013B0E4 < 4) {
        D_8013B0E4 = 4;
    }
    return 0;
}

s32 requestPlayerAction5(void) {
    if (currentPlayerAbsolute->unk128 & 0x775000) {
        return 1;
    }
    if (D_8013B0E4 < 5) {
        D_8013B0E4 = 5;
    }
    return 0;
}

s32 requestPlayerAction19(void) {
    if (currentPlayerAbsolute->unk128 & 0x41001) {
        return 1;
    }
    if (D_8013B0E4 < 19) {
        D_8013B0E4 = 19;
    }
    return 0;
}

s32 requestPlayerAction20(void) {
    if (currentPlayerAbsolute->unk128 & 0x41001) {
        return 1;
    } else {
        if (D_8013B0E4 < 20) {
            D_8013B0E4 = 20;
        }
        return 0;
    }
}

s16 processCurrentPlayerActionRequests(s16 suppressWallAction) {
    u16 pending = currentPlayerAbsolute->unk0E8;

    D_8013B0E4 = 0;
    if (pending) {
        if ((pending & 0x200) && !suppressWallAction) {
            currentPlayerAbsolute->unk0E8 = pending | 1;
        }
        if ((currentPlayerAbsolute->unk0E8 & 1) && requestPlayerAction9() &&
            requestCurrentPlayerAction6() && checkCurrentPlayerFlags43040OrAngleRange()) {
            requestPlayerAction1();
        }
        if ((currentPlayerAbsolute->unk0E8 & 2) && requestPlayerAction9()) {
            requestCurrentPlayerAction6NarrowAngle();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x800) {
            requestCurrentPlayerAction7();
        }
        if ((currentPlayerAbsolute->unk0E8 & 0x400) && requestPlayerAction11()) {
            requestPlayerAction11();
        }
        if ((currentPlayerAbsolute->unk0E8 & 4) && requestPlayerAction10()) {
            requestPlayerAction8();
        }
        if (currentPlayerAbsolute->unk0E8 & 8) {
            requestPlayerAction12();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x100) {
            requestPlayerAction14();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x4000) {
            requestPlayerAction3();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x20) {
            requestCurrentPlayerAction17();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x1000) {
            checkCurrentPlayerFlags275000OrField134();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x10) {
            requestPlayerAction16();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x40) {
            requestPlayerAction4();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x2000) {
            requestPlayerAction5();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x80) {
            requestPlayerAction13();
        }
        currentPlayerAbsolute->unk0E8 = 0;
    }

    if ((currentPlayerAbsolute->surfaceState.word & 0xFFFF0000) == 0x03030000 || currentPlayerAbsolute->surfaceState.fields.unk136 == 8) {
        checkCurrentPlayerFlags345001();
    }
    if (currentPlayerAbsolute->surfaceState.fields.unk136 == 6 || currentPlayerAbsolute->surfaceState.fields.unk136 == 12 || currentPlayerAbsolute->surfaceState.fields.unk136 == 16) {
        requestPlayerAction19();
    }
    if (currentPlayerAbsolute->surfaceState.fields.unk136 == 7 || currentPlayerAbsolute->surfaceState.fields.unk136 == 14 || currentPlayerAbsolute->surfaceState.fields.unk136 == 15) {
        requestPlayerAction20();
    }

    if (!D_8013B0E4) {
        return 0;
    }
    if (currentPlayerAbsolute->unk128 & 0x800) {
        currentPlayerAbsolute->trickAttackMakeCount = 0;
    }

    switch ((s16)(D_8013B0E4 - 1)) {
        case 7 - 1:
            playPlayerSoundEffect(gCurrentTask->data18, 0, 0x302, 0x40);
            /* fall through */
        case 6 - 1:
            if (D_8013B0E4 != 7) {
                playCurrentPlayerSoundEffect(2);
            }
            currentPlayerAbsolute->unk12C = 4;
            currentPlayerAbsolute->unk102 = D_8013B0E6;
            currentPlayerAbsolute->unk104.word = currentPlayerAbsolute->unk0EC;
            currentPlayerAbsolute->unk108.word = currentPlayerAbsolute->unk0F0;
            playPlayerSoundEffect(gCurrentTask->data18, 1, 0x201, 0x20);
            gScratchVector0->vx = currentPlayerAbsolute->worldCoordinate.coord.t[0];
            gScratchVector0->vy = currentPlayerAbsolute->worldCoordinate.coord.t[1] - 48;
            gScratchVector0->vz = currentPlayerAbsolute->worldCoordinate.coord.t[2];
            startAlternateQuadAnimationTask(gCurrentTask->data18, (const PositionEffectOrigin *)gScratchVector0);
            break;
        case 9 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 14;
            playPlayerSoundEffect(gCurrentTask->data18, 1, 0x201, 0x20);
            gScratchVector0->vx = currentPlayerAbsolute->worldCoordinate.coord.t[0];
            gScratchVector0->vy = currentPlayerAbsolute->worldCoordinate.coord.t[1] - 48;
            gScratchVector0->vz = currentPlayerAbsolute->worldCoordinate.coord.t[2];
            startAlternateQuadAnimationTask(gCurrentTask->data18, (const PositionEffectOrigin *)gScratchVector0);
            break;
        case 10 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 14;
            currentPlayerAbsolute->unk102 = D_8013B0E6;
            currentPlayerAbsolute->unk104.word = currentPlayerAbsolute->unk0EC;
            currentPlayerAbsolute->unk108.word = currentPlayerAbsolute->unk0F0;
            break;
        case 2 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 3;
            currentPlayerAbsolute->unk102 = D_8013B0E6;
            currentPlayerAbsolute->unk104.word = currentPlayerAbsolute->unk0EC;
            currentPlayerAbsolute->unk108.word = currentPlayerAbsolute->unk0F0;
            playPlayerSoundEffect(gCurrentTask->data18, 1, 0x201, 0x20);
            gScratchVector0->vx = currentPlayerAbsolute->worldCoordinate.coord.t[0];
            gScratchVector0->vy = currentPlayerAbsolute->worldCoordinate.coord.t[1] - 48;
            gScratchVector0->vz = currentPlayerAbsolute->worldCoordinate.coord.t[2];
            startAlternateQuadAnimationTask(gCurrentTask->data18, (const PositionEffectOrigin *)gScratchVector0);
            break;
        case 3 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 3;
            currentPlayerAbsolute->unk102 = -1;
            currentPlayerAbsolute->unk104.word = 0;
            currentPlayerAbsolute->unk108.word = 0;
            break;
        case 1 - 1:
            currentPlayerAbsolute->unk12E = 0x300;
            break;
        case 8 - 1:
        case 11 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 11;
            break;
        case 12 - 1:
            gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk943 = 1;
            playCurrentPlayerSoundEffect(gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk008 ? 2 : 3);
            currentPlayerAbsolute->unk12C = 40;
            break;
        case 13 - 1:
            gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk943 = 1;
            playCurrentPlayerSoundEffect(gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk008 ? 2 : 3);
            currentPlayerAbsolute->unk12C = 26;
            break;
        case 14 - 1:
            gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk943 = 1;
            playCurrentPlayerSoundEffect(gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk008 ? 2 : 3);
            currentPlayerAbsolute->unk12C = 27;
            break;
        case 15 - 1:
            playCurrentPlayerSoundEffect(4);
            currentPlayerAbsolute->unk12C = 10;
            break;
        case 17 - 1:
        case 18 - 1:
            if (D_8013B0E4 == 18) {
                playCurrentPlayerSoundEffect(gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk008 ? 2 : 3);
            } else {
                playCurrentPlayerSoundEffect(2);
            }
            currentPlayerAbsolute->unk12C = 24;
            break;
        case 16 - 1:
            gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk943 = 1;
            playCurrentPlayerSoundEffect(gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk008 ? 2 : 3);
            currentPlayerAbsolute->unk12C = 23;
            break;
        case 4 - 1:
            gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk943 = 1;
            playCurrentPlayerSoundEffect(gRacePlayers[currentPlayerAbsolute->unk0F4 & 3].unk008 ? 2 : 3);
            currentPlayerAbsolute->unk12C = 25;
            break;
        case 5 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 25;
            break;
        case 19 - 1:
            currentPlayerAbsolute->unk12C = 6;
            break;
        case 20 - 1:
            currentPlayerAbsolute->unk12C = 9;
            break;
    }

    return D_8013B0E4;
}

s16 checkCurrentPlayerSurfaceType(void) {
    RacePlayer *player = currentPlayerAbsolute;
    u8 surface = getCourseSurfaceAtPoint(player->unk92E, player->x >> 8, player->z >> 8);
    if (surface == 3 || surface == 6 || surface == 7 || surface == 8 ||
        surface == 12 || surface == 14 || surface == 15 || surface == 16) {
        return 0;
    }
    return 1;
}

s16 countCurrentPlayerInputGroups(void) {
    s32 count = 0;
    u16 buttons;
    if (currentPlayerAbsolute->unk008) {
        return (randomNextMain() & 3) == 0;
    }
    buttons = gControllerPressedButtons[gCurrentTask->data18];
    if (buttons & 0xF000) {
        count = 1;
    }
    if (buttons & 0x40) {
        count++;
    }
    return count;
}

void clearCurrentPlayerTaskState(void) {
    GameTask *task = gCurrentTask;
    RacePlayer *player = currentPlayerAbsolute;

    task->data1A = 0;
    task->data1C = 0;
    task->data1E = 0;
    player->unk0C2 = 0;
}

void dispatchCurrentPlayerDirectionalAnimation(void) {
    gCurrentPlayerDirectionalAnimationCallbacks[gCurrentTask->data1A]();
}

void startCurrentPlayerDirectionalAnimation(void) {
    RacePlayer *player = currentPlayerAbsolute;
    player->unk0C4 = 0;
    player->unk0C6 = 0;
    if (!player->unk008) {
        if (gControllerPressedButtons[gCurrentTask->data18] & 8) {
            gCurrentTask->data1C = 8;
            player->unk0C4 = 1;
        }
        if (gControllerPressedButtons[gCurrentTask->data18] & 4) {
            gCurrentTask->data1C = 4;
            currentPlayerAbsolute->unk0C4 = 3;
        }
        if (gControllerPressedButtons[gCurrentTask->data18] & 1) {
            gCurrentTask->data1C = 1;
            currentPlayerAbsolute->unk0C4 = 7;
        }
        if (gControllerPressedButtons[gCurrentTask->data18] & 2) {
            gCurrentTask->data1C = 2;
            currentPlayerAbsolute->unk0C4 = 5;
        }
        if (gCurrentTask->data1C) {
            if ((s16)currentPlayerAbsolute->unk0C2 < 6) {
                currentPlayerAbsolute->unk0C4++;
            }
            gCurrentTask->data1A++;
            if (currentPlayerAbsolute->unk128 & 0x400) {
                if (gCurrentTask->data1C & 8) {
                    startCurrentPlayerAnimation(0x1C, 2);
                }
                if (gCurrentTask->data1C & 4) {
                    startCurrentPlayerAnimation(0x1A, 2);
                }
            } else {
                if (gCurrentTask->data1C & 8) {
                    startCurrentPlayerAnimation(0x1A, 2);
                }
                if (gCurrentTask->data1C & 4) {
                    startCurrentPlayerAnimation(0x1C, 2);
                }
            }
            if (gCurrentTask->data1C & 1) {
                startCurrentPlayerAnimation(0x1E, 2);
            }
            if (gCurrentTask->data1C & 2) {
                startCurrentPlayerAnimation(0x20, 2);
            }
            currentPlayerAbsolute->unk128 |= 0x800;
        }
        currentPlayerAbsolute->unk0C2++;
    }
}

void updateCurrentPlayerDirectionalAnimation(void) {
    GameTask *task = gCurrentTask;
    RacePlayer *player;
    if (!(gControllerHeldButtons[task->data18] & task->data1C)) {
        task->data1E = 1;
    }
    player = currentPlayerAbsolute;
    if (player->animationFlags & 0x10) {
        GameTask *animationTask = gCurrentTask;
        if (animationTask->data1E) {
            animationTask->data1A++;
            if (player->unk128 & 0x400) {
                if (animationTask->data1C & 8) {
                    startCurrentPlayerAnimation(0x1D, 2);
                }
                if (gCurrentTask->data1C & 4) {
                    startCurrentPlayerAnimation(0x1B, 2);
                }
            } else {
                if (animationTask->data1C & 8) {
                    startCurrentPlayerAnimation(0x1B, 2);
                }
                if (gCurrentTask->data1C & 4) {
                    startCurrentPlayerAnimation(0x1D, 2);
                }
            }
            if (gCurrentTask->data1C & 1) {
                startCurrentPlayerAnimation(0x1F, 2);
            }
            if (gCurrentTask->data1C & 2) {
                startCurrentPlayerAnimation(0x21, 2);
            }
        } else {
            player->unk0C6++;
        }
    }
    currentPlayerAbsolute->unk128 |= 0x800;
}

void waitForCurrentPlayerDirectionalAnimationEnd(void) {
    if (currentPlayerAbsolute->animationFlags & 0x10) {
        gCurrentTask->data1A++;
    } else {
        currentPlayerAbsolute->unk128 |= 0x800;
    }
}

void func_8002E3C0(void) {
}

void addPlayerCappedCounters(u16 playerIndex, s32 amount) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player;
    players += playerIndex;
    player = players;
    if (player->isActive) {
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

void playCurrentPlayerSoundEffect(s16 index) {
    switch (index) {
    case 0:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, 0, 64);
        break;
    case 1:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, gPlayerSoundVariantsA[randomNextMain() & 1], 64);
        break;
    case 2:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, gPlayerSoundVariantsB[randomNextMain() & 1], 64);
        break;
    case 3:
        playPlayerSoundEffectWithVolumeFloor(gCurrentTask->data18, gCurrentTask->data18 + 2,
                                            gPlayerSoundVariantsB[randomNextMain() & 1], 64);
        break;
    case 4:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, 0x500, 64);
        break;
    case 5:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, 0x600, 64);
        break;
    case 6:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, 0x700, 64);
        break;
    case 7:
        playPlayerPositionalSoundEffect(gCurrentTask->data18, gCurrentTask->data18 + 2, 0x800, 64);
        break;
    }
}

void queuePlayerControllerEffect(u16 controller, s16 effect) {
    if (currentPlayerAbsolute->unk008 == 0) {
        D_8013AF08[controller] = effect;
    }
}

void startPlayerTimerTask(RacePlayer *player) {
    player->unk0FC = 240;
    if (player->unk128 & 0x400) {
        player->unk0FF = 1;
    } else {
        player->unk0FF = 0;
    }
    createChildTaskForParentWithReserve(player->task, initPlayerTimerTask);
    gCreatedTask->data18 = player->playerIndex;
}


void updateCurrentPlayerProjectileUse(u16 playerIndex) {
    s32 activate = 0;

    if (!currentPlayerAbsolute->unk008) {
        activate = ((u8)gControllerPressedButtons[gCurrentTask->data18]) >> 7;
    } else {
        if (currentPlayerAbsolute->projectileUseCooldown == 0) {
            currentPlayerAbsolute->projectileUseCooldown = 10;
            if ((u8)randomNextMain() <= currentPlayerAbsolute->unk02C[1]) {
                activate = 1;
            }
        } else {
            currentPlayerAbsolute->projectileUseCooldown--;
        }

        if (!activate) {
            return;
        }

        if (!currentPlayerAbsolute->unk940) {
            goto tryActivation;
        }
        {
            s32 i = 0;

            /* CPU players check for a nearby human in their forward arc. */
            activate = i;
            for (; i < D_8013AFD2; i++) {
                if (!gRacePlayers[i].unk008) {
                    s32 dx = gRacePlayers[i].x - currentPlayerAbsolute->x;
                    s32 dz = gRacePlayers[i].z - currentPlayerAbsolute->z;

                    if (dx >= -0x5FFFF && dx <= 0x5FFFF &&
                        dz <= 0x5FFFF && dz > -0x60000) {
                        s32 angle = getXZHeading(0, 0, dx, dz) & 0xFFF;
                        if ((u32)(((angle - currentPlayerAbsolute->unk110) & 0xFFF) - 0x200) > 0xC00) {
                            activate = 1;
                            currentPlayerAbsolute->projectileUseCooldown = -62;
                        }
                    }
                }
            }
        }
    }

tryActivation:
    if (!activate) {
        return;
    }

    if (currentPlayerAbsolute->unk940) {
        switch ((s8)(currentPlayerAbsolute->unk93E - 1)) {
        case 0:
            createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerDualResourceEffectTask);
            break;
        case 1:
            createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerVisualPairTask);
            break;
        case 2:
            createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerVisualPairTask5E80);
            break;
        case 3:
            createChildTaskForParentWithReserve(&gGameTasks[4], initializePlayerVisualPairTask6780);
            break;
        case 4: {
            u8 bounce = 0;
            if (!currentPlayerAbsolute->unk008) {
                bounce = (gControllerHeldButtons[playerIndex] & 0xE000) == 0x4000;
            } else {
                s32 roll = randomNextSecondary();
                if (gPlayerProjectileBounceChoices[currentPlayerAbsolute->unk935][roll & 7]) {
                    bounce = 1;
                }
            }
            if (bounce) {
                createChildTaskForParentWithReserve(&gGameTasks[4], initializeBouncingTaskVisual);
            } else {
                createChildTaskForParentWithReserve(&gGameTasks[4], initializeDualResource4C80EffectTask);
            }
            break;
        }
        default:
            goto clearEmptyEffect;
        }
        gCreatedTask->data18 = playerIndex;
        currentPlayerAbsolute->unk940--;

    clearEmptyEffect:
        if (!currentPlayerAbsolute->unk940) {
            currentPlayerAbsolute->unk93E = 0;
        }
    }

    if (currentPlayerAbsolute->unk93C) {
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeDualResource5100EffectTask);
        gCreatedTask->data18 = playerIndex;
        currentPlayerAbsolute->unk93C--;
    }
}

void updateCurrentPlayerItemUse(u16 playerIndex) {
    u16 activate = 0;

    if (!currentPlayerAbsolute->unk008) {
        activate = (gControllerPressedButtons[playerIndex] >> 5) & 1;
    } else {
        if ((s8)currentPlayerAbsolute->itemUseCooldown != 0) {
            currentPlayerAbsolute->itemUseCooldown--;
        } else {
            if ((u8)randomNextMain() <= currentPlayerAbsolute->unk02C[0]) {
                activate = 1;
            }
            currentPlayerAbsolute->itemUseCooldown = 10;
        }
        if ((s8)currentPlayerAbsolute->heldItem == 6) {
            activate = currentPlayerAbsolute->unk946 != 0;
        }
    }

    if (!activate) {
        return;
    }

    switch ((s8)(currentPlayerAbsolute->heldItem - 1)) {
    case 0:
        if (currentPlayerAbsolute->unk0FC != 0) {
            return;
        }
        startPlayerTimerTask(currentPlayerAbsolute);
        queuePlayerControllerEffect(playerIndex, 13);
        goto clearItem;
    case 1:
        tryCreateChildTaskForParentWith20SlotsReserved(&gGameTasks[4], initializeOrbitingPlayerVisualTask);
        gCreatedTask->data18 = playerIndex;
        currentPlayerAbsolute->heldItem = 0;
        return;
    case 2: {
        u16 count = 0;
        u16 index;
        for (index = 0; index < D_8013AFD2; index++) {
            if (index != playerIndex) {
                s32 delay;
                createChildTaskForParentWithReserve(&gGameTasks[4], initializeDelayedPlayerRiseEffectTask);
                delay = count * 3;
                count++;
                gCreatedTask->data18 = index;
                gCreatedTask->data1A = playerIndex;
                gCreatedTask->data1C = delay;
            }
        }
        goto clearItem;
    }
    case 3:
        tryCreateChildTaskForParentWith20SlotsReserved(&gGameTasks[4], initializeFallingTaskObject);
        gCreatedTask->data18 = playerIndex;
        playPlayerSoundEffect(gCurrentTask->data18, 1, 2, 32);
        goto clearItem;
    case 4:
        if (!currentPlayerAbsolute->unk008) {
            tryCreateChildTaskForParentWith20SlotsReserved(&gGameTasks[4], initializeSlidingVisualSequenceTask);
            gCreatedTask->data18 = playerIndex;
        }
    clearItem:
        currentPlayerAbsolute->heldItem = 0;
        return;
    case 5: {
        RacePlayer *player = currentPlayerAbsolute;
        if (player->surfaceState.fields.unk134 == 0) {
            player->heldItem = 0;
            player->surfaceState.fields.unk134 = 180;
            playPlayerSoundEffect(gCurrentTask->data18, 1, 0xE00, 32);
        }
        return;
    }
    default:
        return;
    }
}

void loadCurrentPlayerModelBlock(void) {
    u32 *offsets = DAT_STARTUP_MODELS->offsets;
    u8 *archive = (u8 *)DAT_STARTUP_MODELS;
    u32 (*buffers)[512] = DAT_PLAYER_MODEL_BLOCKS;
    u32 i = 0;
    u16 player = gCurrentTask->data18;
    u32 *dest;
    u32 offset;
    u32 *source;
    u32 *model;
    offset = offsets[D_80123640[gRacePlayers[player].unk015[1]]];
    model = buffers[player];
    source = (u32 *)(offset + archive);
    dest = model;
    for (; i < 512; i++) {
        *dest++ = *source++;
    }
    GsMapModelingData((u32 *)((u32)model | 4));
    {
        u32 (*largeBuffers)[4096] = DAT_PLAYER_MODEL_BUFFERS;
        i = 3;
        dest = model;
        model = largeBuffers[gCurrentTask->data18];
        {
            u32 *secondSource = dest + 3;
            dest = model + 3;
            for (; i < 10; i++) {
                *dest++ = *secondSource++;
            }
        }
    }
}

void initializeNewGameSaveData(void) {
    u16 i, j;
    SaveRecord *first = gPlayerSaveRecords;
    SaveRecord *second;
    SaveRecord *destination;

    initializeSaveRecordDefaults(first);
    second = first + 1;
    initializeSaveRecordDefaults(second);
    destination = &gMergedSaveRecord;
    copyGlobalFlagState(first, destination);
    mergeSaveRecordFlags(second, destination);

    for (i = 0; i < 11; i++) {
        for (j = 0; j < 5; j++) {
            gPlayerSaveRecords[0].lapRecords[i][j].minutes = 3;
            gPlayerSaveRecords[0].lapRecords[i][j].seconds = 59;
            gPlayerSaveRecords[0].lapRecords[i][j].centiseconds = 0;
            gPlayerSaveRecords[0].timeRecords[i][j].minutes = 3;
            gPlayerSaveRecords[0].timeRecords[i][j].seconds = 0;
            gPlayerSaveRecords[0].timeRecords[i][j].centiseconds = 0;
            gPlayerSaveRecords[0].lapRecordIcons[i][j] = j;
            gPlayerSaveRecords[0].countRecordValues[i][j] = 0;
            gPlayerSaveRecords[0].countRecordIcons[i][j] = j;
            gPlayerSaveRecords[0].timeRecordCharacters[i][j] = j;
            gPlayerSaveRecords[0].statsRecordValues[i][j] = 0;
            gPlayerSaveRecords[0].statsRecordIcons[i][j] = j;
        }
        gPlayerSaveRecords[0].bestLapSplitTimes[i].minutes = 1;
        gPlayerSaveRecords[0].bestLapSplitTimes[i].seconds = 24;
        gPlayerSaveRecords[0].bestLapSplitTimes[i].centiseconds = 0;
        gPlayerSaveRecords[0].bestScore[i] = 0;
    }

    for (j = 0; j < 5; j++) {
        gPlayerSaveRecords[0].timeRecords[9][j] = gDefaultRaceRecordTimes[0][j];
        gPlayerSaveRecords[0].timeRecords[0][j] = gDefaultRaceRecordTimes[1][j];
        gPlayerSaveRecords[0].timeRecords[1][j] = gDefaultRaceRecordTimes[2][j];
    }

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 5; j++) {
            gPlayerSaveRecords[0].lapRecords[i][j] = gDefaultLapRecordTimes[i][j];
        }
    }

    playerCountAbsolute = 1;
    D_8013AFE0 = 0;

    for (i = 0; i < 4; i++) {
        gRacePlayers[i].unk009 = i;
        gRacePlayers[i].unk00A[0] = 0;
        gRacePlayers[i].unk00D = 0;
        gRacePlayers[i].unk00A[1] = 0;
        gRacePlayers[i].unk00A[2] = 0;
        gRacePlayers[i].money = 0;
        gRacePlayers[i].unk02A = 0;
        gRacePlayers[i].unk015[0] = 0;
        gRacePlayers[i].unk015[3] = 0;
    }

    for (i = 0; i < 15; i++) {
        gPlayerResourceTextureSelections.entries[0][i] = D_80123650[i];
        gPlayerResourceTextureSelections.entries[1][i] = D_80123650[i];
    }

    playerCountAbsolute = 1;
    D_8013AFD2 = 1;
    D_8013AF7A = 320;
    D_8013AF7C = 240;
    D_8013B10C = -1;
    D_8013B10E = -1;
    gSelectedCourseIndex = 0;
    gMainMenuSelectionIndex = 0;
    D_8013AFD6 = 0;
    D_8013B47C = 0;
    gSelectedCourseMenuItem = 0;
    gShopMenuRememberedSelection = 0;
    boardShopRememberedSelectionAbsolute = 0;
    boardMenuRememberedSelectionAbsolute = 0;
    D_8013B527 = 0;
    gRacePlayers[0].unk009 = 0;
    gRacePlayers[0].unk00A[0] = 0;
    gRacePlayers[0].unk00D = 0;
    gRacePlayers[1].unk009 = 6;
    gRacePlayers[1].unk00A[0] = 0;
    gRacePlayers[1].unk00D = 0;
    gDemoRaceIndex = 0;
    clearHalfwordPairTable();
    D_8013B106 = 0;
    D_8013B110 = 0;
}
