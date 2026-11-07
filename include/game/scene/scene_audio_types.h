#pragma once

#include "common.h"
#include "game/engine/task.h"
#include "game/scene/scene_events.h"

typedef struct SceneMusicOwner {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0xC];
    GameTask *musicTask;
    u8 unk38[8];
    u32 completionTick;
} SceneMusicOwner;

typedef struct SceneCdAudioPlaybackOwner {
    u8 unk00[0x24];
    u32 tick;
    u16 flags;
    u8 unk2A[0xE];
    GameTask *task;
    u8 unk3C[4];
    u32 completionTick;
} SceneCdAudioPlaybackOwner;

typedef struct SceneMusicResource {
    SceneEvent *entry;
    u32 deadline;
} SceneMusicResource;

typedef struct SceneCdAudioOwner {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0x10];
    GameTask *task;
} SceneCdAudioOwner;

typedef struct SceneCdAudioResource {
    SceneEvent *entry;
    u32 deadline;
} SceneCdAudioResource;

typedef struct SceneImageResource {
    SceneEvent *entry;
    u32 deadline;
} SceneImageResource;

typedef struct CdResourceLocation {
    s16 sector;
    s16 count;
} CdResourceLocation;
