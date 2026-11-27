#include "../engine/input_scheduler_internal.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "psx/libetc.h"
#include "psx/libpad.h"

#define CONTROLLER_INPUT_RECORDING_BUFFER ((ControllerInputRecord *)0x8017B800)

enum {
    REPEAT_UP = 2,
    REPEAT_DOWN = 4,
    REPEAT_LEFT = 8,
    REPEAT_RIGHT = 16,
    REPEAT_VERTICAL = REPEAT_UP | REPEAT_DOWN,
    REPEAT_HORIZONTAL = REPEAT_LEFT | REPEAT_RIGHT
};

ControllerReceiveBuffer gControllerReceiveBuffers[2] __attribute__((section(".data.controller_receive_buffers"))) = {0};

u8 gControllerInputRecordState __attribute__((section(".sdata.controller_input_state"))) = 0;

static u8 sControllerInputStatePadding0 __attribute__((section(".sdata.controller_input_state"))) = 0;

static u8 sControllerInputStatePadding1 __attribute__((section(".sdata.controller_input_state"))) = 0;

static u8 sControllerInputStatePadding2 __attribute__((section(".sdata.controller_input_state"))) = 0;

u16 gControllerPressedButtons[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

u16 gControllerHeldButtons[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 D_8013AEF0[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 D_8013AEF4[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 gControllerHorizontalAxes[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 gControllerVerticalAxes[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

ControllerActuatorOutput gControllerActuatorOutputs[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

static u16 sControllerActuatorPadding __attribute__((section(".data.controller_input_arrays"))) = 0;

u8 gControllerAnalogResponse[52] __attribute__((section(".data.controller_analog_response"))) = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x07, 0x0A, 0x0D, 0x10, 0x12, 0x14,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1B, 0x1C, 0x1C, 0x1D, 0x1D, 0x1D, 0x1E,
    0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00,
};

u8 gControllerFlags[4] __attribute__((section(".data"))) = {0, 0, 0, 0};

u8 gControllerVerticalRepeatTimers[2]
    __attribute__((section(".sdata.controller_repeat_timing"))) = {0, 0};

u8 gControllerHorizontalRepeatTimers[2]
    __attribute__((section(".sdata.controller_repeat_timing"))) = {0, 0};

u8 gControllerRepeatInitialDelay __attribute__((section(".sdata.controller_repeat_timing"))) = 12;

u8 gControllerRepeatInterval __attribute__((section(".sdata.controller_repeat_timing"))) = 2;

u8 gControllerActuatorAlignment[6] __attribute__((section(".data.controller_actuator_alignment"))) = {0, 1, 0xFF, 0xFF, 0xFF, 0xFF};

static u16 sControllerActuatorAlignmentPadding __attribute__((section(".data.controller_actuator_alignment"))) = 0;

s16 D_8013AF08[2] __attribute__((section(".data.controller_values"))) = {0, 0};

extern u8 gDemoRaceIndex __attribute__((section(".data")));
extern u8 gControllerPlaybackIndex __attribute__((section(".data")));

void setControllerFlag0(u16 controller) { gControllerFlags[controller] |= 1; }

void clearControllerFlag0(u16 controller) {
    u8 *flags = controller + gControllerFlags;
    *flags &= ~1;
}

void setControllerRepeatTiming(u8 initialDelay, u8 repeatInterval) {
    gControllerRepeatInitialDelay = initialDelay;
    gControllerRepeatInterval = repeatInterval;
}

void setControllerFlag5(u16 controller) {
    gControllerFlags[controller] |= 0x20;
    D_8013AF08[controller] = -1;
}

void clearControllerFlag5(u16 controller) {
    u8 *flags = controller + gControllerFlags;
    *flags &= ~0x20;
    D_8013AF08[controller] = -1;
}

s32 isControllerFlag5Clear(u16 controller) {
    s32 flags = gControllerFlags[controller];
    flags &= 0x20;
    if (flags) {
        return 0;
    }
    return 1;
}

void updateControllerDirectionalRepeat(u16 controller) {
    if (!gControllerFlags[controller]) {
        return;
    }

    /* Each axis keeps its direction flags until both directions are released. */
    if (gControllerHeldButtons[controller] & PADLup) {
        if (gControllerFlags[controller] & REPEAT_UP) {
            if (--gControllerVerticalRepeatTimers[controller] == 0) {
                gControllerPressedButtons[controller] |= PADLup;
                gControllerVerticalRepeatTimers[controller] = gControllerRepeatInterval;
            }
        } else {
            gControllerFlags[controller] |=
                (gControllerFlags[controller] & (u8)~REPEAT_VERTICAL) | REPEAT_UP;
            gControllerVerticalRepeatTimers[controller] = gControllerRepeatInitialDelay;
        }
    } else if (gControllerHeldButtons[controller] & PADLdown) {
        if (gControllerFlags[controller] & REPEAT_DOWN) {
            if (--gControllerVerticalRepeatTimers[controller] == 0) {
                gControllerPressedButtons[controller] |= PADLdown;
                gControllerVerticalRepeatTimers[controller] = gControllerRepeatInterval;
            }
        } else {
            gControllerFlags[controller] |=
                (gControllerFlags[controller] & (u8)~REPEAT_VERTICAL) | REPEAT_DOWN;
            gControllerVerticalRepeatTimers[controller] = gControllerRepeatInitialDelay;
        }
    } else {
        gControllerFlags[controller] &= ~REPEAT_VERTICAL;
    }

    if (gControllerHeldButtons[controller] & PADLleft) {
        if (gControllerFlags[controller] & REPEAT_LEFT) {
            if (--gControllerHorizontalRepeatTimers[controller] == 0) {
                gControllerPressedButtons[controller] |= PADLleft;
                gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInterval;
            }
        } else {
            gControllerFlags[controller] |=
                (gControllerFlags[controller] & (u8)~REPEAT_HORIZONTAL) | REPEAT_LEFT;
            gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInitialDelay;
        }
    } else if (gControllerHeldButtons[controller] & PADLright) {
        if (gControllerFlags[controller] & REPEAT_RIGHT) {
            if (--gControllerHorizontalRepeatTimers[controller] == 0) {
                gControllerPressedButtons[controller] |= PADLright;
                gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInterval;
            }
        } else {
            gControllerFlags[controller] |=
                (gControllerFlags[controller] & (u8)~REPEAT_HORIZONTAL) | REPEAT_RIGHT;
            gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInitialDelay;
        }
    } else {
        gControllerFlags[controller] &= ~REPEAT_HORIZONTAL;
    }
}

void setControllerAxesFromButtons(u16 controller) {
    if (gControllerHeldButtons[controller] & 0x2000) {
        gControllerHorizontalAxes[controller] = 31;
    } else if (gControllerHeldButtons[controller] & 0x8000) {
        gControllerHorizontalAxes[controller] = -31;
    } else {
        gControllerHorizontalAxes[controller] = 0;
    }
    if (gControllerHeldButtons[controller] & 0x4000) {
        gControllerVerticalAxes[controller] = 31;
    } else if (gControllerHeldButtons[controller] & 0x1000) {
        gControllerVerticalAxes[controller] = -31;
    } else {
        gControllerVerticalAxes[controller] = 0;
    }
}

void updateControllerAxesFromButtons(u16 controller) {
    if (gControllerHeldButtons[controller] & 0x2000) {
        gControllerHorizontalAxes[controller] = 31;
    } else if (gControllerHeldButtons[controller] & 0x8000) {
        gControllerHorizontalAxes[controller] = -31;
    }
    if (gControllerHeldButtons[controller] & 0x4000) {
        gControllerVerticalAxes[controller] = 31;
    } else if (gControllerHeldButtons[controller] & 0x1000) {
        gControllerVerticalAxes[controller] = -31;
    }
}

void updateControllerAxesFromAnalog(u16 controller) {
    s16 x, y;
    x = gControllerReceiveBuffers[controller].payload[4] * 100 / 256 - 50;
    y = gControllerReceiveBuffers[controller].payload[5] * 100 / 256 - 50;
    if (x > 45) x = 45;
    if (x < -45) x = -45;
    if (y > 45) y = 45;
    if (y < -45) y = -45;
    if (x >= 0) gControllerHorizontalAxes[controller] = (s8)gControllerAnalogResponse[x];
    else gControllerHorizontalAxes[controller] = -(s8)gControllerAnalogResponse[-x];
    if (y >= 0) gControllerVerticalAxes[controller] = (s8)gControllerAnalogResponse[y];
    else gControllerVerticalAxes[controller] = -(s8)gControllerAnalogResponse[-y];
    updateControllerAxesFromButtons(controller);
    if (gControllerHorizontalAxes[controller] > 26) gControllerHeldButtons[controller] |= 0x2000;
    if (gControllerHorizontalAxes[controller] < -26) gControllerHeldButtons[controller] |= 0x8000;
    if (gControllerVerticalAxes[controller] > 26) gControllerHeldButtons[controller] |= 0x4000;
    if (gControllerVerticalAxes[controller] < -26) gControllerHeldButtons[controller] |= 0x1000;
}

void initializeControllers(void) {
    PadInitDirect((u8 *)&gControllerReceiveBuffers[0], (u8 *)&gControllerReceiveBuffers[1]);
    PadSetAct(0, &gControllerActuatorOutputs[0].control, 2);
    PadSetAct(0x10, &gControllerActuatorOutputs[1].control, 2);
    PadStartCom();
    D_8013AF0C[0] = 0;
    gControllerPressedButtons[0] = 0;
    gControllerHeldButtons[0] = 0;
    gControllerPressedButtons[1] = 0;
    gControllerHeldButtons[1] = 0;
    D_8013AF0C[1] = 0;
    gControllerVerticalAxes[0] = 0;
    gControllerHorizontalAxes[0] = 0;
    D_8013AEF4[0] = 0;
    D_8013AEF0[0] = 0;
    gControllerVerticalAxes[1] = 0;
    gControllerHorizontalAxes[1] = 0;
    D_8013AEF4[1] = 0;
    D_8013AEF0[1] = 0;
    setControllerAxesFromButtons(0);
    setControllerAxesFromButtons(1);
    resetControllerEffects();
    gControllerInputRecordState = 0;
}

s16 isControllerInputSupported(u16 controller) {
    ControllerReceiveBuffer *buffer = gControllerReceiveBuffers;
    ControllerReceiveBuffer *device;
    buffer += controller;
    device = buffer;
    if (buffer->status != 0) {
        return 0;
    }
    switch (device->deviceId) {
    case 0x41:
    case 0x53:
    case 0x73:
        return 1;
    }
    return 0;
}

void updateControllerInput(u16 controller) {
    ControllerReceiveBuffer *buffer;
    ControllerReceiveBuffer *packet;
    s32 buttons;
    s32 device;
    D_8013AF0C[controller] = gControllerHeldButtons[controller];
    if (!isControllerInputSupported(controller)) {
        gControllerPressedButtons[controller] = 0;
        gControllerHeldButtons[controller] = 0;
        gControllerVerticalAxes[controller] = 0;
        gControllerHorizontalAxes[controller] = 0;
        return;
    }
    buffer = gControllerReceiveBuffers;
    packet = buffer + controller;
    device = packet->deviceId & 0xF0;
    if (device == 0x40) {
        gControllerHeldButtons[controller] = ((u8)~packet->payload[0] << 8) | (u8)~packet->payload[1];
        setControllerAxesFromButtons(controller);
    } else if (device == 0x70) {
        gControllerHeldButtons[controller] = ((u8)~packet->payload[0] << 8) | (u8)~packet->payload[1];
        updateControllerAxesFromAnalog(controller);
    } else if (device == 0x50) {
        gControllerHeldButtons[controller] = (u8)~packet->payload[0] << 8;
        buttons = ~packet->payload[1];
        if (buttons & 1) {
            gControllerHeldButtons[controller] |= 1;
        }
        if (buttons & 2) {
            gControllerHeldButtons[controller] |= 4;
        }
        if (buttons & 4) {
            gControllerHeldButtons[controller] |= 0x80;
        }
        if (buttons & 8) {
            gControllerHeldButtons[controller] |= 0x10;
        }
        if (buttons & 0x10) {
            gControllerHeldButtons[controller] |= 8;
        }
        if (buttons & 0x20) {
            gControllerHeldButtons[controller] |= 0x20;
        }
        if (buttons & 0x40) {
            gControllerHeldButtons[controller] |= 0x40;
        }
        if (buttons & 0x80) {
            gControllerHeldButtons[controller] |= 2;
        }
        updateControllerAxesFromAnalog(controller);
    }
    gControllerPressedButtons[controller] = ~D_8013AF0C[controller] & gControllerHeldButtons[controller];
}

void updateControllerActuators(u16 controller) {
    int state;
    if (gControllerFlags[controller] & 0x20) {
        gControllerActuatorOutputs[controller].control = 0;
        gControllerActuatorOutputs[controller].value = 0;
    } else if (PadInfoMode(controller << 4, 2, 0)) {
        updateControllerEffectSequence(controller);
    } else {
        updateLegacyControllerActuators(controller);
    }
    state = PadGetState(controller << 4);
    if (state != 0) {
        if (state == 1) {
            gControllerActuatorOutputs[controller].aligned = 0;
        }
        if (gControllerActuatorOutputs[controller].aligned == 0) {
            PadSetAct(controller << 4, &gControllerActuatorOutputs[controller].control, 2);
            if (state == 2) {
                gControllerActuatorOutputs[controller].aligned = 1;
            } else if (state == 6) {
                if (PadSetActAlign(controller << 4, gControllerActuatorAlignment)) {
                    gControllerActuatorOutputs[controller].aligned = 1;
                }
            }
        }
    }
}

void updateControllers(void) {
    updateControllerInput(0);
    updateControllerDirectionalRepeat(0);
    updateControllerActuators(0);
    updateControllerInput(1);
    updateControllerDirectionalRepeat(1);
    updateControllerActuators(1);
}

void initializeControllerInputRecording(u16 controller) {
    if (gDemoRaceIndex || gControllerPlaybackIndex) {
        gControllerInputRecordBuffers[controller] = ((ControllerInputRecord (*)[256])D_800E959C)[controller];
    } else {
        gControllerInputRecordBuffers[controller] = CONTROLLER_INPUT_RECORDING_BUFFER;
    }
    gControllerInputRecordBuffers[controller]->repeatCount = 0;
    gControllerInputRecordBuffers[controller]->buttons = 0xFFFF;
    gControllerInputRecordState = 2;
    gControllerInputRecordBuffers[controller]->pressedButtons = 0xFFFF;
    gControllerInputRecordIndices[controller] = 1;
}

void recordControllerInput(u16 controller) {
    ControllerInputRecord *records = gControllerInputRecordBuffers[controller];
    ControllerInputRecord *previous;

    previous = (ControllerInputRecord *)(gControllerInputRecordIndices[controller] * sizeof(*records) +
                                        (u32)records) - 1;
    if (previous->buttons == gControllerHeldButtons[controller] &&
        previous->pressedButtons == gControllerPressedButtons[controller] &&
        (s8)previous->horizontal == gControllerHorizontalAxes[controller] &&
        (s8)previous->vertical == gControllerVerticalAxes[controller] &&
        previous->repeatCount < 255) {
        previous->repeatCount++;
        if (!gDemoRaceIndex) {
            captureSelectedPlayerStates();
        }
    } else {
        gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].repeatCount = 0;
        gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].horizontal = gControllerHorizontalAxes[controller];
        gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].vertical = gControllerVerticalAxes[controller];
        gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].buttons = gControllerHeldButtons[controller] & 0xF7FF;
        gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].pressedButtons = gControllerPressedButtons[controller] & 0xF7FF;
        gControllerInputRecordIndices[controller]++;
        if (gDemoRaceIndex) {
            if (gControllerInputRecordIndices[controller] + 1 >= 0x200U) {
                gControllerInputRecordState = 8;
                gControllerInputRecordIndices[controller]--;
            }
        } else if (gControllerPlaybackIndex) {
            if (gControllerInputRecordIndices[controller] + 1 >= 0x100U) {
                gControllerInputRecordState = 8;
                gControllerInputRecordIndices[controller]--;
            }
        } else {
            if (gControllerInputRecordIndices[controller] + 1 >= 0x900U) {
                gControllerInputRecordState = 8;
            } else {
                captureSelectedPlayerStates();
            }
        }
    }
}

void initializeControllerInputPlayback(u16 controller) {
    *(controller + gControllerInputRecordBuffers) = ((ControllerInputRecord (*)[256])D_800E959C)[controller];
    *(controller + gControllerInputRecordIndices) = 1;
    *(controller + D_8013AEE0) = (*(controller + gControllerInputRecordBuffers))[1].repeatCount;
    gControllerInputRecordState = 1;
}

void playbackControllerInput(u16 controller) {
    u8 *counter;
    gControllerHeldButtons[controller] = gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].buttons;
    gControllerPressedButtons[controller] = gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].pressedButtons;
    gControllerHorizontalAxes[controller] = (s8)gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].horizontal;
    gControllerVerticalAxes[controller] = (s8)gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].vertical;
    counter = &D_8013AEE0[controller];
    *counter = *counter ? *counter - 1
                        : (gControllerInputRecordIndices[controller]++,
                           gControllerInputRecordBuffers[controller][gControllerInputRecordIndices[controller]].repeatCount);
}
