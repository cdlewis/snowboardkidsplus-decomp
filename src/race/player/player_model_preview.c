#include "game/io/dat_resources.h"
#include "psx/libetc.h"
#include "../../menu/panel_tasks_internal.h"
#include "game/engine/random.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

extern VECTOR *gScratchVector0 __attribute__((section(".data")));
extern SVECTOR *gScratchShortVector0 __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

u16 playerModelDrawAreaDepths[4] __attribute__((section(".data.player_model_draw_area_depths"))) = {
    0x70, 0x40, 0x20, 0
};

void sortPlayerModelWithDrawArea(u16 playerIndex, u16 depthIndex, RECT *bounds) {
    MATRIX matrix;
    RacePlayer *player = &gRacePlayers[playerIndex];
    GsDOBJ2 *object = player->modelObjects;
    s16 i;
    s16 count;
    currentPlayerAbsolute = player;
    count = player->modelPartCount;
    for (i = 0; i < count; i++, object++) {
        GsGetLw(object->coord2, &matrix);
        GsSetLightMatrix(&matrix);
        GsGetLs(object->coord2, &matrix);
        GsSetLsMatrix(&matrix);
        GsSortObject4(object, &gMainOrderingTables[gActiveGraphicsBuffer], depthIndex, (u32 *)getScratchAddr(0));
    }
    if (bounds->w >= 0 && bounds->h >= 0) {
        addDrawAreaPacket(bounds->x, bounds->y, bounds->w, bounds->h,
                         &gMainOrderingTables[gActiveGraphicsBuffer], playerModelDrawAreaDepths[depthIndex]);
        addDrawAreaPacket(0, 0, 320, 240, &gMainOrderingTables[gActiveGraphicsBuffer], 14);
    }
}

void loadCurrentPlayerModelPose(const s16 *values) {
    RacePlayer *player;
    s16 i;
    initCurrentPlayerModelParts();
    player = currentPlayerAbsolute;
    for (i = 0; i < player->modelPartCount; i++) {
        player->unk5BC[i].x = *values++;
        player->unk5BC[i].y = *values++;
        player->unk5BC[i].z = *values++;
    }
    {
        RacePlayer *translationPlayer = currentPlayerAbsolute;
        translationPlayer->unk62C[0].x = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[0].y = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[0].z = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[1].z = -(*values + 2) / 4;
        values++;
        translationPlayer->unk62C[1].y = (*values + 2) / 4;
        values++;
        translationPlayer->unk62C[1].x = (*values + 2) / 4;
        values++;
    }
    setCurrentPlayerFinalPartTranslations(values);
    updateCurrentPlayerModelPartCoordinates();
}

void setCurrentPlayerFinalPartTranslations(const s16 *values) {
    RacePlayer *player = currentPlayerAbsolute;
    s32 index = player->modelPartCount - 2;
    player->unk62C[index].x = *values++;
    player->unk62C[index].y = *values++;
    player->unk62C[index].z = *values++;
    index = player->modelPartCount - 1;
    player->unk62C[index].x = *values++;
    player->unk62C[index].y = *values++;
    player->unk62C[index].z = *values++;
}

void updateCurrentPlayerModelPartCoordinates(void) {
    s16 i;
    u8 parent;
    for (i = 0; i < currentPlayerAbsolute->modelPartCount; i++) {
        parent = currentPlayerAbsolute->unk13E[i];
        RotMatrix((SVECTOR *)&currentPlayerAbsolute->unk5BC[i], &currentPlayerAbsolute->model.modelCoordinates[i].coord);
        if (i == 0) {
            gScratchShortVector0->vx = 0;
            gScratchShortVector0->vy = 0;
            gScratchShortVector0->vz = 0;
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[0].coord, gScratchShortVector0, gScratchVector0);
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[0] = gScratchVector0->vx + currentPlayerAbsolute->unk62C[0].x;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[1] = gScratchVector0->vy + currentPlayerAbsolute->unk62C[0].y;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[2] = gScratchVector0->vz + currentPlayerAbsolute->unk62C[0].z;
        } else {
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[parent].coord, (SVECTOR *)&currentPlayerAbsolute->unk62C[i], &gSharedScratchVector);
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[0] = gSharedScratchVector.vx + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[0];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[1] = gSharedScratchVector.vy + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[1];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[2] = gSharedScratchVector.vz + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[2];
        }
        setCoordinateRotation((SVECTOR *)&currentPlayerAbsolute->unk5BC[i], &currentPlayerAbsolute->model.modelCoordinates[i]);
    }
}

static inline RegisteredTaskEntry *getSignedModelPreviewEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&gTaskMemoryPool[resource];
    return &entries[index / 10];
}

void createPlayerModelPreviewTask(u16 playerIndex, u16 characterId, u16 modelSlot,
                                  s16 x, s16 y, s16 z, s32 index) {
    u32 *source;
    u32 *destination;
    u32 *model;
    s32 i = 0;
    ModelPreviewState *state;
    RacePlayer *player;
    Movement2D *position;
    RegisteredTaskEntry *entry;

    createChildTaskWithReserve(updatePlayerModelPreviewTask);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1A = modelSlot;
    gCreatedTask->data1C = playerIndex;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();

    currentPlayerAbsolute = &gRacePlayers[gCreatedTask->data1C];
    state = (ModelPreviewState *)&gTaskMemoryPool[gCreatedTask->resource14];
    currentPlayerAbsolute->characterId = characterId;
    {
        u32 (*sourceBuffers)[4096] = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
        u32 (*destinationBuffers)[4096] = DAT_PLAYER_MODEL_BUFFERS;

        destination = destinationBuffers[gCreatedTask->data1C];
        source = sourceBuffers[gCreatedTask->data1A];
    }
    for (; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[gCreatedTask->data1C];
    model++;
    GsMapModelingData(model);
    model++;
    currentPlayerAbsolute->modelPartCount = *(u8 *)model;
    GsInitCoordinate2(0, &currentPlayerAbsolute->worldCoordinate);
    GsInitCoordinate2(&currentPlayerAbsolute->worldCoordinate, &currentPlayerAbsolute->modelCoordinate);
    model++;
    for (i = 0; i < currentPlayerAbsolute->modelPartCount; i++) {
        GsDOBJ2 *object;
        GsLinkObject4(model, &currentPlayerAbsolute->modelObjects[i], i);
        object = &currentPlayerAbsolute->modelObjects[i];
        GsInitCoordinate2(&currentPlayerAbsolute->modelCoordinate, &currentPlayerAbsolute->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &currentPlayerAbsolute->model.modelCoordinates[i];
    }

    player = currentPlayerAbsolute;
    player->worldCoordinate.coord.t[0] = x;
    player->worldCoordinate.coord.t[1] = y;
    player->worldCoordinate.coord.t[2] = z;
    player->unk0B4 = 0;
    player->unk0B6 = 0;
    player->unk0B8 = 0;
    player->modelCoordinate.coord.t[0] = 58;
    player->modelCoordinate.coord.t[1] = 0;
    player->modelCoordinate.coord.t[2] = 0;
    player->modelRotation.vx = 0;
    player->modelRotation.vy = 1024;
    player->modelRotation.vz = 0;
    player->model.modelCoordinates[0].coord.t[0] = 0;
    player->model.modelCoordinates[0].coord.t[1] = 0;
    player->model.modelCoordinates[0].coord.t[2] = 0;
    player->unk5BC[0].x = 512;
    player->unk5BC[0].y = 0;
    player->unk5BC[0].z = 0;

    state->rotationStep = 0;
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    position = (Movement2D *)&gTaskMemoryPool[gCreatedTask->resource16];
    initMovement2D(position, x << 12, y << 12);
    position->unk08 = z;

    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
        entry = getSignedModelPreviewEntry(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
        entry = getSignedModelPreviewEntry(gCurrentTask->resource16, index);
    }
    entry->command = 0;
    entry->flags = 10;
    entry->task = gCreatedTask;
}

void updatePlayerModelPreviewTask(void) {
    u8 padding[0x10];
    ModelPreviewState *state;
    RegisteredTaskEntry *entry;
    currentPlayerAbsolute = &gRacePlayers[gCurrentTask->data1C];
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    state = (ModelPreviewState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    if ((u16)(gCurrentTask->data1E % 10) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }
    switch (entry->command) {
    case 1: state->rotationStep = 64; entry->flags = 0; break;
    case 3: state->rotationStep = 0; entry->flags = 0; break;
    case 0: entry->flags = 0; break;
    case 99:
        entry->task = 0;
        entry->flags = 0;
        deleteCurrentTaskSubtree();
        return;
    }
    currentPlayerAbsolute->unk0B6 = (currentPlayerAbsolute->unk0B6 + state->rotationStep) & 0xFFF;
    setCoordinateRotation((SVECTOR *)&currentPlayerAbsolute->unk0B4, &currentPlayerAbsolute->worldCoordinate);
    setCoordinateRotation(&currentPlayerAbsolute->modelRotation, &currentPlayerAbsolute->modelCoordinate);
    setCoordinateRotation((SVECTOR *)currentPlayerAbsolute->unk5BC, currentPlayerAbsolute->model.modelCoordinates);
    sortPlayerModelWithDrawArea(gCurrentTask->data1C, 0, &gMenuModelDrawAreas[gCurrentTask->data1C]);
}
