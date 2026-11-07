#pragma once

#include "decomp/types.h"

/* Explicit .data placement preserves absolute accesses in this module;
 * some other game units access these same objects through $gp.
 */
extern u32 gGameOptionFlags __attribute__((section(".data")));
extern u16 gUnlockNotificationFlags __attribute__((section(".data")));

/* These getters return raw masks, not normalized booleans. Bit 3 selects stereo
 * audio in applyStereoSoundOption. The purpose of option bit 0 is still unidentified.
 * The per-player option occupies bit (4 + playerIndex).
 */
#ifdef GAME_OPTIONS_GP_ADDRESSING
extern u8 D_8013AFCD;
extern u8 D_8013AFE0;
extern u8 D_8013AFE1;
extern u8 D_8013AFE2;
#else
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013AFE0 __attribute__((section(".data")));
extern u8 D_8013AFE1 __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));
#endif
/* GP-relative alias of D_8013AFCD used by race restart task code. */
extern u8 gameOptionPlayerModeGp;
#ifdef GAME_OPTION_TRANSITION_ABSOLUTE_ADDRESSING
extern u8 D_8013B108 __attribute__((section(".data")));
extern u8 D_8013B109 __attribute__((section(".data")));
#else
extern u8 D_8013B108;
extern u8 D_8013B109;
#endif
extern u8 D_8013B10A;

void initializeGameOptionTask(void);
void resetGameOptionState(void);
s32 getStereoSoundFlag(void);
s32 getGameOptionFlag0(void);
s32 getModeFilteredGameOptionFlag2(void);
/* State overrides return 0 or 1; otherwise returns the raw option bit 1 mask. */
s32 getStateAdjustedGameOptionFlag1(void);
s32 getPlayerOptionFlag(s16 playerIndex);

/* The unlock-message launcher (startUnlockNotification) tests and sets this mask to
 * avoid repeating notifications. The boolean wrapper aliases index 11 to 10.
 */
s32 getUnlockNotificationFlag(s16 index);
void setUnlockNotificationFlag(s16 index);
s32 hasUnlockNotificationFlag(s16 index);
void startUnlockNotification(s16 index);

extern u8 gameOptionPlayerModeAbsolute __attribute__((section(".data")));
