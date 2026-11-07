#include "common.h"
#include "main_tail_sound_slots.h"
#include "game/controller.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/text_entries.h"

typedef struct TextPlaybackState {
    TextEntry *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
    u8 unk22[2];
    s32 tick;
    u16 flags, unk2A;
    GameTask *tasks[5];
    u32 unk40;
    u8 unk44[0xBC];
} TextPlaybackState;

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;

/* Preserve the zero word following the playback dispatch table. */
static const u32 sTextPlaybackTablePadding
    __attribute__((section(".rodata.text_playback_padding"), aligned(4))) = 0;

void initializeLoadedTextEntries(void) {
    TextEntryListHeader *header = (TextEntryListHeader *)0x80180000;
    TextEntry *freeEntries = header->freeEntries;
    TextEntry *activeEntries = header->activeEntries;
    D_8013B5D0 = freeEntries;
    D_8013B5D4 = activeEntries;
    func_800A5AFC();
    setCurrentTaskCallback(initializeTextPlaybackTask);
}

void initializeTextPlaybackTask(void) {
    TextPlaybackState *state;
    s16 i;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (TextPlaybackState *)&D_800D9504[D_8013AE88->resource16];
    state->currentEntry = findKeyedListNode(0, 0);
    if (state->currentEntry == NULL) {
        state->currentEntry = findTextEntryInsertionPoint(0, 0);
    }
    for (i = 0; i < 5; i++) {
        state->unk04[i] = 0;
        state->unk18[i] = 0;
    }
    state->tick = -1;
    state->flags = 0;
    state->unk2A = 0xFFFF;
    state->unk40 = 0;
    for (i = 0; i < 8; i++) {
        gTextSoundSlots[i].voice = 0xFF;
        gTextSoundSlots[i].soundKey = 0;
    }
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(2, 5, 30);
    D_8013B59C = 0;
    D_8013B59E = 0;
    D_8013B59F = 0;
    setCurrentTaskCallback(updateTextPlaybackTask);
}

void updateTextPlaybackTask(void) {
    TextPlaybackState *state;
    TextEntry *entry;
    s16 i;
    s16 ready;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (TextPlaybackState *)&D_800D9504[D_8013AE88->resource16];
    if (state->flags & 1) {
        return;
    }
    if (++state->tick == 0x7FFFFFFF) {
        setCurrentTaskCallback(beginAudioFadeCountdownTask);
        return;
    }
    ready = 0;
    if (D_8013AEE8[0] & 0x800) {
        ready = 1;
    }
    entry = state->currentEntry;
    if (entry == NULL) {
        ready = 1;
        for (i = 0; i < 5; i++) {
            if (state->tasks[i] != NULL) {
                return;
            }
        }
    }
    if (ready) {
        setCurrentTaskCallback(beginAudioFadeCountdownTask);
        return;
    }
    ready = 1;
    for (i = 0; i < 5; i++) {
        if (state->tasks[i] != NULL) {
            ready = 0;
            break;
        }
    }
    if (ready && (u32)state->tick < state->unk40) {
        state->tick = state->unk40;
    }
    /* In the playback list, the entry key is its scheduled tick. */
    while ((u32)entry->text <= (u32)state->tick) {
        switch (entry->unk04) {
        case 0:
            setSecondaryResourceEntryTask(entry);
            break;
        case 1:
            setSecondaryTextEntryTask(entry);
            break;
        case 2:
            setTextEntryMusicTask(entry);
            break;
        case 3:
            setTextEntryCdAudioTask(entry);
            break;
        case 4:
            processTextEntrySound(entry);
            break;
        }
        entry = entry->next;
        if (entry == NULL) {
            break;
        }
    }
    state->currentEntry = entry;
}
