#include "../engine/resource_services_internal.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));

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
        index = i + 1;
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

u16 findFreeSavedImageEntry(void) {
    u16 i;
    for (i = 0; i < 32; i++) {
        if (gSavedImageEntries[i].flags == 0) {
            return i;
        }
    }
    return i;
}

void releaseSavedImageEntry(u16 index) {
    if (index != 0xFF) {
        gSavedImageEntries[index].flags = 0;
    }
}

u16 saveVram16Pixels(s32 x, s32 y) {
    u16 index = findFreeSavedImageEntry();
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
        if ((s16)tryCreateChildTaskWith20SlotsReserved(initializePaletteTransition) != 0) {
            D_8013AE8C->resource14 = allocateTaskMemoryBlock();
            task = D_8013AE8C;
            resource = (PaletteTransitionResource *)&gTaskMemoryPool[task->resource14];
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

void updatePaletteRecords(void) {
    u16 i;
    s16 j;

    for (i = 0; i < 16; i++) {
        if (gSavedImageEntries[i].flags == 0) {
            continue;
        }
        if (gSavedImageEntries[i].flags & 2) {
            for (j = 0; j < 16; j++) {
                gSavedImageEntries[i].transitionPixels[j] = gSavedImageEntries[i].pixels[j];
            }
            gSavedImageEntries[i].flags &= ~2;
        }
        if (gSavedImageEntries[i].flags & 4) {
            PaletteRecordColor *color = &gSavedImageEntries[i].color;
            for (j = 0; j < 16; j++) {
                s16 source = gSavedImageEntries[i].pixels[j];
                s32 green = source >> 5;
                s32 blue = source >> 10;
                s32 intensity = ((source & 31) + (green & 31) + (blue & 31)) / 3;
                s16 r, g, b;

                r = intensity;
                r += color->r;
                g = intensity + color->g;
                b = intensity + color->b;

                if (r > 31) {
                    r = 31;
                } else if (r < 0) {
                    r = 0;
                }
                if (g > 31) {
                    g = 31;
                } else if (g < 0) {
                    g = 0;
                }
                if (b > 31) {
                    b = 31;
                } else if (b < 0) {
                    b = 0;
                }
                gSavedImageEntries[i].transitionPixels[j] = r | (g << 5) | (b << 10);
            }
            gSavedImageEntries[i].flags = (gSavedImageEntries[i].flags & ~4) | 0x20;
        }
        if (gSavedImageEntries[i].flags & 0x18) {
            PaletteRecordColor *color = &gSavedImageEntries[i].color;
            if (--color->b == 0) {
                u16 pixel;
                color->b = color->unk3;
                if (gSavedImageEntries[i].flags & 8) {
                    pixel = gSavedImageEntries[i].transitionPixels[color->g];
                    for (j = color->g; j > color->r; j--) {
                        gSavedImageEntries[i].transitionPixels[j] = gSavedImageEntries[i].transitionPixels[j - 1];
                    }
                    gSavedImageEntries[i].transitionPixels[color->r] = pixel;
                } else {
                    pixel = gSavedImageEntries[i].transitionPixels[color->r];
                    for (j = color->r; j < color->g; j++) {
                        gSavedImageEntries[i].transitionPixels[j] = gSavedImageEntries[i].transitionPixels[j + 1];
                    }
                    gSavedImageEntries[i].transitionPixels[color->g] = pixel;
                }
                LoadImage(&gSavedImageEntries[i].rect, (u32 *)gSavedImageEntries[i].transitionPixels);
            }
        }
        if (gSavedImageEntries[i].flags & 0x20) {
            gSavedImageEntries[i].flags &= ~0x20;
            LoadImage(&gSavedImageEntries[i].rect, (u32 *)gSavedImageEntries[i].transitionPixels);
        }
        if (gSavedImageEntries[i].flags & 0x40) {
            gSavedImageEntries[i].flags &= ~0x40;
            LoadImage(&gSavedImageEntries[i].rect, (u32 *)gSavedImageEntries[i].pixels);
        }
    }
}

void initializePaletteTransition(void) {
    PaletteTransitionState *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PaletteTransitionState *)&gTaskMemoryPool[D_8013AE88->resource14];
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
    resource = (PaletteTransitionState *)&gTaskMemoryPool[D_8013AE88->resource14];
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
        deleteCurrentTaskSubtree();
    }
}
