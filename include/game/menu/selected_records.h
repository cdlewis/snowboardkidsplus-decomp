#pragma once

#include "common.h"

/* Each selected-course record occupies 0x48 bytes. Unknown fields retain
 * offset names until their consumers establish their meaning. */
typedef struct SelectedCourseRecord {
    u16 unk00;
    u16 unk02;
    s16 unk04;
    u16 unk06;
    s32 unk08, unk0C, unk10;
    u16 unk14;
    u16 unk16;
    u32 unk18[8];
    s16 objectFirstSegment;
    s16 objectLastSegment;
    s16 unk3C;
    u16 unk3E;
    u32 unk40[2];
} SelectedCourseRecord;

extern SelectedCourseRecord D_80123D80[];
extern const u8 D_8012D46C[];
extern u8 D_8012D478[];
extern s16 D_8013AFCE;
extern u16 D_8013AFD0;
extern u8 D_8013AFDA;
extern s32 D_8012FC6C[10][3];

extern u8 D_8013AFDB;
extern u8 D_8013AFD6;

u32 getSelectedCoursePrize(s16 unusedCourse, s16 place);
void resetSelectedRecordStates(void);
s32 updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex);

extern s16 selectedFinishLapCountAbsolute __attribute__((section(".data")));
extern u8 raceFinishFlagAbsolute __attribute__((section(".data")));

/* Save player 0 at selected course segments and split its recorded input run. */
void captureSelectedPlayerStates(void);
/* Camera-related state saved alongside selected player snapshots. */
extern u32 D_8013B20C __attribute__((section(".data")));
/* GP-addressed view used when queuing the saved camera overlay. */
extern u32 selectedCameraOverlayModeGp;

s32 restoreSelectedPlayerState(void);
s32 hasCapturedSelectedPlayerState(void);

/* GP-relative access to the same saved camera state. */
