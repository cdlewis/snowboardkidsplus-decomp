#include "game/io/dat_resources.h"
#include "common.h"
#include "game/scene/scene_sound_slots.h"
#include "game/io/controller.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/scene/scene_events.h"
#include "game/scene/scene_task_types.h"

/* Preserve the zero word following the playback dispatch table. */
static const u32 sScenePlaybackTablePadding
    __attribute__((section(".rodata.scene_playback_padding"), aligned(4))) = 0;

void initializeLoadedSceneEvents(void) {
    SceneEventListHeader *header = &DAT_SCENE_WORKSPACE->header;
    SceneEvent *freeEntries = header->freeEntries;
    SceneEvent *activeEntries = header->activeEntries;
    gFreeSceneEvents = freeEntries;
    gActiveSceneEvents = activeEntries;
    parseLoadedSceneTextBlocks();
    setCurrentTaskCallback(initializeScenePlaybackTask);
}

void initializeScenePlaybackTask(void) {
    ScenePlaybackState *state;
    s16 i;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (ScenePlaybackState *)&D_800D9504[D_8013AE88->resource16];
    state->currentEntry = findSceneEvent(0, 0);
    if (state->currentEntry == NULL) {
        state->currentEntry = findSceneEventInsertionPoint(0, 0);
    }
    for (i = 0; i < 5; i++) {
        state->unk04[i] = 0;
        state->unk18[i] = 0;
    }
    state->tick = -1;
    state->flags = 0;
    state->imageId = 0xFFFF;
    state->completionTick = 0;
    for (i = 0; i < 8; i++) {
        gSceneSoundSlots[i].voice = 0xFF;
        gSceneSoundSlots[i].soundKey = 0;
    }
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(2, 5, 30);
    D_8013B59C = 0;
    D_8013B59E = 0;
    D_8013B59F = 0;
    setCurrentTaskCallback(updateScenePlaybackTask);
}

void updateScenePlaybackTask(void) {
    ScenePlaybackState *state;
    SceneEvent *entry;
    s16 i;
    s16 ready;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (ScenePlaybackState *)&D_800D9504[D_8013AE88->resource16];
    if (state->flags & 1) {
        return;
    }
    if (++state->tick == 0x7FFFFFFF) {
        setCurrentTaskCallback(beginScenePlaybackExitTask);
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
        setCurrentTaskCallback(beginScenePlaybackExitTask);
        return;
    }
    ready = 1;
    for (i = 0; i < 5; i++) {
        if (state->tasks[i] != NULL) {
            ready = 0;
            break;
        }
    }
    if (ready && (u32)state->tick < state->completionTick) {
        state->tick = state->completionTick;
    }
    while (entry->tick <= (u32)state->tick) {
        switch (entry->type) {
        case SCENE_EVENT_IMAGE:
            setSceneImageEventTask(entry);
            break;
        case SCENE_EVENT_TEXT:
            setSceneTextEventTask(entry);
            break;
        case SCENE_EVENT_MUSIC:
            setSceneMusicEventTask(entry);
            break;
        case SCENE_EVENT_CD_AUDIO:
            setSceneCdAudioEventTask(entry);
            break;
        case SCENE_EVENT_SOUND:
            processSceneSoundEvent(entry);
            break;
        }
        entry = entry->next;
        if (entry == NULL) {
            break;
        }
    }
    state->currentEntry = entry;
}
