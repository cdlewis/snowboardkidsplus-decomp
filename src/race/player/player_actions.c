#include "player_actions_internal.h"
#include "game/engine/random.h"
#include "game/graphics/quad_animation.h"
#include "../race_internal.h"
#include "game/audio/music.h"
#include "game/engine/angles.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/io/controller.h"
#include "game/io/dat_resources.h"
#include "game/menu/selected_records.h"
#include "game/race/course/course_geometry.h"
#include "game/race/player/player_resource_textures.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_time.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

extern s16 D_8013AFCE __attribute__((section(".data")));
extern VECTOR *D_8013AF88 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));

u16 gPlayerSoundVariantsA[2] __attribute__((section(".data.player_sound_variants"))) = {0x100, 0x200};

u16 gPlayerSoundVariantsB[2] __attribute__((section(".data.player_sound_variants"))) = {0x300, 0x400};

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
        func_8002E368,
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

s32 func_8002CE58(void);
s32 func_8002CE9C(void);
s32 func_8002CEE0(void);
s32 func_8002CF24(void);
s32 func_8002CF68(void);
s32 func_8002CFAC(void);
s32 func_8002CFF0(void);
s32 func_8002D040(void);
s32 func_8002D120(void);
s32 func_8002D1B8(void);
s32 func_8002D2B4(void);
s32 func_8002D2F8(void);
s32 func_8002D33C(void);
s32 func_8002D380(void);

void getPlayerCourseProgress(u16 playerIndex, s32 *progress, s32 *distance) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];
    s32 position;

    *progress = player->unk92E * 8;
    *distance = player->coursePlacementDistance;

    /* Remap course segments onto a common progress scale for race placement. */
    switch (D_8013AFCE) {
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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002B820);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002BC2C);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002C2D8);

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
    if (D_8013AFE4 & 0x40) {
        deleteCurrentTaskSubtree();
    } else {
        if (D_8013AFE2 == 1 && D_8013AFE0 == 1) {
            D_8013B1D4 = 0;
        }
        func_8002BC2C();
        func_8002B820();
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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CE58);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CE9C);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CEE0);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CF24);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CF68);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CFAC);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CFF0);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D040);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D120);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D1B8);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D2B4);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D2F8);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D33C);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D380);

s16 processCurrentPlayerActionRequests(s16 suppressWallAction) {
    u16 pending = currentPlayerAbsolute->unk0E8;

    D_8013B0E4 = 0;
    if (pending) {
        if ((pending & 0x200) && !suppressWallAction) {
            currentPlayerAbsolute->unk0E8 = pending | 1;
        }
        if ((currentPlayerAbsolute->unk0E8 & 1) && func_8002CFF0() &&
            requestCurrentPlayerAction6() && checkCurrentPlayerFlags43040OrAngleRange()) {
            func_8002D120();
        }
        if ((currentPlayerAbsolute->unk0E8 & 2) && func_8002CFF0()) {
            requestCurrentPlayerAction6NarrowAngle();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x800) {
            requestCurrentPlayerAction7();
        }
        if ((currentPlayerAbsolute->unk0E8 & 0x400) && func_8002CEE0()) {
            func_8002CEE0();
        }
        if ((currentPlayerAbsolute->unk0E8 & 4) && func_8002D040()) {
            func_8002CE58();
        }
        if (currentPlayerAbsolute->unk0E8 & 8) {
            func_8002CE9C();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x100) {
            func_8002CF68();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x4000) {
            func_8002CFAC();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x20) {
            requestCurrentPlayerAction17();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x1000) {
            checkCurrentPlayerFlags275000OrField134();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x10) {
            func_8002D1B8();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x40) {
            func_8002D2B4();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x2000) {
            func_8002D2F8();
        }
        if (currentPlayerAbsolute->unk0E8 & 0x80) {
            func_8002CF24();
        }
        currentPlayerAbsolute->unk0E8 = 0;
    }

    if ((currentPlayerAbsolute->surfaceState.word & 0xFFFF0000) == 0x03030000 || currentPlayerAbsolute->surfaceState.fields.unk136 == 8) {
        checkCurrentPlayerFlags345001();
    }
    if (currentPlayerAbsolute->surfaceState.fields.unk136 == 6 || currentPlayerAbsolute->surfaceState.fields.unk136 == 12 || currentPlayerAbsolute->surfaceState.fields.unk136 == 16) {
        func_8002D33C();
    }
    if (currentPlayerAbsolute->surfaceState.fields.unk136 == 7 || currentPlayerAbsolute->surfaceState.fields.unk136 == 14 || currentPlayerAbsolute->surfaceState.fields.unk136 == 15) {
        func_8002D380();
    }

    if (!D_8013B0E4) {
        return 0;
    }
    if (currentPlayerAbsolute->unk128 & 0x800) {
        currentPlayerAbsolute->trickAttackMakeCount = 0;
    }

    switch ((s16)(D_8013B0E4 - 1)) {
        case 7 - 1:
            func_800183FC(D_8013AE88->data18, 0, 0x302, 0x40);
            /* fall through */
        case 6 - 1:
            if (D_8013B0E4 != 7) {
                playCurrentPlayerSoundEffect(2);
            }
            currentPlayerAbsolute->unk12C = 4;
            currentPlayerAbsolute->unk102 = D_8013B0E6;
            currentPlayerAbsolute->unk104.word = currentPlayerAbsolute->unk0EC;
            currentPlayerAbsolute->unk108.word = currentPlayerAbsolute->unk0F0;
            func_800183FC(D_8013AE88->data18, 1, 0x201, 0x20);
            D_8013AF88->vx = currentPlayerAbsolute->worldCoordinate.coord.t[0];
            D_8013AF88->vy = currentPlayerAbsolute->worldCoordinate.coord.t[1] - 48;
            D_8013AF88->vz = currentPlayerAbsolute->worldCoordinate.coord.t[2];
            startAlternateQuadAnimationTask(D_8013AE88->data18, (const PositionEffectOrigin *)D_8013AF88);
            break;
        case 9 - 1:
            playCurrentPlayerSoundEffect(2);
            currentPlayerAbsolute->unk12C = 14;
            func_800183FC(D_8013AE88->data18, 1, 0x201, 0x20);
            D_8013AF88->vx = currentPlayerAbsolute->worldCoordinate.coord.t[0];
            D_8013AF88->vy = currentPlayerAbsolute->worldCoordinate.coord.t[1] - 48;
            D_8013AF88->vz = currentPlayerAbsolute->worldCoordinate.coord.t[2];
            startAlternateQuadAnimationTask(D_8013AE88->data18, (const PositionEffectOrigin *)D_8013AF88);
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
            func_800183FC(D_8013AE88->data18, 1, 0x201, 0x20);
            D_8013AF88->vx = currentPlayerAbsolute->worldCoordinate.coord.t[0];
            D_8013AF88->vy = currentPlayerAbsolute->worldCoordinate.coord.t[1] - 48;
            D_8013AF88->vz = currentPlayerAbsolute->worldCoordinate.coord.t[2];
            startAlternateQuadAnimationTask(D_8013AE88->data18, (const PositionEffectOrigin *)D_8013AF88);
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
    if (currentPlayerAbsolute->unk008) {
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
    RacePlayer *player = currentPlayerAbsolute;

    task->data1A = 0;
    task->data1C = 0;
    task->data1E = 0;
    player->unk0C2 = 0;
}

void dispatchCurrentPlayerDirectionalAnimation(void) {
    gCurrentPlayerDirectionalAnimationCallbacks[D_8013AE88->data1A]();
}

void startCurrentPlayerDirectionalAnimation(void) {
    RacePlayer *player = currentPlayerAbsolute;
    player->unk0C4 = 0;
    player->unk0C6 = 0;
    if (!player->unk008) {
        if (D_8013AEE8[D_8013AE88->data18] & 8) {
            D_8013AE88->data1C = 8;
            player->unk0C4 = 1;
        }
        if (D_8013AEE8[D_8013AE88->data18] & 4) {
            D_8013AE88->data1C = 4;
            currentPlayerAbsolute->unk0C4 = 3;
        }
        if (D_8013AEE8[D_8013AE88->data18] & 1) {
            D_8013AE88->data1C = 1;
            currentPlayerAbsolute->unk0C4 = 7;
        }
        if (D_8013AEE8[D_8013AE88->data18] & 2) {
            D_8013AE88->data1C = 2;
            currentPlayerAbsolute->unk0C4 = 5;
        }
        if (D_8013AE88->data1C) {
            if ((s16)currentPlayerAbsolute->unk0C2 < 6) {
                currentPlayerAbsolute->unk0C4++;
            }
            D_8013AE88->data1A++;
            if (currentPlayerAbsolute->unk128 & 0x400) {
                if (D_8013AE88->data1C & 8) {
                    startCurrentPlayerAnimation(0x1C, 2);
                }
                if (D_8013AE88->data1C & 4) {
                    startCurrentPlayerAnimation(0x1A, 2);
                }
            } else {
                if (D_8013AE88->data1C & 8) {
                    startCurrentPlayerAnimation(0x1A, 2);
                }
                if (D_8013AE88->data1C & 4) {
                    startCurrentPlayerAnimation(0x1C, 2);
                }
            }
            if (D_8013AE88->data1C & 1) {
                startCurrentPlayerAnimation(0x1E, 2);
            }
            if (D_8013AE88->data1C & 2) {
                startCurrentPlayerAnimation(0x20, 2);
            }
            currentPlayerAbsolute->unk128 |= 0x800;
        }
        currentPlayerAbsolute->unk0C2++;
    }
}

void updateCurrentPlayerDirectionalAnimation(void) {
    GameTask *task = D_8013AE88;
    RacePlayer *player;
    if (!(D_8013AEEC[task->data18] & task->data1C)) {
        task->data1E = 1;
    }
    player = currentPlayerAbsolute;
    if (player->animationFlags & 0x10) {
        GameTask *animationTask = D_8013AE88;
        if (animationTask->data1E) {
            animationTask->data1A++;
            if (player->unk128 & 0x400) {
                if (animationTask->data1C & 8) {
                    startCurrentPlayerAnimation(0x1D, 2);
                }
                if (D_8013AE88->data1C & 4) {
                    startCurrentPlayerAnimation(0x1B, 2);
                }
            } else {
                if (animationTask->data1C & 8) {
                    startCurrentPlayerAnimation(0x1B, 2);
                }
                if (D_8013AE88->data1C & 4) {
                    startCurrentPlayerAnimation(0x1D, 2);
                }
            }
            if (D_8013AE88->data1C & 1) {
                startCurrentPlayerAnimation(0x1F, 2);
            }
            if (D_8013AE88->data1C & 2) {
                startCurrentPlayerAnimation(0x21, 2);
            }
        } else {
            player->unk0C6++;
        }
    }
    currentPlayerAbsolute->unk128 |= 0x800;
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002E368);

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
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, 0, 64);
        break;
    case 1:
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, gPlayerSoundVariantsA[randomNextMain() & 1], 64);
        break;
    case 2:
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, gPlayerSoundVariantsB[randomNextMain() & 1], 64);
        break;
    case 3:
        func_8001890C(D_8013AE88->data18, D_8013AE88->data18 + 2, gPlayerSoundVariantsB[randomNextMain() & 1], 64);
        break;
    case 4:
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, 0x500, 64);
        break;
    case 5:
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, 0x600, 64);
        break;
    case 6:
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, 0x700, 64);
        break;
    case 7:
        playPlayerPositionalSoundEffect(D_8013AE88->data18, D_8013AE88->data18 + 2, 0x800, 64);
        break;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002E5A0);

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


void updateCurrentPlayerProjectileUse(u16 playerIndex) {
    s32 activate = 0;

    if (!currentPlayerAbsolute->unk008) {
        activate = ((u8)D_8013AEE8[D_8013AE88->data18]) >> 7;
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
            func_80014930(&gGameTasks[4], initializePlayerDualResourceEffectTask);
            break;
        case 1:
            func_80014930(&gGameTasks[4], initializePlayerVisualPairTask);
            break;
        case 2:
            func_80014930(&gGameTasks[4], initializePlayerVisualPairTask5E80);
            break;
        case 3:
            func_80014930(&gGameTasks[4], initializePlayerVisualPairTask6780);
            break;
        case 4: {
            u8 bounce = 0;
            if (!currentPlayerAbsolute->unk008) {
                bounce = (D_8013AEEC[playerIndex] & 0xE000) == 0x4000;
            } else {
                s32 roll = randomNextSecondary();
                if (gPlayerProjectileBounceChoices[currentPlayerAbsolute->unk935][roll & 7]) {
                    bounce = 1;
                }
            }
            if (bounce) {
                func_80014930(&gGameTasks[4], initializeBouncingTaskVisual);
            } else {
                func_80014930(&gGameTasks[4], initializeDualResource4C80EffectTask);
            }
            break;
        }
        default:
            goto clearEmptyEffect;
        }
        D_8013AE8C->data18 = playerIndex;
        currentPlayerAbsolute->unk940--;

    clearEmptyEffect:
        if (!currentPlayerAbsolute->unk940) {
            currentPlayerAbsolute->unk93E = 0;
        }
    }

    if (currentPlayerAbsolute->unk93C) {
        func_80014930(&gGameTasks[4], initializeDualResource5100EffectTask);
        D_8013AE8C->data18 = playerIndex;
        currentPlayerAbsolute->unk93C--;
    }
}

void updateCurrentPlayerItemUse(u16 playerIndex) {
    u16 activate = 0;

    if (!currentPlayerAbsolute->unk008) {
        activate = (D_8013AEE8[playerIndex] >> 5) & 1;
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
        func_8002E5A0(playerIndex, 13);
        goto clearItem;
    case 1:
        func_8001496C(&gGameTasks[4], initializeOrbitingPlayerVisualTask);
        D_8013AE8C->data18 = playerIndex;
        currentPlayerAbsolute->heldItem = 0;
        return;
    case 2: {
        u16 count = 0;
        u16 index;
        for (index = 0; index < D_8013AFD2; index++) {
            if (index != playerIndex) {
                s32 delay;
                func_80014930(&gGameTasks[4], initializeDelayedPlayerRiseEffectTask);
                delay = count * 3;
                count++;
                D_8013AE8C->data18 = index;
                D_8013AE8C->data1A = playerIndex;
                D_8013AE8C->data1C = delay;
            }
        }
        goto clearItem;
    }
    case 3:
        func_8001496C(&gGameTasks[4], initializeFallingTaskObject);
        D_8013AE8C->data18 = playerIndex;
        func_800183FC(D_8013AE88->data18, 1, 2, 32);
        goto clearItem;
    case 4:
        if (!currentPlayerAbsolute->unk008) {
            func_8001496C(&gGameTasks[4], initializeSlidingVisualSequenceTask);
            D_8013AE8C->data18 = playerIndex;
        }
    clearItem:
        currentPlayerAbsolute->heldItem = 0;
        return;
    case 5: {
        RacePlayer *player = currentPlayerAbsolute;
        if (player->surfaceState.fields.unk134 == 0) {
            player->heldItem = 0;
            player->surfaceState.fields.unk134 = 180;
            func_800183FC(D_8013AE88->data18, 1, 0xE00, 32);
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
    u16 player = D_8013AE88->data18;
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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002EE0C);
