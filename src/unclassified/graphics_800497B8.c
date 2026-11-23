#include "../graphics/render_effects_internal.h"
#include "../race/race_internal.h"
#include "psx/stdarg.h"
#include "game/audio/music.h"
#include "game/engine/random.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/controller_effects.h"
#include "game/race/player/race_player.h"
#include "psx/libgs.h"
#include "psx/sprite.h"

#define HEADING_RATIO_SCALE 640

#define DRAW_DEBUG_HEX_DIGITS(sprite, value, nibble, digit)                 \
    do {                                                                    \
        (digit) = ((value) >> ((nibble) * 4)) & 0xF;                        \
        if ((digit) < 10) {                                                 \
            (digit) += '0';                                                 \
        } else {                                                            \
            (digit) += 'A' - 10;                                            \
        }                                                                   \
        (sprite)->u = ((digit) & 0x1F) * 8;                                 \
        (sprite)->v = (((digit) & 0xE0) >> 3) & ~7;                         \
        GsSortFastSprite((sprite), &D_800EBFF0[D_8013AF78], 0);             \
        (sprite)->x += 8;                                                   \
    } while (--(nibble) >= 0)

typedef struct SlidingVisualStep {
    u8 duration;
    u8 event;
    u8 frame;
    u8 upperFrame;
} SlidingVisualStep;

extern u16 D_8013AF78 __attribute__((section(".data")));
extern u8 D_8013AFD3 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));

ControllerEffectState gControllerEffectStates[2] = {0};

u16 gTextClut = 0;

char gDebugPositionXFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vx:%8d";

char gDebugPositionYFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vy:%8d";

char gDebugPositionZFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vz:%8d";

CourseTriggerRecord gEmptyCourseTriggers[]
    __attribute__((section(".data.empty_course_triggers"), aligned(4))) = {
    {-1, 7, 0},
};

const char gNoProcessMessage[16]
    __attribute__((section(".rodata.no_process_message"))) = "NO PROCESS...";

static TaskVisualFrame sSlidingVisualFrames[]
    __attribute__((section(".data.sliding_visual_frames"), aligned(4))) = {
    {0x00, 0x00, 32, 32, 0xC, 0x3C38},
    {0x20, 0x00, 32, 32, 0xC, 0x3C38},
    {0x40, 0x00, 32, 32, 0xC, 0x3C38},
    {0x60, 0x00, 32, 32, 0xC, 0x3C38},
    {0x80, 0x00, 32, 32, 0xC, 0x3C38},
    {0xA0, 0x00, 32, 32, 0xC, 0x3C38},
    {0xC0, 0x00, 32, 32, 0xC, 0x3C38},
    {0xE0, 0x00, 32, 32, 0xC, 0x3C38},
    {0x00, 0x20, 32, 32, 0xC, 0x3C38},
    {0x20, 0x20, 32, 32, 0xC, 0x3C38},
    {0x40, 0x20, 32, 32, 0xC, 0x3C38},
    {0x60, 0x20, 32, 32, 0xC, 0x3C38},
    {0x80, 0x20, 32, 32, 0xC, 0x3C38},
    {0xA0, 0x20, 32, 32, 0xC, 0x3C38},
    {0xC0, 0x20, 32, 32, 0xC, 0x3C38},
    {0xE0, 0x20, 32, 32, 0xC, 0x3C38},
    {0x00, 0x40, 32, 32, 0xC, 0x3C38},
    {0x20, 0x40, 32, 32, 0xC, 0x3C38},
    {0x40, 0x40, 32, 32, 0xC, 0x3C38},
    {0x60, 0x40, 32, 32, 0xC, 0x3C38},
    {0x80, 0x40, 32, 32, 0xC, 0x3C38},
    {0xA0, 0x40, 32, 32, 0xC, 0x3C38},
    {0xC0, 0x40, 32, 32, 0xC, 0x3C38},
    {0xE0, 0x40, 32, 32, 0xC, 0x3C38},
    {0x00, 0x60, 32, 32, 0xC, 0x3C38},
    {0x20, 0x60, 32, 32, 0xC, 0x3C38},
    {0x40, 0x60, 32, 32, 0xC, 0x3C38},
    {0x60, 0x60, 32, 32, 0xC, 0x3C38},
    {0x80, 0x60, 32, 32, 0xC, 0x3C38},
    {0xA0, 0x60, 32, 32, 0xC, 0x3C38},
    {0xC0, 0x60, 32, 32, 0xC, 0x3C38},
    {0xE0, 0x60, 32, 32, 0xC, 0x3C38},
};

static SlidingVisualStep sSlidingVisualSteps[]
    __attribute__((section(".data.sliding_visual_steps"), aligned(4))) = {
    {1, 0, 0, 0xFF},
    {1, 0, 1, 0xFF},
    {1, 1, 2, 0xFF},
    {1, 0, 3, 0xFF},
    {3, 0, 4, 0xFF},
    {3, 0, 5, 0xFF},
    {3, 4, 6, 15},
    {3, 4, 7, 16},
    {3, 4, 8, 17},
    {3, 4, 9, 18},
    {3, 4, 8, 19},
    {3, 4, 10, 20},
    {3, 4, 6, 21},
    {3, 4, 5, 22},
    {3, 4, 6, 23},
    {3, 4, 7, 24},
    {3, 4, 8, 25},
    {3, 4, 9, 26},
    {3, 4, 8, 27},
    {3, 4, 10, 28},
    {3, 4, 6, 29},
    {3, 4, 5, 30},
    {3, 4, 6, 31},
    {3, 0, 7, 0xFF},
    {3, 255, 8, 0xFF},
    {3, 8, 9, 0xFF},
    {3, 8, 11, 0xFF},
    {3, 8, 12, 0xFF},
    {3, 8, 11, 0xFF},
    {3, 8, 12, 0xFF},
    {3, 8, 11, 0xFF},
    {3, 8, 12, 0xFF},
    {3, 255, 13, 0xFF},
    {1, 2, 14, 0xFF},
};

/* First-octant angles indexed by the scaled ratio of the smaller delta to the larger. */
static s16 sHeadingOctantAngles[]
    __attribute__((section(".data.heading_octant_angles"), aligned(4))) = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 69, 70, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
    85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
    132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 152, 153, 154,
    155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
    167, 168, 169, 170, 171, 172, 173, 173, 174, 175, 176, 177,
    178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
    189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
    201, 202, 203, 203, 204, 205, 206, 207, 208, 209, 210, 211,
    212, 213, 214, 214, 215, 216, 217, 218, 219, 220, 221, 222,
    223, 224, 224, 225, 226, 227, 228, 229, 230, 231, 232, 232,
    233, 234, 235, 236, 237, 238, 239, 240, 240, 241, 242, 243,
    244, 245, 246, 247, 248, 248, 249, 250, 251, 252, 253, 254,
    255, 255, 256, 257, 258, 259, 260, 261, 261, 262, 263, 264,
    265, 266, 267, 267, 268, 269, 270, 271, 272, 273, 273, 274,
    275, 276, 277, 278, 279, 279, 280, 281, 282, 283, 284, 284,
    285, 286, 287, 288, 289, 289, 290, 291, 292, 293, 294, 294,
    295, 296, 297, 298, 298, 299, 300, 301, 302, 303, 303, 304,
    305, 306, 307, 307, 308, 309, 310, 311, 311, 312, 313, 314,
    315, 315, 316, 317, 318, 319, 319, 320, 321, 322, 323, 323,
    324, 325, 326, 327, 327, 328, 329, 330, 330, 331, 332, 333,
    334, 334, 335, 336, 337, 337, 338, 339, 340, 340, 341, 342,
    343, 343, 344, 345, 346, 347, 347, 348, 349, 350, 350, 351,
    352, 353, 353, 354, 355, 356, 356, 357, 358, 358, 359, 360,
    361, 361, 362, 363, 364, 364, 365, 366, 367, 367, 368, 369,
    369, 370, 371, 372, 372, 373, 374, 375, 375, 376, 377, 377,
    378, 379, 380, 380, 381, 382, 382, 383, 384, 384, 385, 386,
    387, 387, 388, 389, 389, 390, 391, 391, 392, 393, 394, 394,
    395, 396, 396, 397, 398, 398, 399, 400, 400, 401, 402, 402,
    403, 404, 404, 405, 406, 406, 407, 408, 408, 409, 410, 410,
    411, 412, 412, 413, 414, 414, 415, 416, 416, 417, 418, 418,
    419, 420, 420, 421, 422, 422, 423, 424, 424, 425, 425, 426,
    427, 427, 428, 429, 429, 430, 431, 431, 432, 432, 433, 434,
    434, 435, 436, 436, 437, 437, 438, 439, 439, 440, 441, 441,
    442, 442, 443, 444, 444, 445, 446, 446, 447, 447, 448, 449,
    449, 450, 450, 451, 452, 452, 453, 453, 454, 455, 455, 456,
    456, 457, 458, 458, 459, 459, 460, 461, 461, 462, 462, 463,
    463, 464, 465, 465, 466, 466, 467, 468, 468, 469, 469, 470,
    470, 471, 472, 472, 473, 473, 474, 474, 475, 476, 476, 477,
    477, 478, 478, 479, 479, 480, 481, 481, 482, 482, 483, 483,
    484, 484, 485, 486, 486, 487, 487, 488, 488, 489, 489, 490,
    490, 491, 492, 492, 493, 493, 494, 494, 495, 495, 496, 496,
    497, 497, 498, 499, 499, 500, 500, 501, 501, 502, 502, 503,
    503, 504, 504, 505, 505, 506, 506, 507, 507, 508, 508, 509,
    509, 510, 510, 511, 511, 511, 511, 511, 511, 511, 511, 511,
};

GsSPRITE gDebugTextSprite __attribute__((section(".data.debug_text_sprite"), aligned(4))) = {0};

void updateSlidingVisualSequenceTask(void) {
    SlidingVisualResource *resource;
    u16 playerIndex;
    RacePlayer *player;
    s32 amount;
    s32 counter;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingVisualResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        switch (D_8013AE88->data1C) {
        case 0:
        case 2:
            resource->offsetY += resource->velocity;
            resource->velocity += 2;
            if (resource->offsetY >= -111) {
                resource->offsetY = -112;
                resource->timer = sSlidingVisualSteps[++resource->step].duration;
                D_8013AE88->data1C++;
            }
            break;
        case 1:
            if (--resource->timer == 0) {
                resource->timer = sSlidingVisualSteps[++resource->step].duration;
                if (sSlidingVisualSteps[resource->step].event == 1) {
                    resource->velocity = -10;
                    D_8013AE88->data1C++;
                }
            }
            break;
        case 3:
            if (--resource->timer == 0) {
                resource->timer = sSlidingVisualSteps[++resource->step].duration;
                if (sSlidingVisualSteps[resource->step].event == 4) {
                    playOwnedSoundEffect(D_8013AE88->data18, 1, 0xE01, 0x40);
                    D_8013AE88->data1C++;
                }
            }
            break;
        case 4:
            if (--resource->timer == 0) {
                resource->timer = sSlidingVisualSteps[++resource->step].duration;
                for (playerIndex = 0; playerIndex < D_8013AFD2; playerIndex++) {
                    player = &gRacePlayers[playerIndex];
                    if (player->isActive && playerIndex != D_8013AE88->data18) {
                        counter = player->unk01C;
                        if (counter >= 166) {
                            amount = 166;
                        } else {
                            amount = counter;
                        }
                        player->unk01C = counter - amount;
                        addPlayerCappedCounters(D_8013AE88->data18, amount);
                    }
                }
                if (sSlidingVisualSteps[resource->step].event == 0) {
                    stopMatchingSoundVoice(D_8013AE88->data18, 1, 0xE01);
                }
                if (sSlidingVisualSteps[resource->step].event == 8) {
                    playSoundEffect(1, 0xF01, 0x20);
                }
                if (sSlidingVisualSteps[resource->step].event == 2) {
                    playSoundEffect(1, 2, 0x20);
                    resource->velocity = 0;
                    D_8013AE88->data1C++;
                }
            }
            break;
        case 5:
            resource->offsetY += resource->velocity;
            resource->velocity -= 2;
            if (resource->offsetY < -320) {
                deleteCurrentTaskSubtree();
                return;
            }
            break;
        }
    }
    resource->coordinate.coord.t[0] = gRacePlayers[D_8013AE88->data18].worldCoordinate.coord.t[0];
    resource->coordinate.coord.t[1] = gRacePlayers[D_8013AE88->data18].worldCoordinate.coord.t[1] + resource->offsetY;
    resource->coordinate.coord.t[2] = gRacePlayers[D_8013AE88->data18].worldCoordinate.coord.t[2];
    resource->framePosition = sSlidingVisualSteps[resource->step].frame << 8;
    setTaskVisualFrame((TaskVisualResource *)resource, sSlidingVisualFrames, 0x1C00);
    drawCameraFacingTaskVisual((TaskVisualResource *)resource, 1);
    if (sSlidingVisualSteps[resource->step].upperFrame != 0xFF) {
        resource->coordinate.coord.t[1] -= 56;
        resource->framePosition = sSlidingVisualSteps[resource->step].upperFrame << 8;
        setTaskVisualFrame((TaskVisualResource *)resource, sSlidingVisualFrames, 0x1C00);
        drawCameraFacingTaskVisual((TaskVisualResource *)resource, 0);
    }
}

void initDebugTextSprite(void) {
    GsSPRITE *sprite = &gDebugTextSprite;

    initSprite(sprite);
    setWH(sprite, 8, 8);
    sprite->tpage = getTPage(0, 0, 960, 256);
    sprite->cx = 768;
    sprite->cy = 240;
    gTextClut = 240;
}

void setTextPalette(s32 palette) {
    gTextClut = (u8)palette + 0xF0;
}

/* %0 through %7 select a palette. %c, %s, and %l draw 2, 4, and 8 hex digits. */
void drawDebugText(s32 x, s32 y, const char *format, ...) {
    GsSPRITE *sprite;
    va_list args;
    s32 nibble;
    s32 value;
    s32 digit;

    gDebugTextSprite.x = x;
    gDebugTextSprite.y = y;
    gDebugTextSprite.attribute |= GsALON;
    if (!*(const s8 *)format) {
        return;
    }
    va_start(args, format);
    sprite = &gDebugTextSprite;

    do {
        nibble = 0;
        if (*(const s8 *)format == '%') {
            if (((const s8 *)format)[1] >= '0') {
                if (((const s8 *)format)[1] <= '7') {
                    gTextClut = (((const s8 *)format)[1] & 7) + 0xF0;
                    format += 2;
                }
            }
            if (*(const s8 *)format == '%') {
                if (((const s8 *)format)[1] == 'c') {
                    nibble = 1;
                }
                if (((const s8 *)format)[1] == 's') {
                    nibble = 2;
                }
                if (((const s8 *)format)[1] == 'l') {
                    nibble = 3;
                }
            }
        }

        sprite->cy = gTextClut;
        switch (nibble) {
            case 0: {
                const s8 *cursor = (const s8 *)format;
                sprite->u = ((u8)*cursor & 0x1F) * 8;
                sprite->v = ((*cursor & 0xE0) >> 3) & ~7;
                GsSortFastSprite(sprite, &D_800EBFF0[D_8013AF78], 0);
                sprite->x += 8;
                format++;
                break;
            }
            case 1:
                value = va_arg(args, s32);
                DRAW_DEBUG_HEX_DIGITS(sprite, value, nibble, digit);
                format += 2;
                break;
            case 2:
                value = va_arg(args, s32);
                nibble = 3;
                DRAW_DEBUG_HEX_DIGITS(sprite, value, nibble, digit);
                format += 2;
                break;
            case 3:
                value = va_arg(args, s32);
                nibble = 7;
                DRAW_DEBUG_HEX_DIGITS(sprite, value, nibble, digit);
                format += 2;
                break;
        }
    } while (*(const s8 *)format);
    va_end(args);
}

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

s16 getXZHeading(s32 x1, s32 z1, s32 x2, s32 z2) {
    s16 angle;
    s32 dx = x2 - x1;
    s32 dz = z2 - z1;

    if (dx == 0 && dz == 0) {
        return 0;
    }

    if (dx >= 0) {
        if (dz >= 0) {
            if (dx < dz) {
                return sHeadingOctantAngles[(s16)(dx * HEADING_RATIO_SCALE / dz)];
            }
            angle = 0x400 - sHeadingOctantAngles[(s16)(dz * HEADING_RATIO_SCALE / dx)];
            goto done;
        }

        dz = -dz;
        if (dz < dx) {
            angle = 0x400 + sHeadingOctantAngles[(s16)(dz * HEADING_RATIO_SCALE / dx)];
            goto done;
        }
        angle = 0x800 - sHeadingOctantAngles[(s16)(dx * HEADING_RATIO_SCALE / dz)];
        goto done;
    }

    if (dz < 0) {
        dx = -dx;
        dz = -dz;
        if (dx < dz) {
            angle = 0x800 + sHeadingOctantAngles[(s16)(dx * HEADING_RATIO_SCALE / dz)];
            goto done;
        }
        angle = 0xC00 - sHeadingOctantAngles[(s16)(dz * HEADING_RATIO_SCALE / dx)];
        goto done;
    }

    dx = -dx;
    if (dz < dx) {
        angle = 0xC00 + sHeadingOctantAngles[(s16)(dz * HEADING_RATIO_SCALE / dx)];
        goto done;
    }
    angle = 0x1000 - sHeadingOctantAngles[(s16)(dx * HEADING_RATIO_SCALE / dz)];

done:
    return angle;
}
