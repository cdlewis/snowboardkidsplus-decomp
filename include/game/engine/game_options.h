#pragma once

#include "decomp/types.h"

void initializeRacePaletteCycles(void);

/* Explicit .data placement preserves absolute accesses in this module;
 * some other game units access these same objects through $gp.
 */
extern u16 gUnlockNotificationFlags __attribute__((section(".data")));

/* These getters return raw masks, not normalized booleans. Bit 3 selects stereo
 * audio in applyStereoSoundOption. The purpose of option bit 0 is still unidentified.
 * The per-player option occupies bit (4 + playerIndex).
 */
extern u32 gGameOptionFlags;
extern u8 D_8013AFCD;
extern u8 D_8013AFE0;
extern u8 D_8013AFE1;
extern u8 D_8013AFE2;
extern u8 D_8013B108;
extern u8 D_8013B109;
extern u8 D_8013B10A;
extern u8 D_8013B564;

void initializeGameOptionTask(void);
void resetGameOptionState(void);
s32 getStereoSoundFlag(void);
s32 getGameOptionFlag0(void);
s32 getModeFilteredGameOptionFlag2(void);
/* State overrides return 0 or 1; otherwise returns the raw option bit 1 mask. */
s32 getStateAdjustedGameOptionFlag1(void);
s32 getPlayerOptionFlag(s16 playerIndex);
s32 getModeFilteredAutoPilotFlag(s16 playerIndex);

/* The unlock-message launcher (startUnlockNotification) tests and sets this mask to
 * avoid repeating notifications. The boolean wrapper aliases index 11 to 10.
 */
s32 getUnlockNotificationFlag(s16 index);
void setUnlockNotificationFlag(s16 index);
s32 hasUnlockNotificationFlag(s16 index);
void startUnlockNotification(s16 index);

extern u8 gameOptionPlayerModeAbsolute __attribute__((section(".data")));

extern s16 D_80124164[];
extern s16 D_8013B1A8 __attribute__((section(".data")));
