#pragma once

#include "common.h"
#include "game/engine/task.h"
#include "game/text/text_entries.h"

typedef struct TextControlState {
    s16 unk00;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u8 unk10[8];
    s16 unk18[5];
} TextControlState;

typedef struct TextResourceState {
    TextEntry *currentEntry;
    u32 unk04[5];
    s16 unk18[5];
} TextResourceState;

typedef struct TextEntryListHeader {
    TextEntry *freeEntries;
    TextEntry *activeEntries;
} TextEntryListHeader;

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

typedef struct TextTaskResource {
    TextEntry *entry;
    u8 unk04[0x2C];
    GameTask *task;
    u8 unk34[0xCC];
} TextTaskResource;

typedef char TextPlaybackStateSizeCheck[(sizeof(TextPlaybackState) == 0x100) ? 1 : -1];
typedef char TextTaskResourceSizeCheck[(sizeof(TextTaskResource) == 0x100) ? 1 : -1];
