#pragma once

#include "decomp/types.h"

/* Explicit .data placement preserves absolute accesses in this module;
 * some other game units access these same objects through $gp.
 */
extern u32 gGameOptionFlags __attribute__((section(".data")));
extern u16 gUnlockNotificationFlags __attribute__((section(".data")));

/* These getters return raw masks, not normalized booleans. Bit 3 selects stereo
 * audio in func_80017BAC. The purpose of option bit 0 is still unidentified.
 * The per-player option occupies bit (4 + playerIndex).
 */
s32 getStereoSoundFlag(void);
s32 getGameOptionFlag0(void);
s32 getPlayerOptionFlag(s16 playerIndex);

/* The unlock-message launcher (func_800A7CE8) tests and sets this mask to
 * avoid repeating notifications. The boolean wrapper aliases index 11 to 10.
 */
s32 getUnlockNotificationFlag(s16 index);
void setUnlockNotificationFlag(s16 index);
s32 hasUnlockNotificationFlag(s16 index);
