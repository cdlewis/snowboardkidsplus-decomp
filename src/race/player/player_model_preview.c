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

extern VECTOR *D_8013AF88 __attribute__((section(".data")));
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));

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
        GsSortObject4(object, &D_800EBFF0[D_8013AF78], depthIndex, (u32 *)getScratchAddr(0));
    }
    if (bounds->w >= 0 && bounds->h >= 0) {
        addDrawAreaPacket(bounds->x, bounds->y, bounds->w, bounds->h,
                         &D_800EBFF0[D_8013AF78], playerModelDrawAreaDepths[depthIndex]);
        addDrawAreaPacket(0, 0, 320, 240, &D_800EBFF0[D_8013AF78], 14);
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
            D_8013AF90->vx = 0;
            D_8013AF90->vy = 0;
            D_8013AF90->vz = 0;
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[0].coord, D_8013AF90, D_8013AF88);
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[0] = D_8013AF88->vx + currentPlayerAbsolute->unk62C[0].x;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[1] = D_8013AF88->vy + currentPlayerAbsolute->unk62C[0].y;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[2] = D_8013AF88->vz + currentPlayerAbsolute->unk62C[0].z;
        } else {
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[parent].coord, (SVECTOR *)&currentPlayerAbsolute->unk62C[i], &D_800EB22C);
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[0] = D_800EB22C.vx + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[0];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[1] = D_800EB22C.vy + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[1];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[2] = D_800EB22C.vz + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[2];
        }
        setCoordinateRotation((SVECTOR *)&currentPlayerAbsolute->unk5BC[i], &currentPlayerAbsolute->model.modelCoordinates[i]);
    }
}

static inline RegisteredTaskEntry *getSignedModelPreviewEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
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
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = modelSlot;
    D_8013AE8C->data1C = playerIndex;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskResource();

    currentPlayerAbsolute = &gRacePlayers[D_8013AE8C->data1C];
    state = (ModelPreviewState *)&D_800D9504[D_8013AE8C->resource14];
    currentPlayerAbsolute->characterId = characterId;
    {
        u32 (*sourceBuffers)[4096] = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
        u32 (*destinationBuffers)[4096] = DAT_PLAYER_MODEL_BUFFERS;

        destination = destinationBuffers[D_8013AE8C->data1C];
        source = sourceBuffers[D_8013AE8C->data1A];
    }
    for (; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[D_8013AE8C->data1C];
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
    D_8013AE8C->resource16 = allocateTaskResource();
    position = (Movement2D *)&D_800D9504[D_8013AE8C->resource16];
    initMovement2D(position, x << 12, y << 12);
    position->unk08 = z;

    if (index % 10 == 0) {
        if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
        entry = getSignedModelPreviewEntry(D_8013AE88->resource14, index);
    } else {
        if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
        entry = getSignedModelPreviewEntry(D_8013AE88->resource16, index);
    }
    entry->command = 0;
    entry->flags = 10;
    entry->task = D_8013AE8C;
}

void updatePlayerModelPreviewTask(void) {
    u8 padding[0x10];
    ModelPreviewState *state;
    RegisteredTaskEntry *entry;
    currentPlayerAbsolute = &gRacePlayers[D_8013AE88->data1C];
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (ModelPreviewState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    if ((u16)(D_8013AE88->data1E % 10) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(D_8013AE98->resource14, D_8013AE88->data1E);
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(D_8013AE98->resource16, D_8013AE88->data1E);
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
    sortPlayerModelWithDrawArea(D_8013AE88->data1C, 0, &D_801300B8[D_8013AE88->data1C]);
}
