#include "../../menu/panel_tasks_internal.h"
#include "psx/libetc.h"
#include "game/engine/linear_random.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/race/player/race_player.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

typedef struct PaletteBurstScratch {
    GsCOORDINATE2 coordinate;
    POLY_FT4 quad;
    VECTOR scale;
    SVECTOR rotation;
    long depth;
    long cue;
    long flags;
} PaletteBurstScratch;

extern u32 GsOUT_PACKET_P __attribute__((section(".data")));

SVECTOR gPaletteBurstVertices[4] __attribute__((section(".data.palette_burst_vertices"))) = {
    {-10, -10, 0, 0},
    { 10, -10, 0, 0},
    {-10,  10, 0, 0},
    { 10,  10, 0, 0},
};

u32 gPlayerPaletteTransitionPixels[8] __attribute__((section(".data.player_palette_transition_pixels"))) = {0};

void copyPlayerPaletteRowWithTransition(s16 playerIndex, s32 unused, s32 paletteRow, u16 mode) {
    RECT rect;
    PlayerTextureOrigin *origin = &D_801204D4[playerIndex];
    if (mode == 0) {
        rect.x = origin->x + 0x210;
        rect.y = origin->y + (s16)(paletteRow + 0x70);
        rect.w = 16;
        rect.h = 1;
        MoveImage(&rect, origin->x + 0x200, origin->y + 0x70);
    } else {
        mode++;
        rect.x = origin->x + 0x210;
        rect.y = origin->y + (s16)(paletteRow + 0x70);
        rect.w = 16;
        rect.h = 1;
        StoreImage(&rect, gPlayerPaletteTransitionPixels);
        startPaletteTransition(origin->x + 0x200, origin->y + 0x70, gPlayerPaletteTransitionPixels, mode);
    }
}

void copyPlayerPaletteRow(s16 playerIndex) {
    copyPlayerPaletteRowWithTransition(playerIndex, gRacePlayers[playerIndex].unk009,
                  gRacePlayers[playerIndex].unk00D, 0);
}

void drawPaletteBurstTask(void) {
    PaletteBurstScratch *scratch;
    PaletteBurstResource *resource;
    POLY_FT4 *packet;
    MATRIX *matrix;
    RacePlayer *player;
    s32 i;
    s32 scale;
    s32 u;
    s32 brightness;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }

    player = &gRacePlayers[3];
    scratch = (PaletteBurstScratch *)getScratchAddr(0);
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    resource = (PaletteBurstResource *)&D_800D9504[D_8013AE88->resource14];
    /* Start with a nine-word packet and a cleared DMA link. */
    scratch->quad.tag = 9u << 24;
    setcode(&scratch->quad, POLY_FT4_CODE | 2);
    scratch->quad.tpage = getTPage(0, 1, 896, 0);
    scratch->quad.clut = getClut(832, 248);
    GsInitCoordinate2(player->model.modelCoordinates, &scratch->coordinate);
    scratch->coordinate.coord.t[0] = resource->x;
    scratch->coordinate.coord.t[1] = resource->y - resource->age * 20 / 8;
    scratch->coordinate.coord.t[2] = -50 * resource->age / 8;
    GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
    for (i = 0; i < 3; i++) {
        scratch->coordinate.coord.t[i] = scratch->coordinate.workm.t[i];
    }

    matrix = &scratch->coordinate.coord;
    setVector(&scratch->rotation, 0, 0, resource->rotation);
    RotMatrix(&scratch->rotation, matrix);
    scale = (resource->age << 10) + 1600;
    scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = scale;
    ScaleMatrix(matrix, &scratch->scale);
    GsSetLsMatrix(matrix);
    scratch->depth = RotTransPers4(&gPaletteBurstVertices[0], &gPaletteBurstVertices[1],
        &gPaletteBurstVertices[2], &gPaletteBurstVertices[3], (long *)&scratch->quad.x0,
        (long *)&scratch->quad.x1, (long *)&scratch->quad.x2,
        (long *)&scratch->quad.x3, &scratch->cue, &scratch->flags);

    u = (resource->age * 5 / 8) * 40;
    scratch->quad.u0 = scratch->quad.u2 = u;
    scratch->quad.u1 = scratch->quad.u3 = u + 39;
    scratch->quad.v0 = scratch->quad.v1 = 160;
    scratch->quad.v2 = scratch->quad.v3 = 191;
    brightness = 128 - resource->age * 16;
    setRGB0(&scratch->quad, resource->r * brightness / 128,
        resource->g * brightness / 128, resource->b * brightness / 128);

    *packet = scratch->quad;
    packet->tpage = getTPage(0, 1, 896, 0);
    packet->clut = getClut(832, 248);
    addPrim(D_800EBFF0[D_8013AF78].org, packet);
    packet++;

    scratch->quad.r0 = scratch->quad.g0 = scratch->quad.b0 = brightness;
    *packet = scratch->quad;
    packet->tpage = getTPage(0, 2, 896, 0);
    packet->clut = getClut(832, 249);
    addPrim(D_800EBFF0[D_8013AF78].org, packet);
    packet++;
    GsOUT_PACKET_P = (u32)packet;

    if (++resource->age >= 9) {
        deleteCurrentTaskSubtree();
    }
}

void spawnPaletteBurstTask(u8 paletteIndex) {
    u16 pixels[16];
    RECT rect;
    PaletteBurstResource *resource;
    u8 row;
    func_80014930(&gGameTasks[4], drawPaletteBurstTask);
    row = paletteIndex;
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PaletteBurstResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = randomNextLinear() % 4 - 2;
    resource->y = randomNextLinear() % 4 - 17;
    resource->z = randomNextLinear() % 4 - 2;
    resource->rotation = randomNextLinear() % 4096;
    resource->age = 0;
    if (row >= 8) row = paletteIndex + 1;
    rect.x = 1008;
    rect.y = row + 240;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, (u32 *)pixels);
    DrawSync(0);
    resource->b = (pixels[1] >> 7) & 0xF8;
    resource->g = (pixels[1] >> 2) & 0xF8;
    resource->r = pixels[1] << 3;
}
