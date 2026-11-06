#pragma once

#include "decomp/types.h"

extern u16 D_8013AEE8[];
/* Two state values; an incomplete array preserves absolute addressing. */
extern u16 D_8013AEEC[];
extern s16 D_8013AEF8[];
extern s16 D_8013AEFC[];

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

/* Map digital directional buttons to signed controller axes. */
void updateControllerAxesFromButtons(u16 controller);

void func_8001553C(u16 controller);
void func_80014D60(u16 controller);
void func_800157DC(u16 controller);
void updateControllers(void);

/* Apply the current recorded input and advance its repeat counter. */
void playbackControllerInput(u16 controller);
