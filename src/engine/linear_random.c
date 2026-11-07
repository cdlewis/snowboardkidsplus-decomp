#include "game/engine/linear_random.h"

s32 randomNextLinear(void) {
    gLinearRandomState = gLinearRandomState * 1103515245 + 12345;
    return ((u32)gLinearRandomState >> 16) & 0x7FFF;
}

s32 randomLinearFromValue(s32 value) {
    value = value * 1103515245 + 12345;
    return (value / 65536) & 0x7FFF;
}
