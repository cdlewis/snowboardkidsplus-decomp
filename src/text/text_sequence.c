#include "../menu/panel_tasks_internal.h"
#include "game/menu/task_widgets.h"
#include "game/audio/music.h"
#include "game/engine/game_loop.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/dat_entries.h"
#include "game/menu/selected_records.h"
#include "game/race/camera/race_camera.h"
#include "game/save/save_data.h"

/* The parser builds this table immediately before the indexed sector buffer. */
#define TEXT_SEQUENCE_ENTRIES ((TextSequenceEntry *)0x801F2000)
#define TEXT_SEQUENCE_MAX_ENTRIES 16
#define TEXT_SEQUENCE_ADVANCE_BUTTONS 0xB0

typedef struct TextSequenceEntry {
    const u16 *text;
    u16 kind;
    u16 length;
} TextSequenceEntry;

extern u8 gModeSequenceStage __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u16 gDisplayWidth __attribute__((section(".data")));
extern u16 gDisplayHeight __attribute__((section(".data")));
extern s16 gPlayerViewVerticalOffset __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));

extern u16 gGameStateFlags __attribute__((section(".data")));

ResourceSectorRange gMode1SectorRanges[10] __attribute__((section(".data.indexed_sector_ranges"))) = {
    {DAT_MODE1_TEXT_000_SECTOR, DAT_MODE1_TEXT_000_SECTOR_COUNT},
    {DAT_MODE1_TEXT_001_SECTOR, DAT_MODE1_TEXT_001_SECTOR_COUNT},
    {DAT_MODE1_TEXT_002_SECTOR, DAT_MODE1_TEXT_002_SECTOR_COUNT},
    {DAT_MODE1_TEXT_003_SECTOR, DAT_MODE1_TEXT_003_SECTOR_COUNT},
    {DAT_MODE1_TEXT_004_SECTOR, DAT_MODE1_TEXT_004_SECTOR_COUNT},
    {DAT_MODE1_TEXT_005_SECTOR, DAT_MODE1_TEXT_005_SECTOR_COUNT},
    {DAT_MODE1_TEXT_006_SECTOR, DAT_MODE1_TEXT_006_SECTOR_COUNT},
    {DAT_MODE1_TEXT_007_SECTOR, DAT_MODE1_TEXT_007_SECTOR_COUNT},
    {DAT_MODE1_TEXT_008_SECTOR, DAT_MODE1_TEXT_008_SECTOR_COUNT},
    {DAT_MODE1_TEXT_009_SECTOR, DAT_MODE1_TEXT_009_SECTOR_COUNT}
};

ResourceSectorRange gOtherModeSectorRanges[11] __attribute__((section(".data.indexed_sector_ranges"))) = {
    {DAT_OTHER_MODE_TEXT_000_SECTOR, DAT_OTHER_MODE_TEXT_000_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_001_SECTOR, DAT_OTHER_MODE_TEXT_001_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_002_SECTOR, DAT_OTHER_MODE_TEXT_002_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_003_SECTOR, DAT_OTHER_MODE_TEXT_003_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_004_SECTOR, DAT_OTHER_MODE_TEXT_004_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_005_SECTOR, DAT_OTHER_MODE_TEXT_005_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_006_SECTOR, DAT_OTHER_MODE_TEXT_006_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_007_SECTOR, DAT_OTHER_MODE_TEXT_007_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_008_SECTOR, DAT_OTHER_MODE_TEXT_008_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_009_SECTOR, DAT_OTHER_MODE_TEXT_009_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_010_SECTOR, DAT_OTHER_MODE_TEXT_010_SECTOR_COUNT}
};

TimedSequenceEntry gTextSequenceAdvancePromptFrames[2]
    __attribute__((section(".data.text_sequence_prompt_frames"))) = {
    {0x00030231, 10},
    {0x00030232, 10},
};

const u16 gTextSequenceNumberLabel[] __attribute__((section(".data.text_sequence_number_label"), aligned(4))) = {
    0x0036, 0x0056, 0x006C, 0x0158, 0x0058, 0x0070, 0x0073, 0x8020, 0x0036, 0x0000,
};

static void drawTextSequence(TextSequenceResource *resource);

void initializeTextSequenceTask(void) {
    TextSequenceResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextSequenceResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    resource->state = 2;
    resource->entryIndex = 0;
    resource->progress = 0;
    resource->duration = 0;
    resource->intensity = 0x80;
    resource->unk3A = 0;
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(1, 0, 30);
    if (resource->mode != 1 || resource->unk0A != 9) {
        queueCdCommand(3, 0);
    }
    setCurrentTaskCallback(updateTextSequenceTask);
}

void updateTextSequenceTask(void) {
    TextSequenceResource *resource;
    ValuePanelEntry *panel;
    TextSequenceEntry *entry;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextSequenceResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    panel = (ValuePanelEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (resource->state) {
    case 2:
        entry = TEXT_SEQUENCE_ENTRIES;
        entry += resource->entryIndex;
        if (entry->kind == 0) {
            resource->headerText = entry->text;
            resource->x = (-(entry->length << 4) / 2) << 12;
            resource->y = -48 * 4096;
            resource->entryIndex++;
            entry++;
        }
        if (entry->kind == 1) {
            resource->bodyText = entry->text;
            if (resource->mode == 1 && resource->unk0A == 9) {
                resource->bodyX = -136 * 4096;
                resource->displacement = -37 * 4096;
            } else {
                resource->bodyX = -136 * 4096;
                resource->displacement = 20 * 4096;
            }
            resource->progress = 0;
            resource->duration = entry->length;
        }
        if (entry->kind == 2) {
            resource->velocity = 0;
            if (resource->mode == 1 && resource->unk0A == 9) {
                gCurrentTask->data18 = 1;
                setCurrentTaskCallback(advanceModeAfterTextSequenceFadeTask);
            } else {
                startCdVolumeFade(103, 20, 0, 1);
                setCurrentTaskCallback(animateTextSequenceExitTask);
            }
        } else {
            resource->state = 3;
        }
        break;
    case 3:
        if (!hasTaskStateValue6(0)) {
            resource->state = 0;
        }
        break;
    case 1:
        if (panel->active <= 0) {
            resource->state = 2;
            resource->entryIndex++;
        }
        break;
    case 0:
    default:
        if ((resource->progress >> 8) >= resource->duration) {
            resource->state = 1;
            createInputTimedSpriteTask(resource->bodyX / 4096 + 248, resource->displacement / 4096 + 52,
                                       gTextSequenceAdvancePromptFrames, 2, 1, TEXT_SEQUENCE_ADVANCE_BUTTONS, 0, 0, 1);
        } else {
            if (gControllerPressedButtons[0] & TEXT_SEQUENCE_ADVANCE_BUTTONS) {
                resource->progress = resource->duration << 8;
            } else {
                resource->progress += 127;
            }
            if ((resource->progress >> 8) > resource->duration) {
                resource->progress = resource->duration << 8;
            }
        }
        break;
    }
    drawTextSequence(resource);
}

void animateTextSequenceExitTask(void) {
    TextSequenceResource *resource;
    u16 previous;
    s16 delta;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    resource->velocity += 0x1000;
    resource->y -= resource->velocity;
    resource->displacement += resource->velocity;
    if (resource->intensity != 0) resource->intensity -= 4;
    previous = gViewClipRects[1].h;
    gViewClipRects[1].h = resource->displacement / 4096 - (u32)(-gDisplayHeight / 2 + 4);
    if (gViewClipRects[1].h > gDisplayHeight) gViewClipRects[1].h = gDisplayHeight;
    delta = gViewClipRects[1].h - previous;
    resource->unk3A += delta;
    gPlayerViewVerticalOffset -= delta / 2;
    if (gPlayerViewVerticalOffset < 0) gPlayerViewVerticalOffset = 0;
    gViewClipRects[2].y += delta;
    gViewClipRects[2].h -= delta;
    if (gViewClipRects[2].h < 0) {
        gViewClipRects[2].y = 240;
        gViewClipRects[2].h = 0;
    }
    gPlayerCameras[0].unk10 += delta * 2;
    if (gPlayerCameras[0].unk10 > -320) gPlayerCameras[0].unk10 = -320;
    if (resource->displacement / 4096 >= 141) {
        taskTransitionFlagGp = 1;
        D_8013B1A8 = 1;
        playerCountAbsolute = 1;
        gTaskTransitionFlags = 0;
        gGameStateFlags |= 0x1000;
        queueCdCommand(D_80124164[gSelectedCourseIndex], 0);
        deleteCurrentTaskSubtree();
    } else {
        drawTextSequence(resource);
    }
}

void advanceModeAfterTextSequenceFadeTask(void) {
    TextSequenceResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    switch (gCurrentTask->data18) {
    case 1:
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        resource->intensity = 0;
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013B55E = 2;
            gModeSequenceStage++;
            deleteCurrentTaskSubtree();
        }
        break;
    }
    drawTextSequence(resource);
}

void queueModeIndexedSectorRead(s16 mode, s16 index) {
    if (mode == 1) {
        queueCdSectorCommand(0, gMode1SectorRanges[index].sector,
                      gMode1SectorRanges[index].sectorCount,
                      INDEXED_SECTOR_READ_DESTINATION, 0);
    } else {
        queueCdSectorCommand(0, gOtherModeSectorRanges[index].sector,
                      gOtherModeSectorRanges[index].sectorCount,
                      INDEXED_SECTOR_READ_DESTINATION, 0);
    }
}

void parseLoadedTextSequence(void) {
    TextSequenceEntry *entry = TEXT_SEQUENCE_ENTRIES;
    u16 *text = (u16 *)INDEXED_SECTOR_READ_DESTINATION;
    TextSequenceEntry *metadata = entry;
    s16 entryIndex;

    for (entryIndex = 0; entryIndex < TEXT_SEQUENCE_MAX_ENTRIES; entry++) {
        if (*text == 0x807A) {
            entry->text = 0;
            metadata->kind = 2;
            metadata->length = 0;
            return;
        }
        if (*text == 0x8074) {
            text++;
            entry->text = text;
            metadata->kind = 0;
        } else {
            entry->text = text;
            metadata->kind = 1;
        }
        metadata->length = 0;
        for (;;) {
            u16 ch = *text;
            if (ch == 0x800A) {
                text++;
                continue;
            }
            if (ch == 0x8065) {
                *text = 0;
                break;
            }
            if (ch == 0x8020 || !(ch & 0xC000)) {
                metadata->length++;
                text++;
            } else {
                text++;
            }
        }
        text++;
        while (*text++ == 0) {}
        text--;
        entryIndex++;
        metadata++;
        if (entryIndex >= TEXT_SEQUENCE_MAX_ENTRIES) {
            break;
        }
    }
}

static void drawTextSequence(TextSequenceResource *resource) {
    s32 bodyX, bodyY;
    s16 x, y, labelY;
    u16 depth = 6;
    GsOT *ot = &gMainOrderingTables[gActiveGraphicsBuffer];

    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    x = resource->x / 4096;
    y = resource->y / 4096;
    labelY = y;
    if (resource->mode == 1 && resource->unk0A < 9) {
        drawWrappedGlyphText16x16((s16)x, (s16)labelY, 30, resource->headerText, 12, 1, ot, 6);
        drawWrappedGlyphText16x16(-72, (labelY = (s16)(labelY - 24)), 30, gTextSequenceNumberLabel, 13, 1, ot, 6);
        drawDecimalNumber16(40, labelY, 1, 0, resource->unk0A + 1, 13, 1, ot, 6);
    } else if (resource->mode == 2) {
        u16 index = resource->unk0A;
        if (index < 5) {
            drawMenuKindSprite(-16, y, index + 1, 1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        } else {
            drawPaletteMappedIcon(-16, y, index + (u16)-4, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        }
        drawWrappedGlyphText16x16((s16)x, (s16)(y - 24), 30, resource->headerText, 12, 1, ot, depth);
    }

    bodyX = resource->bodyX / 4096;
    bodyY = resource->displacement / 4096;
    drawLimitedWrappedGlyphText16x16(bodyX + 8, bodyY + 4, (s16)(resource->progress >> 8), 16,
                                     resource->bodyText, 15, 1, ot, (u16)depth);
    drawDefaultTexturedPanel(bodyX, bodyY, 272, 74, 5, 0, ot, depth);

    drawSpriteByIndexWithBlend((u16)-136 - (u32)resource->unk3A / 3, (u16)-96 - (u32)resource->unk3A / 2,
                              0x198, 0, -1, ot, (depth | 1));
    drawSpriteByIndexWithBlend(120 + (u32)resource->unk3A / 3, (u16)-96 - (u32)resource->unk3A / 2,
                              0x199, 0, -1, ot, (depth | 1));
    drawSpriteByIndexWithBlend((u16)-136 - (u32)resource->unk3A / 3, (u32)resource->unk3A / 2,
                              0x19A, 0, -1, ot, (depth | 1));
    drawSpriteByIndexWithBlend(120 + (u32)resource->unk3A / 3, (u32)resource->unk3A / 2,
                              0x19B, 0, -1, ot, (depth | 1));

    addTilePacketWithBlend(-gDisplayWidth / 2, -gDisplayHeight / 2 - (u32)resource->unk3A / 2,
                  gDisplayWidth, -96 - (-gDisplayHeight / 2), 0, 0, 0, -1, ot, (depth | 1));
    addTilePacketWithBlend(-gDisplayWidth / 2 - (u32)resource->unk3A / 3, -gDisplayHeight / 2,
                  -136 - (-gDisplayWidth / 2), gDisplayHeight, 0, 0, 0, -1, ot, (depth | 1));
    addTilePacketWithBlend(136 + (u32)resource->unk3A / 3, -gDisplayHeight / 2,
                  (u32)gDisplayWidth / 2 - 136, gDisplayHeight, 0, 0, 0, -1, ot, (depth | 1));
    addTilePacketWithBlend(-gDisplayWidth / 2, 16 + (u32)resource->unk3A / 2,
                  gDisplayWidth, (u32)gDisplayHeight / 2, 0, 0, 0, -1, ot, (depth | 1));
    drawScreenDarkeningOverlay(resource->intensity, ot, depth);
}
