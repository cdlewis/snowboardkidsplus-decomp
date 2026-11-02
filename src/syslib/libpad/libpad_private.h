#pragma once

#include "decomp/types.h"

typedef struct PadCommand PadCommand;

typedef struct {
    u8 data[5];
} PadActuatorInfo;

struct PadCommand {
    u8 unknown00[4];
    PadActuatorInfo *actuator_info; /* 0x04 */
    u8 unknown08[0xC];
    void (*send)(PadCommand *);
    int (*receive)(PadCommand *);
    u8 unknown1c[4];
    u8 *alignment;
    u8 parameter;
    u8 unknown25[3];
    u8 *actuators;
    u8 *command_data;
    u8 unknown30[4];
    u8 actuator_length;
    u8 command_length;
    u8 command;
    u8 last_command;
    u8 unknown38[0xe];
    u8 state;
    u8 request_index;
    u8 request_step;
    u8 unknown49[8];
    u8 mode_data[2];
    u8 mode_status;
    u8 unknown54[9];
    u8 alignment_data[6];
    u8 unknown63[0x80];
    u8 mode_count;
    u8 mode_index;
    u8 unknownE5[4];
    u8 actuator_count;
    u8 unknownEA[2];
    int actuator_size;
};
