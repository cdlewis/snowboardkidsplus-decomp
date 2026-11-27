#include "game/text/text_layout.h"
#include "../menu/selection_tasks_internal.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/scene/scene_events.h"
#include "game/scene/scene_task_types.h"

SceneEvent *gFreeSceneEvents __attribute__((section(".sdata.scene_event_lists"))) = NULL;
SceneEvent *gActiveSceneEvents __attribute__((section(".sdata.scene_event_lists"))) = NULL;

u8 gSelectedUnlockIndex __attribute__((section(".data.selected_unlock_index"))) = 0;
const u8 gSelectedUnlockIndexPadding __attribute__((section(".data.selected_unlock_index"))) = 0;

void func_800A5AD4(void);
void func_800A5ADC(void);
void func_800A5AE4(void);
void func_800A5AEC(void);
void func_800A5AF4(void);

u8 gSceneCdAudioDefaultResources[12] __attribute__((section(".data.scene_cd_audio_default_resources"))) = {
    0x00, 0x54, 0x60, 0x75, 0x82, 0x92, 0x9E, 0xB0, 0xC3, 0xD4, 0xDB, 0xE3
};

void (*gSceneEmptyCallbacks[5])(void) __attribute__((section(".data.scene_empty_callbacks"))) = {
    func_800A5AD4, func_800A5ADC, func_800A5AE4, func_800A5AEC, func_800A5AF4,
};

void func_800A5AD4(void) {
}

void func_800A5ADC(void) {
}

void func_800A5AE4(void) {
}

void func_800A5AEC(void) {
}

void func_800A5AF4(void) {
}

void parseLoadedSceneTextBlocks(void) {
    SceneTextBlock *block = ((SceneTextWorkspace *)DAT_SCRATCH_ADDRESS)->blocks;
    u8 *text = ((SceneTextWorkspace *)DAT_SCRATCH_ADDRESS)->text;
    s16 i = 0;
    s16 value;
    do {
        text += parseDecimalS16(text, &value);
        text++;
        block->metadata.fields.unk06 = value;
        text += parseDecimalS16(text, &value);
        text++;
        block->metadata.fields.unk07 = value;
        text += parseDecimalS16(text, &value);
        text++;
        block->metadata.fields.unk08 = value;
        text += parseDecimalS16(text, &value);
        text++;
        block->metadata.fields.unk09 = value;
        text += parseDecimalS16(text, &value);
        text++;
        block->metadata.fields.unk0A = value;
        text += parseDecimalS16(text, &value);
        text++;
        block->metadata.fields.unk0B = value;
        while (*text != '\r') {
            text++;
        }
        text += 2;
        if ((block->metadata.marker & 0xFFFF0000) == 0xFFFF0000) break;
        text += 2;
        block->text = text;
        block->metadata.fields.characterCount = 0;
        while (1) {
            u8 ch;
            s32 spaceCharacter = ' ';
            if (text[0] == '\r' && text[1] == '\n') {
                text += 2;
                continue;
            }
            ch = *text;
            if (ch == '@' && text[1] == 'e') goto end_text;
            if (ch & 0x80) {
                block->metadata.fields.characterCount++;
                text += 2;
            } else if (ch == spaceCharacter) {
                block->metadata.fields.characterCount++;
                text++;
                if (text[1] != ' ') continue;
                text++;
            } else {
                text += 2;
            }
        }
next_block:
        while (*text != '\r') {
            text++;
        }
        text += 2;
        if (++i >= 100) break;
        block++;
        continue;
end_text:
        /* Remove the CRLF immediately before the end marker. */
        text[-1] = 0;
        text[-2] = 0;
        text += 2;
        goto next_block;
    } while (1);
}

s16 parseDecimalS16(const u8 *text, s16 *result) {
    s16 value = 0;
    s16 count = 0;
    u16 digit;
    while ((u32)(*text - '0') < 10) {
        count++;
        digit = *text++;
        value = value * 10 + (digit - (u16)'0');
    }
    *result = value;
    return count;
}

void resetScenePlaybackStates(SceneControlState *control, SceneChannelState *resource) {
    s16 i;
    if (control != NULL) {
        control->unk00 = 0;
        control->unk02 = 1;
        control->unk04 = 0;
        control->unk08 = 0;
        control->unk0C = 0;
        for (i = 0; i < 5; i++) {
            control->unk18[i] = 0;
        }
    }
    if (resource != NULL) {
        for (i = 0; i < 5; i++) {
            resource->unk04[i] = 0;
            resource->unk18[i] = 0;
        }
    }
}

void resetSceneEventPool(void) {
    SceneEvent *previous = NULL;
    SceneEvent *entry = DAT_SCENE_WORKSPACE->entries;
    u16 i;
    gFreeSceneEvents = entry;
    for (i = 0; i < 295; i++) {
        entry->previous = previous;
        previous = entry;
        previous->type = 0;
        previous->resourceId = 0;
        previous->mode = 0;
        previous->duration = 0;
        previous->next = previous + 1;
        entry++;
    }
    entry[-1].next = NULL;
    gActiveSceneEvents = NULL;
}

void initializeSceneEvent(SceneEvent *entry, u32 tick, u16 type) {
    entry->type = type;
    entry->tick = tick;
    if (type == SCENE_EVENT_CD_AUDIO) {
        entry->resourceId = gSceneCdAudioDefaultResources[gSelectedUnlockIndex];
    } else {
        entry->resourceId = 0;
    }
    entry->mode = 0;
    entry->duration = 0;
    entry->parameter14 = 0;
}

SceneEvent *findSceneEvent(u32 tick, u32 type) {
    SceneEvent *node = gActiveSceneEvents;
    SceneEvent *result = NULL;
    while (node != NULL) {
        if (tick == node->tick && type == node->type) {
            result = node;
            break;
        }
        node = node->next;
        result = NULL;
    }
    return result;
}

SceneEvent *findSceneEventInsertionPoint(u32 tick, u32 type) {
    SceneEvent *node = gActiveSceneEvents;
    while (node != NULL) {
        if (tick == node->tick && type < node->type) {
            break;
        }
        if (tick < node->tick) {
            break;
        }
        node = node->next;
    }
    return node;
}

s16 countWrappedTextLines(const s8 *text, s16 width, s16 limit) {
    s16 count = 0;
    s16 column = 0;
    s16 lines = 1;
    while (*text != 0) {
        if (count >= limit) break;
        if (column >= width) {
            if (*text == '\r' && text[1] == '\n') text += 2;
            lines++;
            column = 0;
        }
        if (*text == '\r' && text[1] == '\n') {
            lines++;
            column = 0;
            text += 2;
        }
        if (*text & 0x80) {
            column++;
            count++;
            text += 2;
        } else if (*text == ' ') {
            column++;
            text++;
            count++;
            if (*text != ' ') continue;
            text++;
            continue;
        } else {
            text += 2;
        }
    }
    return lines;
}

s16 advanceTextLine(const s8 **text, s16 limit) {
    const s8 *p = *text;
    s16 count = 0;
    while (*p != 0) {
        if (count >= limit) {
            if (*p == '\r' && p[1] == '\n') {
                p += 2;
            }
            break;
        }
        if (*p == '\r' && p[1] == '\n') {
            p += 2;
            break;
        }
        if (*p & 0x80) {
            count++;
            p += 2;
        } else if (*p == ' ') {
            p++;
            count++;
            if (*p != ' ') {
                continue;
            }
            p++;
            continue;
        } else {
            p += 2;
        }
    }
    *text = p;
    return count;
}
