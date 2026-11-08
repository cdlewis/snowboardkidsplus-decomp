#include "../race_presentation_internal.h"

void drawSlidingRaceTimeDisplay(SlidingTimeResource *resource) {
    u8 padding[0x20];
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    setSpriteColor(128, 128, 128);
    if (resource->hasExtraTime) {
        func_8006C998(resource->x - 102, resource->y, 0x1AE, 0, -1, ot, 1);
        drawRaceTime(&resource->time, resource->x + 38, resource->y, 1, ot, 1);
    } else {
        func_8006C998(resource->x - 98, resource->y, 0x1AD, 4, -1, ot, 1);
        drawRaceTime(&resource->time, resource->x + 34, resource->y, 2, ot, 1);
    }
}

void slideRaceTimeDisplayOut(void) {
    SlidingTimeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed += 4;
    if (resource->speed == 56) {
        func_800149A8();
    } else {
        drawSlidingRaceTimeDisplay(resource);
    }
}

void waitSpriteMotionTask(void) {
    SlidingTimeResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideRaceTimeDisplayOut);
    }
    drawSlidingRaceTimeDisplay(resource);
}

void slideRaceTimeDisplayIn(void) {
    SlidingTimeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed -= 4;
    if (resource->speed == 0) {
        resource->timer = 90;
        setCurrentTaskCallback(waitSpriteMotionTask);
    }
    drawSlidingRaceTimeDisplay(resource);
}

void initializeRaceTimeDifferenceDisplay(void) {
    SlidingTimeResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (SlidingTimeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->y = -44;
    resource->x = 420;
    resource->speed = 56;
    resource->hasExtraTime = calculateRaceTimeDifference(&D_8013B188, &D_80124050[D_8013AFCE], &resource->time);
    setCurrentTaskCallback(slideRaceTimeDisplayIn);
}

void drawFallingSpriteEffect(FallingSpriteEffect *effect) {
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex(effect->x >> 4, effect->y >> 4, effect->frame + 378,
                  effect->palette, -1, 0x7FFF, 0x7FFF, effect->scale,
                  effect->scale, 0, &D_800EBFF0[D_8013AF78], 2);
}

void updateDriftingFallingSpriteEffect(FallingSpriteEffect *effect) {
    effect->frame = (effect->frame + 1) & 3;
    if (effect->unk0E >= 0) {
        if (effect->unk10 != 0) {
            effect->velocityX += 4;
            effect->unk10--;
        } else {
            if ((effect->velocityX -= 4) == 0) {
                effect->unk10 = (randomNextMain() & 31) + 16;
                effect->unk0E = -1;
            }
        }
    } else {
        if (effect->unk10 != 0) {
            effect->velocityX -= 4;
            effect->unk10--;
        } else {
            if ((effect->velocityX += 4) == 0) {
                effect->unk10 = (randomNextMain() & 31) + 16;
                effect->unk0E = 1;
            }
        }
    }
    effect->x += effect->velocityX;
    effect->y += effect->velocityY;
    if (effect->y > 1760) {
        effect->active = 0;
    } else {
        drawFallingSpriteEffect(effect);
    }
}

void updateSwayingFallingSpriteEffect(FallingSpriteEffect *effect) {
    effect->frame = (effect->frame + 1) & 3;
    effect->y += effect->velocityY;
    effect->x += rsin(effect->unk0E = (effect->unk0E + 32) & 0xFFF) * (effect->unk08 + 48) / 4096;
    if (effect->unk0E == 0 || effect->unk0E == 0x800) {
        effect->unk08 = randomNextMain() & 15;
    }
    if (effect->y > 1760) {
        effect->active = 0;
    } else {
        drawFallingSpriteEffect(effect);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80033C2C);

void initializeFallingSpriteEffectsTask(void) {
    FallingSpriteEffect *effects;
    s16 i;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    effects = (FallingSpriteEffect *)&D_800D9504[D_8013AE88->resource14];
    for (i = 0; i < 10; i++) {
        effects[i].active = 0;
    }
    setCurrentTaskCallback(func_80033C2C);
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80033FAC);

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80033FE8);

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80034028);

void drawSlidingDisplaySprite(SlidingDisplayResource *resource) {
    s32 x = (resource->position >> 1) - 56;
    if (gameOptionPlayerModeAbsolute == 1) {
        func_8006C998(x, 8, 0x1DF, 6, -1, &D_800EC018[D_8013AF78], 1);
    } else if (D_8013AE88->data18 == 0) {
        if (func_8003D904(0)) {
            func_8006C998(x, 8, 0x1DF, 6, -1, &D_800EC018[D_8013AF78], 1);
        }
    } else {
        if (func_8003D904(1)) {
            func_8006C998(x, 8, 0x1DF, 6, -1, &D_800EC0B8[D_8013AF78], 1);
        }
    }
}

void slideOutSlidingDisplayTask(void) {
    SlidingDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->position += resource->velocity;
    resource->velocity += 4;
    if (resource->velocity == 56) {
        func_800149A8();
    } else {
        drawSlidingDisplaySprite(resource);
    }
}

void waitSlidingDisplayTask(void) {
    SlidingDisplayResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutSlidingDisplayTask);
    }
    drawSlidingDisplaySprite(resource);
}

void slideInSlidingDisplayTask(void) {
    SlidingDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->timer != 0) {
        resource->timer--;
    } else {
        resource->position += resource->velocity;
        resource->velocity -= 4;
        if (resource->velocity == 0) {
            resource->timer = 30;
            setCurrentTaskCallback(waitSlidingDisplayTask);
        }
    }
    drawSlidingDisplaySprite(resource);
}

void initializeTaskMotionResource(void) {
    TaskMotionResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskMotionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->velocity = 60;
    resource->position = -480;
    resource->delay = 10;
    setCurrentTaskCallback(slideInSlidingDisplayTask);
}

void drawPlayerSpriteAnimation(SpriteAnimationResource *resource) {
    GsOT *ot;
    s32 palette;
    if (resource->checkPlayer == 1) {
        if (!func_8003D904(resource->player)) return;
    }
    if (resource->player == 0) {
        ot = &D_800EC018[D_8013AF78];
    } else {
        ot = &D_800EC0B8[D_8013AF78];
    }
    setSpriteColor(128, 128, 128);
    palette = 3;
    if (!resource->alternateLayer) palette = 2;
    func_8006C998(resource->x, resource->y, (resource->frame >> 1) + 0x1E2,
                 palette, -1, ot, 0);
}

void advancePlayerSpriteAnimationTask(void) {
    SpriteAnimationResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteAnimationResource *)&D_800D9504[D_8013AE88->resource14];
    if (++resource->frame >= 16) {
        func_800149A8();
    } else {
        drawPlayerSpriteAnimation(resource);
    }
}

void createPlayerSpriteAnimationTask(s32 x, s32 y, s32 player, s32 checkPlayer, u16 alternateLayer) {
    SpriteAnimationResource *resource;
    func_80014874(advancePlayerSpriteAnimationTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (SpriteAnimationResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = x - 8;
    resource->y = y - 8;
    resource->player = player;
    resource->checkPlayer = checkPlayer;
    resource->alternateLayer = alternateLayer;
    resource->frame = 0xFFFF;
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80034664);

void slideOutPlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x += resource->speed;
    resource->speed += 4;
    if (resource->speed >= 56) {
        func_800149A8();
    } else {
        func_80034664(resource);
    }
}

void waitPlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutPlayerNumberDisplayTask);
    }
    func_80034664(resource);
}

void slideInPlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed -= 4;
    if (resource->speed <= 0) {
        resource->speed = 0;
        resource->timer = 45;
        setCurrentTaskCallback(waitPlayerNumberDisplayTask);
    }
    func_80034664(resource);
}

void initializePlayerNumberDisplayTask(void) {
    PlayerNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->y = -44;
    resource->speed = 56;
    if (D_8013AFE2 == 0 && D_8013AFCD == 2) {
        resource->x = 452;
    } else if (D_8013AFE2 == 1 && D_8013AFE0 == 2 && D_8013AFCD == 2) {
        resource->x = 460;
    } else {
        resource->x = 420;
    }
    setCurrentTaskCallback(slideInPlayerNumberDisplayTask);
}

void createPlayerNumberDisplayTask(s32 value, s32 showExtraValue, s32 player) {
    PlayerNumberResource *resource;
    func_80014874(initializePlayerNumberDisplayTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PlayerNumberResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->value = value;
    resource->showExtraValue = showExtraValue;
    resource->player = player;
}

void drawSpriteMotionDisplay(SpriteMotionResource *resource) {
    s16 draw = 0;
    GsOT *ot;
    s16 depth = 1;
    setSpriteColor(128, 128, 128);
    if (D_8013AFCD == 1) {
        draw = 1;
        ot = &D_800EC018[D_8013AF78];
    } else if (resource->variant == 0) {
        if (func_8003D904(0)) {
            draw = 1;
            ot = &D_800EC018[D_8013AF78];
        }
    } else {
        if (func_8003D904(1)) {
            draw = 1;
            ot = &D_800EC0B8[D_8013AF78];
        }
    }
    if (draw) {
        if (D_8013AFCD == 1) {
            if (D_8013AFE2 == 1 && D_8013AFE0 == 2) {
                func_8006C998(-60, resource->unk08, 0x1AB, 8, -1, ot, depth);
            } else {
                func_8006C998(-36, resource->unk08, 0x1AB, 8, -1, ot, depth);
            }
        } else {
            func_8006C998(-30, (s16)(resource->unk08 + 32), 0x1AB, 8, -1, ot, depth);
        }
    }
}

void slideOutSpriteMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk04 += resource->unk14;
    resource->unk14 += 4;
    if (resource->unk14 >= 56) {
        func_800149A8();
    } else {
        drawSpriteMotionDisplay(resource);
    }
}

void waitSpriteExitMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutSpriteMotionTask);
    }
    drawSpriteMotionDisplay(resource);
}

void slideInSpriteMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    resource->unk04 -= resource->unk14;
    resource->unk14 -= 4;
    if (resource->unk14 <= 0) {
        resource->unk14 = 0;
        resource->timer = 45;
        setCurrentTaskCallback(waitSpriteExitMotionTask);
    }
    drawSpriteMotionDisplay(resource);
}

void initializeSpriteMotionTask(void) {
    SpriteMotionResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SpriteMotionResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AFCD == 1) {
        resource->unk08 = 8;
    } else {
        resource->unk08 = -8;
    }
    resource->unk14 = 0x38;
    resource->unk04 = 0x1A4;
    setCurrentTaskCallback(slideInSpriteMotionTask);
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_8003504C);

void drawSlidingNumberDisplay(SlidingNumberResource *resource) {
    s16 visible = 0;
    GsOT *ot;
    setSpriteColor(128, 128, 128);
    if (D_8013AFCD == 1) {
        visible = 1;
        ot = &D_800EC018[D_8013AF78];
    } else if (resource->player == 0) {
        if (func_8003D904(0)) {
            visible = 1;
            ot = &D_800EC018[D_8013AF78];
        }
    } else {
        if (func_8003D904(1)) {
            visible = 1;
            ot = &D_800EC0B8[D_8013AF78];
        }
    }
    if (visible) {
        func_8006DFB4((s16)(resource->x - 8), (s16)(resource->y - 8), 5, 0,
                     resource->value, 3, -1, ot, 1);
        func_8006C998((s16)(resource->x - 60), (s16)(resource->y - 4), 0x1B4, 4, -1, ot, 1);
        func_8006C998((s16)(resource->x + 44), (s16)(resource->y - 4), 0x1B5, 4, -1, ot, 1);
    }
}

void slideOutSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x += resource->speed;
    resource->speed += 4;
    if (resource->speed >= 56) {
        func_800149A8();
    } else {
        drawSlidingNumberDisplay(resource);
    }
}

void waitSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->timer == 0) {
        setCurrentTaskCallback(slideOutSlidingNumberDisplayTask);
    }
    drawSlidingNumberDisplay(resource);
}

void slideInSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    resource->x -= resource->speed;
    resource->speed -= 4;
    if (resource->speed <= 0) {
        resource->speed = 0;
        resource->timer = 45;
        setCurrentTaskCallback(waitSlidingNumberDisplayTask);
    }
    drawSlidingNumberDisplay(resource);
}

void initializeSlidingNumberDisplayTask(void) {
    SlidingNumberResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingNumberResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AFCD == 1) {
        resource->y = -36;
    } else {
        resource->y = -22;
    }
    resource->speed = 56;
    if (D_8013AFE2 == 0 && D_8013AFCD == 2) {
        resource->x = 452;
    } else if (D_8013AFE2 == 1 && D_8013AFE0 == 2 && D_8013AFCD == 2) {
        resource->x = 460;
    } else {
        resource->x = 420;
    }
    setCurrentTaskCallback(slideInSlidingNumberDisplayTask);
}

void createActivePlayerSlidingNumberTask(s32 value, s16 player) {
    SlidingNumberResource *resource;
    if (gRacePlayers[player].isActive != 0) {
        func_80014874(initializeSlidingNumberDisplayTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SlidingNumberResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->value = value;
        resource->player = player;
    }
}

void drawFallingPairedVisualTask(void) {
    FallingVisualResource *resource;
    GsCOORDINATE2 *base;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    base = (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource16];
    resource->coordinate.coord.t[0] = base->coord.t[0];
    resource->coordinate.coord.t[1] = base->coord.t[1] + (resource->height >> 8);
    resource->coordinate.coord.t[2] = base->coord.t[2];
    resource->framePosition = 0;
    setTaskVisualFrame((TaskVisualResource *)resource, gFallingPairedVisualFrames, 0x1E00);
    drawCameraFacingTaskVisual((TaskVisualResource *)resource, 0);
    resource->coordinate.coord.t[1] += 0x49;
    resource->framePosition = gFallingPairedVisualAnimation[resource->animationTick >> 1] << 8;
    setTaskVisualFrame((TaskVisualResource *)resource, gFallingPairedVisualFrames, 0x1E00);
    drawCameraFacingTaskVisual((TaskVisualResource *)resource, 0);
}

void updateFallingPairedVisualTask(void) {
    FallingVisualResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (resource->delay > 0) {
            resource->delay--;
        } else {
            resource->height += resource->velocity;
            resource->velocity -= 30;
            if (resource->height <= -48000 && resource->velocity < 0) {
                if (!resource->triggered) {
                    playSoundEffect(1, 0x600, 0x20);
                    resource->triggered = 1;
                }
            }
            if (resource->triggered) {
                if (resource->animationTick < 11) {
                    resource->animationTick++;
                }
            }
        }
    }
    drawFallingPairedVisualTask();
}

void initializeBouncingVisualTask(void) {
    BouncingVisualResource *resource;
    GsCOORDINATE2 *position;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (BouncingVisualResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    position = (GsCOORDINATE2 *)&D_800D9504[D_8013AE88->resource16];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->r0 = resource->g0 = resource->b0 = 128;
    resource->code = POLY_FT4_CODE;
    resource->timer = 0;
    resource->framePosition = 0;
    if (D_8013AFD2 == 1) {
        position->coord.t[0] = gRacePlayers[0].worldCoordinate.coord.t[0];
    } else {
        position->coord.t[0] = 0;
    }
    position->coord.t[1] = gRacePlayers[0].worldCoordinate.coord.t[1];
    position->coord.t[2] = gRacePlayers[0].worldCoordinate.coord.t[2] + 50;
    resource->offsetY = -37120;
    resource->velocity = 0;
    resource->frame = 0;
    do {
        resource->velocity += 30;
        resource->offsetY -= resource->velocity;
    } while (resource->offsetY > -89600);
    resource->soundPlayed = 0;
    resource->delay = 15;
    setCurrentTaskCallback(updateFallingPairedVisualTask);
}

void drawFixedSpriteAndBottomBarTask(void) {
    addTexturedSprite(D_8013AF6C, 0x1C, 0x3DFE, 0x28, 0x52,
                      0x68, 0x18, 8, 0xD8, 0x80, 0x80, 0x80);
    addFlatRectangle(D_8013AF6C + 1, -(D_8013AF7A >> 1),
                     (s16)((D_8013AF7C >> 1) - 0x20),
                     (s16)D_8013AF7A, 0x20, 0, 0, 0);
}

void drawDemonstrationOverlay(void) {
    setSpriteColor(128, 128, 128);
    drawUppercaseProportionalText(-52, -8, 20, gDemonstrationText, 1, -1, &D_800EBFF0[D_8013AF78], 3);
    if (gFrameCounter & 8) {
        drawUppercaseProportionalText(-40, 28, 20, gDemonstrationStartText, 2, -1, &D_800EBFF0[D_8013AF78], 3);
    }
}

void drawAutoPilotLabelTask(void) {
    s32 frame;
    if (!(D_8013B0D8->unk128 & 0x40) && !D_8013AFD6) {
        setSpriteColor(128, 128, 128);
        if (D_8013AFCD == 1) {
            frame = gFrameCounter & 2;
            drawUppercaseProportionalText(-40, -18, 20, gAutoPilotLabel,
                         (frame / 2) + 1, -1,
                         &D_800EBFF0[D_8013AF78], 3);
        } else if (D_8013AE88->data18 == 0) {
            frame = gFrameCounter & 2;
            drawUppercaseProportionalText(-72, -66, 20, gAutoPilotLabel,
                         (frame / 2) + 1, -1,
                         &D_800EBFF0[D_8013AF78], 3);
        } else {
            frame = gFrameCounter & 2;
            drawUppercaseProportionalText(-72, 46, 20, gAutoPilotLabel,
                         (frame / 2) + 1, -1,
                         &D_800EBFF0[D_8013AF78], 3);
        }
    }
}

void checkControllerInputSequenceTask(void) {
    if (D_8013AEE8[0] != 0) {
        if (D_8013AEE8[0] == gControllerInputSequences[D_8013AE88->data18][D_8013AE88->data1A]) {
            D_8013AE88->data1A++;
            if (gControllerInputSequences[D_8013AE88->data18][D_8013AE88->data1A] == 0xFFFF) {
                D_8013B11A |= 1 << D_8013AE88->data18;
                playSoundEffect(0, 0, 0x20);
                func_800149A8();
            }
        } else {
            D_8013AE88->data1A = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80035CF8);

void drawResultDisplayLabels(ResultDisplayResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    func_8006C998(-104, -44, 392, 6, -1, &D_800EBFF0[D_8013AF78], 1);
    func_8006C998((s16)(-104 - gResultLabelXOffsets[resource->unk08]), 6,
                  resource->unk08 + 382, gResultLabelPalettes[resource->unk08], -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_800401DC(-66, -12, resource->unk06, &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(-34, 4, resource->unk06 + 393, gResultIconPalettes[resource->unk06], -1,
                  &D_800EBFF0[D_8013AF78], 1);
}

void drawResultNumberAndIcon(ResultDisplayResource *resource) {
    setSpriteColor(128, 128, 128);
    func_8006DFB4(32, -63, 5, 0, resource->unk04, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, -63, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawFirstPlayerResultCounterAndIcon(ResultDisplayResource *resource) {
    s32 palette = 1;
    if (resource->unk06 == 3) {
        palette = 4;
    }
    setSpriteColor(128, 128, 128);
    if (resource->unk06 == 3) {
        func_8006C998(24, -11, 397, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    }
    func_8006DFB4(32, -15, 5, 0, gRacePlayers[0].unk01C, palette, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, -15, 26, palette, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawFirstPlayerResultNumberAndIcon(ResultDisplayResource *resource) {
    setSpriteColor(128, 128, 128);
    func_8006DFB4(24, 33, 6, 0, gRacePlayers[0].money, 1, -1,
                  &D_800EBFF0[D_8013AF78], 1);
    func_8006C998(72, 33, 26, 1, -1, &D_800EBFF0[D_8013AF78], 1);
}

void drawResultPrizeLabels(ResultDisplayResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness,
                       (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    drawLimitedText8x8(8, -72, 12, gResultRankPrizeLabel, 3, -1,
                      &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(0, -24, 12, gResultGetCoinLabel, 3, -1,
                      &D_800EBFF0[D_8013AF78], 1);
    drawLimitedText8x8(4, 24, 13, gResultTotalMoneyLabel, 3, -1,
                      &D_800EBFF0[D_8013AF78], 1);
}

void waitResultDisplayTaskFlagsTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if ((D_8013AFE4 & 0x50) != 0x10) {
        func_800149A8();
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawFirstPlayerResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultFlagsInputTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AEE8[0] & 0xB0) {
        D_8013AFE4 |= 0x10;
        setCurrentTaskCallback(waitResultDisplayTaskFlagsTask);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawFirstPlayerResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80036454);

void waitResultDisplayInputTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        resource->timer = 0;
        setCurrentTaskCallback(func_80036454);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawFirstPlayerResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultDisplayConfirmationTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(waitResultDisplayInputTask);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawFirstPlayerResultCounterAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultNumberDisplayTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        playSoundEffect(0, 0x700, 0x20);
        resource->timer = 20;
        setCurrentTaskCallback(waitResultDisplayConfirmationTask);
    }
    drawResultDisplayLabels(resource);
    drawResultNumberAndIcon(resource);
    drawResultPrizeLabels(resource);
}

void waitResultDisplayTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->timer--;
    if (D_8013AEE8[0] & 0xB0) {
        resource->timer = 0;
    }
    if (resource->timer == 0) {
        resource->timer = 20;
        playSoundEffect(0, 0x700, 0x20);
        setCurrentTaskCallback(waitResultNumberDisplayTask);
    }
    drawResultDisplayLabels(resource);
    drawResultPrizeLabels(resource);
}

void fadeInResultDisplayTask(void) {
    ResultDisplayResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 20;
        resource->brightness = 128;
        setCurrentTaskCallback(waitResultDisplayTask);
    }
    drawResultDisplayLabels(resource);
    drawResultPrizeLabels(resource);
}

void initializeResultDisplayTask(void) {
    ResultDisplayResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ResultDisplayResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    resource->unk06 = (s8)gRacePlayers[0].unk935;
    resource->unk04 = 0;
    if (resource->unk06 < 3) {
        resource->unk04 = D_8012FC6C[D_8013AFCE][resource->unk06];
    }
    gRacePlayers[0].money = D_80130BF0[0];
    resource->unk08 = gRacePlayers[0].characterId;
    setCurrentTaskCallback(fadeInResultDisplayTask);
}

INCLUDE_ASM("asm/nonmatchings/race/ui/race_panels", func_80036BA4);

void drawPlayerResultRowValues(s16 row, s16 x, s16 y, TaskFadeResource *resource) {
    char text[32];
    s32 length;
    s32 mode;
    if (row != 0 || (gFrameCounter & 1)) {
        mode = 6;
    } else {
        mode = 0;
    }
    sprintf(text, gResultValueFormat, gRacePlayers[resource->playerOrder[0]].unk02A);
    length = strlen(text);
    drawLimitedText8x8(x + 104, y + 24, 1, gResultValueMultiplierLabel, mode, -1, &D_800EBFF0[D_8013AF78], 1);
    func_8006D66C(x + 112, y + 24, length, 0, gRacePlayers[resource->playerOrder[row]].unk02A, mode, -1, &D_800EBFF0[D_8013AF78], 1);
    if (gRacePlayers[resource->playerOrder[row]].unk008 == 0) {
        func_8006D66C(x + 88, y + 6, 1, 0, gRacePlayers[resource->playerOrder[row]].playerIndex + 1, 5, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(x + 96, y + 6, 1, gPlayerNumberSuffixLabel, 5, -1, &D_800EBFF0[D_8013AF78], 1);
    } else {
        drawLimitedText8x8(x + 88, y + 6, 3, gComputerPlayerLabel, 4, -1, &D_800EBFF0[D_8013AF78], 1);
    }
}

void drawPlayerResultRows(TaskFadeResource *resource) {
    if (resource->brightness != 128) {
        setSpriteColor((u8)resource->brightness, (u8)resource->brightness, (u8)resource->brightness);
    } else {
        setSpriteColor(128, 128, 128);
    }
    if (resource->playerOrder[0] != -1) {
        drawPlayerResultRowValues(0, -136, -80, resource);
    }
    if (resource->playerOrder[1] != -1) {
        drawPlayerResultRowValues(1, 8, -48, resource);
    }
    if (resource->playerOrder[2] != -1) {
        drawPlayerResultRowValues(2, -136, -16, resource);
    }
    if (resource->playerOrder[3] != -1) {
        drawPlayerResultRowValues(3, 8, 16, resource);
    }
}

void updatePlayerResultOrder(TaskFadeResource *resource) {
    s32 i, j;
    s32 temp;
    for (i = 0; i < D_8013AFD2; i++) {
        resource->playerOrder[i] = i;
    }
    for (i = 0; i < D_8013AFD2 - 1; i++) {
        for (j = i + 1; j < D_8013AFD2; j++) {
            if (gRacePlayers[resource->playerOrder[i]].unk935 >
                gRacePlayers[resource->playerOrder[j]].unk935) {
                temp = resource->playerOrder[i];
                resource->playerOrder[i] = resource->playerOrder[j];
                resource->playerOrder[j] = temp;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (!(gRacePlayers[resource->playerOrder[i]].unk128 & 0x40)) {
            resource->playerOrder[i] = -1;
        }
    }
}

void drawCurrentTaskPlayerResults(void) {
    TaskResourceRecord *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    if (!(D_8013AFE4 & 0x10)) {
        func_800149A8();
    }
    updatePlayerResultOrder((TaskFadeResource *)resource);
    func_80036BA4(resource);
    drawPlayerResultRows((TaskFadeResource *)resource);
}

void waitPlayerResultsInputTask(void) {
    s32 player;
    TaskResourceRecord *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = &D_800D9504[D_8013AE88->resource14];
    for (player = 0; player < gameOptionPlayerModeAbsolute; player++) {
        if (D_8013AEE8[player] & 0xB0) {
            D_8013AFE4 |= 0x10;
            setCurrentTaskCallback(drawCurrentTaskPlayerResults);
        }
    }
    updatePlayerResultOrder((TaskFadeResource *)resource);
    func_80036BA4(resource);
    drawPlayerResultRows((TaskFadeResource *)resource);
}

void fadeInPlayerResultsTask(void) {
    TaskFadeResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness += 8;
    if (resource->brightness > 128) {
        resource->timer = 150;
        resource->brightness = 128;
        setCurrentTaskCallback(waitPlayerResultsInputTask);
    }
    updatePlayerResultOrder((TaskFadeResource *)resource);
    func_80036BA4((TaskResourceRecord *)resource);
    drawPlayerResultRows(resource);
}

void initializeTaskFade(void) {
    TaskFadeResource *resource;

    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskFadeResource *)&D_800D9504[D_8013AE88->resource14];
    resource->brightness = 0;
    setCurrentTaskCallback(fadeInPlayerResultsTask);
}
