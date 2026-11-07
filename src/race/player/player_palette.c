#include "../../menu/panel_tasks_internal.h"

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
    copyPlayerPaletteRowWithTransition(playerIndex, D_80120E90[playerIndex].unk009,
                  D_80120E90[playerIndex].unk00D, 0);
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_palette", func_8009B700);

void spawnPaletteBurstTask(u8 paletteIndex) {
    u16 pixels[16];
    RECT rect;
    PaletteBurstResource *resource;
    u8 row;
    func_80014930(&gGameTasks[4], func_8009B700);
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
