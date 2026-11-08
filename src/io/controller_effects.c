#include "../graphics/render_effects_internal.h"

typedef struct ControllerEffectDefinition {
    u8 priority;
    u8 pad[3];
    ControllerEffectStep *steps;
    s32 duration;
} ControllerEffectDefinition;

static ControllerEffectStep gControllerEffectSteps0[] = {
    {200, -20, {0, 0}, 10},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps1[] = {
    {255, -15, {0, 0}, 10},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps2[] = {
    {255, 5, {0, 0}, 5},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps3[] = {
    {128, 0, {0, 0}, 25},
    {128, -12, {0, 0}, 10},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps4[] = {
    {128, 0, {0, 0}, -1},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps5[] = {
    {180, 0, {0, 0}, 25},
    {180, -12, {0, 0}, 15},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps6[] = {
    {128, -6, {0, 0}, 8},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps7[] = {
    {164, -6, {0, 0}, 8},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps8[] = {
    {200, -8, {0, 0}, 10},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectStep gControllerEffectSteps9[] = {
    {0, 90, {0, 0}, 6},
    {90, 6, {0, 0}, 10},
    {150, 10, {0, 0}, 8},
    {230, -10, {0, 0}, 8},
    {150, -8, {0, 0}, 10},
    {70, 0, {0, 0}, -1},
    {0, 0, {0, 0}, 0},
};

static ControllerEffectDefinition gControllerEffectDefinitions[] = {
    {255, {0, 0, 0}, 0, 0},
    {10, {0, 0, 0}, gControllerEffectSteps1, 0},
    {7, {0, 0, 0}, gControllerEffectSteps4, 15},
    {12, {0, 0, 0}, gControllerEffectSteps5, 15},
    {9, {0, 0, 0}, gControllerEffectSteps0, 5},
    {8, {0, 0, 0}, gControllerEffectSteps3, 0},
    {11, {0, 0, 0}, gControllerEffectSteps0, 5},
    {13, {0, 0, 0}, gControllerEffectSteps2, 5},
    {4, {0, 0, 0}, gControllerEffectSteps6, 5},
    {2, {0, 0, 0}, gControllerEffectSteps8, 0},
    {2, {0, 0, 0}, gControllerEffectSteps7, 0},
    {2, {0, 0, 0}, gControllerEffectSteps6, 0},
    {2, {0, 0, 0}, 0, 5},
    {3, {0, 0, 0}, gControllerEffectSteps9, 0},
    {6, {0, 0, 0}, 0, 10},
    {2, {0, 0, 0}, 0, 2},
    {0, {0, 0, 0}, 0, 0},
};

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

void updateControllerEffectSequence(u16 controller) {
    s16 effect;
    s32 changed;
    ControllerEffectState *state;
    ControllerEffectDefinition *definition;
    if (D_8013AFD3) {
        D_8013AF00[controller].control = 0;
        D_8013AF00[controller].value = 0;
        return;
    }
    effect = D_8013AF08[controller];
    D_8013AF08[controller] = -1;
    changed = 0;
    if (effect >= 16) return;
    state = &gControllerEffectStates[controller];
    if (effect >= 0) {
        definition = &gControllerEffectDefinitions[effect];
        if (definition->priority >= state->priority) {
            state->effect = effect;
            state->priority = definition->priority;
            state->step = definition->steps;
            state->elapsed = 0;
            if (state->step) state->stepEnd = state->step->duration;
            else state->stepEnd = 0;
            state->effectEnd = definition->duration;
            changed = 1;
        }
    }
    if (state->effect == 0) {
        D_8013AF00[controller].value = 0;
        D_8013AF00[controller].control = 0;
        state->priority = 0;
        return;
    }
    if (state->stepEnd > 0 && state->step && state->elapsed >= state->stepEnd) {
        state->step++;
        if (state->step->duration > 0) state->stepEnd = state->elapsed + state->step->duration;
        else state->stepEnd = state->step->duration;
        changed = 1;
    }
    if (state->stepEnd == 0) D_8013AF00[controller].value = 0;
    else if (state->stepEnd < 0 || changed == 1) D_8013AF00[controller].value = state->step->value;
    else D_8013AF00[controller].value += state->step->delta;
    if (state->effectEnd > 0 && state->elapsed >= state->effectEnd) state->effectEnd = 0;
    if (state->effectEnd == 0) D_8013AF00[controller].control = 0;
    else D_8013AF00[controller].control = 1;
    if (state->stepEnd == 0 && state->effectEnd == 0) {
        state->effect = 0;
        state->priority = 0;
    }
    if (state->stepEnd > 0 || state->effectEnd > 0) state->elapsed++;
}

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
