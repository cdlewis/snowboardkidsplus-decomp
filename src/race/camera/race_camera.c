#include "../race_presentation_internal.h"

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
    cameras = D_801241E8;
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
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[index];
    RacePlayer *players;
    RacePlayer *player;
    GsRVIEW2 *views;
    GsRVIEW2 *view;
    RotMatrixYXZ((D_8013AF90->vx = camera->pitch & 0xFFF,
                  D_8013AF90->vy = camera->yaw & 0xFFF,
                  D_8013AF90->vz = 0, D_8013AF90), D_8013AF98);
    D_8013AF94->vx = 0;
    D_8013AF94->vy = 0;
    D_8013AF94->vz = camera->unk10 - camera->unk12 + camera->unk14;
    ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
    players = gRacePlayers;
    player = &players[index];
    camera->x = player->x + (D_8013AF88->vx << 8);
    camera->y = player->y + (D_8013AF88->vy << 8);
    camera->z = player->z + (D_8013AF88->vz << 8);
    views = gPlayerCameraViews;
    view = &views[index];
    view->vpx = camera->x / 256;
    view->vpy = camera->y / 256 - 120;
    view->vpz = camera->z / 256;
}

void updatePlayerCameraPitch(u16 playerIndex) {
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];
    PlayerCamera *cameras = D_801241E8;
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
    D_801241E8[playerIndex].pitch = func_8004A124(0, 0, SquareRoot0(x * x + z * z), y / scale) - 0x400;
}

void updatePlayerCameraAngles(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    RacePlayer *players = gRacePlayers;
    RacePlayer *player = &players[playerIndex];
    camera->yaw = func_8004A124(camera->x, camera->z, player->x, player->z);
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
        cameras = D_801241E8;
        camera = &cameras[playerIndex];
        D_8013AF90->vx = camera->pitch & 0xFFF;
        D_8013AF90->vy = camera->yaw & 0xFFF;
        D_8013AF90->vz = 0;
        RotMatrixYXZ(D_8013AF90, D_8013AF98);
        D_8013AF94->vx = 0;
        D_8013AF94->vy = 0;
        D_8013AF94->vz = -320;
        ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
        view->vpx = view->vrx + D_8013AF88->vx;
        view->vpy = view->vry + D_8013AF88->vy;
        view->vpz = view->vrz + D_8013AF88->vz;
    }
}

void clampPlayerCameraHeight(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    PlayerCamera *camera = &cameras[playerIndex];
    GsRVIEW2 *views = gPlayerCameraViews;
    GsRVIEW2 *view = &views[playerIndex];
    s32 height;
    camera->unk18 = getAdjacentGeometryRegionForPoint(camera->unk18, view->vpx, view->vpz);
    height = func_8004A764(camera->unk18, view->vpx, view->vpz);
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
    cameras = D_801241E8;
    cameras[index].unk18 = 0;
    updatePlayerCameraPosition(index);
    clampPlayerCameraHeight(index);
    targets = gPlayerCameraViews;
    players = gRacePlayers;
    targets[D_8013AE88->data18].vrx = players[D_8013AE88->data18].worldCoordinate.coord.t[0] + players[D_8013AE88->data18].unk084;
    targets[D_8013AE88->data18].vry = players[D_8013AE88->data18].worldCoordinate.coord.t[1] + (s16)(cameras[D_8013AE88->data18].unk14 / 8 - 120) + players[D_8013AE88->data18].unk086;
    targets[D_8013AE88->data18].vrz = players[D_8013AE88->data18].worldCoordinate.coord.t[2] + players[D_8013AE88->data18].unk088;
}

void initPlayerZeroTask(void) {
    initPlayerCameraAndTarget(0);
    setCurrentTaskCallback(func_8003EE70);
}

void initCurrentPlayerCameraTask(void) {
    GsRVIEW2 *views;
    PlayerCamera *cameras;
    if (D_8013AFD7 != 0) {
        D_801241E8[D_8013AE88->data18].unk16 = 1;
        initPlayerCameraAndTarget(D_8013AE88->data18);
        if (D_8013AE88->data18 == 0) {
            queueCdCommand(D_80124164[D_8013AFCE], 0);
        } else {
            D_8013AFAA |= 0x800;
        }
        setCurrentTaskCallback(func_8003EE70);
    } else {
        D_8013AE88->data1A = 64;
        initPlayerCameraAndTarget(D_8013AE88->data18);
        views = gPlayerCameraViews;
        views[D_8013AE88->data18].vpx = 0;
        views[D_8013AE88->data18].vpz += 2560;
        cameras = D_801241E8;
        cameras[D_8013AE88->data18].unk18 = getAdjacentGeometryRegionForPoint(cameras[D_8013AE88->data18].unk18, views[D_8013AE88->data18].vpx, views[D_8013AE88->data18].vpz);
        views[D_8013AE88->data18].vpy = func_8004A764(cameras[D_8013AE88->data18].unk18, views[D_8013AE88->data18].vpx, views[D_8013AE88->data18].vpz) - 120;
        cameras[D_8013AE88->data18].x = views[D_8013AE88->data18].vpx << 8;
        cameras[D_8013AE88->data18].y = views[D_8013AE88->data18].vpy << 8;
        cameras[D_8013AE88->data18].z = views[D_8013AE88->data18].vpz << 8;
        updatePlayerCameraAngles(D_8013AE88->data18);
        setCurrentTaskCallback(func_8003EA20);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/camera/race_camera", func_8003EA20);

void updateCurrentPlayerCameraTarget(void) {
    GsRVIEW2 *targets = gPlayerCameraViews;
    RacePlayer *players = gRacePlayers;
    targets[D_8013AE88->data18].vrx = players[D_8013AE88->data18].worldCoordinate.coord.t[0] + players[D_8013AE88->data18].unk084;
    targets[D_8013AE88->data18].vry = players[D_8013AE88->data18].worldCoordinate.coord.t[1] + (s16)(D_801241E8[D_8013AE88->data18].unk14 / 8 - 120) + players[D_8013AE88->data18].unk086;
    targets[D_8013AE88->data18].vrz = players[D_8013AE88->data18].worldCoordinate.coord.t[2] + players[D_8013AE88->data18].unk088;
}
