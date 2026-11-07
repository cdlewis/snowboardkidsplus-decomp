#include "../graphics/render_effects_internal.h"

void resetControllerEffects(void) {
    s32 i;
    for (i = 0; i < 2; i++) {
        D_8013AF08[i] = -1;
        gControllerEffectStates[i].effect = 0;
        gControllerEffectStates[i].priority = 0;
        gControllerEffectStates[i].step = 0;
        gControllerEffectStates[i].elapsed = 0;
        gControllerEffectStates[i].stepEnd = 0;
        gControllerEffectStates[i].effectEnd = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/io/controller_effects", func_8006BA4C);

void updateLegacyControllerActuators(u16 controller) {
    if (D_8013AFD3) {
        D_8013AF00[controller].control = 0x40;
        D_8013AF00[controller].value = 0;
    } else if (D_8013AEF0[controller] >= 25) {
        D_8013AF00[controller].control = 0x40;
        D_8013AF00[controller].value = 1;
    } else {
        D_8013AF00[controller].control = 0x40;
        D_8013AF00[controller].value = 0;
    }
}
