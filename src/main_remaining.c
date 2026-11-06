#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/graphics.h"
#include "game/coordinate.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/cd_queue.h"
#include "game/controller.h"
#include "game/race_player.h"
#include "game/game_options.h"
#include "psx/libsnd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"
#include "psx/libmcrd.h"

#define PLAYER_ANIMATION_ARCHIVE ((u8 *)0x80180000)
#define PLAYER_ANIMATION_OFFSETS ((u32 *)0x80180004)

/* The first word holds the count; later words hold tagged byte offsets.
 * words includes that header word, so the first image uses word index 1.
 */
typedef union TimArchive {
    u16 count;
    s32 words[1];
} TimArchive;

typedef struct MemoryCardSaveHeader {
    u8 magic[2];
    u8 iconDisplay;
    u8 blockCount;
    u8 title[64];
    u8 reserved[28];
    u16 iconPalette[16];
    u8 icons[3][128];
    u32 version;
    u32 payloadSize;
    u32 checksum;
    u32 unk20C;
    u8 payload[0x870];
} MemoryCardSaveHeader;

typedef struct PaletteRecordColor {
    u8 r, g, b, unk3;
} PaletteRecordColor;

typedef struct SoundVoiceRecord {
    u8 owner;
    u8 bank;
    u16 note;
} SoundVoiceRecord;

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
SoundVoiceRecord gSoundVoiceRecords[8] __attribute__((section(".data.sound_voice_records"))) = {0};

MemoryCardSaveHeader gMemoryCardSaveHeader __attribute__((section(".data.memory_card_save_header"))) = {0};

SavedImageEntry gSavedImageEntries[32] __attribute__((section(".data.saved_image_entries"))) = {0};
u32 gSynchronizedFrameCount __attribute__((section(".sdata.synchronized_frame_count"))) = 0;
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
extern s16 D_8013AF3E;
extern u16 D_8013AFAA __attribute__((section(".data")));
extern u8 D_8013AFD6;
extern u8 D_80120EA4[];
extern u8 D_8013AFE2;
extern u16 D_8013AFE8;
extern void *D_8013AF4C;
extern void *gMemoryCardLoadDestination;

void func_8001D044(void);
void func_8001D42C(void);
void uploadTimArchive(TimArchive *archive, s16 *position);
void *memcpy(void *destination, const void *source, unsigned int size);
u16 func_800162C8(void);
s32 func_80017268(void);
void func_80018D80(u8 slot, void *destination);
void func_80019980(void);
void func_8001C0DC(void);
void func_8001B7F0(void);
void func_80019A70(void);
void func_8001E0C0(void);
void func_80018AA4(void);
void func_80019B94(void);
void func_8001D100(void);
void advanceTaskWhenFlagAFE4Bit8Set(void);
void startResourceLoadWhenFlagAFE4Bit9Set(void);
void func_8001E148(void);
void func_8001E1B8(void);
void func_800180F4(s16 bankId);
void func_80040A60(void);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80015EE0);

void uploadTimArchive(TimArchive *archive, s16 *position) {
    u16 i = 0;
    u32 count = archive->count;
    /* Keep the archive index signed, as in the on-disc table lookup. */
    while (1) {
        s32 offset;
        s32 index;
        if (i >= count) {
            break;
        }
        index = (u16)i + 1;
        offset = archive->words[(s16)index];
        func_80015EE0((u32 *)((u8 *)archive + ((offset >> 1) << 1)), position);
        i++;
    }
}

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

u16 saveVram16Pixels(s32 x, s32 y) {
    u16 index = func_800162C8();
    if (index != 32) {
        SavedImageEntry *entries = gSavedImageEntries;
        SavedImageEntry *entry = entries;
        entry += index;
        entry->rect.w = 16;
        entry->rect.x = x;
        entry->rect.y = y;
        entry->rect.h = 1;
        StoreImage(&gSavedImageEntries[index].rect, (u32 *)gSavedImageEntries[index].pixels);
        entry->flags = 1;
        return index;
    }
    return 32;
}

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

void startCurrentPlayerAnimation(u8 animation, u8 mode) {
    RacePlayer *player;
    currentPlayerAbsolute->animationIndex = D_800EB09C[currentPlayerAbsolute->unk014][animation];
    currentPlayerAbsolute->animationFlags = mode;
    currentPlayerAbsolute->animationStep = 0;
    player = currentPlayerAbsolute;
    player->animationStream = (u16 *)(PLAYER_ANIMATION_ARCHIVE + PLAYER_ANIMATION_OFFSETS[player->animationIndex]);
    player->animationFrameCount = *player->animationStream;
    player->animationStream++;
    player->animationTicks = *player->animationStream;
    player->animationStream++;
    func_80017268();
    currentPlayerAbsolute->animationFlags |= 0x20;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016F5C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017268);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001758C);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017BAC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017C20);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017D20);

void openMusicSequence(s16 bankId, u32 *sequence) {
    if (gMusicSequenceActive) {
        closeMusicSequence();
    }
    gMusicSequenceId = SsSeqOpen(sequence, bankId);
    SsUtSetReverbDepth(48, 48);
    gMusicSequenceActive = 1;
}

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

void startSoundSystem(void) {
    if (!gSoundSystemActive) {
        SsSetStereo();
        SsStart();
    }
    SsSetMVol(127, 127);
    gSoundSystemActive = 1;
}

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

void stopMatchingSoundVoice(u8 owner, u8 bank, u16 note) {
    u8 i;
    SoundVoiceRecord *voice;
    for (i = 0; i < 8; i++) {
        voice = &gSoundVoiceRecords[i];
        if (voice->owner == owner && voice->bank == bank && voice->note == note) {
            SsUtKeyOffV(i + 12);
            voice->owner = 0xFF;
            break;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800187B8);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018804);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001890C);

void initializeMemoryCardSaveHeader(void) {
    u16 i;
    MemCardInit(0);
    MemCardStart();
    gMemoryCardSaveHeader.magic[0] = 'S';
    gMemoryCardSaveHeader.magic[1] = 'C';
    gMemoryCardSaveHeader.iconDisplay = 0x13;
    for (i = 0; i < 28; i++) {
        gMemoryCardSaveHeader.reserved[i] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018AA4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018BA4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018D80);

s16 validateAndCopyMemoryCardSave(void) {
    u32 i;
    u32 checksum;

    if (gMemoryCardSaveHeader.version != 0) {
        return 0;
    }
    i = 0;
    checksum = 0;
    for (; i < gMemoryCardSaveHeader.payloadSize; i++) {
        checksum += gMemoryCardSaveHeader.payload[i];
    }
    if (gMemoryCardSaveHeader.checksum != checksum) {
        return 0;
    }
    memcpy(gMemoryCardLoadDestination, gMemoryCardSaveHeader.payload,
           gMemoryCardSaveHeader.payloadSize);
    return 1;
}

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

void beginMemoryCardBufferRead(void) {
    if (tryAcceptMemoryCard() == 1) {
        func_80018D80(0, D_8013AF4C);
        setCurrentTaskCallback(func_80019980);
    }
}

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

void waitForMovieOutput(MovieDecodeState *state) {
    volatile s32 timeout = 0x800000;
    while (state->outputComplete == 0) {
        if (--timeout == 0) {
            state->outputComplete = 1;
            state->frameIndex = !state->frameIndex;
            state->slice.x = state->frames[state->frameIndex].x;
            state->slice.y = state->frames[state->frameIndex].y;
        }
    }
    state->outputComplete = 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A474);

void waitForFrameHcount(u32 minimumHcount) {
    DrawSync(0);
    while ((u32)VSync(1) < minimumHcount) {
    }
    VSync(0);
    gSynchronizedFrameCount++;
}

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

void begin30TickTransitionOnFlag10(void) {
    if (D_8013AFE4 & 0x10) {
        D_8013AE88->data18 = 30;
        func_80070274(2, 0, 30);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(func_8001D100);
    }
}

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

void beginResourceLoadOrUnlockNotification(void) {
    D_8013AFAA |= 0x48;
    if (D_8013AFE4 & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    } else {
        func_800A7CE8(D_80120EA4[0] + 1);
        setCurrentTaskCallback(startResourceLoadWhenFlagAFE4Bit9Set);
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001CF18);

void startTaskTransitionWhenFlagAEE8Set(void) {
    if (D_8013AE88->data18 == 0) {
        if (D_8013AEE8[0] & 0xB0) {
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

void initializeRaceTaskState(void) {
    D_8013AFE4 = 1;
    if (D_8013AFE2 == 2) {
        resetSecondaryRng();
    }
    initializeActiveRacePlayers();
    func_80040A60();
    func_800149A8();
}

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

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate) {
    D_8013AF98->t[0] = coordinate->coord.t[0];
    D_8013AF98->t[1] = coordinate->coord.t[1];
    D_8013AF98->t[2] = coordinate->coord.t[2];
    RotMatrixZYX(rotation, D_8013AF98);
    coordinate->coord = *D_8013AF98;
    coordinate->flg = 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001E524);
