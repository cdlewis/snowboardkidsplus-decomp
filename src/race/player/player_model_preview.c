#include "../../menu/panel_tasks_internal.h"

u16 playerModelDrawAreaDepths[4] __attribute__((section(".data.player_model_draw_area_depths"))) = {
    0x70, 0x40, 0x20, 0
};

void sortPlayerModelWithDrawArea(u16 playerIndex, u16 depthIndex, RECT *bounds) {
    MATRIX matrix;
    RacePlayer *player = &gRacePlayers[playerIndex];
    GsDOBJ2 *object = player->modelObjects;
    s16 i;
    s16 count;
    D_8013B0D8 = player;
    count = player->modelPartCount;
    for (i = 0; i < count; i++, object++) {
        GsGetLw(object->coord2, &matrix);
        GsSetLightMatrix(&matrix);
        GsGetLs(object->coord2, &matrix);
        GsSetLsMatrix(&matrix);
        GsSortObject4(object, &D_800EBFF0[D_8013AF78], depthIndex, (u32 *)0x1F800000);
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
    player = D_8013B0D8;
    for (i = 0; i < player->modelPartCount; i++) {
        player->unk5BC[i].x = *values++;
        player->unk5BC[i].y = *values++;
        player->unk5BC[i].z = *values++;
    }
    {
        RacePlayer *translationPlayer = D_8013B0D8;
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
    RacePlayer *player = D_8013B0D8;
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
    for (i = 0; i < D_8013B0D8->modelPartCount; i++) {
        parent = D_8013B0D8->unk13E[i];
        RotMatrix((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i].coord);
        if (i == 0) {
            D_8013AF90->vx = 0;
            D_8013AF90->vy = 0;
            D_8013AF90->vz = 0;
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[0].coord, D_8013AF90, D_8013AF88);
            D_8013B0D8->model.modelCoordinates[0].coord.t[0] = D_8013AF88->vx + D_8013B0D8->unk62C[0].x;
            D_8013B0D8->model.modelCoordinates[0].coord.t[1] = D_8013AF88->vy + D_8013B0D8->unk62C[0].y;
            D_8013B0D8->model.modelCoordinates[0].coord.t[2] = D_8013AF88->vz + D_8013B0D8->unk62C[0].z;
        } else {
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[parent].coord, (SVECTOR *)&D_8013B0D8->unk62C[i], &D_800EB22C);
            D_8013B0D8->model.modelCoordinates[i].coord.t[0] = D_800EB22C.vx + D_8013B0D8->model.modelCoordinates[parent].coord.t[0];
            D_8013B0D8->model.modelCoordinates[i].coord.t[1] = D_800EB22C.vy + D_8013B0D8->model.modelCoordinates[parent].coord.t[1];
            D_8013B0D8->model.modelCoordinates[i].coord.t[2] = D_800EB22C.vz + D_8013B0D8->model.modelCoordinates[parent].coord.t[2];
        }
        func_8001E3E4((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i]);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_model_preview", func_80097858);

static inline RegisteredTaskEntry *getModelPreviewEntry(u32 resource, u32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&D_800D9504[resource];
    return &entries[(u16)(index / 10)];
}

void updatePlayerModelPreviewTask(void) {
    u8 padding[0x10];
    ModelPreviewState *state;
    RegisteredTaskEntry *entry;
    D_8013B0D8 = &gRacePlayers[D_8013AE88->data1C];
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
        func_800149A8();
        return;
    }
    D_8013B0D8->unk0B6 = (D_8013B0D8->unk0B6 + state->rotationStep) & 0xFFF;
    func_8001E3E4((SVECTOR *)&D_8013B0D8->unk0B4, &D_8013B0D8->worldCoordinate);
    func_8001E3E4(&D_8013B0D8->modelRotation, &D_8013B0D8->modelCoordinate);
    func_8001E3E4((SVECTOR *)D_8013B0D8->unk5BC, D_8013B0D8->model.modelCoordinates);
    sortPlayerModelWithDrawArea(D_8013AE88->data1C, 0, &D_801300B8[D_8013AE88->data1C]);
}
