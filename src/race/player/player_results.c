#include "../race_presentation_internal.h"

s16 compareRaceTimes(const RaceTime *a, const RaceTime *b) {
    s32 first = (a->centiseconds & 0xFF00) + a->seconds * 25600 + a->minutes * 1536000;
    s32 second = (b->centiseconds & 0xFF00) + b->seconds * 25600 + b->minutes * 1536000;
    s32 result;

    if (first > second) {
        result = 1;
    } else if (first < second) {
        result = -1;
    } else {
        result = 0;
    }
    return result;
}

void assignTwoPlayerResultSlots(s16 result) {
    if (result == 1) {
        gRacePlayers[0].unk100[0] = 0;
        gRacePlayers[1].unk100[0] = 1;
    } else if (result == 2) {
        gRacePlayers[0].unk100[0] = 1;
        gRacePlayers[1].unk100[0] = 0;
    } else if ((rand() & 1) == 0) {
        gRacePlayers[0].unk100[0] = 2;
        gRacePlayers[1].unk100[0] = 3;
    } else {
        gRacePlayers[0].unk100[0] = 3;
        gRacePlayers[1].unk100[0] = 2;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_results", func_8003CAF0);

INCLUDE_ASM("asm/nonmatchings/race/player/player_results", func_8003D904);

void renderCurrentCameraPlayersAndUpdateSound(void) {
    u16 i;
    if (func_8003D904(D_8013AE88->data18)) {
        GsSetRefView2L(&gPlayerCameraViews[D_8013AE88->data18]);
        updateCameraRotationMatrices(D_8013AE88->data18);
        if (D_8013AE88->data18 != 0 && (D_8013AFAA & 0x2000)) {
            for (i = 0; i < D_8013AFD2; i++) {
                if (isPointWithinCameraForwardRange(gRacePlayers[i].worldCoordinate.coord.t[0], gRacePlayers[i].worldCoordinate.coord.t[2], 0)) {
                    func_8001E3E4((SVECTOR *)&gRacePlayers[i].unk0B4, &gRacePlayers[i].worldCoordinate);
                    setCoordinateRotationZYX(&gRacePlayers[i].modelRotation, &gRacePlayers[i].modelCoordinate);
                    func_8001AF64(i, 0);
                }
            }
            drawCourseModelsForView(0);
        } else {
            for (i = 0; i < D_8013AFD2; i++) {
                if (isPointWithinCameraForwardRange(gRacePlayers[i].worldCoordinate.coord.t[0], gRacePlayers[i].worldCoordinate.coord.t[2], D_8013AE88->data18)) {
                    func_8001E3E4((SVECTOR *)&gRacePlayers[i].unk0B4, &gRacePlayers[i].worldCoordinate);
                    setCoordinateRotationZYX(&gRacePlayers[i].modelRotation, &gRacePlayers[i].modelCoordinate);
                    func_8001AF64(i, D_8013AE88->data18);
                }
            }
            drawCourseModelsForView(D_8013AE88->data18);
        }
    }
    updateCurrentPlayerSound();
}

void updateCurrentPlayerSound(void) {
    s32 semitone;
    s16 fine;
    if (D_8013AFD3) {
        gRacePlayers[D_8013AE88->data18].unk96C = 0;
    }
    if (gRacePlayers[D_8013AE88->data18].unk96C != 0) {
        if (gRacePlayers[D_8013AE88->data18].soundVoice == 0xFF) {
            gRacePlayers[D_8013AE88->data18].soundVoice = playOwnedSoundEffect(D_8013AE88->data18, 1, gRacePlayers[D_8013AE88->data18].unk96E, 0x20);
            gRacePlayers[D_8013AE88->data18].unk970 = gRacePlayers[D_8013AE88->data18].unk96E;
        } else if (gRacePlayers[D_8013AE88->data18].unk970 != gRacePlayers[D_8013AE88->data18].unk96E) {
            func_800187B8(gRacePlayers[D_8013AE88->data18].soundVoice);
            gRacePlayers[D_8013AE88->data18].soundVoice = playOwnedSoundEffect(D_8013AE88->data18, 1, gRacePlayers[D_8013AE88->data18].unk96E, 0x20);
            gRacePlayers[D_8013AE88->data18].unk970 = gRacePlayers[D_8013AE88->data18].unk96E;
        }
        semitone = gRacePlayers[D_8013AE88->data18].unk972 / 128;
        fine = 127 - gRacePlayers[D_8013AE88->data18].unk972 % 128;
        SsUtChangePitch(gRacePlayers[D_8013AE88->data18].soundVoice, 1, gRacePlayers[D_8013AE88->data18].unk96E & 255, gRacePlayers[D_8013AE88->data18].unk96E >> 8, fine, -semitone, fine);
        SsUtSetVVol(gRacePlayers[D_8013AE88->data18].soundVoice, gRacePlayers[D_8013AE88->data18].unk96C, gRacePlayers[D_8013AE88->data18].unk96C);
    } else if (gRacePlayers[D_8013AE88->data18].soundVoice != 0xFF) {
        func_800187B8(gRacePlayers[D_8013AE88->data18].soundVoice);
        gRacePlayers[D_8013AE88->data18].soundVoice = 0xFF;
    }
}

void stopPlayerSound(u16 playerIndex) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];

    player->unk96C = 0;
    if (player->soundVoice != 0xFF) {
        func_800187B8(player->soundVoice);
        player->soundVoice = 0xFF;
        player->unk96E = 0xFFFF;
        player->unk970 = 0xFFFF;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_results", func_8003DEE0);
