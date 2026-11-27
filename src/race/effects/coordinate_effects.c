#include "common.h"

extern u8 gRaceOrderPlayerIds[] __attribute__((section(".data")));

#include "../race_presentation_internal.h"
#include "game/engine/random.h"
#include "../race_internal.h"
#include "game/audio/music.h"
#include "game/engine/angles.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_geometry.h"
#include "game/race/player/race_player.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

typedef union CollectionTaskResource {
    TaskVisualResource visual;
    struct {
        GsCOORDINATE2 coordinate;
        VECTOR position;
    } trail;
} CollectionTaskResource;

extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

TaskVisualFrame gPlayerParticleHistoryVisualFrames[6]
    __attribute__((section(".data.player_particle_history_visual_frames"), aligned(4))) = {
        {0x30, 0x28, 32, 32, 0xD, 0x3D79},
        {0x50, 0x28, 32, 32, 0xD, 0x3D79},
        {0x70, 0x28, 32, 32, 0xD, 0x3D79},
        {0x90, 0x28, 32, 32, 0xD, 0x3D79},
        {0xB0, 0x28, 32, 32, 0xD, 0x3D79},
        {0xD0, 0x28, 32, 32, 0xD, 0x3D79}
    };

static TaskVisualFrame sOrbitingPlayerVisualFrames[4]
    __attribute__((section(".data.orbiting_player_visual_frames"), aligned(4))) = {
        {0x00, 0x78, 32, 32, 0xD, 0x3CF9},
        {0x20, 0x78, 32, 32, 0xD, 0x3CF9},
        {0x40, 0x78, 32, 32, 0xD, 0x3CF9},
        {0x60, 0x78, 32, 32, 0xD, 0x3CF9}
    };

void drawPlayerFlagTaskVisual(TaskVisualResource *resource) {
    PlayerCamera *cameras = gPlayerCameras;
    gSharedScratchVector.vx = rcos(-cameras[getRenderCameraIndex()].yaw) * 40 / 4096;
    gSharedScratchVector.vz = rsin(-cameras[getRenderCameraIndex()].yaw) * 40 / 4096;
    resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[0] + gSharedScratchVector.vx;
    resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1] - 80;
    resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[2] + gSharedScratchVector.vz;
    setTaskVisualFrame(resource, gPlayerFlagVisualFrames, 0x1800);
    drawCameraFacingTaskVisual(resource, 1);
    advanceTaskVisualFrame(resource);
}

void initializeTaskCoordinateResource(void) {
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    GsInitCoordinate2(0, (GsCOORDINATE2 *)&gTaskMemoryPool[gCurrentTask->resource14]);
    gCurrentTask->data1A = 0;
    gCurrentTask->data1C = 1;
    setCurrentTaskCallback(updatePlayerFlagTaskVisual);
}

void updatePlayerFlagTaskVisual(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gRacePlayers[gCurrentTask->data18].unk946) {
        if (!gCurrentTask->data1A) {
            resource->unkC6 = 0x600;
            resource->unkC8 = 0x100;
            resource->r0 = resource->g0 = resource->b0 = 0x80;
            resource->code = POLY_FT4_CODE;
            resource->framePosition = 0;
            gCurrentTask->data1A = 1;
            gCurrentTask->data1C = 7;
        }
        drawPlayerFlagTaskVisual(resource);
    } else if (gCurrentTask->data1A) {
        gCurrentTask->data1C--;
        if (gCurrentTask->data1C == 0) {
            gCurrentTask->data1A = 0;
        } else {
            drawPlayerFlagTaskVisual(resource);
        }
    }
}

void initializePlayerSideVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (OrbitingPlayerVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->frameLimit = 0x400;
    resource->frameStep = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->framePosition = 0;
    playPlayerSoundEffect(gCurrentTask->data18, 1, 0x600, 0x20);
    queuePlayerControllerEffect(gCurrentTask->data18, 12);
    if (gRacePlayers[gCurrentTask->data18].unk128 & 0x400) {
        resource->radius = -52;
    } else {
        resource->radius = 52;
    }
    setCurrentTaskCallback(updatePlayerSideVisualTask);
}

void updatePlayerSideVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (OrbitingPlayerVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    gSharedScratchVector.vx = rsin((s16)gRacePlayers[gCurrentTask->data18].unk110) * resource->radius / 4096;
    gSharedScratchVector.vz = rcos((s16)gRacePlayers[gCurrentTask->data18].unk110) * resource->radius / 4096;
    resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[0] + gSharedScratchVector.vx;
    resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[2] + gSharedScratchVector.vz;
    setTaskVisualFrame((TaskVisualResource *)resource, gPlayerSideVisualFrames, 0x2000);
    drawCameraFacingTaskVisual((TaskVisualResource *)resource, 1);
    if (!gRacePauseState) {
        if (!advanceTaskVisualFrame((TaskVisualResource *)resource)) {
            deleteCurrentTaskSubtree();
        }
    }
}

void updateOneShotTaskVisual(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    setTaskVisualFrame(resource, D_80124304, 0x2000);
    drawCameraFacingTaskVisual(resource, 1);
    if (!gRacePauseState) {
        if (!advanceTaskVisualFrame(resource)) {
            deleteCurrentTaskSubtree();
        }
    }
}

void initializeExpandingCoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    ExpandingCoordinateEffectResource *initial;
    s32 x, y, z;
    s16 index;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    initial = (ExpandingCoordinateEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    index = initial->rotation.vz;
    x = initial->coordinate.coord.t[0];
    y = initial->coordinate.coord.t[1];
    z = initial->coordinate.coord.t[2];
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (ExpandingCoordinateEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    getCourseHeightAtPoint(index, x, z);
    computeSelectedTriangleAngles(&resource->pitch, &resource->yaw);
    resource->unkCA = 0;
    gCurrentTask->data1A = 45;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    setCurrentTaskCallback(updateExpandingCoordinateEffectTask);
}

void updateExpandingCoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    u16 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (ExpandingCoordinateEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        if (gCurrentTask->data1A >= 16) {
            resource->scale.vx += 0x40;
            resource->scale.vy += 0x40;
            resource->scale.vz += 0x40;
            for (i = 0; i < gActiveRacePlayerCount; i++) {
                addPlayerFlagsIfWithinRadius(&resource->position, resource->scale.vx * 24, 0x80, i);
                gRacePlayers[i].unk0F4 = gCurrentTask->data18;
            }
        } else {
            resource->scale.vx -= 0x80;
            resource->scale.vy -= 0x80;
            resource->scale.vz -= 0x80;
        }
        resource->rotation.vy += 0x80;
        gCurrentTask->data1A--;
    }
    if (isPositionInCameraRange(resource->parent.coord.t[0], resource->parent.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[15].coord2 = &resource->coordinate;
        setCoordinateRotation((SVECTOR *)&resource->pitch, &resource->parent);
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(15);
    }
    if (!gCurrentTask->data1A) { deleteCurrentTaskSubtree(); }
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
        tryCreateChildTaskWith20SlotsReserved(initializeFallingRotatingObjectTask);
        gCreatedTask->data18 = gCurrentTask->data18;
        gCreatedTask->data1A = i;
        gCreatedTask->data1C = value;
        gCreatedTask->data1E = mode;
    }
}

void initializePlayerState23CoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    RacePlayer *players;
    if (gCurrentTask->resource14 != 0xFFFF) { while (1) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (ExpandingCoordinateEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->parent);
    GsInitCoordinate2(&resource->parent, &resource->coordinate);
    players = gRacePlayers;
    resource->selectedIndex = players[gCurrentTask->data18].unk92E;
    resource->rotation.vx = 0;
    resource->rotation.vy = players[gCurrentTask->data18].unk110;
    resource->rotation.vz = 0;
    resource->position.x = players[gCurrentTask->data18].x;
    resource->position.y = players[gCurrentTask->data18].y;
    resource->position.z = players[gCurrentTask->data18].z;
    getCourseHeightAtPoint(resource->selectedIndex, resource->position.x >> 8, resource->position.z >> 8);
    computeSelectedTriangleAngles(&resource->pitch, &resource->yaw);
    resource->unkCA = 0;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    players[gCurrentTask->data18].isActive = 0;
    setCurrentTaskCallback(updatePlayerState23CoordinateEffectTask);
}

void updatePlayerState23CoordinateEffectTask(void) {
    ExpandingCoordinateEffectResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (ExpandingCoordinateEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        RacePlayer *players = gRacePlayers;
        resource->position.x = players[gCurrentTask->data18].x;
        resource->position.y = players[gCurrentTask->data18].y;
        resource->position.z = players[gCurrentTask->data18].z;
        resolvePlayerCollisionsWithCylinder(&resource->position, 0x2FC0, 0x5C00, 2);
        resource->parent.coord.t[0] = resource->position.x >> 8;
        resource->parent.coord.t[1] = resource->position.y >> 8;
        resource->parent.coord.t[2] = resource->position.z >> 8;
        if (resource->scale.vx < 0x1000) {
            resource->scale.vx += 0x400;
            resource->scale.vy = resource->scale.vz = resource->scale.vx;
        }
    }
    if (isPositionInCameraRange(resource->parent.coord.t[0], resource->parent.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[13].coord2 = &resource->coordinate;
        setCoordinateRotation((SVECTOR *)&resource->pitch, &resource->parent);
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(13);
    }
    if (!gRacePauseState) {
        RacePlayer *players = gRacePlayers;
        if (players[gCurrentTask->data18].unk12C != 0x17) {
            players[gCurrentTask->data18].isActive = 1;
            playPlayerSoundEffect(gCurrentTask->data18, 1, 0x301, 0x20);
            createNineIndexedChildTasks(0x12, 0);
            gCurrentTask->callback = deleteCurrentTask;
            gCurrentTask->unk10 |= 1;
        } else if (isFirstPlayerUsingCallback26D10()) {
            deleteCurrentTaskSubtree();
        }
    }
}

void initializeFallingRotatingObjectTask(void) {
    FallingObjectResource *resource;
    s32 heightLimit;

    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (FallingObjectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    if (gCurrentTask->data1E == 0) {
        u8 random;

        random = randomNextSecondary();
        resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[0] + (u8)(random % 96) - 48;
        random = randomNextSecondary();
        resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1] + (u8)(random % 96) - 128;
        random = randomNextSecondary();
        resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[2] + (u8)(random % 96) - 48;
        heightLimit = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1];
    } else {
        GsCOORDINATE2 *parent;

        if (gCurrentTaskParent->resource14 == 0xFFFF) {
            while (1) {}
        }
        parent = (GsCOORDINATE2 *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        resource->coordinate.coord.t[0] = parent->coord.t[0] + (u8)((u8)randomNextSecondary() % 96) - 48;
        resource->coordinate.coord.t[1] = parent->coord.t[1] + (u8)((u8)randomNextSecondary() % 96) - 128;
        resource->coordinate.coord.t[2] = parent->coord.t[2] + (u8)((u8)randomNextSecondary() % 96) - 48;
        heightLimit = parent->coord.t[1];
    }
    resource->heightLimit = heightLimit;
    resource->velocity.x = (u8)((u8)randomNextSecondary() % 21) - 10;
    resource->velocity.y = -(randomNextSecondary() & 32);
    resource->velocity.z = (randomNextSecondary() & 21) - 10;
    gCurrentTask->data1C += gCurrentTask->data1A % 3U;
    setCurrentTaskCallback(updateFallingRotatingObjectTask);
}

void updateFallingRotatingObjectTask(void) {
    FallingObjectResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (FallingObjectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        if (gCurrentTask->data1A & 1) {
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
            deleteCurrentTaskSubtree();
            return;
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[gCurrentTask->data1C].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->parentRotation, &resource->parent);
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(gCurrentTask->data1C);
    }
}

void initializeScaledTaskCoordinateResource(void) {
    ScaledCoordinateResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (ScaledCoordinateResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0xC00;
    setCurrentTaskCallback(updatePlayerScaledObjectTask);
}

void updatePlayerScaledObjectTask(void) {
    PlayerEffectResource *resource;
    RacePlayer *players;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (PlayerEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    players = gRacePlayers;
    resource->coordinate.coord.t[0] = players[gCurrentTask->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = players[gCurrentTask->data18].worldCoordinate.coord.t[1];
    resource->coordinate.coord.t[2] = players[gCurrentTask->data18].worldCoordinate.coord.t[2];
    resource->rotation.vx = players[gCurrentTask->data18].unk0B4;
    resource->rotation.vy = players[gCurrentTask->data18].unk0B6;
    resource->rotation.vz = players[gCurrentTask->data18].unk0B8;
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[10].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(10);
    }
    if (players[gCurrentTask->data18].unk12C != 25) {
        playPlayerSoundEffect(gCurrentTask->data18, 1, 0xF00, 0x20);
        createNineIndexedChildTasks(0x15, 0);
        gCurrentTask->callback = deleteCurrentTask;
        gCurrentTask->unk10 |= 1;
    }
}

void initializePlayerParticleHistoryTask(void) {
    TaskVisualResource *resource;
    PlayerParticleResource *trail;
    u16 i;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    trail = (PlayerParticleResource *)&gTaskMemoryPool[gCurrentTask->resource16];
    GsInitCoordinate2(0, &resource->coordinate);
    gCurrentTask->data1A = 0;
    gCurrentTask->data1C = 0;
    gCurrentTask->data1E = 0;
    for (i = 0; i < 16; i++) {
        trail->entries[i].framePosition = 0xFFFF;
    }
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->code = POLY_FT4_CODE;
    setCurrentTaskCallback(updatePlayerParticleHistoryTask);
}

void updatePlayerParticleHistoryTask(void) {
    TaskVisualResource *resource;
    PlayerParticleResource *trail;
    RacePlayer *players;
    u16 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    trail = (PlayerParticleResource *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (!gRacePauseState) {
        players = gRacePlayers;
        if (players[gCurrentTask->data18].unk12C == 0x1A) {
            gCurrentTask->data1A++;
            if (gCurrentTask->data1A & 1) {
                trail->entries[gCurrentTask->data1C].position.x = players[gCurrentTask->data18].worldCoordinate.coord.t[0] + (((u32)randomNextSecondary() & 0xFF) >> 1) - 0x40;
                trail->entries[gCurrentTask->data1C].position.y = players[gCurrentTask->data18].worldCoordinate.coord.t[1];
                trail->entries[gCurrentTask->data1C].position.z = players[gCurrentTask->data18].worldCoordinate.coord.t[2] + (((u32)randomNextSecondary() & 0xFF) >> 1) - 0x40;
                trail->entries[gCurrentTask->data1C].framePosition = 0;
                gCurrentTask->data1C = (gCurrentTask->data1C + 1) & 0xF;
            }
        } else {
            if (!gCurrentTask->data1E) {
                gCurrentTask->data1E = 1;
                gCurrentTask->data1A = 13;
            }
            if (gCurrentTask->data1E) {
                if (--gCurrentTask->data1A == 0) {
                    deleteCurrentTaskSubtree();
                    return;
                }
            }
        }
    }
    for (i = 0; i < 16; i++) {
        if (trail->entries[i].framePosition < 0x600) {
            resource->framePosition = trail->entries[i].framePosition;
            resource->coordinate.coord.t[0] = trail->entries[i].position.x;
            resource->coordinate.coord.t[1] = trail->entries[i].position.y;
            resource->coordinate.coord.t[2] = trail->entries[i].position.z;
            if (!gRacePauseState) {
                trail->entries[i].position.y -= 2;
                trail->entries[i].framePosition += 0x80;
            }
            setTaskVisualFrame(resource, gPlayerParticleHistoryVisualFrames, 0x2000);
            drawCameraFacingTaskVisual(resource, 1);
        }
    }
}

void initializePlayerCoordinateEffectTask(void) {
    PlayerEffectResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (PlayerEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    gCurrentTask->data1A = 0;
    resource->rotation.vx = resource->rotation.vy = resource->rotation.vz = 0;
    resource->scale.vx = resource->scale.vy = resource->scale.vz = 0;
    playPlayerSoundEffect(gCurrentTask->data18, 1, 0x800, 0x20);
    setCurrentTaskCallback(updatePlayerCoordinateEffectTask);
}

void updatePlayerCoordinateEffectTask(void) {
    PlayerEffectResource *resource;
    RacePlayer *players;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (PlayerEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        players = gRacePlayers;
        if (players[gCurrentTask->data18].unk12C == 0x1B) {
            resource->coordinate.coord.t[0] = players[gCurrentTask->data18].worldCoordinate.coord.t[0];
            resource->coordinate.coord.t[1] = players[gCurrentTask->data18].worldCoordinate.coord.t[1];
            resource->coordinate.coord.t[2] = players[gCurrentTask->data18].worldCoordinate.coord.t[2];
            if (resource->scale.vx < 0x1000) {
                resource->scale.vx += 0x200;
                resource->scale.vy += 0x200;
                resource->scale.vz += 0x200;
            }
        } else {
            if (isFirstPlayerUsingCallback26D10()) {
                deleteCurrentTaskSubtree();
                return;
            }
            if (resource->scale.vy == 0) {
                resource->scale.vx -= 0x400;
                resource->scale.vz -= 0x400;
                if (resource->scale.vx <= 0) {
                    deleteCurrentTaskSubtree();
                    return;
                }
            } else {
                resource->scale.vy -= 0x100;
            }
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[11].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(11);
    }
}

s32 findNearestHomingTargetPlayer(const Position3D *position, s32 radius, s16 facing, s16 excludedPlayer, s16 *bearing) {
    s32 dx, dy, dz, distance;
    s16 angle;
    RacePlayer *player;
    RacePlayer *players;
    s32 i;
    s32 nearest = radius + 0x1000;
    s32 result = -1;

    for (i = 0; i < gActiveRacePlayerCount; i++) {
        players = gRacePlayers;
        player = &players[i];
        if (i != excludedPlayer && player->surfaceState.fields.unk134 == 0
            && !(player->unk128 & 0x402000) && player->isActive) {
            dx = player->x - position->x;
            if (dx < 0) {
                dx = -dx;
            }
            if (dx < radius) {
                dy = player->unk074 - position->y;
                if (dy < 0) {
                    dy = -dy;
                }
                if (dy < radius) {
                    dz = player->z - position->z;
                    if (dz < 0) {
                        dz = -dz;
                    }
                    if (dz < radius) {
                        angle = getXZHeading(position->x, position->z, player->x, player->z);
                        if (((s16)(angle - facing + 0x380) & 0xFFF) < 0x700) {
                            distance = SquareRoot0(dx * dx + dz * dz);
                            if (distance < radius && distance < nearest) {
                                result = i;
                                nearest = distance;
                                *bearing = angle;
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

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
    if (gRacePlayers[gCurrentTask->data18].unk128 & 0x400) {
        resource->unkD8 = (s16)gRacePlayers[gCurrentTask->data18].unk110 + 0x800;
    } else {
        resource->unkD8 = (s16)gRacePlayers[gCurrentTask->data18].unk110;
    }
    players = gRacePlayers;
    resource->coordinate.coord.t[0] = players[gCurrentTask->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = players[gCurrentTask->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = players[gCurrentTask->data18].worldCoordinate.coord.t[2];
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(players[gCurrentTask->data18].unk92E, players[gCurrentTask->data18].x >> 8, players[gCurrentTask->data18].z >> 8);
    resource->unkDC = value;
    resource->groundY = scale;
    player = &players[gCurrentTask->data18];
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
    gCurrentTask->data1A = 0;
    tryCreateChildTaskForParentWith20SlotsReserved(&gGameTasks[4], initializePlayerSideVisualTask);
    gCreatedTask->data18 = gCurrentTask->data18;
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
    drawPreparedTaskVisualQuad(resource, frames, 0);
    resource->code = POLY_FT4_CODE | 2;
    for (i = 0; i < 4; i++) {
        entry = history;
        entry += i;
        if (entry->pad != 0) {
            resource->coordinate.coord.t[0] = entry->vx >> 8;
            resource->coordinate.coord.t[1] = entry->vy >> 8;
            resource->coordinate.coord.t[2] = entry->vz >> 8;
            phase = (gCurrentTask->data1A - i) & 3;
            resource->r0 = resource->g0 = resource->b0 = 0x80 - (phase << 3);
            resource->scaleX = 0x1A00 + (phase << 9);
            resource->scaleY = 0x1A00 + (phase << 9);
            resource->scaleZ = 0x1A00 + (phase << 9);
            prepareTaskVisualQuad(resource, frames, 1);
            drawPreparedTaskVisualQuad(resource, frames, 0);
        }
    }
    if (gRacePauseState == 0) {
        gCurrentTask->data1A = (gCurrentTask->data1A + 1) & 3;
    }
}

void updateHomingTaskVisualMovement(TaskVisualResource *resource, VECTOR *history, s32 distance, s32 maxTurn) {
    VECTOR *entry;
    s16 angle;
    s32 player;
    s32 oldY;
    s32 ground;
    entry = history;
    entry += gCurrentTask->data1A;
    entry->vx = resource->position.x;
    entry = history;
    entry += gCurrentTask->data1A;
    entry->vy = resource->position.y;
    entry = history;
    entry += gCurrentTask->data1A;
    entry->vz = resource->position.z;
    history += gCurrentTask->data1A;
    history->pad = 1;
    player = (s16)findNearestHomingTargetPlayer(&resource->position, distance, resource->unkD8, gCurrentTask->data18, &angle);
    resource->unkE0 = player;
    if (player != -1) {
        gRacePlayers[player].unk946 = 1;
        angle = (angle - resource->unkD8) & 0xFFF;
        if (angle > 0x800) angle -= 0x1000;
        if (angle > (s16)maxTurn) angle = maxTurn;
        if (angle < -(s16)maxTurn) angle = -maxTurn;
        resource->unkD8 += angle;
    }
    gSharedScratchVector.vx = rsin(resource->unkD8) * resource->groundY / 4096;
    gSharedScratchVector.vz = rcos(resource->unkD8) * resource->groundY / 4096;
    oldY = resource->position.y;
    resource->position.x += gSharedScratchVector.vx;
    resource->position.y += resource->velocityY;
    resource->position.z += gSharedScratchVector.vz;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
    ground = getCourseHeightAtPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    if (resource->position.y > ground) resource->position.y = ground;
    resource->velocityY = resource->position.y - oldY + 0x1900;
}

void checkTaskVisualOtherPlayerCollision(TaskVisualResource *resource, s16 flags) {
    u16 i;
    RacePlayer *player;
    for (i = 0; i < gActiveRacePlayerCount; i++) {
        if (i != gCurrentTask->data18) {
            player = &gRacePlayers[i];
            if (player->unk946 != 0) {
                if (addPlayerFlagsIfWithinRadius(&resource->position, 0xF00, flags, i) != 0) {
                    player->unk0F4 = gCurrentTask->data18;
                    resource->unkDC = 0;
                    break;
                }
            }
        }
    }
}

void initializePlayerDualResourceEffectTask(void) {
    TaskMemoryBlock *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = &gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16], 300, 0x5580);
    setCurrentTaskCallback(updatePlayerDualResourceEffectTask);
}

void updatePlayerDualResourceEffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x63000, 0x1C);
        getGeometryBoundaryDisplacement(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerDualResourceEffectFrame);
    if (gRacePauseState == 0) {
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
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializePlayerVisualHistory(resource, (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16], 300, 0x5580);
    setCurrentTaskCallback(updatePlayerVisualPairTask);
}

void updatePlayerVisualPairTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x51000, 0x12);
        getGeometryBoundaryDisplacement(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPairFrame);
    if (gRacePauseState == 0) {
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
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializePlayerVisualHistory(resource, (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16], 300, 0x5E80);
    setCurrentTaskCallback(updatePlayerVisualPairTask5E80);
}

void updatePlayerVisualPairTask5E80(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x3F000, 0x18);
        getGeometryBoundaryDisplacement(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPair5E80Frame);
    if (gRacePauseState == 0) {
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
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializePlayerVisualHistory(resource, (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16], 180, 0x6780);
    setCurrentTaskCallback(updatePlayerVisualPairTask6780);
}

void updatePlayerVisualPairTask6780(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x1B000, 0x1C);
        reflectVelocityAndResolveGeometryBoundary(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz, &gSharedScratchVector.vx, &gSharedScratchVector.vz);
        if (dx != 0 || dz != 0) {
            resource->position.x += dx;
            resource->position.z += dz;
            resource->unkD8 = getXZHeading(0, 0, gSharedScratchVector.vx, gSharedScratchVector.vz);
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerVisualPair6780Frame);
    if (gRacePauseState == 0) {
        checkTaskVisualOtherPlayerCollision(resource, 0x40);
        if (resource->unkDC == 0) {
            resetTaskVisualState(resource);
        } else {
            resource->unkDC--;
        }
    }
}

void initializeDualResource4C80EffectTask(void) {
    TaskMemoryBlock *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = &gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16], 300, 0x4C80);
    setCurrentTaskCallback(updateDualResource4C80EffectTask);
}

void updateDualResource4C80EffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        updateHomingTaskVisualMovement(resource, history, 0x2D000, 0x10);
        getGeometryBoundaryDisplacement(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, D_80124354);
    if (gRacePauseState == 0) {
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
    if (gCurrentTask->resource14 != 0xFFFF) { while (1) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x200;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    if (gRacePlayers[gCurrentTask->data18].unk128 & 0x400) {
        resource->unkD8 = (s16)gRacePlayers[gCurrentTask->data18].unk110;
    } else {
        resource->unkD8 = (s16)gRacePlayers[gCurrentTask->data18].unk110 + 0x800;
    }
    gSharedScratchVector.vx = rsin(resource->unkD8) / 64;
    gSharedScratchVector.vz = rcos(resource->unkD8) / 64;
    players = gRacePlayers;
    resource->coordinate.coord.t[0] = players[gCurrentTask->data18].worldCoordinate.coord.t[0] + gSharedScratchVector.vx;
    resource->coordinate.coord.t[1] = players[gCurrentTask->data18].worldCoordinate.coord.t[1] - 18;
    resource->coordinate.coord.t[2] = players[gCurrentTask->data18].worldCoordinate.coord.t[2] + gSharedScratchVector.vz;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(players[gCurrentTask->data18].unk92E, players[gCurrentTask->data18].x >> 8, players[gCurrentTask->data18].z >> 8);
    resource->velocityY = -0xA00;
    resource->groundY = getCourseHeightAtPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    resource->bounceCount = 0;
    playPlayerSoundEffect(gCurrentTask->data18, 1, 2, 0x20);
    setCurrentTaskCallback(updateBouncingTaskVisual);
}

void updateBouncingTaskVisual(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
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
    drawCameraFacingTaskVisual(resource, 1);
    if (!gRacePauseState && resource->bounceCount == 2) {
        resource->unkDC = 30;
        setCurrentTaskCallback(updateAcceleratingTaskVisualCountdown);
    }
}

void updateAcceleratingTaskVisualCountdown(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    setTaskVisualFrame(resource, D_80124354, 0x1A00);
    drawCameraFacingTaskVisual(resource, 1);
    if (!gRacePauseState) {
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
    TaskMemoryBlock *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = &gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    initializePlayerVisualHistory((TaskVisualResource *)resource, (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16], 300, 0x5100);
    gCurrentTask->data1A = 0;
    setCurrentTaskCallback(updateDualResource5100EffectTask);
}

void updateDualResource5100EffectTask(void) {
    TaskVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    s32 oldY, ground;
    u16 i;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->vx = resource->position.x;
        }
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->vy = resource->position.y;
        }
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->vz = resource->position.z;
        }
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->pad = 1;
        }

        gSharedScratchVector.vx = rsin(resource->unkD8) * resource->groundY / 4096;
        gSharedScratchVector.vz = rcos(resource->unkD8) * resource->groundY / 4096;
        oldY = resource->position.y;
        resource->position.x += gSharedScratchVector.vx;
        resource->position.y += resource->velocityY;
        resource->position.z += gSharedScratchVector.vz;
        resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
        ground = getCourseHeightAtPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
        if (resource->position.y > ground) {
            resource->position.y = ground;
        }
        resource->velocityY = resource->position.y - oldY + 0x1900;
        getGeometryBoundaryDisplacement(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz);
        if (dx != 0 || dz != 0) {
            resource->unkDC = 0;
            resource->position.x += dx;
            resource->position.z += dz;
        }
    }
    drawTaskVisualPositionHistory(resource, history, &gPlayerDualResource5100EffectFrame);
    if (gRacePauseState == 0) {
        for (i = 0; i < gActiveRacePlayerCount; i++) {
            if (i != gCurrentTask->data18 &&
                addPlayerFlagsIfWithinRadius(&resource->position, 0x1000, 0x4000, i) != 0) {
                gRacePlayers[i].unk0F4 = gCurrentTask->data18;
                resource->unkDC = 0;
                return;
            }
        }
        if (resource->unkDC == 0) {
            gRacePlayers[gCurrentTask->data18].unk93C++;
            resetTaskVisualState(resource);
        } else {
            PlayerCollectionEntry *entry;
            u16 playerIndex;
            resource->unkDC--;
            /* The vector pad shares storage with the collecting player index. */
            gPlayerCollectionEntries[gPlayerCollectionEntryCount].location.vector = ((CollectionTaskResource *)resource)->trail.position;
            entry = &gPlayerCollectionEntries[gPlayerCollectionEntryCount];
            playerIndex = gCurrentTask->data18;
            entry->active = &resource->unkDC;
            entry->location.player.playerIndex = playerIndex;
            if (++gPlayerCollectionEntryCount >= 10) {
                gPlayerCollectionEntryCount--;
            }
        }
    }
}

void initializeVisualTrailTask(void) {
    TrailVisualResource *resource;
    VECTOR *vectors;
    u16 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (TrailVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) { while (1) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    vectors = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC6 = 0;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    gSharedScratchVector.vx = rsin(resource->angle) / 64;
    gSharedScratchVector.vz = rcos(resource->angle) / 64;
    resource->lifetime = 60;
    resource->horizontalSpeed = 0x6780;
    resource->velocityY = 0;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    for (i = 0; i < 4; i++) {
        VECTOR *vector = &vectors[i];
        vector->vx = vector->vy = vector->vz = 0;
        vector->pad = 0;
    }
    gCurrentTask->data1A = 0;
    setCurrentTaskCallback(updateVisualTrailTask);
}

void updateVisualTrailTask(void) {
    TrailVisualResource *resource;
    VECTOR *history;
    s32 dx, dz;
    s32 oldY;
    s32 ground;
    u16 i;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TrailVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    history = (VECTOR *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gRacePauseState == 0) {
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->vx = resource->position.x;
        }
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->vy = resource->position.y;
        }
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->vz = resource->position.z;
        }
        {
            VECTOR *entry = &history[gCurrentTask->data1A];
            entry->pad = 1;
        }

        gSharedScratchVector.vx = rsin(resource->angle) * resource->horizontalSpeed / 4096;
        gSharedScratchVector.vz = rcos(resource->angle) * resource->horizontalSpeed / 4096;
        oldY = resource->position.y;
        resource->position.x += gSharedScratchVector.vx;
        resource->position.y += resource->velocityY;
        resource->position.z += gSharedScratchVector.vz;
        resource->geometryIndex = getAdjacentGeometryRegionForPoint(resource->geometryIndex, resource->position.x >> 8, resource->position.z >> 8);
        ground = getCourseHeightAtPoint(resource->geometryIndex, resource->position.x >> 8, resource->position.z >> 8) << 8;
        if (resource->position.y > ground) {
            resource->position.y = ground;
        }
        resource->velocityY = resource->position.y - oldY + 0x1900;
        reflectVelocityAndResolveGeometryBoundary(resource->geometryIndex, resource->position.x >> 8, resource->position.z >> 8, 2, &dx, &dz, &gSharedScratchVector.vx, &gSharedScratchVector.vz);
        if (dx != 0 || dz != 0) {
            resource->position.x += dx;
            resource->position.z += dz;
            resource->angle = getXZHeading(0, 0, gSharedScratchVector.vx, gSharedScratchVector.vz);
        }
    }
    drawTaskVisualPositionHistory((TaskVisualResource *)resource, history, &gPlayerVisualPair6780Frame);
    if (gRacePauseState == 0) {
        for (i = 0; i < gActiveRacePlayerCount; i++) {
            if (addPlayerFlagsIfWithinRadius(&resource->position, 0x1000, 0x2000, i) != 0) {
                resource->lifetime = 0;
                break;
            }
        }
        if (resource->lifetime == 0) {
            resetTaskVisualState((TaskVisualResource *)resource);
        } else {
            resource->lifetime--;
        }
    }
}

void initPlayerTimerTask(void) {
    PlayerTimerState *timer = &gRacePlayers[gCurrentTask->data18].timer;

    GsInitCoordinate2(0, &timer->coordinate);
    timer->unk60 = 0;
    gRacePlayers[gCurrentTask->data18].unk0FE = 1;
    setCurrentTaskCallback(updatePlayerTimerTask);
}

void updatePlayerTimerTask(void) {
    if (!gRacePauseState) {
        if (gRacePlayers[gCurrentTask->data18].unk128 & 0x3040) {
            gRacePlayers[gCurrentTask->data18].unk0FC = 0;
        }
        if (gRacePlayers[gCurrentTask->data18].unk0FC != 0) {
            gRacePlayers[gCurrentTask->data18].unk0FC--;
        }
        if (gRacePlayers[gCurrentTask->data18].unk0FC == 0) {
            if (!gRacePlayers[gCurrentTask->data18].unk008 &&
                gControllerEffectStates[gCurrentTask->data18].effect == 13 &&
                !(gRacePlayers[gCurrentTask->data18].unk128 & 0x2000)) {
                D_8013AF08[gCurrentTask->data18] = 0;
            }
            setCurrentTaskCallback(fadeOutPlayerTimerTask);
        } else if (gRacePlayers[gCurrentTask->data18].timer.unk60 < 0x1000) {
            gRacePlayers[gCurrentTask->data18].timer.unk60 += 0x100;
        }
    }
}

void fadeOutPlayerTimerTask(void) {
    if (!gRacePauseState) {
        if (gRacePlayers[gCurrentTask->data18].unk0FC == 0) {
            gRacePlayers[gCurrentTask->data18].timer.unk60 -= 0x200;
            if (gRacePlayers[gCurrentTask->data18].timer.unk60 > 0) {
                return;
            }
            gRacePlayers[gCurrentTask->data18].timer.unk60 = 0;
            gRacePlayers[gCurrentTask->data18].unk0FE = 0;
        }
        deleteCurrentTaskSubtree();
    }
}

void initializeOrbitingPlayerVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (OrbitingPlayerVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    playPlayerSoundEffect(gCurrentTask->data18, 1, 0xE00, 0x20);
    resource->countdown = 0x10;
    gCurrentTask->data1C = 0;
    setCurrentTaskCallback(updateOrbitingPlayerVisualTask);
}

void updateOrbitingPlayerVisualTask(void) {
    OrbitingPlayerVisualResource *resource;
    PlayerCamera *cameras;
    u32 roll;
    u8 target;
    u16 cameraIndex;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (OrbitingPlayerVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        switch (gCurrentTask->data1C) {
        case 0:
            resource->offset.y -= 4;
            resource->r0 += 4;
            resource->g0 += 4;
            resource->b0 += 4;
            if (--resource->countdown == 0) {
                resource->code &= ~2;
                resource->countdown = 8;
                gCurrentTask->data1C++;
            }
            break;
        case 1:
            if (--resource->countdown == 0) {
                resource->countdown = 16;
                gCurrentTask->data1C++;
            }
            break;
        case 2:
            if (resource->scale > 0) {
                resource->scale -= 0x180;
            }
            if (--resource->countdown == 0) {
                roll = randomNextMain() & 0xFF;
                if (roll != 0) {
                    target = 0;
                    if (gActiveRacePlayerCount >= 4) {
                        u32 player = gRaceOrderPlayerIds[3];

                        if (gCurrentTask->data18 != player) {
                            target = player;
                        }
                    }
                    if (gActiveRacePlayerCount >= 3) {
                        u8 *order = gRaceOrderPlayerIds;
                        u8 player = order[2];

                        if (gCurrentTask->data18 != player && roll >= 2 &&
                            !(gRacePlayers[order[2]].unk128 & 0x40)) {
                            target = player;
                        }
                    }
                    if (gActiveRacePlayerCount >= 2) {
                        u8 *order = gRaceOrderPlayerIds;
                        u8 player = order[1];

                        if (gCurrentTask->data18 != player && roll >= 3 &&
                            !(gRacePlayers[order[1]].unk128 & 0x40)) {
                            target = player;
                        }
                    }
                    if (gActiveRacePlayerCount > 0) {
                        u8 *order = gRaceOrderPlayerIds;
                        u8 player = order[0];

                        if (gCurrentTask->data18 != player && roll >= 4 &&
                            !(gRacePlayers[order[0]].unk128 & 0x40)) {
                            target = player;
                        }
                    }
                    gCurrentTask->data18 = target & 3;
                }
                gRacePlayers[gCurrentTask->data18].unk0FA++;
                resource->offset.y = -48;
                resource->radius = 48;
                resource->r0 = resource->g0 = resource->b0 = 0x60;
                resource->code |= 2;
                resource->countdown = 300;
                resource->unkD0 = 0;
                playPlayerSoundEffect(gCurrentTask->data18, 1, 0xE00, 0x20);
                gCurrentTask->data1C++;
            }
            break;
        case 3:
            if (resource->countdown < 16) {
                resource->scale -= 0x180;
            } else if (resource->scale < 0x1800) {
                resource->scale += 0x180;
            }
            if (resource->unkD0 == 0) {
                resource->r0 += 4;
                resource->g0 += 4;
                resource->b0 += 4;
                if (resource->r0 >= 0xA0) {
                    resource->unkD0 = 1;
                }
            } else {
                resource->r0 -= 4;
                resource->g0 -= 4;
                resource->b0 -= 4;
                if (resource->r0 <= 0x60) {
                    resource->unkD0 = 0;
                }
            }
            resource->angle += 0x60;
            if (--resource->countdown == 0) {
                gRacePlayers[gCurrentTask->data18].unk0FA--;
                deleteCurrentTaskSubtree();
                return;
            }
            break;
        }
        cameraIndex = getRenderCameraIndex();
        cameras = gPlayerCameras;
        resource->offset.x = rsin(cameras[cameraIndex].yaw + resource->angle) * resource->radius / 4096;
        resource->offset.z = rcos(cameras[getRenderCameraIndex()].yaw + resource->angle) * resource->radius / 4096;
        resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[0] + resource->offset.x;
        resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1] + resource->offset.y;
        resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[2] + resource->offset.z;
        resource->framePosition += resource->frameStep;
        if (resource->framePosition >= resource->frameLimit) {
            resource->framePosition &= 0xFF;
        }
    }
    setTaskVisualFrame((TaskVisualResource *)resource, sOrbitingPlayerVisualFrames, resource->scale);
    drawCameraFacingTaskVisual((TaskVisualResource *)resource, 1);
}

void initializeDelayedPlayerRiseEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (gCurrentTask->data1C == 0) {
        if (gCurrentTask->resource14 != 0xFFFF) {
            while (1) {}
        }
        gCurrentTask->resource14 = allocateTaskMemoryBlock();
        resource = (RisingPlayerEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
        gCurrentTask->data1C--;
    }
}

void updateRisingPlayerEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RisingPlayerEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        resource->velocity -= 2;
        resource->offset.y += resource->velocity;
        if (resource->offset.y < -320) {
            resource->velocity = 2;
            resource->unkD8 = 1;
            gCurrentTask->data1C = 0;
            setCurrentTaskCallback(updateFallingPlayerEffectTask);
            return;
        }
        resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data1A].worldCoordinate.coord.t[0];
        resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data1A].worldCoordinate.coord.t[1] + resource->offset.y;
        resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data1A].worldCoordinate.coord.t[2];
        if (resource->scale.vx < 0x1000) {
            resource->scale.vx += 0x100;
            resource->scale.vy += 0x100;
            resource->scale.vz += 0x100;
        }
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[12].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(12);
    }
}

void updateFallingPlayerEffectTask(void) {
    RisingPlayerEffectResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RisingPlayerEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        resource->velocity += 2;
        resource->offset.y += resource->velocity;
        switch (gCurrentTask->data1C) {
        case 0:
            gRacePlayers[gCurrentTask->data18].unk946 = 1;
            if (resource->offset.y > -128 && resource->unkD8) {
                playPlayerSoundEffect(gCurrentTask->data18, 1, 0x900, 0x20);
                gRacePlayers[gCurrentTask->data18].unk0E8 |= 0x1000;
                gRacePlayers[gCurrentTask->data18].unk0F4 = gCurrentTask->data1A;
                resource->unkD8 = 0;
                return;
            }
            if (resource->offset.y > -64) {
                resource->velocity = -16;
                resource->countdown = 16;
                gCurrentTask->data1C++;
            }
            break;
        case 1:
            if (--resource->countdown == 0) {
                deleteCurrentTaskSubtree();
                return;
            }
            break;
        }
        resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[0];
        resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1] + resource->offset.y;
        resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[2];
    }
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[12].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        ScaleMatrix(&resource->coordinate.coord, &resource->scale);
        drawTaskObject(12);
    }
}

void initializeSlidingVisualSequenceTask(void) {
    SlidingVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (SlidingVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 128;
    resource->code = POLY_FT4_CODE;
    resource->step = 0;
    resource->timer = 0;
    resource->framePosition = 0;
    resource->offsetY = -352;
    resource->velocity = 0;
    playSoundEffect(1, 0xD01, 0x20);
    gCurrentTask->data1C = 0;
    setCurrentTaskCallback(updateSlidingVisualSequenceTask);
}
