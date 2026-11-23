#include "../engine/input_scheduler_internal.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "psx/libpad.h"

ControllerReceiveBuffer D_800E9558[2] __attribute__((section(".data.controller_receive_buffers"))) = {0};

u8 D_8013AEE4 __attribute__((section(".sdata.controller_input_state"))) = 0;

static u8 sControllerInputStatePadding0 __attribute__((section(".sdata.controller_input_state"))) = 0;

static u8 sControllerInputStatePadding1 __attribute__((section(".sdata.controller_input_state"))) = 0;

static u8 sControllerInputStatePadding2 __attribute__((section(".sdata.controller_input_state"))) = 0;

u16 D_8013AEE8[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

u16 D_8013AEEC[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 D_8013AEF0[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 D_8013AEF4[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 D_8013AEF8[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

s16 D_8013AEFC[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

ControllerActuatorOutput D_8013AF00[2] __attribute__((section(".data.controller_input_arrays"))) = {0};

static u16 sControllerActuatorPadding __attribute__((section(".data.controller_input_arrays"))) = 0;

u8 gControllerAnalogResponse[52] __attribute__((section(".data.controller_analog_response"))) = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x07, 0x0A, 0x0D, 0x10, 0x12, 0x14,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1B, 0x1C, 0x1C, 0x1D, 0x1D, 0x1D, 0x1E,
    0x1E, 0x1E, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00,
};

u8 gControllerFlags[4] __attribute__((section(".data"))) = {0, 0, 0, 0};

u8 gControllerHorizontalRepeatTimers[2]
    __attribute__((section(".sdata.controller_repeat_timing"))) = {0, 0};

u8 gControllerRepeatInitialDelay __attribute__((section(".sdata.controller_repeat_timing"))) = 12;

u8 gControllerRepeatInterval __attribute__((section(".sdata.controller_repeat_timing"))) = 2;

u8 gControllerActuatorAlignment[6] __attribute__((section(".data.controller_actuator_alignment"))) = {0, 1, 0xFF, 0xFF, 0xFF, 0xFF};

static u16 sControllerActuatorAlignmentPadding __attribute__((section(".data.controller_actuator_alignment"))) = 0;

s16 D_8013AF08[2] __attribute__((section(".data.controller_values"))) = {0, 0};

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

/* DECOMP_ATTEMPT: inline 142/155 */
#ifdef NON_MATCHING
extern u8 D_8013AEBC[];

void func_80014D60(u16 controller) {
    u8 flags;
    if (gControllerFlags[controller]) {
        if (D_8013AEEC[controller] & 0x1000) {
            if (gControllerFlags[controller] & 0x2) {
                if (--D_8013AEBC[controller] == 0) {
                    D_8013AEE8[controller] |= 0x1000;
                    D_8013AEBC[controller] = gControllerRepeatInterval;
                }
            } else {
                gControllerFlags[controller] |= (gControllerFlags[controller] & 0xF9) | 0x2;
                D_8013AEBC[controller] = gControllerRepeatInitialDelay;
            }
        }         else if (D_8013AEEC[controller] & 0x4000) {
            flags = gControllerFlags[controller];
            if (flags & 0x4) {
                if (--D_8013AEBC[controller] == 0) {
                    D_8013AEE8[controller] |= 0x4000;
                    D_8013AEBC[controller] = gControllerRepeatInterval;
                }
            } else {
                gControllerFlags[controller] |= (flags & 0xF9) | 0x4;
                D_8013AEBC[controller] = gControllerRepeatInitialDelay;
            }
        } else {
            gControllerFlags[controller] &= 0xF9;
        }
        if (D_8013AEEC[controller] & 0x8000) {
            if (gControllerFlags[controller] & 0x8) {
                if (--gControllerHorizontalRepeatTimers[controller] == 0) {
                    D_8013AEE8[controller] |= 0x8000;
                    gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInterval;
                }
            } else {
                gControllerFlags[controller] |= (gControllerFlags[controller] & 0xE7) | 0x8;
                gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInitialDelay;
            }
        }         else if (D_8013AEEC[controller] & 0x2000) {
            if (gControllerFlags[controller] & 0x10) {
                if (--gControllerHorizontalRepeatTimers[controller] == 0) {
                    D_8013AEE8[controller] |= 0x2000;
                    gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInterval;
                }
            } else {
                gControllerFlags[controller] |= (gControllerFlags[controller] & 0xE7) | 0x10;
                gControllerHorizontalRepeatTimers[controller] = gControllerRepeatInitialDelay;
            }
        } else {
            gControllerFlags[controller] &= 0xE7;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/io/controller_input", func_80014D60);
#endif

void setControllerAxesFromButtons(u16 controller) {
    if (D_8013AEEC[controller] & 0x2000) {
        D_8013AEF8[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x8000) {
        D_8013AEF8[controller] = -31;
    } else {
        D_8013AEF8[controller] = 0;
    }
    if (D_8013AEEC[controller] & 0x4000) {
        D_8013AEFC[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x1000) {
        D_8013AEFC[controller] = -31;
    } else {
        D_8013AEFC[controller] = 0;
    }
}

void updateControllerAxesFromButtons(u16 controller) {
    if (D_8013AEEC[controller] & 0x2000) {
        D_8013AEF8[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x8000) {
        D_8013AEF8[controller] = -31;
    }
    if (D_8013AEEC[controller] & 0x4000) {
        D_8013AEFC[controller] = 31;
    } else if (D_8013AEEC[controller] & 0x1000) {
        D_8013AEFC[controller] = -31;
    }
}

void updateControllerAxesFromAnalog(u16 controller) {
    s16 x, y;
    x = D_800E9558[controller].payload[4] * 100 / 256 - 50;
    y = D_800E9558[controller].payload[5] * 100 / 256 - 50;
    if (x > 45) x = 45;
    if (x < -45) x = -45;
    if (y > 45) y = 45;
    if (y < -45) y = -45;
    if (x >= 0) D_8013AEF8[controller] = (s8)gControllerAnalogResponse[x];
    else D_8013AEF8[controller] = -(s8)gControllerAnalogResponse[-x];
    if (y >= 0) D_8013AEFC[controller] = (s8)gControllerAnalogResponse[y];
    else D_8013AEFC[controller] = -(s8)gControllerAnalogResponse[-y];
    updateControllerAxesFromButtons(controller);
    if (D_8013AEF8[controller] > 26) D_8013AEEC[controller] |= 0x2000;
    if (D_8013AEF8[controller] < -26) D_8013AEEC[controller] |= 0x8000;
    if (D_8013AEFC[controller] > 26) D_8013AEEC[controller] |= 0x4000;
    if (D_8013AEFC[controller] < -26) D_8013AEEC[controller] |= 0x1000;
}

void initializeControllers(void) {
    PadInitDirect((u8 *)&D_800E9558[0], (u8 *)&D_800E9558[1]);
    PadSetAct(0, &D_8013AF00[0].control, 2);
    PadSetAct(0x10, &D_8013AF00[1].control, 2);
    PadStartCom();
    D_8013AF0C[0] = 0;
    D_8013AEE8[0] = 0;
    D_8013AEEC[0] = 0;
    D_8013AEE8[1] = 0;
    D_8013AEEC[1] = 0;
    D_8013AF0C[1] = 0;
    D_8013AEFC[0] = 0;
    D_8013AEF8[0] = 0;
    D_8013AEF4[0] = 0;
    D_8013AEF0[0] = 0;
    D_8013AEFC[1] = 0;
    D_8013AEF8[1] = 0;
    D_8013AEF4[1] = 0;
    D_8013AEF0[1] = 0;
    setControllerAxesFromButtons(0);
    setControllerAxesFromButtons(1);
    resetControllerEffects();
    D_8013AEE4 = 0;
}

s16 isControllerInputSupported(u16 controller) {
    ControllerReceiveBuffer *buffer = D_800E9558;
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
    D_8013AF0C[controller] = D_8013AEEC[controller];
    if (!isControllerInputSupported(controller)) {
        D_8013AEE8[controller] = 0;
        D_8013AEEC[controller] = 0;
        D_8013AEFC[controller] = 0;
        D_8013AEF8[controller] = 0;
        return;
    }
    buffer = D_800E9558;
    packet = buffer + controller;
    device = packet->deviceId & 0xF0;
    if (device == 0x40) {
        D_8013AEEC[controller] = ((u8)~packet->payload[0] << 8) | (u8)~packet->payload[1];
        setControllerAxesFromButtons(controller);
    } else if (device == 0x70) {
        D_8013AEEC[controller] = ((u8)~packet->payload[0] << 8) | (u8)~packet->payload[1];
        updateControllerAxesFromAnalog(controller);
    } else if (device == 0x50) {
        D_8013AEEC[controller] = (u8)~packet->payload[0] << 8;
        buttons = ~packet->payload[1];
        if (buttons & 1) {
            D_8013AEEC[controller] |= 1;
        }
        if (buttons & 2) {
            D_8013AEEC[controller] |= 4;
        }
        if (buttons & 4) {
            D_8013AEEC[controller] |= 0x80;
        }
        if (buttons & 8) {
            D_8013AEEC[controller] |= 0x10;
        }
        if (buttons & 0x10) {
            D_8013AEEC[controller] |= 8;
        }
        if (buttons & 0x20) {
            D_8013AEEC[controller] |= 0x20;
        }
        if (buttons & 0x40) {
            D_8013AEEC[controller] |= 0x40;
        }
        if (buttons & 0x80) {
            D_8013AEEC[controller] |= 2;
        }
        updateControllerAxesFromAnalog(controller);
    }
    D_8013AEE8[controller] = ~D_8013AF0C[controller] & D_8013AEEC[controller];
}

void updateControllerActuators(u16 controller) {
    int state;
    if (gControllerFlags[controller] & 0x20) {
        D_8013AF00[controller].control = 0;
        D_8013AF00[controller].value = 0;
    } else if (PadInfoMode(controller << 4, 2, 0)) {
        updateControllerEffectSequence(controller);
    } else {
        updateLegacyControllerActuators(controller);
    }
    state = PadGetState(controller << 4);
    if (state != 0) {
        if (state == 1) {
            D_8013AF00[controller].aligned = 0;
        }
        if (D_8013AF00[controller].aligned == 0) {
            PadSetAct(controller << 4, &D_8013AF00[controller].control, 2);
            if (state == 2) {
                D_8013AF00[controller].aligned = 1;
            } else if (state == 6) {
                if (PadSetActAlign(controller << 4, gControllerActuatorAlignment)) {
                    D_8013AF00[controller].aligned = 1;
                }
            }
        }
    }
}

void updateControllers(void) {
    updateControllerInput(0);
    func_80014D60(0);
    updateControllerActuators(0);
    updateControllerInput(1);
    func_80014D60(1);
    updateControllerActuators(1);
}

/* DECOMP_ATTEMPT: inline 45/49 */
#ifdef NON_MATCHING
extern u8 D_8013AFD9 __attribute__((section(".data")));

void func_8001596C(u16 controller) {
    if (D_8013AFD7 || D_8013AFD9) {
        D_8013AED4[controller] = ((ControllerInputRecord (*)[256])D_800E959C)[controller];
    } else {
        ControllerInputRecord **slot = D_8013AED4;
        ControllerInputRecord *buffer = (ControllerInputRecord *)0x8017B800;
        slot += controller;
        *slot = buffer;
    }
    D_8013AED4[controller]->repeatCount = 0;
    D_8013AED4[controller]->buttons = 0xFFFF;
    D_8013AEE4 = 2;
    D_8013AED4[controller]->pressedButtons = 0xFFFF;
    D_8013AEDC[controller] = 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/io/controller_input", func_8001596C);
#endif

/* DECOMP_ATTEMPT: inline 152/153 */
#ifdef NON_MATCHING
void func_80015A30(u16 controller) {
    ControllerInputRecord *previous;
    previous = D_8013AED4[controller] + D_8013AEDC[controller] + -1;
    if (previous->buttons == D_8013AEEC[controller] &&
        previous->pressedButtons == D_8013AEE8[controller] &&
        (s8)previous->horizontal == D_8013AEF8[controller] &&
        (s8)previous->vertical == D_8013AEFC[controller] &&
        previous->repeatCount < 255) {
        previous->repeatCount++;
        if (!D_8013AFD7) captureSelectedPlayerStates();
    } else {
        D_8013AED4[controller][D_8013AEDC[controller]].repeatCount = 0;
        D_8013AED4[controller][D_8013AEDC[controller]].horizontal = D_8013AEF8[controller];
        D_8013AED4[controller][D_8013AEDC[controller]].vertical = D_8013AEFC[controller];
        D_8013AED4[controller][D_8013AEDC[controller]].buttons = D_8013AEEC[controller] & 0xF7FF;
        D_8013AED4[controller][D_8013AEDC[controller]].pressedButtons = D_8013AEE8[controller] & 0xF7FF;
        D_8013AEDC[controller]++;
        if (D_8013AFD7) {
            if (D_8013AEDC[controller] + 1 >= 0x200U) {
                D_8013AEE4 = 8;
                D_8013AEDC[controller]--;
            }
        } else if (D_8013AFD9) {
            if (D_8013AEDC[controller] + 1 >= 0x100U) {
                D_8013AEE4 = 8;
                D_8013AEDC[controller]--;
            }
        } else {
            if (D_8013AEDC[controller] + 1 >= 0x900U) D_8013AEE4 = 8;
            else captureSelectedPlayerStates();
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/io/controller_input", func_80015A30);
#endif

void initializeControllerInputPlayback(u16 controller) {
    *(controller + D_8013AED4) = ((ControllerInputRecord (*)[256])D_800E959C)[controller];
    *(controller + D_8013AEDC) = 1;
    *(controller + D_8013AEE0) = (*(controller + D_8013AED4))[1].repeatCount;
    D_8013AEE4 = 1;
}

void playbackControllerInput(u16 controller) {
    u8 *counter;
    D_8013AEEC[controller] = D_8013AED4[controller][D_8013AEDC[controller]].buttons;
    D_8013AEE8[controller] = D_8013AED4[controller][D_8013AEDC[controller]].pressedButtons;
    D_8013AEF8[controller] = (s8)D_8013AED4[controller][D_8013AEDC[controller]].horizontal;
    D_8013AEFC[controller] = (s8)D_8013AED4[controller][D_8013AEDC[controller]].vertical;
    counter = &D_8013AEE0[controller];
    *counter = *counter ? *counter - 1
                        : (D_8013AEDC[controller]++,
                           D_8013AED4[controller][D_8013AEDC[controller]].repeatCount);
}
