#include "psx/libetc.h"
#include "render_effects_internal.h"
#include "../race/race_internal.h"
#include "game/engine/game_loop.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
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
extern s16 D_8013AFCE __attribute__((section(".data")));
extern u32 *D_8013AF6C __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern u8 D_8013AFD3 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));

void drawLayeredBackgroundAndSetFog(s16 playerIndex);
void func_8004E718(s16 playerIndex);
void func_8004EBF8(s16 playerIndex);
void func_8004FB70(s16 playerIndex);
void drawRipplingBackgroundAndSetFog(s16 playerIndex);

TILE gLapAnimatedBackgroundTile __attribute__((section(".data.lap_animated_background_tile"))) = {
    3 << 24, 224, 224, 232, TILE_CODE, -160, -120, 320, 240,
};

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
    func_8004E718,
    func_8004EBF8,
    drawCyanBackgroundAndSetFog,
    func_8004FB70,
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

const VECTOR gCourseModelScale __attribute__((section(".rodata.course_model_scale"))) = {
    5 * 4096, 5 * 4096, 5 * 4096, 0,
};

RECT gCourseModelTextureWindow __attribute__((section(".data.course_model_texture_window"))) = {0};

DR_MODE gCourseModelDrawMode __attribute__((section(".bss.course_model_draw_mode")));

extern VECTOR D_80124BDC;
extern GsCOORDINATE2 D_80124FF4;
extern s32 D_8013AFB8 __attribute__((section(".data")));
extern s16 D_8013B22C, D_8013B22E;
extern Position3D *D_8013B280 __attribute__((section(".data")));
extern u16 D_8013B654[2];
extern s32 D_8013B664;
extern TmdObject *D_8013BBF8[];

extern u32 GsOUT_PACKET_P __attribute__((section(".data")));

void getCourseSteeringTarget(s16 region, s32 x, s32 z, s32 *targetX, s32 *targetZ,
                             u16 tableIndex, u16 playerIndex) {
    s32 dx, dz, forward, cosine, lateral;
    s16 vertexX, vertexZ;
    s16 nextRegion;
    s32 vertexIndex;
    s16 *vertices;
    SelectedCourseRecord *course;
    SelectedCourseRecord *courses = D_80123D80;

    if (region != (s16)courses[D_8013AFCE].unk00) {
        D_8013B624 = rsin(-(s16)((GeometryPlacement *)D_8013B1F4)[region].angle);
        cosine = rcos(-(s16)((GeometryPlacement *)D_8013B1F4)[region].angle);
        vertexIndex = ((GeometryPlacement *)D_8013B1F4)[region].vertexIndex;
        dx = x - D_8013B1EC[vertexIndex * 3] * 5;
        dz = z - D_8013B1EC[vertexIndex * 3 + 2] * 5;
        D_8013B628 = cosine;
        dz = (-D_8013B624 * dx + cosine * dz) / 4096;

        course = &courses[D_8013AFCE];
        if ((region >= course->objectFirstSegment && region <= course->objectLastSegment) ||
            (D_8013AFCE == 3 && (u16)(region - 285) < 2)) {
            dz -= 960;
            *targetX = -D_8013B624 * dz / 4096;
            *targetZ = D_8013B628 * dz / 4096;
        } else {
            dz += 960;
            *targetX = -D_8013B624 * dz / 4096;
            *targetZ = D_8013B628 * dz / 4096;
        }

        *targetX += D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[region].vertexIndex * 3] * 5;
        *targetZ += D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[region].vertexIndex * 3 + 2] * 5;

        nextRegion = getAdjacentGeometryRegionForPoint(region, *targetX, *targetZ);
        D_8013B624 = rsin(-(s16)((GeometryPlacement *)D_8013B1F4)[nextRegion].angle);
        D_8013B628 = rcos(-(s16)((GeometryPlacement *)D_8013B1F4)[nextRegion].angle);
        vertexIndex = ((GeometryPlacement *)D_8013B1F4)[nextRegion].vertexIndex * 3;
        vertices = D_8013B1EC;
        vertexX = vertices[vertexIndex];
        vertexZ = vertices[vertexIndex + 2];
        dx = *targetX - vertexX * 5;
        dz = *targetZ - vertexZ * 5;
        forward = updatePlayerField94CFromSelectedTable(tableIndex, nextRegion, playerIndex);

        lateral = (-D_8013B624 * dx + D_8013B628 * dz) / 4096;
        *targetX = (D_8013B628 * forward + -D_8013B624 * lateral) / 4096;
        *targetZ = (D_8013B624 * forward + D_8013B628 * lateral) / 4096;
        *targetX += D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[nextRegion].vertexIndex * 3] * 5;
        *targetZ += D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[nextRegion].vertexIndex * 3 + 2] * 5;
    } else {
        *targetX = (s32)courses[D_8013AFCE].unk18[0] >> 8;
        *targetZ = (s32)courses[D_8013AFCE].unk18[2] >> 8;
    }
}

void adjustDebugPositionAndDisplay(VECTOR *position) {
    if (D_8013AEEC[1] & 0x1000) position->vx += 32;
    if (D_8013AEEC[1] & 0x4000) position->vx -= 32;
    if (D_8013AEEC[1] & 0x8000) position->vz += 32;
    if (D_8013AEEC[1] & 0x2000) position->vz -= 32;
    if (D_8013AEEC[1] & 8) position->vy += 32;
    if (D_8013AEEC[1] & 2) position->vy -= 32;
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

    cameras = D_801241E8;
    camera = &cameras[playerIndex];
    yaw = camera->yaw & 0xFFF;
    if (yaw > 0x400 && yaw < 0xC00) {
        return;
    }
    x = -320 * rsin(camera->yaw + D_8013B22C) / 4096;
    if ((u16)((y = 240 * rsin(camera->pitch + D_8013B22E) / 4096) + 120) > 240) {
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

        addSemiTransparentGouraudTriangle(D_8013AF6C, getTPage(0, 1, 0, 0),
            outerX, outerY, rightX, rightY, centerX, centerY,
            (u8)red, (u8)green, (u8)blue, 0, 0, 0, 64, 64, 64);
        addSemiTransparentGouraudTriangle(D_8013AF6C, getTPage(0, 1, 0, 0),
            outerX, outerY, centerX, centerY, leftX, leftY,
            (u8)red, (u8)green, (u8)blue, 64, 64, 64, 0, 0, 0);
    }

    addTexturedQuad(D_8013AF6C,
        (s16)(x - 32), (s16)(y - 32), (s16)(x + 32), (s16)(y - 32),
        (s16)(x - 32), (s16)(y + 32), (s16)(x + 32), (s16)(y + 32),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 241),
        0, 0, 63, 0, 0, 63, 63, 63, 96, 96, 96);
    addTexturedQuad(D_8013AF6C,
        (s16)(x / 2 - 16), (s16)(y / 2 - 16), (s16)(x / 2 + 16), (s16)(y / 2 - 16),
        (s16)(x / 2 - 16), (s16)(y / 2 + 16), (s16)(x / 2 + 16), (s16)(y / 2 + 16),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 240),
        0, 0, 63, 0, 0, 63, 63, 63, 38, 24, 24);
    addTexturedQuad(D_8013AF6C,
        -20, -20, 20, -20, -20, 20, 20, 20,
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 241),
        0, 0, 63, 0, 0, 63, 63, 63, 50, 68, 50);
    addTexturedQuad(D_8013AF6C,
        (s16)(-x / 2 - 40), (s16)(-y / 2 - 40), (s16)(-x / 2 + 40), (s16)(-y / 2 - 40),
        (s16)(-x / 2 - 40), (s16)(-y / 2 + 40), (s16)(-x / 2 + 40), (s16)(-y / 2 + 40),
        SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0), getClut(880, 242),
        0, 0, 63, 0, 0, 63, 63, 63, 10, 20, 10);
    addTexturedQuad(D_8013AF6C,
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
        ot = &D_800EC018[D_8013AF78];
    } else {
        ot = &D_800EC0B8[D_8013AF78];
    }

    cameras = D_801241E8;
    camera = &cameras[playerIndex];
    if ((u16)camera->unk18 < 8) {
        D_8013B22C = 0xF28;
        D_8013B22E = 0xF70;
        drawBackgroundLensFlare(playerIndex);
    }

    x = -160 - ((camera->yaw >> 1) & 255);
    y = camera->pitch / 2;
    horizon = y - 64;

    addTexturedSprite(&D_8013AF6C[1024], getTPage(0, 0, 640, 256), getClut(656, 481),
        x, (s16)(y - 64), 256, 48, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], getTPage(0, 0, 640, 256), getClut(656, 480),
        x, (s16)(y - 16), 256, 80, 0, 48, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], getTPage(0, 0, 640, 256), getClut(656, 481),
        (s16)(x + 256), (s16)(y - 64), 256, 48, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], getTPage(0, 0, 640, 256), getClut(656, 480),
        (s16)(x + 256), (s16)(y - 16), 256, 80, 0, 48, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], getTPage(0, 0, 640, 256), getClut(656, 481),
        (s16)(x + 512), (s16)(y - 64), 256, 48, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], getTPage(0, 0, 640, 256), getClut(656, 480),
        (s16)(x + 512), (s16)(y - 16), 256, 80, 0, 48, 128, 128, 128);

    addGouraudQuad(&D_8013AF6C[1024],
        -160, (s16)(y - 184), 160, (s16)(y - 184), -160, (s16)(y - 64), 160, (s16)(y - 64),
        128, 255, 255, 128, 255, 255, 41, 123, 230, 0, 112, 230);
    addGouraudQuad(&D_8013AF6C[1024],
        -160, (s16)(y + 64), 160, (s16)(y + 64), -160, (s16)(y + 184), 160, (s16)(y + 184),
        255, 255, 255, 255, 255, 255, 220, 220, 220, 220, 220, 220);

    if (horizon > 120) horizon = 120;
    if (horizon < -120) horizon = -120;
    addFlatRectangle(&D_8013AF6C[1024], -160, -120, 320, (s16)(horizon + 120), 128, 255, 255);
    addFlatRectangle(&D_8013AF6C[1024], -160, horizon, 320, (s16)(120 - horizon), 220, 220, 220);

    if (isPointWithinCameraForwardRange(-0xF02D, 0xC5B6, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, -0xEE39, 0x14DAC, 0xC7AA);
        RotMatrixY(512, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        ScaleMatrix(matrix, &D_80124BDC);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(D_80125044[0], ot, 2, 3);
    }
    if (isPointWithinCameraForwardRange(gLayeredBackgroundModelPosition1.vx, gLayeredBackgroundModelPosition1.vz, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, gLayeredBackgroundModelPosition1.vx, gLayeredBackgroundModelPosition1.vy, gLayeredBackgroundModelPosition1.vz);
        RotMatrixY(-1000, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(D_80125044[1], ot, 2, 3);
    }
    if (isPointWithinCameraForwardRange(gLayeredBackgroundModelPosition2.vx, gLayeredBackgroundModelPosition2.vz, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        setVector((VECTOR *)coordinate->coord.t, gLayeredBackgroundModelPosition2.vx, gLayeredBackgroundModelPosition2.vy, gLayeredBackgroundModelPosition2.vz);
        RotMatrixY(600, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(D_80125044[1], ot, 2, 3);
    }

    SetFarColor(160, 160, 200);
    SetFogNearFar(8000, 10000, 400);
}

/* DECOMP_ATTEMPT: inline 309/312 */
#ifdef NON_MATCHING
extern POLY_G4 D_80124C3C[];

void func_8004E718(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    PlayerCamera *camera;
    s16 horizon;
    s32 r, g, b;

    if (playerIndex == 0) {
        ot = &D_800EC018[D_8013AF78];
    } else {
        ot = &D_800EC0B8[D_8013AF78];
    }
    *(POLY_G4 *)coordinate = D_80124C3C[0];
    camera = D_801241E8;
    camera = playerIndex + camera;
    rsin(camera->pitch);
    horizon = camera->pitch / 2;
    if (horizon < -60) horizon = -60;
    if (horizon > 120) horizon = 120;

    addGouraudQuad(&D_8013AF6C[4095], -160, -120, 160, -120, -160, horizon, 160, horizon,
                  255, 64, 0, 255, 64, 0, 255, 255, 128, 255, 255, 128);
    addGouraudQuad(&D_8013AF6C[4095], -160, -120, 160, -120, -160, horizon, 160, horizon,
                  255, 64, 0, 255, 64, 0, 255, 255, 128, 255, 255, 128);

    if ((u32)((u16)gRacePlayers[playerIndex].unk92E - 106) < 6) {
        r = 10;
        g = 20;
        b = 128;
    } else {
        r = 200;
        g = 210;
        b = 240;
    }
    if (horizon < 60) {
        addGouraudQuad(&D_8013AF6C[4095], -160, horizon, 160, horizon, -160, 60, 160, 60,
                      255, 255, 128, 255, 255, 128, 200, 210, 240, 200, 210, 240);
        addFlatRectangle(&D_8013AF6C[4095], -160, 60, 320, 60, r, g, b);
    } else {
        addGouraudQuad(&D_8013AF6C[4095], -160, horizon, 160, horizon, -160, 120, 160, 120,
                      255, 255, 128, 255, 255, 128, 200, 210, 240, 200, 210, 240);
    }

    if (selectedCameraOverlayModeGp) {
        addFlatRectangleWithDrawMode(D_8013AF6C, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 0, 16, 32);
    } else {
        addFlatRectangleWithDrawMode(D_8013AF6C, getTPage(0, 2, 0, 0), SEMI_TRANSPARENT_FLAG,
                                     -160, -120, 320, 240, 0, 32, 64);
    }
    SetFarColor(128, 100, 80);
    SetFogNearFar(8000, 10000, 400);
    GsInitCoordinate2(0, coordinate);
    GsGetLs(coordinate, matrix);
    ScaleMatrix(matrix, &D_80124BDC);
    GsSetLsMatrix(matrix);
    drawProjectedModelPrimitives(D_80125044[0], ot, 2, 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/graphics/course_rendering", func_8004E718);
#endif

/* DECOMP_ATTEMPT: inline 219/222 */
#ifdef NON_MATCHING
void func_8004EBF8(s16 playerIndex) {
    s16 x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    s32 y = D_801241E8[playerIndex].pitch / 2;
    s16 spriteY = y;

    addTexturedSprite(&D_8013AF6C[4095], getTPage(0, 0, 640, 256), getClut(656, 496),
                      x, (s16)spriteY, 256, 128, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[4095], getTPage(0, 0, 640, 256), getClut(656, 496),
                      (s16)(x + 256), (s16)spriteY, 256, 128, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[4095], getTPage(0, 0, 640, 256), getClut(656, 496),
                      (s16)(x + 512), (s16)spriteY, 256, 128, 0, 0, 128, 128, 128);
    addGouraudQuad(&D_8013AF6C[4095], -160, (s16)(y - 120), 160, (s16)(y - 120),
                   -160, (s16)spriteY, 160, (s16)spriteY, 0, 0, 0, 0, 0, 0, 0, 32, 80, 0, 32, 80);
    addGouraudQuad(&D_8013AF6C[4095], -160, (s16)(y + 128), 160, (s16)(y + 128),
                   -160, (s16)(y + 248), 160, (s16)(y + 248),
                   0, 0, 16, 0, 0, 16, 0, 0, 16, 0, 0, 16);
    if ((s16)spriteY > -120) {
        addFlatRectangle(&D_8013AF6C[4095], -160, -120, 320, (s16)(y + 120), 0, 0, 0);
    }
    addFlatRectangle(&D_8013AF6C[4095], -160, (s16)(y + 248), 320, 120, 0, 0, 16);
    if (D_8013B20C) {
        addFlatRectangleWithDrawMode(D_8013AF6C, getTPage(0, 2, 0, 0),
                                    SEMI_TRANSPARENT_FLAG, -160, -120, 320, 240, 24, 24, 16);
    } else {
        addFlatRectangleWithDrawMode(D_8013AF6C, getTPage(0, 2, 0, 0),
                                    SEMI_TRANSPARENT_FLAG, -160, -120, 320, 240, 48, 48, 32);
    }
    SetFarColor(32, 32, 80);
    SetFogNearFar(8000, 10000, 400);
}
#else
INCLUDE_ASM("asm/nonmatchings/graphics/course_rendering", func_8004EBF8);
#endif

void drawCyanBackgroundAndSetFog(s16 playerIndex) {
    addFlatRectangle(&D_8013AF6C[4095], -160, -120, 320, 240, 0, 235, 230);
    SetFarColor(80, 128, 104);
    SetFogNearFar(5500, 10000, 400);
}

void drawOscillatingQuadTask(void) {
    OscillatingQuadScratchpad *scratch;
    OscillatingQuadResource *resource;
    GouraudQuadPacket *packet;
    s32 i;
    s16 cameraIndex;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (OscillatingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (GouraudQuadPacket *)GsOUT_PACKET_P;
    cameraIndex = (s16)func_80059254();
    scratch = (OscillatingQuadScratchpad *)getScratchAddr(0);
    if (isPositionInCameraRange(resource->position[0], resource->position[2], cameraIndex)) {
        /* The draw-mode command shares the quad tag and adds one packet word. */
        packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
        packet->code = POLY_G4_CODE | SEMI_TRANSPARENT_FLAG;
        setRGB0(packet, 0, 0, 0);
        setRGB1(packet, 0, 0, 0);
        packet->tpage = getTPage(0, 1, 0, 0);
        packet->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
        if (!D_8013AFD3 && !(D_8013AFAA & 0x400) && (gFrameCounter & 1)) {
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
            addPrim(&D_8013AF6C[scratch->depth >> 2], packet);
        }
        GsOUT_PACKET_P = (u32)(packet + 1);
        if (!D_8013AFD3) {
            resource->elapsed++;
        }
    }
}

void createOscillatingQuadTask(s32 value, VECTOR *position, s32 amplitude, s32 r, s32 g, s32 b) {
    OscillatingQuadResource *resource;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawOscillatingQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (OscillatingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
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
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    cameras = D_801241E8;
    camera = &cameras[D_8013AE88->data18];
    resource = (MovingLineResource *)&D_800D9504[D_8013AE88->resource14];
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
    addPrim(&D_8013AF6C[4094], line);
    GsOUT_PACKET_P = (u32)(line + 1);
    if (!D_8013AFD3) {
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
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = (MovingLineResource *)&D_800D9504[D_8013AE8C->resource14];
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
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (FlickeringDotsResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (TILE_1 *)GsOUT_PACKET_P;
    scratch = (TILE_1 *)getScratchAddr(0);
    if (D_8013AFAA & 0x400) {
        if (D_8013AE88->data18 != D_8013AF78) return;
    } else if (D_8013AE88->data18 != 0) return;
    cameras = D_801241E8;
    camera = &cameras[D_8013AE88->data18];
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
        addPrim(&D_8013AF6C[4094], packet);
        packet++;
    }
    GsOUT_PACKET_P = (u32)packet;
    if (!D_8013AFD3) {
        if ((++resource->frame & 15) == 0 && (randomNextLinear() & 15) == 0) {
            createMovingLineTask(D_8013AE88->data18);
        }
    }
}

void createFlickeringDotsTask(s32 playerIndex) {
    FlickeringDotsResource *resource;
    s32 i;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawFlickeringDotsTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = playerIndex;
    resource = (FlickeringDotsResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 25; i++) {
        resource->positions[i].x = randomNextLinear() % 320;
        resource->positions[i].y = randomNextLinear() % 120;
    }
    resource->frame = 0;
}

/* DECOMP_ATTEMPT: inline 323/325 */
#ifdef NON_MATCHING
extern u32 D_8013B238;

void func_8004FB70(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    s16 x, horizon;
    u32 y;

    if (playerIndex == 0) {
        ot = &D_800EC040[D_8013AF78];
    } else {
        ot = &D_800EC0E0[D_8013AF78];
    }

    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    y = (u32)(D_801241E8[playerIndex].pitch / 2) & 0xFFFF;
    horizon = y;
    addTexturedSprite(&D_8013AF6C[4093], getTPage(0, 0, 640, 256), getClut(656, 480),
                      (s16)x, (s16)y, 256, 96, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[4093], getTPage(0, 0, 640, 256), getClut(656, 480),
                      (s16)(x + 256), (s16)y, 256, 96, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[4093], getTPage(0, 0, 640, 256), getClut(656, 480),
                      (s16)(x + 512), (s16)y, 256, 96, 0, 0, 128, 128, 128);

    {
        s32 right = 160;
    if (horizon < -248) horizon = -248;
    if (horizon > 120) horizon = 120;
    addSemiTransparentGouraudQuadWithDrawMode(&D_8013AF6C[4094], getTPage(0, 1, 0, 0),
        -160, (s16)(horizon + 96), right, (s16)(horizon + 96),
        -160, (s16)(horizon + 128), right, (s16)(horizon + 128),
        0, 24, 32, 0, 24, 32, 0, 0, 0, 0, 0, 0);
    addFlatRectangle(&D_8013AF6C[4094], -160, (s16)(horizon + 128), 320,
                     (s16)(-8 - horizon), 0, 0, 0);
    addSemiTransparentGouraudQuadWithDrawMode(&D_8013AF6C[4095], getTPage(0, 1, 0, 0),
        -160, (s16)(horizon - 120), right, (s16)(horizon - 120),
        -160, horizon, right, horizon,
        0, 64, 128, 0, 64, 128, 0, 24, 32, 0, 24, 32);
    addSemiTransparentGouraudQuadWithDrawMode(&D_8013AF6C[4095], getTPage(0, 1, 0, 0),
        -160, -120, right, -120, -160, (s16)(horizon - 120), right, (s16)(horizon - 120),
        0, 128, 128, 0, 128, 128, 0, 64, 128, 0, 64, 128);

    }
    if (isPositionInCameraRange(0x102D, 0x3395, playerIndex)) {
        GsInitCoordinate2(0, coordinate);
        coordinate->coord.t[0] = 0x102D;
        coordinate->coord.t[1] = 0x7F1;
        coordinate->coord.t[2] = 0x3395;
        RotMatrixY(-D_8013B238 * 64, &coordinate->coord);
        GsGetLs(coordinate, matrix);
        ScaleMatrix(matrix, &D_80124BDC);
        GsSetLsMatrix(matrix);
        drawProjectedModelPrimitives(D_80125044[1], ot, 2, 1);
        if (!D_8013AFD3) {
            D_8013B238++;
        }
    }

    if (selectedCameraOverlayModeGp) {
        addFlatRectangleWithDrawMode(&D_8013AF6C[0], getTPage(0, 2, 0, 0), 2,
                                     -160, -120, 320, 240, 24, 24, 24);
    } else {
        addFlatRectangleWithDrawMode(&D_8013AF6C[0], getTPage(0, 2, 0, 0), 2,
                                     -160, -120, 320, 240, 48, 48, 48);
    }
    SetFarColor(80, 80, 100);
    SetFogNearFar(8000, 10000, 400);
}
#else
INCLUDE_ASM("asm/nonmatchings/graphics/course_rendering", func_8004FB70);
#endif

void drawRipplingBackgroundAndSetFog(s16 playerIndex) {
    s32 column, strip, offset, x, y;
    u32 segment = (u16)D_801241E8[playerIndex].unk18;

    if (segment < 2 || (u32)(segment - 10) < 10 ||
        (u32)(segment - 41) < 9 || (u32)(segment - 125) < 4 ||
        (u32)(segment - 249) < 5 || (u32)(segment - 79) < 12 ||
        (s16)segment == 254 || (u32)(segment - 311) < 7) {
        D_8013B22C = 0xE90;
        D_8013B22E = 0xF60;
        drawBackgroundLensFlare(playerIndex);
    }

    if (!D_8013AFD3) {
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

    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    y = D_801241E8[playerIndex].pitch / 4 - 64;
    /* The bottom edge of the last strip uses the halfword following the offsets. */
    for (column = 0; column < 3; column++, x += 256) {
        strip = offset = 0;
        for (strip = 0; strip < 12; strip++, offset += 8) {
            addTexturedQuad(&D_8013AF6C[4094],
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
    addFlatRectangle(&D_8013AF6C[4095], -160, -120, 320, (s16)(y + 120), 72, 184, 248);
    addFlatRectangle(&D_8013AF6C[4095], -160, (s16)y, 320, (s16)(120 - y), 232, 208, 144);
    SetFarColor(112, 100, 80);
    SetFogNearFar(8000, 10000, 400);
}

void drawLapAnimatedBackgroundAndSetFog(s16 playerIndex) {
    AnimatedBackgroundScratchpad *scratch = (AnimatedBackgroundScratchpad *)getScratchAddr(0);
    GsOT *ot;

    if (playerIndex == 0) {
        ot = &D_800EC040[D_8013AF78];
    } else {
        ot = &D_800EC0E0[D_8013AF78];
    }
    addCopiedPrimitive(&D_8013AF6C[2048], (u32 *)&gLapAnimatedBackgroundTile);
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
            drawProjectedModelPrimitives(D_80125044[0], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((-gLapAnimatedBackgroundFrame << 6) & 0xFC0, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(D_80125044[1], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((gLapAnimatedBackgroundFrame << 6) & 0xFC0, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(D_80125044[2], ot, 2, 1);
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
            drawProjectedModelPrimitives(D_80125044[0], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((-gLapAnimatedBackgroundFrame << 9) & 0xE00, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(D_80125044[1], ot, 2, 1);

            GsInitCoordinate2(&scratch->root, &scratch->child);
            RotMatrixY((gLapAnimatedBackgroundFrame << 9) & 0xE00, &scratch->child.coord);
            GsGetLs(&scratch->child, &scratch->localScreen);
            ScaleMatrix(&scratch->localScreen, (VECTOR *)&gRipplingBackgroundStripData[12]);
            GsSetLsMatrix(&scratch->localScreen);
            drawProjectedModelPrimitives(D_80125044[2], ot, 2, 1);
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
    y = D_801241E8[playerIndex].pitch / 2;
    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    if (y > 240) y = 240;
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, x, y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)(x + 256), y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)(x + 512), y, 256, 64, 0, 0, 128, 128, 128);
    if (y >= -119) {
        addFlatRectangle(&D_8013AF6C[1024], -160, -120, 320, (s16)(y + 120), 0, 168, 248);
    }
    if (y < -120) y = -184;
    addFlatRectangle(&D_8013AF6C[1024], -160, (s16)(y + 64), 320, (s16)(120 - y), 248, 248, 248);
    SetFarColor(248, 248, 248);
    SetFogNearFar(8000, 10000, 400);
}

void drawCameraCulledBackgroundAndSetFog(s16 playerIndex) {
    GsCOORDINATE2 *coordinate = (GsCOORDINATE2 *)getScratchAddr(0);
    MATRIX *matrix = (MATRIX *)getScratchAddr(20);
    GsOT *ot;
    s16 x, y, horizon;

    if (playerIndex == 0) {
        ot = &D_800EC090[D_8013AF78];
    } else {
        ot = &D_800EC130[D_8013AF78];
    }
    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    y = D_801241E8[playerIndex].pitch / 4;
    horizon = y;
    addTexturedSprite(&D_8013AF6C[512], 26, 0x7C29, x, y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[512], 26, 0x7C29, (s16)(x + 256), y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[512], 26, 0x7C29, (s16)(x + 512), y, 256, 64, 0, 0, 128, 128, 128);
    if (horizon < -120) horizon = -120;
    if (horizon > 120) horizon = 120;
    addFlatRectangle(&D_8013AF6C[1024], -160, -120, 320, (s16)(horizon + 120), 112, 168, 248);
    addFlatRectangle(&D_8013AF6C[1024], -160, horizon, 320, (s16)(120 - horizon), 200, 216, 240);
    GsInitCoordinate2(0, coordinate);
    GsGetLs(coordinate, matrix);
    ScaleMatrix(matrix, &D_80124BDC);
    GsSetLsMatrix(matrix);
    if (isPositionInCameraRange(0xADA, 0xCD1, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[0], ot, 2, 1);
    }
    if (isPositionInCameraRange(0xA61, 0x18D4, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[1], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x24E0, 0x1F11, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[2], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x47E0, 0x2B52, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[3], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x6356, 0x32B7, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[4], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x6B5E, 0x5C1C, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[5], ot, 2, 1);
    }
    if (isPositionInCameraRange(0x6B5E, 0x5C1C, (s16)func_80059254())) {
        drawProjectedModelPrimitives(D_80125044[6], ot, 2, 1);
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
        ot = &D_800EC040[D_8013AF78];
    } else {
        ot = &D_800EC0E0[D_8013AF78];
    }
    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    y = D_801241E8[playerIndex].pitch / 2;
    horizon = y;
    addTexturedSprite(&D_8013AF6C[512], 26, 0x7C2A, x, y, 256, 64, 0, 128, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[512], 26, 0x7C2A, (s16)(x + 256), y, 256, 64, 0, 128, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[512], 26, 0x7C2A, (s16)(x + 512), y, 256, 64, 0, 128, 128, 128, 128);
    if (horizon < -120) horizon = -120;
    if (horizon > 120) horizon = 120;
    addFlatRectangle(&D_8013AF6C[4095], -160, -120, 320, (s16)(horizon + 120), 0, 120, 240);
    addFlatRectangle(&D_8013AF6C[4095], -160, horizon, 320, (s16)(120 - horizon), 255, 255, 255);
    GsInitCoordinate2(0, coordinate);
    GsGetLs(coordinate, matrix);
    ScaleMatrix(matrix, &D_80124BDC);
    GsSetLsMatrix(matrix);
    drawProjectedModelPrimitives(D_80125044[0], ot, 2, 1);
    drawProjectedModelPrimitives(D_80125044[1], ot, 2, 1);
    drawProjectedModelPrimitives(D_80125044[2], ot, 2, 1);
    SetFarColor(110, 110, 128);
    SetFogNearFar(8000, 10000, 400);
}

void drawCourseBackgroundForView(s16 view) {
    sCourseBackgroundCallbacks[D_8013AFCE](view);
}

s32 isPositionInCameraRange(s32 x, s32 z, s16 playerIndex) {
    s32 centerX, centerZ;
    s32 scaledX, scaledZ;
    centerX = gPlayerCameraViews[playerIndex].vpx / 256 +
        ((D_8013B660 / 2) * rsin(D_801241E8[playerIndex].yaw)) / 4096;
    centerZ = gPlayerCameraViews[playerIndex].vpz / 256 +
        ((D_8013B660 / 2) * rcos(D_801241E8[playerIndex].yaw)) / 4096;
    scaledX = x / 256;
    scaledZ = z / 256;
    return SquareRoot0((centerX - scaledX) * (centerX - scaledX) +
        (centerZ - scaledZ) * (centerZ - scaledZ)) < D_8013B660;
}

s32 isPointWithinCameraForwardRange(s32 x, s32 z, s16 player) {
    s32 cameraX, cameraZ;
    s32 scaledX, scaledZ;
    cameraX = gPlayerCameraViews[player].vpx / 256 + ((D_8013B660 / 2) * rsin(D_801241E8[player].yaw)) / 4096;
    cameraZ = gPlayerCameraViews[player].vpz / 256 + ((D_8013B660 / 2) * rcos(D_801241E8[player].yaw)) / 4096;
    scaledX = x / 256;
    scaledZ = z / 256;
    return SquareRoot0((cameraX - scaledX) * (cameraX - scaledX) + (cameraZ - scaledZ) * (cameraZ - scaledZ)) < (D_8013B660 * 3) / 4;
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

/* DECOMP_ATTEMPT: inline 24/26 */
#ifdef NON_MATCHING
extern RECT D_8013B248 __attribute__((section(".sdata")));

void func_8005162C(s32 x, s32 y, u32 r, u32 g, u32 b, u32 stp) {
    u16 color;
    u32 rgb = (r & 0xF8) >> 3;
    u32 alpha = stp & 1;
    u32 green = (g & 0xF8) << 2;
    u32 blue;
    D_8013B248.x = x;
    D_8013B248.y = y;
    rgb |= green;
    blue = (b & 0xF8) << 7;
    rgb |= blue;
    alpha <<= 15;
    color = rgb | alpha;
    LoadImage(&D_8013B248, (u32 *)&color);
}
#else
INCLUDE_ASM("asm/nonmatchings/graphics/course_rendering", func_8005162C);
#endif

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

/* DECOMP_ATTEMPT: inline 18/28 */
#ifdef NON_MATCHING
void func_80051700(void) {
    s32 i;
    for (i = 0; i < 0x40; i++) {
        func_8005162C((s16)(i + 704), 0x90, 0xE0, 0xE0, 0xE8, 0);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/graphics/course_rendering", func_80051700);
#endif

void queueScanlineCopyPackets(void) {
    DR_MOVE *move;
    DR_STP *stp;
    s32 y, i, depth;

    move = (DR_MOVE *)GsOUT_PACKET_P;
    for (y = 256; y < 512; y += 16) {
        SetDrawMove(move, &gScanlineSeedRect, 640, y);
        addPrim(&D_8013AF6C[4095], move);
        move += 2;
    }
    GsOUT_PACKET_P = (u32)move;
    if (selectedCameraOverlayModeGp) {
        stp = (DR_STP *)GsOUT_PACKET_P;
        for (i = 0; i < 15; i++) {
            depth = (i * 200) / 15 + 150;
            SetDrawStp(stp, 1);
            addPrim(&D_8013AF6C[depth], stp);
            stp++;
        }
        GsOUT_PACKET_P = (u32)stp;
    }
    move = (DR_MOVE *)GsOUT_PACKET_P;
    for (i = 0; i < 15; i++) {
        y = 256;
        gScanlineCopyRect.y = i + 257;
        depth = (i * 200) / 15 + 150;
        for (; y < 512; y += 16) {
            SetDrawMove(move, &gScanlineCopyRect, 640, y);
            addPrim(&D_8013AF6C[depth], move);
            gScanlineCopyRect.y += 16;
            move++;
        }
    }
    GsOUT_PACKET_P = (u32)move;
    if (selectedCameraOverlayModeGp) {
        stp = (DR_STP *)GsOUT_PACKET_P;
        for (i = 0; i < 15; i++) {
            depth = (i * 200) / 15 + 150;
            SetDrawStp(stp, 0);
            addPrim(&D_8013AF6C[depth], stp);
            stp++;
        }
        GsOUT_PACKET_P = (u32)stp;
    }
}

/* DECOMP_ATTEMPT: inline 161/181 */
#ifdef NON_MATCHING
extern Position3D *D_801250EC[];
extern u32 D_8013B658, D_8013B65C;
extern s32 D_80124BEC[], D_80124C14[];
extern void func_80051700(void);

void func_80051A68(void) {
    u32 *model;
    register u32 *offset;
    s16 i;
    Position3D *positions;
    bindGeometryRecordData((s16 *)0x801C0000);
    positions = D_8013B280 = D_801250EC[D_8013AFCE];
    for (i = 0; i < 100; i++) {
        positions[i].x *= 5;
        positions[i].y *= 5;
        positions[i].z *= 5;
    }
    offset = (u32 *)0x801C8804;
    D_8013B654[0] = *(u16 *)0x801C8800;
    for (i = 0; i < D_8013B654[0]; i++) {
        model = (u32 *)0x801C8804;
        model = (u32 *)((u32)model + *offset);
        GsMapModelingData(model);
        model++;
        D_8013B658 = *model++;
        D_8013BBF8[i] = (TmdObject *)model;
        offset++;
        D_8013B280[i].x /= 256;
        D_8013B280[i].y /= 256;
        D_8013B280[i].z /= 256;
    }
    GsInitCoordinate2(0, &D_80124FF4);
    D_8013B660 = D_80124BEC[D_8013AFCE] / 256;
    D_8013B664 = D_80124C14[D_8013AFCE] / 256;
    if ((u16)gCourseOptionalResourceSectors[D_8013AFCE].sector != 0) {
        offset = (u32 *)0x801F0804;
        D_8013B654[1] = *(u16 *)0x801F0800;
        for (i = 0; i < D_8013B654[1]; i++) {
            model = (u32 *)(*offset++ + 0x801F0800);
            model++;
            GsMapModelingData(model);
            model++;
            D_8013B65C = *model++;
            D_80125044[i] = (TmdObject *)model;
        }
    }
    if (D_8013AFCE == 6) func_80051700();
}
#else
INCLUDE_ASM("asm/nonmatchings/graphics/course_rendering", func_80051A68);
#endif

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
        ot = &D_800EC040[D_8013AF78];
    } else {
        ot = &D_800EC0E0[D_8013AF78];
    }
    scale = gCourseModelScale;
    if (D_8013AFB8 != 0) {
        return;
    }
    drawCourseBackgroundForView(view);
    if (D_8013AFCE == 6) {
        queueScanlineCopyPackets();
    }
    cameras = D_801241E8;
    camera = &cameras[view];
    sine = rsin(camera->yaw);
    views = gPlayerCameraViews;
    cameraView = &views[view];
    centerX = cameraView->vpx / 256 + ((D_8013B660 / 2) * sine) / 4096;
    cosine = rcos(camera->yaw);
    centerZ = cameraView->vpz / 256 + ((D_8013B660 / 2) * cosine) / 4096;
    cameraX = cameraView->vpx / 256;
    cameraZ = cameraView->vpz / 256;

    GsGetLs(&D_80124FF4, matrix);
    ScaleMatrix(matrix, &scale);
    GsSetLsMatrix(matrix);

    if (D_8013AFCE == 6) {
        flags = 1;
    } else {
        flags = 3;
    }

    i = 0;
    models = D_8013BBF8;
    do {
        const Position3D *positions;
        s32 x, z, dx, dz;

        if (i >= D_8013B654[0]) {
            goto queue_draw_mode;
        }
        positions = D_8013B280;
        x = positions[i].x;
        z = positions[i].z;
        dx = centerX - x;
        dz = centerZ - z;
        if (SquareRoot0(dx * dx + dz * dz) <= D_8013B660) {
            s32 dx = cameraX - x;
            s32 dz = cameraZ - z;
            if (SquareRoot0(dx * dx + dz * dz) >= D_8013B664) {
                drawProjectedModelPrimitives(models[i], ot, 2, flags);
            } else {
                func_800559F4(models[i], ot, 2, flags);
            }
        }
        i++;
    } while (1);

queue_draw_mode:
    SetDrawMode(&gCourseModelDrawMode, 0, 1, 0, &gCourseModelTextureWindow);
    addPrim(&D_8013AF6C[4095], &gCourseModelDrawMode);
    if (selectedCameraOverlayModeGp) {
        queueSelectedCameraOverlayPackets();
    }
}

void queueSelectedCameraOverlayPackets(void) {
    DR_STP *packet;
    switch (selectedCameraOverlayModeGp) {
    case 1:
        if (D_8013AF78) {
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer1[0]);
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer1[1]);
        } else {
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer0[0]);
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer0[1]);
        }
        break;
    case 2:
        if (D_8013AF78) {
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer1[1]);
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer1[0]);
        } else {
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer0[1]);
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer0[0]);
        }
        break;
    }
    packet = (DR_STP *)GsOUT_PACKET_P;
    SetDrawStp(packet, 1);
    addPrim(&D_8013AF6C[4095], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void queueOffsetScreenCopySprites(void) {
    DR_STP *packet;
    if (D_8013AF78) {
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer1[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer1[1]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer1[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer1[1]);
    } else {
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer0[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer0[1]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer0[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer0[1]);
    }
    packet = (DR_STP *)GsOUT_PACKET_P;
    SetDrawStp(packet, 1);
    addPrim(&D_8013AF6C[4095], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawNoProcessMessage(void) {
    drawDebugText(0, 30, gNoProcessMessage);
}
