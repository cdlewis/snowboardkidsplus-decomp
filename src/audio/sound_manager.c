#include "../engine/resource_services_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/system_runtime.h"
#include "game/io/cd_queue.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player.h"
#include "psx/libcd.h"
#include "psx/libsnd.h"

s8 gSoundBankLoaded[12] __attribute__((section(".data.sound_bank_loaded"), aligned(4))) = {0};

char gSoundSequenceTable[0xB0] __attribute__((section(".data.sound_sequence_table"), aligned(4))) = {0};

SoundVoiceRecord gSoundVoiceRecords[8] __attribute__((section(".data.sound_voice_records"))) = {0};

s8 gSoundSystemActive = 0;

u8 D_8013AF29 = 0;

s16 gMusicSequenceId = 0;

u8 gMusicSequenceActive __attribute__((section(".sdata.music_state"))) = 0;

u8 gMusicFadeMode __attribute__((section(".sdata.music_state"))) = 0;

s16 gMusicFadeStep __attribute__((section(".sdata.music_state"))) = 0;

s16 D_8013AF30 __attribute__((section(".sdata.music_state"))) = 0;

s16 D_8013AF32 __attribute__((section(".sdata.music_state"))) = 0;

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

void playMusicSequenceAtDefaultVolume(s16 repeatCount) {
    if (gMusicSequenceActive) {
        D_8013AF30 = 0x40 << 8;
        SsSeqSetVol(gMusicSequenceId, 0x40, 0x40);
        SsSeqPlay(gMusicSequenceId, 1, repeatCount);
    }
}

void fadeInMusicSequence(s16 step, s16 repeatCount) {
    if (gMusicSequenceActive && gMusicFadeMode == 0) {
        gMusicFadeMode = 1;
        gMusicFadeStep = step;
        D_8013AF30 = 0;
        SsSeqSetVol(gMusicSequenceId, 0, 0);
        SsSeqPlay(gMusicSequenceId, 1, repeatCount);
    }
}

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

/* DECOMP_ATTEMPT: inline 20/22 */
#ifdef NON_MATCHING
void SsVabClose(s16 id);

void func_800180F4(s16 id) {
    s8 *open;
    if (gSoundSystemActive) {
        open = &gSoundBankLoaded[id];
        if (*open) {
            SsVabClose(id);
            *open = 0;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/audio/sound_manager", func_800180F4);
#endif

void stopAllSoundVoices(void) {
    SsUtAllKeyOff(0);
}

/* DECOMP_ATTEMPT: inline 15/17 */
#ifdef NON_MATCHING
void stopAllSoundVoices(void);

void func_8001816C(void) {
    u32 bank;

    stopAllSoundVoices();
    for (bank = 0; (u8)bank < 9; bank++) {
        func_800180F4((u8)bank);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/audio/sound_manager", func_8001816C);
#endif

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

/* DECOMP_ATTEMPT: inline 78/79 */
#ifdef NON_MATCHING
s32 func_800183FC(u16 playerIndex, u8 bank, u16 programAndTone, s16 volume) {
    u16 playbackVolume = volume;
    const RacePlayer *players = gRacePlayers;
    const RacePlayer *player = &players[playerIndex];

    if (player->unk008) {
        playbackVolume = attenuateSoundVolume(
            getNearestCameraManhattanDistance(player->x, player->y, player->z), volume);
        if (playbackVolume == 0) {
            return 0xFF;
        }
    }

    if (gSoundBankLoaded[bank]) {
        D_8013AF29++;
        if ((s8)D_8013AF29 >= 12) {
            D_8013AF29 = 0;
        }
        SsUtKeyOffV((s8)D_8013AF29);
        SsUtKeyOnV((s8)D_8013AF29, bank, programAndTone & 0xFF,
                   programAndTone >> 8, programAndTone >> 8, 0,
                   playbackVolume, playbackVolume);
        return (s8)D_8013AF29;
    }
    return 0xFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/audio/sound_manager", func_800183FC);
#endif

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

/* DECOMP_ATTEMPT: inline 17/19 */
#ifdef NON_MATCHING
void func_800187B8(u8 voice) {
    SsUtKeyOffV(voice);
    gSoundVoiceRecords[voice - 12].owner = 0xFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/audio/sound_manager", func_800187B8);
#endif

void playPlayerPositionalSoundEffect(u8 playerIndex, u8 bank, u16 programAndTone, s16 volume) {
    u16 note;
    volume = attenuateSoundVolume(getNearestCameraManhattanDistance(
        gRacePlayers[playerIndex].x, gRacePlayers[playerIndex].y,
        gRacePlayers[playerIndex].z), volume);
    note = programAndTone;
    if (volume == 0) return;
    if (gSoundBankLoaded[bank]) {
        SsUtKeyOffV(playerIndex + 20);
        SsUtKeyOnV(playerIndex + 20, bank, programAndTone & 0xFF,
                   note >> 8, note >> 8, 0, volume, volume);
    }
}

/* DECOMP_ATTEMPT: inline 74/74 */
#ifdef NON_MATCHING
void func_8001890C(u8 playerIndex, u8 bank, u16 programAndTone, s16 volume) {
    RacePlayer *players = gRacePlayers;
    s16 playbackVolume = volume;

    if (players[playerIndex].unk008) {
        playbackVolume = attenuateSoundVolume(getNearestCameraManhattanDistance(
            players[playerIndex].x, players[playerIndex].y, players[playerIndex].z), volume);
        if (playbackVolume < 24) {
            playbackVolume = 24;
        }
    }
    if (gSoundBankLoaded[bank]) {
        SsUtKeyOffV(playerIndex + 20);
        SsUtKeyOnV(playerIndex + 20, bank, programAndTone & 0xFF,
                   programAndTone >> 8, programAndTone >> 8, 0,
                   playbackVolume, playbackVolume);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/audio/sound_manager", func_8001890C);
#endif
