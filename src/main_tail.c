#include "common.h"
#include "game/game_options.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/text_entries.h"
#include "game/task.h"

typedef struct UnlockMenuState {
    s16 selection, previousSelection;
    s32 unk04[7];
    s32 unk20[7];
    s32 unk3C[7];
    s32 unk58[7];
    s16 unk74[4];
    u8 unk7C[0x84];
} UnlockMenuState;

extern u8 D_8013B55E __attribute__((section(".data")));

void func_800A39AC(void);
void func_800A42CC(void);
void func_800A499C(void);
void func_8009B5A0(s16 playerIndex, s32 unused, s32 paletteRow, u16 mode);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C708);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008C8BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008CB88);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008CD98);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D144);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D200);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008D958);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008DD04);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008DDC0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008E66C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008EB94);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008EC50);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008F474);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008F970);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8008FA8C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800903C4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80090450);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80091318);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800916F4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80091B80);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80091FAC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80092090);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800929C4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80092A50);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800937BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80093A20);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80093A54);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009448C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80094564);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80094760);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009488C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095674);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095A58);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095E24);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095EB4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80095F30);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800960BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096238);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800964B0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096590);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096AB0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80096EC8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800972B8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009747C);

void setCurrentPlayerFinalPartTranslations(const s16 *values) {
    RacePlayer *player = D_8013B0D8;
    s32 index = player->modelPartCount - 2;
    player->unk62C[index].x = *values++;
    player->unk62C[index].y = *values++;
    player->unk62C[index].z = *values++;
    index = player->modelPartCount - 1;
    player->unk62C[index].x = *values++;
    player->unk62C[index].y = *values++;
    player->unk62C[index].z = *values++;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009766C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097858);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097C2C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097EBC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80097FF4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80098D68);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80098F38);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099010);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009919C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099318);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800995A8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800996F8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009974C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_80099880);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009A560);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009A87C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009ADAC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009AEC0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B0E8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B2D8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B5A0);

void copyPlayerPaletteRow(s16 playerIndex) {
    func_8009B5A0(playerIndex, D_80120E90[playerIndex].unk009,
                  D_80120E90[playerIndex].unk00D, 0);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009B700);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BB68);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BCFC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009BDB8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009C568);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009C9A0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009CA90);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009D29C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009D354);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009DB70);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009DEF0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009E468);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009E848);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009ECCC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F158);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F224);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F900);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_8009F9B8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A008C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A046C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A050C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A05C0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A09FC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0A4C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0AA8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0C6C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0D10);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0D54);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0E28);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0EE8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0F14);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A0FDC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A12E4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1510);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1604);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1670);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1774);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1DA0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1ECC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A1F50);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2210);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2324);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2460);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A24B0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A24EC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2510);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A25F8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A29A4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2AB0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2C40);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2C74);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A2D68);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3028);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A31C0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3750);

void advanceTaskWhenFlagB55ESet(void) {
    if (D_8013B55E != 0) {
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(func_800A39AC);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A39AC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3AA4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3C3C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A3D9C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A42CC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A46E0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A48E4);

void resetUnlockMenuOffsets(UnlockMenuState *state) {
    s16 i;
    for (i = 0; i < 6; i++) {
        i[state->unk3C] = 0;
        i[state->unk04] = 0;
        i[state->unk58] = 0;
        i[state->unk20] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A499C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A4C28);

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A4FB8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5050);

void resumeUnlockSelectionTask(void) {
    func_800719A8(0);
    setCurrentTaskCallback(func_800A499C);
}

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5204);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A525C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A528C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A52D8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A53BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5418);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A55A0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5630);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A59B8);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5AFC);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5E0C);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5F00);

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

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5F90);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A5FE8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A60FC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A61E0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A624C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6508);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6548);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6684);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A68B4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6A28);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6A8C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6CC4);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6DD0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A6E88);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A73BC);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7590);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7664);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A77B0);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7928);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7B0C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7B9C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7C8C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7CE8);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7D8C);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A7F84);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A8044);

INCLUDE_ASM("asm/nonmatchings/main_tail", func_800A8104);
