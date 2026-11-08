#include "../../graphics/render_effects_internal.h"

const FadingTextGlyphBanks gFadingTextGlyphBanks
    __attribute__((section(".rodata.fading_text_glyph_banks"))) = {
    {D_8012EC04, D_8012ECC8, D_8012EECC, D_8012F0D0}
};

PlayerRenderSnapshot gPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots"))) = {0};

PlayerRenderSnapshot gPreviousPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots"))) = {0};

MATRIX gPlayerRenderIdentityMatrix
    __attribute__((section(".data.player_render_snapshots"))) = {0};

POLY_FT4 gPlayerSnapshotTrailPacket
    __attribute__((section(".data.player_snapshot_trail"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 0, 0, 0,
    TEXTURED_QUAD_CODE | SEMI_TRANSPARENT_FLAG,
    0, 0, 64, 0, getClut(880, 243),
    0, 0, 127, 0, getTPage(0, 2, 704, 0),
    0, 0, 64, 31, 0,
    0, 0, 127, 31, 0,
};

SVECTOR gPlayerSnapshotTrailEndpoints[2]
    __attribute__((section(".data.player_snapshot_trail"))) = {
    {0, 0, -20, 0}, {0, 0, 20, 0},
};

VECTOR gPlayerPairedQuadOffsets[2]
    __attribute__((section(".data.player_paired_quad_offsets"))) = {
    {-50, 0, 0, 0}, {50, 0, 0, 0}
};

SVECTOR gPlayerTripleQuadOffsets[3]
    __attribute__((section(".data.player_triple_quad_offsets"))) = {
    {-40, 0, 0, 0}, {40, 0, 0, 0}, {0, 0, 0, 0}
};

POLY_FT4 gQuadShrinkPacket __attribute__((section(".data.quad_shrink"))) = {
    0x09000000, 0x80, 0x80, 0x80, 0x2C,
    0, 0, 0, 0x10, getClut(944, 240),
    0, 0, 0x27, 0x10, getTPage(0, 0, 960, 0),
    0, 0, 0, 0x1F, 0,
    0, 0, 0x27, 0x1F, 0
};

SVECTOR gQuadShrinkVertices[4] __attribute__((section(".data.quad_shrink"))) = {
    {-144, 24, 0, 0}, {-104, 24, 0, 0},
    {-144, 39, 0, 0}, {-104, 39, 0, 0}
};

SVECTOR gRightQuadShrinkVertices[4] __attribute__((section(".data.right_quad_vertices"))) = {
    {-64, 24, 0, 0}, {-24, 24, 0, 0},
    {-64, 39, 0, 0}, {-24, 39, 0, 0}
};

POLY_FT4 gRightQuadShrinkPacket __attribute__((section(".data.right_quad_packet"))) = {
    0x09000000, 0x80, 0x80, 0x80, 0x2C,
    0, 0, 0x50, 0x10, getClut(944, 240),
    0, 0, 0x77, 0x10, getTPage(0, 0, 960, 0),
    0, 0, 0x50, 0x1F, 0,
    0, 0, 0x77, 0x1F, 0
};

QuadAnimationFrame gQuadAnimationFrames[5] __attribute__((section(".data.quad_animation"))) = {
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 0, 184, 31, 31},
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 32, 184, 31, 31},
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 64, 184, 31, 31},
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 96, 184, 31, 31},
    {-1, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 96, 184, 31, 31}
};

QuadAnimationStep gAlternateQuadAnimationSteps[5]
    __attribute__((section(".data.alternate_quad_steps"))) = {
    {2, 8192, 128, 128, 128, 0,
     getTPage(0, 0, 704, 0), getClut(880, 250), 0, 192, 63, 63},
    {2, 8192, 128, 128, 128, 0,
     getTPage(0, 0, 704, 0), getClut(880, 250), 64, 192, 63, 63},
    {2, 8192, 255, 255, 255, SEMI_TRANSPARENT_FLAG,
     getTPage(0, 0, 704, 0), getClut(880, 250), 128, 192, 63, 63},
    {2, 8192, 255, 255, 255, SEMI_TRANSPARENT_FLAG,
     getTPage(0, 0, 704, 0), getClut(880, 250), 192, 192, 63, 63},
    {-1, 8192, 255, 255, 255, SEMI_TRANSPARENT_FLAG,
     getTPage(0, 0, 704, 0), getClut(880, 250), 96, 192, 63, 63},
};

QuadVertex gMiddleShrinkQuadVertices[4]
    __attribute__((section(".data.middle_shrink_vertices"))) = {
    {-104, 24, 0, 0}, {-64, 24, 0, 0},
    {-104, 39, 0, 0}, {-64, 39, 0, 0},
};

POLY_FT4 gMiddleShrinkQuadPacket
    __attribute__((section(".data.middle_shrink_packet"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 128, 128, 128, TEXTURED_QUAD_CODE,
    0, 0, 40, 16, getClut(944, 240),
    0, 0, 79, 16, getTPage(0, 0, 960, 0),
    0, 0, 40, 31, 0,
    0, 0, 79, 31, 0,
};

POLY_FT4 gSlidingQuadPacket
    __attribute__((section(".data.sliding_quad_packet"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 128, 128, 128, TEXTURED_QUAD_CODE,
    160, 24, 0, 0, 0,
    320, 24, 159, 0, getTPage(0, 0, 960, 0),
    160, 39, 0, 0, 0,
    159, 39, 159, 0, 0,
};

PaletteColor gCapturedColorCyclePalette[16]
    __attribute__((section(".data.color_cycle_palette"), aligned(4))) = {0};

u16 gWhitePulsePalette[16]
    __attribute__((section(".bss.white_pulse_palette"), aligned(4)));

RECT gWhitePulsePaletteRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};

RECT gColorCycleCaptureRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};

void updatePlayerRenderSnapshots(void) {
    GsCOORDINATE2 coordinate;
    u32 i;
    RacePlayer *player;
    GsInitCoordinate2(0, &coordinate);
    GsGetLs(&coordinate, &gPlayerRenderIdentityMatrix);
    for (i = 0; i < 2; i++) {
        player = &gRacePlayers[i];
        gPreviousPlayerRenderSnapshots[i] = gPlayerRenderSnapshots[i];
        gPlayerRenderSnapshots[i].position.vx = player->x >> 8;
        gPlayerRenderSnapshots[i].position.vy = player->y >> 8;
        gPlayerRenderSnapshots[i].position.vz = player->z >> 8;
        gPlayerRenderSnapshots[i].unk10 = player->unk0B4;
        gPlayerRenderSnapshots[i].unk12 = player->unk0B6;
        gPlayerRenderSnapshots[i].unk14 = player->unk0B8;
        gPlayerRenderSnapshots[i].unk18 = player->unk0BE;
    }
}

void startRacePlayerTasks(void) {
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < D_8013AFD2; playerIndex++) {
        func_80064754(playerIndex);
        func_80065464(playerIndex);
        createPlayerTrailTask(playerIndex);
        func_80067D88(playerIndex);
        func_800688C0(playerIndex);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80060FE4);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800612BC);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80061304);

void drawPlayerSnapshotTrailTask(void) {
    s32 p, flag;
    PlayerSnapshotTrailResource *resource;
    POLY_FT4 *packet;
    MATRIX *scratch = (MATRIX *)0x1F800000;
    s32 depth;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerSnapshotTrailResource *)&D_800D9504[D_8013AE88->resource14];
    packet = &gPlayerSnapshotTrailPacket;
    packet->r0 = resource->remainingFrames * 48 / 90;
    packet->g0 = resource->remainingFrames * 32 / 90;
    packet->b0 = resource->remainingFrames * 32 / 90;
    GsGetLs(&resource->current, scratch);
    ScaleMatrix(scratch, &resource->currentScale);
    GsSetLsMatrix(scratch);
    depth = RotTransPers(&gPlayerSnapshotTrailEndpoints[0], (s32 *)&packet->x0, &p, &flag);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[1], (s32 *)&packet->x1, &p, &flag);
    GsGetLs(&resource->previous, scratch);
    ScaleMatrix(scratch, &resource->previousScale);
    GsSetLsMatrix(scratch);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[0], (s32 *)&packet->x2, &p, &flag);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[1], (s32 *)&packet->x3, &p, &flag);
    if (depth >= 0 && flag >= 0) {
        addCopiedPrimitive(&D_8013AF6C[depth >> 4], (u32 *)packet);
    }
    if (--resource->remainingFrames == -1 || gPlayerSnapshotTrailPacket.y0 > 160) {
        func_800149A8();
        D_8013B300--;
    }
}

void initializePlayerSnapshotTrailTask(void) {
    PlayerSnapshotTrailResource *resource;
    SVECTOR rotation;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PlayerSnapshotTrailResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->current);
    GsInitCoordinate2(0, &resource->previous);
    rotation.vx = gPlayerRenderSnapshots[D_8013AE88->data1C].unk10;
    rotation.vy = gPlayerRenderSnapshots[D_8013AE88->data1C].unk18 + 0x400;
    rotation.vz = gPlayerRenderSnapshots[D_8013AE88->data1C].unk14;
    resource->currentScale.vx = 0x1000;
    resource->currentScale.vy = 0x1000;
    resource->currentScale.vz = (((s16)gPlayerRenderSnapshots[D_8013AE88->data1C].unk18 &
        (0x3FF - (s16)gPlayerRenderSnapshots[D_8013AE88->data1C].unk12) & 0x3FF) << 2) + 0x1000;
    RotMatrix(&rotation, &resource->current.coord);
    resource->current.coord.t[0] = gPlayerRenderSnapshots[D_8013AE88->data1C].position.vx;
    resource->current.coord.t[1] = gPlayerRenderSnapshots[D_8013AE88->data1C].position.vy;
    resource->current.coord.t[2] = gPlayerRenderSnapshots[D_8013AE88->data1C].position.vz;
    rotation.vx = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk10;
    rotation.vy = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk18 + 0x400;
    rotation.vz = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk14;
    resource->previousScale.vx = 0x1000;
    resource->previousScale.vy = 0x1000;
    resource->previousScale.vz = (((s16)gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk18 &
        (0x3FF - (s16)gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk12) & 0x3FF) << 2) + 0x1000;
    RotMatrix(&rotation, &resource->previous.coord);
    resource->previous.coord.t[0] = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].position.vx;
    resource->previous.coord.t[1] = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].position.vy;
    resource->previous.coord.t[2] = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].position.vz;
    resource->remainingFrames = D_8013AE88->data1A;
    setCurrentTaskCallback(drawPlayerSnapshotTrailTask);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800618A0);

void startIndexedRenderTask(s32 index) {
    TaskResourceRecord *resource;
    func_80014930(D_8013B304, func_800618A0);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80061E48);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80062354);

void startIndexedRenderTaskPair(s32 index) {
    TaskResourceRecord *resource;
    func_80014930(D_8013B304, func_80061E48);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
    func_80014930(D_8013B304, func_80062354);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
}

void drawFallingParticlesTask(void) {
    s32 i;
    FallingParticleResource *resource;
    SPRT_8 *packet;
    SPRT_8 *scratch;
    DR_TPAGE *page;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (SPRT_8 *)0x1F800000;
    resource = (FallingParticleResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (SPRT_8 *)GsOUT_PACKET_P;
    setcode(scratch, SPRT_8_CODE);
    setRGB0(scratch, 128, 128, 128);
    scratch->v0 = 232;
    for (i = 0; i < 40; i++) {
        scratch->x0 = (resource->x[i] + rcos(resource->frame * 32 + i * 256) / 64) % 320 - 160;
        scratch->y0 = resource->y[i] - 120;
        resource->y[i] += 2 + (i & 3);
        if (resource->y[i] >= 240) resource->y[i] = -4;
        scratch->u0 = (((u8)resource->frame + i) & 3) * 8 + 216U;
        scratch->clut = getClut(992, 240 + i % 5);
        scratch->tag = (3 << 24) | getaddr(D_8013AF6C);
        *packet = *scratch;
        setaddr(D_8013AF6C, packet);
        packet++;
    }
    page = (DR_TPAGE *)packet;
    setlen(page, 1);
    page->code[0] = ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(0, 0, 768, 256);
    addPrim(D_8013AF6C, page);
    GsOUT_PACKET_P = (u32)(page + 1);
    resource->frame++;
}

void createFallingParticleTask(void) {
    FallingParticleResource *resource;
    s32 i;
    func_80014930(&gGameTasks[4], drawFallingParticlesTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (FallingParticleResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 40; i++) {
        resource->x[i] = randomNextLinear() % 320;
        resource->y[i] = -(randomNextLinear() % 244);
    }
    resource->frame = 0;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80062EA0);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800632C4);

void createPlayerParticleTask(s32 playerIndex) {
    PlayerParticlePosition *position;
    s32 i;
    func_80014930(&gGameTasks[4], func_800632C4);
    D_8013AE8C->resource14 = allocateTaskResource();
    position = (PlayerParticlePosition *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 40; i++) {
        position[i].x = randomNextLinear() % 1024;
        position[i].y = randomNextLinear() % 512;
        position[i].z = randomNextLinear() % 1024;
    }
    D_8013AE8C->data18 = playerIndex;
    D_8013AE8C->data1A = 0;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80063890);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80063B58);

void createPositionedEffectTask(s32 x, s32 y, s32 z) {
    PositionedEffectResource *resource;

    func_80014874(func_80063B58);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PositionedEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = x >> 8;
    resource->y = y >> 8;
    resource->z = z >> 8;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80063DF4);

void startRotatingQuadEffectTask(s32 variant, const VECTOR *position, s32 pitch, s32 yaw, s32 roll) {
    RotatingEffectResource *resource;
    func_80014874(func_80063DF4);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RotatingEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = position->vx >> 8;
    resource->y = position->vy >> 8;
    resource->z = position->vz >> 8;
    resource->rotation.vx = 0;
    resource->rotation.vy = yaw;
    resource->rotation.vz = 0;
    resource->parentRotation.vx = pitch;
    resource->parentRotation.vy = 0;
    resource->parentRotation.vz = roll;
    resource->elapsed = 0;
    resource->clut = variant == 0 ? 0x3EA8 : 0x3EE8;
    resource->textureU = variant == 0 ? 0x40 : 0x80;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80064454);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80064754);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006479C);

void initializeDelayedEightEntryEffectTask(void) {
    EightEntryEffectResource *primary;
    EightEntryEffectSecondaryResource *secondary;
    s32 i;
    if (D_8013AE88->data18 == 0) {
        func_80014930(D_8013B304, func_8006479C);
        D_8013AE8C->resource14 = allocateTaskResource();
        primary = (EightEntryEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        D_8013AE8C->resource16 = allocateTaskResource();
        secondary = (EightEntryEffectSecondaryResource *)&D_800D9504[D_8013AE8C->resource16];
        D_8013AE8C->data18 = D_8013AE88->data1A;
        D_8013AE8C->data1A = 0;
        for (i = 0; i < 8; i++) {
            primary->positions[i].vx = 0;
            primary->positions[i].vy = 0;
            primary->positions[i].vz = 0;
            secondary->vectors[i].vx = 0;
            secondary->vectors[i].vy = 0;
            secondary->vectors[i].vz = 0;
            primary->rotations[i].vx = 0;
            primary->rotations[i].vy = 0;
            primary->rotations[i].vz = 0;
            secondary->values40[i] = 0;
            secondary->values50[i] = 0;
            secondary->values60[i] = 0;
            primary->values[i] = 32;
        }
        primary->elapsed = 0;
        func_800149A8();
    } else {
        D_8013AE88->data18--;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80065464);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800654B4);

void createPlayerPairedQuadEffectTask(s32 playerIndex, s32 variant) {
    PairedEffectResource *resource;
    RacePlayer *player;
    s32 i;
    func_80014930(D_8013B304, func_800654B4);
    D_8013AE8C->resource14 = allocateTaskResource();
    player = &gRacePlayers[playerIndex];
    resource = (PairedEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 2; i++) {
        gPlayerPairedQuadOffsets[i].vz = randomNextLinear() % 40 - 20;
        ApplyMatrixLV(&player->model.effect.effectMatrix180, &gPlayerPairedQuadOffsets[i], &resource->positions[i]);
        resource->positions[i].vx += player->model.effect.effectMatrix180.t[0];
        resource->positions[i].vy += player->model.effect.effectMatrix180.t[1];
        resource->positions[i].vz += player->model.effect.effectMatrix180.t[2];
        resource->positions[i].vx -= player->worldCoordinate.coord.t[0] + player->unk084;
        resource->positions[i].vy -= player->worldCoordinate.coord.t[1] + player->unk086;
        resource->positions[i].vz -= player->worldCoordinate.coord.t[2] + player->unk088;
    }
    resource->elapsed = 0;
    resource->clut = variant == 0 ? getClut(800, 250) : getClut(800, 251);
    D_8013AE8C->data18 = playerIndex;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800659F0);

GameTask *createQuadAnimationTask(u16 unusedId, VECTOR *position) {
    PrimaryQuadAnimationResource *resource;
    func_80014930(D_8013B304, func_800659F0);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PrimaryQuadAnimationResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = position->vx;
    resource->y = position->vy;
    resource->z = position->vz;
    resource->elapsed = 0;
    resource->frame = 0;
    resource->frames = gQuadAnimationFrames;
    return D_8013AE8C;
}

GameTask *startAlternateQuadAnimationTask(s32 unused, const PositionEffectOrigin *position) {
    QuadAnimationResource *resource;
    GameTask *task;
    func_80014930(D_8013B304, func_800659F0);
    D_8013AE8C->resource14 = allocateTaskResource();
    task = D_8013AE8C;
    resource = (QuadAnimationResource *)&D_800D9504[task->resource14];
    resource->position.x = position->x;
    resource->position.y = position->y;
    resource->position.z = position->z;
    resource->elapsed = 0;
    resource->frame = 0;
    resource->steps = gAlternateQuadAnimationSteps;
    return task;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80065EC8);

void emitRadialEffectTask(void) {
    PositionEffectOrigin *origin;
    RadialEffectResource *resource;
    s32 i;
    s32 angle;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    origin = (PositionEffectOrigin *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AFD3 == 0 && (gFrameCounter & 3) == 0) {
        func_80014930(D_8013B304, func_80065EC8);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (RadialEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->coordinate.coord.t[0] = origin->x;
        resource->coordinate.coord.t[1] = origin->y;
        resource->coordinate.coord.t[2] = origin->z;
        for (i = 0, angle = 0; i < 6; i++) {
            resource->positions[i].vx = resource->innerPositions[i].vx = 0;
            resource->positions[i].vz = resource->innerPositions[i].vz = 0;
            resource->positions[i].vy = resource->innerPositions[i].vy = 0;
            resource->velocities[i].vx = rcos(angle) / 512;
            resource->velocities[i].vz = rsin(angle) / 512;
            angle += 0x2AA;
            resource->velocities[i].vy = -32;
        }
        resource->elapsed = 0;
        if (--D_8013AE88->data18 == 0) {
            func_800149A8();
        }
    }
}

void startPositionEffectTask(const PositionEffectOrigin *position) {
    PositionEffectOrigin *resource;
    func_80014930(D_8013B304, emitRadialEffectTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = 3;
    resource = (PositionEffectOrigin *)&D_800D9504[D_8013AE8C->resource14];
    *resource = *position;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80066474);

void createPlayerTrailTask(s32 playerIndex) {
    PlayerTrailResource *first;
    PlayerTrailResource *second;
    s32 i;
    func_80014930(D_8013B304, func_80066474);
    D_8013AE8C->resource14 = allocateTaskResource();
    first = (PlayerTrailResource *)&D_800D9504[D_8013AE8C->resource14];
    D_8013AE8C->resource16 = allocateTaskResource();
    second = (PlayerTrailResource *)&D_800D9504[D_8013AE8C->resource16];
    for (i = 0; i < 16; i++) {
        first->positions[i].x = first->positions[i].y = first->positions[i].z = 0;
        second->positions[i].x = second->positions[i].y = second->positions[i].z = 0;
        second->state[i] = 0;
    }
    first->state[0] = 0;
    D_8013AE8C->data18 = playerIndex;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80066D18);

void createPlayerPositionTrailTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerPositionTrailResource *resource;
    s32 i, j;

    player = &gRacePlayers[playerIndex];
    func_80014930(D_8013B304, func_80066D18);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = playerIndex;
    resource = (PlayerPositionTrailResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            resource->positions[i][j] = player->worldCoordinate.coord.t[j];
        }
    }
    resource->unk4E = 1;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800676FC);

void emitPlayerTurnTripleQuadEffectTask(void) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i, sine;
    player = &gRacePlayers[D_8013AE88->data18];
    if (D_8013AFD3 != 0) return;
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)func_80059254())) return;
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) return;
    sine = rsin((s16)player->unk0BE - (s16)player->unk110);
    if (sine < 0) sine = -sine;
    if ((gFrameCounter & 3) == 0 && !(player->unk128 & 0x677DA21) && player->unk0BC >= 0x800 && sine >= 0x200) {
        func_80014930(D_8013B304, func_800676FC);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (PlayerTripleQuadEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        vertex = gPlayerTripleQuadOffsets;
        for (i = 0; i < 3; i++) {
            ApplyMatrix(&player->model.effect.effectMatrix180, vertex, (VECTOR *)&resource->positions[i]);
            resource->positions[i].x += player->worldCoordinate.coord.t[0];
            resource->positions[i].y += player->worldCoordinate.coord.t[1];
            resource->positions[i].z += player->worldCoordinate.coord.t[2];
            vertex++;
        }
        resource->elapsed = 0;
        resource->rotation.vx = 0;
        resource->rotation.vy = 0;
        resource->rotation.vz = randomNextLinear() % 4096;
        resource->state = player->unk96E;
        resource->scale = rsin((s16)player->unk0BE - (s16)player->unk110);
        if (resource->scale < 0) resource->scale = -resource->scale;
        D_8013AE8C->data18 = D_8013AE88->data18;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80067D88);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80067DD0);

void createPlayerRingEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerRingEffectResource *resource;
    s32 i;
    player = &gRacePlayers[playerIndex];
    if (D_8013AFCE != 3) {
        func_80014930(D_8013B304, func_80067DD0);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (PlayerRingEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        for (i = 0; i < 3; i++) {
            resource->position[i] = player->worldCoordinate.coord.t[i];
        }
        resource->position[1] = func_8004A764(player->unk92E, resource->position[0], resource->position[2]);
        for (i = 0; i < 6; i++) {
            resource->rotations[i].vy = 0;
            resource->rotations[i].vx = 0;
            resource->rotations[i].vz = randomNextLinear() % 4096;
        }
        resource->elapsed = 0;
        D_8013AE8C->data18 = playerIndex;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006836C);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800688C0);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800689EC);

s32 createPlayerTripleQuadEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i;
    s32 state;
    player = &gRacePlayers[playerIndex];
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)func_80059254())) {
        return 0;
    }
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) {
        return 0;
    }
    func_80014930(D_8013B304, func_800689EC);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PlayerTripleQuadEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    vertex = gPlayerTripleQuadOffsets;
    for (i = 0; i < 3; i++) {
        ApplyMatrix(&player->model.effect.effectMatrix180, vertex, (VECTOR *)&resource->positions[i]);
        resource->positions[i].x += player->worldCoordinate.coord.t[0];
        resource->positions[i].y += player->worldCoordinate.coord.t[1];
        resource->positions[i].z += player->worldCoordinate.coord.t[2];
        vertex++;
    }
    resource->elapsed = 0;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = randomNextLinear() % 4096;
    state = player->unk96E;
    resource->state = state;
    resource->scale = 0;
    D_8013AE8C->data18 = playerIndex;
    return state;
}

void drawRisingTexturedStripTask(void) {
    POLY_FT4 *packet;
    RotatingQuadTrailResource *resource;
    s32 y;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RotatingQuadTrailResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    packet->tag = 9 << 24;
    setcode(packet, POLY_FT4_CODE);
    packet->clut = getClut(0x3B0, 0xF0);
    packet->tpage = getTPage(0, 0, 0x3C0, 0);
    packet->u0 = 0;
    packet->v0 = 0;
    packet->v1 = 0;
    packet->u2 = 0;
    packet->v2 = 15;
    packet->u1 = 215;
    packet->u3 = 215;
    packet->v3 = 15;
    packet->x0 = -108;
    packet->x1 = 108;
    packet->x2 = -108;
    packet->x3 = 108;
    y = -8 - resource->countdown * 8;
    setRGB0(packet, 128, 128, 128);
    packet->y1 = y;
    packet->y0 = y;
    packet->y3 = y + 15;
    packet->y2 = y + 15;
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (D_8013B338 >= 50) {
        if (++resource->countdown > 10) {
            resource->countdown = 10;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80068D1C);

void createRotatingQuadTrailTask(void) {
    RotatingQuadTrailResource *resource;
    s32 i;

    func_80014930(&gGameTasks[4], func_80068D1C);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RotatingQuadTrailResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 30;
    for (i = 0; i < 8; i++) {
        resource->angles[i] = resource->countdown * 256;
        resource->scales[i] = resource->countdown * 400 + 0x1000;
    }
}

void drawLoweringTexturedQuadTask(void) {
    POLY_FT4 *packet;
    ShrinkingQuadResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ShrinkingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->x0 = resource->vertices[0].x;
    packet->y0 = resource->vertices[0].y + (resource->countdown << 6) / 10;
    packet->x1 = resource->vertices[1].x;
    packet->y1 = resource->vertices[1].y + (resource->countdown << 6) / 10;
    packet->x2 = resource->vertices[2].x;
    packet->y2 = resource->vertices[2].y + (resource->countdown << 6) / 10;
    packet->x3 = resource->vertices[3].x;
    packet->y3 = resource->vertices[3].y + (resource->countdown << 6) / 10;
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (D_8013B338 >= 50) {
        if (++resource->countdown > 10) { resource->countdown = 10; }
    }
}

void drawShrinkingTexturedQuadTask(void) {
    s32 i;
    POLY_FT4 *packet;
    QuadTransitionResource *resource;
    QuadScaleScratchpad *scratch;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    scratch = (QuadScaleScratchpad *)0x1F800000;
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch->rotation.vx = scratch->rotation.vy = scratch->rotation.vz = 0;
    scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = resource->frames * 1000 + 0x1000;
    RotMatrix((SVECTOR *)0x1F800020, &scratch->matrix);
    ScaleMatrix(&scratch->matrix, (VECTOR *)0x1F800028);
    for (i = 0; i < 4; i++) {
        ApplyMatrixSV(&scratch->matrix, &resource->vertices[i], &scratch->vertices[i]);
    }
    *packet = *resource->packet;
    packet->x0 = scratch->vertices[0].vx;
    packet->y0 = scratch->vertices[0].vy;
    packet->x1 = scratch->vertices[1].vx;
    packet->y1 = scratch->vertices[1].vy;
    packet->x2 = scratch->vertices[2].vx;
    packet->y2 = scratch->vertices[2].vy;
    packet->x3 = scratch->vertices[3].vx;
    packet->y3 = scratch->vertices[3].vy;
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (--resource->frames <= 0) {
        setCurrentTaskCallback(drawLoweringTexturedQuadTask);
        resource->frames = 0;
    }
}

void createQuadShrinkTask(void) {
    QuadTransitionResource *resource;

    func_80014930(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->frames = 5;
    resource->vertices = gQuadShrinkVertices;
    resource->packet = &gQuadShrinkPacket;
}

void startMiddleQuadShrinkTask(void) {
    ShrinkingQuadResource *resource;

    func_80014930(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ShrinkingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 5;
    resource->vertices = gMiddleShrinkQuadVertices;
    resource->packet = &gMiddleShrinkQuadPacket;
}

void createRightQuadShrinkTask(void) {
    QuadTransitionResource *resource;

    func_80014930(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->frames = 5;
    resource->vertices = gRightQuadShrinkVertices;
    resource->packet = &gRightQuadShrinkPacket;
}

void drawPaletteCyclingStripTask(void) {
    POLY_FT4 *packet;
    PaletteStripResource *resource;
    s32 paletteRow;
    s32 paletteOffset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PaletteStripResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    packet->x0 = packet->x2 = -16;
    packet->x1 = packet->x3 = 143;
    packet->y0 = packet->y1 = 88;
    packet->y2 = packet->y3 = 103;
    paletteOffset = D_8013AE88->data18 * 3;
    paletteRow = resource->frame % 3 + 242;
    packet->clut = getClut(944, paletteOffset + paletteRow);
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    resource->frame++;
}

void drawLoweringQuadStripTask(void) {
    POLY_FT4 *packet;
    SlidingQuadResource *resource;
    s32 offset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    packet->x0 = packet->x2 = -16;
    packet->x1 = packet->x3 = 143;
    offset = (resource->countdown << 6) / 10;
    packet->y0 = packet->y1 = offset + 24;
    packet->y2 = packet->y3 = offset + 39;
    packet->clut = getClut(944, D_8013AE88->data18 * 3 + 242);
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (D_8013B338 >= 50) {
        if (++resource->countdown > 10) {
            setCurrentTaskCallback(drawPaletteCyclingStripTask);
            resource->countdown = 0;
        }
    }
}

void drawSlidingQuadTask(void) {
    POLY_FT4 *packet;
    SlidingQuadResource *resource;
    s32 offset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    offset = resource->countdown * resource->countdown;
    packet->x0 = packet->x2 = offset - 16;
    packet->x1 = packet->x3 = offset + 143;
    packet->clut = getClut(944, D_8013AE88->data18 * 3 + 242);
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (--resource->countdown <= 0) {
        setCurrentTaskCallback(drawLoweringQuadStripTask);
        resource->countdown = 0;
    }
}

void startSlidingQuadTask(s32 index) {
    SlidingQuadResource *resource;
    func_80014930(&gGameTasks[4], drawSlidingQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 10;
    resource->packet = &gSlidingQuadPacket;
    resource->textureV = (index << 4) + 32;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80069BAC);

void updatePaletteWhitePulse(void) {
    s32 amount, i;
    u8 r, g, b;

    if (D_8013AE88->data18 < 10) {
        amount = D_8013AE88->data18;
    } else if (D_8013AE88->data18 < 20) {
        amount = 20 - D_8013AE88->data18;
    } else {
        amount = 0;
    }
    for (i = 0; i < 16; i++) {
        r = (gCapturedColorCyclePalette[i].bytes[0] & 31);
        g = ((gCapturedColorCyclePalette[i].value >> 5) & 31);
        b = ((gCapturedColorCyclePalette[i].value >> 10) & 31);
        if (i != 0) {
            r += (31 - r) * amount / 10;
            g += (31 - g) * amount / 10;
            b += (31 - b) * amount / 10;
        }
        gWhitePulsePalette[i] = (u8)r | ((u8)g << 5) | ((u8)b << 10);
    }
    LoadImage(&gWhitePulsePaletteRect, (u32 *)gWhitePulsePalette);
    DrawSync(0);
    D_8013AE88->data18 = (D_8013AE88->data18 + 1) % 60;
}

void capturePaletteForColorCycle(void) {
    StoreImage(&gColorCycleCaptureRect, (u32 *)gCapturedColorCyclePalette);
    DrawSync(0);
    setCurrentTaskCallback(updatePaletteWhitePulse);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80069E08);

void updateQuadTransitionSequenceTask(void) {
    s32 frame = D_8013B338;
    if (frame == 0) createRotatingQuadTrailTask();
    if (frame == 25) createQuadShrinkTask();
    if (frame == 30) startMiddleQuadShrinkTask();
    if (frame == 35) createRightQuadShrinkTask();
    if (frame == 40) startSlidingQuadTask(D_8013AE88->data1A);
    if (frame == 50) func_80069E08(10);
    D_8013B338++;
    if (D_8013B338 == 100) func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80069F1C);

void updatePrimaryValueTransition(void) {
    s32 value;
    value = D_8013AE88->data1C +
        (D_8013AE88->data1E - D_8013AE88->data1C) *
        D_8013AE88->data18 / D_8013AE88->data1A;
    D_8013B35C = value;
    D_8013AE88->data18++;
    D_8013B364 = 0;
    if (D_8013AE88->data18 >= D_8013AE88->data1A) {
        func_800149A8();
        D_8013B364 = 1;
    }
}

void startPrimaryValueTransition(s32 duration, s32 start, s32 end) {
    func_80014930(D_8013AE88, updatePrimaryValueTransition);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
    D_8013AE8C->data1C = start;
    D_8013AE8C->data1E = end;
}

void updateValueInterpolationTask(void) {
    s32 step;
    s32 start = D_8013AE88->data1C;
    step = (D_8013AE88->data1E - start) * D_8013AE88->data18;
    step /= D_8013AE88->data1A;
    D_8013B360 = start + step;
    D_8013AE88->data18++;
    D_8013B364 = 0;
    if (D_8013AE88->data18 >= D_8013AE88->data1A) {
        func_800149A8();
        D_8013B364 = 1;
    }
}

void startValueInterpolationTask(s32 duration, s32 start, s32 end) {
    func_80014930(D_8013AE88, updateValueInterpolationTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
    D_8013AE8C->data1C = start;
    D_8013AE8C->data1E = end;
}

void drawFullScreenSpriteTask(void) {
    ScreenSpriteResource *resource;
    DrawModeSpritePacket *packets;
    s32 i;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    packets = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    resource = (ScreenSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    for (i = 0; i < 2; i++) {
        resource->sprites[i].r0 = resource->sprites[i].g0 = resource->sprites[i].b0 = *(const u8 *)resource->brightness;
    }
    for (i = 0; i < 2; i++) {
        packets[i] = resource->sprites[i];
        addPrim(&D_8013AF6C[1], &packets[i]);
    }
    GsOUT_PACKET_P = (u32)(packets + 2);
    if (++resource->elapsed >= resource->duration || (D_8013AFE4 & 0x100)) {
        func_800149A8();
    }
}

void createFullScreenSpriteTask(s32 duration, s32 tpage, u16 clut, u16 u, u16 v,
                  const s32 *brightness) {
    ScreenSpriteResource *resource;
    s32 i;

    func_80014930(&gGameTasks[4], drawFullScreenSpriteTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ScreenSpriteResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 2; i++) {
        resource->sprites[i].tag = (sizeof(DrawModeSpritePacket) / sizeof(u32) - 1) << 24;
        resource->sprites[i].code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
        resource->sprites[i].drawModeCode = DRAW_MODE_COMMAND_HIGH;
        resource->sprites[i].tpage = tpage | getTPage(0, 1, 0, 0);
        tpage += 2;
        resource->sprites[i].clut = clut;
        resource->sprites[i].r0 = resource->sprites[i].g0 = resource->sprites[i].b0 = 0x60;
        resource->sprites[i].u0 = u;
        resource->sprites[i].v0 = v;
        resource->sprites[i].h = 240;
        resource->sprites[i].y0 = -120;
    }
    resource->sprites[0].w = 256;
    resource->sprites[1].w = 64;
    resource->sprites[0].x0 = -160;
    resource->sprites[1].x0 = 96;
    resource->elapsed = 0;
    resource->duration = duration;
    resource->brightness = brightness;
}

void *getOffsetArchiveEntry(OffsetArchive *archive, s32 index) {
    u32 *offsets = archive->offsets;
    return (u8 *)archive + offsets[index];
}

void uploadTimImageAndPaletteAt(u32 *tim, s32 x, s32 y, s32 clutX, s32 clutY) {
    GsIMAGE image;
    RECT rect;

    GsGetTimInfo(tim + 1, &image);
    rect.x = x;
    rect.y = y;
    rect.w = image.pw;
    rect.h = image.ph;
    LoadImage(&rect, image.pixel);
    rect.x = clutX;
    rect.y = clutY;
    rect.w = image.cw;
    rect.h = image.ch;
    LoadImage(&rect, image.clut);
    DrawSync(0);
}

void drawFadingText(FadingTextResource *resource) {
    FadingTextGlyphBanks tables = gFadingTextGlyphBanks;
    DrawModeSpritePacket *scratch = (DrawModeSpritePacket *)0x1F800000, *packet;
    const s8 *text, *glyph;
    s32 table, index;
    scratch->tag = (sizeof(*scratch) / 4 - 1) << 24;
    scratch->code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
    scratch->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    scratch->clut = resource->clut;
    scratch->x0 = resource->x;
    scratch->y0 = resource->y;
    scratch->r0 = scratch->g0 = scratch->b0 = *(u8 *)&resource->brightness;
    scratch->w = 16;
    scratch->h = 16;
    text = (const s8 *)resource->text;
    packet = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    while (*text) {
        for (table = 0; table < 4; table++) {
            glyph = tables.entries[table];
            for (index = 0; glyph[index]; index += 2) {
                if (glyph[index] == text[0] && glyph[index + 1] == text[1]) {
                    scratch->u0 = ((index >> 1) & 15) << 4;
                    scratch->v0 = (index >> 1) & ~15;
                    switch (table) {
                        case 0:
                            scratch->tpage = getTPage(0, 0, 960, 256);
                            scratch->v0 += 160;
                            break;
                        case 1:
                            scratch->tpage = getTPage(0, 0, 896, 256);
                            break;
                        case 2:
                            scratch->tpage = getTPage(0, 0, 768, 256);
                            break;
                        case 3:
                            scratch->tpage = getTPage(0, 0, 704, 256);
                            break;
                    }
                    *packet = *scratch;
                    packet->tpage |= getTPage(0, 1, 0, 0);
                    addPrim(&D_8013AF6C[0], packet);
                    packet++;
                    *packet = *scratch;
                    packet->clut = getClut(768, 248);
                    packet->tpage |= getTPage(0, 2, 0, 0);
                    addPrim(&D_8013AF6C[0], packet);
                    packet++;
                    goto nextCharacter;
                }
            }
        }
    nextCharacter:
        scratch->x0 += 16;
        text += 2;
    }
    GsOUT_PACKET_P = (u32)packet;
}

void updateFadingTextTask(void) {
    FadingTextResource *resource;
    s32 remaining;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FadingTextResource *)&D_800D9504[D_8013AE88->resource14];
    setSpriteColor(255, 255, 255);
    remaining = resource->elapsed;
    if (remaining < 30 || (remaining = resource->duration - remaining) < 30) {
        resource->brightness = remaining * 255 / 30;
    } else {
        resource->brightness = 255;
    }
    drawFadingText(resource);
    D_8013B368 = 0;
    if (++resource->elapsed == resource->duration) {
        func_800149A8();
        D_8013B368 = 1;
    }
}

void createFadingTextTask(s32 duration, s32 x, s32 y, const char *text, s32 palette) {
    FadingTextResource *resource;
    func_80014930(D_8013AE88, updateFadingTextTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (FadingTextResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->duration = duration;
    resource->elapsed = 0;
    resource->x = x;
    resource->y = y;
    resource->text = text;
    resource->clut = getClut(768, palette + 240);
}

void finishTimedSpriteSequence(void) {
    TimedSpriteSequenceResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->duration < 0) {
        func_800149A8();
    }
}

void updateTimedSpriteSequence(void) {
    TimedSpriteSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->frame >= resource->step->frame) {
        createFadingTextTask(resource->step->duration, resource->step->x,
            resource->step->y, resource->step->text, resource->step->palette);
        resource->duration = resource->step->duration;
        resource->step++;
    }
    resource->frame++;
    if (resource->step->frame == -1) {
        setCurrentTaskCallback(finishTimedSpriteSequence);
    }
}

void startTimedSpriteSequence(const TimedSpriteStep *steps) {
    TimedSpriteSequenceResource *resource;
    func_80014930(D_8013AE88, updateTimedSpriteSequence);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->step = steps;
    resource->frame = 0;
}

void finishTimedSpritePresentation(void) {
    if (D_8013AE88->data18++ >= 30) {
        startCdVolumeFade(103, 20, 0, 1);
        D_8013AFE4 |= 0x100;
        func_800149E0();
        func_800149A8();
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006AB8C);

void initializeTimedSpritePresentation(void) {
    setSecondaryDisplayTaskMode(0);
    queueCdCommand(6, 1);
    D_8013B35C = 0;
    D_8013B360 = 0;
    D_8013B364 = 0;
    D_8013B368 = 0;
    D_8013B36C = 0;
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(func_8006AB8C);
}

void loadTimedSpritePresentationResources(void) {
    setSecondaryDisplayTaskMode(1);
    queueCdSectorCommand(1, DAT_ENTRY_177_SECTOR, DAT_ENTRY_177_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(1, DAT_ENTRY_178_SECTOR, DAT_ENTRY_178_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(1, DAT_ENTRY_232_SECTOR, DAT_ENTRY_232_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(1, DAT_ENTRY_233_SECTOR, DAT_ENTRY_233_SECTOR_COUNT, DAT_SCRATCH, 0);
    DrawSync(0);
    queueCdSectorCommand(0, DAT_PLAYER_ANIMATION_SECTOR, DAT_PLAYER_ANIMATION_SECTOR_COUNT, DAT_SCRATCH, 0);
    setCurrentTaskCallback(initializeTimedSpritePresentation);
    D_8013AE88->unk10 |= 2;
}

void createResourceLoadTask(void) {
    func_80014930(&gGameTasks[4], loadTimedSpritePresentationResources);
}

void addTextCharacter(s32 x, s32 y, u32 character) {
    SPRT_8 *sprite = (SPRT_8 *)GsOUT_PACKET_P;

    setcode(sprite, SPRT_8_CODE);
    setRGB0(sprite, 255, 255, 255);
    sprite->u0 = (character & 31) << 3;
    sprite->v0 = (character >> 3) & 24;
    setXY0(sprite, x, y);
    sprite->tag = (sizeof(SPRT_8) / sizeof(u32) - 1) << 24;
    sprite->clut = getClut(0x300, D_8013B376 + 0xF0);
    addPrim(D_8013AF6C, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

void addStretchedTextCharacter(s32 x, s32 y, s32 width, u32 character) {
    POLY_FT4 *quad;
    width = x + width - 1;
    quad = (POLY_FT4 *)GsOUT_PACKET_P;
    quad->tag = (sizeof(POLY_FT4) / sizeof(u32) - 1) << 24;
    setcode(quad, POLY_FT4_CODE);
    quad->clut = getClut(768, D_8013B376 + 240);
    quad->tpage = getTPage(0, 0, 960, 256);
    setRGB0(quad, 255, 255, 255);
    quad->y0 = y;
    quad->y1 = y;
    quad->y2 = y + 7;
    quad->y3 = y + 7;
    quad->x1 = width;
    quad->x3 = width;
    quad->u0 = (character & 31) << 3;
    quad->v0 = (character >> 3) & 24;
    quad->x0 = x;
    quad->x2 = x;
    quad->u1 = quad->u0 + 7;
    quad->v1 = quad->v0;
    quad->u2 = quad->u0;
    quad->v2 = quad->v0 + 7;
    quad->u3 = quad->u0 + 7;
    quad->v3 = quad->v0 + 7;
    addPrim(D_8013AF6C, quad);
    GsOUT_PACKET_P = (u32)(quad + 1);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006B4A0);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006B9EC);
