#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "psx/libsnd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libmcrd.h"

typedef struct PaletteRecordColor {
    u8 r, g, b, unk3;
} PaletteRecordColor;

typedef struct SavedImageEntry {
    u32 flags;
    PaletteRecordColor color;
    RECT rect;
    u16 pixels[32];
} SavedImageEntry;

u8 D_80120200[16] __attribute__((section(".data.player_model_parts"))) = {
    255, 0, 1, 2, 1, 4, 1, 6, 6, 8, 6, 10, 0, 0, 0, 0
};

u8 D_80120210[16] __attribute__((section(".data.player_model_parts"))) = {
    0, 1, 4, 5, 2, 3, 6, 7, 10, 11, 8, 9, 0, 0, 0, 0
};

s16 D_80120220[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 16, 0, -12, 12, 0, 0, -17, 0, 12, 12, 0, 0, -17, 0, -15, 11, 0, 15, 11, 0
};

s16 D_80120274[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 14, 0, -10, 12, 0, 0, -17, 0, 10, 12, 0, 0, -17, 0, -15, 11, 0, 15, 11, 0
};

s16 D_801202C8[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 50, 0, -8, -8, 0, 0, -16, 0, 8, -8, 0, 0, -16, 0, 0, 5, 0, 0, 31, 0, -24, 24, 0, 0, -28, 0, 24, 24, 0, 0, -28, 0, -17, 13, 0, 17, 13, 0
};

s16 D_8012031C[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 16, 0, -12, 12, 0, 0, -17, 0, 12, 12, 0, 0, -17, 0, -10, 11, 0, 10, 11, 0
};

s16 D_80120370[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 15, 0, -10, 12, 0, 0, -17, 0, 10, 12, 0, 0, -17, 0, -15, 11, 0, 15, 11, 0
};

s16 *D_801203C4[10] __attribute__((section(".data.player_model_parts"))) = {
    D_80120220, D_80120274, D_80120220, D_80120370, D_801202C8, D_80120220, D_80120220, D_801202C8, D_8012031C, D_8012031C
};

SavedImageEntry gSavedImageEntries[32] __attribute__((section(".data.saved_image_entries"))) = {0};
s8 gSoundSystemActive = 0;
u8 D_8013AF29 = 0;
s16 gMusicSequenceId = 0;
s32 gLinearRandomState __attribute__((section(".data.linear_random_state"))) = 1;
typedef struct MovieDecodeState {
    u32 *vlcBuffers[2];
    s32 vlcBufferIndex;
    u32 *imageBuffers[2];
    s32 imageBufferIndex;
    RECT frames[2];
    s32 frameIndex;
    RECT slice;
    s32 outputComplete;
} MovieDecodeState;


extern s32 D_8013AF34;
extern s16 D_8013AF38;
extern char D_8013AF3C[];
extern s32 D_8013B600[];
extern struct DIRENTRY D_8013B700[][15];
extern u16 D_8013AEE8 __attribute__((section(".data")));
extern s16 D_8013AF3E;
extern u16 D_8013AFAA __attribute__((section(".data")));
extern u8 D_8013AFD6;
extern u16 D_8013AFE8;
/* Two state values; an incomplete array preserves absolute addressing. */
extern u16 D_8013AEEC[];

void func_8001D044(void);
void func_8001D42C(void);
void func_8001C0DC(void);
void func_8001B7F0(void);
void func_80019A70(void);
void func_8001E0C0(void);
void func_80018AA4(void);
void func_80019B94(void);
void func_8001E148(void);
void func_8001E1B8(void);
void func_800180F4(s16 bankId);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80015EE0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80015FC4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001604C);

void uploadImage64x256(u32 *pixels, s16 x, s16 y) {
    RECT rect;
    rect.x = x;
    rect.y = y;
    rect.w = 64;
    rect.h = 256;
    LoadImage(&rect, pixels);
    DrawSync(0);
}

void resetPaletteRecords(void) {
    u16 i;
    for (i = 0; i < 32; i++) {
        gSavedImageEntries[i].flags = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800162C8);

void releaseSavedImageEntry(u16 index) {
    if (index != 0xFF) {
        gSavedImageEntries[index].flags = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016344);

void readVram16Pixels(s32 x, s32 y, u32 *pixels) {
    RECT rect;
    rect.x = x;
    rect.y = y;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, pixels);
}

void setPaletteRecordColor(u16 index, u8 r, u8 g, u8 b) {
    PaletteRecordColor *color;
    if (index != 32) {
        color = &gSavedImageEntries[index].color;
        color->r = r;
        color->g = g;
        color->b = b;
        gSavedImageEntries[index].flags |= 4;
    }
}

void startPaletteCycle(u16 direction, u16 index, u8 first, u8 last, u8 interval) {
    PaletteRecordColor *color;
    if (index != 32) {
        color = &gSavedImageEntries[index].color;
        color->r = first;
        color->g = last;
        color->unk3 = interval;
        color->b = interval;
        if (direction == 0) {
            gSavedImageEntries[index].flags |= 0xA;
        } else {
            gSavedImageEntries[index].flags |= 0x12;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800164E8);

void requestSavedImageRestore(u16 index) {
    if (index != 32) {
        gSavedImageEntries[index].flags |= 0x40;
    }
}

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

void closeMusicSequence(void) {
    if (gMusicSequenceActive) {
        SsSeqStop(gMusicSequenceId);
        SsSeqClose(gMusicSequenceId);
        func_800180F4(2);
        gMusicSequenceActive = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017E7C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017ED4);

void fadeOutMusicSequence(s16 step) {
    if (gMusicSequenceActive) {
        gMusicFadeMode = 2;
        gMusicFadeStep = step;
    }
}

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

void endSoundSystem(void) {
    if (gSoundSystemActive) {
        SsEnd();
        gSoundSystemActive = 0;
    } else {
        gSoundSystemActive = 0;
    }
}

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

void readMemoryCardDirectory(void) {
    MemCardGetDirentry(D_8013AF34, D_8013AF3C, D_8013B700[D_8013AF38],
                       &D_8013B600[D_8013AF38], 0, 15);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018FBC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001907C);

s32 tryAcceptMemoryCard(void) {
    if (MemCardSync(1, 0, 0) == -1) {
        MemCardAccept(D_8013AF34);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019288);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019520);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019670);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001983C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198B8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198C4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001993C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019980);

s32 startMemoryCardReadTask(void) {
    if (D_8013AF3E == 5) {
        func_80014874(func_80019A70);
        D_8013AF3E = 3;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019A70);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019AF4);

void waitForMemoryCardAcceptTask(void) {
    if (tryAcceptMemoryCard() == 1) {
        func_80018AA4();
        setCurrentTaskCallback(func_80019B94);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019B94);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019C40);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019C84);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019CB8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019D3C);

void initMovieDecodeState(MovieDecodeState *state, s16 x0, s16 y0, s16 x1, s16 y1) {
    state->vlcBuffers[0] = (u32 *)0x801A1000;
    state->vlcBuffers[1] = (u32 *)0x801C9000;
    state->vlcBufferIndex = 0;
    state->imageBuffers[0] = (u32 *)0x801F1000;
    state->imageBuffers[1] = (u32 *)0x801F4000;
    state->imageBufferIndex = 0;
    state->frames[0].x = x0;
    state->frames[0].y = y0;
    state->frames[1].x = x1;
    state->frameIndex = 0;
    state->slice.x = x0;
    state->slice.y = y0;
    state->slice.w = 24;
    state->outputComplete = 0;
    state->frames[1].y = y1;
}

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

void initCurrentPlayerModelParts(void) {
    s32 i;
    s16 *values;
    RacePlayer *player;
    for (i = 0; i < 14; i++) {
        D_8013B0D8->unk13E[i] = D_80120200[i];
        D_8013B0D8->unk14C[i] = D_80120210[i];
    }
    values = D_801203C4[D_8013B0D8->unk014];
    player = D_8013B0D8;
    for (i = 0; i < 14; i++) {
        player->unk5BC[i].x = player->unk5BC[i].y = player->unk5BC[i].z = 0;
        player->unk62C[i].x = *values++;
        player->unk62C[i].y = -*values++;
        player->unk62C[i].z = -*values++;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001AF64);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B450);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B4EC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B778);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B7F0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B858);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001BD50);

void beginTask60TickCountdown(void) {
    D_8013AE88->data1A = 60;
    setCurrentTaskCallback(func_8001C0DC);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C0DC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C128);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C2E0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C85C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001C9F0);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CD00);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CD64);

void advanceTaskWhenFlagAFE4Bit8Set(void) {
    if (D_8013AFE4 & 0x100) {
        D_8013AFE4 = 0;
        D_8013AFAA |= 0x90;
        setCurrentTaskCallback(func_8001B7F0);
    }
}

void startResourceLoadWhenFlagAFE4Bit9Set(void) {
    if (D_8013AFE4 & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CEAC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CF18);

void startTaskTransitionWhenFlagAEE8Set(void) {
    if (D_8013AE88->data18 == 0) {
        if (D_8013AEE8 & 0xB0) {
            D_8013AE88->data18 = 30;
            func_80070274(2, 0, 30);
            setCurrentTaskCallback(func_8001D044);
        }
    } else {
        D_8013AE88->data18--;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D044);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D100);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D42C);

void startThreeFrameScreenBlend(void) {
    func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    D_8013AE88->data18 = 3;
    setCurrentTaskCallback(func_8001D42C);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D528);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D678);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D89C);

void animateAlternatingVramRow(void) {
    RECT rect;
    if (--D_8013AE88->data1A == 0) {
        rect.x = 640;
        if (D_8013AE88->data18 == 0) {
            rect.y = 503;
            D_8013AE88->data1A = 4;
        } else {
            rect.y = 504;
            D_8013AE88->data1A = 6;
        }
        rect.w = 16;
        rect.h = 1;
        MoveImage(&rect, 640, 502);
        D_8013AE88->data18 ^= 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001D97C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001DB7C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001DFDC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E028);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E0C0);

void processTaskWhenFlagAFAABit7Set(void) {
    if (D_8013AFAA & 0x80) {
        func_800149E0();
        D_8013AFAA &= ~0xC0;
        setCurrentTaskCallback(func_8001E0C0);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E148);

void resetTaskCallbackOnFlag4(void) {
    if (D_8013AFAA & 0x10) {
        func_800149E0();
        setCurrentTaskCallback(func_8001E148);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E1B8);

void restoreSavedStateAndHandleFlag4(void) {
    if (D_8013AFD6) {
        D_8013AEEC[0] = D_8013AFE8;
    }
    if (D_8013AFAA & 0x10) {
        D_8013AFAA &= ~0x18;
        setCurrentTaskCallback(func_8001E1B8);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E244);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E3E4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E484);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E524);
