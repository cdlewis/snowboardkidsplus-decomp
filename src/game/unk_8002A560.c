#include "common.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/selected_record.h"
#include "game/selected_records.h"

void copySelectedRecordToCurrentPlayer(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk116 = D_8013B1F0[D_8013B1FE].unk8;
    player->unk11A = D_8013B1F0[D_8013B1FE].unkA;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002A5A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002A6AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002A7B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002A8E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002AE38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002AE6C);
