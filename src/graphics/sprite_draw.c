#include "render_effects_internal.h"

u8 gUppercaseGlyphMetrics[52] __attribute__((section(".data.uppercase_glyph_metrics"))) = {
    0xC0, 0x08, 0xC8, 0x08, 0xD0, 0x08, 0xD8, 0x08,
    0xE0, 0x07, 0xE8, 0x07, 0xF0, 0x08, 0xC0, 0x08,
    0xC8, 0x04, 0xCC, 0x06, 0xD4, 0x08, 0xDC, 0x07,
    0xE4, 0x0A, 0xF0, 0x08, 0x80, 0x08, 0x88, 0x08,
    0x90, 0x09, 0x9C, 0x08, 0xA4, 0x07, 0xAC, 0x08,
    0xB4, 0x08, 0xBC, 0x09, 0xC8, 0x0A, 0xD4, 0x09,
    0xE0, 0x0A, 0xEC, 0x08,
};

void initPrimitiveTemplates(void) {
    initTexturedQuadTemplate();
    initSpritePrimitiveTemplate();
}

void initTexturedQuadTemplate(void) {
    SetPolyFT4((P_TAG *)&gTexturedQuadTemplate);
    setTexturedQuadTexture(0, 0);
    setRGB0(&gTexturedQuadTemplate, 0x80, 0x80, 0x80);
}

void setTexturedQuadTexture(s32 index, s32 palette) {
    TextureRegionSet *texture = &D_8012DDA4[index];
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    quad->tpage = GetTPage(texture->depth, 1, texture->x, texture->y);
    quad->clut = GetClut(texture->clutX, texture->clutY + palette);
    D_8013B6AC = texture->regions;
}

void setTexturedQuadColor(s32 r, s32 g, s32 b) {
    setRGB0(&gTexturedQuadTemplate, r, g, b);
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006BEEC);

void drawTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region = &D_8013B6AC[index];
    quad->x0 = x;
    quad->y0 = y;
    quad->x1 = x + region->w;
    quad->y1 = y;
    quad->x2 = x;
    quad->y2 = y + region->h;
    quad->x3 = x + region->w;
    quad->y3 = y + region->h;
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = region->u + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = region->v + region->h;
    quad->u3 = region->u + region->w;
    quad->v3 = region->v + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawStretchedTexturedQuad(s16 x, s16 y, s16 width, s16 height, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region;
    s16 right;
    quad->y0 = y;
    quad->y1 = y;
    y += height;
    quad->y2 = y;
    quad->y3 = y;
    right = x + width;
    quad->x0 = x;
    quad->x1 = right;
    quad->x2 = x;
    quad->x3 = right;
    region = &D_8013B6AC[index];
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = region->u + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = region->v + region->h;
    quad->u3 = region->u + region->w;
    quad->v3 = region->v + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame = &D_8012DFF4[index];

    setTexturedQuadTexture(frame->textureIndex, 0);
    drawTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

void drawTexturedQuadByIndexWithBlend(s16 x, s16 y, s32 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame;
    s32 semiTrans;
    s32 blend;

    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    frame = &D_8012DFF4[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    func_8006BEEC(semiTrans);
    drawTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

void drawStretchedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame;
    TextureRegion *region;
    s32 semiTrans;
    s32 blend;
    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    frame = &D_8012DFF4[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    func_8006BEEC(semiTrans);
    region = &D_8013B6AC[frame->frameIndex];
    if (width == 0) width = region->w;
    if (height == 0) height = region->h;
    drawStretchedTexturedQuad(x, y, width, height, frame->frameIndex, ot, depth);
}

void drawTexturedQuadList(s16 x, s16 y, TexturedQuadListEntry *entry, s16 blend, GsOT *ot, u16 depth) {
    while (entry->palette < 255) {
        drawStretchedTexturedQuadByIndexWithBlend(x + entry->x, y + entry->y, entry->width, entry->height,
                     entry->quadIndex, entry->palette, blend, ot, depth);
        entry++;
    }
}

void drawSkewedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 unused, s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame;
    TextureRegion *region;
    POLY_FT4 *quad;
    s32 semiTrans;
    s32 blend;
    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    frame = &D_8012DFF4[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    func_8006BEEC(semiTrans);
    region = &D_8013B6AC[frame->frameIndex];
    if (width == 0) width = region->w;
    if (height == 0) height = region->h;
    quad = &gTexturedQuadTemplate;
    /* Start with a rectangle, then skew its corners by eight pixels. */
    setXY0(quad, x, y);
    quad->x1 = x + width;
    quad->y1 = y;
    quad->x2 = x;
    quad->y2 = y + height;
    quad->x3 = x + width;
    quad->y3 = y + height;
    quad->x0 = x - 8;
    quad->y0 = y + 8;
    quad->x1 = x + width - 8;
    quad->y1 = y - 8;
    quad->x2 = x + 8;
    quad->y2 = y + height + 8;
    quad->x3 = x + width + 8;
    quad->y3 = y + height - 8;
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = region->u + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = region->v + region->h;
    quad->u3 = region->u + region->w;
    quad->v3 = region->v + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawInclusiveTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region = &D_8013B6AC[index];
    u16 extent;

    /* Inclusive edges use a wrapping 16-bit decrement of each dimension. */
    quad->x0 = x;
    quad->y0 = y;
    extent = region->w + (u16)-1;
    quad->x1 = x + extent;
    quad->y1 = y;
    quad->x2 = x;
    extent = region->h + (u16)-1;
    quad->y2 = y + extent;
    extent = region->w + (u16)-1;
    quad->x3 = x + extent;
    extent = region->h + (u16)-1;
    quad->y3 = y + extent;
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = (u8)(region->u - 1) + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = (u8)(region->v - 1) + region->h;
    quad->u3 = (u8)(region->u - 1) + region->w;
    quad->v3 = (u8)(region->v - 1) + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawInclusiveTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame = &D_8012DFF4[index];

    setTexturedQuadTexture(frame->textureIndex, 0);
    drawInclusiveTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006C798);

void initSpritePrimitiveTemplate(void) {
    SPRT *sprite = &gSpritePrimitiveTemplate.sprite;
    SetSprt((P_TAG *)sprite);
    SetDrawTPage(&gSpritePrimitiveTemplate.drawPage, 1, 1, GetTPage(0, 0, 0, 0));
    setRGB0(sprite, 0x80, 0x80, 0x80);
}

void setSpriteTextureBlend(s32 index, s32 palette, s32 blend) {
    TextureRegionSet *texture = &D_8012DDA4[index];
    SpritePrimitiveTemplate *sprite = &gSpritePrimitiveTemplate;
    SetDrawTPage(&sprite->drawPage, 1, 1,
                 GetTPage(texture->depth, blend, texture->x, texture->y));
    sprite->sprite.clut = GetClut(texture->clutX, texture->clutY + palette);
    D_8013B6AC = texture->regions;
}

void setSpriteColor(s32 r, s32 g, s32 b) {
    setRGB0(&gSpriteTemplate, r, g, b);
    setRGB0(&gTexturedSpriteTemplate, r, g, b);
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006C970);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006C998);

void drawSpriteTextureRegion(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    SPRT *sprite = (SPRT *)GsGetWorkBase();
    DR_TPAGE *page;
    TextureRegion *region;
    *sprite = gSpritePrimitiveTemplate.sprite;
    region = &D_8013B6AC[index];
    sprite->x0 = x;
    sprite->y0 = y;
    sprite->u0 = region->u;
    sprite->v0 = region->v;
    sprite->w = region->w;
    sprite->h = region->h;
    AddPrim(&ot->org[depth], sprite++);
    page = (DR_TPAGE *)sprite;
    *page = gSpritePrimitiveTemplate.drawPage;
    AddPrim(&ot->org[depth], page);
    func_800BF2C8((u32)(page + 1));
}

void addSpritePacket(SPRT *source, GsOT *ot, u16 depth) {
    SPRT *packet = (SPRT *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

void addDrawPagePacket(DR_TPAGE *source, GsOT *ot, u16 depth) {
    DR_TPAGE *packet = (DR_TPAGE *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

void drawRotatedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex, s16 blendMode, s16 angle, GsOT *ot, u16 depth) {
    s32 palette = paletteIndex;
    s32 blend;
    s32 transparent;
    TexturedQuadFrame *frame;
    TextureRegionSet *texture;
    TextureRegion *region;
    GsSPRITE *sprite;
    if (blendMode >= 0) {
        blend = blendMode;
        transparent = 1;
    } else {
        blend = 0;
        transparent = 0;
    }
    frame = &D_8012DFF4[index];
    texture = &D_8012DDA4[frame->textureIndex];
    region = &texture->regions[frame->frameIndex];
    sprite = &gRotatedSpriteTemplate;
    sprite->attribute = 0;
    if (transparent == 1) {
        sprite->attribute = 0x40000000;
        sprite->attribute |= blend << 28;
    }
    sprite->x = x;
    sprite->y = y;
    sprite->w = region->w;
    sprite->h = region->h;
    sprite->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    sprite->u = region->u;
    sprite->v = region->v;
    sprite->cx = texture->clutX;
    sprite->cy = texture->clutY + palette;
    sprite->r = gTexturedSpriteTemplate.r0;
    sprite->g = gTexturedSpriteTemplate.g0;
    sprite->b = gTexturedSpriteTemplate.b0;
    sprite->mx = region->w / 2;
    sprite->my = region->h / 2;
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
    sprite->rotate = -(angle << 12);
    GsSortSprite(sprite, ot, depth);
}

void drawTransformedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex, s16 blendMode, s16 pivotX, s16 pivotY, s16 scaleX, s16 scaleY, s32 rotation, GsOT *ot, u16 depth) {
    s32 palette = paletteIndex;
    s32 blend;
    s32 transparent;
    TexturedQuadFrame *frame;
    TextureRegionSet *texture;
    TextureRegion *region;
    GsSPRITE *sprite;
    if (blendMode >= 0) {
        blend = blendMode;
        transparent = 1;
    } else {
        blend = 0;
        transparent = 0;
    }
    sprite = &gRotatedSpriteTemplate;
    frame = &D_8012DFF4[index];
    texture = &D_8012DDA4[frame->textureIndex];
    region = &texture->regions[frame->frameIndex];
    sprite->attribute = 0;
    sprite->attribute |= texture->depth << 24;
    if (transparent == 1) {
        sprite->attribute |= 0x40000000;
        sprite->attribute |= blend << 28;
    }
    sprite->x = x;
    sprite->y = y;
    sprite->w = region->w;
    sprite->h = region->h;
    sprite->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    sprite->u = region->u;
    sprite->v = region->v;
    sprite->cx = texture->clutX;
    sprite->cy = texture->clutY + palette;
    sprite->r = gTexturedSpriteTemplate.r0;
    sprite->g = gTexturedSpriteTemplate.g0;
    sprite->b = gTexturedSpriteTemplate.b0;
    if (pivotX == 0x7FFF) {
        sprite->mx = region->w / 2;
    } else {
        sprite->mx = pivotX;
    }
    if (pivotY == 0x7FFF) {
        sprite->my = region->h / 2;
    } else {
        sprite->my = pivotY;
    }
    sprite->scalex = scaleX;
    sprite->scaley = scaleY;
    sprite->rotate = rotation;
    GsSortSprite(sprite, ot, depth);
}

void drawWrappedText8x8(s16 x, s16 y, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 currentX = x;
    s16 currentY = y;
    s16 count = 0;
    s16 defaultPalette = palette;
    while (*(const s8 *)text) {
        if (count >= columns) {
            currentX = x;
            currentY += 8;
            count = 0;
        }
        if ((s8)*text == '\n') {
            count = columns;
            text++;
        } else {
            if ((s8)*text == 'c') {
                if ((u32)((u8)text[1] - '0') < 10) {
                    palette = (s8)text[1] - '0';
                } else {
                    palette = defaultPalette;
                }
            } else {
                func_8006D29C(currentX, currentY, 8, 8, text, palette, blend, ot, depth);
                currentX += 8;
                count++;
            }
            text += 2;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006D29C);

void drawLimitedText8x8(s16 x, s16 y, s16 limit, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 count = 0;
    s16 startY = y;
    while (*(const s8 *)text && count < limit) {
        if ((s8)*text == '\n') {
            count = limit;
        } else {
            drawSizedTextGlyph(x, startY, 8, 8, text, palette, blend, ot, depth);
            x += 8;
            count++;
        }
        text++;
    }
}

void drawSizedTextGlyph(s16 x, s16 y, s16 width, s16 height, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s16 u, v;
    s32 blendMode;
    SPRT *sprite;
    if (blend >= 0) {
        blendMode = blend;
        semiTrans = 1;
    } else {
        blendMode = 0;
        semiTrans = 0;
    }
    u = (((s8)*text - 32) % 32) * 8;
    v = (((s8)*text - 32) / 32) * 8;
    if (u >= 0 && v >= 0) {
        setSpriteTextureBlend(0, palette, blendMode);
        func_8006C970(semiTrans);
        sprite = &gSpriteTemplate;
        sprite->x0 = x;
        sprite->y0 = y;
        sprite->u0 = u;
        sprite->v0 = v;
        sprite->w = width;
        sprite->h = height;
        addSpritePacket(sprite, ot, depth);
        addDrawPagePacket(&gSpritePrimitiveTemplate.drawPage, ot, depth);
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006D66C);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006D984);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006DC94);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006DFB4);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006E2C4);

void drawHalfStepSpriteBar(s16 x, s16 y, s16 count, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s32 blendMode;
    SpritePrimitiveTemplate *template;
    SPRT *packet;
    DR_TPAGE *page;
    s32 i;
    if (count > 0) {
        if (blend >= 0) {
            blendMode = blend;
            semiTrans = 1;
        } else {
            blendMode = 0;
            semiTrans = 0;
        }
        template = &gSpritePrimitiveTemplate;
        SetDrawTPage(&template->drawPage, 1, 1, GetTPage(0, blendMode, 0x340, 0x100));
        template->sprite.clut = GetClut(0x320, palette + 0xF0);
        SetSemiTrans(&template->sprite, semiTrans);
        template->sprite.w = 8;
        template->sprite.h = 8;
        template->sprite.x0 = x;
        template->sprite.y0 = y;
        template->sprite.u0 = 0x70;
        template->sprite.v0 = 0;
        packet = (SPRT *)GsGetWorkBase();
        for (i = 0; i < count / 2; i++) {
            *packet = template->sprite;
            AddPrim(&ot->org[depth], packet++);
            template->sprite.x0 += 8;
        }
        if ((s16)(count % 2) > 0) {
            *packet = template->sprite;
            packet->u0 = 0x78;
            AddPrim(&ot->org[depth], packet);
            packet++;
        }
        page = (DR_TPAGE *)packet;
        *page = gSpritePrimitiveTemplate.drawPage;
        AddPrim(&ot->org[depth], page);
        func_800BF2C8((u32)(page + 1));
    }
}

void drawPackedTime(s16 x, s16 y, u32 value, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 field;
    value >>= 8;
    field = value & 255;
    func_8006DFB4(x + 48, y, 2, 1, field, palette, blend, ot, depth);
    value >>= 8;
    func_8006C998(x + 40, y, 24, palette, blend, ot, depth);
    field = value & 255;
    func_8006DFB4(x + 24, y, 2, 1, field, 1, -1, ot, depth);
    func_8006C998(x + 16, y, 25, palette, blend, ot, depth);
    value >>= 8;
    func_8006DFB4(x, y, 2, 1, value, 1, -1, ot, depth);
}

void drawWrappedDoubleByteText(s16 x, s16 y, s16 columns, s8 *text, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 startX = x;
    s16 savedColumns = columns;
    s16 column;
    s16 currentPalette = palette;
    s16 originalPalette = palette;
    column = 0;
    while (*text) {
        if (column >= columns) {
            x = startX;
            y += 16;
            column = 0;
        }
        if (*text == '\n') {
            column = savedColumns;
            text++;
            continue;
        }
        if (*text == 'c') {
            if ((u32)((u8)text[1] - '0') < 10) {
                currentPalette = text[1] - '0';
            } else {
                currentPalette = originalPalette;
            }
        } else {
            func_8006EB00(x, y, 8, 16, text, currentPalette, blend, ot, depth);
            x += 8;
            column++;
        }
        text += 2;
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006EB00);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006EC84);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006EF30);

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006F248);

void drawUppercaseProportionalText(s16 x, s16 y, s16 count, const char *text, s16 palette,
                   s16 blend, GsOT *ot, u16 depth) {
    DrawModeSpritePacket *packet = (DrawModeSpritePacket *)0x1F800000;
    s16 i;
    s8 glyph;
    packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
    packet->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    packet->code = TEXTURED_SPRITE_CODE;
    setRGB0(packet, gTexturedSpriteTemplate.r0, gTexturedSpriteTemplate.g0, gTexturedSpriteTemplate.b0);
    packet->tpage = getTPage(0, 0, 832, 256);
    if (blend >= 0) {
        packet->code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
        packet->tpage = (blend << 5) | getTPage(0, 0, 832, 256);
    }
    packet->clut = getClut(800, palette + 240);
    packet->x0 = x;
    packet->y0 = y;
    packet->h = 16;
    for (i = 0; *(const s8 *)text; i++, text++) {
        if (i >= count) {
            break;
        }
        if ((u8)(*text - 'A') < 26) {
            glyph = *text - 'A';
            packet->u0 = gUppercaseGlyphMetrics[glyph * 2];
            if (*(const s8 *)text >= 'O') {
                packet->v0 = 0x90;
            } else if (*(const s8 *)text >= 'H') {
                packet->v0 = 0x80;
            } else {
                packet->v0 = 0x70;
            }
            packet->w = gUppercaseGlyphMetrics[glyph * 2 + 1];
            addTexturedSpritePacket((TexturedSpritePacket *)packet, ot, depth);
            packet->x0 += packet->w;
        } else {
            packet->x0 += 8;
        }
    }
}

void drawWrappedText16x16(s16 x, s16 y, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 count = 0;
    s16 startX = x;
    while (*(const s8 *)text) {
        if ((s8)*text == 'c') {
            if ((u32)((u8)text[1] - '0') < 10) {
                palette = (s8)text[1] - '0' + 8;
            }
            text += 2;
            if (!*(const s8 *)text) break;
        }
        if (count >= columns) {
            x = startX;
            y += 16;
            count = 0;
        }
        func_8006F8A4(x, y, 16, 16, text, palette, blend, ot, depth);
        x += 16;
        text += 2;
        count++;
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006F8A4);

void drawLimitedWrappedText16x16(s16 x, s16 y, s16 limit, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 count = 0;
    s16 total = 0;
    s16 startX = x;
    while (*(const s8 *)text && total < limit) {
        if ((s8)text[0] == '\r' && (s8)text[1] == '\n') {
            x = startX;
            y += 16;
            count = 0;
            text += 2;
            continue;
        } else if ((s8)text[0] == 'c') {
            if ((u32)((u8)text[1] - '0') < 10) {
                palette = (s8)text[1] - '0' + 8;
            }
            text += 2;
            continue;
        } else {
            if (count >= columns) {
                x = startX;
                y += 16;
                count = 0;
            }
            func_8006F8A4(x, y, 16, 16, text, palette, blend, ot, depth);
            x += 16;
            count++;
            total++;
        }
        text += 2;
    }
}

void setTexturedQuadTextureBlend(s32 index, s32 palette, s32 blend) {
    TextureRegionSet *texture = &D_8012DDA4[index];
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    quad->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    quad->clut = GetClut(texture->clutX, texture->clutY + palette);
    D_8013B6AC = texture->regions;
}

INCLUDE_ASM("asm/nonmatchings/graphics/sprite_draw", func_8006FE68);

/* Style 0 uses an eight-pixel border on each side. */
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 8, 8, 0, clutOffset, blendMode, ot, depth);
}

void drawTexturedFrameStyle9(s16 x, s16 y, s16 w, s16 h, s16 palette,
                             s16 blend, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 8, 8, 9, palette, blend, ot, depth);
}

/* Style 0x20D uses a sixteen-pixel border on each side. */
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 16, 16, 0x20D, clutOffset, blendMode, ot, depth);
}
