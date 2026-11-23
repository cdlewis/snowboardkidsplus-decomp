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
#define TEXT_SEQUENCE_ADVANCE_BUTTONS 0xB0

typedef struct TextSequenceEntry {
    const u16 *text;
    u16 kind;
    u16 length;
} TextSequenceEntry;

extern u8 D_8013AFD8 __attribute__((section(".data")));
extern s16 D_8013AFCE __attribute__((section(".data")));
extern u16 D_8013AF7C __attribute__((section(".data")));
extern s16 D_8013AF80 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));

extern u16 D_8013AFAA __attribute__((section(".data")));

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

void initializeTextSequenceTask(void) {
    TextSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    panel = (ValuePanelEntry *)&D_800D9504[D_8013AE88->resource16];

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
                D_8013AE88->data18 = 1;
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
            if (D_8013AEE8[0] & TEXT_SEQUENCE_ADVANCE_BUTTONS) {
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
    func_800A1774(resource);
}

void animateTextSequenceExitTask(void) {
    TextSequenceResource *resource;
    u16 previous;
    s16 delta;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    resource->velocity += 0x1000;
    resource->y -= resource->velocity;
    resource->displacement += resource->velocity;
    if (resource->intensity != 0) resource->intensity -= 4;
    previous = D_800EC158[1].h;
    D_800EC158[1].h = resource->displacement / 4096 - (u32)(-D_8013AF7C / 2 + 4);
    if (D_800EC158[1].h > D_8013AF7C) D_800EC158[1].h = D_8013AF7C;
    delta = D_800EC158[1].h - previous;
    resource->unk3A += delta;
    D_8013AF80 -= delta / 2;
    if (D_8013AF80 < 0) D_8013AF80 = 0;
    D_800EC158[2].y += delta;
    D_800EC158[2].h -= delta;
    if (D_800EC158[2].h < 0) {
        D_800EC158[2].y = 240;
        D_800EC158[2].h = 0;
    }
    D_801241E8[0].unk10 += delta * 2;
    if (D_801241E8[0].unk10 > -320) D_801241E8[0].unk10 = -320;
    if (resource->displacement / 4096 >= 141) {
        taskTransitionFlagGp = 1;
        D_8013B1A8 = 1;
        D_8013AFCD = 1;
        D_8013AFE4 = 0;
        D_8013AFAA |= 0x1000;
        queueCdCommand(D_80124164[D_8013AFCE], 0);
        deleteCurrentTaskSubtree();
    } else {
        func_800A1774(resource);
    }
}

void advanceModeAfterTextSequenceFadeTask(void) {
    TextSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 1:
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        resource->intensity = 0;
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013B55E = 2;
            D_8013AFD8++;
            deleteCurrentTaskSubtree();
        }
        break;
    }
    func_800A1774(resource);
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

/* DECOMP_ATTEMPT: inline 63/65 */
#ifdef NON_MATCHING
typedef struct LoadedTextDescriptor {
    u16 *text;
    u16 type;
    u16 width;
} LoadedTextDescriptor;

void func_800A1670(void) {
    LoadedTextDescriptor *entry = (LoadedTextDescriptor *)0x801F2000;
    u16 *text = (u16 *)0x801F2080;
    LoadedTextDescriptor *current;
    s16 i;
    for (i = 0; i < 16; entry++) {
        current = entry;
        if (*text == 0x807A) {
            current->text = 0;
            current->type = 2;
            current->width = 0;
            return;
        }
        if (*text == 0x8074) {
            text++;
            current->text = text;
            current->type = 0;
        } else {
            current->text = text;
            current->type = 1;
        }
        current->width = 0;
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
                current->width++;
                text++;
            } else {
                text++;
            }
        }
        text++;
        while (*text++ == 0) {}
        text--;
        i++;
        if (i >= 16) break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/text/text_sequence", func_800A1670);
#endif

/* DECOMP_ATTEMPT: inline 366/395 */
#ifdef NON_MATCHING
extern u16 D_801327B0[];

void func_800A1774(TextSequenceResource *resource) {
    s32 x, y, bodyX, bodyY, labelY;
    s32 depth = 6;
    GsOT *ot = &D_800EBFF0[D_8013AF78];

    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    x = resource->x / 4096;
    y = resource->y / 4096;
    labelY = y;
    if (resource->mode == 1 && resource->unk0A < 9) {
        func_8006EF30((s16)x, (s16)labelY, 30, resource->headerText, 12, 1, ot, depth);
        func_8006EF30(-72, (labelY = (s16)(labelY - 24)), 30, D_801327B0, 13, 1, ot, depth);
        drawDecimalNumber16(40, labelY, 1, 0, resource->unk0A + 1, 13, 1, ot, depth);
    } else if (resource->mode == 2) {
        u16 index = resource->unk0A;
        if (index < 5) {
            drawMenuKindSprite(-16, y, index + 1, 1, &D_800EBFF0[D_8013AF78], 4);
        } else {
            drawPaletteMappedIcon(-16, y, index + (u16)-4, &D_800EBFF0[D_8013AF78], 4);
        }
        func_8006EF30((s16)x, (s16)(y - 24), 30, resource->headerText, 12, 1, ot, depth);
    }

    bodyX = resource->bodyX / 4096;
    bodyY = resource->displacement / 4096;
    drawLimitedWrappedGlyphText16x16(bodyX + 8, bodyY + 4, (s16)(resource->progress >> 8), 16,
                                     resource->bodyText, 15, 1, ot, depth);
    drawDefaultTexturedPanel(bodyX, bodyY, 272, 74, 5, 0, ot, depth);

    func_8006C998((u16)-136 - (u32)resource->unk3A / 3, (u16)-96 - (u32)resource->unk3A / 2,
                  0x198, 0, -1, ot, (depth | 1));
    func_8006C998(120 + (u32)resource->unk3A / 3, (u16)-96 - (u32)resource->unk3A / 2,
                  0x199, 0, -1, ot, (depth | 1));
    func_8006C998((u16)-136 - (u32)resource->unk3A / 3, (u32)resource->unk3A / 2,
                  0x19A, 0, -1, ot, (depth | 1));
    func_8006C998(120 + (u32)resource->unk3A / 3, (u32)resource->unk3A / 2,
                  0x19B, 0, -1, ot, (depth | 1));

    func_8007240C(-D_8013AF7A / 2, -D_8013AF7C / 2 - (u32)resource->unk3A / 2,
                  D_8013AF7A, -96 - (-D_8013AF7C / 2), 0, 0, 0, -1, ot, (depth | 1));
    func_8007240C(-D_8013AF7A / 2 - (u32)resource->unk3A / 3, -D_8013AF7C / 2,
                  -136 - (-D_8013AF7A / 2), D_8013AF7C, 0, 0, 0, -1, ot, (depth | 1));
    func_8007240C(136 + (u32)resource->unk3A / 3, -D_8013AF7C / 2,
                  (u32)D_8013AF7A / 2 - 136, D_8013AF7C, 0, 0, 0, -1, ot, (depth | 1));
    func_8007240C(-D_8013AF7A / 2, 16 + (u32)resource->unk3A / 2,
                  D_8013AF7A, (u32)D_8013AF7C / 2, 0, 0, 0, -1, ot, (depth | 1));
    func_80070C5C(resource->intensity, ot, depth);
}
#else
INCLUDE_ASM("asm/nonmatchings/text/text_sequence", func_800A1774);
#endif
