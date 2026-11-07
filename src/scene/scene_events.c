#include "game/text/text_layout.h"
#include "../menu/selection_tasks_internal.h"

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

INCLUDE_ASM("asm/nonmatchings/scene/scene_events", parseLoadedSceneTextBlocks);

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

INCLUDE_ASM("asm/nonmatchings/scene/scene_events", initializeSceneEvent);

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
