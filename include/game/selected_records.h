#pragma once

#include "common.h"

/* Each selected-course record occupies 0x48 bytes. Unknown fields retain
 * offset names until their consumers establish their meaning. */
typedef struct SelectedCourseRecord {
    u16 unk00;
    u16 unk02;
    u32 unk04[17];
} SelectedCourseRecord;

extern SelectedCourseRecord D_80123D80[];
#ifdef SELECTED_RECORDS_GP_ADDRESSING
extern s16 D_8013AFCE;
#else
extern s16 D_8013AFCE __attribute__((section(".data")));
#endif
extern u16 D_8013AFD0;
extern u8 D_8013AFDA;

void resetSelectedRecordStates(void);
void updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex);
