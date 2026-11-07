#include "common.h"
#include "game/engine/game_options.h"

u32 gGameOptionFlags = 0x3E;
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

INCLUDE_ASM("asm/nonmatchings/engine/game_options", func_8008C670);

s32 getUnlockNotificationFlag(s16 index) {
    return gUnlockNotificationFlags & (1 << index);
}

void setUnlockNotificationFlag(s16 index) {
    gUnlockNotificationFlags |= 1 << index;
}
