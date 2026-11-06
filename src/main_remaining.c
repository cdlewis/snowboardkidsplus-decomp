#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/graphics.h"
#include "game/coordinate.h"
#include "game/race_camera.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/cd_queue.h"
#include "game/controller.h"
#include "game/race_player.h"
#define GAME_OPTIONS_GP_ADDRESSING
#include "game/game_options.h"
#define SELECTED_RECORDS_GP_ADDRESSING
#include "game/selected_records.h"
#include "psx/libsnd.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"
#include "psx/libmcrd.h"
#include "psx/libcd.h"
#include "psx/libpress.h"

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

extern char D_800101B8[];
extern char D_800101D0[];
extern char D_800101E8[];
extern char D_80010200[];
extern char D_80010218[];

char *gMemoryCardFileNames[5] __attribute__((section(".data.memory_card_file_names"))) = {
    D_800101B8, D_800101D0, D_800101E8, D_80010200, D_80010218
};

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
/* Nine bank flags with the original three trailing padding bytes. */
s8 gSoundBankLoaded[12] __attribute__((section(".data.sound_bank_loaded"), aligned(4))) = {0};

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
extern s32 D_8013B600[2];
extern struct DIRENTRY D_8013B700[2][15];
extern char *D_8013AF40;
extern s16 D_8013AF3E;
extern u16 D_8013AFAA __attribute__((section(".data")));
extern u8 D_8013AFD6;
extern s32 D_8013B20C __attribute__((section(".data")));
extern u8 D_8013B562 __attribute__((section(".data")));
extern u8 D_80120EA4[];
extern s16 D_8013B104 __attribute__((section(".data")));
extern u8 D_8013AFE2;
extern u8 D_8013B1AB __attribute__((section(".data")));
extern u16 D_8013AFE8;
extern void *D_8013AF4C;
extern void *gMemoryCardLoadDestination;
extern u8 D_8013AF3A;
extern u8 D_8013AF48;

int strcmp(const char *, const char *);

u32 *func_8001A2B8(MovieDecodeState *state);
void func_800131B4(void);
void func_8001AB3C(void);
void func_80019D3C(s32 movieIndex);
void func_8001A540(void);
void startMovieCdRead(CdlLOC *location);
void func_8002EE0C(void);
void func_8001D044(void);
void func_8001D42C(void);
void func_8001DB7C(void);
void func_8001CD64(void);
void updateThreeFrameScreenBlend(void);
void uploadTimArchive(TimArchive *archive, s16 *position);
void *memcpy(void *destination, const void *source, unsigned int size);
u16 func_800162C8(void);
s32 func_80017268(void);
void selectMemoryCardLoadFile(u8 slot, void *destination);
void pollMemoryCardBufferReadTask(void);
void func_8001C0DC(void);
void func_8003DEE0(void);
void func_8001B858(void);
void func_8002FFC0(void);
s32 func_80014838(TaskCallback callback);
s32 func_80019520(void);
void pollMemoryCardReadTask(void);
void func_80018AA4(void);
s32 func_80019288(void);
s32 func_80019670(void);
void pollMemoryCardStatusTask(void);
void func_8001D100(void);
void advanceTaskWhenFlagAFE4Bit8Set(void);
void startResourceLoadWhenFlagAFE4Bit9Set(void);
void func_800180F4(s16 bankId);
void func_80040A60(void);

void uploadTimImageWithOffset(u32 *tim, s16 *position) {
    RECT rect;
    GsIMAGE image;
    GsGetTimInfo(tim + 1, &image);
    rect.x = image.px;
    rect.y = image.py;
    rect.w = image.pw;
    rect.h = image.ph;
    if (position != NULL) {
        rect.x += position[0];
        rect.y += position[1];
    }
    LoadImage(&rect, image.pixel);
    if (image.pmode & 8) {
        rect.x = image.cx;
        rect.y = image.cy;
        rect.w = image.cw;
        rect.h = image.ch;
        if (position != NULL) {
            rect.x += position[0];
            rect.y += position[1];
        }
        LoadImage(&rect, image.clut);
    }
}

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
        uploadTimImageWithOffset((u32 *)((u8 *)archive + ((offset >> 1) << 1)), position);
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

void applyStereoSoundOption(void) {
    CdlATV volume;
    if ((s16)getStereoSoundFlag()) {
        volume.val0 = 128;
        volume.val1 = 0;
        volume.val2 = 128;
        volume.val3 = 0;
        CdMix(&volume);
        SsSetStereo();
    } else {
        volume.val0 = 128;
        volume.val1 = 128;
        volume.val2 = 128;
        volume.val3 = 128;
        CdMix(&volume);
        SsSetMono();
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80017C20);

void loadSoundBank(s16 bankId, u8 *header, u8 *body) {
    if (gSoundBankLoaded[bankId] == 0) {
        while (bankId != SsVabOpenHead(header, bankId)) {
        }
        SsVabTransBody(body, bankId);
        SsVabTransCompleted(1);
        gSoundBankLoaded[bankId] = 1;
    }
}

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

void updateMusicSequenceFade(void) {
    if (gMusicFadeMode != 0) {
        if (gMusicFadeMode & 1) {
            D_8013AF30 += gMusicFadeStep;
            if (D_8013AF30 > 0x4000) {
                D_8013AF30 = 0x4000;
                gMusicFadeMode = 0;
            }
        } else {
            D_8013AF30 -= gMusicFadeStep;
            if (D_8013AF30 < 0) {
                D_8013AF30 = 0;
                gMusicFadeMode = 0;
                stopMusicSequence();
            }
        }
        SsSeqSetVol(gMusicSequenceId, D_8013AF30 >> 8, D_8013AF30 >> 8);
    }
}

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

s32 playSoundEffect(u8 bank, u16 programAndTone, s16 volume) {
    u16 note = programAndTone;
    if (gSoundBankLoaded[bank]) {
        D_8013AF29++;
        if ((s8)D_8013AF29 >= 12) {
            D_8013AF29 = 0;
        }
        SsUtKeyOffV((s8)D_8013AF29);
        SsUtKeyOnV((s8)D_8013AF29, bank, programAndTone & 0xFF,
                   note >> 8, note >> 8, 0, volume, volume);
        return (s8)D_8013AF29;
    }
    return 0xFF;
}

s32 getNearestCameraManhattanDistance(s32 x, s32 y, s32 z) {
    s32 dx, dy, dz;
    s32 distance, other;
    PlayerCamera *cameras = D_801241E8;
    dx = x - cameras[0].x;
    if (dx < 0) dx = -dx;
    dy = y - cameras[0].y;
    if (dy < 0) dy = -dy;
    dz = z - cameras[0].z;
    if (dz < 0) dz = -dz;
    distance = (dx + dy + dz) >> 8;
    if (D_8013AFCD == 2) {
        dx = x - cameras[1].x;
        if (dx < 0) dx = -dx;
        dy = y - cameras[1].y;
        if (dy < 0) dy = -dy;
        dz = z - cameras[1].z;
        if (dz < 0) dz = -dz;
        other = (dx + dy + dz) >> 8;
        if (other < distance) distance = other;
    }
    return distance;
}

s32 attenuateSoundVolume(s32 distance, s16 volume) {
    s32 result;
    if (distance > 0x2400) {
        result = 0;
    } else if (distance <= 0x400) {
        result = volume;
    } else {
        result = volume - distance / (0x2000 / volume);
    }
    return result;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800183FC);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018538);

s32 playOwnedSoundEffect(u8 owner, u8 bank, u16 note, s16 volume) {
    u8 i;
    SoundVoiceRecord *voice;
    if (gSoundBankLoaded[bank]) {
        for (i = 0; i < 8; i++) {
            voice = &gSoundVoiceRecords[i];
            if (voice->owner == 0xFF) {
                SsUtKeyOnV(i + 12, bank, note & 0xFF, note >> 8,
                           note >> 8, 0, volume, volume);
                voice->owner = owner;
                voice->bank = bank;
                voice->note = note;
                return (s16)i + 12;
            }
        }
    }
    return 0xFF;
}

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

void selectMemoryCardLoadFile(u8 slot, void *destination) {
    switch (slot) {
    case 1:
    case 2:
    case 3:
        gMemoryCardFileNames[slot][18] = (D_8013AFCE / 10) | 0x30;
        gMemoryCardFileNames[slot][19] = (D_8013AFCE % 10) | 0x30;
        break;
    case 4:
        gMemoryCardFileNames[4][18] = (D_8013AF3A / 10) | 0x30;
        gMemoryCardFileNames[4][19] = (D_8013AF3A % 10) | 0x30;
        break;
    }
    gMemoryCardLoadDestination = destination;
    D_8013AF48 = slot;
    D_8013AF40 = gMemoryCardFileNames[slot];
}

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

s32 memoryCardFileExists(void) {
    s32 i;
    for (i = 0; i < D_8013B600[D_8013AF38]; i++) {
        if (strcmp(D_8013B700[D_8013AF38][i].name, D_8013AF40) == 0) {
            return 1;
        }
    }
    return 0;
}

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

void resetMemoryCardTaskState(void) {
    D_8013AF3E = 0;
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198C4);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_800198EC);

void beginMemoryCardBufferRead(void) {
    if (tryAcceptMemoryCard() == 1) {
        selectMemoryCardLoadFile(0, D_8013AF4C);
        setCurrentTaskCallback(pollMemoryCardBufferReadTask);
    }
}

void pollMemoryCardBufferReadTask(void) {
    switch (func_80019288()) {
    case 0: D_8013AF3E = 1; return;
    case 5: D_8013AF3E = 2; return;
    case 2: D_8013AF3E = 6; break;
    case 1: D_8013AF3E = 5; break;
    case 3: D_8013AF3E = 7; break;
    case 4: D_8013AF3E = 8; break;
    case 6: D_8013AF3E = 9; break;
    case 7: D_8013AF3E = 10; break;
    case 8: D_8013AF3E = 11; break;
    }
    func_800149A8();
}

s32 startMemoryCardReadTask(void) {
    if (D_8013AF3E == 5) {
        func_80014874(pollMemoryCardReadTask);
        D_8013AF3E = 3;
        return 1;
    }
    return 0;
}

void pollMemoryCardReadTask(void) {
    s32 result = func_80019520();
    if (result != 0) {
        switch (result) {
        case 1: D_8013AF3E = 0; break;
        case 2: D_8013AF3E = 7; break;
        case 3: D_8013AF3E = 8; break;
        case 4: D_8013AF3E = 11; break;
        case 5: D_8013AF3E = 12; break;
        }
        func_800149A8();
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019AF4);

void waitForMemoryCardAcceptTask(void) {
    if (tryAcceptMemoryCard() == 1) {
        func_80018AA4();
        setCurrentTaskCallback(pollMemoryCardStatusTask);
    }
}

void pollMemoryCardStatusTask(void) {
    switch (func_80019288()) {
    case 0: D_8013AF3E = 1; return;
    case 5: D_8013AF3E = 2; return;
    case 2: D_8013AF3E = 6; break;
    case 8: D_8013AF3E = 11; break;
    case 1: D_8013AF3E = 5; break;
    case 3: D_8013AF3E = 7; break;
    case 4: D_8013AF3E = 8; break;
    case 6: D_8013AF3E = 9; break;
    case 7: D_8013AF3E = 10; break;
    }
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019C40);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80019C84);

void pollMemoryCardWriteTask(void) {
    s32 result = func_80019670();
    if (result != 0) {
        switch (result) {
        case 1: D_8013AF3E = 0; break;
        case 2: D_8013AF3E = 7; break;
        case 3: D_8013AF3E = 8; break;
        case 4: D_8013AF3E = 9; break;
        case 5: D_8013AF3E = 10; break;
        }
        func_800149A8();
    }
}


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

void startMovieStream(CdlLOC *location, DecDCCb outputCallback) {
    DecDCTReset(0);
    DecDCToutCallback(outputCallback);
    StSetRing((u8 *)0x80180000, 32);
    StSetStream(1, 1, -1, 0, 0);
    startMovieCdRead(location);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001A0EC);

s32 decodeNextMovieFrame(MovieDecodeState *state) {
    u32 *frame;
    s32 timeout = 2000;
    while (timeout != 0) {
        frame = func_8001A2B8(state);
        if (frame != 0) {
            state->vlcBufferIndex = !state->vlcBufferIndex;
            DecDCTvlc2(frame, state->vlcBuffers[state->vlcBufferIndex], (u16 *)0x80190000);
            StFreeRing(frame);
            return 0;
        }
        timeout--;
    }
    return -1;
}

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

void startMovieCdRead(CdlLOC *location) {
    u8 mode = CdlModeSpeed;
    do {
        while (!CdControl(CdlSetloc, (u8 *)location, 0)) {}
        while (!CdControl(CdlSetmode, &mode, 0)) {}
        VSync(3);
    } while (!CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT | CdlModeSize1));
}

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

void runMovieAndGameLoop(void) {
    func_800131B4();
    while (1) {
        func_8001AB3C();
        func_80019D3C(0);
        func_80019D3C(1);
        func_8001A540();
        ResetGraph(3);
        StopCallback();
    }
}

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

void sortLitObject(GsDOBJ2 *object) {
    GsGetLw(object->coord2, D_8013AF98);
    GsSetLightMatrix(D_8013AF98);
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    GsSortObject4(object, &D_800EC018[D_8013AF78], 2, (u32 *)0x1F800000);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_8001B4EC);

void initializeGameOptionTask(void) {
    queueCdSectorCommand(2, 0x130, 0x32, (void *)0x80180000, 0);
    D_8013B562 = 0;
    D_8013AFD7 = 0;
    D_8013AE88->data18 = 0;
    func_8002EE0C();
    resetGameOptionState();
    setCurrentTaskCallback(initializeGameOptionViewTask);
    D_8013AE88->unk10 |= 2;
}

void initializeGameOptionViewTask(void) {
    initPlayerCamera(0);
    func_8003DEE0();
    setControllerFlag0(0);
    setControllerFlag0(1);
    setCurrentTaskCallback(func_8001B858);
    func_80014838(func_8002FFC0);
    D_8013AE88->unk10 |= 1;
}

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
        setCurrentTaskCallback(initializeGameOptionViewTask);
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
        startUnlockNotification(D_80120EA4[0] + 1);
        setCurrentTaskCallback(startResourceLoadWhenFlagAFE4Bit9Set);
    }
}

void resetStateAndBeginResourceLoadTransition(u8 requestLoad) {
    GameTask *savedTask;
    resetPendingCdRequests();
    D_8013AFD6 = 0;
    D_8013B20C = 0;
    resetPaletteRecords();
    func_800149E0();
    savedTask = D_8013AE88;
    D_8013AE88 = D_8013B304;
    func_800149E0();
    D_8013AE88 = savedTask;
    D_8013AFAA |= 0x90;
    if (requestLoad) {
        D_8013AFE4 |= 0x200;
    }
    D_8013AFE4 |= 0x80;
    setCurrentTaskCallback(beginResourceLoadOrUnlockNotification);
}

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

void updateThreeFrameScreenBlend(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013B1AB) {
            setCurrentTaskCallback(func_8001CD64);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    }
}

void startThreeFrameScreenBlend(void) {
    func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    D_8013AE88->data18 = 3;
    setCurrentTaskCallback(updateThreeFrameScreenBlend);
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

void initializeSelectedCourseTransition(void) {
    D_8013AFD0 = D_80123D80[D_8013AFCE].unk02;
    if (D_8013AFE2 != 0 || D_8013AFCD == 1 || D_8013AFDA == 0) {
        func_80070274(1, 0, 30);
        func_80071490(3, D_8013B104, 0);
        setCurrentTaskCallback(func_8001DB7C);
    }
}

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

/* This routine accesses the shared player pointer through GP. */
#undef D_8013B0D8

u16 advanceCurrentPlayerInputSequence(u8 sequence[][2], s16 slot) {
    u8 input = D_8013AEEC[D_8013AE88->data18] >> 12;
    switch (D_8013B0D8->unk0CA[slot]) {
    case 0:
        if (input != sequence[D_8013B0D8->unk0D6[slot]][0]) {
            D_8013B0D8->unk0D6[slot] = 0;
            break;
        }
        if (sequence[D_8013B0D8->unk0D6[slot]][1] != 0)
            return sequence[D_8013B0D8->unk0D6[slot]][1];
        D_8013B0D8->unk0CA[slot]++;
    case 1:
        if (input != sequence[D_8013B0D8->unk0D6[slot]][0] && input != 0) {
            D_8013B0D8->unk0CA[slot] = 0;
            D_8013B0D8->unk0D6[slot]++;
        }
        break;
    }
    return 0;
}
