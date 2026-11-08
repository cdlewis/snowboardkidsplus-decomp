#pragma once

#include "decomp/types.h"

typedef struct ControllerEffectStep {
    u8 value;
    s8 delta;
    u8 pad[2];
    s32 duration;
} ControllerEffectStep;
typedef struct ControllerEffectState {
    u8 effect;
    u8 priority;
    u8 pad[2];
    ControllerEffectStep *step;
    s32 elapsed;
    s32 stepEnd;
    s32 effectEnd;
} ControllerEffectState;

extern ControllerEffectState gControllerEffectStates[2];

typedef struct ControllerActuatorOutput {
    u8 control;
    u8 value;
    u8 aligned;
} ControllerActuatorOutput;

/* Shared controller storage remains in the main controller data object. */
extern ControllerActuatorOutput D_8013AF00[];
extern s16 D_8013AEF0[];
extern u8 D_8013AFD3 __attribute__((section(".data")));

void updateControllerEffectSequence(u16 controller);
void updateLegacyControllerActuators(u16 controller);

/* Pending effect IDs remain in the shared controller data object. */
#ifdef CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
extern s16 D_8013AF08[2] __attribute__((section(".data")));
#else
extern s16 D_8013AF08[];
#endif

void resetControllerEffects(void);
