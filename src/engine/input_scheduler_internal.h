#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#include "psx/libcd.h"
#include "psx/libpad.h"
#include "psx/libapi.h"
#include "psx/libapi.h"
#include "game/graphics/graphics.h"
#include "game/audio/music.h"
#include "game/io/cd_queue.h"
#include "psx/libsnd.h"
#include "game/io/controller.h"
#include "game/engine/task.h"
#include "game/engine/game_options.h"
#include "game/io/controller_effects.h"

typedef struct CdCommand {
    s16 command;
    s16 value;
} CdCommand;

typedef struct CdRequest {
    s32 unk_00;
    void *destination;
    char *filename;
    s32 sector;
    s32 sectorCount;
    s16 command;
    s16 unk_16; /* Nonzero selects sectors relative to SNB.DAT. */
    s32 unk_18;
} CdRequest;

extern CdlFILE gAtMovieFile __attribute__((section(".data.cd_files")));
extern CdlFILE gRaMovieFile __attribute__((section(".data.cd_files")));
extern CdlFILE gOpeningMovieFile __attribute__((section(".data.cd_files")));
extern CdlFILE gDatFile __attribute__((section(".data.dat_file")));
extern s32 gDatStartSector __attribute__((section(".sdata.dat_start_sector")));
extern u16 D_8013AE68 __attribute__((section(".sdata.cd_read_state")));

extern s32 D_8013AE6C __attribute__((section(".sdata.cd_read_state")));
extern s32 D_8013AE7C __attribute__((section(".sdata.cd_io_flags")));
extern u8 D_8013B5DC[8] __attribute__((section(".sdata.cd_read_delay")));
extern u8 D_8013B5E4[8] __attribute__((section(".data.cd_sync_result")));
extern CdlFILE D_8013B6C8 __attribute__((section(".data.cd_search_file")));

extern GameTask gGameTasks[200] __attribute__((section(".data.game_tasks")));

extern GameTask *D_8013AE88 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AE8C __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AE90 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AE94 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AE98 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AE9C __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AEA0 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AEA4 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AEA8 __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AEAC __attribute__((section(".sdata.task_scheduler")));
extern GameTask *D_8013AEB0 __attribute__((section(".sdata.task_scheduler")));
extern u16 D_8013AEB4 __attribute__((section(".sdata.task_scheduler")));

extern CdRequest gCdRequests[64] __attribute__((section(".data.cd_requests")));

extern CdCommand gCdCommands[8] __attribute__((section(".data.cd_commands")));

extern u16 gCdRequestPhase;
extern u16 gCdRequestWriteIndex;
extern u16 gCdRequestReadIndex;
extern s16 D_8013AE76;
typedef struct ControllerReceiveBuffer {
    u8 status;
    u8 deviceId;
    u8 payload[32];
} ControllerReceiveBuffer;

extern ControllerReceiveBuffer D_800E9558[2] __attribute__((section(".data.controller_receive_buffers")));
extern u8 D_8013AEE0[];

extern u8 D_8013AEE4 __attribute__((section(".sdata.controller_input_state")));

extern u16 D_8013AEE8[2] __attribute__((section(".data.controller_input_arrays")));
extern u16 D_8013AEEC[2] __attribute__((section(".data.controller_input_arrays")));
extern s16 D_8013AEF0[2] __attribute__((section(".data.controller_input_arrays")));
extern s16 D_8013AEF4[2] __attribute__((section(".data.controller_input_arrays")));
extern s16 D_8013AEF8[2] __attribute__((section(".data.controller_input_arrays")));
extern s16 D_8013AEFC[2] __attribute__((section(".data.controller_input_arrays")));
extern ControllerActuatorOutput D_8013AF00[2] __attribute__((section(".data.controller_input_arrays")));

extern u8 gCdVolumeFadeActive __attribute__((section(".sdata.cd_volume_fade")));
extern u8 gCdVolumeFadeInterval __attribute__((section(".sdata.cd_volume_fade")));
extern s16 gCdVolumeFadeCurrent __attribute__((section(".sdata.cd_volume_fade")));
extern s16 gCdVolumeFadeTarget __attribute__((section(".sdata.cd_volume_fade")));
extern s16 gCdVolumeFadeStep __attribute__((section(".sdata.cd_volume_fade")));
extern s16 gCdVolumeFadeTimer __attribute__((section(".sdata.cd_volume_fade_timer")));

extern u16 gTaskResourceFreeBits[16] __attribute__((section(".data.task_resource_bitmap")));

extern u8 gControllerAnalogResponse[52] __attribute__((section(".data.controller_analog_response")));

extern u8 gControllerFlags[4] __attribute__((section(".data")));

extern u8 gControllerHorizontalRepeatTimers[2]
    __attribute__((section(".sdata.controller_repeat_timing")));
extern u8 gControllerRepeatInitialDelay __attribute__((section(".sdata.controller_repeat_timing")));
extern u8 gControllerRepeatInterval __attribute__((section(".sdata.controller_repeat_timing")));

extern u8 gControllerActuatorAlignment[6] __attribute__((section(".data.controller_actuator_alignment")));

extern s16 D_8013AF08[2] __attribute__((section(".data.controller_values")));

extern u16 D_8013AE48 __attribute__((section(".sdata.cd_command_state")));
extern s16 D_8013AE4A __attribute__((section(".sdata.cd_command_state")));
extern s16 D_8013AE4C __attribute__((section(".sdata.cd_command_state")));
extern u16 D_8013AE4E __attribute__((section(".sdata.cd_command_state")));
extern u16 D_8013AE50 __attribute__((section(".sdata.cd_command_state")));

extern s32 D_8013AE54 __attribute__((section(".sdata.cd_command_state")));
extern s32 D_8013AE58 __attribute__((section(".sdata.cd_command_state")));
extern s32 D_8013AE5C __attribute__((section(".sdata.cd_command_state")));
extern s32 D_8013AE60 __attribute__((section(".sdata.cd_command_state")));
extern s32 D_8013AE64 __attribute__((section(".sdata.cd_command_state")));
extern s16 D_8013AE78 __attribute__((section(".sdata.cd_command_active")));

extern TaskCallback gInitialTaskCallbacks[6] __attribute__((section(".data.initial_task_callbacks")));
/* Scratch links used while removing a subtree from the task list. */
extern GameTask *D_8013AEA0;
extern GameTask *D_8013AEA4;
extern GameTask *D_8013AEA8;
extern GameTask *D_8013AEAC;
extern GameTask *D_8013AEB0;
extern u16 D_8013AEB4;

void func_80013540(void);
void updateControllerAxesFromAnalog(u16 controller);

void func_800131B4(void);
void resetCdVolumeFade(void);
void startCdVolumeFade(s16 startVolume, s16 step, s16 targetVolume, u8 interval);
void updateCdVolumeFade(void);
void initializeCdFiles(void);
s32 queueCdSectorCommand(s16 command, s16 sector, s16 count, void *destination, s32 value);
s32 isCdReading(void);
s32 isCdCommandQueueIdle(void);
s32 isCdRequestQueueReady(void);
void updateCdRequestQueue(void);
void queueCdCommand(s16 command, s16 value);
void resetCdCommandState(void);
void resetPendingCdRequests(void);
void initializeTaskScheduler(void);
void deletePendingTaskSubtree(void);
void setCurrentTaskCallback(TaskCallback callback);
void deleteCurrentTask(void);
u32 allocateTaskResource(void);
void releaseTaskResource(u16 resource);
void setControllerFlag0(u16 controller);
void clearControllerFlag0(u16 controller);
void setControllerRepeatTiming(u8 initialDelay, u8 repeatInterval);
void setControllerFlag5(u16 controller);
void clearControllerFlag5(u16 controller);
s32 isControllerFlag5Clear(u16 controller);
void setControllerAxesFromButtons(u16 controller);
void updateControllerAxesFromButtons(u16 controller);
void initializeControllers(void);
s16 isControllerInputSupported(u16 controller);
void updateControllerInput(u16 controller);
void updateControllerActuators(u16 controller);
void updateControllers(void);
void playbackControllerInput(u16 controller);
