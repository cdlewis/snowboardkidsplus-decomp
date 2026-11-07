#pragma once

#include "decomp/types.h"

typedef struct TextEntry {
    const char *text;
    u16 unk04;
    u16 unk06;
    u16 unk08;
    u16 unk0A;
    struct TextEntry *previous;
    struct TextEntry *next;
    u16 unk14;
    u16 unk16;
} TextEntry;
extern TextEntry *D_8013B5D0;
extern TextEntry *D_8013B5D4;
extern s16 D_8013B59C;
extern u8 D_8013B59E;
extern u8 D_8013B59F;

void resetTextEntryPool(void);
void initializeLoadedTextEntries(void);

s16 parseDecimalS16(const u8 *text, s16 *result);

TextEntry *findTextEntryInsertionPoint(u32 key, u32 value);

s16 advanceTextLine(const s8 **text, s16 limit);

s16 countWrappedTextLines(const s8 *text, s16 width, s16 limit);

void setSecondaryResourceEntryTask(TextEntry *entry);

void setTextEntryCdAudioTask(TextEntry *entry);

void processTextEntrySound(TextEntry *entry);

void setTextEntryMusicTask(TextEntry *entry);
void initializeSecondaryResourceTask(void);
void updateSecondaryResourceTask(void);
void drawSecondaryTextBackgroundTask(void);
void func_800A624C(void);

TextEntry *findKeyedListNode(u32 key, u32 value);
void func_800A5AFC(void);
void initializeTextPlaybackTask(void);
void updateTextPlaybackTask(void);
void beginAudioFadeCountdownTask(void);
void setSecondaryTextEntryTask(TextEntry *entry);
void setTextEntryMusicTask(TextEntry *entry);
void processTextEntrySound(TextEntry *entry);
