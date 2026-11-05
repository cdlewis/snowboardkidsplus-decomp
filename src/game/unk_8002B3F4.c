#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/race_player.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/game_options.h"
#include "psx/libsnd.h"

typedef struct Position3D {
    s32 x, y, z;
} Position3D;

typedef struct UnpackedWordBytes {
    u8 unk0;
    u8 unk1;
    u16 unk2;
} UnpackedWordBytes;


extern s16 D_8013B0E4;
extern u8 D_8013AFCD __attribute__((section(".data")));

void func_8003E60C(u16 index);
void func_8003EE70(void);
void func_8002F5F8(void);
void func_80032F84(void);
void func_80033404(void);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CC78);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CD18);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CDB8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CE58);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CE9C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CEE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CF24);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CF68);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CFAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002CFF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D040);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D090);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002D1FC);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002DD70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002DDF4);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E5D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002E644);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002EA0C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002ED00);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002EE0C);

void resetGameOptionState(void) {
    D_8013B108 = 2;
    D_8013B109 = 0;
    D_8013B10A = 0;
    gGameOptionFlags = 0x3E;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F1C8);

void uploadBufferedTexturePair(void) {
    func_80015EE0((u32 *)0x80187800, NULL);
    func_80015EE0((u32 *)0x80181000, NULL);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F29C);

void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map) {
    func_80071300(mode, unused, background, map,
                  (u8 *)0x80180000, (u8 *)0x80180800);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F348);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F394);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F3C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F468);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F548);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002F584);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002FF64);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8002FFC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80030108);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003012C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80030270);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800302F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80030974);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032B78);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032C14);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032D48);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032E44);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032E94);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032F3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80032F84);

void beginSound601TaskSequence(void) {
    func_800181B0(0, 0x601, 0x20);
    setCurrentTaskCallback(func_80032F84);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033100);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033280);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033700);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033794);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033820);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800338BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033974);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033A24);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033B5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033C2C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033F00);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033FAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80033FE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034028);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034090);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800341DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034270);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800342FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800343B0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034434);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003452C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800345BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034664);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034928);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800349BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034A48);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034AE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034BC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034C40);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034DFC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034E90);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034F1C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80034FBC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003504C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800350AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003527C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035310);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003539C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003543C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035528);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800355D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800356FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035808);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003597C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035A38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035B24);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035C18);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035CF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035D5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035EDC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80035FAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800360F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800361C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800362FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800363A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036454);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800366C0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036790);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003686C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036940);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036A0C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036AB8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80036BA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037074);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037354);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037424);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800375B0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003763C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003772C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800377E0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037850);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800379B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037BF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037C80);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037D18);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037D90);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037E34);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037EC8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80037F74);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038010);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038090);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038270);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038574);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003881C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038A28);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038A98);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038B34);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038C70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038E8C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038F2C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80038FD4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003908C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039144);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039210);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800392BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039360);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039484);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039564);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800398F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039B34);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039EE8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80039F6C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A008);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A094);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A158);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A3D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A478);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A520);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A5D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A690);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A760);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A814);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A8C0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003A96C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003AAE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003AD48);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B020);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B2BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B434);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B4B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B554);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B5CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B690);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B8AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B94C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003B9F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BAAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BB64);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BC18);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BCC4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BD70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003BE58);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C55C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C5D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C69C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C74C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C7F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003C9A4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003CA58);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003CAF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003D904);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003D940);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DB94);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DE64);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DEE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003DF3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E050);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E1D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E360);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E3F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E55C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E60C);

void initPlayerZeroTask(void) {
    func_8003E60C(0);
    setCurrentTaskCallback(func_8003EE70);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003E794);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003EA20);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003ED6C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8003EE70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004002C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800400C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80040140);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800401DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004023C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800402C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800403C8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800439AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043A4C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043ACC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043C28);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043C5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043F84);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80043FBC);

s16 isFirstPlayerUsingCallback26D10(void) {
    return D_80120E90[0].task->callback == func_80026D10;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800442F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004449C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044524);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004463C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044768);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044990);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044A38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044BC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044DC4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044E38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80044EC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800450BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004531C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800456DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80045868);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800458F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80045B4C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80045C7C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80045F8C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046044);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046228);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046404);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046708);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046894);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046AEC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046BEC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046CB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046E30);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80046EF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047074);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004713C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800472B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047380);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047538);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047600);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_8004777C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047A3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047CC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80047D94);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800481A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048340);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048668);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048714);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800488B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800489A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80048AA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80049168);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80049240);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_80049404);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002B3F4", func_800496EC);
