#include "../engine/resource_services_internal.h"

SavedImageEntry gSavedImageEntries[32] __attribute__((section(".data.saved_image_entries"))) = {0};

void uploadTimImageWithOffset(u32 *tim, s16 *position) {
    RECT rect;
    GsIMAGE image;
    GsGetTimInfo(tim + 1, &image);
    rect.x = image.px;
    rect.y = image.py;
    rect.w = image.pw;
    rect.h = image.ph;
    if (position != NULL) {
        rect.x += position[0];
        rect.y += position[1];
    }
    LoadImage(&rect, image.pixel);
    if (image.pmode & 8) {
        rect.x = image.cx;
        rect.y = image.cy;
        rect.w = image.cw;
        rect.h = image.ch;
        if (position != NULL) {
            rect.x += position[0];
            rect.y += position[1];
        }
        LoadImage(&rect, image.clut);
    }
}

void uploadTimArchive(TimArchive *archive, s16 *position) {
    u16 i = 0;
    u32 count = archive->count;
    /* Keep the archive index signed, as in the on-disc table lookup. */
    while (1) {
        s32 offset;
        s32 index;
        if (i >= count) {
            break;
        }
        index = (u16)i + 1;
        offset = archive->words[(s16)index];
        uploadTimImageWithOffset((u32 *)((u8 *)archive + ((offset >> 1) << 1)), position);
        i++;
    }
}

void offsetTmdObjectTextures(u16 index, TmdObject *objects, ModelTextureOffset *offset) {
    TmdObject *object = &objects[index];
    u8 *primitive = (u8 *)object->primitives;
    u16 i;
    for (i = 0; i < object->primitiveCount; i++) {
        TmdTexturedPrimitive *p = (TmdTexturedPrimitive *)primitive;
        TmdTexturedPrimitive *header = (TmdTexturedPrimitive *)primitive;
        switch (header->header.code) {
        case TMD_TEXTURE_HEADER(POLY_FT4_CODE, 0, 7, 9):
        case TMD_TEXTURE_HEADER(POLY_FT4_CODE | 1, 1, 7, 9):
        case TMD_TEXTURE_HEADER(POLY_FT4_CODE | 2, 0, 7, 9):
        case TMD_TEXTURE_HEADER(POLY_FT4_CODE | 2 | 1, 1, 7, 9):
        case TMD_TEXTURE_HEADER(POLY_GT4_CODE, 0, 8, 12):
        case TMD_TEXTURE_HEADER(POLY_GT4_CODE | 1, 1, 10, 12):
        case TMD_TEXTURE_HEADER(POLY_GT4_CODE | 2, 0, 8, 12):
        case TMD_TEXTURE_HEADER(POLY_GT4_CODE | 2 | 1, 1, 10, 12):
            p->uv3 += offset->uv;
            /* Fall through to the three shared texture coordinates. */
        case TMD_TEXTURE_HEADER(POLY_FT3_CODE, 0, 5, 7):
        case TMD_TEXTURE_HEADER(POLY_FT3_CODE | 1, 1, 6, 7):
        case TMD_TEXTURE_HEADER(POLY_FT3_CODE | 2, 0, 5, 7):
        case TMD_TEXTURE_HEADER(POLY_FT3_CODE | 2 | 1, 1, 6, 7):
        case TMD_TEXTURE_HEADER(POLY_GT3_CODE, 0, 6, 9):
        case TMD_TEXTURE_HEADER(POLY_GT3_CODE | 1, 1, 8, 9):
        case TMD_TEXTURE_HEADER(POLY_GT3_CODE | 2, 0, 6, 9):
        case TMD_TEXTURE_HEADER(POLY_GT3_CODE | 2 | 1, 1, 8, 9):
            p->clut += offset->clut;
            p->tpage += offset->tpage;
            p->uv0 += offset->uv;
            p->uv1 += offset->uv;
            p->uv2 += offset->uv;
        }
        primitive += header->header.bytes.inputLength * 4 + 4;
    }
}

void uploadImage64x256(u32 *pixels, s16 x, s16 y) {
    RECT rect;
    rect.x = x;
    rect.y = y;
    rect.w = 64;
    rect.h = 256;
    LoadImage(&rect, pixels);
    DrawSync(0);
}

void resetPaletteRecords(void) {
    u16 i;
    for (i = 0; i < 32; i++) {
        gSavedImageEntries[i].flags = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/texture_upload", func_800162C8);

void releaseSavedImageEntry(u16 index) {
    if (index != 0xFF) {
        gSavedImageEntries[index].flags = 0;
    }
}

u16 saveVram16Pixels(s32 x, s32 y) {
    u16 index = func_800162C8();
    if (index != 32) {
        SavedImageEntry *entries = gSavedImageEntries;
        SavedImageEntry *entry = entries;
        entry += index;
        entry->rect.w = 16;
        entry->rect.x = x;
        entry->rect.y = y;
        entry->rect.h = 1;
        StoreImage(&gSavedImageEntries[index].rect, (u32 *)gSavedImageEntries[index].pixels);
        entry->flags = 1;
        return index;
    }
    return 32;
}

void readVram16Pixels(s32 x, s32 y, u32 *pixels) {
    RECT rect;
    rect.x = x;
    rect.y = y;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, pixels);
}

void setPaletteRecordColor(u16 index, u8 r, u8 g, u8 b) {
    PaletteRecordColor *color;
    if (index != 32) {
        color = &gSavedImageEntries[index].color;
        color->r = r;
        color->g = g;
        color->b = b;
        gSavedImageEntries[index].flags |= 4;
    }
}

void startPaletteCycle(u16 direction, u16 index, u8 first, u8 last, u8 interval) {
    PaletteRecordColor *color;
    if (index != 32) {
        color = &gSavedImageEntries[index].color;
        color->r = first;
        color->g = last;
        color->unk3 = interval;
        color->b = interval;
        if (direction == 0) {
            gSavedImageEntries[index].flags |= 0xA;
        } else {
            gSavedImageEntries[index].flags |= 0x12;
        }
    }
}

s32 startPaletteTransition(s16 x, s16 y, u32 *pixels, u16 duration) {
    u16 index = saveVram16Pixels(x, y);
    GameTask *task;
    PaletteTransitionResource *resource;
    if (index != 32) {
        if ((s16)func_800148B4(initializePaletteTransition) != 0) {
            D_8013AE8C->resource14 = allocateTaskResource();
            task = D_8013AE8C;
            resource = (PaletteTransitionResource *)&D_800D9504[task->resource14];
            resource->pixels = pixels;
            task->data1C = 1;
            task->data18 = duration;
            task->data1A = index;
            gSavedImageEntries[index].flags |= 2;
            return 1;
        }
        gSavedImageEntries[index].flags = 0;
    }
    return 0;
}

void requestSavedImageRestore(u16 index) {
    if (index != 32) {
        gSavedImageEntries[index].flags |= 0x40;
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/texture_upload", func_80016628);

void initializePaletteTransition(void) {
    PaletteTransitionState *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteTransitionState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1C == 0) {
        for (i = 0; i < 16; i++) {
            resource->red[i] = (gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] & 0x1F) << 8;
            resource->green[i] = (gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] & 0x3E0) << 3;
            resource->blue[i] = (gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] & 0x7C00) >> 2;
            resource->redStep[i] = (((resource->pixels[i] & 0x1F) << 8) - (s16)resource->red[i]) / (s16)D_8013AE88->data18;
            resource->greenStep[i] = (((resource->pixels[i] & 0x3E0) << 3) - (s16)resource->green[i]) / (s16)D_8013AE88->data18;
            resource->blueStep[i] = (((resource->pixels[i] & 0x7C00) >> 2) - (s16)resource->blue[i]) / (s16)D_8013AE88->data18;
            resource->mask[i] = resource->pixels[i] & 0x8000;
        }
        setCurrentTaskCallback(updatePaletteTransition);
    } else {
        D_8013AE88->data1C--;
    }
}

void updatePaletteTransition(void) {
    PaletteTransitionState *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteTransitionState *)&D_800D9504[D_8013AE88->resource14];
    if (--D_8013AE88->data18 != 0) {
        for (i = 0; i < 16; i++) {
            resource->red[i] += resource->redStep[i];
            resource->green[i] += resource->greenStep[i];
            resource->blue[i] += resource->blueStep[i];
            gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] =
                ((resource->red[i] & 0x1F00) >> 8) |
                ((resource->green[i] & 0x1F00) >> 3) |
                ((resource->blue[i] & 0x1F00) << 2) | resource->mask[i];
        }
        gSavedImageEntries[D_8013AE88->data1A].flags |= 0x20;
    } else {
        for (i = 0; i < 16; i++) {
            gSavedImageEntries[D_8013AE88->data1A].transitionPixels[i] = resource->pixels[i];
        }
        gSavedImageEntries[D_8013AE88->data1A].flags = 0x20;
        func_800149A8();
    }
}
