#include "psx/libetc.h"
#include "render_effects_internal.h"
#include "game/graphics/draw_mode_sprite.h"
#include "game/graphics/graphics.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/sprite.h"

typedef struct DigitSpritePacket {
    u32 tag;
    u16 tpage, drawModeCode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} DigitSpritePacket;

const s8 gDoubleByteGlyphCodes8x8[128] __attribute__((section(".data.double_byte_glyph_codes_8x8"))) =
    "\x20\x20\x81\x49\x81\x68\x81\x94\x81\x90\x81\x93\x81\x95\x81\x66"
    "\x81\x69\x81\x6A\x81\x96\x81\x7B\x81\x43\x81\x7C\x81\x44\x81\x5E"
    "\x82\x4F\x82\x50\x82\x51\x82\x52\x82\x53\x82\x54\x82\x55\x82\x56"
    "\x82\x57\x82\x58\x81\x46\x81\x47\x81\x83\x81\x81\x81\x84\x81\x48"
    "\x81\x97\x82\x60\x82\x61\x82\x62\x82\x63\x82\x64\x82\x65\x82\x66"
    "\x82\x67\x82\x68\x82\x69\x82\x6A\x82\x6B\x82\x6C\x82\x6D\x82\x6E"
    "\x82\x6F\x82\x70\x82\x71\x82\x72\x82\x73\x82\x74\x82\x75\x82\x76"
    "\x82\x77\x82\x78\x82\x79\x81\x6D\x81\x8F\x81\x6E";

DigitSpritePacket gDigitSprite16Template __attribute__((section(".data.digit_sprite16_template"))) = {
    0x04000000, 0x001F, 0xE100,
    0x80, 0x80, 0x80, SPRT_16_CODE,
    0, 0, 0, 0, 0
};

const s8 gDoubleByteGlyphCodes8x16[576] __attribute__((section(".data.double_byte_glyph_codes_8x16"))) =
    "\x82\x4F\x82\x50\x82\x51\x82\x52\x82\x53\x82\x54\x82\x55\x82\x56"
    "\x82\x57\x82\x58\x82\x60\x82\x61\x82\x62\x82\x63\x82\x64\x82\x65"
    "\x82\x66\x82\x67\x82\x68\x82\x69\x82\x6A\x82\x6B\x82\x6C\x82\x6D"
    "\x82\x6E\x82\x6F\x82\x70\x82\x71\x82\x72\x82\x73\x82\x74\x82\x75"
    "\x82\x76\x82\x77\x82\x78\x82\x79\x81\x49\x81\x68\x81\x94\x81\x66"
    "\x81\x96\x81\x7B\x81\x43\x81\x7C\x81\x44\x81\x5E\x81\x46\x81\x81"
    "\x81\x48\x81\x97\x81\x42\x81\x75\x81\x76\x81\x69\x81\x6A\x81\x41"
    "\x83\x40\x83\x42\x83\x44\x83\x46\x83\x48\x83\x62\x83\x83\x83\x85"
    "\x83\x87\x83\x41\x83\x43\x83\x45\x83\x47\x83\x49\x83\x4A\x83\x4C"
    "\x83\x4E\x83\x50\x83\x52\x83\x54\x83\x56\x83\x58\x83\x5A\x83\x5C"
    "\x83\x5E\x83\x60\x83\x63\x83\x65\x83\x67\x83\x69\x83\x6A\x83\x6B"
    "\x83\x6C\x83\x6D\x83\x6E\x83\x71\x83\x74\x83\x77\x83\x7A\x83\x7D"
    "\x83\x7E\x83\x80\x83\x81\x83\x82\x83\x84\x83\x86\x83\x88\x83\x89"
    "\x83\x8A\x83\x8B\x83\x8C\x83\x8D\x83\x8F\x83\x92\x83\x93\x83\x4B"
    "\x83\x4D\x83\x4F\x83\x51\x83\x53\x83\x55\x83\x57\x83\x59\x83\x5B"
    "\x83\x5D\x83\x5F\x83\x61\x83\x64\x83\x66\x83\x68\x83\x6F\x83\x72"
    "\x83\x75\x83\x78\x83\x7B\x83\x70\x83\x73\x83\x76\x83\x79\x83\x7C"
    "\x82\x9F\x82\xA1\x82\xA3\x82\xA5\x82\xA7\x82\xC1\x82\xE1\x82\xE3"
    "\x82\xE5\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8\x82\xA9\x82\xAB"
    "\x82\xAD\x82\xAF\x82\xB1\x82\xB3\x82\xB5\x82\xB7\x82\xB9\x82\xBB"
    "\x82\xBD\x82\xBF\x82\xC2\x82\xC4\x82\xC6\x82\xC8\x82\xC9\x82\xCA"
    "\x82\xCB\x82\xCC\x82\xCD\x82\xD0\x82\xD3\x82\xD6\x82\xD9\x82\xDC"
    "\x82\xDD\x82\xDE\x82\xDF\x82\xE0\x82\xE2\x82\xE4\x82\xE6\x82\xE7"
    "\x82\xE8\x82\xE9\x82\xEA\x82\xEB\x82\xED\x82\xF0\x82\xF1\x82\xAA"
    "\x82\xAC\x82\xAE\x82\xB0\x82\xB2\x82\xB4\x82\xB6\x82\xB8\x82\xBA"
    "\x82\xBC\x82\xBE\x82\xC0\x82\xC3\x82\xC5\x82\xC7\x82\xCE\x82\xD1"
    "\x82\xD4\x82\xD7\x82\xDA\x82\xCF\x82\xD2\x82\xD5\x82\xD8\x82\xDB"
    "\x91\x49\x91\xF0\x8F\x49\x97\xB9\x92\x86\x95\xCA\x92\x65\x8E\x63"
    "\x88\xE2\x90\xD5\x8C\xA2\x93\x58\x92\xB7\x92\x8D\x88\xD3\x90\xE1"
    "\x8E\x52\x94\x83\x97\xAC\x8D\xBB\x95\xBD\x8B\xCF\x93\x49\x8B\x4B"
    "\x92\xE8\x90\x94\x8D\xB6\x89\x45\x8B\x90\x91\xE5\x97\x8E\x90\xCE"
    "\x96\xE9\x93\xB9\x8F\xDC\x8B\xE0\x95\x4B\x97\x76\x90\xEA\x97\x70"
    "\x8E\xFC\x89\xF1\x90\x81\x8C\xA9\x8B\xC8\x8B\x43\x8D\x82\x91\x90"
    "\x88\xCA\x8A\x4A\x8E\x6E\x90\xDA\x91\xB1\x91\x8A\x8E\xE8\x8E\xB8"
    "\x94\x73\x89\x9F\x89\xF3\x8F\xE3\x96\x7B\x91\xCC\x88\xEA\x92\x85"
    "\x90\x6C\x90\x46\x8D\x44\x94\x74\x8F\x91\x89\xBA\x93\xFC";

const u16 gDigitSpriteBlendBits[4] __attribute__((section(".data.digit_sprite_blend_bits"))) = {
    0x0000, 0x0020, 0x0040, 0x0060
};

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
    TextureRegionSet *texture = &gSpriteTextureRegionSets[index];
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    quad->tpage = GetTPage(texture->depth, 1, texture->x, texture->y);
    quad->clut = GetClut(texture->clutX, texture->clutY + palette);
    gCurrentSpriteTextureRegions = texture->regions;
}

void setTexturedQuadColor(s32 r, s32 g, s32 b) {
    setRGB0(&gTexturedQuadTemplate, r, g, b);
}

void setTexturedQuadSemiTrans(s32 enabled) {
    SetSemiTrans(&gTexturedQuadTemplate, enabled);
}

void drawTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region = &gCurrentSpriteTextureRegions[index];
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
    region = &gCurrentSpriteTextureRegions[index];
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
    TexturedQuadFrame *frame = &gTexturedQuadFrames[index];

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
    frame = &gTexturedQuadFrames[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    setTexturedQuadSemiTrans(semiTrans);
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
    frame = &gTexturedQuadFrames[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    setTexturedQuadSemiTrans(semiTrans);
    region = &gCurrentSpriteTextureRegions[frame->frameIndex];
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
    frame = &gTexturedQuadFrames[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    setTexturedQuadSemiTrans(semiTrans);
    region = &gCurrentSpriteTextureRegions[frame->frameIndex];
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
    TextureRegion *region = &gCurrentSpriteTextureRegions[index];
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
    TexturedQuadFrame *frame = &gTexturedQuadFrames[index];

    setTexturedQuadTexture(frame->textureIndex, 0);
    drawInclusiveTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

void drawInclusiveTexturedQuadByIndexWithBlend(s16 x, s16 y, s32 index, s16 palette,
                                              s16 blendMode, GsOT *ot, u16 depth) {
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
    frame = &gTexturedQuadFrames[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    setTexturedQuadSemiTrans(semiTrans);
    drawInclusiveTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

void initSpritePrimitiveTemplate(void) {
    SPRT *sprite = &gSpritePrimitiveTemplate.sprite;
    SetSprt((P_TAG *)sprite);
    SetDrawTPage(&gSpritePrimitiveTemplate.drawPage, 1, 1, GetTPage(0, 0, 0, 0));
    setRGB0(sprite, 0x80, 0x80, 0x80);
}

void setSpriteTextureBlend(s32 index, s32 palette, s32 blend) {
    TextureRegionSet *texture = &gSpriteTextureRegionSets[index];
    SpritePrimitiveTemplate *sprite = &gSpritePrimitiveTemplate;
    SetDrawTPage(&sprite->drawPage, 1, 1,
                 GetTPage(texture->depth, blend, texture->x, texture->y));
    sprite->sprite.clut = GetClut(texture->clutX, texture->clutY + palette);
    gCurrentSpriteTextureRegions = texture->regions;
}

void setSpriteColor(s32 r, s32 g, s32 b) {
    setRGB0(&gSpriteTemplate, r, g, b);
    setRGB0(&gTexturedSpriteTemplate, r, g, b);
}

void setSpriteTemplateSemiTrans(s32 enabled) {
    SetSemiTrans(&gSpriteTemplate, enabled);
}

/* The old-style definition keeps the promoted palette parameter in the public
 * prototype and uses a 16-bit palette when building the CLUT. */
void drawSpriteByIndexWithBlend(x, y, index, palette, blendMode, ot, depth)
s16 x, y, palette;
s32 index, blendMode;
GsOT *ot;
u16 depth;
{
    TexturedQuadFrame *frame = &gTexturedQuadFrames[index];
    TextureRegionSet *texture = &gSpriteTextureRegionSets[frame->textureIndex];
    TextureRegion *region = &texture->regions[frame->frameIndex];
    DrawModeSpritePacket *sprite = (DrawModeSpritePacket *)GsGetWorkBase();

    sprite->tag = (sizeof(*sprite) / sizeof(u32) - 1) << 24;
    sprite->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    setRGB0(sprite, gTexturedSpriteTemplate.r0, gTexturedSpriteTemplate.g0, gTexturedSpriteTemplate.b0);
    if ((s16)blendMode >= 0) {
        u16 tp = texture->depth;
        s16 ty = texture->y;
        u16 tx = texture->x;
        sprite->code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
        sprite->tpage = getTPage(tp, blendMode, tx, ty);
    } else {
        u16 tp = texture->depth;
        s16 ty = texture->y;
        u16 tx = texture->x;
        sprite->code = TEXTURED_SPRITE_CODE;
        sprite->tpage = getTPage(tp, 0, tx, ty);
    }

    {
        u16 clut = getClut(texture->clutX, texture->clutY + palette);
        setXY0(sprite, x, y);
        sprite->clut = clut;
    }
    setUV0(sprite, region->u, region->v);
    setWH(sprite, region->w, region->h);
    AddPrim(&ot->org[depth], sprite);
    GsSetWorkBase((PACKET *)(sprite + 1));
}

void drawSpriteTextureRegion(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    SPRT *sprite = (SPRT *)GsGetWorkBase();
    DR_TPAGE *page;
    TextureRegion *region;
    *sprite = gSpritePrimitiveTemplate.sprite;
    region = &gCurrentSpriteTextureRegions[index];
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
    GsSetWorkBase((PACKET *)(page + 1));
}

void addSpritePacket(SPRT *source, GsOT *ot, u16 depth) {
    SPRT *packet = (SPRT *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    GsSetWorkBase((PACKET *)(packet + 1));
}

void addDrawPagePacket(DR_TPAGE *source, GsOT *ot, u16 depth) {
    DR_TPAGE *packet = (DR_TPAGE *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    GsSetWorkBase((PACKET *)(packet + 1));
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
    frame = &gTexturedQuadFrames[index];
    texture = &gSpriteTextureRegionSets[frame->textureIndex];
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
    frame = &gTexturedQuadFrames[index];
    texture = &gSpriteTextureRegionSets[frame->textureIndex];
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
                drawDoubleByteGlyph8x8(currentX, currentY, 8, 8, text, palette, blend, ot, depth);
                currentX += 8;
                count++;
            }
            text += 2;
        }
    }
}

void drawDoubleByteGlyph8x8(s16 x, s16 y, s16 width, s16 height, const char *text, s16 palette,
                            s16 blendArg, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s16 u, v;
    const s8 *glyph;
    s32 i;
    s16 page;
    s32 blend;
    SPRT *sprite;
    if (blendArg >= 0) {
        blend = blendArg;
        semiTrans = 1;
    } else {
        semiTrans = blend = 0;
    }
    v = u = -1;
    glyph = gDoubleByteGlyphCodes8x8;
    i = 0;
    while (1) {
        if (!*glyph) {
            break;
        }
        if ((s8)text[0] == glyph[0] && (s8)text[1] == glyph[1]) {
            u = (i % 32) * 8;
            v = (i / 32) * 8;
            page = 0;
            break;
        }
        glyph += 2;
        i++;
    }
    if (u >= 0 && v >= 0) {
        setSpriteTextureBlend(page, palette, blend);
        setSpriteTemplateSemiTrans(semiTrans);
        sprite = &gSpriteTemplate;
        setXY0(sprite, x, y);
        setUV0(sprite, u, v);
        setWH(sprite, width, height);
        addSpritePacket(sprite, ot, depth);
        addDrawPagePacket(&gSpritePrimitiveTemplate.drawPage, ot, depth);
    }
}

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
        setSpriteTemplateSemiTrans(semiTrans);
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

void drawDecimalNumber8x8(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                         s16 palette, s16 blend, GsOT *ot, u16 depth) {
    u8 padding[0x10]; /* Preserve the original stack frame. */
    s32 semiTrans;
    s32 blendMode;
    SpritePrimitiveTemplate *template;
    SPRT *packet;
    DR_TPAGE *page;
    s32 i;
    s32 drawn;

    if (digits > 0) {
        if (blend >= 0) {
            blendMode = blend;
            semiTrans = 1;
        } else {
            blendMode = 0;
            semiTrans = 0;
        }
        template = &gSpritePrimitiveTemplate;
        SetDrawTPage(&template->drawPage, 1, 1, GetTPage(0, blendMode, 0x3C0, 0x100));
        template->sprite.clut = GetClut(0x300, palette + 0xF0);
        SetSemiTrans(&template->sprite, semiTrans);
        setXY0(&template->sprite, x + (digits - 1) * 8, y);
        setWH(&template->sprite, 8, 8);
        setUV0(&template->sprite, 0, 0);
        packet = (SPRT *)GsGetWorkBase();

        drawn = 0;
        for (i = 0; i < digits;) {
            if (value <= 0) {
                break;
            }
            *packet = template->sprite;
            packet->u0 = (value % 10) * 8 + 0x80;
            AddPrim(&ot->org[depth], packet);
            i++;
            packet++;
            value /= 10;
            drawn = 1;
            template->sprite.x0 -= 8;
        }
        if (!drawn) {
            *packet = template->sprite;
            packet->u0 = 0x80;
            AddPrim(&ot->org[depth], packet);
            packet++;
            i++;
            template->sprite.x0 -= 8;
        }
        if (zeroFill == 1) {
            for (; i < digits; i++) {
                *packet = template->sprite;
                packet->u0 = 0x80;
                AddPrim(&ot->org[depth], packet);
                packet++;
                template->sprite.x0 -= 8;
            }
        }
        page = (DR_TPAGE *)packet;
        *page = gSpritePrimitiveTemplate.drawPage;
        AddPrim(&ot->org[depth], page);
        GsSetWorkBase((PACKET *)(page + 1));
    }
}

void drawDecimalNumber8x16(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                          s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s32 blendMode;
    SpritePrimitiveTemplate *template;
    SPRT *packet;
    DR_TPAGE *page;
    s32 i;
    s32 drawn;

    if (digits > 0) {
        if (blend >= 0) {
            blendMode = blend;
            semiTrans = 1;
        } else {
            blendMode = 0;
            semiTrans = 0;
        }
        template = &gSpritePrimitiveTemplate;
        SetDrawTPage(&template->drawPage, 1, 1, GetTPage(0, blendMode, 0x3C0, 0x100));
        template->sprite.clut = GetClut(0x300, palette + 0xF0);
        SetSemiTrans(&template->sprite, semiTrans);
        setXY0(&template->sprite, x + (digits - 1) * 8, y);
        setWH(&template->sprite, 8, 16);
        setUV0(&template->sprite, 0, 16);
        packet = (SPRT *)GsGetWorkBase();

        drawn = 0;
        for (i = 0; i < digits;) {
            if (value <= 0) {
                break;
            }
            *packet = template->sprite;
            packet->u0 = (value % 10) * 8;
            AddPrim(&ot->org[depth], packet);
            i++;
            packet++;
            value /= 10;
            drawn = 1;
            template->sprite.x0 -= 8;
        }
        if (!drawn) {
            *packet = template->sprite;
            packet->u0 = 0;
            AddPrim(&ot->org[depth], packet);
            packet++;
            i++;
            template->sprite.x0 -= 8;
        }
        if (zeroFill == 1) {
            for (; i < digits; i++) {
                *packet = template->sprite;
                packet->u0 = 0;
                AddPrim(&ot->org[depth], packet);
                packet++;
                template->sprite.x0 -= 8;
            }
        }
        page = (DR_TPAGE *)packet;
        *page = gSpritePrimitiveTemplate.drawPage;
        AddPrim(&ot->org[depth], page);
        GsSetWorkBase((PACKET *)(page + 1));
    }
}

void drawMenuDecimalNumber8x16(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                              s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s32 blendMode;
    SpritePrimitiveTemplate *template;
    SPRT *packet;
    DR_TPAGE *page;
    s32 i;
    s32 drawn;

    if (digits > 0) {
        if (blend >= 0) {
            blendMode = blend;
            semiTrans = 1;
        } else {
            blendMode = 0;
            semiTrans = 0;
        }
        template = &gSpritePrimitiveTemplate;
        SetDrawTPage(&template->drawPage, 1, 1, GetTPage(0, blendMode, 0x3C0, 0));
        template->sprite.clut = GetClut(0x330, palette + 0xF0);
        SetSemiTrans(&template->sprite, semiTrans);
        setXY0(&template->sprite, x + (digits - 1) * 8, y);
        setWH(&template->sprite, 8, 16);
        setUV0(&template->sprite, 0, 0x38);
        packet = (SPRT *)GsGetWorkBase();

        drawn = 0;
        for (i = 0; i < digits;) {
            if (value <= 0) {
                break;
            }
            *packet = template->sprite;
            packet->u0 = 0x60 + (value % 10) * 8;
            AddPrim(&ot->org[depth], packet);
            i++;
            packet++;
            value /= 10;
            drawn = 1;
            template->sprite.x0 -= 8;
        }
        if (!drawn) {
            *packet = template->sprite;
            packet->u0 = 0x60;
            AddPrim(&ot->org[depth], packet);
            packet++;
            i++;
            template->sprite.x0 -= 8;
        }
        if (zeroFill == 1) {
            for (; i < digits; i++) {
                *packet = template->sprite;
                packet->u0 = 0x60;
                AddPrim(&ot->org[depth], packet);
                packet++;
                template->sprite.x0 -= 8;
            }
        }
        page = (DR_TPAGE *)packet;
        *page = gSpritePrimitiveTemplate.drawPage;
        AddPrim(&ot->org[depth], page);
        GsSetWorkBase((PACKET *)(page + 1));
    }
}

void drawHudDecimalNumber8x16(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                             s16 palette, s16 blend, GsOT *ot, u16 depth) {
    u8 padding[0x10]; /* Preserve the original stack frame. */
    s32 semiTrans;
    s32 blendMode;
    SpritePrimitiveTemplate *template;
    SPRT *packet;
    DR_TPAGE *page;
    s32 i;
    s32 drawn;

    if (digits > 0) {
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
        setXY0(&template->sprite, x + (digits - 1) * 8, y);
        setWH(&template->sprite, 8, 16);
        setUV0(&template->sprite, 0, 0);
        packet = (SPRT *)GsGetWorkBase();

        drawn = 0;
        for (i = 0; i < digits;) {
            if (value <= 0) {
                break;
            }
            *packet = template->sprite;
            packet->u0 = (value % 10) * 8;
            AddPrim(&ot->org[depth], packet);
            i++;
            packet++;
            value /= 10;
            drawn = 1;
            template->sprite.x0 -= 8;
        }
        if (!drawn) {
            *packet = template->sprite;
            packet->u0 = 0;
            AddPrim(&ot->org[depth], packet);
            packet++;
            i++;
            template->sprite.x0 -= 8;
        }
        if (zeroFill == 1) {
            for (; i < digits; i++) {
                *packet = template->sprite;
                packet->u0 = 0;
                AddPrim(&ot->org[depth], packet);
                packet++;
                template->sprite.x0 -= 8;
            }
        }
        page = (DR_TPAGE *)packet;
        *page = gSpritePrimitiveTemplate.drawPage;
        AddPrim(&ot->org[depth], page);
        GsSetWorkBase((PACKET *)(page + 1));
    }
}

void drawDecimalNumber16(s16 x, s16 y, s16 count, s16 mode, s32 value,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    DigitSpritePacket digit;
    DigitSpritePacket *packet;
    s16 i;
    s16 drawn;
    if (count > 0) {
        digit = gDigitSprite16Template;
        packet = &digit;
        if (blend >= 0) {
            packet->tpage |= gDigitSpriteBlendBits[blend];
            /* The draw-mode word occupies the standalone sprite tag slot. */
            setSemiTrans((SPRT_16 *)&packet->tpage, 1);
        }
        setRGB0(packet, gTexturedSpriteTemplate.r0, gTexturedSpriteTemplate.g0, gTexturedSpriteTemplate.b0);
        i = 0;
        setXY0(packet, x, y);
        packet->x0 += (count - 1) * 16;
        setUV0(packet, 0, 0xA0);
        packet->clut = getClut(0x300, palette + 0xF0);
        packet = (DigitSpritePacket *)GsGetWorkBase();
        drawn = 0;
        for (; i < count; i++) {
            if (value <= 0) break;
            *packet = digit;
            packet->u0 = (value % 10) * 16;
            AddPrim(&ot->org[depth], packet);
            packet++;
            value /= 10;
            drawn = 1;
            digit.x0 -= 16;
        }
        if (!drawn) {
            *packet = digit;
            packet->u0 = 0;
            AddPrim(&ot->org[depth], packet);
            packet++;
            i++;
            digit.x0 -= 16;
        }
        if (mode == 1) {
            for (; i < count; i++) {
                *packet = digit;
                packet->u0 = 0;
                AddPrim(&ot->org[depth], packet);
                packet++;
                digit.x0 -= 16;
            }
        }
        GsSetWorkBase((PACKET *)packet);
    }
}

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
        GsSetWorkBase((PACKET *)(page + 1));
    }
}

void drawPackedTime(s16 x, s16 y, u32 value, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 field;
    value >>= 8;
    field = value & 255;
    drawHudDecimalNumber8x16(x + 48, y, 2, 1, field, palette, blend, ot, depth);
    value >>= 8;
    drawSpriteByIndexWithBlend(x + 40, y, 24, palette, blend, ot, depth);
    field = value & 255;
    drawHudDecimalNumber8x16(x + 24, y, 2, 1, field, 1, -1, ot, depth);
    drawSpriteByIndexWithBlend(x + 16, y, 25, palette, blend, ot, depth);
    value >>= 8;
    drawHudDecimalNumber8x16(x, y, 2, 1, value, 1, -1, ot, depth);
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
            drawDoubleByteGlyph8x16(x, y, 8, 16, text, currentPalette, blend, ot, depth);
            x += 8;
            column++;
        }
        text += 2;
    }
}

void drawDoubleByteGlyph8x16(s16 x, s16 y, s16 width, s16 height, const s8 *text, s16 palette,
                            s16 blendArg, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s16 u, v;
    const s8 *glyph;
    s32 i;
    s16 page;
    s32 blend;
    SPRT *sprite;
    if (blendArg >= 0) {
        blend = blendArg;
        semiTrans = 1;
    } else {
        semiTrans = blend = 0;
    }
    v = u = -1;
    glyph = gDoubleByteGlyphCodes8x16;
    i = 0;
    while (1) {
        if (!*glyph) {
            break;
        }
        if (text[0] == glyph[0] && text[1] == glyph[1]) {
            u = (i % 32) * 8;
            v = (i / 32) * 16 + 16;
            page = 0;
            break;
        }
        glyph += 2;
        i++;
    }
    if (u >= 0 && v >= 0) {
        setSpriteTextureBlend(page, palette, blend);
        setSpriteTemplateSemiTrans(semiTrans);
        sprite = &gSpriteTemplate;
        setXY0(sprite, x, y);
        setUV0(sprite, u, v);
        setWH(sprite, width, height);
        addSpritePacket(sprite, ot, depth);
        addDrawPagePacket(&gSpritePrimitiveTemplate.drawPage, ot, depth);
    }
}

void drawWrappedGlyphText8x16(s32 x, s32 y, s16 columnLimit, const u16 *text,
                             s16 palette, s16 blend, GsOT *ot, u16 depth) {
    DrawModeSpritePacket *packet = (DrawModeSpritePacket *)getScratchAddr(0);
    DrawModeSpritePacket *out;
    s16 startX = x;
    s16 currentX;
    s16 savedColumns = columnLimit;
    s16 column;
    s16 previous;
    u16 glyph;
    u8 r, g, b;

    packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
    packet->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    setcode((SPRT *)&packet->tpage, TEXTURED_SPRITE_CODE);
    r = gTexturedSpriteTemplate.r0;
    g = gTexturedSpriteTemplate.g0;
    b = gTexturedSpriteTemplate.b0;
    setRGB0(packet, r, g, b);
    packet->tpage = getTPage(0, 0, 960, 256);
    if (blend >= 0) {
        setcode((SPRT *)&packet->tpage, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG);
        packet->tpage = (blend << 5) | getTPage(0, 0, 960, 256);
    }
    packet->clut = getClut(768, palette + 240);
    setWH(packet, 8, 16);

    out = (DrawModeSpritePacket *)GsGetWorkBase();
    currentX = x;
    column = 0;
    while (*text) {
        if (column >= columnLimit) {
            currentX = startX;
            y += 16;
            column = 0;
        }
        if (*text == 0x800A) {
            if (previous != 2 || column != 0) {
                column = savedColumns;
            }
            text++;
            previous = 0;
        } else if ((*text & 0xC000) == 0xC000) {
            packet->clut = getClut(768, (*text & 15) + 240);
            text++;
            previous = 1;
        } else if (*text == 0x8020) {
            currentX += 8;
            column++;
            text++;
            previous = 2;
            continue;
        } else {
            DrawModeSpritePacket *glyphPacket;
            u8 u, v;
            *out = *packet;
            glyph = *text - 1;
            u = (glyph % 32) * 8;
            v = (glyph / 32) * 16 + 16;
            setUV0(out, u, v);
            setXY0(out, currentX, y);
            glyphPacket = out++;
            currentX += 8;
            column++;
            AddPrim(&ot->org[depth], glyphPacket);
            *out = *packet;
            text++;
            previous = 2;
        }
    }
    GsSetWorkBase((PACKET *)out);
}

void drawWrappedGlyphText16x16(s16 x, s16 y, s16 columnLimit, const u16 *text,
                             s32 palette, s16 blendArg, GsOT *ot, u16 depthArg) {
    enum { TOKEN_NEWLINE, TOKEN_PALETTE, TOKEN_CELL };
    s16 currentX, currentY;
    s16 column;
    s16 textureY;
    u16 code, glyph;
    DrawModeSpritePacket *scratch = (DrawModeSpritePacket *)getScratchAddr(0);
    DrawModeSpritePacket *packet;
    s16 previousToken;
    s16 columns = columnLimit;
    s16 blendMode = blendArg;
    u16 depth = depthArg;
    u16 clut = getClut(768, palette + 240);
    u8 red, green, blue;

    scratch->tag = (sizeof(*scratch) / sizeof(u32) - 1) << 24;
    scratch->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    setcode((SPRT *)&scratch->tpage, TEXTURED_SPRITE_CODE);
    red = gTexturedSpriteTemplate.r0;
    green = gTexturedSpriteTemplate.g0;
    blue = gTexturedSpriteTemplate.b0;
    setRGB0(scratch, red, green, blue);
    setWH(scratch, 16, 16);
    scratch->clut = clut;

    packet = (DrawModeSpritePacket *)GsGetWorkBase();
    currentX = x;
    currentY = y;
    column = 0;
    while (*text) {
        DrawModeSpritePacket *glyphPacket;
        u8 u, v;

        if (column >= columns) {
            currentX = x;
            currentY += 16;
            column = 0;
        }
        code = *text;
        if (code == 0x800A) {
            /* A newline following a full row advances only once. */
            if (previousToken != TOKEN_CELL || column != 0) {
                column = columnLimit;
            }
            text++;
            previousToken = TOKEN_NEWLINE;
            continue;
        }
        if ((code & 0xC000) == 0xC000) {
            scratch->clut = getClut(768, (code & 15) + 240);
            text++;
            previousToken = TOKEN_PALETTE;
            continue;
        }
        if (code == 0x8020) {
            currentX += 16;
            column++;
            text++;
            previousToken = TOKEN_CELL;
            continue;
        }
        glyph = code - 1;
        if (glyph < 0x60) {
            scratch->tpage = getTPage(0, 0, 960, 256);
            textureY = 160;
        } else if (glyph < 0x160) {
            scratch->tpage = getTPage(0, 0, 896, 256);
            textureY = 0;
            glyph = code - 0x61;
        } else if (glyph < 0x260) {
            scratch->tpage = getTPage(0, 0, 768, 256);
            textureY = 0;
            glyph = code - 0x161;
        }

        if (blendMode >= 0) {
            setSemiTrans((SPRT *)&scratch->tpage, 1);
            scratch->tpage |= (u16)blendMode << 5;
        }
        *packet = *scratch;
        u = (glyph % 16) * 16;
        v = textureY + (glyph / 16) * 16;
        setUV0(packet, u, v);
        setXY0(packet, currentX, currentY);
        glyphPacket = packet++;
        currentX += 16;
        column++;
        AddPrim(&ot->org[depth], glyphPacket);
        previousToken = TOKEN_CELL;
        *packet = *scratch;
        text++;
    }
    GsSetWorkBase((PACKET *)packet);
}

void drawLimitedWrappedGlyphText16x16(s16 x, s16 yArg, s32 countArg, s32 columnsArg, const u16 *text,
                                       s16 palette, s32 blendArg, GsOT *ot, u32 depthArg) {
    s16 currentX = x;
    s16 column = 0;
    s16 drawn;
    s16 previous;
    s16 textureY;
    u16 code, glyph;
    DrawModeSpritePacket *scratch = (DrawModeSpritePacket *)getScratchAddr(0);
    DrawModeSpritePacket *packet;
    s16 startX;
    s16 count = countArg;
    s16 columns = columnsArg;
    s16 blend = blendArg;
    u16 depth = depthArg;
    s16 y = yArg;
    u8 red, green, blue;
    u16 clut = getClut(768, palette + 240);

    scratch->tag = (sizeof(*scratch) / sizeof(u32) - 1) << 24;
    scratch->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    setcode((SPRT *)&scratch->tpage, TEXTURED_SPRITE_CODE);
    red = gTexturedSpriteTemplate.r0;
    green = gTexturedSpriteTemplate.g0;
    blue = gTexturedSpriteTemplate.b0;
    setRGB0(scratch, red, green, blue);
    setWH(scratch, 16, 16);
    scratch->clut = clut;
    packet = (DrawModeSpritePacket *)GsGetWorkBase();
    drawn = column;
    startX = x;
    while (*text) {
        DrawModeSpritePacket *glyphPacket;
        u8 u, v;
        if (drawn >= count) {
            break;
        }
        if (column >= columns) {
            column = 0;
            currentX = startX;
            y += 16;
        }
        code = *text;
        if (code == 0x800A) {
            if (previous != 2 || column != 0) {
                column = columns;
            }
            text++;
            previous = 0;
            continue;
        }
        if ((code & 0xC000) == 0xC000) {
            scratch->clut = getClut(768, (code & 15) + 240);
            text++;
            previous = 1;
            continue;
        }
        if (code == 0x8020) {
            currentX += 16;
            column++;
            text++;
            drawn++;
            previous = 2;
            continue;
        }
        glyph = code - 1;
        if (glyph < 0x60) {
            scratch->tpage = getTPage(0, 0, 960, 256);
            textureY = 160;
        } else if (glyph < 0x160) {
            scratch->tpage = getTPage(0, 0, 896, 256);
            textureY = 0;
            glyph = code - 0x61;
        } else if (glyph < 0x260) {
            scratch->tpage = getTPage(0, 0, 768, 256);
            textureY = 0;
            glyph = code - 0x161;
        }
        if (blend >= 0) {
            setSemiTrans((SPRT *)&scratch->tpage, 1);
            scratch->tpage |= (u16)blend << 5;
        }
        *packet = *scratch;
        u = (glyph % 16) * 16;
        v = textureY + (glyph / 16) * 16;
        glyphPacket = packet;
        setUV0(packet, u, v);
        setXY0(packet, currentX, y);
        packet++;
        currentX += 16;
        column++;
        AddPrim(&ot->org[depth], glyphPacket);
        drawn++;
        previous = 2;
        *packet = *scratch;
        text++;
    }
    GsSetWorkBase((PACKET *)packet);
}

void drawUppercaseProportionalText(s16 x, s16 y, s16 count, const char *text, s16 palette,
                   s16 blend, GsOT *ot, u16 depth) {
    DrawModeSpritePacket *packet = (DrawModeSpritePacket *)getScratchAddr(0);
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
        drawDoubleByteGlyph16x16(x, y, 16, 16, text, palette, blend, ot, depth);
        x += 16;
        text += 2;
        count++;
    }
}

void drawDoubleByteGlyph16x16(s16 x, s16 y, s16 unusedWidth, s16 unusedHeight, const char *glyphText,
                              s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 screenX = x;
    s16 screenY = y;
    s16 u = -1;
    s16 v = -1;
    s16 page;
    s16 offset;
    const s8 *first;
    const s8 *second;
    const s8 *third;
    DrawModeSpritePacket *packet;
    TextureRegionSet *texture;

    first = gDoubleByteGlyphCodes16x16Page0;
    second = gDoubleByteGlyphCodes16x16Page1;
    third = gDoubleByteGlyphCodes16x16Page2;
    offset = 0;
    /* The first two tables determine when the glyph search ends. */
    for (;; offset += 2) {
        s8 firstByte = *first;
        if (!firstByte) {
            if (!*second) {
                break;
            }
        } else {
            if ((s8)glyphText[0] == firstByte && (s8)glyphText[1] == first[1]) {
                u = (offset / 2 % 16) * 16;
                v = (offset / 32) * 16 + 160;
                page = 0;
                break;
            }
            first += 2;
        }
        if (*second) {
            if ((s8)glyphText[0] == second[0] && (s8)glyphText[1] == second[1]) {
                u = (offset / 2 % 16) * 16;
                v = (offset / 32) * 16;
                page = 1;
                break;
            }
            second += 2;
        }
        if (*third) {
            if ((s8)glyphText[0] == third[0] && (s8)glyphText[1] == third[1]) {
                u = (offset / 2 % 16) * 16;
                v = (offset / 32) * 16;
                page = 29;
                break;
            }
            third += 2;
        }
    }
    if (u >= 0 && v >= 0) {
        u16 tpage;
        u16 clut;
        packet = (DrawModeSpritePacket *)GsGetWorkBase();
        packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
        packet->drawModeCode = DRAW_MODE_COMMAND_HIGH;
        setRGB0(packet, gTexturedSpriteTemplate.r0, gTexturedSpriteTemplate.g0, gTexturedSpriteTemplate.b0);
        texture = &gSpriteTextureRegionSets[page];
        if (blend >= 0) {
            s16 textureDepth = texture->depth;
            s16 ty = texture->y;
            s16 tx = texture->x;
            packet->code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
            tpage = getTPage(textureDepth, blend, tx, ty);
        } else {
            s16 textureDepth = texture->depth;
            s16 ty = texture->y;
            s16 tx = texture->x;
            packet->code = TEXTURED_SPRITE_CODE;
            tpage = getTPage(textureDepth, 0, tx, ty);
        }
        packet->tpage = tpage;
        clut = getClut((u16)texture->clutX, (u16)texture->clutY + palette);
        setXY0(packet, screenX, screenY);
        setUV0(packet, u, v);
        setWH(packet, 16, 16);
        packet->clut = clut;
        AddPrim(&ot->org[depth], packet);
        GsSetWorkBase((PACKET *)(packet + 1));
    }
}

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
            drawDoubleByteGlyph16x16(x, y, 16, 16, text, palette, blend, ot, depth);
            x += 16;
            count++;
            total++;
        }
        text += 2;
    }
}

void setTexturedQuadTextureBlend(s32 index, s32 palette, s32 blend) {
    TextureRegionSet *texture = &gSpriteTextureRegionSets[index];
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    quad->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    quad->clut = GetClut(texture->clutX, texture->clutY + palette);
    gCurrentSpriteTextureRegions = texture->regions;
}

void drawTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 borderW, s16 borderH,
                  s16 style, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *entry;
    s32 semiTrans, blend, frame;
    s16 right, bottom, innerX, innerY, innerW, innerH;
    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    entry = &gTexturedQuadFrames[style];
    frame = entry->frameIndex;
    setTexturedQuadTextureBlend(entry->textureIndex, palette, blend);
    setTexturedQuadSemiTrans(semiTrans);
    right = x + w - borderW;
    bottom = y + h - borderH;
    drawTexturedQuad(x, y, frame, ot, depth);
    drawTexturedQuad(x, bottom, frame + 6, ot, depth);
    innerX = x + borderW;
    innerW = right - innerX;
    drawStretchedTexturedQuad(innerX, y, innerW, borderH, frame + 1, ot, depth);
    drawStretchedTexturedQuad(innerX, bottom, innerW, borderH, frame + 7, ot, depth);
    drawTexturedQuad(right, y, frame + 2, ot, depth);
    drawTexturedQuad(right, bottom, frame + 8, ot, depth);
    innerY = y + borderH;
    innerH = bottom - innerY;
    drawStretchedTexturedQuad(x, innerY, borderW, innerH, frame + 3, ot, depth);
    drawStretchedTexturedQuad(right, innerY, borderW, innerH, frame + 5, ot, depth);
    drawStretchedTexturedQuad(innerX, innerY, innerW, innerH, frame + 4, ot, depth);
}

/* Style 0 uses an eight-pixel border on each side. */
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    drawTexturedPanel(x, y, w, h, 8, 8, 0, clutOffset, blendMode, ot, depth);
}

void drawTexturedFrameStyle9(s16 x, s16 y, s16 w, s16 h, s16 palette,
                             s16 blend, GsOT *ot, u16 depth) {
    drawTexturedPanel(x, y, w, h, 8, 8, 9, palette, blend, ot, depth);
}

/* Style 0x20D uses a sixteen-pixel border on each side. */
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    drawTexturedPanel(x, y, w, h, 16, 16, 0x20D, clutOffset, blendMode, ot, depth);
}
