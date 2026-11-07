#include "../race_presentation_internal.h"

void drawPlayerFlagTaskVisual(TaskVisualResource *resource) {
    PlayerCamera *cameras = D_801241E8;
    D_800EB22C.vx = rcos(-cameras[func_80059254()].yaw) * 40 / 4096;
    D_800EB22C.vz = rsin(-cameras[func_80059254()].yaw) * 40 / 4096;
    resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[0] + D_800EB22C.vx;
    resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[1] - 80;
    resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[2] + D_800EB22C.vz;
    setTaskVisualFrame(resource, gPlayerFlagVisualFrames, 0x1800);
    func_8005AC4C(resource, 1);
    func_80059290(resource);
}

void initializeTaskCoordinateResource(void) {
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    GsInitCoordinate2(0, (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource14]);
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 1;
    setCurrentTaskCallback(updatePlayerFlagTaskVisual);
}

void updatePlayerFlagTaskVisual(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_80120E90[D_8013AE88->data18].unk946) {
        if (!D_8013AE88->data1A) {
            resource->unkC6 = 0x600;
            resource->unkC8 = 0x100;
            resource->r0 = resource->g0 = resource->b0 = 0x80;
            resource->code = POLY_FT4_CODE;
            resource->framePosition = 0;
            D_8013AE88->data1A = 1;
            D_8013AE88->data1C = 7;
        }
        drawPlayerFlagTaskVisual(resource);
    } else if (D_8013AE88->data1A) {
        D_8013AE88->data1C--;
        if (D_8013AE88->data1C == 0) {
            D_8013AE88->data1A = 0;
        } else {
            drawPlayerFlagTaskVisual(resource);
        }
    }
}

void initializePlayerSideVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (OrbitingPlayerVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->frameLimit = 0x400;
    resource->frameStep = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->framePosition = 0;
    func_800183FC(D_8013AE88->data18, 1, 0x600, 0x20);
    func_8002E5A0(D_8013AE88->data18, 12);
    if (D_80120E90[D_8013AE88->data18].unk128 & 0x400) {
        resource->radius = -52;
    } else {
        resource->radius = 52;
    }
    setCurrentTaskCallback(updatePlayerSideVisualTask);
}

void updatePlayerSideVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (OrbitingPlayerVisualResource *)&D_800D9504[D_8013AE88->resource14];
    D_800EB22C.vx = rsin((s16)D_80120E90[D_8013AE88->data18].unk110) * resource->radius / 4096;
    D_800EB22C.vz = rcos((s16)D_80120E90[D_8013AE88->data18].unk110) * resource->radius / 4096;
    resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[0] + D_800EB22C.vx;
    resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[2] + D_800EB22C.vz;
    setTaskVisualFrame((TaskVisualResource *)resource, gPlayerSideVisualFrames, 0x2000);
    func_8005AC4C((TaskVisualResource *)resource, 1);
    if (!D_8013AFD3) {
        if (!func_80059290((TaskVisualResource *)resource)) {
            func_800149A8();
        }
    }
}

void updateOneShotTaskVisual(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    setTaskVisualFrame(resource, D_80124304, 0x2000);
    func_8005AC4C(resource, 1);
    if (!D_8013AFD3) {
        if (!func_80059290(resource)) {
            func_800149A8();
        }
    }
}

void initializeExpandingCoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    ExpandingCoordinateEffectResource *initial;
    s32 x, y, z;
    s16 index;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    initial = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    index = initial->rotation.vz;
    x = initial->coordinate.coord.t[0];
    y = initial->coordinate.coord.t[1];
    z = initial->coordinate.coord.t[2];
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->parent);
    GsInitCoordinate2(&resource->parent, &resource->coordinate);
    resource->position.x = x << 8;
    resource->position.y = y << 8;
    resource->position.z = z << 8;
    resource->selectedIndex = index;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->parent.coord.t[0] = x;
    resource->parent.coord.t[1] = y;
    resource->parent.coord.t[2] = z;
    playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0x101, 0x20);
    func_8004A764(index, x, z);
    computeSelectedTriangleAngles(&resource->pitch, &resource->yaw);
    resource->unkCA = 0;
    D_8013AE88->data1A = 45;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    setCurrentTaskCallback(updateExpandingCoordinateEffectTask);
}

void updateExpandingCoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (D_8013AE88->data1A >= 16) {
            resource->scale.vx += 0x40;
            resource->scale.vy += 0x40;
            resource->scale.vz += 0x40;
            for (i = 0; i < D_8013AFD2; i++) {
                addPlayerFlagsIfWithinRadius(&resource->position, resource->scale.vx * 24, 0x80, i);
                D_80120E90[i].unk0F4 = D_8013AE88->data18;
            }
        } else {
            resource->scale.vx -= 0x80;
            resource->scale.vy -= 0x80;
            resource->scale.vz -= 0x80;
        }
        resource->rotation.vy += 0x80;
        D_8013AE88->data1A--;
    }
    if (isPositionInCameraRange(resource->parent.coord.t[0], resource->parent.coord.t[2], (s16)func_80059254())) {
        D_80129A80[15].coord2 = &resource->coordinate;
        func_8001E3E4((SVECTOR *)&resource->pitch, &resource->parent);
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(15);
    }
    if (!D_8013AE88->data1A) { func_800149A8(); }
}

void resetTaskVisualState(TaskVisualResource *resource) {
    resource->unkC6 = 0x400;
    resource->unkC8 = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->framePosition = 0;
    playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z,
                  1, 0x700, 0x20);
    setCurrentTaskCallback(updateOneShotTaskVisual);
}

void createNineIndexedChildTasks(u16 value, u16 mode) {
    u16 i;
    for (i = 0; i < 9; i++) {
        func_800148B4(func_8004531C);
        D_8013AE8C->data18 = D_8013AE88->data18;
        D_8013AE8C->data1A = i;
        D_8013AE8C->data1C = value;
        D_8013AE8C->data1E = mode;
    }
}

void initializePlayerState23CoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ExpandingCoordinateEffectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->parent);
    GsInitCoordinate2(&resource->parent, &resource->coordinate);
    players = D_80120E90;
    resource->selectedIndex = players[D_8013AE88->data18].unk92E;
    resource->rotation.vx = 0;
    resource->rotation.vy = players[D_8013AE88->data18].unk110;
    resource->rotation.vz = 0;
    resource->position.x = players[D_8013AE88->data18].x;
    resource->position.y = players[D_8013AE88->data18].y;
    resource->position.z = players[D_8013AE88->data18].z;
    func_8004A764(resource->selectedIndex, resource->position.x >> 8, resource->position.z >> 8);
    computeSelectedTriangleAngles(&resource->pitch, &resource->yaw);
    resource->unkCA = 0;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    players[D_8013AE88->data18].isActive = 0;
    setCurrentTaskCallback(func_800450BC);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_800450BC);

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_8004531C);

void updateFallingRotatingObjectTask(void) {
    FallingObjectResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (FallingObjectResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (D_8013AE88->data1A & 1) {
            resource->rotation.vx += 0x100;
            resource->rotation.vy += 0x100;
            resource->rotation.vz += 0x100;
        } else {
            resource->rotation.vx -= 0x100;
            resource->rotation.vy -= 0x100;
            resource->rotation.vz -= 0x100;
        }
        resource->coordinate.coord.t[0] += resource->velocity.x;
        resource->coordinate.coord.t[1] += resource->velocity.y;
        resource->coordinate.coord.t[2] += resource->velocity.z;
        resource->velocity.y += 6;
        if (resource->coordinate.coord.t[1] > resource->heightLimit) {
            func_800149A8();
            return;
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->parentRotation, &resource->parent);
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
}

void initializeScaledTaskCoordinateResource(void) {
    ScaledCoordinateResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ScaledCoordinateResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0xC00;
    setCurrentTaskCallback(updatePlayerScaledObjectTask);
}

void updatePlayerScaledObjectTask(void) {
    PlayerEffectResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    players = D_80120E90;
    resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1];
    resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2];
    resource->rotation.vx = players[D_8013AE88->data18].unk0B4;
    resource->rotation.vy = players[D_8013AE88->data18].unk0B6;
    resource->rotation.vz = players[D_8013AE88->data18].unk0B8;
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[10].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(10);
    }
    if (players[D_8013AE88->data18].unk12C != 25) {
        func_800183FC(D_8013AE88->data18, 1, 0xF00, 0x20);
        createNineIndexedChildTasks(0x15, 0);
        D_8013AE88->callback = deleteCurrentTask;
        D_8013AE88->unk10 |= 1;
    }
}

void initializePlayerParticleHistoryTask(void) {
    TaskVisualResource *resource;
    PlayerParticleResource *trail;
    u16 i;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    trail = (PlayerParticleResource *)&D_800D9504[D_8013AE88->resource16];
    GsInitCoordinate2(0, &resource->coordinate);
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 0;
    D_8013AE88->data1E = 0;
    for (i = 0; i < 16; i++) {
        trail->entries[i].framePosition = 0xFFFF;
    }
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->code = POLY_FT4_CODE;
    setCurrentTaskCallback(func_80045C7C);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_80045C7C);

void initializePlayerCoordinateEffectTask(void) {
    PlayerEffectResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    D_8013AE88->data1A = 0;
    resource->rotation.vx = resource->rotation.vy = resource->rotation.vz = 0;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    func_800183FC(D_8013AE88->data18, 1, 0x800, 0x20);
    setCurrentTaskCallback(updatePlayerCoordinateEffectTask);
}

void updatePlayerCoordinateEffectTask(void) {
    PlayerEffectResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        players = D_80120E90;
        if (players[D_8013AE88->data18].unk12C == 0x1B) {
            resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0];
            resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1];
            resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2];
            if (resource->scale.vx < 0x1000) {
                resource->scale.vx += 0x200;
                resource->scale.vy += 0x200;
                resource->scale.vz += 0x200;
            }
        } else {
            if (isFirstPlayerUsingCallback26D10()) {
                func_800149A8();
                return;
            }
            if (resource->scale.vy == 0) {
                resource->scale.vx -= 0x400;
                resource->scale.vz -= 0x400;
                if (resource->scale.vx <= 0) {
                    func_800149A8();
                    return;
                }
            } else {
                resource->scale.vy -= 0x100;
            }
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[11].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(11);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_80046228);

void initializePlayerVisualHistory(TaskVisualResource *resource, VECTOR *vectors, s32 value, s32 scale) {
    RacePlayer *players;
    RacePlayer *player;
    VECTOR *entry;
    u16 i;
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    if (D_80120E90[D_8013AE88->data18].unk128 & 0x400) {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110 + 0x800;
    } else {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110;
    }
    players = D_80120E90;
    resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2];
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(players[D_8013AE88->data18].unk92E, players[D_8013AE88->data18].x >> 8, players[D_8013AE88->data18].z >> 8);
    resource->unkDC = value;
    resource->groundY = scale;
    player = &players[D_8013AE88->data18];
    if (player->unk0BC != 0) {
        resource->velocityY = (player->unk062 * ((scale << 8) / player->unk0BC)) / 256;
    } else {
        resource->velocityY = player->unk062;
    }
    for (i = 0; i < 4; i++) {
        entry = vectors;
        entry += i;
        entry->vx = entry->vy = entry->vz = 0;
        entry->pad = 0;
    }
    D_8013AE88->data1A = 0;
    func_8001496C(&gGameTasks[4], initializePlayerSideVisualTask);
    D_8013AE8C->data18 = D_8013AE88->data18;
}

void drawTaskVisualPositionHistory(TaskVisualResource *resource, VECTOR *history, TaskVisualFrame *frames) {
    u16 i;
    s32 phase;
    VECTOR *entry;
    resource->code = POLY_FT4_CODE;
    resource->scaleX = 0x1A00;
    resource->scaleY = 0x1A00;
    resource->scaleZ = 0x1A00;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    prepareTaskVisualQuad(resource, frames, 1);
    func_8005A788(resource, frames, 0);
    resource->code = POLY_FT4_CODE | 2;
    for (i = 0; i < 4; i++) {
        entry = history;
        entry += i;
        if (entry->pad != 0) {
            resource->coordinate.coord.t[0] = entry->vx >> 8;
            resource->coordinate.coord.t[1] = entry->vy >> 8;
            resource->coordinate.coord.t[2] = entry->vz >> 8;
            phase = (D_8013AE88->data1A - i) & 3;
            resource->r0 = resource->g0 = resource->b0 = 0x80 - (phase << 3);
            resource->scaleX = 0x1A00 + (phase << 9);
            resource->scaleY = 0x1A00 + (phase << 9);
            resource->scaleZ = 0x1A00 + (phase << 9);
            prepareTaskVisualQuad(resource, frames, 1);
            func_8005A788(resource, frames, 0);
        }
    }
    if (D_8013AFD3 == 0) {
        D_8013AE88->data1A = (D_8013AE88->data1A + 1) & 3;
    }
}

void updateHomingTaskVisualMovement(TaskVisualResource *resource, VECTOR *history, s32 distance, s32 maxTurn) {
    VECTOR *entry;
    s16 angle;
    s32 player;
    s32 oldY;
    s32 ground;
    entry = history;
    entry += D_8013AE88->data1A;
    entry->vx = resource->position.x;
    entry = history;
    entry += D_8013AE88->data1A;
    entry->vy = resource->position.y;
    entry = history;
    entry += D_8013AE88->data1A;
    entry->vz = resource->position.z;
    history += D_8013AE88->data1A;
    history->pad = 1;
    player = func_80046228(&resource->position, distance, resource->unkD8, D_8013AE88->data18, &angle);
    resource->unkE0 = player;
    if (player != -1) {
        D_80120E90[player].unk946 = 1;
        angle = (angle - resource->unkD8) & 0xFFF;
        if (angle > 0x800) angle -= 0x1000;
        if (angle > (s16)maxTurn) angle = maxTurn;
        if (angle < -(s16)maxTurn) angle = -maxTurn;
        resource->unkD8 += angle;
    }
    D_800EB22C.vx = rsin(resource->unkD8) * resource->groundY / 4096;
    D_800EB22C.vz = rcos(resource->unkD8) * resource->groundY / 4096;
    oldY = resource->position.y;
    resource->position.x += D_800EB22C.vx;
    resource->position.y += resource->velocityY;
    resource->position.z += D_800EB22C.vz;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
    ground = func_8004A764(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    if (resource->position.y > ground) resource->position.y = ground;
    resource->velocityY = resource->position.y - oldY + 0x1900;
}

void checkTaskVisualOtherPlayerCollision(TaskVisualResource *resource, s16 flags) {
    u16 i;
    RacePlayer *player;
    for (i = 0; i < D_8013AFD2; i++) {
        if (i != D_8013AE88->data18) {
            player = &D_80120E90[i];
            if (player->unk946 != 0) {
                if (addPlayerFlagsIfWithinRadius(&resource->position, 0xF00, flags, i) != 0) {
                    player->unk0F4 = D_8013AE88->data18;
                    resource->unkDC = 0;
                    break;
                }
            }
        }
    }
}

void initializePlayerDualResourceEffectTask(void) {
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5580);
    setCurrentTaskCallback(updatePlayerDualResourceEffectTask);
}

void updatePlayerDualResourceEffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x63000, 0x1C);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerDualResourceEffectFrame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 8);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializePlayerVisualPairTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory(resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5580);
    setCurrentTaskCallback(updatePlayerVisualPairTask);
}

void updatePlayerVisualPairTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x51000, 0x12);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPairFrame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x100);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializePlayerVisualPairTask5E80(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory(resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5E80);
    setCurrentTaskCallback(updatePlayerVisualPairTask5E80);
}

void updatePlayerVisualPairTask5E80(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x3F000, 0x18);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPair5E80Frame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x10);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializePlayerVisualPairTask6780(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory(resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 180, 0x6780);
    setCurrentTaskCallback(updatePlayerVisualPairTask6780);
}

void updatePlayerVisualPairTask6780(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x1B000, 0x1C);
        func_8004BC3C(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz, &D_800EB22C.vx, &D_800EB22C.vz);
        if (dx != 0 || dz != 0) {
            resource->position.x += dx;
            resource->position.z += dz;
            resource->unkD8 = func_8004A124(0, 0, D_800EB22C.vx, D_800EB22C.vz);
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPair6780Frame);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x40);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializeDualResource4C80EffectTask(void) {
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x4C80);
    setCurrentTaskCallback(updateDualResource4C80EffectTask);
}

void updateDualResource4C80EffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AFD3 == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x2D000, 0x10);
        func_8004C968(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, D_80124354);
    if (D_8013AFD3 == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x80);
        if (resource->unkDC == 0) {
            initializeExpandingCoordinateEffectTask();
        } else {
            resource->unkDC--;
        }
    }
}

void initializeBouncingTaskVisual(void) {
    TaskVisualResource *resource;
    RacePlayer *players;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x200;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    if (D_80120E90[D_8013AE88->data18].unk128 & 0x400) {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110;
    } else {
        resource->unkD8 = (s16)D_80120E90[D_8013AE88->data18].unk110 + 0x800;
    }
    D_800EB22C.vx = rsin(resource->unkD8) / 64;
    D_800EB22C.vz = rcos(resource->unkD8) / 64;
    players = D_80120E90;
    resource->coordinate.coord.t[0] = players[D_8013AE88->data18].worldCoordinate.coord.t[0] + D_800EB22C.vx;
    resource->coordinate.coord.t[1] = players[D_8013AE88->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = players[D_8013AE88->data18].worldCoordinate.coord.t[2] + D_800EB22C.vz;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(players[D_8013AE88->data18].unk92E, players[D_8013AE88->data18].x >> 8, players[D_8013AE88->data18].z >> 8);
    resource->velocityY = -0xA00;
    resource->groundY = func_8004A764(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    resource->bounceCount = 0;
    func_800183FC(D_8013AE88->data18, 1, 2, 0x20);
    setCurrentTaskCallback(updateBouncingTaskVisual);
}

void updateBouncingTaskVisual(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->velocityY += 0x180;
        resource->position.y += resource->velocityY;
        if (resource->velocityY > 0 && resource->position.y > resource->groundY) {
            if (resource->bounceCount == 0) {
                resource->velocityY = -0xA00;
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xB01, 12);
            } else {
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xB01, 8);
            }
            resource->position.y = resource->groundY;
            resource->bounceCount++;
        }
        resource->coordinate.coord.t[0] = resource->position.x >> 8;
        resource->coordinate.coord.t[1] = resource->position.y >> 8;
        resource->coordinate.coord.t[2] = resource->position.z >> 8;
    }
    setTaskVisualFrame(resource, D_80124354, 0x1A00);
    func_8005AC4C(resource, 1);
    if (!D_8013AFD3 && resource->bounceCount == 2) {
        resource->unkDC = 30;
        setCurrentTaskCallback(updateAcceleratingTaskVisualCountdown);
    }
}

void updateAcceleratingTaskVisualCountdown(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    setTaskVisualFrame(resource, D_80124354, 0x1A00);
    func_8005AC4C(resource, 1);
    if (!D_8013AFD3) {
        if (resource->unkDC >= 5) {
            resource->unkC8 += 8;
            if (resource->unkC8 > 0x80) {
                resource->unkC8 = 0x80;
            }
            resource->framePosition += resource->unkC8;
            if (resource->framePosition >= resource->unkC6) {
                resource->framePosition &= 0xFF;
            }
        } else {
            resource->framePosition = 0x100;
        }
        if (--resource->unkDC == 0) {
            initializeExpandingCoordinateEffectTask();
        }
    }
}

void initializeDualResource5100EffectTask(void) {
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = &D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&D_800D9504[D_8013AE88->resource16], 300, 0x5100);
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(func_80047D94);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_80047D94);

void initializeVisualTrailTask(void) {
    TrailVisualResource *resource;
    VECTOR *vectors;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (TrailVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    vectors = (VECTOR *)&D_800D9504[D_8013AE88->resource16];
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    D_800EB22C.vx = rsin(resource->angle) / 64;
    D_800EB22C.vz = rcos(resource->angle) / 64;
    resource->lifetime = 60;
    resource->scale = 0x6780;
    resource->unkD4 = 0;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    for (i = 0; i < 4; i++) {
        VECTOR *vector = &vectors[i];
        vector->vx = vector->vy = vector->vz = 0;
        vector->pad = 0;
    }
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(func_80048340);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_80048340);

void initPlayerTimerTask(void) {
    PlayerTimerState *timer = &D_80120E90[D_8013AE88->data18].timer;

    GsInitCoordinate2(0, &timer->coordinate);
    timer->unk60 = 0;
    D_80120E90[D_8013AE88->data18].unk0FE = 1;
    setCurrentTaskCallback(updatePlayerTimerTask);
}

void updatePlayerTimerTask(void) {
    if (!D_8013AFD3) {
        if (D_80120E90[D_8013AE88->data18].unk128 & 0x3040) {
            D_80120E90[D_8013AE88->data18].unk0FC = 0;
        }
        if (D_80120E90[D_8013AE88->data18].unk0FC != 0) {
            D_80120E90[D_8013AE88->data18].unk0FC--;
        }
        if (D_80120E90[D_8013AE88->data18].unk0FC == 0) {
            if (!D_80120E90[D_8013AE88->data18].unk008 &&
                gControllerEffectStates[D_8013AE88->data18].effect == 13 &&
                !(D_80120E90[D_8013AE88->data18].unk128 & 0x2000)) {
                D_8013AF08[D_8013AE88->data18] = 0;
            }
            setCurrentTaskCallback(fadeOutPlayerTimerTask);
        } else if (D_80120E90[D_8013AE88->data18].timer.unk60 < 0x1000) {
            D_80120E90[D_8013AE88->data18].timer.unk60 += 0x100;
        }
    }
}

void fadeOutPlayerTimerTask(void) {
    if (!D_8013AFD3) {
        if (D_80120E90[D_8013AE88->data18].unk0FC == 0) {
            D_80120E90[D_8013AE88->data18].timer.unk60 -= 0x200;
            if (D_80120E90[D_8013AE88->data18].timer.unk60 > 0) {
                return;
            }
            D_80120E90[D_8013AE88->data18].timer.unk60 = 0;
            D_80120E90[D_8013AE88->data18].unk0FE = 0;
        }
        func_800149A8();
    }
}

void initializeOrbitingPlayerVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (OrbitingPlayerVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->frameLimit = 0x400;
    resource->frameStep = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE | 2;
    resource->offset.y = -32;
    resource->radius = 8;
    resource->r0 = resource->g0 = resource->b0 = 0x40;
    resource->framePosition = 0;
    resource->scale = 0x1800;
    resource->angle = 0;
    func_800183FC(D_8013AE88->data18, 1, 0xE00, 0x20);
    resource->countdown = 0x10;
    D_8013AE88->data1C = 0;
    setCurrentTaskCallback(func_80048AA4);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_80048AA4);

void initializeDelayedPlayerRiseEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (D_8013AE88->data1C == 0) {
        if (D_8013AE88->resource14 != 0xFFFF) {
            while (1) {}
        }
        D_8013AE88->resource14 = allocateTaskResource();
        resource = (RisingPlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->offset.x = 0;
        resource->offset.y = -64;
        resource->offset.z = 0;
        resource->velocity = 0;
        resource->rotation.vx = resource->rotation.vy = resource->rotation.vz = 0;
        resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
        playSoundEffect(1, 0xA00, 0x30);
        setCurrentTaskCallback(updateRisingPlayerEffectTask);
    } else {
        D_8013AE88->data1C--;
    }
}

void updateRisingPlayerEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RisingPlayerEffectResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->velocity -= 2;
        resource->offset.y += resource->velocity;
        if (resource->offset.y < -320) {
            resource->velocity = 2;
            resource->unkD8 = 1;
            D_8013AE88->data1C = 0;
            setCurrentTaskCallback(func_80049404);
            return;
        }
        resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data1A].worldCoordinate.coord.t[0];
        resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data1A].worldCoordinate.coord.t[1] + resource->offset.y;
        resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data1A].worldCoordinate.coord.t[2];
        if (resource->scale.vx < 0x1000) {
            resource->scale.vx += 0x100;
            resource->scale.vy += 0x100;
            resource->scale.vz += 0x100;
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[12].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(12);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/coordinate_effects", func_80049404);

void initializeSlidingVisualSequenceTask(void) {
    SlidingVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (SlidingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 128;
    resource->code = POLY_FT4_CODE;
    resource->step = 0;
    resource->timer = 0;
    resource->framePosition = 0;
    resource->unk74 = -352;
    resource->velocity = 0;
    playSoundEffect(1, 0xD01, 0x20);
    D_8013AE88->data1C = 0;
    setCurrentTaskCallback(func_800497B8);
}
