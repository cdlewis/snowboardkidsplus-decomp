#include "text_encoding.h"
#include "../race_presentation_internal.h"
#include "game/race/ui/race_hud.h"
#include "../race_internal.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "game/graphics/draw_mode_sprite.h"
#include "game/graphics/graphics.h"
#include "game/menu/selected_records.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_time.h"
#include "game/save/save_data.h"
#include "psx/libc.h"
#include "psx/libetc.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

extern s8 gActiveRacePlayerCount __attribute__((section(".data")));

#define DRAW_MODE_COMMAND_HIGH 0xE100
#define TEXTURED_SPRITE_CODE 0x64

typedef struct CourseProgressScratch {
    u32 reserved[6];
    s16 playerOrder[4];
    s32 progress;
    s32 distance;
} CourseProgressScratch;

RaceTime gAttackTimeRemaining __attribute__((section(".sdata.attack_race_times"), aligned(4))) = {0, 0, 0};
RaceTime gLapSplitTime __attribute__((section(".sdata.attack_race_times"))) = {0, 0, 0};
u8 gLapSplitTimeCaptured __attribute__((section(".sdata.attack_race_times"))) = 0;

u8 gAttackTimeExpired __attribute__((section(".sdata.attack_race_times"))) = 0;
static u16 attackTimeExpiredPadding __attribute__((section(".sdata.attack_race_times"))) = 0;

RaceTime gPlayerRaceTimes[2] __attribute__((section(".sdata.player_race_times"), aligned(4))) = {{0, 0, 0}, {0, 0, 0}};
RaceTime gPlayerAttackTimesRemaining[2] __attribute__((section(".sdata.player_race_times"))) = {{0, 0, 0}, {0, 0, 0}};
u8 gPlayerAttackTimeExpired[2] __attribute__((section(".sdata.player_race_times"))) = {0, 0};
static u16 playerRaceTimesPadding __attribute__((section(".sdata.player_race_times"))) = 0;
static u32 playerRaceTimesPaddingTail __attribute__((section(".sdata.player_race_times"))) = 0;

s16 gRaceHudEnabled __attribute__((section(".data.race_hud_enabled"))) = 0;

s16 gRaceHudMode __attribute__((section(".sdata.race_hud_mode"), aligned(4))) = 0;
static u16 raceHudModePadding __attribute__((section(".sdata.race_hud_mode"))) = 0;

RaceTime gCourseCoinTargetTimes[10]
    __attribute__((section(".data.course_coin_target_times"), aligned(4))) = {
        {2, 30, 0}, {2, 30, 0}, {3, 5, 0}, {3, 5, 0}, {3, 5, 0},
        {3, 5, 0}, {3, 5, 0}, {3, 5, 0}, {1, 0, 0}, {1, 0, 0}
    };

extern u8 gModeSequenceStage __attribute__((section(".data")));
extern u8 gControllerPlaybackIndex __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));
extern u8 gSelectedPlayerReplayActive __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern u16 gGameStateFlags __attribute__((section(".data")));

extern u8 gRacePlacementPalettes[4] __attribute__((section(".data")));

u8 gRaceProgressMeterIconUVs[10][2] __attribute__((section(".data.race_progress_icons"), aligned(4))) = {
    {0, 80}, {0, 96}, {0, 112}, {0, 128}, {0, 144},
    {56, 112}, {56, 96}, {56, 128}, {56, 80}, {56, 144},
};

u8 gRaceProgressMeterIconPalettes[12] __attribute__((section(".data.race_progress_icons"), aligned(4))) = {
    8, 9, 8, 9, 8, 9, 13, 13, 8, 8,
};

u8 gRaceProgressMeterAlternatePalettes[12] __attribute__((section(".data.race_progress_icons"), aligned(4))) = {
    0, 1, 0, 1, 0, 1, 2, 2, 0, 0,
};

/* Full-width Shift-JIS labels used by the race HUD. */
const char gRaceRankLabel[12] __attribute__((section(".rodata.race_rank_label"), aligned(4))) =
    _("ＲＡＮＫ");

const char gAttackTimeLimitLabel[24] __attribute__((section(".rodata.trick_attack_labels"), aligned(4))) =
    _("ＴＩＭＥ  ＬＩＭＩＴ");

const char gTrickAttackPointLabel[12] __attribute__((section(".rodata.trick_attack_labels"), aligned(4))) =
    _("ＰＯＩＮＴ");

const char gSplitScreenTimeLimitLabel[12] __attribute__((section(".rodata.trick_attack_labels"), aligned(4))) =
    "TIME LIMIT";

const char gTargetTimeLabel[12] __attribute__((section(".rodata.target_time_label"), aligned(4))) =
    "TARGET TIME";

void drawSinglePlayerRaceHud(void);
void drawSplitScreenCourseProgressHud(void);
void drawSplitScreenRaceHud(s16 player);
void drawSplitScreenTrickAttackHud(s16 player);
void drawScoreAttackHud(void);
void drawSplitScreenScoreAttackHud(s16 player);

#define CAMERA gPlayerCameras[gCurrentTask->data18]
#define PLAYER gRacePlayers[gCurrentTask->data18]
#define VIEW gPlayerCameraViews[gCurrentTask->data18]

void updateCurrentPlayerCameraTask(void) {
    PlayerCamera *shiftedCameras;
    u16 updateIndex;
    s32 cameraValue;
    if (gSelectedPlayerReplayActive == 0) {
        if (gRacePauseState != 0) {
            if (CAMERA.unk16 != 0) {
                CAMERA.unk17 = CAMERA.unk16;
                CAMERA.unk16 = 0;
            }
        }
        if (gRacePauseState == 0) {
            if (CAMERA.unk16 == 0) {
                CAMERA.unk16 = CAMERA.unk17;
            }
        }
    }

    switch (CAMERA.unk16) {
    case 1:
    case 4:
    case 7: {
        s32 height;
        s16 previousRegion;
        PlayerCamera *cameras;
        GsRVIEW2 *views;

        if (!(gGameStateFlags & 0xC00)) {
            CAMERA.unk10 = -320;
        } else {
            CAMERA.unk10 = -576;
        }

        if (PLAYER.unk0BC > 512) {
            s32 dx = PLAYER.x - CAMERA.x;
            s32 dy = PLAYER.y - CAMERA.y;
            s32 dz = PLAYER.z - CAMERA.z;
            s32 scale;

            CAMERA.yaw = getXZHeading(CAMERA.x, CAMERA.z, PLAYER.x, PLAYER.z);
            if (playerCountAbsolute == 1 && PLAYER.unk0BC > 4096) {
                s32 delta = ((u16)CAMERA.yaw - PLAYER.unk0BE) & 0xFFF;
                if ((u32)(delta + 16) > 32) {
                    s16 step;
                    if (delta > 2048) {
                        step = (4096 - delta) / 32;
                        if (step > 8) {
                            step = 8;
                        }
                    } else {
                        step = -(delta / 32);
                        if (step < -8) {
                            step = -8;
                        }
                    }
                    CAMERA.yaw += step;
                }
            }

            dx /= 8;
            dy /= 8;
            dz /= 8;
            scale = 1;
            while (1) {
                if ((u32)(dx + 32767) <= 65534 && (u32)(dz + 32767) <= 65534) {
                    break;
                }
                scale *= 2;
                dx /= 2;
                dz /= 2;
            }
            CAMERA.pitch = getXZHeading(0, 0, SquareRoot0(dx * dx + dz * dz), dy / scale) - 1024;
            if (CAMERA.pitch < -1024) {
                CAMERA.pitch = -1024;
            }
            if (PLAYER.unk062 > 10240 && !(CAMERA.unk1A & 1)) {
                if (PLAYER.unk128 & 0x201) {
                    if (PLAYER.unk062 > 0) {
                        CAMERA.pitch += PLAYER.unk062 >> 10;
                    }
                } else {
                    CAMERA.pitch += PLAYER.unk0BC >> 10;
                }
            }
        }

        switch (CAMERA.unk16) {
        case 1: {
            s16 target = PLAYER.unk0BC / 100;
            s16 step;
            if (target > 64) {
                target = 32;
            }
            step = CAMERA.unk12 / 8;
            if (step <= 0) {
                step = 1;
            }
            if (CAMERA.unk12 < target) {
                CAMERA.unk12 += step;
                if (CAMERA.unk12 > target) {
                    CAMERA.unk12 = target;
                }
            } else if (CAMERA.unk12 > target) {
                CAMERA.unk12 -= step;
                if (CAMERA.unk12 < target) {
                    CAMERA.unk12 = target;
                }
            }
            break;
        }
        case 4:
            if (CAMERA.unk12 < 128) {
                CAMERA.unk12 += PLAYER.unk0BC / 2048;
            }
            break;
        }

        if (PLAYER.unk128 & 0x800) {
            if (CAMERA.unk14 < 96) {
                CAMERA.unk14 += 24;
            }
        } else {
            if (CAMERA.unk14 != 0) {
                CAMERA.unk14 -= 8;
            }
        }

        updatePlayerCameraPosition(gCurrentTask->data18);
        CAMERA.unk18 = PLAYER.unk92E;
        do {
            previousRegion = CAMERA.unk18;
            CAMERA.unk18 = getAdjacentGeometryRegionForPoint(
                previousRegion, VIEW.vpx, VIEW.vpz);
        } while (previousRegion != CAMERA.unk18);

        cameras = gPlayerCameras;
        views = gPlayerCameraViews;
        height = getCourseHeightAtPoint(cameras[gCurrentTask->data18].unk18,
                              views[gCurrentTask->data18].vpx, views[gCurrentTask->data18].vpz);
        if (cameras[gCurrentTask->data18].unk16 != 7) {
            if (height - 120 < views[gCurrentTask->data18].vpy) {
                views[gCurrentTask->data18].vpy = height - 120;
                cameras[gCurrentTask->data18].y = height << 8;
                updatePlayerCameraPitch(gCurrentTask->data18);
                cameras[gCurrentTask->data18].unk1A |= 1;
            } else {
                cameras[gCurrentTask->data18].unk1A &= ~1;
            }
            VIEW.vpy += CAMERA.unk14 / 8;
        } else {
            views[gCurrentTask->data18].vpy = gSelectedCourseRecords[gSelectedCourseIndex].unk0C / 256 - 120;
            cameras[gCurrentTask->data18].y = (views[gCurrentTask->data18].vpy + 120) << 8;
            updatePlayerCameraPitch(gCurrentTask->data18);
            if (views[gCurrentTask->data18].vpy < height - 120) {
                cameras[gCurrentTask->data18].unk16 = 1;
            }
        }
        updateCurrentPlayerCameraTarget();
        if (playerCountAbsolute == 1 && gSelectedCourseIndex == 7 && PLAYER.unk92E < 4) {
            clampPlayerCameraDistance(gCurrentTask->data18);
        }
        break;
    }
    case 3: {
        SelectedCourseRecord *records = gSelectedCourseRecords;
        SelectedCourseRecord *record = &records[gSelectedCourseIndex];
        s32 dx = (s32)record->unk18[6] - CAMERA.x;
        s32 dz = (s32)record->unk18[7] - CAMERA.z;
        CAMERA.x += dx / 50;
        CAMERA.z += dz / 50;
        VIEW.vpx = CAMERA.x / 256;
        VIEW.vpz = CAMERA.z / 256;
        updateIndex = gCurrentTask->data18;
        goto updateAngles;
    }
    case 5: {
        u16 yaw;
        s16 delta;
        cameraValue = (u16)CAMERA.yaw;
        yaw = cameraValue;
        delta = (yaw - PLAYER.unk0B6) & 0xFFF;
        if (delta >= 2048) {
            delta |= 0xF000;
        }
        if (delta > 48) {
            CAMERA.yaw = yaw - 48;
        } else if (delta < -48) {
            CAMERA.yaw = yaw + 48;
        } else {
            CAMERA.yaw = PLAYER.unk0B6;
        }
        if (CAMERA.pitch > 8) {
            CAMERA.pitch -= 8;
        } else if (CAMERA.pitch < -8) {
            CAMERA.pitch += 8;
        } else {
            CAMERA.pitch = 0;
        }
        if (CAMERA.unk12 > -64) {
            CAMERA.unk12 -= 8;
            if (CAMERA.unk12 < -64) {
                CAMERA.unk12 = -64;
            }
        }
        shiftedCameras = gPlayerCameras;
        if (shiftedCameras[gCurrentTask->data18].unk14 != 0) {
            shiftedCameras[gCurrentTask->data18].unk14 -= 8;
        }
        updateIndex = gCurrentTask->data18;
        goto updateShiftedTarget;
    }
    case 6:
        shiftedCameras = gPlayerCameras;
        if (shiftedCameras[gCurrentTask->data18].unk12 < -24) {
            shiftedCameras[gCurrentTask->data18].unk12 += 2;
        }
        if (shiftedCameras[gCurrentTask->data18].pitch > -80) {
            shiftedCameras[gCurrentTask->data18].pitch -= 4;
        }
        shiftedCameras[gCurrentTask->data18].yaw -= 8;
        updateIndex = gCurrentTask->data18;
    updateShiftedTarget:
        updatePlayerCameraPosition(updateIndex);
        updateCurrentPlayerCameraTarget();
        VIEW.vry -= (s16)(shiftedCameras[gCurrentTask->data18].unk12 / 6);
        break;
    case 8:
        cameraValue = CAMERA.x;
        VIEW.vpx = cameraValue / 256;
        VIEW.vpy = CAMERA.y / 256 - 120;
        VIEW.vpz = CAMERA.z / 256;
        updatePlayerCameraAngles(gCurrentTask->data18);
        goto updateTarget;
    updateAngles:
        updatePlayerCameraAngles(updateIndex);
    updateTarget:
        updateCurrentPlayerCameraTarget();
        break;
    case 9:
        updatePlayerCameraPosition(gCurrentTask->data18);
        VIEW.vrx = PLAYER.worldCoordinate.coord.t[0];
        VIEW.vry = PLAYER.worldCoordinate.coord.t[1] - 60;
        VIEW.vrz = PLAYER.worldCoordinate.coord.t[2];
        break;
    }

    if (gCurrentTask->data18 == 0) {
        if (gGameStateFlags & 0x2000) {
            goto stopCamera;
        }
    } else if (!(gGameStateFlags & 0x2400)) {
    stopCamera:
        stopPlayerSound(gCurrentTask->data18);
        deleteCurrentTaskSubtree();
        return;
    }
    renderCurrentCameraPlayersAndUpdateSound();
}

#undef CAMERA
#undef PLAYER
#undef VIEW

void addFourWordGpuPacket(GpuPacket4 *source, GsOT *ot, u16 depth) {
    GpuPacket4 *packet = (GpuPacket4 *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    GsSetWorkBase((PACKET *)(packet + 1));
}

void addOneWordGpuPacket(GpuPacket1 *source, GsOT *ot, u16 depth) {
    GpuPacket1 *packet = (GpuPacket1 *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    GsSetWorkBase((PACKET *)(packet + 1));
}

void addTexturedSpritePacket(TexturedSpritePacket *source, GsOT *ot, u16 depth) {
    TexturedSpritePacket *packet = (TexturedSpritePacket *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    GsSetWorkBase((PACKET *)(packet + 1));
}

void drawRacePlacementSprite(s16 x, s16 y, s16 placement, GsOT *ot, u16 depth) {
    u32 packetDepth = (u16)depth;

    /* Fourth place uses the byte immediately after the first palette group. */
    drawSpriteByIndexWithBlend(x, y, placement + 0x1D4, ((const u8 *)&gMenuSpritePaletteGroups)[placement],
                  -1, ot, packetDepth);
}

void drawMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, GsOT *ot, u16 depth) {
    s32 frameIndex = frame;
    s32 sprite;
    if (kind == 0) {
        frame = 5;
        sprite = 0x1BE;
    } else {
        s32 baseSprite = 0x1BE;
        u8 *palettes = gMenuSpritePaletteGroups.kind;
        frame = palettes[kind];
        sprite = (kind - 1) * 3 + (frameIndex + baseSprite);
    }
    drawSpriteByIndexWithBlend(x, y, (s16)sprite, frame, -1, ot, depth);
}

void drawScaledMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, s16 scaleX, s16 scaleY, GsOT *ot, u16 depth) {
    s32 frameIndex = frame;
    s32 sprite;
    if (kind == 0) {
        frame = 5;
        sprite = 0x1BE;
    } else {
        s32 baseSprite = 0x1BE;
        u8 *palettes = gMenuSpritePaletteGroups.kind;
        frame = palettes[kind];
        sprite = (kind - 1) * 3 + (frameIndex + baseSprite);
    }
    if (scaleX > 0 && scaleY > 0) {
        drawTransformedSpriteByIndex((s16)(x + 16), (s16)(y + 16), (s16)sprite, frame, -1,
                     0x7FFF, 0x7FFF, scaleX, scaleY, 0, ot, depth);
    } else {
        drawSpriteByIndexWithBlend(x, y, (s16)sprite, frame, -1, ot, depth);
    }
}

void drawPaletteMappedIcon(s16 x, s16 y, s16 index, struct GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        u8 *palettes = gMenuSpritePaletteGroups.mappedIconPalettes;
        palette = palettes[index];
        sprite = index + 461;
    }
    drawSpriteByIndexWithBlend(x, y, sprite, palette, -1, ot, depth);
}

void drawScaledPaletteMappedIcon(s16 x, s16 y, s16 index, s16 scaleX, s16 scaleY, GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        u8 *palettes = gMenuSpritePaletteGroups.mappedIconPalettes;
        palette = palettes[index];
        sprite = index + 461;
    }
    if (scaleX > 0 && scaleY > 0) {
        drawTransformedSpriteByIndex((s16)(x + 16), (s16)(y + 16), sprite, palette, -1,
                       0x7FFF, 0x7FFF, scaleX, scaleY, 0, ot, depth);
    } else {
        drawSpriteByIndexWithBlend(x, y, sprite, palette, -1, ot, depth);
    }
}

void unpackUpperWordBytes(u32 packed, UnpackedWordBytes *out) {
    u32 byte;

    packed >>= 8;
    byte = packed & 0xFF;
    packed >>= 8;
    out->unk1 = packed;
    packed >>= 8;
    out->unk2 = byte;
    out->unk0 = packed;
}

void drawRaceCourseProgressMeter(s16 x, s16 y, GsOT *ot, s32 depth) {
    CourseProgressScratch *scratch = (CourseProgressScratch *)getScratchAddr(0);
    s16 i = 0;
    s16 j;
    DrawModeSpritePacket sprite;
    DrawModeSpritePacket *packet;
    u16 packetDepth;
    s16 iconSize;

    scratch->playerOrder[0] = 0;
    scratch->playerOrder[1] = 1;
    scratch->playerOrder[2] = 2;
    scratch->playerOrder[3] = 3;
    packetDepth = depth;
    for (; i < gActiveRacePlayerCount - 1; i++) {
        for (j = i + 1; j < gActiveRacePlayerCount; j++) {
            if (gRacePlayers[scratch->playerOrder[i]].unk935 >
                gRacePlayers[scratch->playerOrder[j]].unk935) {
                s16 swap = scratch->playerOrder[i];
                scratch->playerOrder[i] = scratch->playerOrder[j];
                scratch->playerOrder[j] = swap;
            }
        }
    }

    j = 0;
    packet = &sprite;
    sprite.tag = 5u << 24;
    packet->tpage = getTPage(0, 0, 832, 256);
    setRGB0(packet, 128, 128, 128);
    packet->code = TEXTURED_SPRITE_CODE;
    packet->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    if (j < gActiveRacePlayerCount) {
        do {
            RacePlayer *player;
            s16 progress;
            u8 *uv;
            u8 character;
            s32 squashed;

            i = scratch->playerOrder[j];
            squashed = gRacePlayers[i].unk128 & 0x200000;
            iconSize = 16;

            if (squashed || gRacePlayers[i].progressMeterSquashFrame) {
                gRacePlayers[i].progressMeterSquashFrame++;
            }
            if ((gRacePlayers[i].unk128 & 0x200000) && gRacePlayers[i].progressMeterSquashFrame > 4) {
                gRacePlayers[i].progressMeterSquashFrame = 4;
            }
            player = &gRacePlayers[i];
            if (player->progressMeterSquashFrame > 5) {
                player->progressMeterSquashFrame = 0;
            }

            packet->x0 = x - 4;
            getPlayerCourseProgress(i, &scratch->progress, &scratch->distance);
            progress = (scratch->progress << 7) / ((s16)gSelectedCourseRecords[gSelectedCourseIndex].unk00 << 3);
            if (progress < 0) {
                progress = 0;
            }
            if (progress > 128) {
                progress = 128;
            }
            packet->y0 = (s16)(progress - 6) + y;

            character = player->characterId;
            player->progressMeterPosition = progress;
            uv = gRaceProgressMeterIconUVs[character];
            if (player->progressMeterSquashFrame > 0) {
                if (player->progressMeterSquashFrame < 3) {
                    setUV0(packet, uv[0] + 36, uv[1]);
                    setWH(packet, 20, iconSize);
                } else {
                    setUV0(packet, uv[0] + 16, uv[1]);
                    setWH(packet, 20, iconSize);
                }
            } else {
                setUV0(packet, uv[0], uv[1]);
                setWH(packet, iconSize, iconSize);
            }
            if (gRacePlayers[i].unk0FA) {
                packet->clut = getClut(784, gRaceProgressMeterAlternatePalettes[character] + 240);
            } else {
                packet->clut = getClut(800, gRaceProgressMeterIconPalettes[character] + 240);
            }
            addTexturedSpritePacket((TexturedSpritePacket *)packet, ot, packetDepth);
        } while (++j < gActiveRacePlayerCount);
    }
    drawSpriteByIndexWithBlend(x, y, 0x1DE, 15, -1, ot, packetDepth);
}

void initializeRaceRulesAndCounters(void) {
    RaceTime *times;
    u8 *flags;
    gRaceHudMode = -1;
    switch (gMainMenuSelectionIndex) {
    case 0:
        if (playerCountAbsolute == 1) {
            gRaceHudMode = 0;
        } else {
            gRaceHudMode = 1;
        }
        break;
    case 1:
        if (gRaceRuleset == 0) {
            if (playerCountAbsolute == 1) {
                gRaceHudMode = 2;
            } else {
                gRaceHudMode = 3;
            }
            gRacePlayers[0].collectedCourseCoinMarkerCount = 0;
            gRacePlayers[0].totalCourseCoinMarkerCount = gCourseCoinMarkerTargets[gSelectedCourseIndex];
            gRacePlayers[1].collectedCourseCoinMarkerCount = 0;
            gRacePlayers[1].totalCourseCoinMarkerCount = gCourseCoinMarkerTargets[gSelectedCourseIndex];
        } else if (gRaceRuleset == 1) {
            if (playerCountAbsolute == 1) {
                gRaceHudMode = 4;
                gAttackTimeRemaining = gScoreAttackTimeLimits[gSelectedCourseIndex];
            } else {
                gRaceHudMode = 5;
                gAttackTimeRemaining = gScoreAttackTimeLimits[gSelectedCourseIndex];
                gPlayerAttackTimesRemaining[0] = gScoreAttackTimeLimits[gSelectedCourseIndex];
                gPlayerAttackTimesRemaining[1] = gScoreAttackTimeLimits[gSelectedCourseIndex];
            }
            gRacePlayers[0].scoreAttackPointTotal = 0;
            gRacePlayers[0].scoreAttackPointTarget = gCourseScoreAttackPointTargets[gSelectedCourseIndex];
            gRacePlayers[1].scoreAttackPointTotal = 0;
            gRacePlayers[1].scoreAttackPointTarget = gCourseScoreAttackPointTargets[gSelectedCourseIndex];
        } else if (gRaceRuleset == 2) {
            if (playerCountAbsolute == 1) {
                gRaceHudMode = 6;
                gAttackTimeRemaining = gTrickAttackTimeLimit;
            } else {
                gRaceHudMode = 7;
                gAttackTimeRemaining = gTrickAttackTimeLimit;
                gPlayerAttackTimesRemaining[0] = gTrickAttackTimeLimit;
                gPlayerAttackTimesRemaining[1] = gTrickAttackTimeLimit;
            }
        }
        break;
    case 2:
        gRaceHudMode = 8;
        break;
    }
    if (gModeSequenceStage != 0 || gControllerPlaybackIndex != 0) {
        gRaceHudMode = 9;
    }
    flags = gPlayerAttackTimeExpired;
    flags[0] = 0;
    gRaceElapsedTime.minutes = 0;
    gRaceElapsedTime.seconds = 0;
    gRaceElapsedTime.centiseconds = 0;
    gLapSplitTimeCaptured = 0;
    gAttackTimeExpired_gp = 0;
    raceHudEnabledGp = 0;
    flags[1] = 0;
    times = gPlayerRaceTimes;
    times[0].minutes = 0;
    times[0].seconds = 0;
    times[0].centiseconds = 0;
    times[1].minutes = 0;
    times[1].seconds = 0;
    times[1].centiseconds = 0;
}

void updateRaceHud(void) {
    RaceTime difference;
    s32 progress;
    s32 distance;
    s16 player;

    if (raceHudEnabledGp) {
        switch (gRaceHudMode) {
        case 0:
            if (gRacePlayers[0].unk93F) {
                if (gRacePlayers[0].unk93F == 4) {
                    createPlayerSpriteAnimationTask(-32, -96, 0, 0, 0);
                }
                gRacePlayers[0].unk93F--;
            }
            if (gRacePlayers[0].unk942) {
                if (gRacePlayers[0].unk942 == 4) {
                    createPlayerSpriteAnimationTask(0, -96, 0, 0, 1);
                }
                gRacePlayers[0].unk942--;
            }
            drawSinglePlayerRaceHud();
            break;
        case 1:
            for (player = 0; player < 2; player++) {
                if (gRacePlayers[player].unk93F) {
                    if (gRacePlayers[player].unk93F == 4) {
                        createPlayerSpriteAnimationTask(-104, -40, player, 1, 0);
                    }
                    gRacePlayers[player].unk93F--;
                }
                if (gRacePlayers[player].unk942) {
                    if (gRacePlayers[player].unk942 == 4) {
                        createPlayerSpriteAnimationTask(-72, -40, player, 1, 1);
                    }
                    gRacePlayers[player].unk942--;
                }
            }
            drawSplitScreenCourseProgressHud();
            drawSplitScreenRaceHud(0);
            drawSplitScreenRaceHud(1);
            break;
        case 2:
            if (!(gTaskTransitionFlags & 3) && !(gRacePlayers[0].unk128 & 0x40)) {
                advanceRaceElapsedTime();
            }
            if (calculateRaceTimeDifference(&gCourseCoinTargetTimes[gSelectedCourseIndex], &gRaceElapsedTime, &difference)) {
                gAttackTimeExpired_gp = 1;
            }
            drawCourseCoinHud();
            break;
        case 3:
            if (!(gTaskTransitionFlags & 3)) {
                advanceRaceElapsedTime();
                if (!(gRacePlayers[0].unk128 & 0x40)) {
                    gPlayerRaceTimes[0] = gRaceElapsedTime;
                    if (calculateRaceTimeDifference(&gCourseCoinTargetTimes[gSelectedCourseIndex], &gPlayerRaceTimes[0], &difference)) {
                        gPlayerAttackTimeExpired[0] = 1;
                    }
                }
                if (!(gRacePlayers[1].unk128 & 0x40)) {
                    gPlayerRaceTimes[1] = gRaceElapsedTime;
                    if (calculateRaceTimeDifference(&gCourseCoinTargetTimes[gSelectedCourseIndex], &gPlayerRaceTimes[1], &difference)) {
                        gPlayerAttackTimeExpired[1] = 1;
                    }
                }
            }
            drawSplitScreenCourseCoinHud(0);
            drawSplitScreenCourseCoinHud(1);
            break;
        case 4:
            if (!(gTaskTransitionFlags & 3) && !(gRacePlayers[0].unk128 & 0x40)) {
                decrementRaceCountdown();
            }
            if (!gAttackTimeRemaining.minutes) {
                if (!gAttackTimeRemaining.seconds) {
                    if (!gAttackTimeRemaining.centiseconds) {
                        gAttackTimeExpired_gp = 1;
                    }
                }
            }
            drawScoreAttackHud();
            break;
        case 5:
            if (!(gTaskTransitionFlags & 3)) {
                decrementRaceCountdown();
                advanceRaceElapsedTime();
                if (!(gRacePlayers[0].unk128 & 0x40)) {
                    gPlayerAttackTimesRemaining[0] = gAttackTimeRemaining;
                    if (!gPlayerAttackTimesRemaining[0].minutes) {
                        if (!gPlayerAttackTimesRemaining[0].seconds) {
                            if (!gPlayerAttackTimesRemaining[0].centiseconds) {
                                gPlayerAttackTimeExpired[0] = 1;
                            }
                        }
                    }
                    gPlayerRaceTimes[0] = gRaceElapsedTime;
                }
                if (!(gRacePlayers[1].unk128 & 0x40)) {
                    gPlayerAttackTimesRemaining[1] = gAttackTimeRemaining;
                    if (!gPlayerAttackTimesRemaining[1].minutes) {
                        if (!gPlayerAttackTimesRemaining[1].seconds) {
                            if (!gPlayerAttackTimesRemaining[1].centiseconds) {
                                gPlayerAttackTimeExpired[1] = 1;
                            }
                        }
                    }
                    gPlayerRaceTimes[1] = gRaceElapsedTime;
                }
            }
            drawSplitScreenScoreAttackHud(0);
            drawSplitScreenScoreAttackHud(1);
            break;
        case 6:
            if (!(gTaskTransitionFlags & 3) && !(gRacePlayers[0].unk128 & 0x40)) {
                decrementRaceCountdown();
            }
            if (!gAttackTimeRemaining.minutes) {
                if (!gAttackTimeRemaining.seconds) {
                    if (!gAttackTimeRemaining.centiseconds) {
                        gAttackTimeExpired_gp = 1;
                    }
                }
            }
            drawTrickAttackHud();
            break;
        case 7:
            if (!(gTaskTransitionFlags & 3)) {
                decrementRaceCountdown();
                advanceRaceElapsedTime();
                if (!(gRacePlayers[0].unk128 & 0x40)) {
                    gPlayerAttackTimesRemaining[0] = gAttackTimeRemaining;
                    if (!gPlayerAttackTimesRemaining[0].minutes) {
                        if (!gPlayerAttackTimesRemaining[0].seconds) {
                            if (!gPlayerAttackTimesRemaining[0].centiseconds) {
                                gPlayerAttackTimeExpired[0] = 1;
                            }
                        }
                    }
                    gPlayerRaceTimes[0] = gRaceElapsedTime;
                }
                if (!(gRacePlayers[1].unk128 & 0x40)) {
                    gPlayerAttackTimesRemaining[1] = gAttackTimeRemaining;
                    if (!gPlayerAttackTimesRemaining[1].minutes) {
                        if (!gPlayerAttackTimesRemaining[1].seconds) {
                            if (!gPlayerAttackTimesRemaining[1].centiseconds) {
                                gPlayerAttackTimeExpired[1] = 1;
                            }
                        }
                    }
                    gPlayerRaceTimes[1] = gRaceElapsedTime;
                }
            }
            drawSplitScreenTrickAttackHud(0);
            drawSplitScreenTrickAttackHud(1);
            break;
        case 8:
            if (!(gTaskTransitionFlags & 3)) {
                if (!(gRacePlayers[0].unk128 & 0x40)) {
                    advanceRaceElapsedTime();
                }
                if (!gLapSplitTimeCaptured) {
                    getPlayerCourseProgress(0, &progress, &distance);
                    if (gSelectedCourseRecords[gSelectedCourseIndex].unk3C * 8 < progress) {
                        gLapSplitTime = gRaceElapsedTime;
                        gLapSplitTimeCaptured = 1;
                        createChildTaskWithReserve(initializeRaceTimeDifferenceDisplay);
                    }
                }
            }
            drawLapTimeHud();
            break;
        case 9:
            drawFirstPlayerStatusHud();
            break;
        }
    }
}

void drawSinglePlayerRaceHud(void) {
    GsOT *ot = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
    RacePlayer *player;
    s16 scale;
    RacePlayer *firstPlayer;
    s32 timer;
    s16 frame;
    s32 palette;
    s32 money;
    s32 rank;
    u16 depth = 1;

    setSpriteColor(128, 128, 128);
    timer = gRacePlayers[0].unk93F;
    if (timer != 0) {
        scale = 4096 / (timer + 1);
    } else {
        scale = 0;
    }
    firstPlayer = gRacePlayers;
    drawScaledMenuKindSprite(-32, -96, firstPlayer->unk93E, firstPlayer->unk940,
                             scale, scale, ot, depth);
    timer = firstPlayer->unk942;
    if (timer != 0) {
        scale = 4096 / (timer + 1);
    } else {
        scale = 0;
    }
    player = gRacePlayers;
    drawScaledPaletteMappedIcon(0, -96, (s8)player->heldItem, scale, scale, ot, depth);
    rank = player->unk935;
    drawSpriteByIndexWithBlend(-136, 64, rank + 468, gRacePlacementPalettes[rank], -1, ot, depth);
    drawWrappedText8x8(-136, 54, 4, gRaceRankLabel, 4, -1, ot, depth);
    drawSpriteByIndexWithBlend(70, -96, 434, 3, -1, ot, depth);
    drawHudDecimalNumber8x16(104, -96, 1, 0, player->unk934 + 1, 3, -1, ot, depth);
    drawSpriteByIndexWithBlend(116, -96, 27, 4, -1, ot, depth);
    drawHudDecimalNumber8x16(128, -96, 1, 0, selectedFinishLapCountAbsolute, 3, -1, ot, depth);
    drawRaceCourseProgressMeter(124, -68, ot, depth);
    frame = (s16)gFrameCounter % 12;
    drawSpriteByIndexWithBlend(120, 80, frame / 2 + 472, 7, -1, ot, depth);
    money = player->unk01C;
    palette = money < 100 ? 2 : 3;
    drawHudDecimalNumber8x16(78, 80, 5, 0, money, palette, -1, ot, depth);
}

void drawSplitScreenCourseProgressHud(void) {
    GsOT *ot = &gMainOrderingTables[gActiveGraphicsBuffer];

    setSpriteColor(128, 128, 128);
    drawRaceCourseProgressMeter(81, -68, ot, 1);
}

void drawSplitScreenRaceHud(s16 player) {
    GsOT *hudOt = &gMainOrderingTables[gActiveGraphicsBuffer];
    GsOT *playerOt;
    s32 hudY;
    s32 depth;
    s16 placement;
    s16 panelOffset;
    s8 timer;
    s16 scale;
    s16 frame;
    s32 moneyPalette;

    if (player == 0) {
        playerOt = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
        hudY = 0;
    } else {
        playerOt = &gPlayer2OrderingTables[gActiveGraphicsBuffer];
        hudY = 105;
    }

    setSpriteColor(128, 128, 128);
    drawDecimalNumber16(107, hudY - 73, 1, 0, gRacePlayers[player].unk934 + 1, 8, 1, hudOt, 1);
    drawSpriteByIndexWithBlend(123, hudY - 75, 481, 8, 1, hudOt, 1);
    drawDecimalNumber16(132, hudY - 77, 1, 0, selectedFinishLapCountAbsolute, 8, 1, hudOt, 1);
    drawSpriteByIndexWithBlend(102, hudY - 103, 416, 11, -1, hudOt, 1);

    depth = 1;
    if (gTaskTransitionFlags & 1) {
        placement = 0;
    } else {
        placement = gRacePlayers[player].unk935;
    }
    drawSpriteByIndexWithBlend(110, hudY - 40, placement + 468, gRacePlacementPalettes[placement],
                               -1, hudOt, depth);
    drawSpriteByIndexWithBlend(102, hudY - 56, 415, 10, 0, hudOt, depth);
    if (player == 1) {
        hudY += 18;
    }
    panelOffset = player + 121;
    drawSpriteByIndexWithBlend(96, hudY - panelOffset, 417, 12, -1, hudOt, depth);

    if (shouldRenderPlayerView(player)) {
        timer = (s8)gRacePlayers[player].unk93F;
        scale = timer ? 0x1000 / (timer + 1) : 0;
        drawScaledMenuKindSprite(-112, -40, gRacePlayers[player].unk93E,
                                 gRacePlayers[player].unk940, scale, scale, playerOt, depth);
        timer = (s8)gRacePlayers[player].unk942;
        scale = timer ? 0x1000 / (timer + 1) : 0;
        drawScaledPaletteMappedIcon(-80, -40, (s8)gRacePlayers[player].heldItem,
                                    scale, scale, playerOt, depth);
        frame = (s16)gFrameCounter % 12;
        drawSpriteByIndexWithBlend(-68, 36, frame / 2 + 472, 7, -1, playerOt, depth);
        moneyPalette = gRacePlayers[player].unk01C < 100 ? 2 : 3;
        drawHudDecimalNumber8x16(-110, 36, 5, 0, gRacePlayers[player].unk01C,
                                 moneyPalette, -1, playerOt, depth);
    }
}

void drawTrickAttackHud(void) {
    s16 palette = 1;
    GsOT *ot = &gMainOrderingTables[gActiveGraphicsBuffer];
    u16 depth = palette;
    s16 x;
    s16 i;

    if (gAttackTimeRemaining.minutes == 0 && gAttackTimeRemaining.seconds < 10 && (gFrameCounter & 1)) {
        palette = 2;
    }

    setSpriteColor(128, 128, 128);
    if (gAttackTimeExpired_gp) {
        drawSpriteByIndexWithBlend(64, 84, 398, 5, -1, ot, depth);
        palette = 2;
    }
    drawHudDecimalNumber8x16(64, 80, 2, 1, gAttackTimeRemaining.minutes, palette, -1, ot, depth);
    drawHudDecimalNumber8x16(88, 80, 2, 1, gAttackTimeRemaining.seconds, palette, -1, ot, depth);
    drawHudDecimalNumber8x16(112, 80, 2, 1, gAttackTimeRemaining.centiseconds >> 8, palette, -1, ot, depth);
    drawSpriteByIndexWithBlend(80, 80, 25, palette, -1, ot, depth);
    drawSpriteByIndexWithBlend(104, 80, 24, palette, -1, ot, depth);
    drawWrappedText8x8(56, 71, 10, gAttackTimeLimitLabel, 5, -1, ot, depth);

    if (gRacePlayers[0].trickAttackMakeCount < 10) {
        x = -112;
        for (i = 0; i < gRacePlayers[0].trickAttackMakeCount; i++) {
            drawTransformedSpriteByIndex(x, -72, 431, 13, -1, 0x7FFF, 0x7FFF, 0x555, 0x555, 0, ot, depth);
            x += 8;
        }
    } else {
        drawSpriteByIndexWithBlend(-132, -96, 431, 13, -1, ot, depth);
        drawHudDecimalNumber8x16(-100, -89, 2, 0, gRacePlayers[0].trickAttackMakeCount, 1, -1, ot, depth);
    }

    drawHudDecimalNumber8x16(92, -88, 5, 0, gRacePlayers[0].trickAttackPointTotal, 1, -1, ot, depth);
    drawWrappedText8x8(96, -97, 5, gTrickAttackPointLabel, 5, -1, ot, depth);
    drawSpriteByIndexWithBlend(-112, 64, 422, 2, -1, ot, depth);
    drawSpriteByIndexWithBlend(-136, 64, 419, 14, -1, ot, depth);
}

void drawSplitScreenTrickAttackHud(s16 player) {
    GsOT *hudOt = &gMainOrderingTables[gActiveGraphicsBuffer];
    GsOT *timeOt;
    s32 hudY;
    s16 timeY;
    s16 palette;
    u16 depth[1] = {1};
    u8 padding[0x10]; /* Preserve the original stack frame. */

    if (player == 0) {
        timeOt = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
        hudY = 0;
        timeY = 0;
    } else {
        timeOt = &gPlayer2OrderingTables[gActiveGraphicsBuffer];
        hudY = 104;
        timeY = 0;
    }

    palette = 1;
    if (gPlayerAttackTimesRemaining[player].minutes == 0 && gPlayerAttackTimesRemaining[player].seconds < 10 && ((gFrameCounter >> 1) & 1)) {
        palette = 2;
    }

    setSpriteColor(128, 128, 128);
    if (shouldRenderPlayerView(player)) {
        if (gPlayerAttackTimeExpired[player]) {
            drawSpriteByIndexWithBlend(-112, timeY + 39, 398, 5, -1, timeOt, depth[0]);
            palette = 2;
        }
        drawHudDecimalNumber8x16(-112, timeY + 36, 2, 1, gPlayerAttackTimesRemaining[player].minutes, palette, -1, timeOt, depth[0]);
        drawHudDecimalNumber8x16(-88, timeY + 36, 2, 1, gPlayerAttackTimesRemaining[player].seconds, palette, -1, timeOt, depth[0]);
        drawHudDecimalNumber8x16(-64, timeY + 36, 2, 1, gPlayerAttackTimesRemaining[player].centiseconds >> 8, palette, -1, timeOt, depth[0]);
        drawSpriteByIndexWithBlend(-96, timeY + 36, 25, palette, -1, timeOt, depth[0]);
        drawSpriteByIndexWithBlend(-72, timeY + 36, 24, palette, -1, timeOt, depth[0]);
        drawLimitedText8x8(-112, timeY + 28, 11, gSplitScreenTimeLimitLabel, 5, -1, timeOt, depth[0]);
    }

    drawHudDecimalNumber8x16(104, hudY - 88, 5, 0, gRacePlayers[player].trickAttackPointTotal, 1, -1, hudOt, depth[0]);
    drawLimitedText8x8(104, hudY - 96, 5, "POINT", 5, -1, hudOt, depth[0]);
    drawTransformedSpriteByIndex(135, (s16)(hudY - 24), 422, 2, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth[0]);
    drawTransformedSpriteByIndex(120, (s16)(hudY - 24), 419, 14, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth[0]);
    if (player == 1) {
        hudY += 18;
    }
    drawSpriteByIndexWithBlend(96, hudY - 121, 417, 12, -1, hudOt, depth[0]);
}

void drawScoreAttackHud(void) {
    s16 palette = 1;
    GsOT *ot = &gMainOrderingTables[gActiveGraphicsBuffer];
    u16 depth = palette;
    s32 xOffset;
    s16 i;
    char text[16];

    if (gAttackTimeRemaining.minutes == 0 && gAttackTimeRemaining.seconds < 10 && (gFrameCounter & 1)) {
        palette = 2;
    }

    setSpriteColor(128, 128, 128);
    if (gAttackTimeExpired_gp) {
        drawSpriteByIndexWithBlend(64, 84, 398, 5, -1, ot, depth);
        palette = 2;
    }
    drawHudDecimalNumber8x16(64, 80, 2, 1, gAttackTimeRemaining.minutes, palette, -1, ot, depth);
    drawHudDecimalNumber8x16(88, 80, 2, 1, gAttackTimeRemaining.seconds, palette, -1, ot, depth);
    drawHudDecimalNumber8x16(112, 80, 2, 1, gAttackTimeRemaining.centiseconds >> 8, palette, -1, ot, depth);
    drawSpriteByIndexWithBlend(80, 80, 25, palette, -1, ot, depth);
    drawSpriteByIndexWithBlend(104, 80, 24, palette, -1, ot, depth);
    drawWrappedText8x8(56, 71, 10, gAttackTimeLimitLabel, 5, -1, ot, depth);

    i = 0;
    if (gRacePlayers[0].unk93C > 0) {
        xOffset = 0;
        do {
            drawSpriteByIndexWithBlend(xOffset - 136, -96, 445, 13, -1, ot, depth);
            xOffset += 8;
            i++;
        } while (i < gRacePlayers[0].unk93C);
    }

    drawSpriteByIndexWithBlend(-112, 64, 424, 0, -1, ot, depth);
    drawSpriteByIndexWithBlend(-136, 64, 421, 14, -1, ot, depth);
    drawHudDecimalNumber8x16(96, -80, 2, 0, gRacePlayers[0].scoreAttackPointTotal, 1, -1, ot, depth);
    sprintf(text, "/%d", gRacePlayers[0].scoreAttackPointTarget);
    drawLimitedText8x8(112, -72, 10, text, 6, -1, ot, depth);
    drawSpriteByIndexWithBlend(104, -96, 432, 0, -1, ot, depth);
}

void drawSplitScreenScoreAttackHud(s16 player) {
    GsOT *timeOt;
    s16 hudY;
    u8 padding[0x10]; /* Preserve the original stack frame. */
    s16 timeY;
    GsOT *hudOt = &gMainOrderingTables[gActiveGraphicsBuffer];
    s16 palette;
    s32 iconX;
    u16 depth[1] = {1};
    u16 fixedDepth = 1;

    if (player == 0) {
        timeOt = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
        timeY = hudY = 0;
    } else {
        timeOt = &gPlayer2OrderingTables[gActiveGraphicsBuffer];
        hudY = 104;
        timeY = 0;
    }
    palette = 1;
    if (gPlayerAttackTimesRemaining[player].minutes == 0 && gPlayerAttackTimesRemaining[player].seconds < 10 &&
        ((gFrameCounter >> 1) & 1)) {
        palette = 2;
    }

    setSpriteColor(128, 128, 128);
    if (shouldRenderPlayerView(player)) {
        s16 i;
        s16 timeDigitsY;
        if (gPlayerAttackTimeExpired[player]) {
            drawSpriteByIndexWithBlend(-112, timeY + 39, 398, 5, -1, timeOt, fixedDepth);
            palette = 2;
        }

        drawHudDecimalNumber8x16(-112, (timeDigitsY = timeY + 36), 2, 1,
                                gPlayerAttackTimesRemaining[player].minutes, palette, -1, timeOt, depth[0]);
        iconX = -112;
        drawHudDecimalNumber8x16(-88, timeDigitsY, 2, 1, gPlayerAttackTimesRemaining[player].seconds, palette, -1, timeOt, depth[0]);
        drawHudDecimalNumber8x16(-64, timeDigitsY, 2, 1, gPlayerAttackTimesRemaining[player].centiseconds >> 8, palette, -1, timeOt, depth[0]);
        drawSpriteByIndexWithBlend(-96, timeDigitsY, 25, palette, -1, timeOt, depth[0]);
        drawSpriteByIndexWithBlend(-72, timeDigitsY, 24, palette, -1, timeOt, depth[0]);
        drawLimitedText8x8(iconX, timeY + 28, 11, gSplitScreenTimeLimitLabel, 5, -1, timeOt, depth[0]);

        for (i = 0; i < gRacePlayers[player].unk93C; i++) {
            drawSpriteByIndexWithBlend(iconX, -36, 445, 13, -1, timeOt, fixedDepth);
            iconX += 8;
        }
    }

    drawTransformedSpriteByIndex(112, (s16)(hudY - 80), 432, 0, -1, 0x7FFF, 0x7FFF,
                                 0xC00, 0xC00, 0, hudOt, depth[0]);
    drawLimitedText8x8(124, hudY - 76, 1, "*", 6, -1, hudOt, depth[0]);
    drawHudDecimalNumber8x16(132, hudY - 84, 2, 0,
                            gRacePlayers[player].scoreAttackPointTotal, 1, -1, hudOt, depth[0]);
    timeY = hudY - 24;
    drawTransformedSpriteByIndex(135, timeY, 424, 0, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth[0]);
    drawTransformedSpriteByIndex(120, timeY, 421, 14, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth[0]);
    if (player == 1) {
        hudY += 18;
    }
    drawSpriteByIndexWithBlend(96, hudY - 121, 417, 12, -1, hudOt, depth[0]);
}

void drawCourseCoinHud(void) {
    GsOT *ot = &gMainOrderingTables[gActiveGraphicsBuffer];
    char text[16];
    s16 digits;

    setSpriteColor(128, 128, 128);
    if (gAttackTimeExpired_gp) {
        drawSpriteByIndexWithBlend(72, 84, 398, 5, -1, ot, 1);
    }
    drawHudDecimalNumber8x16(72, 80, 2, 1, gRaceElapsedTime.minutes, 1, -1, ot, 1);
    drawHudDecimalNumber8x16(96, 80, 2, 1, gRaceElapsedTime.seconds, 1, -1, ot, 1);
    drawHudDecimalNumber8x16(120, 80, 2, 1, gRaceElapsedTime.centiseconds >> 8, 1, -1, ot, 1);
    drawSpriteByIndexWithBlend(88, 80, 25, 1, -1, ot, 1);
    drawSpriteByIndexWithBlend(112, 80, 24, 1, -1, ot, 1);
    drawLimitedText8x8(72, 71, 10, gLapTimeLabel, 5, -1, ot, 1);

    sprintf(text, gLapTimeFormat, gCourseCoinTargetTimes[gSelectedCourseIndex].minutes,
            gCourseCoinTargetTimes[gSelectedCourseIndex].seconds, gCourseCoinTargetTimes[gSelectedCourseIndex].centiseconds >> 8);
    drawLimitedText8x8(72, -87, 20, text, 7, -1, ot, 1);
    drawLimitedText8x8(50, -96, 11, gTargetTimeLabel, 7, -1, ot, 1);

    drawPaletteMappedIcon(-136, -96, 1, ot, 1);
    sprintf(text, "%d", gRacePlayers[0].collectedCourseCoinMarkerCount);
    digits = strlen(text);
    drawLimitedText8x8(-104, -72, 1, "*", 6, -1, ot, 1);
    drawHudDecimalNumber8x16(-96, -80, digits, 0, gRacePlayers[0].collectedCourseCoinMarkerCount, 1, -1, ot, 1);
    sprintf(text, "/%d", gRacePlayers[0].totalCourseCoinMarkerCount);
    drawLimitedText8x8(-96 + digits * 8, -72, 10, text, 6, -1, ot, 1);

    drawSpriteByIndexWithBlend(-112, 64, 423, 3, -1, ot, 1);
    drawSpriteByIndexWithBlend(-136, 64, 420, 14, -1, ot, 1);
}

void drawSplitScreenCourseCoinHud(s16 player) {
    GsOT *hudOt = &gMainOrderingTables[gActiveGraphicsBuffer];
    GsOT *timeOt;
    s32 hudY;
    s16 timeY;
    u16 depth;
    char text[16];

    if (player == 0) {
        timeOt = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
        hudY = 0;
        timeY = 0;
    } else {
        timeOt = &gPlayer2OrderingTables[gActiveGraphicsBuffer];
        hudY = 104;
        timeY = 0;
    }

    depth = 1;
    setSpriteColor(128, 128, 128);
    if (shouldRenderPlayerView(player)) {
        if (gPlayerAttackTimeExpired[player]) {
            drawSpriteByIndexWithBlend(-112, timeY + 39, 398, 5, -1, timeOt, 1);
        }
        drawHudDecimalNumber8x16(-112, timeY + 36, 2, 1, gPlayerRaceTimes[player].minutes, 1, -1, timeOt, 1);
        drawHudDecimalNumber8x16(-88, timeY + 36, 2, 1, gPlayerRaceTimes[player].seconds, 1, -1, timeOt, 1);
        drawHudDecimalNumber8x16(-64, timeY + 36, 2, 1, gPlayerRaceTimes[player].centiseconds >> 8, 1, -1, timeOt, 1);
        drawSpriteByIndexWithBlend(-96, timeY + 36, 25, 1, -1, timeOt, 1);
        drawSpriteByIndexWithBlend(-72, timeY + 36, 24, 1, -1, timeOt, 1);
        drawLimitedText8x8(-112, timeY + 28, 10, gLapTimeLabel, 5, -1, timeOt, 1);
        sprintf(text, gLapTimeFormat, gCourseCoinTargetTimes[gSelectedCourseIndex].minutes,
                gCourseCoinTargetTimes[gSelectedCourseIndex].seconds, gCourseCoinTargetTimes[gSelectedCourseIndex].centiseconds >> 8);
        drawLimitedText8x8(-112, timeY - 28, 20, text, 7, -1, timeOt, 1);
        drawLimitedText8x8(-112, timeY - 36, 11, gTargetTimeLabel, 7, -1, timeOt, 1);
    }

    drawTransformedSpriteByIndex(112, (s16)(hudY - 80), 462, 5, -1, 0x7FFF, 0x7FFF,
                                 0xC00, 0xC00, 0, hudOt, depth);
    drawLimitedText8x8(124, hudY - 76, 1, "*", 6, -1, hudOt, depth);
    drawHudDecimalNumber8x16(132, hudY - 84, 2, 0,
                            gRacePlayers[player].collectedCourseCoinMarkerCount, 1, -1, hudOt, depth);
    drawTransformedSpriteByIndex(135, (s16)(hudY - 24), 423, 3, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth);
    drawTransformedSpriteByIndex(120, (s16)(hudY - 24), 420, 14, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth);
    if (player == 1) {
        hudY += 18;
    }
    drawSpriteByIndexWithBlend(96, hudY - 121, 417, 12, -1, hudOt, depth);
}
