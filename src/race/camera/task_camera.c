#include "../../menu/panel_tasks_internal.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player.h"

void initializeTaskPlayerCamera(void) {
    PlayerCamera *cameras;
    D_8013B0D8 = &gRacePlayers[D_8013AE88->data18];
    initPlayerCameraAndTarget(D_8013AE88->data18);
    cameras = D_801241E8;
    cameras[D_8013AE88->data18].yaw = D_8013B0D8->unk110;
    cameras[D_8013AE88->data18].pitch = 0;
    cameras[D_8013AE88->data18].unk18 = D_8013B0D8->unk92E;
    updatePlayerCameraPosition(D_8013AE88->data18);
    clampPlayerCameraHeight(D_8013AE88->data18);
    cameras[D_8013AE88->data18].unk10 = -0x230;
    setCurrentTaskCallback(updateTaskViewAndCheckPlayerLimit);
}

void updateTaskViewAndCheckPlayerLimit(void) {
    renderCurrentCameraPlayersAndUpdateSound();
    if (D_8013B55E != 0) {
        if (D_8013AE88->data18 < D_8013AFCD) {
            setCurrentTaskCallback(func_8003EE70);
            return;
        }
        func_800149A8();
    }
    updatePlayerCameraPosition(D_8013AE88->data18);
}
