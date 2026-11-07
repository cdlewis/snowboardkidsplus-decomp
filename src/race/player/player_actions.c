#include "player_actions_internal.h"

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

RaceTime gRaceElapsedTime = {0, 0, 0};

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002B3F4);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002BF88);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002C2D8);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002C520);

u16 isPositionWithinPlayerCollisionRange(const Position3D *position, s32 radius, s32 height, s16 playerIndex) {
    RacePlayer *player;
    RacePlayer *players;
    s32 x, y, z, range, verticalRange;
    players = D_80120E90;
    player = &players[playerIndex];
    if (player->isActive) {
        if (arePositionsWithinXZRange(position, (Position3D *)&D_80120E90[playerIndex].x, 0x10000)) {
            y = position->y - player->unk074;
            verticalRange = height;
            if (y < 0) {
                y = -y;
            } else {
                verticalRange = player->unk0B0;
            }
            if (y < verticalRange) {
                RacePlayer *horizontalPlayer;
                RacePlayer *horizontalPlayers = D_80120E90;
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
    if (D_80120E90[playerIndex].isActive) {
        if (flags != 4 && D_80120E90[playerIndex].unk134 != 0) {
            return 0;
        }
        players = D_80120E90;
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
        player = &D_80120E90[i];
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
        func_800149A8();
    } else {
        if (D_8013AFE2 == 1 && D_8013AFE0 == 1) {
            D_8013B1D4 = 0;
        }
        func_8002BC2C();
        func_8002B820();
        for (i = 0; i < D_8013AFD2; i++) {
            D_8013B0D8 = &D_80120E90[i];
            assignCurrentPlayerFinishSlot(i);
        }
    }
}

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CE58);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CE9C);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CEE0);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CF24);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CF68);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CFAC);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002CFF0);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D040);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D120);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D1B8);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D2B4);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D2F8);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D33C);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D380);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002D3C4);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002DEA4);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002DEE8);

void updateCurrentPlayerDirectionalAnimation(void) {
    GameTask *task = D_8013AE88;
    RacePlayer *player;
    if (!(D_8013AEEC[task->data18] & task->data1C)) {
        task->data1E = 1;
    }
    player = D_8013B0D8;
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
    D_8013B0D8->unk128 |= 0x800;
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002E368);

void func_8002E3C0(void) {
}

void addPlayerCappedCounters(u16 playerIndex, s32 amount) {
    RacePlayer *players = D_80120E90;
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


INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002E644);

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002EA0C);

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

INCLUDE_ASM("asm/nonmatchings/race/player/player_actions", func_8002EE0C);
