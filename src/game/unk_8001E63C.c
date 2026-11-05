#include "common.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/selected_record.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/selected_records.h"
#include "psx/libsnd.h"


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


INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001E63C);

u8 readSelectedByteTable(s32 index) {
    return D_8012090C[D_8013AFCE][index];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001EC88);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001EEBC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F0C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F130);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F1B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F258);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F2D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F388);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F444);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F528);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F6D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F6F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F718);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F7A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F874);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F8E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F970);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FA38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FAC4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FC00);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FCB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FD70);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FDEC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FEE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FFA8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80020018);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800200F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80020328);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80020380);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800204D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800205B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80021258);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80021B20);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800240D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024288);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800245C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024804);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002497C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024F10);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025014);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800252CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025508);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800255C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800257AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025840);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025A38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025B08);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025DD8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025FAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026010);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800260E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026500);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800266A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026754);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026AA8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026D10);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026DAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80027014);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800271D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800273D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80027624);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80027918);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80028234);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80028680);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80028C80);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029168);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029208);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800293F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029C30);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029DD0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029E6C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029EE4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029FE4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A3A4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A454);

void copySelectedRecordToCurrentPlayer(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk116 = D_8013B1F0[D_8013B1FE].unk8;
    player->unk11A = D_8013B1F0[D_8013B1FE].unkA;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A5A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A6AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A7B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A8E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002AE38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002AE6C);
