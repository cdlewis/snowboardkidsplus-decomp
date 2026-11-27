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
extern u8 gPlayerCount;
extern u8 gRaceRuleset;
/* One-based ruleset menu choice; zero starts at the first entry. */
extern u8 gRaceRulesetMenuRememberedSelection
    __attribute__((section(".data.main_tail_money_currency_label")));
extern u8 gRaceResultCode;
extern u8 gMainMenuSelectionIndex;
extern u8 gTitleMenuResourcesInitialized;
extern u8 gMenuScreenId;
extern u8 gMenuTransitionState;
/* Resource request caches; -1 forces the next menu resource read. */
extern s16 gMenuTextureResourceId;
extern s16 gMenuGridResourceId;
extern u8 gTitleAttractSequenceIndex;
extern u8 gTitleMenuSelectionIndex;
/* Nonzero releases the mode's waiting playback and camera tasks. */
extern u8 gModeTaskTransitionState;
/* GP-relative alias of gModeTaskTransitionState. */
extern u8 modeTaskTransitionStateGp;
/* Course selected before the next mode race; 99 forces a resource reload. */
extern s16 gPreviousSelectedCourseIndex;
extern s16 previousSelectedCourseIndexAbsolute __attribute__((section(".data")));
/* Prevents restarting XA command 3 while moving between selection menus. */
extern u8 gSelectionMenuMusicStarted;

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

extern u8 playerCountAbsolute __attribute__((section(".data")));

/* XA playback command selected by course when the race begins. */
extern s16 gCourseRaceMusicXaCommands[10];
extern s16 gRaceHudEnabled __attribute__((section(".data")));

/* Reuse resident course and player model resources during mode transitions. */
extern u8 gReuseCourseResources;
/* Absolute-addressed view used by startup and playback cleanup. */
extern u8 reuseCourseResourcesAbsolute __attribute__((section(".data")));
