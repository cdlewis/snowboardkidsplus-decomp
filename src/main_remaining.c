#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/graphics.h"
#include "game/coordinate.h"
#include "game/race_camera.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/global_flags.h"
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
#include "main_remaining.h"

#define PLAYER_ANIMATION_ARCHIVE ((u8 *)0x80180000)
#define PLAYER_ANIMATION_OFFSETS ((u32 *)0x80180004)

/* The first word holds the count; later words hold tagged byte offsets.
 * words includes that header word, so the first image uses word index 1.
 */
typedef union TimArchive {
    u16 count;
    s32 words[1];
} TimArchive;



typedef struct PaletteTransitionResource {
    u32 *pixels;
} PaletteTransitionResource;

typedef struct PaletteTransitionState {
    u16 *pixels;
    u16 red[16], green[16], blue[16];
    u16 redStep[16], greenStep[16], blueStep[16];
    u16 mask[16];
} PaletteTransitionState;
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
    u16 pixels[16];
    u16 transitionPixels[16];
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

char gSoundSequenceTable[0xB0] __attribute__((section(".data.sound_sequence_table"), aligned(4))) = {0};

SoundVoiceRecord gSoundVoiceRecords[8] __attribute__((section(".data.sound_voice_records"))) = {0};

MemoryCardSaveHeader gMemoryCardSaveHeader __attribute__((section(".data.memory_card_save_header"))) = {0};

SavedImageEntry gSavedImageEntries[32] __attribute__((section(".data.saved_image_entries"))) = {0};
u32 gSynchronizedFrameCount __attribute__((section(".sdata.synchronized_frame_count"))) = 0;
s8 gSoundSystemActive = 0;
u8 D_8013AF29 = 0;
s16 gMusicSequenceId = 0;
u8 gMusicSequenceActive __attribute__((section(".sdata.music_state"))) = 0;
u8 gMusicFadeMode __attribute__((section(".sdata.music_state"))) = 0;
s16 gMusicFadeStep __attribute__((section(".sdata.music_state"))) = 0;
s16 D_8013AF30 __attribute__((section(".sdata.music_state"))) = 0;
s16 D_8013AF32 __attribute__((section(".sdata.music_state"))) = 0;
s32 gLinearRandomState __attribute__((section(".data.linear_random_state"))) = 1;
MovieDecodeState gMovieDecodeState __attribute__((section(".bss.movie_decode_state")));



void uploadTimArchive(TimArchive *archive, s16 *position);

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

s32 startPaletteTransition(s16 x, s16 y, u32 *pixels, u16 duration) {
    u16 index = saveVram16Pixels(x, y);
    GameTask *task;
    PaletteTransitionResource *resource;
    if (index != 32) {
        if ((s16)func_800148B4(func_80016A7C) != 0) {
            D_8013AE8C->resource14 = allocateTaskResource();
            task = D_8013AE8C;
            resource = (PaletteTransitionResource *)&D_800D9504[task->resource14];
            resource->pixels = pixels;
            task->data1C = 1;
            task->data18 = duration;
            task->data1A = index;
            gSavedImageEntries[index].flags |= 2;
            return 1;
        }
        gSavedImageEntries[index].flags = 0;
    }
    return 0;
}

void requestSavedImageRestore(u16 index) {
    if (index != 32) {
        gSavedImageEntries[index].flags |= 0x40;
    }
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016628);

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80016A7C);

void updatePaletteTransition(void) {
    PaletteTransitionState *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteTransitionState *)&D_800D9504[D_8013AE88->resource14];
    if (--D_8013AE88->data18 != 0) {
        for (i = 0; i < 16; i++) {
            resource->red[i] += resource->redStep[i];
            resource->green[i] += resource->greenStep[i];
            resource->blue[i] += resource->blueStep[i];
            gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] =
                ((resource->red[i] & 0x1F00) >> 8) |
                ((resource->green[i] & 0x1F00) >> 3) |
                ((resource->blue[i] & 0x1F00) << 2) | resource->mask[i];
        }
        gSavedImageEntries[D_8013AE88->data1A].flags |= 0x20;
    } else {
        for (i = 0; i < 16; i++) {
            gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] = resource->pixels[i];
        }
        gSavedImageEntries[D_8013AE88->data1A].flags = 0x20;
        func_800149A8();
    }
}

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

void initializeSoundSystem(s32 hot) {
    u8 i;
    if (hot == 0) {
        func_800A89F4();
    } else {
        SsInitHot();
    }
    resetCdVolumeFade();
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(0, 103, 103);
    SsSetTickMode(SS_TICKVSYNC);
    applyStereoSoundOption();
    SsSetTableSize(gSoundSequenceTable, 1, 1);
    for (i = 0; i < 9; i++) {
        gSoundBankLoaded[i] = 0;
    }
    gSoundSystemActive = 0;
    D_8013AF29 = 0;
    D_8013AF32 = 0;
    gMusicSequenceActive = 0;
    SsUtSetReverbType(SS_REV_TYPE_ROOM);
    SsUtReverbOn();
    for (i = 0; i < 8; i++) {
        gSoundVoiceRecords[i].owner = 255;
    }
}

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
    if (gameOptionPlayerModeAbsolute == 2) {
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

s32 playPositionalSoundEffect(s32 x, s32 y, s32 z, u8 bank, u16 programAndTone, s16 volume) {
    u16 note;
    volume = attenuateSoundVolume(getNearestCameraManhattanDistance(x, y, z), volume);
    note = programAndTone;
    if (volume == 0) return 0xFF;
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

void playPlayerPositionalSoundEffect(u8 playerIndex, u8 bank, u16 programAndTone, s16 volume) {
    u16 note;
    volume = attenuateSoundVolume(getNearestCameraManhattanDistance(
        D_80120E90[playerIndex].x, D_80120E90[playerIndex].y,
        D_80120E90[playerIndex].z), volume);
    note = programAndTone;
    if (volume == 0) return;
    if (gSoundBankLoaded[bank]) {
        SsUtKeyOffV(playerIndex + 20);
        SsUtKeyOnV(playerIndex + 20, bank, programAndTone & 0xFF,
                   note >> 8, note >> 8, 0, volume, volume);
    }
}

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

void captureMemoryCardSaveIcon(void) {
    RECT paletteRect;
    RECT iconRect;
    s16 icon;

    if ((s16)getGlobalFlagMask(0)) {
        icon = 2;
    } else if ((s16)getGlobalFlagMask(1)) {
        icon = 1;
    } else {
        icon = 0;
    }
    switch (icon) {
    case 0:
        paletteRect.y = 0xF4;
        iconRect.x = 0x34C;
        break;
    case 1:
        paletteRect.y = 0xF5;
        iconRect.x = 0x350;
        break;
    case 2:
        paletteRect.y = 0xF6;
        iconRect.x = 0x354;
        break;
    }
    paletteRect.x = 0x310;
    paletteRect.w = 16;
    paletteRect.h = 1;
    iconRect.y = 0x1D0;
    iconRect.w = 4;
    iconRect.h = 48;
    StoreImage(&paletteRect, (u32 *)gMemoryCardSaveHeader.iconPalette);
    StoreImage(&iconRect, (u32 *)gMemoryCardSaveHeader.icons);
}

INCLUDE_ASM("asm/nonmatchings/main_remaining", func_80018BA4);

void selectMemoryCardLoadFile(u8 slot, void *destination) {
    switch (slot) {
    case 1:
    case 2:
    case 3:
        gMemoryCardFileNames[slot][18] = (selectedCourseIndexAbsolute / 10) | 0x30;
        gMemoryCardFileNames[slot][19] = (selectedCourseIndexAbsolute % 10) | 0x30;
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
