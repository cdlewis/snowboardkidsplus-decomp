#include "../graphics/render_effects_internal.h"

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

RECT gReadPixelRect __attribute__((section(".data.read_pixel_rect"))) = {0, 0, 1, 1};

RECT gScanlineCopyRect __attribute__((section(".data.scanline_copy_rects"))) = {640, 256, 64, 1};

RECT gScanlineSeedRect __attribute__((section(".data.scanline_copy_rects"))) = {704, 144, 64, 1};

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004D330);

void adjustDebugPositionAndDisplay(VECTOR *position) {
    if (D_8013AEEC[1] & 0x1000) position->vx += 32;
    if (D_8013AEEC[1] & 0x4000) position->vx -= 32;
    if (D_8013AEEC[1] & 0x8000) position->vz += 32;
    if (D_8013AEEC[1] & 0x2000) position->vz -= 32;
    if (D_8013AEEC[1] & 8) position->vy += 32;
    if (D_8013AEEC[1] & 2) position->vy -= 32;
    func_8006B4A0(0, 10, debugPositionXFormatAbsolute, position->vx);
    func_8006B4A0(0, 11, debugPositionYFormatAbsolute, position->vy);
    func_8006B4A0(0, 12, debugPositionZFormatAbsolute, position->vz);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004D93C);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004E09C);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004E718);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004EBF8);

void drawCyanBackgroundAndSetFog(void) {
    addFlatRectangle(&D_8013AF6C[4095], -160, -120, 320, 240, 0, 235, 230);
    SetFarColor(80, 128, 104);
    SetFogNearFar(5500, 10000, 400);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004EFE0);

void createOscillatingQuadTask(s32 value, VECTOR *position, s32 amplitude, s32 r, s32 g, s32 b) {
    OscillatingQuadResource *resource;
    func_80014930(&gGameTasks[4], func_8004EFE0);
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
    resource->x = position->vx;
    resource->y = position->vy;
    resource->z = position->vz;
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
            func_800149A8();
        }
    }
}

void createMovingLineTask(s32 index) {
    MovingLineResource *resource;
    func_80014930(&gGameTasks[4], drawMovingLineTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = (MovingLineResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = randomNextLinear() % 320;
    resource->y = 0;
    resource->elapsed = 0;
    resource->duration = randomNextLinear() % 15 + 15;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004F780);

void createFlickeringDotsTask(s32 playerIndex) {
    FlickeringDotsResource *resource;
    s32 i;
    func_80014930(&gGameTasks[4], func_8004F780);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = playerIndex;
    resource = (FlickeringDotsResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 25; i++) {
        resource->positions[i].x = randomNextLinear() % 320;
        resource->positions[i].y = randomNextLinear() % 120;
    }
    resource->frame = 0;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8004FB70);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80050080);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80050504);

void drawSnowBackgroundAndSetFog(s16 playerIndex) {
    s16 y;
    s16 x;
    y = D_801241E8[playerIndex].pitch / 2;
    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    if (y > 240) y = 240;
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)x, (s16)y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)(x + 256), (s16)y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)(x + 512), (s16)y, 256, 64, 0, 0, 128, 128, 128);
    if ((s16)y >= -119) {
        addFlatRectangle(&D_8013AF6C[1024], -160, -120, 320, (s16)(y + 120), 0, 168, 248);
    }
    if ((s16)y < -120) y = -184;
    addFlatRectangle(&D_8013AF6C[1024], -160, (s16)(y + 64), 320, (s16)(120 - y), 248, 248, 248);
    SetFarColor(248, 248, 248);
    SetFogNearFar(8000, 10000, 400);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80050B18);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80050F84);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_800512A4);

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
    readPixelRectGp.x = x;
    readPixelRectGp.y = y;
    StoreImage(&gReadPixelRect, (u32 *)&pixel);
    out->r = pixel << 3;
    out->g = (pixel >> 2) & 0xF8;
    out->b = (pixel >> 7) & 0xF8;
    out->stp = pixel >> 15;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_8005162C);

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

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80051700);

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
    if (selectedCameraStateGp) {
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
        scanlineCopyRectGp.y = i + 257;
        depth = (i * 200) / 15 + 150;
        for (; y < 512; y += 16) {
            SetDrawMove(move, &gScanlineCopyRect, 640, y);
            addPrim(&D_8013AF6C[depth], move);
            scanlineCopyRectGp.y += 16;
            move++;
        }
    }
    GsOUT_PACKET_P = (u32)move;
    if (selectedCameraStateGp) {
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

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80051A68);

INCLUDE_ASM("asm/nonmatchings/unclassified/graphics_8004D330", func_80051D3C);

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
    func_80049D48(0, 30, gNoProcessMessage);
}
