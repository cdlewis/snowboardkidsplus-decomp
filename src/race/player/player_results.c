#include "../race_presentation_internal.h"
#include "game/race/player/player_results.h"
#include "../race_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/io/controller_effects.h"
#include "game/menu/selected_records.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_time.h"
#include "game/save/save_data.h"
#include "psx/libgs.h"
#include "psx/libsnd.h"
#include "psx/stdlib.h"

#define ATTACK_SAVE_RECORD (&gPlayerSaveRecords[0])
#define FULL_RACE_TIME(time) \
    ((time).centiseconds + (time).seconds * 25600 + (time).minutes * 1536000)
#define RECORD_ICON(player) \
    ((player).characterId + (player).unk015[1] * 16)

GsF_LIGHT gDefaultSceneLights[3]
    __attribute__((section(".data.default_scene_lights"))) = {0};

extern u8 D_8013AFE1 __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern RaceTime gRaceElapsedTime __attribute__((section(".data")));
extern RaceTime D_8013B188 __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));

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

void updateAttackResultsAndRecords(void) {
    s16 player;
    s16 rank;
    s16 row;
    s16 comparison;
    s32 time;
    s32 recordTime;
    s32 faster;

    switch (D_8013AFE2) {
    case 0:
        break;
    case 1:
        if (D_8013AFE0 == 0) {
            if (playerCountAbsolute == 1) {
                D_8013B190[0] = gRaceElapsedTime;
                D_8013B1A0[0] = D_8013B18D;
            } else {
                comparison = compareRaceTimes(&D_8013B190[0], &D_8013B190[1]);
                if (comparison > 0) {
                    D_8013AFE1 = 2;
                } else if (comparison < 0) {
                    D_8013AFE1 = 1;
                } else if (gRacePlayers[0].scoreAttackPointTotal > gRacePlayers[1].scoreAttackPointTotal) {
                    D_8013AFE1 = 1;
                } else if (gRacePlayers[0].scoreAttackPointTotal < gRacePlayers[1].scoreAttackPointTotal) {
                    D_8013AFE1 = 2;
                } else {
                    D_8013AFE1 = 3;
                }
                assignTwoPlayerResultSlots(D_8013AFE1);
                if (D_8013B1A0[0]) {
                    if (!D_8013B1A0[1]) {
                        D_8013AFE1 = 2;
                        gRacePlayers[0].unk100[0] += 5;
                    } else {
                        D_8013AFE1 = 3;
                    }
                } else if (D_8013B1A0[1]) {
                    D_8013AFE1 = 1;
                    gRacePlayers[1].unk100[0] += 5;
                }
                return;
            }

            for (player = 0; player < playerCountAbsolute; player++) {
                time = FULL_RACE_TIME(D_8013B190[player]);
                for (rank = 0; rank < 5; rank++) {
                    recordTime = FULL_RACE_TIME(ATTACK_SAVE_RECORD->timeRecords[gSelectedCourseIndex][rank]);
                    faster = time < recordTime;
                    if (faster) {
                        break;
                    }
                }
                if (D_8013B1A0[player]) {
                    rank = 5;
                }
                gRacePlayers[player].unk100[0] = rank;
            }
            for (player = 0; player < playerCountAbsolute; player++) {
                rank = gRacePlayers[player].unk100[0];
                if (rank < 5) {
                    for (row = 3; row >= rank; row--) {
                        ATTACK_SAVE_RECORD->timeRecords[gSelectedCourseIndex][row + 1] =
                            ATTACK_SAVE_RECORD->timeRecords[gSelectedCourseIndex][row];
                        ATTACK_SAVE_RECORD->timeRecordCharacters[gSelectedCourseIndex][row + 1] =
                            ATTACK_SAVE_RECORD->timeRecordCharacters[gSelectedCourseIndex][row];
                    }
                    ATTACK_SAVE_RECORD->timeRecords[gSelectedCourseIndex][rank] = D_8013B190[player];
                    ATTACK_SAVE_RECORD->timeRecordCharacters[gSelectedCourseIndex][rank] = RECORD_ICON(gRacePlayers[player]);
                    D_8013AFE1 = 1;
                } else {
                    D_8013AFE1 = 2;
                }
            }
        } else if (D_8013AFE0 == 1) {
            if (playerCountAbsolute == 1) {
                D_8013B1A0[0] = D_8013B18D;
            } else {
                if (gRacePlayers[0].scoreAttackPointTotal > gRacePlayers[1].scoreAttackPointTotal) {
                    D_8013AFE1 = 1;
                } else if (gRacePlayers[0].scoreAttackPointTotal < gRacePlayers[1].scoreAttackPointTotal) {
                    D_8013AFE1 = 2;
                } else {
                    comparison = compareRaceTimes(&D_8013B190[0], &D_8013B190[1]);
                    if (comparison > 0) {
                        D_8013AFE1 = 2;
                    } else if (comparison < 0) {
                        D_8013AFE1 = 1;
                    } else {
                        D_8013AFE1 = 3;
                    }
                }
                assignTwoPlayerResultSlots(D_8013AFE1);
                if (D_8013B1A0[0]) {
                    if (!D_8013B1A0[1]) {
                        D_8013AFE1 = 2;
                        gRacePlayers[0].unk100[0] += 5;
                    } else {
                        D_8013AFE1 = 3;
                    }
                } else if (D_8013B1A0[1]) {
                    D_8013AFE1 = 1;
                    gRacePlayers[1].unk100[0] += 5;
                }
                return;
            }

            for (player = 0; player < playerCountAbsolute; player++) {
                for (rank = 0; rank < 5; rank++) {
                    if (gRacePlayers[player].scoreAttackPointTotal >
                        ATTACK_SAVE_RECORD->countRecordValues[gSelectedCourseIndex][rank]) {
                        break;
                    }
                }
                if (D_8013B1A0[player]) {
                    rank = 5;
                }
                gRacePlayers[player].unk100[0] = rank;
            }
            for (player = 0; player < playerCountAbsolute; player++) {
                rank = gRacePlayers[player].unk100[0];
                if (rank < 5) {
                    for (row = 3; row >= rank; row--) {
                        ATTACK_SAVE_RECORD->countRecordValues[gSelectedCourseIndex][row + 1] =
                            ATTACK_SAVE_RECORD->countRecordValues[gSelectedCourseIndex][row];
                        ATTACK_SAVE_RECORD->countRecordIcons[gSelectedCourseIndex][row + 1] =
                            ATTACK_SAVE_RECORD->countRecordIcons[gSelectedCourseIndex][row];
                    }
                    ATTACK_SAVE_RECORD->countRecordValues[gSelectedCourseIndex][rank] = gRacePlayers[player].scoreAttackPointTotal;
                    ATTACK_SAVE_RECORD->countRecordIcons[gSelectedCourseIndex][rank] = RECORD_ICON(gRacePlayers[player]);
                    D_8013AFE1 = 1;
                } else {
                    D_8013AFE1 = 2;
                }
            }
        } else if (D_8013AFE0 == 2) {
            if (playerCountAbsolute == 1) {
                D_8013B1A0[0] = D_8013B18D;
            } else {
                if (gRacePlayers[0].trickAttackPointTotal > gRacePlayers[1].trickAttackPointTotal) {
                    D_8013AFE1 = 1;
                } else if (gRacePlayers[0].trickAttackPointTotal < gRacePlayers[1].trickAttackPointTotal) {
                    D_8013AFE1 = 2;
                } else {
                    comparison = compareRaceTimes(&D_8013B190[0], &D_8013B190[1]);
                    if (comparison > 0) {
                        D_8013AFE1 = 2;
                    } else if (comparison < 0) {
                        D_8013AFE1 = 1;
                    } else {
                        D_8013AFE1 = 3;
                    }
                }
                assignTwoPlayerResultSlots(D_8013AFE1);
                if (D_8013B1A0[0]) {
                    if (D_8013B1A0[1]) {
                        D_8013AFE1 = 3;
                    } else {
                        D_8013AFE1 = 2;
                        gRacePlayers[0].unk100[0] += 5;
                    }
                } else if (D_8013B1A0[1]) {
                    D_8013AFE1 = 1;
                    gRacePlayers[1].unk100[0] += 5;
                }
                return;
            }

            for (player = 0; player < playerCountAbsolute; player++) {
                for (rank = 0; rank < 5; rank++) {
                    if (gRacePlayers[player].trickAttackPointTotal >
                        ATTACK_SAVE_RECORD->statsRecordValues[gSelectedCourseIndex][rank]) {
                        break;
                    }
                }
                if (D_8013B1A0[player]) {
                    rank = 5;
                }
                gRacePlayers[player].unk100[0] = rank;
            }
            for (player = 0; player < playerCountAbsolute; player++) {
                rank = gRacePlayers[player].unk100[0];
                if (rank < 5) {
                    for (row = 3; row >= rank; row--) {
                        ATTACK_SAVE_RECORD->statsRecordValues[gSelectedCourseIndex][row + 1] =
                            ATTACK_SAVE_RECORD->statsRecordValues[gSelectedCourseIndex][row];
                        ATTACK_SAVE_RECORD->statsRecordIcons[gSelectedCourseIndex][row + 1] =
                            ATTACK_SAVE_RECORD->statsRecordIcons[gSelectedCourseIndex][row];
                    }
                    ATTACK_SAVE_RECORD->statsRecordValues[gSelectedCourseIndex][rank] = gRacePlayers[player].trickAttackPointTotal;
                    ATTACK_SAVE_RECORD->statsRecordIcons[gSelectedCourseIndex][rank] = RECORD_ICON(gRacePlayers[player]);
                    D_8013AFE1 = 1;
                } else {
                    D_8013AFE1 = 2;
                }
            }
        }
        break;
    case 2:
        time = FULL_RACE_TIME(gRaceElapsedTime);
        for (rank = 0; rank < 5; rank++) {
            faster = time < FULL_RACE_TIME(ATTACK_SAVE_RECORD->lapRecords[gSelectedCourseIndex][rank]);
            if (faster) {
                break;
            }
        }
        gRacePlayers[0].unk100[0] = rank;
        if (rank < 5) {
            for (row = 3; row >= rank; row--) {
                ATTACK_SAVE_RECORD->lapRecords[gSelectedCourseIndex][row + 1] =
                    ATTACK_SAVE_RECORD->lapRecords[gSelectedCourseIndex][row];
                ATTACK_SAVE_RECORD->lapRecordIcons[gSelectedCourseIndex][row + 1] =
                    ATTACK_SAVE_RECORD->lapRecordIcons[gSelectedCourseIndex][row];
            }
            ATTACK_SAVE_RECORD->lapRecords[gSelectedCourseIndex][rank] = gRaceElapsedTime;
            ATTACK_SAVE_RECORD->lapRecordIcons[gSelectedCourseIndex][rank] = RECORD_ICON(gRacePlayers[0]);
            if (rank == 0) {
                ATTACK_SAVE_RECORD->bestLapSplitTimes[gSelectedCourseIndex] = D_8013B188;
            }
        }
        if (rank < 5) {
            D_8013AFE1 = 1;
        } else {
            D_8013AFE1 = 2;
        }
        break;
    }
}

s16 shouldRenderPlayerView(u16 player) {
    if (!(D_8013AFAA & 0x400) || ((gFrameCounter & 1) == player)) {
        return 1;
    }
    return 0;
}

void renderCurrentCameraPlayersAndUpdateSound(void) {
    u16 i;
    if (shouldRenderPlayerView(gCurrentTask->data18)) {
        GsSetRefView2L(&gPlayerCameraViews[gCurrentTask->data18]);
        updateCameraRotationMatrices(gCurrentTask->data18);
        if (gCurrentTask->data18 != 0 && (D_8013AFAA & 0x2000)) {
            for (i = 0; i < D_8013AFD2; i++) {
                if (isPointWithinCameraForwardRange(gRacePlayers[i].worldCoordinate.coord.t[0], gRacePlayers[i].worldCoordinate.coord.t[2], 0)) {
                    setCoordinateRotation((SVECTOR *)&gRacePlayers[i].unk0B4, &gRacePlayers[i].worldCoordinate);
                    setCoordinateRotationZYX(&gRacePlayers[i].modelRotation, &gRacePlayers[i].modelCoordinate);
                    sortRacePlayerModelForView(i, 0);
                }
            }
            drawCourseModelsForView(0);
        } else {
            for (i = 0; i < D_8013AFD2; i++) {
                if (isPointWithinCameraForwardRange(gRacePlayers[i].worldCoordinate.coord.t[0], gRacePlayers[i].worldCoordinate.coord.t[2], gCurrentTask->data18)) {
                    setCoordinateRotation((SVECTOR *)&gRacePlayers[i].unk0B4, &gRacePlayers[i].worldCoordinate);
                    setCoordinateRotationZYX(&gRacePlayers[i].modelRotation, &gRacePlayers[i].modelCoordinate);
                    sortRacePlayerModelForView(i, gCurrentTask->data18);
                }
            }
            drawCourseModelsForView(gCurrentTask->data18);
        }
    }
    updateCurrentPlayerSound();
}

void updateCurrentPlayerSound(void) {
    s32 semitone;
    s16 fine;
    if (gRacePauseState) {
        gRacePlayers[gCurrentTask->data18].unk96C = 0;
    }
    if (gRacePlayers[gCurrentTask->data18].unk96C != 0) {
        if (gRacePlayers[gCurrentTask->data18].soundVoice == 0xFF) {
            gRacePlayers[gCurrentTask->data18].soundVoice = playOwnedSoundEffect(gCurrentTask->data18, 1, gRacePlayers[gCurrentTask->data18].unk96E, 0x20);
            gRacePlayers[gCurrentTask->data18].unk970 = gRacePlayers[gCurrentTask->data18].unk96E;
        } else if (gRacePlayers[gCurrentTask->data18].unk970 != gRacePlayers[gCurrentTask->data18].unk96E) {
            stopOwnedSoundVoice(gRacePlayers[gCurrentTask->data18].soundVoice);
            gRacePlayers[gCurrentTask->data18].soundVoice = playOwnedSoundEffect(gCurrentTask->data18, 1, gRacePlayers[gCurrentTask->data18].unk96E, 0x20);
            gRacePlayers[gCurrentTask->data18].unk970 = gRacePlayers[gCurrentTask->data18].unk96E;
        }
        semitone = gRacePlayers[gCurrentTask->data18].unk972 / 128;
        fine = 127 - gRacePlayers[gCurrentTask->data18].unk972 % 128;
        SsUtChangePitch(gRacePlayers[gCurrentTask->data18].soundVoice, 1, gRacePlayers[gCurrentTask->data18].unk96E & 255, gRacePlayers[gCurrentTask->data18].unk96E >> 8, fine, -semitone, fine);
        SsUtSetVVol(gRacePlayers[gCurrentTask->data18].soundVoice, gRacePlayers[gCurrentTask->data18].unk96C, gRacePlayers[gCurrentTask->data18].unk96C);
    } else if (gRacePlayers[gCurrentTask->data18].soundVoice != 0xFF) {
        stopOwnedSoundVoice(gRacePlayers[gCurrentTask->data18].soundVoice);
        gRacePlayers[gCurrentTask->data18].soundVoice = 0xFF;
    }
}

void stopPlayerSound(u16 playerIndex) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];

    player->unk96C = 0;
    if (player->soundVoice != 0xFF) {
        stopOwnedSoundVoice(player->soundVoice);
        player->soundVoice = 0xFF;
        player->unk96E = 0xFFFF;
        player->unk970 = 0xFFFF;
    }
}

void initializeDefaultSceneLighting(void) {
    u8 padding[0x10]; /* Preserve the original stack frame. */

    gDefaultSceneLights[0].vx = 20;
    gDefaultSceneLights[0].vy = -100;
    gDefaultSceneLights[0].vz = -100;
    gDefaultSceneLights[0].r = 245;
    gDefaultSceneLights[0].g = 245;
    gDefaultSceneLights[0].b = 245;
    GsSetFlatLight(0, &gDefaultSceneLights[0]);
    GsSetAmbient(0xE14, 0xE14, 0xE14);
}
