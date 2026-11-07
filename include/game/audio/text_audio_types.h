#pragma once

#include "common.h"
#include "game/engine/task.h"
#include "game/text/text_entries.h"

typedef struct MusicOwnerState {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0xC];
    GameTask *musicTask;
    u8 unk38[8];
    u32 completionTick;
} MusicOwnerState;

typedef struct CdAudioPlaybackOwner {
    u8 unk00[0x24];
    u32 tick;
    u16 flags;
    u8 unk2A[0xE];
    GameTask *task;
    u8 unk3C[4];
    u32 completionTick;
} CdAudioPlaybackOwner;

typedef struct MusicResource {
    TextEntry *entry;
    u32 deadline;
} MusicResource;

typedef struct CdAudioOwner {
    u8 unk00[0x24];
    u32 tick;
    u8 unk28[0x10];
    GameTask *task;
} CdAudioOwner;

typedef struct CdAudioResource {
    TextEntry *entry;
    u32 deadline;
} CdAudioResource;

typedef struct SecondaryResource {
    TextEntry *entry;
    u32 deadline;
} SecondaryResource;

typedef struct CdResourceLocation {
    s16 sector;
    s16 count;
} CdResourceLocation;
