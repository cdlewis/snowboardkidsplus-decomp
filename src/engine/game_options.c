#include "common.h"
#include "game/engine/game_options.h"

u32 gGameOptionFlags __attribute__((section(".data"))) = 0x3E;
u16 gUnlockNotificationFlags = 0;
static u16 sUnlockNotificationPadding __attribute__((section(".data"))) = 0;

extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));

s32 getStereoSoundFlag(void) {
    return gGameOptionFlags & 8;
}

s32 getGameOptionFlag0(void) {
    return gGameOptionFlags & 1;
}

s32 getPlayerOptionFlag(s16 playerIndex) {
    return gGameOptionFlags & (16 << playerIndex);
}

s32 getModeFilteredAutoPilotFlag(s16 playerIndex) {
    s32 result = 0;

    if (D_8013AFD8 || D_8013AFE2) {
        return result;
    }
    result = gGameOptionFlags & (16 << playerIndex);
    return result;
}

s32 getUnlockNotificationFlag(s16 index) {
    return gUnlockNotificationFlags & (1 << index);
}

void setUnlockNotificationFlag(s16 index) {
    gUnlockNotificationFlags |= 1 << index;
}
