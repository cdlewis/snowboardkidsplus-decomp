#include "../race_presentation_internal.h"
#include "../race_internal.h"
#include "game/audio/music.h"
#include "game/engine/angles.h"
#include "game/engine/game_options.h"
#include "game/engine/random.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/io/cd_queue.h"
#include "game/menu/selected_records.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_geometry.h"
#include "game/race/player/race_player.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern VECTOR *gScratchVector0 __attribute__((section(".data")));
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern SVECTOR *D_8013AF94 __attribute__((section(".data")));
extern MATRIX *gScratchMatrix0 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013B1AB __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));
extern u8 D_8013AFD7 __attribute__((section(".data")));

void initPlayerCamera(u16 index) {
    GsRVIEW2 *view;
    GsRVIEW2 *initialView;
    PlayerCamera *camera;
    GsRVIEW2 *views;
    GsRVIEW2 *initialViews;
    PlayerCamera *cameras;
    RacePlayer *players;
    GsSetProjection(D_8013AF84);
    initialViews = gPlayerCameraViews;
    initialView = &initialViews[index];
    initialView->vpx = 0;
    initialView->vpy = 0;
    initialView->vpz = 0;
    initialView->vrx = 0;
    initialView->vry = 0;
    if (!(D_8013AFAA & 0xC00)) initialView->vrz = 320;
    else initialView->vrz = 576;
    views = gPlayerCameraViews;
    view = &views[index];
    cameras = gPlayerCameras;
    camera = &cameras[index];
    players = gRacePlayers;
    view->rz = 0;
    view->super = 0;
    camera->yaw = players[index].unk0B6 + 0x400;
    camera->pitch = -16;
    camera->unk10 = view->vpz - view->vrz;
    camera->unk12 = camera->unk14 = 0;
    camera->unk16 = 1;
    camera->unk1A = 0;
    GsSetRefView2L(view);
}

void updatePlayerCameraPosition(u16 index) {
    PlayerCamera *cameras = gPlayerCameras;
    PlayerCamera *camera = &cameras[index];
    RacePlayer *players;
    RacePlayer *player;
    GsRVIEW2 *views;
    GsRVIEW2 *view;
    RotMatrixYXZ((D_8013AF90->vx = camera->pitch & 0xFFF,
                  D_8013AF90->vy = camera->yaw & 0xFFF,
                  D_8013AF90->vz = 0, D_8013AF90), gScratchMatrix0);
    D_8013AF94->vx = 0;
    D_8013AF94->vy = 0;
    D_8013AF94->vz = camera->unk10 - camera->unk12 + camera->unk14;
    ApplyMatrix(gScratchMatrix0, D_8013AF94, gScratchVector0);
    players = gRacePlayers;
    player = &players[index];
    camera->x = player->x + (gScratchVector0->vx << 8);
    camera->y = player->y + (gScratchVector0->vy << 8);
    camera->z = player->z + (gScratchVector0->vz << 8);
    views = gPlayerCameraViews;
    view = &views[index];
    view->vpx = camera->x / 256;
    view->vpy = camera->y / 256 - 120;
    view->vpz = camera->z / 256;
}

void updatePlayerCameraPitch(u16 playerIndex) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];
    PlayerCamera *cameras = gPlayerCameras;
    PlayerCamera *camera = &cameras[playerIndex];
    s32 x = (player->x - camera->x) / 8;
    s32 y = (player->y - camera->y) / 8;
    s32 z = (player->z - camera->z) / 8;
    s32 scale = 1;
    while (1) {
        if ((u32)(x + 32767) <= 65534 && (u32)(z + 32767) <= 65534) {
            break;
        }
        scale *= 2;
        x /= 2;
        z /= 2;
    }
    gPlayerCameras[playerIndex].pitch = getXZHeading(0, 0, SquareRoot0(x * x + z * z), y / scale) - 0x400;
}

void updatePlayerCameraAngles(u16 playerIndex) {
    PlayerCamera *cameras = gPlayerCameras;
    PlayerCamera *camera = &cameras[playerIndex];
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];
    camera->yaw = getXZHeading(camera->x, camera->z, player->x, player->z);
    updatePlayerCameraPitch(playerIndex);
}

void clampPlayerCameraDistance(u16 playerIndex) {
    GsRVIEW2 *views = gPlayerCameraViews;
    GsRVIEW2 *view = &views[playerIndex];
    s32 dx = view->vpx - view->vrx;
    s32 dz = view->vpz - view->vrz;
    s32 dy = view->vpy - view->vry;
    s32 distance = SquareRoot0(dx * dx + dz * dz);
    PlayerCamera *cameras;
    PlayerCamera *camera;
    distance = SquareRoot0(distance * distance + dy * dy);
    if (distance < 320) {
        cameras = gPlayerCameras;
        camera = &cameras[playerIndex];
        D_8013AF90->vx = camera->pitch & 0xFFF;
        D_8013AF90->vy = camera->yaw & 0xFFF;
        D_8013AF90->vz = 0;
        RotMatrixYXZ(D_8013AF90, gScratchMatrix0);
        D_8013AF94->vx = 0;
        D_8013AF94->vy = 0;
        D_8013AF94->vz = -320;
        ApplyMatrix(gScratchMatrix0, D_8013AF94, gScratchVector0);
        view->vpx = view->vrx + gScratchVector0->vx;
        view->vpy = view->vry + gScratchVector0->vy;
        view->vpz = view->vrz + gScratchVector0->vz;
    }
}

void clampPlayerCameraHeight(u16 playerIndex) {
    PlayerCamera *cameras = gPlayerCameras;
    PlayerCamera *camera = &cameras[playerIndex];
    GsRVIEW2 *views = gPlayerCameraViews;
    GsRVIEW2 *view = &views[playerIndex];
    s32 height;
    camera->unk18 = getAdjacentGeometryRegionForPoint(camera->unk18, view->vpx, view->vpz);
    height = getCourseHeightAtPoint(camera->unk18, view->vpx, view->vpz);
    if (height - 120 < view->vpy) {
        view->vpy = height - 120;
        camera->y = height << 8;
        updatePlayerCameraPitch(playerIndex);
    }
}

void initPlayerCameraAndTarget(u16 index) {
    GsRVIEW2 *targets;
    RacePlayer *players;
    PlayerCamera *cameras;
    initPlayerCamera(index);
    cameras = gPlayerCameras;
    cameras[index].unk18 = 0;
    updatePlayerCameraPosition(index);
    clampPlayerCameraHeight(index);
    targets = gPlayerCameraViews;
    players = gRacePlayers;
    targets[gCurrentTask->data18].vrx = players[gCurrentTask->data18].worldCoordinate.coord.t[0] + players[gCurrentTask->data18].unk084;
    targets[gCurrentTask->data18].vry = players[gCurrentTask->data18].worldCoordinate.coord.t[1] + (s16)(cameras[gCurrentTask->data18].unk14 / 8 - 120) + players[gCurrentTask->data18].unk086;
    targets[gCurrentTask->data18].vrz = players[gCurrentTask->data18].worldCoordinate.coord.t[2] + players[gCurrentTask->data18].unk088;
}

void initPlayerZeroTask(void) {
    initPlayerCameraAndTarget(0);
    setCurrentTaskCallback(updateCurrentPlayerCameraTask);
}

void initCurrentPlayerCameraTask(void) {
    GsRVIEW2 *views;
    PlayerCamera *cameras;
    if (D_8013AFD7 != 0) {
        gPlayerCameras[gCurrentTask->data18].unk16 = 1;
        initPlayerCameraAndTarget(gCurrentTask->data18);
        if (gCurrentTask->data18 == 0) {
            queueCdCommand(D_80124164[gSelectedCourseIndex], 0);
        } else {
            D_8013AFAA |= 0x800;
        }
        setCurrentTaskCallback(updateCurrentPlayerCameraTask);
    } else {
        gCurrentTask->data1A = 64;
        initPlayerCameraAndTarget(gCurrentTask->data18);
        views = gPlayerCameraViews;
        views[gCurrentTask->data18].vpx = 0;
        views[gCurrentTask->data18].vpz += 2560;
        cameras = gPlayerCameras;
        cameras[gCurrentTask->data18].unk18 = getAdjacentGeometryRegionForPoint(cameras[gCurrentTask->data18].unk18, views[gCurrentTask->data18].vpx, views[gCurrentTask->data18].vpz);
        views[gCurrentTask->data18].vpy = getCourseHeightAtPoint(cameras[gCurrentTask->data18].unk18, views[gCurrentTask->data18].vpx, views[gCurrentTask->data18].vpz) - 120;
        cameras[gCurrentTask->data18].x = views[gCurrentTask->data18].vpx << 8;
        cameras[gCurrentTask->data18].y = views[gCurrentTask->data18].vpy << 8;
        cameras[gCurrentTask->data18].z = views[gCurrentTask->data18].vpz << 8;
        updatePlayerCameraAngles(gCurrentTask->data18);
        setCurrentTaskCallback(updateRaceIntroCameraTask);
    }
}

void updateRaceIntroCameraTask(void) {
    GsRVIEW2 *views;
    PlayerCamera *cameras;
    GameTask *tasks;

    if ((--gCurrentTask->data1A & 0x8000) == 0) {
        views = gPlayerCameraViews;
        views[gCurrentTask->data18].vpz -= 40;
        cameras = gPlayerCameras;
        cameras[gCurrentTask->data18].unk18 = getAdjacentGeometryRegionForPoint(
            cameras[gCurrentTask->data18].unk18,
            views[gCurrentTask->data18].vpx, views[gCurrentTask->data18].vpz);
        views[gCurrentTask->data18].vpy = getCourseHeightAtPoint(
            cameras[gCurrentTask->data18].unk18,
            views[gCurrentTask->data18].vpx, views[gCurrentTask->data18].vpz) - 120;
        cameras[gCurrentTask->data18].y = (views[gCurrentTask->data18].vpy + 120) << 8;
        cameras[gCurrentTask->data18].z = views[gCurrentTask->data18].vpz << 8;
        updatePlayerCameraAngles(gCurrentTask->data18);

        if (gCurrentTask->data1A == 15) {
            configureTaskStateTransition(5, 0, 15);
        }
        if (gCurrentTask->data1A == 0) {
            if (D_8013AFCD == 2) {
                D_8013AFAA |= 0x800;
            }
            initPlayerCameraAndTarget(gCurrentTask->data18);
            configureTaskStateTransition(4, 0, 40);
            D_8013B1A8 = 1;
            if (gCurrentTask->data18 == 0) {
                tasks = gGameTasks;
                createChildTaskForParentWithReserve(&tasks[0], beginSpriteScaleTransitionDelay);
                if (gCurrentTask->data18 == 0) {
                    createChildTaskForParentWithReserve(&tasks[4], initializeBouncingVisualTask);
                }
            }
        }
    }

    if (gCurrentTask->data18 == 0 || (D_8013AFAA & 0x400)) {
        renderCurrentCameraPlayersAndUpdateSound();
    }

    if (gCurrentTask->data18 == 0 && gCurrentTask->data1A == (u16)-50) {
        playSoundEffect(0, 1, 32);
    }
    if (gCurrentTask->data1A == (u16)-104) {
        if (gCurrentTask->data18 == 0) {
            queueCdCommand(D_80124164[gSelectedCourseIndex], 0);
            playSoundEffect(0, 0x101, 32);
            gTaskTransitionFlags = 0;
            D_8013B1AB = 0;
        }
        setCurrentTaskCallback(updateCurrentPlayerCameraTask);
    }
}

void updateCurrentPlayerCameraTarget(void) {
    GsRVIEW2 *targets = gPlayerCameraViews;
    RacePlayer *players = gRacePlayers;
    targets[gCurrentTask->data18].vrx = players[gCurrentTask->data18].worldCoordinate.coord.t[0] + players[gCurrentTask->data18].unk084;
    targets[gCurrentTask->data18].vry = players[gCurrentTask->data18].worldCoordinate.coord.t[1] + (s16)(gPlayerCameras[gCurrentTask->data18].unk14 / 8 - 120) + players[gCurrentTask->data18].unk086;
    targets[gCurrentTask->data18].vrz = players[gCurrentTask->data18].worldCoordinate.coord.t[2] + players[gCurrentTask->data18].unk088;
}
