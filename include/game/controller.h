#pragma once

#include "decomp/types.h"

typedef struct ControllerInputRecord {
    u8 repeatCount;
    u8 horizontal;
    u8 vertical;
    u8 unk_03;
    u16 buttons;
    u16 pressedButtons;
} ControllerInputRecord;

#ifdef CONTROLLER_INPUT_ABSOLUTE_ADDRESSING
extern ControllerInputRecord *D_8013AED4[] __attribute__((section(".data")));
extern u16 D_8013AEDC[] __attribute__((section(".data")));
extern u8 D_8013AEE0[] __attribute__((section(".data")));
#else
extern ControllerInputRecord *D_8013AED4[];
extern u16 D_8013AEDC[];
extern u8 D_8013AEE0[];
#endif

/* Shared recording/playback buffer; storage remains in the controller data unit. */
extern u8 D_800E959C[];

extern u16 D_8013AEE8[];
/* Two state values; an incomplete array preserves absolute addressing. */
extern u16 D_8013AEEC[];
extern s16 D_8013AEF8[];
extern s16 D_8013AEFC[];
extern s16 D_8013AEF4[];
extern u16 D_8013AF0C[2];
#ifdef CONTROLLER_STATE_ABSOLUTE_ADDRESSING
extern u8 D_8013AEE4 __attribute__((section(".data")));
#else
extern u8 D_8013AEE4;
#endif
extern u16 D_8013AEE2 __attribute__((section(".data")));

void initializeControllers(void);

extern u8 gControllerFlags[4] __attribute__((section(".data")));

extern s16 D_8013AF08[2] __attribute__((section(".data.controller_values")));

extern u8 gControllerHorizontalRepeatTimers[2];
extern u8 gControllerRepeatInitialDelay;
extern u8 gControllerRepeatInterval;

/* Configure the initial delay and interval for held directional input. */
void setControllerRepeatTiming(u8 initialDelay, u8 repeatInterval);

void setControllerFlag5(u16 controller);
void setControllerFlag0(u16 controller);
s32 isControllerFlag5Clear(u16 controller);

/* Accept standard digital and analog controller input packets. */
s16 isControllerInputSupported(u16 controller);

/* Set digital axes, clearing each axis when neither direction is held. */
void setControllerAxesFromButtons(u16 controller);

/* Map digital directional buttons to signed controller axes. */
void updateControllerAxesFromButtons(u16 controller);

/* Decode the current controller packet and compute newly pressed buttons. */
void updateControllerInput(u16 controller);
void func_80014D60(u16 controller);
/* Update actuator output and negotiate its controller packet alignment. */
void updateControllerActuators(u16 controller);
void updateControllers(void);

/* Apply the current recorded input and advance its repeat counter. */
void playbackControllerInput(u16 controller);
