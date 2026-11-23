#include "common.h"
#include "game/engine/task.h"
#include "game/engine/system_runtime.h"
#include "game/audio/music.h"
#include "game/io/cd_queue.h"
#include "game/scene/scene_task_types.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/scene/scene_events.h"

#define DAT_SCENE_TEXT (((SceneTextWorkspace *)DAT_SCRATCH_ADDRESS)->text)

typedef struct SceneSoundBankSectors {
    s16 bodySector;
    s16 bodySectorCount;
    s16 headerSector;
    s16 headerSectorCount;
} SceneSoundBankSectors;

extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));
extern u8 D_8013AF3A __attribute__((section(".data")));

static ResourceSectorRange sSceneTextSectors[11] __attribute__((section(".data.scene_text_sectors"))) = {
    { DAT_SCENE_TEXT_DATA_00_SECTOR, DAT_SCENE_TEXT_DATA_00_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_01_SECTOR, DAT_SCENE_TEXT_DATA_01_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_02_SECTOR, DAT_SCENE_TEXT_DATA_02_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_03_SECTOR, DAT_SCENE_TEXT_DATA_03_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_04_SECTOR, DAT_SCENE_TEXT_DATA_04_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_05_SECTOR, DAT_SCENE_TEXT_DATA_05_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_06_SECTOR, DAT_SCENE_TEXT_DATA_06_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_07_SECTOR, DAT_SCENE_TEXT_DATA_07_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_08_SECTOR, DAT_SCENE_TEXT_DATA_08_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_09_SECTOR, DAT_SCENE_TEXT_DATA_09_SECTOR_COUNT },
    { DAT_SCENE_TEXT_DATA_10_SECTOR, DAT_SCENE_TEXT_DATA_10_SECTOR_COUNT },
};

static ResourceSectorRange sSceneEventSectors[11] __attribute__((section(".data.scene_event_sectors"))) = {
    { DAT_SCENE_EVENTS_00_SECTOR, DAT_SCENE_EVENTS_00_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_01_SECTOR, DAT_SCENE_EVENTS_01_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_02_SECTOR, DAT_SCENE_EVENTS_02_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_03_SECTOR, DAT_SCENE_EVENTS_03_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_04_SECTOR, DAT_SCENE_EVENTS_04_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_05_SECTOR, DAT_SCENE_EVENTS_05_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_06_SECTOR, DAT_SCENE_EVENTS_06_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_07_SECTOR, DAT_SCENE_EVENTS_07_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_08_SECTOR, DAT_SCENE_EVENTS_08_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_09_SECTOR, DAT_SCENE_EVENTS_09_SECTOR_COUNT },
    { DAT_SCENE_EVENTS_10_SECTOR, DAT_SCENE_EVENTS_10_SECTOR_COUNT },
};

static SceneSoundBankSectors sSceneSoundBankSectors[12] __attribute__((section(".data.scene_sound_bank_sectors"))) = {
    { DAT_SCENE_SOUND_BODY_00_SECTOR, DAT_SCENE_SOUND_BODY_00_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_00_SECTOR, DAT_SCENE_SOUND_HEADER_00_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_01_SECTOR, DAT_SCENE_SOUND_BODY_01_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_01_SECTOR, DAT_SCENE_SOUND_HEADER_01_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_02_SECTOR, DAT_SCENE_SOUND_BODY_02_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_02_SECTOR, DAT_SCENE_SOUND_HEADER_02_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_03_SECTOR, DAT_SCENE_SOUND_BODY_03_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_03_SECTOR, DAT_SCENE_SOUND_HEADER_03_SECTOR_COUNT },
    { 0, 0, 0, 0 },
    { DAT_SCENE_SOUND_BODY_05_SECTOR, DAT_SCENE_SOUND_BODY_05_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_05_SECTOR, DAT_SCENE_SOUND_HEADER_05_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_06_SECTOR, DAT_SCENE_SOUND_BODY_06_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_06_SECTOR, DAT_SCENE_SOUND_HEADER_06_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_07_SECTOR, DAT_SCENE_SOUND_BODY_07_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_07_SECTOR, DAT_SCENE_SOUND_HEADER_07_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_08_SECTOR, DAT_SCENE_SOUND_BODY_08_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_08_SECTOR, DAT_SCENE_SOUND_HEADER_08_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_09_SECTOR, DAT_SCENE_SOUND_BODY_09_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_09_SECTOR, DAT_SCENE_SOUND_HEADER_09_SECTOR_COUNT },
    { DAT_SCENE_SOUND_BODY_10_SECTOR, DAT_SCENE_SOUND_BODY_10_SECTOR_COUNT,
      DAT_SCENE_SOUND_HEADER_10_SECTOR, DAT_SCENE_SOUND_HEADER_10_SECTOR_COUNT },
    { 0, 0, 0, 0 },
};

void initializeScenePlaybackResourcesTask(void) {
    ScenePlaybackState *state;
    s16 i;

    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    state = (ScenePlaybackState *)&D_800D9504[D_8013AE88->resource16];
    resetScenePlaybackStates(NULL, (SceneChannelState *)state);
    resetSceneEventPool();
    for (i = 0; i < 5; i++) {
        state->tasks[i] = NULL;
    }
    D_8013AFE4 &= ~0x200;
    configureTaskStateTransition(0, 0, 0);
    closeMusicSequence();
    closeAllSoundBanks();

    queueCdSectorCommand(0, DAT_MODE11_RESOURCE_07CD_SECTOR, DAT_MODE11_RESOURCE_07CD_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(3, DAT_MODE11_RESOURCE_0801_SECTOR, DAT_MODE11_RESOURCE_0801_SECTOR_COUNT, DAT_SOUND_HEADER_0, 0);
    if (sSceneSoundBankSectors[D_8013AF3A].bodySectorCount > 0) {
        closeSoundBank(1);
        queueCdSectorCommand(0, sSceneSoundBankSectors[D_8013AF3A].bodySector,
                             sSceneSoundBankSectors[D_8013AF3A].bodySectorCount, DAT_SCRATCH, 0);
        queueCdSectorCommand(4, sSceneSoundBankSectors[D_8013AF3A].headerSector,
                             sSceneSoundBankSectors[D_8013AF3A].headerSectorCount, DAT_SOUND_HEADER_1, 0);
    }
    closeSoundBank(2);
    queueCdSectorCommand(0, DAT_ENTRY_154_SECTOR, DAT_ENTRY_154_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(5, DAT_ENTRY_155_SECTOR, DAT_ENTRY_155_SECTOR_COUNT, DAT_SOUND_HEADER_2, 0);
    queueCdSectorCommand(9, DAT_ENTRY_153_SECTOR, DAT_ENTRY_153_SECTOR_COUNT, DAT_MUSIC_SEQUENCE, 0);
    queueCdSectorCommand(1, DAT_ENTRY_176_SECTOR, DAT_ENTRY_176_SECTOR_COUNT, DAT_SCENE_TEXT, 0);
    queueCdSectorCommand(1, DAT_ENTRY_177_SECTOR, DAT_ENTRY_177_SECTOR_COUNT, DAT_SCENE_TEXT, 0);
    queueCdSectorCommand(0, sSceneEventSectors[D_8013AF3A].sector,
                         sSceneEventSectors[D_8013AF3A].sectorCount, DAT_SCRATCH, 0);
    queueCdSectorCommand(0, sSceneTextSectors[D_8013AF3A].sector,
                         sSceneTextSectors[D_8013AF3A].sectorCount, DAT_SCENE_TEXT, 0);

    setSecondaryDisplayTaskMode(1);
    setCurrentTaskCallback(initializeLoadedSceneEvents);
    D_8013AE88->unk10 |= 2;
}
