#pragma once

#include "decomp/types.h"

typedef struct ControllerActuatorOutput {
    u8 control;
    u8 value;
    u8 aligned;
} ControllerActuatorOutput;

/* Shared controller storage remains in the main controller data object. */
extern ControllerActuatorOutput D_8013AF00[];
extern s16 D_8013AEF0[];
extern u8 D_8013AFD3 __attribute__((section(".data")));

void updateLegacyControllerActuators(u16 controller);

/* Pending effect IDs remain in the shared controller data object. */
#ifdef CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
extern s16 D_8013AF08[2] __attribute__((section(".data")));
#else
extern s16 D_8013AF08[];
#endif

void resetControllerEffects(void);
