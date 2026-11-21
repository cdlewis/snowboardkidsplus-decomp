#include "common.h"
#include "game/engine/game_options.h"

u32 gGameOptionFlags __attribute__((section(".data"))) = 0x3E;
u16 gUnlockNotificationFlags = 0;
static u16 sUnlockNotificationPadding __attribute__((section(".data"))) = 0;

s32 getStereoSoundFlag(void) {
    return gGameOptionFlags & 8;
}

s32 getGameOptionFlag0(void) {
    return gGameOptionFlags & 1;
}

s32 getPlayerOptionFlag(s16 playerIndex) {
    return gGameOptionFlags & (16 << playerIndex);
}

/* DECOMP_ATTEMPT: inline 16/19 */
#ifdef NON_MATCHING
extern u8 D_8013AFD8 __attribute__((section(".data")));

s16 func_8008C670(s16 playerIndex) {
    if (D_8013AFD8) goto zero;
    if (D_8013AFE2) goto zero;
    return gGameOptionFlags & (16 << playerIndex);
zero:
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/engine/game_options", func_8008C670);
#endif

s32 getUnlockNotificationFlag(s16 index) {
    return gUnlockNotificationFlags & (1 << index);
}

void setUnlockNotificationFlag(s16 index) {
    gUnlockNotificationFlags |= 1 << index;
}
