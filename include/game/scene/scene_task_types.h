#pragma once

#include "common.h"
#include "game/engine/task.h"
#include "game/scene/scene_events.h"

typedef struct SceneControlState {
    s16 unk00;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u8 unk10[8];
    s16 unk18[5];
} SceneControlState;

typedef struct SceneChannelState {
    SceneEvent *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
} SceneChannelState;

typedef struct SceneEventListHeader {
    SceneEvent *freeEntries;
    SceneEvent *activeEntries;
} SceneEventListHeader;

/* This workspace overlays the shared DAT staging buffer during scene playback. */
typedef struct SceneEventWorkspace {
    SceneEventListHeader header;
    SceneEvent entries[295];
} SceneEventWorkspace;

typedef char SceneEventListHeaderSizeCheck[(sizeof(SceneEventListHeader) == 8) ? 1 : -1];
typedef char SceneEventSizeCheck[(sizeof(SceneEvent) == 0x18) ? 1 : -1];
typedef char SceneEventWorkspaceSizeCheck[(sizeof(SceneEventWorkspace) == 0x1BB0) ? 1 : -1];

typedef union SceneTextMetadata {
    u32 marker;
    struct {
        u16 characterCount;
        u8 unk06, unk07, unk08, unk09, unk0A, unk0B;
    } fields;
} SceneTextMetadata;

typedef struct SceneTextBlock {
    u8 *text;
    SceneTextMetadata metadata;
} SceneTextBlock;

/* The text file follows the 100-entry block table in the DAT staging buffer. */
typedef struct SceneTextWorkspace {
    SceneEventWorkspace events;
    SceneTextBlock blocks[100];
    u8 text[0];
} SceneTextWorkspace;

typedef char SceneTextBlockSizeCheck[(sizeof(SceneTextBlock) == 0xC) ? 1 : -1];
typedef char SceneTextWorkspaceSizeCheck[(sizeof(SceneTextWorkspace) == 0x2060) ? 1 : -1];

typedef struct ScenePlaybackState {
    SceneEvent *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
    u8 unk22[2];
    s32 tick;
    u16 flags, imageId;
    GameTask *tasks[5]; /* Indexed by SceneEventType. */
    u32 completionTick;
    u8 unk44[0xBC];
} ScenePlaybackState;

typedef struct SceneTextTaskResource {
    SceneEvent *entry;
    u8 unk04[0x2C];
    GameTask *task;
    u8 unk34[0xCC];
} SceneTextTaskResource;

typedef char ScenePlaybackStateSizeCheck[(sizeof(ScenePlaybackState) == 0x100) ? 1 : -1];
typedef char SceneTextTaskResourceSizeCheck[(sizeof(SceneTextTaskResource) == 0x100) ? 1 : -1];

void resetScenePlaybackStates(SceneControlState *control, SceneChannelState *resource);
