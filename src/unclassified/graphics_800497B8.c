#include "../graphics/render_effects_internal.h"

ControllerEffectState gControllerEffectStates[2] = {0};

u16 gTextClut = 0;

char gDebugPositionXFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vx:%8d";

char gDebugPositionYFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vy:%8d";

char gDebugPositionZFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vz:%8d";

CourseTriggerRecord gEmptyCourseTriggers[]
    __attribute__((section(".data.empty_course_triggers"), aligned(4))) = {
    {-1, 7, 0},
};

const char gNoProcessMessage[16] = "NO PROCESS...";

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_800497B8", func_800497B8);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_800497B8", func_80049CDC);

void setTextPalette(s32 palette) {
    gTextClut = (u8)palette + 0xF0;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_800497B8", func_80049D48);

void initSprite(GsSPRITE *sprite) {
    sprite->r = 0x80;
    sprite->g = 0x80;
    sprite->b = 0x80;
    sprite->attribute = 0;
    sprite->x = 0;
    sprite->y = 0;
    sprite->w = 0;
    sprite->h = 0;
    sprite->tpage = 0;
    sprite->u = 0;
    sprite->v = 0;
    sprite->cx = 0;
    sprite->cy = 0;
    sprite->mx = 0;
    sprite->my = 0;
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
    sprite->rotate = 0;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_800497B8", func_8004A124);
