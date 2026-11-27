#include "save_record_tasks_internal.h"
#include "game/menu/save_record_tasks.h"
#include "game/io/cd_queue.h"
#include "game/io/memory_card.h"
#include "game/menu/task_widgets.h"
#include "task_widget_data.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/menu/movement.h"
#include "game/menu/selected_records.h"
#include "game/race/player/player_resource_textures.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"

#define MEMORY_CARD_SAVE_BUFFER ((SaveRecord *)0x801E4000)
#define PANEL_OT (&gMainOrderingTables[gActiveGraphicsBuffer])
#define PANEL_DEPTH (gCurrentTask->data1C)

typedef struct SaveMenuResource {
    u8 unk00[4];
    s16 command04;
    s16 busy06;
    s16 unk08;
    s16 slot;
    s16 command0C;
    s16 result0E;
    u8 unk10[14];
    s16 result1E;
} SaveMenuResource;

u8 gSaveRecordSavedOrLoaded
    __attribute__((section(".sdata.save_record_saved_or_loaded"), aligned(4))) = 0;

u8 gSaveRecordSavedOrLoadedPadding0
    __attribute__((section(".sdata.save_record_saved_or_loaded"))) = 0;

u8 gSaveRecordSavedOrLoadedPadding1
    __attribute__((section(".sdata.save_record_saved_or_loaded"))) = 0;

u8 gSaveRecordSavedOrLoadedPadding2
    __attribute__((section(".sdata.save_record_saved_or_loaded"))) = 0;

const u16 gMemoryCardAccessInProgressText[38]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00A7, 0x0042, 0x0049,
    0x004F, 0x004E, 0x800A, 0x00DD, 0x00CD, 0x009E, 0x0033, 0x0115,
    0x0116, 0x00A7, 0x009C, 0x00BD, 0x00B8, 0x00A6, 0x0093, 0x00CD,
    0x0099, 0x00CA, 0x009C, 0x0093, 0x0033, 0x0000,
};

const u16 gMemoryCardMissingRetryText[32]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00A7, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00C0, 0x0092, 0x00B9,
    0x00B0, 0x009F, 0x800A, 0x00BF, 0x0033, 0x009B, 0x00AA, 0x00B0,
    0x00B0, 0x010B, 0x00B3, 0x00B0, 0x009E, 0x0097, 0x0031, 0x0000,
};

const u16 gMemoryCardLoadErrorRetryText[36]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00A7, 0x007D, 0x002C,
    0x0051, 0x00C0, 0x800A, 0x0092, 0x00B9, 0x00B0, 0x009F, 0x00BF,
    0x0033, 0x009B, 0x00AA, 0x00B0, 0x00B0, 0x010B, 0x00B3, 0x00B0,
    0x009E, 0x0097, 0x0031, 0x0000,
};

const u16 gMemoryCardCorruptDataText[30]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00AA, 0x004F, 0x002C,
    0x0081, 0x007D, 0x800A, 0x002C, 0x0051, 0x00C0, 0x0113, 0x00BB,
    0x00A4, 0x0093, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gMemoryCardLoadRetryConfirmationText[16]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x009B, 0x00AA, 0x00B0, 0x00B0, 0x0073, 0x002C, 0x0062, 0x00BE,
    0x010B, 0x00B3, 0x00B0, 0x009E, 0x0097, 0x0031, 0x0000, 0x0000,
};

const u16 gMemoryCardUnformattedText[30]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00AB, 0x005D, 0x003D,
    0x002C, 0x0060, 0x800A, 0x003E, 0x0055, 0x009C, 0x00BB, 0x00A4,
    0x0093, 0x00B0, 0x009F, 0x00BF, 0x0033, 0x0000,
};

const u16 gMemoryCardFormatConfirmationText[28]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00BE, 0x005D, 0x003D,
    0x002C, 0x0060, 0x800A, 0x003E, 0x0055, 0x009D, 0x00B0, 0x009E,
    0x0097, 0x0031, 0x0000, 0x0000,
};

const u16 gMemoryCardFormatCompleteText[14]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x005D, 0x003D, 0x002C, 0x0060, 0x003E, 0x0055, 0x00DB, 0x00DC,
    0x009D, 0x00B0, 0x009D, 0x00A1, 0x0033, 0x0000,
};

const u16 gMemoryCardFormatFailedText[16]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x005D, 0x003D, 0x002C, 0x0060, 0x003E, 0x0055, 0x00A7, 0x0110,
    0x0111, 0x009D, 0x00B0, 0x009D, 0x00A1, 0x0033, 0x0000, 0x0000,
};

const u16 gMemoryCardText8013049C[36]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00C0, 0x0119, 0x00BB,
    0x0097, 0x00BD, 0x800A, 0x008E, 0x00A4, 0x0093, 0x00B0, 0x009E,
    0x0033, 0x009B, 0x00AA, 0x00B0, 0x00B0, 0x010B, 0x00B3, 0x00B0,
    0x009E, 0x0097, 0x0031, 0x0000,
};

const u16 gMemoryCardText801304E4[36]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00A7, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00BE, 0x010C, 0x010D,
    0x009D, 0x00A4, 0x800A, 0x00A6, 0x00A7, 0x0097, 0x0083, 0x0051,
    0x006F, 0x00BE, 0x0096, 0x009D, 0x00A4, 0x0099, 0x00CA, 0x009C,
    0x0093, 0x0033, 0x0000, 0x0000,
};

const u16 gMemoryCardLoadInProgressText[38]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x8020, 0x8020, 0x8020, 0x007D, 0x002C, 0x0051, 0x00BE, 0x006C,
    0x002C, 0x007E, 0x009D, 0x00A4, 0x0093, 0x00B0, 0x009E, 0x0033,
    0x8020, 0x8020, 0x800A, 0x8020, 0x8020, 0x8020, 0x0115, 0x0116,
    0x00A7, 0x009C, 0x00BD, 0x00B8, 0x00A6, 0x0093, 0x00CD, 0x0099,
    0x00CA, 0x009C, 0x0093, 0x0033, 0x0000, 0x0000,
};

const u16 gMemoryCardSaveInProgressText[38]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x8020, 0x8020, 0x8020, 0x007D, 0x002C, 0x0051, 0x00BE, 0x004F,
    0x002C, 0x0081, 0x009D, 0x00A4, 0x0093, 0x00B0, 0x009E, 0x0033,
    0x8020, 0x8020, 0x800A, 0x8020, 0x8020, 0x8020, 0x0115, 0x0116,
    0x00A7, 0x009C, 0x00BD, 0x00B8, 0x00A6, 0x0093, 0x00CD, 0x0099,
    0x00CA, 0x009C, 0x0093, 0x0033, 0x0000, 0x0000,
};

const u16 gMemoryCardMissingText[22]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00A7, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00C0, 0x0092, 0x00B9,
    0x00B0, 0x009F, 0x800A, 0x00BF, 0x0033, 0x0000,
};

const u16 gMemoryCardFullText[36]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004E, 0x006C, 0x003E, 0x0055, 0x8020, 0x00AA, 0x0063, 0x0064,
    0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00AA, 0x0081, 0x006C,
    0x003E, 0x0049, 0x800A, 0x00F2, 0x00C0, 0x00A1, 0x00B9, 0x00B0,
    0x009F, 0x00BF, 0x0036, 0x00FD, 0x00FE, 0x0081, 0x006C, 0x003E,
    0x0049, 0x0002, 0x0037, 0x0000,
};

const u16 gMemoryCardChangedSaveConfirmationText[26]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x0063, 0x0064, 0x0069, 0x002C, 0x0047, 0x002C, 0x007E, 0x00C0,
    0x0097, 0x0095, 0x00B8, 0x00BB, 0x00B0, 0x009D, 0x00A1, 0x0033,
    0x800A, 0x0114, 0x011D, 0x0098, 0x009D, 0x00B0, 0x009E, 0x0097,
    0x0031, 0x0000,
};

const u16 gMemoryCardLoadSlotChoiceText[20]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x006C, 0x002C, 0x007E, 0x009E, 0x00BA, 0x004E, 0x006C, 0x003E,
    0x0055, 0x00BE, 0x0095, 0x00B8, 0x00BF, 0x00CD, 0x011E, 0x009C,
    0x0093, 0x0033, 0x0000, 0x0000,
};

const u16 gMemoryCardSaveSlotChoiceText[20]
    __attribute__((section(".data.memory_card_menu_text"), aligned(4))) = {
    0x004F, 0x002C, 0x0081, 0x009E, 0x00BA, 0x004E, 0x006C, 0x003E,
    0x0055, 0x00BE, 0x0095, 0x00B8, 0x00BF, 0x00CD, 0x011E, 0x009C,
    0x0093, 0x0033, 0x0000, 0x0000,
};

const u16 gMemoryCardSaveCompleteText[8]
    __attribute__((section(".data.memory_card_result_text"), aligned(4))) = {
    0x00DB, 0x00DC, 0x009D, 0x00B0, 0x009D, 0x00A1, 0x0033, 0x0000,
};

const u16 gMemoryCardLoadOptionsText[14]
    __attribute__((section(".data.memory_card_result_text"), aligned(4))) = {
    0x007D, 0x002C, 0x0051, 0x00BE, 0x006C, 0x002C, 0x007E,
    0x009D, 0x00B0, 0x009E, 0x0097, 0x0031, 0x0000, 0x0000,
};

const u16 gMemoryCardSaveOptionsText[14]
    __attribute__((section(".data.memory_card_result_text"), aligned(4))) = {
    0x007D, 0x002C, 0x0051, 0x00BE, 0x004F, 0x002C, 0x0081,
    0x009D, 0x00B0, 0x009E, 0x0097, 0x0031, 0x0000, 0x0000,
};

const u16 gMemoryCardSaveMenuExitConfirmationText[14]
    __attribute__((section(".data.memory_card_save_confirmation_text"), aligned(4))) = {
    0x004F, 0x002C, 0x0081, 0x009F, 0x00C7, 0x00A7, 0x00DB,
    0x00BD, 0x00B9, 0x00B0, 0x009E, 0x0097, 0x0031, 0x0000,
};

const u16 gMemoryCardSaveConfirmationText[10]
    __attribute__((section(".data.memory_card_save_confirmation_text"), aligned(4))) = {
    0x0114, 0x011D, 0x0098, 0x009D, 0x00B0, 0x009E, 0x0097, 0x0031,
    0x0000, 0x0000,
};

const u16 gMemoryCardOperationFailedText[8]
    __attribute__((section(".data.memory_card_save_confirmation_text"), aligned(4))) = {
    0x0110, 0x0111, 0x009D, 0x00B0, 0x009D, 0x00A1, 0x0033, 0x0000,
};


extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u32 gGameOptionFlags __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

extern s16 gMemoryCardTaskState __attribute__((section(".data")));

const u32 gStateValueHistoryPadding
    __attribute__((section(".rodata.state_value_history_padding"))) = 0;

const u32 gMemoryCardSaveMenuPadding
    __attribute__((section(".rodata.memory_card_save_menu_padding"))) = 0;

u16 gTaskShadowedText[14] __attribute__((section(".data.task_shadowed_text"))) = {
    0x001A, 0x001F, 0x001D, 0x0012, 0x8020, 0x8020, 0x000C,
    0x001F, 0x001E, 0x001E, 0x0019, 0x0018, 0, 0
};

const u16 gMemoryCardSaveRecordHeading[10]
    __attribute__((section(".data.memory_card_save_record_heading"), aligned(4))) = {
    0x000E, 0x000B, 0x001E, 0x000B, 0x8020, 0x001D, 0x000B, 0x0020, 0x000F, 0
};

/* Each six-halfword configuration holds two choices and a cursor as sprite/palette pairs. */
const u16 gSaveRecordConfirmationSprites[42]
    __attribute__((section(".data.memory_card_save_record_sprites"), aligned(4))) = {
    0x0125, 0, 0x0126, 0, 0x00B0, 0,
    0x0123, 0, 0x0125, 0, 0x00B0, 0,
    0x0125, 0, 0x0122, 0, 0x00B0, 0,
    0x0124, 0, 0x0123, 0, 0x00B0, 0,
    0x0125, 0, 0x0126, 0, 0x00B0, 0,
    0x0121, 0, 0x0122, 0, 0x00B0, 0,
    0x0121, 0, 0x0123, 0, 0x00B0, 0,
};

const u16 gMemoryCardSlotChoiceSprites[6]
    __attribute__((section(".data.memory_card_save_record_sprites"), aligned(4))) = {
    0x0235, 0, 0x0236, 0, 0x00B0, 0,
};

SaveRecord gPlayerSaveRecords[2] __attribute__((section(".data.save_records"))) = {0};
SaveRecord gMergedSaveRecord __attribute__((section(".data.merged_save_record"))) = {0};

/* Incomplete arrays let the compiler schedule the address halves separately. */
extern s8 gMemoryCardSaveRecordCurrencyLabel[]
    __attribute__((section(".data.memory_card_save_record_currency_label")));

void drawMemoryCardSaveRecordPanel(void);

void repeatByteStore(u8 *destination, s32 value, s32 count) {
    s16 i;

    /* The original loop writes repeatedly to the same byte. */
    for (i = 0; i < count; i++) {
        *destination = value;
    }
}

void initializeSaveRecordDefaults(SaveRecord *record) {
    s16 i;

    repeatByteStore((u8 *)record, 0, sizeof(*record));
    record->flags = 0;
    record->boardUnlockFlagsLow = 0;
    for (i = 0; i < 10; i++) {
        record->masks[i] = 0;
    }
    record->masks[SAVE_RECORD_COMBINED_MASK_INDEX] = 0;
    record->boardUnlockFlagsHigh = 0;
    for (i = 0; i < 5; i++) {
        record->boardUnlockFlagsLow |= 1 << (i * 5);
        record->boardUnlockFlagsHigh |= 1 << (i * 5);
    }
    record->flags |= 0x124;
    record->money = 0;
    i = 0;
    do {
        record->boardSelections[i] = 0;
        record->paletteSelections[i] = 0;
        i++;
    } while (i < 10);
}

void copyGlobalFlagState(SaveRecord *source, SaveRecord *destination) {
    s16 i;

    destination->flags = source->flags;
    for (i = 0; i < 10; i++) {
        i[destination->masks] = i[source->masks];
    }
    destination->masks[SAVE_RECORD_COMBINED_MASK_INDEX] = source->masks[SAVE_RECORD_COMBINED_MASK_INDEX];
    destination->money = source->money;
}

void mergeSaveRecordFlags(SaveRecord *source, SaveRecord *destination) {
    s16 i;
    destination->flags |= source->flags;
    for (i = 0; i < 10; i++) {
        i[destination->masks] |= i[source->masks];
    }
    destination->masks[SAVE_RECORD_COMBINED_MASK_INDEX] |= source->masks[SAVE_RECORD_COMBINED_MASK_INDEX];
}

void copySaveRecordAndMergeFlags(void) {
    gMergedSaveRecord = gPlayerSaveRecords[0];
    mergeSaveRecordFlags(&gPlayerSaveRecords[1], &gMergedSaveRecord);
}

void applyLoadedSaveRecordSettings(SaveRecord *source, s16 index) {
    s16 i = 0;
    s32 playerIndex = index;

    for (; i < 9; i++) {
        gPlayerResourceTextureSelections.entries[playerIndex][i] = source->resourceTextureSelections[i];
    }
    if (playerCountAbsolute == 1) {
        *(volatile u16 *)&gUnlockNotificationFlags = source->unlockNotificationFlags;
    } else {
        *(volatile u16 *)&gUnlockNotificationFlags = 0;
    }
}

void prepareSaveRecordForWrite(SaveRecord *state) {
    u32 i;

    for (i = 0; i < 9; i++) {
        state->resourceTextureSelections[i] = gPlayerResourceTextureSelections.entries[0][i];
    }
    state->unlockNotificationFlags = gUnlockNotificationFlags;
    state->checksum = calculateByteSum((const u8 *)state, sizeof(*state) - sizeof(state->checksum));
}

u32 calculateByteSum(const u8 *data, u32 size) {
    u32 sum;
    u32 i;

    sum = 0;
    for (i = 0; i < size; i++) {
        sum += *data++;
    }
    return sum;
}

void updateStateValueHistoryTask(void) {
    switch (gCurrentTask->data18) {
    case 0:
        resetMemoryCardTaskState();
        if (gMemoryCardSlotHistory.channel == 0) {
            gMemoryCardSlotHistory.activeIndex = isValue16(gMemoryCardSlotHistory.channel = 16);
        } else {
            gMemoryCardSlotHistory.activeIndex = isValue16(gMemoryCardSlotHistory.channel = 0);
        }
        setMemoryCardChannel(gMemoryCardSlotHistory.channel);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (gMemoryCardTaskState == 7)
            gMemoryCardSlotHistory.unk_16[gMemoryCardSlotHistory.activeIndex] = 0;
        switch (gMemoryCardTaskState) {
        case 0:
            if (gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] == 2) {
                gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] = 0;
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 4;
                return;
            }
            if (gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] == 4) {
                gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] = 0;
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 7;
                gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                    gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
                return;
            }
            break;
        case 2:
        case 3:
        case 4:
            gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
            gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
            break;
        case 5:
        case 6:
        case 11:
            gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
            gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
            if (gMemoryCardSlotHistory.unk_12[gMemoryCardSlotHistory.activeIndex] == 3) {
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 6;
                return;
            } else {
                gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 1;
                gCurrentTask->data18 = 0;
            }
            break;
        case 7:
        case 8:
        case 9:
        case 10:
        case 12:
            gMemoryCardSlotHistory.previous[gMemoryCardSlotHistory.activeIndex] =
                gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex];
            gMemoryCardSlotHistory.current[gMemoryCardSlotHistory.activeIndex] = gMemoryCardTaskState;
            gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 1;
            gCurrentTask->data18 = 0;
            break;
        }
        if ((s16)startMemoryCardSaveStatusTask(MEMORY_CARD_SAVE_BUFFER, 0x810) == 1)
            gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 0;
        break;
    }
}

void startStateValueHistoryTask(void) {
    s16 i;
    createChildTaskWithReserve(updateStateValueHistoryTask);
    gCreatedTask->data18 = 0;
    gMemoryCardSlotHistory.channel = 16;
    gMemoryCardSlotHistory.activeIndex = 1;
    for (i = 0; i < 2; i++) {
        gMemoryCardSlotHistory.current[i] = 0;
        gMemoryCardSlotHistory.previous[i] = 0;
        gMemoryCardSlotHistory.unk_0E[i] = 0;
        gMemoryCardSlotHistory.unk_12[i] = 0;
        gMemoryCardSlotHistory.unk_16[i] = 1;
    }
}

void initializeMemoryCardSaveMenuTask(void) {
    SaveMenuResource *menu;
    SaveRecord *destination;
    GameTask *task;

    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 0;
    gCurrentTaskParent->data1C++;

    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    menu = (SaveMenuResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    menu->command0C = 0;

    prepareSaveRecordForWrite(&gPlayerSaveRecords[0]);
    destination = MEMORY_CARD_SAVE_BUFFER;
    *destination++ = gPlayerSaveRecords[0];
    *destination = gPlayerSaveRecords[0];

    startStateValueHistoryTask();
    setCurrentTaskCallback(updateMemoryCardSaveMenuTask);
    task = gCurrentTask;
    task->unk10 |= 2;
}

void updateMemoryCardSaveMenuTask(void) {
    SaveMenuResource *primary;
    SaveMenuResource *menu;
    SaveMenuResource *input;
    s16 x;
    s16 y;
    s16 player;
    u8 texture;
    u16 enabled;
    /* Preserve the original task's 0x60-byte stack frame. */
    u8 padding[0x10];
    s16 status;
    s32 formatResult;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    menu = (SaveMenuResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    primary = menu;
    input = (SaveMenuResource *)&gTaskMemoryPool[gCurrentTask->resource16];
    player = gCurrentTask->data1A;
    texture = gRacePlayers[player].unk009;
    x = -76;
    y = -30;

    enabled = 1;
    switch (gCurrentTask->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        configureTaskStateTransition(1, 0, 30);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        primary->busy06 = 0;
        createRegisteredCallbackTask(0, 0, 30, drawMemoryCardSaveRecordPanel, 6, 1);
        uploadTextureArchiveEntry(player * 128 + 576, 256, 768, 238 - player * 2, texture);
        uploadTextureArchiveEntry(320, 256, 768, 234, 11);
        goto showTitle;
    case 2:
        createRegisteredCallbackTask(0, 0, 30, drawTaskShadowedText, 4, 11);
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if (input->busy06 || input->result0E)
            break;
        input->command04 = 22;
        input->command0C = 21;
        configureColorCycleTask(2, 64, 128, 4, 0);
        createInputTimedSpriteTask(-16, 54, gMenuInputPromptSequence, 2, 1, 176, 64, 4, 31);
        gCurrentTask->data18 = 5;
        break;
    case 5:
        if (input->result1E > 0)
            break;
        if (input->result1E < 0) {
            gCurrentTask->data18 = 39;
        } else {
            input->command0C = 99;
            gCurrentTask->data18 = 9;
            break;
        }
        break;
    case 6:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardSaveSlotChoiceText,
                                              -1, enabled, 1, gMemoryCardSlotChoiceSprites, 4, 11);
        gCurrentTask->data18 = 7;
        break;
    case 7:
        status = input->result0E;
        if (status == 0)
            break;
        if (status == 1) {
            gCurrentTask->data18 = 9;
            break;
        }
        if (status == 2) {
            menu->slot = 0;
            goto checkCard;
        }
        if (status == 3) {
            menu->slot = 1;
            goto checkCard;
        }
        goto selectSlot;
    case 8:
        switch (gMemoryCardSlotHistory.current[menu->slot]) {
        case 0:
            break;
        case 1:
        case 2:
            createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardAccessInProgressText, menu->slot, 4, 11);
            break;
        case 6:
            gMemoryCardSlotHistory.previous[menu->slot] = gMemoryCardSlotHistory.current[menu->slot];
            if (gMemoryCardSlotHistory.unk_16[menu->slot] == 1)
                goto showSaveConfirmation;
            gCurrentTask->data18 = 26;
            gMemoryCardSlotHistory.unk_16[menu->slot] = 1;
            break;
        case 11: {
            s16 slot = menu->slot;
            gMemoryCardSlotHistory.previous[slot] = gMemoryCardSlotHistory.current[slot];
            gCurrentTask->data18 = 16;
            break;
        }
        case 5:
        showSaveConfirmation:
            gCurrentTask->data18 = 14;
            break;
        case 7:
            gMemoryCardSlotHistory.previous[menu->slot] = gMemoryCardSlotHistory.current[menu->slot];
            gCurrentTask->data18 = 28;
            break;
        case 8:
        case 12:
            gCurrentTask->data18 = 24;
            createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardCorruptDataText, player, 4, 11);
            gMemoryCardSlotHistory.previous[menu->slot] = gMemoryCardSlotHistory.current[menu->slot];
            break;
        case 9:
            gMemoryCardSlotHistory.previous[menu->slot] = gMemoryCardSlotHistory.current[menu->slot];
            gCurrentTask->data18 = 32;
            break;
        case 10:
            gMemoryCardSlotHistory.previous[menu->slot] = gMemoryCardSlotHistory.current[menu->slot];
            gCurrentTask->data18 = 30;
            break;
        }
        break;
    case 9:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardSaveOptionsText,
                                              -1, (u16)menu->command0C | enabled, 1, gSaveRecordConfirmationSprites, 4, 11);
        gCurrentTask->data18 = 10;
        break;
    case 10:
        menu->command0C = 0;
        status = input->result0E;
        if (status == 0)
            break;
        if (status == 1)
            goto showTitle;
        if (status == 2)
            goto selectSlot;
        if (status == 3)
            goto showRetry;
        gCurrentTask->data18 = 9;
        break;
    showTitle:
        gCurrentTask->data18 = 2;
        break;
    case 14:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardSaveConfirmationText,
                                              -1, enabled, 1, gSaveRecordConfirmationSprites, 4, 11);
        gCurrentTask->data18 = 15;
        break;
    case 15:
        if (gMemoryCardSlotHistory.current[menu->slot] == 7)
            goto checkCard;
        status = input->result0E;
        if (status == 0)
            break;
        if (status == 1)
            goto showRetry;
        if (status == 2)
            goto startSave;
        goto showRetry;
    case 16:
        gMemoryCardSlotHistory.unk_12[menu->slot] = 3;
        createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardSaveInProgressText, -1, 4, 11);
        gCurrentTask->data18 = 17;
        break;
    case 17:
        if (gMemoryCardSlotHistory.current[menu->slot] == 7)
            goto showSaveFailure;
        if (gMemoryCardSlotHistory.unk_0E[menu->slot] == 6)
            gCurrentTask->data18 = 18;
        break;
    case 18:
        gMemoryCardTaskState = 5;
        gMemoryCardSlotHistory.unk_12[menu->slot] = 4;
        startMemoryCardWriteTask();
        gCurrentTask->data18 = 19;
        break;
    case 19:
        status = gMemoryCardSlotHistory.current[menu->slot];
        if (status == 5)
            break;
        if (status == 4)
            break;
        if (status == 0 || status == 5) {
            createRegisteredStaticTextPanelTask(x, y, x + 20, y + 12, gMemoryCardSaveCompleteText, -1, 4, 11);
            gCurrentTask->data18 = (gSaveRecordSavedOrLoaded = 1, 22);
            break;
        }
        gMemoryCardSlotHistory.unk_0E[gMemoryCardSlotHistory.activeIndex] = 7;
    showSaveFailure:
        gCurrentTask->data18 = 20;
        break;
    case 20:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardOperationFailedText,
                                              -1, enabled, 1, 0, 4, 11);
        gCurrentTask->data18 = 21;
        break;
    case 22:
        gCurrentTaskParent->data18 = 2;
        fadeOutMusicSequence(512);
        gCurrentTask->data18 = 40;
        break;
    case 24:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardSaveMenuExitConfirmationText,
                                              -1, enabled, 1, gSaveRecordConfirmationSprites, 4, 11);
        gCurrentTask->data18 = 25;
        break;
    case 25:
        status = input->result0E;
        if (status == 0)
            break;
        if (status == 1) {
            gCurrentTask->data18 = 9;
            break;
        }
        if (status != 2) {
            gCurrentTask->data18 = 9;
            break;
        }
        gCurrentTask->data18 = 22;
        break;
    case 26:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardChangedSaveConfirmationText,
                                              -1, enabled, 1, gSaveRecordConfirmationSprites, 4, 11);
        gCurrentTask->data18 = 27;
        break;
    case 27:
        if (gMemoryCardSlotHistory.current[menu->slot] == 7)
            goto checkCard;
        status = input->result0E;
        if (status == 0)
            break;
        if (status == 1)
            goto selectSlot;
        if (status == 2)
            goto startSave;
    selectSlot:
        gCurrentTask->data18 = 6;
        break;
    case 28:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardMissingText,
                                              menu->slot, enabled, 1, 0, 4, 11);
        gCurrentTask->data18 = 29;
        break;
    case 29:
        if (hasStateValueChanged(menu->slot)) {
            gCurrentTask->data18 = 9;
        } else if (input->result0E) {
            menu->command0C = 0x100;
            gCurrentTask->data18 = 24;
        }
        break;
    case 30:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardFullText,
                                              menu->slot, enabled, 1, 0, 4, 11);
        gCurrentTask->data18 = 31;
        break;
    case 31:
        if (hasStateValueChanged(menu->slot))
            goto checkCard;
        goto waitForPanel;
    case 32:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardUnformattedText,
                                              menu->slot, enabled, 1, 0, 4, 11);
        gCurrentTask->data18 = 33;
        break;
    case 33:
        if (gMemoryCardSlotHistory.current[menu->slot] == 7)
            goto checkCard;
        if (!input->result0E)
            break;
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardFormatConfirmationText,
                                              menu->slot, enabled | 0x100, 1, gSaveRecordConfirmationSprites, 4, 11);
        gCurrentTask->data18 = 34;
        break;
    case 34:
        if (gMemoryCardSlotHistory.current[menu->slot] == 7) {
    checkCard:
            gCurrentTask->data18 = 8;
            break;
        }
        status = input->result0E;
        if (status == 0)
            break;
        if (status == 1)
            goto showRetry;
        if (status != 2)
            goto showRetry;
        gCurrentTask->data18 = 35;
        break;
    case 35:
        createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardAccessInProgressText, menu->slot, 4, 11);
        gMemoryCardSlotHistory.unk_12[menu->slot] = 2;
        gCurrentTask->data18 = 36;
        break;
    case 36:
        if (gMemoryCardSlotHistory.current[menu->slot] == 7)
            goto showFormatFailure;
        if (gMemoryCardSlotHistory.unk_0E[menu->slot] != 4)
            break;
        resetMemoryCardTaskState();
        if (menu->slot == 0)
            setMemoryCardChannel(0);
        else
            setMemoryCardChannel(16);
        formatResult = formatMemoryCard();
        gMemoryCardSlotHistory.unk_0E[menu->slot] = 5;
        if (formatResult == 0) {
            createRegisteredStaticTextPanelTask(x, y, x + 4, y + 2, gMemoryCardFormatCompleteText, -1, 4, 11);
            gMemoryCardSlotHistory.unk_12[menu->slot] = 3;
    startSave:
            gCurrentTask->data18 = 16;
            break;
        }
        if (formatResult < 0)
            break;
        if (formatResult < 3) {
    showFormatFailure:
            gCurrentTask->data18 = 37;
        }
        break;
    case 37:
        createRegisteredDetailedTextPanelTask(x, y, x + 4, y + 2, gMemoryCardFormatFailedText,
                                              -1, enabled, 1, 0, 4, 11);
        gCurrentTask->data18 = 38;
        break;
    case 21:
    case 38:
    waitForPanel:
        if (!input->result0E)
            break;
    showRetry:
        gCurrentTask->data18 = 24;
        break;
    case 39:
        gCurrentTaskParent->data18 = 6;
        /* Fall through. */
    case 40:
        queueMenuResourceReads(gCurrentTaskParent->data18);
        configureTaskStateTransition(2, 0, 30);
        gCurrentTask->data18 = 41;
        break;
    case 41:
        if (isTaskStateTransitionBelowMaximum(0) == 0)
            gCurrentTask->data18 = 42;
        break;
    case 42:
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawMemoryCardSaveRecordPanel(void) {
    Movement2D *position;
    s16 player;
    s16 character;
    s16 i;
    s16 pair;
    SaveRecord *records;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    player = gCurrentTaskParent->data1A;
    character = gRacePlayers[player].unk009;
    setSpriteColor(128, 128, 128);
    drawWrappedGlyphText16x16((s16)(position->x / 4096 - 68), (s16)(position->y / 4096 - 96),
                              9, gMemoryCardSaveRecordHeading, 14, -1, PANEL_OT, PANEL_DEPTH);
    drawDecimalNumber8x16(position->x / 4096 + 38, position->y / 4096 - 3, 6, 0,
                          gPlayerSaveRecords[player].money,
                          0, -1, PANEL_OT, PANEL_DEPTH);
    drawWrappedDoubleByteText(position->x / 4096 + 86, position->y / 4096 - 3,
                              2, gMemoryCardSaveRecordCurrencyLabel, 0, -1, PANEL_OT, PANEL_DEPTH);
    addTilePacketWithBlend(position->x / 4096 + 34, position->y / 4096 - 3,
                  64, 16, 128, 128, 128, 2, PANEL_OT, PANEL_DEPTH);
    drawSpriteByIndexWithBlend(position->x / 4096 - 99, position->y / 4096 - 78, 110, 6, -1,
                  PANEL_OT, PANEL_DEPTH);
    drawSpriteByIndexWithBlend(position->x / 4096 - 101, position->y / 4096 - 61, 91, 6, -1,
                  PANEL_OT, PANEL_DEPTH);
    drawSpriteByIndexWithBlend(position->x / 4096 - 61, position->y / 4096 - 61, 89, 6, -1,
                  PANEL_OT, PANEL_DEPTH);
    drawSpriteByIndexWithBlend((s8)gSelectionPanelLabelOffsets[character] + position->x / 4096 - 71,
                  position->y / 4096 - 77, character + 254, (s16)(player + 2), -1,
                  PANEL_OT, PANEL_DEPTH);

    for (i = 0; i < 3; i++) {
        records = gPlayerSaveRecords;
        if (records[player].masks[character] & (0x1000000U << i)) {
            setSpriteColor(128, 128, 128);
        } else {
            setSpriteColor(32, 32, 32);
        }
        {
            s32 x = (s16)(position->x / 4096 - 96);
            s32 rowOffset = i * 22;
            s32 y = (s16)(position->y / 4096 + rowOffset - 50);
            drawSpriteByIndexWithBlend(x, y, i + 18, 6, -1, PANEL_OT, PANEL_DEPTH);
        }
    }

    for (i = 0; i < 9; i++) {
        setSpriteColor(128, 128, 128);
        pair = getSaveRecordFlagPair(player, character, i);
        if (pair == 3) {
            s32 columnOffset = (s16)(i % 3) * 16;
            s32 x = (s16)(position->x / 4096 + columnOffset - 60);
            s32 rowOffset = (s16)(i / 3) * 22;
            s32 y = (s16)(position->y / 4096 + rowOffset - 49);
            drawSpriteByIndexWithBlend(x, y, i + 70, 9, -1, PANEL_OT, PANEL_DEPTH);
        } else if (pair == 2) {
            s32 columnOffset = (s16)(i % 3) * 16;
            s32 x = (s16)(position->x / 4096 + columnOffset - 60);
            s32 rowOffset = (s16)(i / 3) * 22;
            s32 y = (s16)(position->y / 4096 + rowOffset - 49);
            drawSpriteByIndexWithBlend(x, y, i + 70, 10, -1, PANEL_OT, PANEL_DEPTH);
        } else if (pair == 1) {
            s32 columnOffset = (s16)(i % 3) * 16;
            s32 x = (s16)(position->x / 4096 + columnOffset - 60);
            s32 rowOffset = (s16)(i / 3) * 22;
            s32 y = (s16)(position->y / 4096 + rowOffset - 49);
            drawSpriteByIndexWithBlend(x, y, i + 70, 11, -1, PANEL_OT, PANEL_DEPTH);
        } else {
            s32 columnOffset, rowOffset, x, y;

            setSpriteColor(32, 32, 32);
            columnOffset = (s16)(i % 3) * 16;
            x = (s16)(position->x / 4096 + columnOffset - 60);
            rowOffset = (s16)(i / 3) * 22;
            y = (s16)(position->y / 4096 + rowOffset - 49);
            drawSpriteByIndexWithBlend(x, y, 69, 9, -1, PANEL_OT, PANEL_DEPTH);
        }
    }

    setTexturedQuadColor(128, 128, 128);
    drawTexturedQuadList(position->x / 4096 - 104, position->y / 4096 - 80,
                         gPlayerPanelQuadLists[player], 1, PANEL_OT, PANEL_DEPTH);
    setSpriteColor(128, 128, 128);
    drawSpriteByIndexWithBlend(position->x / 4096 - 8, position->y / 4096 - 80,
                  player * 2 + 241, 0, 0, PANEL_OT, PANEL_DEPTH);
    drawSpriteByIndexWithBlend(position->x / 4096 - 8, position->y / 4096 - 80,
                  267, 0, -1, PANEL_OT, PANEL_DEPTH);
}

s8 gMemoryCardSaveRecordCurrencyLabel[4]
    __attribute__((section(".data.memory_card_save_record_currency_label"), aligned(4))) = {
    (s8)0x82, 'f', 0, 0
};

void drawTaskShadowedText(void) {
    Movement2D *resource;
    s32 x, y;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = resource->x / 4096;
    y = resource->y / 4096;
    drawWrappedGlyphText8x16((s16)(x - 48), (s16)(y + 24), 12, gTaskShadowedText, 0, -1,
                             &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    setSpriteColor(0, 0, 0);
    drawWrappedGlyphText8x16((s16)(x - 47), (s16)(y + 25), 12, gTaskShadowedText, 0, -1,
                             &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawWrappedGlyphText8x16((s16)(x - 46), (s16)(y + 25), 12, gTaskShadowedText, 0, -1,
                             &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

s16 getSaveRecordMaskTier(s16 recordIndex, s16 maskIndex) {
    s16 result = 0;
    s16 i = 3;
    SaveRecord *records;

    for (; i >= 0; i--) {
        records = gPlayerSaveRecords;
        if (records[recordIndex].masks[maskIndex] & (0x1000000 << i)) {
            result = i + 1;
            break;
        }
    }
    return result;
}

void setSaveRecordMaskTier(s16 recordIndex, s16 maskIndex, s16 tier) {
    s16 i;
    SaveRecord *records = gPlayerSaveRecords;

    records[recordIndex].masks[maskIndex] &= 0xF0FFFFFF;
    for (i = 3; i >= 0; i--) {
        if (i < tier) {
            records[recordIndex].masks[maskIndex] |= 0x1000000 << i;
        }
    }
    rebuildCombinedStateMask(&gPlayerSaveRecords[recordIndex]);
    copySaveRecordAndMergeFlags();
}

s16 getStateMaskTier(s16 stateIndex, s16 maskIndex) {
    s16 result = 0;
    s16 i;
    u32 mask;

    mask = gWorkingSaveRecords[stateIndex]->masks[maskIndex];
    for (i = 3; i >= 0; i--) {
        if (mask & (0x1000000 << i)) {
            result = i + 1;
            break;
        }
    }
    return result;
}

s16 getSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex) {
    u32 first;
    u32 second;
    s16 result;
    SaveRecord *records = gPlayerSaveRecords;

    first = records[recordIndex].masks[maskIndex] & (1U << (pairIndex * 2));
    second = records[recordIndex].masks[maskIndex] & (2U << (pairIndex * 2));
    if (first && second) {
        result = 3;
    } else if (second) {
        result = 2;
    } else if (first) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

void setSaveRecordFlagPair(s16 recordIndex, s16 maskIndex, s16 pairIndex, s16 value) {
    SaveRecord *state = &gPlayerSaveRecords[0];

    state[recordIndex].masks[maskIndex] &= ~(3 << (pairIndex * 2));
    if (value == 1 || value == 3) {
        gPlayerSaveRecords[recordIndex].masks[maskIndex] |= 1 << (pairIndex * 2);
    }
    if (value == 2 || value == 3) {
        SaveRecord *secondState = &gPlayerSaveRecords[0];
        secondState[recordIndex].masks[maskIndex] |= 2 << (pairIndex * 2);
    }
    rebuildCombinedStateMask(&gPlayerSaveRecords[recordIndex]);
    copySaveRecordAndMergeFlags();
}

s32 getStateMaskPair(s16 stateIndex, s16 maskIndex, s16 pairIndex) {
    u32 mask = gWorkingSaveRecords[stateIndex]->masks[maskIndex];
    u32 low = mask & (1 << (pairIndex * 2));
    u32 high = mask & (2 << (pairIndex * 2));
    return low && high ? 3 : high ? 2 : low ? 1 : 0;
}

void rebuildCombinedStateMask(SaveRecord *state) {
    s16 i;

    state->masks[SAVE_RECORD_COMBINED_MASK_INDEX] = 0;
    for (i = 0; i < 10; i++) {
        state->masks[SAVE_RECORD_COMBINED_MASK_INDEX] |= i[state->masks];
    }
}

s16 updateRaceUnlockProgress(s16 characterId, s16 finishPosition) {
    s16 savedCharacterId = characterId;
    s16 unlockTier;
    s16 coursePair;
    s16 finishRating;
    s16 i;
    s16 recordIndex = 0;

    if (finishPosition >= 4) {
        return 0;
    }
    unlockTier = 0;
    coursePair = gCourseMenuIndicesByCourse[gSelectedCourseIndex];
    finishRating = 3 - finishPosition;
    if (getSaveRecordFlagPair(recordIndex, characterId, coursePair) < finishRating) {
        setSaveRecordFlagPair(recordIndex, characterId, coursePair, finishRating);
        switch (getSaveRecordMaskTier(recordIndex, characterId)) {
        case 2:
            if (finishRating == 3 && coursePair == 7) {
                unlockTier = 3;
            }
            break;
        case 1:
            if (finishRating == 3 && coursePair == 6) {
                unlockTier = 2;
            }
            break;
        case 0:
            unlockTier = 1;
            for (i = 0; i < 6; i++) {
                if (getSaveRecordFlagPair(recordIndex, characterId, i) < 3) {
                    unlockTier = 0;
                    break;
                }
            }
            break;
        }
    }
    if (finishRating == 3 && coursePair == 8) {
        unlockTier = 4;
    }
    if (unlockTier != 0) {
        setSaveRecordMaskTier(recordIndex, savedCharacterId, unlockTier);
    }
    copySaveRecordAndMergeFlags();
    return unlockTier;
}

u32 getGlobalFlagMask(s16 bitIndex) {
    return gMergedSaveRecord.flags & (1U << bitIndex);
}

void updatePairedStateFlag(s16 index) {
    if (index < 0) {
        gPlayerSaveRecords[0].flags &= ~3U;
        gMergedSaveRecord.flags &= ~3U;
    } else {
        gPlayerSaveRecords[0].flags |= 1U << index;
        gMergedSaveRecord.flags |= 1U << index;
    }
}

void setPairedStateIndexedFlag(s16 index) {
    SaveRecord *first = &gPlayerSaveRecords[0];
    u32 mask = 0x800U << (index + 3);
    SaveRecord *second = &gMergedSaveRecord;

    first->flags |= mask;
    second->flags |= mask;
}

s32 getModeFilteredGameOptionFlag2(void) {
    if (gModeSequenceStage != 0) {
        return (u32)(gModeSequenceStage - 7) < 2;
    }
    if (gControllerPlaybackIndex != 0) {
        return 0;
    }
    if (playerCountAbsolute < 2) {
        return 1;
    }
    return gGameOptionFlags & 4;
}

s32 getStateAdjustedGameOptionFlag1(void) {
    if (gModeSequenceStage != 0) {
        return (u32)(gModeSequenceStage - 7) < 2;
    }
    if (gControllerPlaybackIndex != 0) {
        return 0;
    }
    if (playerCountAbsolute < 2) {
        return 1;
    }
    return gGameOptionFlags & 2;
}
