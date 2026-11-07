#pragma once

#include "decomp/types.h"

typedef enum SceneEventType {
    SCENE_EVENT_IMAGE = 0,
    SCENE_EVENT_TEXT = 1,
    SCENE_EVENT_MUSIC = 2,
    SCENE_EVENT_CD_AUDIO = 3,
    SCENE_EVENT_SOUND = 4
} SceneEventType;

/* Sorted by scheduled tick, then event type. Text payloads live in a separate
 * block table; the first word of this record is never a string pointer.
 */
typedef struct SceneEvent {
    u32 tick;
    u16 type; /* SceneEventType, stored as a halfword on disc. */
    u16 resourceId; /* Event-specific index; zero commonly stops a channel. */
    u16 mode; /* Event-specific playback, transition or sound operation. */
    u16 duration;
    struct SceneEvent *previous;
    struct SceneEvent *next;
    u16 parameter14; /* Volume/repeats for audio; text reveal interval for text. */
    u16 unk16;
} SceneEvent;
extern SceneEvent *gFreeSceneEvents;
extern SceneEvent *gActiveSceneEvents;
extern s16 D_8013B59C;
extern u8 D_8013B59E;
extern u8 D_8013B59F;

void resetSceneEventPool(void);
void initializeSceneEvent(SceneEvent *entry, u32 tick, u16 type);
SceneEvent *findSceneEvent(u32 tick, u32 type);
SceneEvent *findSceneEventInsertionPoint(u32 tick, u32 type);
void parseLoadedSceneTextBlocks(void);

void initializeScenePlaybackResourcesTask(void);
void initializeLoadedSceneEvents(void);
void initializeScenePlaybackTask(void);
void updateScenePlaybackTask(void);
void beginScenePlaybackExitTask(void);
void finishScenePlaybackExitTask(void);

void setSceneImageEventTask(SceneEvent *entry);
void initializeSceneImageTask(void);
void updateSceneImageTask(void);
void drawSceneImageTask(void);
void setSceneTextEventTask(SceneEvent *entry);
void updateSceneTextTask(void);
void setSceneMusicEventTask(SceneEvent *entry);
void finishSceneMusicTaskAtDeadline(void);
void setSceneCdAudioEventTask(SceneEvent *entry);
void updateSceneCdAudioTask(void);
void processSceneSoundEvent(SceneEvent *entry);
void stopSceneSoundVoices(void);
