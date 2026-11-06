#pragma once

#include "common.h"

/* Each selected-course record occupies 0x48 bytes. Unknown fields retain
 * offset names until their consumers establish their meaning. */
typedef struct SelectedCourseRecord {
    u16 unk00;
    u16 unk02;
    u32 unk04[4];
    u16 unk14;
    u16 unk16;
    u32 unk18[12];
} SelectedCourseRecord;

extern SelectedCourseRecord D_80123D80[];
#ifdef SELECTED_RECORDS_GP_ADDRESSING
extern s16 D_8013AFCE;
#else
extern s16 D_8013AFCE __attribute__((section(".data")));
#endif
#ifdef SELECTED_RECORDS_ABSOLUTE_ADDRESSING
extern u16 D_8013AFD0 __attribute__((section(".data")));
#else
extern u16 D_8013AFD0;
#endif
extern u8 D_8013AFDA;
extern s32 D_8012FC6C[10][3];

extern u8 D_8013AFDB __attribute__((section(".data")));
#ifdef RACE_FINISH_ABSOLUTE_ADDRESSING
extern u8 D_8013AFD6 __attribute__((section(".data")));
#else
extern u8 D_8013AFD6;
#endif

void resetSelectedRecordStates(void);
void updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex);
