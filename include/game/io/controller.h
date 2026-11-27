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

typedef struct ControllerReceiveBuffer {
    u8 status;
    u8 deviceId;
    u8 payload[32];
} ControllerReceiveBuffer;

extern ControllerReceiveBuffer gControllerReceiveBuffers[2]
    __attribute__((section(".data.controller_receive_buffers")));

/* Two controller slots; incomplete bounds preserve absolute addressing. */
extern ControllerInputRecord *gControllerInputRecordBuffers[];
extern u16 gControllerInputRecordIndices[];
extern u8 gControllerPlaybackRepeatCounters[];

extern ControllerInputRecord gControllerPlaybackRecords[2][256]
    __attribute__((section(".data.controller_playback_records")));

/* Per-controller vibration suppression settings (0 or 0x20). */
extern u8 gControllerVibrationDisabled[];

/* Button presses this update, including generated directional repeats. */
extern u16 gControllerPressedButtons[];
/* Two state values; an incomplete array preserves absolute addressing. */
extern u16 gControllerHeldButtons[];
extern s16 gControllerHorizontalAxes[];
extern s16 gControllerVerticalAxes[];
extern s16 D_8013AEF4[];
extern u16 gControllerPreviousButtons[2];
/* Recording/playback state bits: playback 1, recording 2, complete 4, full 8. */
extern u8 gControllerInputRecordState;
extern u16 gSelectedPlayerReplayEndRecordIndex __attribute__((section(".data")));

void initializeControllers(void);

extern u8 gControllerFlags[4] __attribute__((section(".data")));

extern s16 gControllerPendingEffects[2] __attribute__((section(".data.controller_values")));

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
/* Apply the analog response curve and merge directional button input. */
void updateControllerAxesFromAnalog(u16 controller);

void updateControllerDirectionalRepeat(u16 controller);
/* Update actuator output and negotiate its controller packet alignment. */
void updateControllerActuators(u16 controller);
void updateControllers(void);

void initializeControllerInputRecording(u16 controller);
void recordControllerInput(u16 controller);
void initializeControllerInputPlayback(u16 controller);

/* Apply the current recorded input and advance its repeat counter. */
void playbackControllerInput(u16 controller);
