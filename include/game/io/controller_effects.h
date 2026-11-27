#pragma once

#include "decomp/types.h"
#include "game/io/controller.h"

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
extern ControllerActuatorOutput gControllerActuatorOutputs[];
/* Legacy actuator output is enabled when the controller input is at least 25. */
extern s16 gControllerLegacyActuatorInputs[];
extern u8 gRacePauseState;

void updateControllerEffectSequence(u16 controller);
void updateLegacyControllerActuators(u16 controller);

void resetControllerEffects(void);
