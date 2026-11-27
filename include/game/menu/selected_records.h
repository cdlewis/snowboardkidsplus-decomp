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

extern SelectedCourseRecord gSelectedCourseRecords[10];
extern const u8 gCourseMenuCourseIndices[];
/* Inverse of gCourseMenuCourseIndices, also used for saved course flag pairs. */
extern const u8 gCourseMenuIndicesByCourse[];
extern s16 gSelectedCourseIndex;
extern u16 gSelectedLapCount;
extern u16 selectedLapCountAbsolute __attribute__((section(".data")));
extern u8 D_8013AFDA;
extern s32 gCoursePlacementPrizes[10][3];

extern u8 gNextRaceFinishSlot;
/* Playback of the captured player run, with saved-state restoration. */
extern u8 gSelectedPlayerReplayActive;

u32 getSelectedCoursePrize(s16 unusedCourse, s16 place);
void resetSelectedRecordStates(void);
s32 updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex);

/* Race finish comparisons and HUD draws use signed halfword loads. */
extern s16 selectedFinishLapCountAbsolute __attribute__((section(".data")));
/* Absolute-addressed alias of gSelectedPlayerReplayActive. */
extern u8 selectedPlayerReplayActiveAbsolute __attribute__((section(".data")));

/* Save player 0 at selected course segments and split its recorded input run. */
void captureSelectedPlayerStates(void);
/* Camera-related state saved alongside selected player snapshots. */
extern u32 gSelectedCameraOverlayMode __attribute__((section(".data")));
/* GP-addressed view used when queuing the saved camera overlay. */
extern u32 selectedCameraOverlayModeGp;

s32 restoreSelectedPlayerState(void);
s32 hasCapturedSelectedPlayerState(void);

/* GP-relative access to the same saved camera state. */
