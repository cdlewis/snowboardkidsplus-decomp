#include "../scene/scene_event_tasks_internal.h"

void startUnlockNotification(s16 index) {
    if ((u16)index < 11 && (s16)getUnlockNotificationFlag(index) == 0) {
        D_8013AF3A = index;
        D_8013AFE4 &= ~0x200;
        setUnlockNotificationFlag(index);
        func_80014874(initializeScenePlaybackResourcesTask);
    } else {
        D_8013AFE4 |= 0x200;
    }
}

void drawPagedTexturedRectangle(s16 x, s16 y, s16 width, s16 height, s16 tpage,
                   s16 clut, GsOT *ot, u16 depth) {
    DrawModeSpritePacket *packet = (DrawModeSpritePacket *)0x1F800000;
    s16 remainingWidth;
    s16 textureDepth;
    u16 initialTpage;
    packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
    packet->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    packet->tpage = tpage;
    setRGB0(packet, gTexturedSpriteTemplate.r0, gTexturedSpriteTemplate.g0, gTexturedSpriteTemplate.b0);
    packet->code = TEXTURED_SPRITE_CODE;
    packet->y0 = y;
    packet->u0 = 0;
    packet->v0 = 0;
    initialTpage = tpage;
    textureDepth = (initialTpage >> 7) & 3;
    packet->clut = clut;
    while (height > 0) {
        if (height > 256) {
            packet->h = 256;
            height -= 256;
        } else {
            packet->h = height;
            height = 0;
        }
        remainingWidth = width;
        packet->x0 = x;
        while (remainingWidth > 0) {
            if (remainingWidth > 256) {
                packet->w = 256;
                remainingWidth -= 256;
            } else {
                packet->w = remainingWidth;
                remainingWidth = 0;
            }
            addTexturedSpritePacket((TexturedSpritePacket *)packet, ot, depth);
            packet->x0 += 256;
            switch (textureDepth) {
            case 0: packet->tpage += 1; break;
            case 1: packet->tpage += 2; break;
            case 2: packet->tpage += 4; break;
            }
        }
        packet->tpage = initialTpage + 16;
        packet->y0 += 256;
    }
}
