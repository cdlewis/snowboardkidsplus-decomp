#include "../../menu/panel_tasks_internal.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player.h"

extern GameTask *gCurrentTask __attribute__((section(".data")));

void initializeTaskPlayerCamera(void) {
    PlayerCamera *cameras;
    currentPlayerAbsolute = &gRacePlayers[gCurrentTask->data18];
    initPlayerCameraAndTarget(gCurrentTask->data18);
    cameras = gPlayerCameras;
    cameras[gCurrentTask->data18].yaw = currentPlayerAbsolute->unk110;
    cameras[gCurrentTask->data18].pitch = 0;
    cameras[gCurrentTask->data18].unk18 = currentPlayerAbsolute->unk92E;
    updatePlayerCameraPosition(gCurrentTask->data18);
    clampPlayerCameraHeight(gCurrentTask->data18);
    cameras[gCurrentTask->data18].unk10 = -0x230;
    setCurrentTaskCallback(updateTaskViewAndCheckPlayerLimit);
}

void updateTaskViewAndCheckPlayerLimit(void) {
    renderCurrentCameraPlayersAndUpdateSound();
    if (D_8013B55E != 0) {
        if (gCurrentTask->data18 < playerCountAbsolute) {
            setCurrentTaskCallback(updateCurrentPlayerCameraTask);
            return;
        }
        deleteCurrentTaskSubtree();
    }
    updatePlayerCameraPosition(gCurrentTask->data18);
}
