#include "common.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/race_player.h"
#include "psx/libsnd.h"

typedef struct SavedImageEntry {
    u32 flags;
    u8 r, g, b, unk07;
    RECT rect;
    u16 pixels[32];
} SavedImageEntry;

SavedImageEntry gSavedImageEntries[32] __attribute__((section(".data.saved_image_entries"))) = {0};
s32 gLinearRandomState = 1;

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80015EE0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80015FC4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001604C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016244);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001628C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800162C8);

void releaseSavedImageEntry(u16 index) {
    if (index != 0xFF) {
        gSavedImageEntries[index].flags = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016344);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800163E4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016420);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016470);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800164E8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800165EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016628);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016A7C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016CBC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016E80);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016F5C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017268);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001758C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017BAC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017C20);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017D20);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017DD0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017E38);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017E7C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017ED4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017F44);

void stopMusicSequence(void) {
    if (gMusicSequenceActive)
        SsSeqStop(gMusicSequenceId);
}

void pauseMusicSequence(void) {
    SsSeqPause(gMusicSequenceId);
}

void resumeMusicSequence(void) {
    SsSeqReplay(gMusicSequenceId);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017FD8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018084);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800180C8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800180F4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001814C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001816C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800181B0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018288);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018358);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800183FC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018538);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018624);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018720);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800187B8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018804);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001890C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018A34);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018AA4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018BA4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018D80);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018EC4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018F58);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018FBC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001907C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019244);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019288);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019520);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019670);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001983C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198B8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198C4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001993C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019980);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019A2C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019A70);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019AF4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019B54);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019B94);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019C40);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019C84);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019CB8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019D3C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A018);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A080);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A0EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A22C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A2B8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A3DC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A474);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A4E8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A540);

void func_8001AB34(void) {
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001AB3C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001AE40);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001AE98);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001AF64);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B450);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B4EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B778);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B7F0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B858);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001BD50);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C0A8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C0DC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C128);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C2E0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C85C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C9F0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CD00);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CD64);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CE2C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CE74);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CEAC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CF18);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CFD0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D044);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D100);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D42C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D4C8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D528);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D678);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D89C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D8D4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D97C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001DB7C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001DFDC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E028);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E0C0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E0F4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E148);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E17C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E1B8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E1EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E244);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E3E4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E484);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E524);

void clearCurrentPlayerSequenceState(void) {
    s32 i;
    for (i = 0; i < 6; i++) {
        D_8013B0D8->unk0CA[i] = 0;
        D_8013B0D8->unk0D6[i] = 0;
    }
}
