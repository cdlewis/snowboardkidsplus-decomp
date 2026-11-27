#include "psx/libetc.h"
#include "render_effects_internal.h"
#include "../race/race_internal.h"
#include "game/engine/game_loop.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/dat_resources.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/menu/selected_records.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_geometry.h"
#include "game/race/player/race_player.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

typedef struct OscillatingQuadScratchpad {
    GsCOORDINATE2 coordinate;
    MATRIX rotationMatrix;
    SVECTOR vertices[4];
    long depth, cue, flags;
} OscillatingQuadScratchpad;

typedef struct AnimatedBackgroundScratchpad {
    GsCOORDINATE2 root;
    GsCOORDINATE2 child;
    MATRIX localScreen;
    SVECTOR rotation;
} AnimatedBackgroundScratchpad;

extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u32 *gCurrentPlayerOrderingTable __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern RECT gWritePixelRect __attribute__((section(".sdata.write_pixel_rect")));

void drawLayeredBackgroundAndSetFog(s16 playerIndex);
void drawSunsetBackgroundAndSetFog(s16 playerIndex);
void drawNightHighwayBackgroundAndSetFog(s16 playerIndex);
void drawRotatingModelBackgroundAndSetFog(s16 playerIndex);
void drawRipplingBackgroundAndSetFog(s16 playerIndex);

u32 gSelectedCameraOverlayMode __attribute__((section(".data"))) = 0;
u32 D_8013B210 __attribute__((section(".data"))) = 1;

static VECTOR sBackgroundModelScale __attribute__((section(".data.background_model_scale"))) = {
    5 * 4096, 5 * 4096, 5 * 4096, 0,
};

static POLY_G4 sSunsetBackgroundQuad __attribute__((section(".data.sunset_background_quad"))) = {
    8 << 24,
    111, 127, 255, POLY_G4_CODE, -160, -240,
    111, 127, 255, 0,             159, -240,
     31, 127, 175, 0,            -160,    0,
     31, 127, 175, 0,             159,    0,
};

/* Unreferenced background packet templates retained in their original order. */
POLY_G4 gUnusedBackgroundQuads[6] __attribute__((section(".data.unused_background_packets"))) = {
    {8 << 24,
     111, 127, 255, POLY_G4_CODE, -160, -240,
     111, 127, 255, 0, 159, -240,
     31, 127, 175, 0, -160, 0,
     31, 127, 175, 0, 159, 0,
    },
    {8 << 24,
     64, 0, 127, POLY_G4_CODE, -160, -240,
     128, 48, 0, 0, 159, -240,
     255, 255, 0, 0, -160, 0,
     255, 255, 112, 0, 159, 0,
    },
    {8 << 24,
     0, 0, 15, POLY_G4_CODE, -160, -240,
     0, 0, 15, 0, 159, -240,
     0, 0, 127, 0, -160, 0,
     0, 0, 127, 0, 159, 0,
    },
    {8 << 24,
     31, 127, 175, POLY_G4_CODE, -160, 0,
     31, 127, 175, 0, 159, 0,
     255, 255, 255, 0, -160, 239,
     255, 255, 255, 0, 159, 239,
    },
    {8 << 24,
     255, 255, 0, POLY_G4_CODE, -160, 0,
     255, 255, 112, 0, 159, 0,
     255, 255, 255, 0, -160, 239,
     255, 255, 255, 0, 159, 239,
    },
    {8 << 24,
     15, 15, 127, POLY_G4_CODE, -160, 0,
     15, 15, 127, 0, 159, 0,
     15, 63, 255, 0, -160, 239,
     15, 63, 255, 0, 159, 239,
    },
};

FlatRectanglePacket gUnusedBackgroundRectangles[3] __attribute__((section(".data.unused_background_packets"))) = {
    {4 << 24, getTPage(0, 1, 0, 0), DRAW_MODE_COMMAND_HIGH,
     0, 0, 0, FLAT_RECTANGLE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 320, 240},
    {4 << 24, getTPage(0, 2, 0, 0), DRAW_MODE_COMMAND_HIGH,
     0, 32, 64, FLAT_RECTANGLE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 320, 240},
    {4 << 24, getTPage(0, 2, 0, 0), DRAW_MODE_COMMAND_HIGH,
     64, 32, 16, FLAT_RECTANGLE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 320, 240},
};

TILE gLapAnimatedBackgroundTile __attribute__((section(".data.lap_animated_background_tile"))) = {
    3 << 24, 224, 224, 232, TILE_CODE, -160, -120, 320, 240,
};

static u32 sRotatingBackgroundFrame __attribute__((section(".sdata.rotating_background_frame"))) = 0;

u32 gLapAnimatedBackgroundFrame __attribute__((section(".sdata.lap_animated_background_frame"))) = 0;

VECTOR gLayeredBackgroundModelPosition1 __attribute__((section(".data.layered_background_model_positions"))) = {
    -0xC2E8, 0x7147, 0x2D37, 0,
};

VECTOR gLayeredBackgroundModelPosition2 __attribute__((section(".data.layered_background_model_positions"))) = {
    -0xF499, 0x14D45, 0xCAD5, 0,
};

SVECTOR gOscillatingQuadVertices[4] __attribute__((section(".data.oscillating_quad_vertices"))) = {
    {-512, -4096, 0, 0},
    {512, -4096, 0, 0},
    {-128, 0, 0, 0},
    {128, 0, 0, 0},
};

/* Twelve animated offsets followed by the four words used by the next background. */
s16 gRipplingBackgroundStripData[20] __attribute__((section(".data.rippling_background"), aligned(4))) = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x5000, 0, 0x5000, 0, 0x5000, 0, 0, 0,
};

static void (*sCourseBackgroundCallbacks[10])(s16 playerIndex)
    __attribute__((section(".data.course_background_callbacks"))) = {
    drawLayeredBackgroundAndSetFog,
    drawSunsetBackgroundAndSetFog,
    drawNightHighwayBackgroundAndSetFog,
    drawCyanBackgroundAndSetFog,
    drawRotatingModelBackgroundAndSetFog,
    drawRipplingBackgroundAndSetFog,
    drawLapAnimatedBackgroundAndSetFog,
    drawSnowBackgroundAndSetFog,
    drawCameraCulledBackgroundAndSetFog,
    drawBackgroundModelsAndSetFog,
};

TexturedSpritePacket gCameraOverlaySpritesBuffer0[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, 96, -120, 0, 0, 0, 64, 240},
};

TexturedSpritePacket gCameraOverlaySpritesBuffer1[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, 96, -120, 0, 0, 0, 64, 240},
};

POLY_FT4 gCameraOverlayQuadsBuffer0[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, 101, -120, 0, 8, 0, 159, -120, 55, 8, getTPage(2, 0, 256, 0), 101, 119, 0, 231, 0, 159, 119, 55, 231, 0},
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 8, 8, 0, 101, -120, 255, 8, getTPage(2, 0, 0, 0), -160, 119, 8, 231, 0, 101, 119, 255, 231, 0},
};

POLY_FT4 gCameraOverlayQuadsBuffer1[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, 101, -120, 0, 8, 0, 159, -120, 55, 8, getTPage(2, 0, 256, 256), 101, 119, 0, 231, 0, 159, 119, 55, 231, 0},
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 8, 8, 0, 101, -120, 255, 8, getTPage(2, 0, 0, 256), -160, 119, 8, 231, 0, 101, 119, 255, 231, 0},
};

TexturedSpritePacket gScreenCopyPlusBuffer1[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -159, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     97, -120, 0, 0, 0, 64, 240}
};

TexturedSpritePacket gScreenCopyPlusBuffer0[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -159, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     97, -120, 0, 0, 0, 64, 240}
};

TexturedSpritePacket gScreenCopyMinusBuffer1[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -161, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     95, -120, 0, 0, 0, 64, 240}
};

TexturedSpritePacket gScreenCopyMinusBuffer0[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -161, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     95, -120, 0, 0, 0, 64, 240}
};

/* Unreferenced zero-filled storage between the sprite packets and model coordinate. */
u8 gCourseRenderingReserved24FD4[0x20] __attribute__((aligned(4), section(".data.course_rendering_reserved_24fd4"))) = {0};

const VECTOR gCourseModelScale __attribute__((section(".rodata.course_model_scale"))) = {
    5 * 4096, 5 * 4096, 5 * 4096, 0,
};

RECT gCourseModelTextureWindow __attribute__((section(".data.course_model_texture_window"))) = {0};

DR_MODE gCourseModelDrawMode __attribute__((section(".bss.course_model_draw_mode")));

s32 gCourseModelCullDistances[10] __attribute__((section(".data.course_model_distances"))) = {
    7000, 7000, 6000, 7000, 7000, 6000, 8000, 7000, 6000, 7000,
};

s32 gCourseModelDetailDistances[10] __attribute__((section(".data.course_model_distances"))) = {
    6000, 6000, 6000, 6000, 6000, 6000, 6000, 3000, 6000, 6000,
};

GsCOORDINATE2 gCourseModelCoordinate __attribute__((section(".data.course_model_coordinate"))) = {0};

TmdObject *gOptionalCourseModels[7] __attribute__((section(".data.optional_course_models"))) = {0};

/* Unreferenced zero-filled storage between optional models and position buffers. */
u8 gCourseRenderingReserved25060[0x8C] __attribute__((aligned(4), section(".data.course_rendering_reserved_25060"))) = {0};

Position3D *gCourseModelPositionBuffers[10] __attribute__((section(".data.course_model_position_buffers"))) = {
    gCourseModelPositionSector.positions, gCourseModelPositionSector.positions,
    gCourseModelPositionSector.positions, gCourseModelPositionSector.positions,
    gCourseModelPositionSector.positions, gCourseModelPositionSector.positions,
    gCourseModelPositionSector.positions, gCourseModelPositionSector.positions,
    gCourseModelPositionSector.positions, gCourseModelPositionSector.positions,
};

Position3D *gCourseModelPositions __attribute__((section(".data.course_model_positions"))) = 0;

u16 gCourseModelCounts[2] __attribute__((section(".sdata.course_model_state"))) = {0};

u32 gCourseModelObjectCount __attribute__((section(".sdata.course_model_state"))) = 0;

u32 gOptionalCourseModelObjectCount __attribute__((section(".sdata.course_model_state"))) = 0;

s32 gCourseModelCullDistance __attribute__((section(".sdata.course_model_state"))) = 0;

s32 gCourseModelDetailDistance __attribute__((section(".sdata.course_model_state"))) = 0;

TmdObject *gCourseModels[100] __attribute__((section(".bss.course_models")));

s32 gCourseRenderingDisabled __attribute__((section(".data.course_rendering_disabled"))) = 0;
/* Unreferenced words following the rendering gate in the original data. */
static u32 sCourseRenderingDisabledPadding[3]
    __attribute__((section(".data.course_rendering_disabled"))) = {0};
s16 gBackgroundLensFlareYawOffset
    __attribute__((section(".sdata.background_lens_flare_state"))) = 0;
s16 gBackgroundLensFlarePitchOffset
    __attribute__((section(".sdata.background_lens_flare_state"))) = -128;

/* Unreferenced words between the lens-flare angles and rotating background frame. */
static u32 sBackgroundData_8013B230[2]
    __attribute__((section(".sdata.background_lens_flare_state"))) = {1, 0x8FC};


void getCourseSteeringTarget(s16 region, s32 x, s32 z, s32 *targetX, s32 *targetZ,
                             u16 tableIndex, u16 playerIndex) {
    s32 dx, dz, forward, cosine, lateral;
    s16 vertexX, vertexZ;
    s16 nextRegion;
    s32 vertexIndex;
    s16 *vertices;
    SelectedCourseRecord *course;
    SelectedCourseRecord *courses = gSelectedCourseRecords;

    if (region != (s16)courses[gSelectedCourseIndex].unk00) {
        gGeometryProjectionBasisX = rsin(-(s16)gCourseGeometryRegionData[region].angle);
        cosine = rcos(-(s16)gCourseGeometryRegionData[region].angle);
        vertexIndex = gCourseGeometryRegionData[region].vertexIndex;
        dx = x - gCourseGeometryVertexData[vertexIndex].x * 5;
        dz = z - gCourseGeometryVertexData[vertexIndex].z * 5;
        gGeometryProjectionBasisZ = cosine;
        dz = (-gGeometryProjectionBasisX * dx + cosine * dz) / 4096;

        course = &courses[gSelectedCourseIndex];
        if ((region >= course->objectFirstSegment && region <= course->objectLastSegment) ||
            (gSelectedCourseIndex == 3 && (u16)(region - 285) < 2)) {
            dz -= 960;
            *targetX = -gGeometryProjectionBasisX * dz / 4096;
            *targetZ = gGeometryProjectionBasisZ * dz / 4096;
        } else {
            dz += 960;
            *targetX = -gGeometryProjectionBasisX * dz / 4096;
            *targetZ = gGeometryProjectionBasisZ * dz / 4096;
        }

        *targetX += gCourseGeometryVertexData[gCourseGeometryRegionData[region].vertexIndex].x * 5;
        *targetZ += gCourseGeometryVertexData[gCourseGeometryRegionData[region].vertexIndex].z * 5;

        nextRegion = getAdjacentGeometryRegionForPoint(region, *targetX, *targetZ);
        gGeometryProjectionBasisX = rsin(-(s16)gCourseGeometryRegionData[nextRegion].angle);
        gGeometryProjectionBasisZ = rcos(-(s16)gCourseGeometryRegionData[nextRegion].angle);
        vertexIndex = gCourseGeometryRegionData[nextRegion].vertexIndex * 3;
        vertices = (s16 *)gCourseGeometryVertexData;
        vertexX = vertices[vertexIndex];
        vertexZ = vertices[vertexIndex + 2];
        dx = *targetX - vertexX * 5;
        dz = *targetZ - vertexZ * 5;
        forward = updatePlayerField94CFromSelectedTable(tableIndex, nextRegion, playerIndex);

        lateral = (-gGeometryProjectionBasisX * dx + gGeometryProjectionBasisZ * dz) / 4096;
        *targetX = (gGeometryProjectionBasisZ * forward + -gGeometryProjectionBasisX * lateral) / 4096;
        *targetZ = (gGeometryProjectionBasisX * forward + gGeometryProjectionBasisZ * lateral) / 4096;
        *targetX += gCourseGeometryVertexData[gCourseGeometryRegionData[nextRegion].vertexIndex].x * 5;
        *targetZ += gCourseGeometryVertexData[gCourseGeometryRegionData[nextRegion].vertexIndex].z * 5;
    } else {
        *targetX = (s32)courses[gSelectedCourseIndex].unk18[0] >> 8;
        *targetZ = (s32)courses[gSelectedCourseIndex].unk18[2] >> 8;
    }
}

void adjustDebugPositionAndDisplay(VECTOR *position) {
    if (gControllerHeldButtons[1] & 0x1000) position->vx += 32;
    if (gControllerHeldButtons[1] & 0x4000) position->vx -= 32;
    if (gControllerHeldButtons[1] & 0x8000) position->vz += 32;
    if (gControllerHeldButtons[1] & 0x2000) position->vz -= 32;
    if (gControllerHeldButtons[1] & 8) position->vy += 32;
    if (gControllerHeldButtons[1] & 2) position->vy -= 32;
    drawStretchedDebugText(0, 10, gDebugPositionXFormat, position->vx);
    drawStretchedDebugText(0, 11, gDebugPositionYFormat, position->vy);
    drawStretchedDebugText(0, 12, gDebugPositionZFormat, position->vz);
}

void drawBackgroundLensFlare(s16 playerIndex) {
    CVECTOR colors[2];
    u8 padding[0x18];
    s16 angle, leftX, leftY;
    s16 outerX, outerY, rightX, rightY;
    s16 red, blue;
    u16 green;
    s16 x, y, centerX, centerY;
    u16 yaw;
    s32 rayIndex;
    s32 cosine, radius;
    PlayerCamera *cameras;
    PlayerCamera *camera;

    cameras = gPlayerCameras;
    camera = &cameras[playerIndex];
    yaw = camera->yaw & 0xFFF;
    if (yaw > 0x400 && yaw < 0xC00) {
        return;
    }
    x = -320 * rsin(camera->yaw + gBackgroundLensFlareYawOffset) / 4096;
    if ((u16)((y = 240 * rsin(camera->pitch + gBackgroundLensFlarePitchOffset) / 4096) + 120) > 240) {
        return;
    }
    if ((u16)(x + 160) > 320) {
        return;
    }

    rayIndex = 0;
    /* The original initializes these packet codes even though it draws through helpers. */
    colors[0].cd = 6;
    colors[1].cd = 48;
    for (angle = yaw; angle < (s16)yaw + 0x1000; angle += 0x100, rayIndex++) {
        cosine = rcos(angle);
        radius = 64 + (rayIndex & 1) * 32;
        outerX = x + radius * cosine / 4096;
        outerY = y + radius * rsin(angle) / 4096;
        rightX = x + rcos(angle + 128) / 128;
        rightY = y + rsin(angle + 128) / 128;
        leftX = x + rcos(angle - 128) / 128;
        leftY = y + rsin(angle - 128) / 128;

        centerX = x;
        centerY = y;
        if ((s16)((s16)rayIndex % 3)) {
            red = 16;
            blue = green = 0;
        } else if (((s16)rayIndex + 1) % 3) {
            red = 0;
            green = 16;
            blue = 0;
        } else if (((s16)rayIndex + 2) % 3) {
            blue = 16;
            red = 0;
            green = 0;
        }

        addSemiTransparentGouraudTriangle(gCurrentPlayerOrderingTable, getTPage(0, 1, 0, 0),
            outerX, outerY, rightX, rightY, centerX, centerY,
            (u8)red, (u8)green, (u8)blue, 0, 0, 0, 64, 64, 64);
        addSemiTransparentGouraudTriangle(gCurrentPlayerOrderingTable, getTPage(0, 1, 0, 0),
            outerX, outerY, centerX, centerY, leftX, leftY,
            (u8)red, (u8)green, (u8)blue, 64, 64, 64, 0, 0, 0);
    }

    addTexturedQuad(gCurrentPlayerOrderingTable,
        (s16)(x - 32), (s16)(y - 32), (s16)(x + 32), (s16)(y - 32),
        (s16)(x - 32), (s16)(y + 32), (s16)(x + 32), (s16)(y + 32),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 241),
        0, 0, 63, 0, 0, 63, 63, 63, 96, 96, 96);
    addTexturedQuad(gCurrentPlayerOrderingTable,
        (s16)(x / 2 - 16), (s16)(y / 2 - 16), (s16)(x / 2 + 16), (s16)(y / 2 - 16),
        (s16)(x / 2 - 16), (s16)(y / 2 + 16), (s16)(x / 2 + 16), (s16)(y / 2 + 16),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 240),
        0, 0, 63, 0, 0, 63, 63, 63, 38, 24, 24);
    addTexturedQuad(gCurrentPlayerOrderingTable,
        -20, -20, 20, -20, -20, 20, 20, 20,
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 241),
        0, 0, 63, 0, 0, 63, 63, 63, 50, 68, 50);
    addTexturedQuad(gCurrentPlayerOrderingTable,
        (s16)(-x / 2 - 40), (s16)(-y / 2 - 40), (s16)(-x / 2 + 40), (s16)(-y / 2 - 40),
        (s16)(-x / 2 - 40), (s16)(-y / 2 + 40), (s16)(-x / 2 + 40), (s16)(-y / 2 + 40),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 242),
        0, 0, 63, 0, 0, 63, 63, 63, 10, 20, 10);
    addTexturedQuad(gCurrentPlayerOrderingTable,
        (s16)(-x - 20), (s16)(-y - 20), (s16)(-x + 20), (s16)(-y - 20),
        (s16)(-x - 20), (s16)(-y + 20), (s16)(-x + 20), (s16)(-y + 20),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 240),
        0, 0, 63, 0, 0, 63, 63, 63, 40, 20, 20);
}

void drawLayeredBackgroundAndSetFog(s16 playerIndex) {
    u8 padding[0x10];
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    PlayerCamera *cameras;
    PlayerCamera *camera;
    s16 x, y, horizon;

    if (playerIndex == 0) {
        ot = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2OrderingTables[gActiveGraphicsBuffer];
    }

    cameras = gPlayerCameras;
    camera = &cameras[playerIndex];
    if ((u16)camera->unk18 < 8) {
        gBackgroundLensFlareYawOffset = 0xF28;
        gBackgroundLensFlarePitchOffset = 0xF70;
        drawBackgroundLensFlare(playerIndex);
    }

    x = -160 - ((camera->yaw >> 1) & 255);
    y = camera->pitch / 2;
    horizon = y - 64;

    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], getTPage(0, 0, 640, 256), getClut(656, 481),
        x, (s16)(y - 64), 256, 48, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], getTPage(0, 0, 640, 256), getClut(656, 480),
        x, (s16)(y - 16), 256, 80, 0, 48, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], getTPage(0, 0, 640, 256), getClut(656, 481),
        (s16)(x + 256), (s16)(y - 64), 256, 48, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], getTPage(0, 0, 640, 256), getClut(656, 480),
        (s16)(x + 256), (s16)(y - 16), 256, 80, 0, 48, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], getTPage(0, 0, 640, 256), getClut(656, 481),
        (s16)(x + 512), (s16)(y - 64), 256, 48, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], getTPage(0, 0, 640, 256), getClut(656, 480),
        (s16)(x + 512), (s16)(y - 16), 256, 80, 0, 48, 128, 128, 128);

    addGouraudQuad(&gCurrentPlayerOrderingTable[1024],
        -160, (s16)(y - 184), 160, (s16)(y - 184), -160, (s16)(y - 64), 160, (s16)(y - 64),
        128, 255, 255, 128, 255, 255, 41, 123, 230, 0, 112, 230);
    addGouraudQuad(&gCurrentPlayerOrderingTable[1024],
        -160, (s16)(y + 64), 160, (s16)(y + 64), -160, (s16)(y + 184), 160, (s16)(y + 184),
        255, 255, 255, 255, 255, 255, 220, 220, 220, 220, 220, 220);

    if (horizon > 120) horizon = 120;
    if (horizon < -120) horizon = -120;
    addFlatRectangle(&gCurrentPlayerOrderingTable[1024], -160, -120, 320, (s16)(horizon + 120), 128, 255, 255);
    addFlatRectangle(&gCurrentPlayerOrderingTable[1024], -160, horizon, 320, (s16)(120 - horizon), 220, 220, 220);

    if (isPointWithinCameraForwardRange(-0xF02D, 0xC5B6, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, -0xEE39, 0x14DAC, 0xC7AA);
        RotMatrixY(512, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        ScaleMatrix(matrix, &sBackgroundModelScale);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(gOptionalCourseModels[0], ot, 2, 3);
    }
    if (isPointWithinCameraForwardRange(gLayeredBackgroundModelPosition1.vx, gLayeredBackgroundModelPosition1.vz, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, gLayeredBackgroundModelPosition1.vx, gLayeredBackgroundModelPosition1.vy, gLayeredBackgroundModelPosition1.vz);
        RotMatrixY(-1000, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 3);
    }
    if (isPointWithinCameraForwardRange(gLayeredBackgroundModelPosition2.vx, gLayeredBackgroundModelPosition2.vz, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, gLayeredBackgroundModelPosition2.vx, gLayeredBackgroundModelPosition2.vy, gLayeredBackgroundModelPosition2.vz);
        RotMatrixY(600, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 3);
    }

    SetFarColor(160, 160, 200);
    SetFogNearFar(8000, 10000, 400);
}

void drawSunsetBackgroundAndSetFog(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    PlayerCamera *cameras;
    PlayerCamera *camera;
    s16 horizon;
    u16 segment;
    s32 r, g, b;

    if (playerIndex == 0) {
        ot = &gPlayer1OrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2OrderingTables[gActiveGraphicsBuffer];
    }
    *(POLY_G4 *)getScratchAddr(0) = sSunsetBackgroundQuad;
    cameras = gPlayerCameras;
    camera = &cameras[playerIndex];
    /* The rsin result is discarded. Its projected height is inferred from the lens flare. */
    horizon = 240 * rsin(camera->pitch) / 4096;
    horizon = camera->pitch / 2;
    if (horizon < -60) horizon = -60;
    if (horizon > 120) horizon = 120;

    addGouraudQuad(&gCurrentPlayerOrderingTable[4095], -160, -120, 160, -120, -160, horizon, 160, horizon,
                  255, 64, 0, 255, 64, 0, 255, 255, 128, 255, 255, 128);
    addGouraudQuad(&gCurrentPlayerOrderingTable[4095], -160, -120, 160, -120, -160, horizon, 160, horizon,
                  255, 64, 0, 255, 64, 0, 255, 255, 128, 255, 255, 128);

    segment = gRacePlayers[playerIndex].unk92E;
    if (segment >= 106 && segment < 112) {
        r = 10;
        g = 20;
        b = 128;
    } else {
        r = 200;
        g = 210;
        b = 240;
    }
    if (horizon < 60) {
        addGouraudQuad(&gCurrentPlayerOrderingTable[4095], -160, horizon, 160, horizon, -160, 60, 160, 60,
                      255, 255, 128, 255, 255, 128, 200, 210, 240, 200, 210, 240);
        addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, 60, 320, 60, r, g, b);
    } else {
        addGouraudQuad(&gCurrentPlayerOrderingTable[4095], -160, horizon, 160, horizon, -160, 120, 160, 120,
                      255, 255, 128, 255, 255, 128, 200, 210, 240, 200, 210, 240);
    }

    if (selectedCameraOverlayModeGp) {
        addFlatRectangleWithDrawMode(gCurrentPlayerOrderingTable, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 0, 16, 32);
    } else {
        addFlatRectangleWithDrawMode(gCurrentPlayerOrderingTable, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 0, 32, 64);
    }
    SetFarColor(128, 100, 80);
    SetFogNearFar(8000, 10000, 400);
    GsInitCoordinate2(0, coordinate);
    GsGetLs(coordinate, matrix);
    ScaleMatrix(matrix, &sBackgroundModelScale);
    GsSetLsMatrix(matrix);
    drawProjectedModelPrimitives(gOptionalCourseModels[0], ot, 2, 1);
}

void drawNightHighwayBackgroundAndSetFog(s16 playerIndex) {
    s16 x, y;
    s32 pitch, pitchFixed;
    s32 scrollTerm;

    scrollTerm = (u32)(u16)gPlayerCameras[playerIndex].yaw << 16;
    x = -160 - ((scrollTerm >> 17) & 255);

    /* Bias negative pitches by one so halving rounds toward zero. */
    pitchFixed = (u32)(u16)gPlayerCameras[playerIndex].pitch << 16;
    pitch = pitchFixed >> 16;
    scrollTerm = (u32)pitchFixed >> 31;
    y = (pitch + scrollTerm) >> 1;

    addTexturedSprite(&gCurrentPlayerOrderingTable[4095], getTPage(0, 0, 640, 256), getClut(656, 496),
                      x, y, 256, 128, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[4095], getTPage(0, 0, 640, 256), getClut(656, 496),
                      (s16)(x + 256), y, 256, 128, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[4095], getTPage(0, 0, 640, 256), getClut(656, 496),
                      (s16)(x + 512), y, 256, 128, 0, 0, 128, 128, 128);

    addGouraudQuad(&gCurrentPlayerOrderingTable[4095],
                   -160, (s16)(y - 120), 160, (s16)(y - 120), -160, y, 160, y,
                   0, 0, 0, 0, 0, 0, 0, 32, 80, 0, 32, 80);
    addGouraudQuad(&gCurrentPlayerOrderingTable[4095],
                   -160, (s16)(y + 128), 160, (s16)(y + 128),
                   -160, (s16)(y + 248), 160, (s16)(y + 248),
                   0, 0, 16, 0, 0, 16, 0, 0, 16, 0, 0, 16);

    if (y > -120) {
        addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, -120, 320, (s16)(y + 120), 0, 0, 0);
    }
    addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, (s16)(y + 248), 320, 120, 0, 0, 16);

    if (selectedCameraOverlayModeGp) {
        addFlatRectangleWithDrawMode(gCurrentPlayerOrderingTable, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 24, 24, 16);
    } else {
        addFlatRectangleWithDrawMode(gCurrentPlayerOrderingTable, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 48, 48, 32);
    }
    SetFarColor(32, 32, 80);
    SetFogNearFar(8000, 10000, 400);
}

void drawCyanBackgroundAndSetFog(s16 playerIndex) {
    addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, -120, 320, 240, 0, 235, 230);
    SetFarColor(80, 128, 104);
    SetFogNearFar(5500, 10000, 400);
}

void drawOscillatingQuadTask(void) {
    OscillatingQuadScratchpad *scratch;
    OscillatingQuadResource *resource;
    GouraudQuadPacket *packet;
    s32 i;
    s16 cameraIndex;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (OscillatingQuadResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (GouraudQuadPacket *)GsOUT_PACKET_P;
    cameraIndex = (s16)getRenderCameraIndex();
    scratch = (OscillatingQuadScratchpad *)getScratchAddr(0);
    if (isPositionInCameraRange(resource->position[0], resource->position[2], cameraIndex)) {
        /* The draw-mode command shares the quad tag and adds one packet word. */
        packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
        packet->code = POLY_G4_CODE | SEMI_TRANSPARENT_FLAG;
        setRGB0(packet, 0, 0, 0);
        setRGB1(packet, 0, 0, 0);
        packet->tpage = getTPage(0, 1, 0, 0);
        packet->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
        if (!gRacePauseState && !(gGameStateFlags & 0x400) && (gFrameCounter & 1)) {
            setRGB2(packet, resource->r / 2, resource->g / 2, resource->b / 2);
            setRGB3(packet, resource->r / 2, resource->g / 2, resource->b / 2);
        } else {
            setRGB2(packet, resource->r, resource->g, resource->b);
            setRGB3(packet, resource->r, resource->g, resource->b);
        }

        resource->rotation.vz = resource->amplitude * rsin(resource->elapsed << 4) / 4096;
        RotMatrix(&resource->rotation, &scratch->rotationMatrix);
        for (i = 0; i < 4; i++) {
            ApplyMatrixSV(&scratch->rotationMatrix, &gOscillatingQuadVertices[i], &scratch->vertices[i]);
        }

        GsInitCoordinate2(0, &scratch->coordinate);
        for (i = 0; i < 3; i++) {
            scratch->coordinate.coord.t[i] = resource->position[i];
        }
        GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
        for (i = 0; i < 3; i++) {
            scratch->coordinate.coord.t[i] = scratch->coordinate.workm.t[i];
        }
        GsSetLsMatrix(&scratch->coordinate.coord);

        scratch->depth = RotTransPers4(&scratch->vertices[0], &scratch->vertices[1],
            &scratch->vertices[2], &scratch->vertices[3], (long *)&packet->x0,
            (long *)&packet->x1, (long *)&packet->x2, (long *)&packet->x3,
            &scratch->cue, &scratch->flags);
        if (!(scratch->flags & 0x7FFBE000) && scratch->depth >= 0 && scratch->depth < 4096) {
            addPrim(&gCurrentPlayerOrderingTable[scratch->depth >> 2], packet);
        }
        GsOUT_PACKET_P = (PACKET *)(packet + 1);
        if (!gRacePauseState) {
            resource->elapsed++;
        }
    }
}

void createOscillatingQuadTask(s32 value, VECTOR *position, s32 amplitude, s32 r, s32 g, s32 b) {
    OscillatingQuadResource *resource;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawOscillatingQuadTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (OscillatingQuadResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->unk68 = value;
    resource->r = r;
    resource->g = g;
    resource->b = b;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->elapsed = 0;
    resource->amplitude = amplitude;
    GsInitCoordinate2(0, &resource->coordinate);
    resource->position[0] = position->vx;
    resource->position[1] = position->vy;
    resource->position[2] = position->vz;
}

void drawMovingLineTask(void) {
    PlayerCamera *camera;
    PlayerCamera *cameras;
    MovingLineResource *resource;
    MovingLinePacket *line;
    u32 brightness;
    s32 yaw;
    s16 pitch;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    cameras = gPlayerCameras;
    camera = &cameras[gCurrentTask->data18];
    resource = (MovingLineResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    yaw = -((camera->yaw & 0xFFF) << 11) / 4096;
    pitch = camera->pitch;
    line = (MovingLinePacket *)GsOUT_PACKET_P;
    line->tag = (sizeof(MovingLinePacket) / sizeof(u32) - 1) << 24;
    line->code = GOURAUD_LINE_CODE | SEMI_TRANSPARENT_FLAG;
    line->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    line->tpage = getTPage(0, 1, 0, 0);
    line->x0 = ((u32)yaw + resource->x) % 320 - 160;
    line->y0 = ((u32)(pitch / 2) + resource->y) % 240 - 120;
    line->r1 = line->g1 = line->b1 = 0;
    line->x1 = line->x0 + 64;
    line->y1 = line->y0 - 32;
    brightness = 256;
    if (resource->elapsed < 16) {
        brightness = resource->elapsed * 16;
    }
    if (resource->duration - 16 < resource->elapsed) {
        brightness = (resource->duration - resource->elapsed) * 16;
    }
    line->b0 = brightness / 2;
    line->r0 = line->g0 = brightness * 255 / 256;
    addPrim(&gCurrentPlayerOrderingTable[4094], line);
    GsOUT_PACKET_P = (PACKET *)(line + 1);
    if (!gRacePauseState) {
        resource->x -= 8;
        resource->y += 4;
        if (++resource->elapsed == resource->duration) {
            deleteCurrentTaskSubtree();
        }
    }
}

void createMovingLineTask(s32 index) {
    MovingLineResource *resource;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawMovingLineTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = index;
    resource = (MovingLineResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->x = randomNextLinear() % 320;
    resource->y = 0;
    resource->elapsed = 0;
    resource->duration = randomNextLinear() % 15 + 15;
}

void drawFlickeringDotsTask(void) {
    FlickeringDotsResource *resource;
    TILE_1 *packet;
    TILE_1 *scratch;
    PlayerCamera *camera;
    PlayerCamera *cameras;
    s32 yaw, pitch;
    u32 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (FlickeringDotsResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (TILE_1 *)GsOUT_PACKET_P;
    scratch = (TILE_1 *)getScratchAddr(0);
    if (gGameStateFlags & 0x400) {
        if (gCurrentTask->data18 != gActiveGraphicsBuffer) return;
    } else if (gCurrentTask->data18 != 0) return;
    cameras = gPlayerCameras;
    camera = &cameras[gCurrentTask->data18];
    yaw = -((camera->yaw & 0xFFF) << 11) / 4096;
    pitch = camera->pitch / 2;
    scratch->tag = 2 << 24;
    setcode(scratch, TILE_1_CODE);
    for (i = 0; i < 25; i++) {
        scratch->x0 = ((u32)resource->positions[i].x + yaw) % 320 + (u16)-160;
        scratch->y0 = ((u32)resource->positions[i].y + pitch) % 240 + (u16)-120;
        scratch->r0 = scratch->g0 = rcos(randomLinearFromValue(i * 211) + (resource->frame << 7)) / 64 - 65;
        scratch->b0 = scratch->r0 / 2;
        *packet = *scratch;
        addPrim(&gCurrentPlayerOrderingTable[4094], packet);
        packet++;
    }
    GsOUT_PACKET_P = (PACKET *)packet;
    if (!gRacePauseState) {
        if ((++resource->frame & 15) == 0 && (randomNextLinear() & 15) == 0) {
            createMovingLineTask(gCurrentTask->data18);
        }
    }
}

void createFlickeringDotsTask(s32 playerIndex) {
    FlickeringDotsResource *resource;
    s32 i;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawFlickeringDotsTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = playerIndex;
    resource = (FlickeringDotsResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    for (i = 0; i < 25; i++) {
        resource->positions[i].x = randomNextLinear() % 320;
        resource->positions[i].y = randomNextLinear() % 120;
    }
    resource->frame = 0;
}

void drawRotatingModelBackgroundAndSetFog(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    s16 x, horizon;
    u32 y;

    if (playerIndex == 0) {
        ot = &gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer];
    }

    x = -160 - ((gPlayerCameras[playerIndex].yaw >> 1) & 255);
    y = (u32)(gPlayerCameras[playerIndex].pitch / 2);
    horizon = y;
    y &= 0xFFFF;
    addTexturedSprite(&gCurrentPlayerOrderingTable[4093], getTPage(0, 0, 640, 256), getClut(656, 480),
                      (s16)x, (s16)y, 256, 96, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[4093], getTPage(0, 0, 640, 256), getClut(656, 480),
                      (s16)(x + 256), (s16)y, 256, 96, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[4093], getTPage(0, 0, 640, 256), getClut(656, 480),
                      (s16)(x + 512), (s16)y, 256, 96, 0, 0, 128, 128, 128);

    {
        s32 right = 160;

        if ((s16)y < -248) horizon = -248;
        if (horizon > 120) horizon = 120;
        addSemiTransparentGouraudQuadWithDrawMode(&gCurrentPlayerOrderingTable[4094], getTPage(0, 1, 0, 0),
            -160, (s16)(horizon + 96), right, (s16)(horizon + 96),
            -160, (s16)(horizon + 128), right, (s16)(horizon + 128),
            0, 24, 32, 0, 24, 32, 0, 0, 0, 0, 0, 0);
        addFlatRectangle(&gCurrentPlayerOrderingTable[4094], -160, (s16)(horizon + 128), 320,
                         (s16)(-8 - horizon), 0, 0, 0);
        addSemiTransparentGouraudQuadWithDrawMode(&gCurrentPlayerOrderingTable[4095], getTPage(0, 1, 0, 0),
            -160, (s16)(horizon - 120), right, (s16)(horizon - 120),
            -160, horizon, right, horizon,
            0, 64, 128, 0, 64, 128, 0, 24, 32, 0, 24, 32);
        addSemiTransparentGouraudQuadWithDrawMode(&gCurrentPlayerOrderingTable[4095], getTPage(0, 1, 0, 0),
            -160, -120, right, -120, -160, (s16)(horizon - 120), right, (s16)(horizon - 120),
            0, 128, 128, 0, 128, 128, 0, 64, 128, 0, 64, 128);
    }

    if (isPositionInCameraRange(0x102D, 0x3395, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, 0x102D, 0x7F1, 0x3395);
        RotMatrixY(-sRotatingBackgroundFrame * 64, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        ScaleMatrix(matrix, &sBackgroundModelScale);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 1);
        if (!gRacePauseState) {
            sRotatingBackgroundFrame++;
        }
    }

    if (selectedCameraOverlayModeGp) {
        addFlatRectangleWithDrawMode(gCurrentPlayerOrderingTable, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 24, 24, 24);
    } else {
        addFlatRectangleWithDrawMode(gCurrentPlayerOrderingTable, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 48, 48, 48);
    }
    SetFarColor(80, 80, 100);
    SetFogNearFar(8000, 10000, 400);
}

void drawRipplingBackgroundAndSetFog(s16 playerIndex) {
    s32 column, strip, offset, x, y;
    u32 segment = (u16)gPlayerCameras[playerIndex].unk18;

    if (segment < 2 || (u32)(segment - 10) < 10 ||
        (u32)(segment - 41) < 9 || (u32)(segment - 125) < 4 ||
        (u32)(segment - 249) < 5 || (u32)(segment - 79) < 12 ||
        (s16)segment == 254 || (u32)(segment - 311) < 7) {
        gBackgroundLensFlareYawOffset = 0xE90;
        gBackgroundLensFlarePitchOffset = 0xF60;
        drawBackgroundLensFlare(playerIndex);
    }

    if (!gRacePauseState) {
        for (column = 0; column < 11; column++) {
            gRipplingBackgroundStripData[column] = gRipplingBackgroundStripData[column + 1];
        }
        gRipplingBackgroundStripData[11] = randomNextLinear() % 4 - 2;
        if (randomNextLinear() % 30) {
            gRipplingBackgroundStripData[0] = randomNextLinear() % 5 - 3;
        }
        if (randomNextLinear() % 120) {
            gRipplingBackgroundStripData[0] = randomNextLinear() % 8 - 4;
        }
    }

    x = -160 - ((gPlayerCameras[playerIndex].yaw >> 1) & 255);
    y = gPlayerCameras[playerIndex].pitch / 4 - 64;
    /* The bottom edge of the last strip uses the halfword following the offsets. */
    for (column = 0; column < 3; column++, x += 256) {
        strip = offset = 0;
        for (strip = 0; strip < 12; strip++, offset += 8) {
            addTexturedQuad(&gCurrentPlayerOrderingTable[4094],
                (s16)(x + gRipplingBackgroundStripData[strip]), (s16)(y + offset),
                (s16)(x + gRipplingBackgroundStripData[strip] + 256), (s16)(y + offset),
                (s16)(x + gRipplingBackgroundStripData[strip + 1]), (s16)(y + offset + 8),
                (s16)(x + gRipplingBackgroundStripData[strip + 1] + 256), (s16)(y + offset + 8),
                POLY_FT4_CODE, getTPage(0, 0, 640, 256), getClut(656, 510),
                0, (u8)(128 + offset), 255, (u8)(128 + offset),
                0, (u8)(135 + offset), 255, (u8)(135 + offset), 128, 128, 128);
        }
    }

    if (y < -120) y = -120;
    if (y > 120) y = 120;
    addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, -120, 320, (s16)(y + 120), 72, 184, 248);
    addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, (s16)y, 320, (s16)(120 - y), 232, 208, 144);
    SetFarColor(112, 100, 80);
    SetFogNearFar(8000, 10000, 400);
}

void drawLapAnimatedBackgroundAndSetFog(s16 playerIndex) {
    AnimatedBackgroundScratchpad *scratch = (AnimatedBackgroundScratchpad *)getScratchAddr(0);
    GsOT *ot;

    if (playerIndex == 0) {
        ot = &gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer];
    }
    addCopiedPrimitive(&gCurrentPlayerOrderingTable[2048], (u32 *)&gLapAnimatedBackgroundTile);
    if (isPositionInCameraRange(-0x152D4, 0xB540, playerIndex)) {
        switch (gRacePlayers[playerIndex].unk934) {
        case 0:
            GsInitCoordinate2(0, &scratch->root);
            scratch->root.coord.t[0] = -0x152D4;
            scratch->root.coord.t[1] = 0x7C4A;
            scratch->root.coord.t[2] = 0xB540;
            setVector(&scratch->rotation, 0, 0x9C2, 0);
            RotMatrix(&scratch->rotation, &scratch->root.coord);
            GsGetLs(&scratch->root, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(gOptionalCourseModels[0], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((-gLapAnimatedBackgroundFrame << 6) & 0xFC0, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((gLapAnimatedBackgroundFrame << 6) & 0xFC0, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(gOptionalCourseModels[2], ot, 2, 1);
            break;
        case 1:
            GsInitCoordinate2(0, &scratch->root);
            scratch->root.coord.t[0] = -0x152D4;
            scratch->root.coord.t[1] = 0x7B6E + rsin(gLapAnimatedBackgroundFrame << 6) * 25 / 4096;
            scratch->root.coord.t[2] = 0xB540;
            setVector(&scratch->rotation, 0, 0x9C2, 0);
            RotMatrix(&scratch->rotation, &scratch->root.coord);
            GsGetLs(&scratch->root, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(gOptionalCourseModels[0], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((-gLapAnimatedBackgroundFrame << 9) & 0xE00, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((gLapAnimatedBackgroundFrame << 9) & 0xE00, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(gOptionalCourseModels[2], ot, 2, 1);
            break;
        }
    }

    gLapAnimatedBackgroundFrame++;
    SetFarColor(224, 224, 232);
    SetFogNearFar(2400, 5600, 400);
}

void drawSnowBackgroundAndSetFog(s16 playerIndex) {
    s16 y;
    s16 x;
    y = gPlayerCameras[playerIndex].pitch / 2;
    x = -160 - ((gPlayerCameras[playerIndex].yaw >> 1) & 255);
    if (y > 240) y = 240;
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], 26, 0x7C29, x, y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], 26, 0x7C29, (s16)(x + 256), y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[1024], 26, 0x7C29, (s16)(x + 512), y, 256, 64, 0, 0, 128, 128, 128);
    if (y >= -119) {
        addFlatRectangle(&gCurrentPlayerOrderingTable[1024], -160, -120, 320, (s16)(y + 120), 0, 168, 248);
    }
    if (y < -120) y = -184;
    addFlatRectangle(&gCurrentPlayerOrderingTable[1024], -160, (s16)(y + 64), 320, (s16)(120 - y), 248, 248, 248);
    SetFarColor(248, 248, 248);
    SetFogNearFar(8000, 10000, 400);
}

void drawCameraCulledBackgroundAndSetFog(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    s16 x, y, horizon;

    if (playerIndex == 0) {
        ot = &gPlayer1AlternateLayerOrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2AlternateLayerOrderingTables[gActiveGraphicsBuffer];
    }
    x = -160 - ((gPlayerCameras[playerIndex].yaw >> 1) & 255);
    y = gPlayerCameras[playerIndex].pitch / 4;
    horizon = y;
    addTexturedSprite(&gCurrentPlayerOrderingTable[512], 26, 0x7C29, x, y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[512], 26, 0x7C29, (s16)(x + 256), y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[512], 26, 0x7C29, (s16)(x + 512), y, 256, 64, 0, 0, 128, 128, 128);
    if (horizon < -120) horizon = -120;
    if (horizon > 120) horizon = 120;
    addFlatRectangle(&gCurrentPlayerOrderingTable[1024], -160, -120, 320, (s16)(horizon + 120), 112, 168, 248);
    addFlatRectangle(&gCurrentPlayerOrderingTable[1024], -160, horizon, 320, (s16)(120 - horizon), 200, 216, 240);
    GsInitCoordinate2(0, coordinate);
    GsGetLs(coordinate, matrix);
    ScaleMatrix(matrix, &sBackgroundModelScale);
    GsSetLsMatrix(matrix);
    if (isPositionInCameraRange(0xADA, 0xCD1, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[0], ot, 2, 1);
    }
    if (isPositionInCameraRange(0xA61, 0x18D4, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x24E0, 0x1F11, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[2], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x47E0, 0x2B52, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[3], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x6356, 0x32B7, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[4], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x6B5E, 0x5C1C, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[5], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x6B5E, 0x5C1C, (s16)getRenderCameraIndex())) {
        drawProjectedModelPrimitives(gOptionalCourseModels[6], ot, 2, 1);
    }
    SetFarColor(112, 168, 248);
    SetFogNearFar(8000, 10000, 400);
}

void drawBackgroundModelsAndSetFog(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    s16 x, y, horizon;

    if (playerIndex == 0) {
        ot = &gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer];
    }
    x = -160 - ((gPlayerCameras[playerIndex].yaw >> 1) & 255);
    y = gPlayerCameras[playerIndex].pitch / 2;
    horizon = y;
    addTexturedSprite(&gCurrentPlayerOrderingTable[512], 26, 0x7C2A, x, y, 256, 64, 0, 128, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[512], 26, 0x7C2A, (s16)(x + 256), y, 256, 64, 0, 128, 128, 128, 128);
    addTexturedSprite(&gCurrentPlayerOrderingTable[512], 26, 0x7C2A, (s16)(x + 512), y, 256, 64, 0, 128, 128, 128, 128);
    if (horizon < -120) horizon = -120;
    if (horizon > 120) horizon = 120;
    addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, -120, 320, (s16)(horizon + 120), 0, 120, 240);
    addFlatRectangle(&gCurrentPlayerOrderingTable[4095], -160, horizon, 320, (s16)(120 - horizon), 255, 255, 255);
    GsInitCoordinate2(0, coordinate);
    GsGetLs(coordinate, matrix);
    ScaleMatrix(matrix, &sBackgroundModelScale);
    GsSetLsMatrix(matrix);
    drawProjectedModelPrimitives(gOptionalCourseModels[0], ot, 2, 1);
    drawProjectedModelPrimitives(gOptionalCourseModels[1], ot, 2, 1);
    drawProjectedModelPrimitives(gOptionalCourseModels[2], ot, 2, 1);
    SetFarColor(110, 110, 128);
    SetFogNearFar(8000, 10000, 400);
}

void drawCourseBackgroundForView(s16 view) {
    sCourseBackgroundCallbacks[gSelectedCourseIndex](view);
}

s32 isPositionInCameraRange(s32 x, s32 z, s16 playerIndex) {
    s32 centerX, centerZ;
    s32 scaledX, scaledZ;
    centerX = gPlayerCameraViews[playerIndex].vpx / 256 +
        ((gCourseModelCullDistance / 2) * rsin(gPlayerCameras[playerIndex].yaw)) / 4096;
    centerZ = gPlayerCameraViews[playerIndex].vpz / 256 +
        ((gCourseModelCullDistance / 2) * rcos(gPlayerCameras[playerIndex].yaw)) / 4096;
    scaledX = x / 256;
    scaledZ = z / 256;
    return SquareRoot0((centerX - scaledX) * (centerX - scaledX) +
        (centerZ - scaledZ) * (centerZ - scaledZ)) < gCourseModelCullDistance;
}

s32 isPointWithinCameraForwardRange(s32 x, s32 z, s16 player) {
    s32 cameraX, cameraZ;
    s32 scaledX, scaledZ;
    cameraX = gPlayerCameraViews[player].vpx / 256 + ((gCourseModelCullDistance / 2) * rsin(gPlayerCameras[player].yaw)) / 4096;
    cameraZ = gPlayerCameraViews[player].vpz / 256 + ((gCourseModelCullDistance / 2) * rcos(gPlayerCameras[player].yaw)) / 4096;
    scaledX = x / 256;
    scaledZ = z / 256;
    return SquareRoot0((cameraX - scaledX) * (cameraX - scaledX) + (cameraZ - scaledZ) * (cameraZ - scaledZ)) < (gCourseModelCullDistance * 3) / 4;
}

void readVramPixelColor(s32 x, s32 y, TextureColor *out) {
    u16 pixel;
    gReadPixelRect.x = x;
    gReadPixelRect.y = y;
    StoreImage(&gReadPixelRect, (u32 *)&pixel);
    out->r = pixel << 3;
    out->g = (pixel >> 2) & 0xF8;
    out->b = (pixel >> 7) & 0xF8;
    out->stp = pixel >> 15;
}

void writeVramPixelColor(s32 x, s32 y, u32 r, u32 g, u32 b, u32 stp) {
    u16 color;
    u32 rgb = (r & 0xF8) >> 3;
    u32 alpha = stp & 1;
    u32 green = (g & 0xF8) << 2;
    u32 blue;
    gWritePixelRect.x = x;
    gWritePixelRect.y = y;
    rgb |= green;
    blue = (b & 0xF8) << 7;
    rgb |= blue;
    alpha <<= 15;
    color = rgb | alpha;
    LoadImage(&gWritePixelRect, (u32 *)&color);
}

void unpackPsxColor(u32 color, UnpackedPsxColor *out) {
    out->r = color & 0x1F;
    out->g = (color & 0x3E0) >> 5;
    out->b = (color & 0x7C00) >> 10;
    out->stp = (color & 0x8000) >> 15;
}

u32 packTextureColor(TextureColor *color) {
    u32 rgb = color->r >> 3;
    u32 stp = color->stp & 1;
    u32 g = (color->g & 0xF8) << 2;
    u32 b;
    rgb |= g;
    b = (color->b & 0xF8) << 7;
    rgb |= b;
    stp <<= 15;
    return stp | rgb;
}

void initializeScanlineCopySeed(void) {
    u8 padding[0x20];
    s32 i;
    for (i = 0; i < 0x40; i++) {
        writeVramPixelColor((s16)(i + 704), 0x90, 0xE0, 0xE0, 0xE8, 0);
    }
}

void queueScanlineCopyPackets(void) {
    DR_MOVE *move;
    DR_STP *stp;
    s32 y, i, depth;

    move = (DR_MOVE *)GsOUT_PACKET_P;
    for (y = 256; y < 512; y += 16) {
        SetDrawMove(move, &gScanlineSeedRect, 640, y);
        addPrim(&gCurrentPlayerOrderingTable[4095], move);
        move += 2;
    }
    GsOUT_PACKET_P = (PACKET *)move;
    if (selectedCameraOverlayModeGp) {
        stp = (DR_STP *)GsOUT_PACKET_P;
        for (i = 0; i < 15; i++) {
            depth = (i * 200) / 15 + 150;
            SetDrawStp(stp, 1);
            addPrim(&gCurrentPlayerOrderingTable[depth], stp);
            stp++;
        }
        GsOUT_PACKET_P = (PACKET *)stp;
    }
    move = (DR_MOVE *)GsOUT_PACKET_P;
    for (i = 0; i < 15; i++) {
        y = 256;
        gScanlineCopyRect.y = i + 257;
        depth = (i * 200) / 15 + 150;
        for (; y < 512; y += 16) {
            SetDrawMove(move, &gScanlineCopyRect, 640, y);
            addPrim(&gCurrentPlayerOrderingTable[depth], move);
            gScanlineCopyRect.y += 16;
            move++;
        }
    }
    GsOUT_PACKET_P = (PACKET *)move;
    if (selectedCameraOverlayModeGp) {
        stp = (DR_STP *)GsOUT_PACKET_P;
        for (i = 0; i < 15; i++) {
            depth = (i * 200) / 15 + 150;
            SetDrawStp(stp, 0);
            addPrim(&gCurrentPlayerOrderingTable[depth], stp);
            stp++;
        }
        GsOUT_PACKET_P = (PACKET *)stp;
    }
}

void initializeCourseModels(void) {
    u32 *model;
    u32 *offset;
    s16 i;
    Position3D *positions;

    bindGeometryRecordData((s16 *)DAT_COURSE_FIRST);
    positions = gCourseModelPositions = gCourseModelPositionBuffers[gSelectedCourseIndex];
    for (i = 0; i < 100; i++) {
        positions[i].x *= 5;
        positions[i].y *= 5;
        positions[i].z *= 5;
    }

    offset = (u32 *)DAT_COURSE_SECOND;
    gCourseModelCounts[0] = *(u16 *)offset;
    offset++;
    for (i = 0; i < gCourseModelCounts[0]; i++) {
        model = ((OffsetArchive *)DAT_COURSE_SECOND)->offsets;
        model = (u32 *)((u8 *)model + *offset);
        GsMapModelingData(model);
        model++;
        gCourseModelObjectCount = *model++;
        gCourseModels[i] = (TmdObject *)model;
        offset++;
        gCourseModelPositions[i].x /= 256;
        gCourseModelPositions[i].y /= 256;
        gCourseModelPositions[i].z /= 256;
    }

    GsInitCoordinate2(0, &gCourseModelCoordinate);
    gCourseModelCullDistance = gCourseModelCullDistances[gSelectedCourseIndex] / 256;
    gCourseModelDetailDistance = gCourseModelDetailDistances[gSelectedCourseIndex] / 256;
    if ((u16)gCourseOptionalResourceSectors[gSelectedCourseIndex].sector != 0) {
        offset = (u32 *)DAT_COURSE_OPTIONAL;
        gCourseModelCounts[1] = *(u16 *)offset;
        offset++;
        for (i = 0; i < gCourseModelCounts[1]; i++) {
            model = (u32 *)(DAT_COURSE_OPTIONAL + *offset++);
            model++;
            GsMapModelingData(model);
            model++;
            gOptionalCourseModelObjectCount = *model++;
            gOptionalCourseModels[i] = (TmdObject *)model;
        }
    }
    if (gSelectedCourseIndex == 6) {
        initializeScanlineCopySeed();
    }
}

void drawCourseModelsForView(s16 view) {
    MATRIX *matrix = (MATRIX *)getScratchAddr(0);
    TmdObject **models;
    VECTOR scale;
    GsRVIEW2 *cameraView;
    PlayerCamera *camera;
    PlayerCamera *cameras;
    GsRVIEW2 *views;
    s32 centerX, centerZ;
    s32 cameraX, cameraZ;
    GsOT *ot;
    s32 sine, cosine;
    u32 flags;
    u16 i;

    if (view == 0) {
        ot = &gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer];
    } else {
        ot = &gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer];
    }
    scale = gCourseModelScale;
    if (gCourseRenderingDisabled != 0) {
        return;
    }
    drawCourseBackgroundForView(view);
    if (gSelectedCourseIndex == 6) {
        queueScanlineCopyPackets();
    }
    cameras = gPlayerCameras;
    camera = &cameras[view];
    sine = rsin(camera->yaw);
    views = gPlayerCameraViews;
    cameraView = &views[view];
    centerX = cameraView->vpx / 256 + ((gCourseModelCullDistance / 2) * sine) / 4096;
    cosine = rcos(camera->yaw);
    centerZ = cameraView->vpz / 256 + ((gCourseModelCullDistance / 2) * cosine) / 4096;
    cameraX = cameraView->vpx / 256;
    cameraZ = cameraView->vpz / 256;

    GsGetLs(&gCourseModelCoordinate, matrix);
    ScaleMatrix(matrix, &scale);
    GsSetLsMatrix(matrix);

    if (gSelectedCourseIndex == 6) {
        flags = 1;
    } else {
        flags = 3;
    }

    i = 0;
    models = gCourseModels;
    do {
        const Position3D *positions;
        s32 x, z, dx, dz;

        if (i >= gCourseModelCounts[0]) {
            goto queue_draw_mode;
        }
        positions = gCourseModelPositions;
        x = positions[i].x;
        z = positions[i].z;
        dx = centerX - x;
        dz = centerZ - z;
        if (SquareRoot0(dx * dx + dz * dz) <= gCourseModelCullDistance) {
            s32 dx = cameraX - x;
            s32 dz = cameraZ - z;
            if (SquareRoot0(dx * dx + dz * dz) >= gCourseModelDetailDistance) {
                drawProjectedModelPrimitives(models[i], ot, 2, flags);
            } else {
                drawAdaptiveModelPrimitives(models[i], ot, 2, flags);
            }
        }
        i++;
    } while (1);

queue_draw_mode:
    SetDrawMode(&gCourseModelDrawMode, 0, 1, 0, &gCourseModelTextureWindow);
    addPrim(&gCurrentPlayerOrderingTable[4095], &gCourseModelDrawMode);
    if (selectedCameraOverlayModeGp) {
        queueSelectedCameraOverlayPackets();
    }
}

void queueSelectedCameraOverlayPackets(void) {
    DR_STP *packet;
    switch (selectedCameraOverlayModeGp) {
    case 1:
        if (gActiveGraphicsBuffer) {
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlaySpritesBuffer1[0]);
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlaySpritesBuffer1[1]);
        } else {
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlaySpritesBuffer0[0]);
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlaySpritesBuffer0[1]);
        }
        break;
    case 2:
        if (gActiveGraphicsBuffer) {
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlayQuadsBuffer1[1]);
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlayQuadsBuffer1[0]);
        } else {
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlayQuadsBuffer0[1]);
            addPrim(&gCurrentPlayerOrderingTable[2], &gCameraOverlayQuadsBuffer0[0]);
        }
        break;
    }
    packet = (DR_STP *)GsOUT_PACKET_P;
    SetDrawStp(packet, 1);
    addPrim(&gCurrentPlayerOrderingTable[4095], packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
}

void queueOffsetScreenCopySprites(void) {
    DR_STP *packet;
    if (gActiveGraphicsBuffer) {
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyPlusBuffer1[0]);
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyPlusBuffer1[1]);
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyMinusBuffer1[0]);
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyMinusBuffer1[1]);
    } else {
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyPlusBuffer0[0]);
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyPlusBuffer0[1]);
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyMinusBuffer0[0]);
        addPrim(&gCurrentPlayerOrderingTable[2], &gScreenCopyMinusBuffer0[1]);
    }
    packet = (DR_STP *)GsOUT_PACKET_P;
    SetDrawStp(packet, 1);
    addPrim(&gCurrentPlayerOrderingTable[4095], packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
}

void drawNoProcessMessage(void) {
    drawDebugText(0, 30, gNoProcessMessage);
}
