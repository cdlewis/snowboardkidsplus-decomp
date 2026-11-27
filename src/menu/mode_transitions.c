#include "../race/race_presentation_internal.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"
#include "../race/race_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/io/movie_stream.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"

typedef struct TitleLogoResource {
    s32 logoX, logoY;
    s32 unk08;
    s32 logoVelocityY;
    s16 logoScaleX, logoScaleY;
    s32 delay;
    s32 subtitleX, subtitleY;
    s32 subtitleVelocityX;
    s32 unk24;
    s16 subtitleScaleX, subtitleScaleY;
    s16 blendMode;
    u8 flash, unk2F, brightness, soundState;
} TitleLogoResource;

typedef struct ModeSelectionInput {
    s16 unk00;
    s16 selection;
    s16 previousSelection;
    s16 confirmed;
} ModeSelectionInput;

typedef struct ModeSelectionState {
    ModeSelectionInput input;
    s16 unk08;
    s16 idleFrames;
    s16 selectionFrames;
    s16 promptState;
    s16 selectionDelay;
} ModeSelectionState;

extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));
extern u8 gDemoRaceIndex __attribute__((section(".data")));
extern u8 gRaceExitRequested __attribute__((section(".data")));
extern u8 gRaceRestartRequested __attribute__((section(".data")));

extern const u8 D_80123B50[];
extern const u8 D_8013B0FC[];
extern TexturedQuadListEntry D_80123850[16];
extern TexturedQuadListEntry D_801239D0[16];
extern TexturedQuadListEntry D_80123910[16];
extern TexturedQuadListEntry D_80123A90[];

extern s32 gTitleLogoSoundTimer;

const char gTitleScreenStartPrompt[16]
    __attribute__((section(".rodata.title_screen_start_prompt"), aligned(4))) = "PUSH START";

const u32 gTitleMenuJumpTablePadding
    __attribute__((section(".rodata.title_menu_jump_table_padding"))) = 0;

const char gBoardReversePromptPalette1[16]
    __attribute__((section(".rodata.board_reverse_prompt"), aligned(4))) = "%1BOARD REVERSE";

const char gBoardReversePromptPalette2[16]
    __attribute__((section(".rodata.board_reverse_prompt"), aligned(4))) = "%2BOARD REVERSE";

const GridSpriteEntry gTitleMenuSprites[6]
    __attribute__((section(".data.title_menu_sprites"), aligned(4))) = {
    {0x52, 7, 0, 0},
    {0x53, 7, 0, 0},
    {0x54, 7, 0, 0},
    {0x55, 8, 0, 0},
    {0x56, 8, 0, 0},
    {0x57, 8, 0, 0},
};

const u8 gTitleMenuSpriteMapping[8]
    __attribute__((section(".data.title_menu_sprite_mapping"), aligned(4))) = {
    0, 1, 2, 3, 4, 5,
};

TexturedQuadListEntry *gRegisteredTaskQuadLists[4]
    __attribute__((section(".data.registered_task_quad_lists"))) = {
    D_80123850, D_801239D0, D_80123910, D_80123A90
};

const u16 gRegisteredPlayerModePanelText0[28]
    __attribute__((section(".data.registered_player_mode_panel_text"), aligned(4))) = {
    0xC00F, 0x006C, 0x002C, 0x004F, 0x00A8, 0x00DA, 0x00DB, 0x009F,
    0x00BB, 0x0087, 0x006C, 0x0044, 0x0066, 0x002C, 0x00AB, 0x00DC,
    0x00DD, 0x00BF, 0x00DE, 0x00B4, 0x00A5, 0x009A, 0x00CB, 0x009D,
    0x0094, 0x0033, 0x0000, 0x0000,
};

const u16 gRegisteredPlayerModePanelText1[38]
    __attribute__((section(".data.registered_player_mode_panel_text"), aligned(4))) = {
    0xC00E, 0x0080, 0x0056, 0x006B, 0x006C, 0x002C, 0x004F, 0xC00F,
    0x0039, 0xC00E, 0x0052, 0x0044, 0x0063, 0x0043, 0x0052, 0x003F,
    0x004A, 0xC00F, 0x0039, 0xC00E, 0x004F, 0x0049, 0x006B, 0x0074,
    0x002C, 0x0063, 0xC00F, 0x00C1, 0x0087, 0x006C, 0x0044, 0x00CE,
    0x0099, 0x00B1, 0x009F, 0x0033, 0x0000, 0x0000,
};

const u16 gRegisteredPlayerModePanelText2[28]
    __attribute__((section(".data.registered_player_mode_panel_text"), aligned(4))) = {
    0xC00E, 0x0080, 0x0056, 0x006B, 0x006C, 0x002C, 0x004F, 0xC00F,
    0x00A6, 0xC00E, 0x004F, 0x0049, 0x006B, 0x0074, 0x002C, 0x0063,
    0xC00F, 0x00C1, 0x0087, 0x006C, 0x0044, 0x00CE, 0x0099, 0x00B1,
    0x009F, 0x0033, 0x0000, 0x0000,
};

const u16 *gRegisteredPlayerModePanelTexts[3]
    __attribute__((section(".data.registered_player_mode_panel_texts"))) = {
    gRegisteredPlayerModePanelText0, gRegisteredPlayerModePanelText1, gRegisteredPlayerModePanelText2
};

void createTitleLogoTask(s32 depth, s32 index);
void updateTitleLogoTask(void);
void updatePlayerModeSelectionTask(void);
void createRegisteredPlayerModePanelTask(s16 x, s16 y, s32 depth, s32 index);

void dispatchMode11TransitionTask(void) {
    if (gMenuTransitionState != 1) {
        deleteCurrentTaskSubtree();
        return;
    }
    switch (gMenuScreenId) {
    case 2:
        createChildTaskWithReserve(scheduleMenuTransitionResourcesTask);
        gCurrentTask->unk10 |= 1;
        break;
    case 25:
        createChildTaskWithReserve(initializeTaskResourcePairWithOneTickTransition);
        gCurrentTask->unk10 |= 1;
        break;
    case 27:
        createChildTaskWithReserve(initializeMode1Task);
        gCurrentTask->unk10 |= 1;
        gMenuTransitionState = 3;
        gDemoRaceIndex = 0;
        break;
    case 28:
        createChildTaskWithReserve(initializeSelectionMenuTask);
        gCurrentTask->unk10 |= 1;
        gMenuTransitionState = 3;
        gDemoRaceIndex = 0;
        break;
    case 26:
        createChildTaskWithReserve(initializeUnlockMenuResourceTask);
        gCurrentTask->unk10 |= 1;
        gMenuScreenId = 2;
        break;
    default:
        createChildTaskWithReserve(initializeBackgroundCameraTask);
        gCreatedTask->data18 = gMenuScreenId;
        gCurrentTask->unk10 |= 1;
        gTitleMenuResourcesInitialized = 0;
        break;
    }
}

void initializeMode11TransitionTask(void) {
    configureTaskStateTransition(11, 0, 30);
    setSecondaryDisplayTaskMode(1);
    queueMode11TransitionResources();
    setCurrentTaskCallback(dispatchMode11TransitionTask);
    gCurrentTask->unk10 |= 2;
}

void dispatchGameOptionTransitionTask(void) {
    gProjectionDistance = 0x100;
    initPlayerCamera(0);
    initializeDefaultSceneLighting();
    switch (gMenuTransitionState) {
    case 0:
        playMovie(2);
        resetPendingCdRequests();
        gMenuTransitionState = 1;
        gTitleMenuResourcesInitialized = 0;
        break;
    case 1:
        createChildTaskWithReserve(initializeMode11TransitionTask);
        gCurrentTask->unk10 |= 1;
        break;
    case 2:
        configureTaskStateTransition(1, 0, 30);
        setSecondaryDisplayTaskMode(1);
        queueCdSectorCommand(0, DAT_STARTUP_MODELS_SECTOR, DAT_STARTUP_MODELS_SECTOR_COUNT, DAT_STARTUP_MODELS, 0);
        queueCdSectorCommand(0, DAT_STARTUP_TEXTURES_SECTOR, DAT_STARTUP_TEXTURES_SECTOR_COUNT, DAT_STARTUP_TEXTURES, 0);
        gMenuTransitionState = 1;
        gTitleMenuResourcesInitialized = 0;
        gCurrentTask->callback = deleteCurrentTaskWhenCdCommandQueueIdle;
        break;
    case 3:
        gMenuTransitionState = 1;
        gTitleMenuResourcesInitialized = 0;
        deleteCurrentTaskSubtree();
        break;
    }
}

void scheduleMenuTransitionResourcesTask(void) {
    setCurrentTaskCallback(initializeMenuTransitionResourcesTask);
}

void initializeMenuTransitionResourcesTask(void) {
    MenuTransitionResource *resource;
    gCurrentTask->data18 = 0;
    if (gCurrentTask->resource14 != 0xFFFF) { while (1) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (MenuTransitionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) { while (1) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    resource->unk0E = 0;
    setTaskBackgroundMode(0, 0, 0);
    initializeNewGameSaveData();
    if (gTitleMenuResourcesInitialized == 0) {
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(3);
        queueCdSectorCommand(2, DAT_MODE2_TEXTURES_SECTOR, DAT_MODE2_TEXTURES_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        gTitleMenuResourcesInitialized = 1;
        gCurrentTask->data18 = 0;
        setCurrentTaskCallback(waitMode2TransitionTask);
        gCurrentTask->unk10 |= 2;
    } else {
        setCurrentTaskCallback(updateTitleScreenMenuTask);
    }
}

void waitMode2TransitionTask(void) {
    switch (gCurrentTask->data18) {
    case 0:
        configureTaskStateTransition(2, 0, 30);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            gCurrentTask->data18 = 0;
            setCurrentTaskCallback(updateTitleScreenMenuTask);
        }
        break;
    }
}

void updateTitleScreenMenuTask(void) {
    ModeSelectionState *state;
    SelectedTaskEntry *entries;
    ModeSelectionInput *input;
    u16 countdown;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (ModeSelectionState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    input = &state->input;

    switch (gCurrentTask->data18) {
    case 0:
        createControllerInputSequenceTasks();
        setMusicSequenceEnabled(0);
        setSecondaryDisplayTaskMode(0);
        configureTaskStateTransition(11, 0, 30);
        gTitleLogoSoundTimer = 0;
        gCurrentTask->data1A = 15;
        gCurrentTask->data18 = 1;
        break;
    case 1:
        createTitleLogoTask(4, 1);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (entries[0].unk06 != 0) {
            break;
        }
        input->unk00 = 0;
        input->confirmed = 0;
        input->previousSelection = input->selection = gTitleMenuSelectionIndex;
        createRegisteredSpacedChildTasks(1, 0x8B0, 0, (const u8 *)gTitleMenuSprites, gTitleMenuSpriteMapping,
                                        0, 1, 0, 0, 316, 48, 0, 15, 3, 0,
                                        drawSelectedTaskSideMarkerSprite,
                                        drawRegisteredTaskGridSpriteWithDefaultBlend, 3, 11);
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if (entries[1].unk06 != 0) {
            break;
        }
        entries[1].unk04 = 23;
        state->selectionDelay = 150;
        state->idleFrames = 0;
        state->selectionFrames = 0;
        gCurrentTask->data18 = 4;
        break;
    case 4:
        if (gCurrentTask->data1A == 0) {
            if (input->selection != input->previousSelection) {
                input->previousSelection = input->selection;
                state->selectionFrames = 0;
            }
            if (state->idleFrames >= 1290) {
                if (state->selectionFrames >= state->selectionDelay) {
                    gTitleAttractSequenceIndex++;
                    if (gTitleAttractSequenceIndex >= 10) {
                        gTitleAttractSequenceIndex = 0;
                    }
                    if (gTitleAttractSequenceIndex & 1) {
                        gMenuTransitionState = 2;
                        gDemoRaceIndex = (gTitleAttractSequenceIndex >> 1) + 1;
                    } else {
                        gMenuTransitionState = 0;
                        gDemoRaceIndex = 0;
                    }
                    gTitleMenuSelectionIndex = 0;
                    gMenuScreenId = 2;
                    configureTaskStateTransition(2, 0, 30);
                    input->confirmed = 1;
                    gCurrentTask->data18 = 7;
                    break;
                } else {
                    state->selectionFrames++;
                }
            } else {
                state->idleFrames++;
            }
        }
        if (input->confirmed != 0 && entries[1].unk06 == 0) {
            configureTaskStateTransition(2, 0, 30);
            startCdVolumeFade(103, 20, 0, 1);
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        if (isTaskStateTransitionBelowMaximum(0)) {
            break;
        }
        switch (input->selection) {
        case 0:
            gMenuScreenId = 3;
            break;
        case 1:
            gMenuScreenId = 25;
            break;
        case 2:
            gMenuScreenId = 26;
            break;
        }
        resetPendingCdRequests();
        gTitleMenuSelectionIndex = input->selection;
        gCurrentTask->data18 = 8;
        break;
    case 7:
        if (!isTaskStateTransitionBelowMaximum(0)) {
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
    default:
        resetPendingCdRequests();
        deleteCurrentTaskSubtree();
        return;
    }

    if (gTitleLogoSoundTimer < 41) {
        if (gTitleLogoSoundTimer > 0) {
            gTitleLogoSoundTimer++;
        }
    } else {
        countdown = gCurrentTask->data1A;
        if (countdown > 1) {
            gCurrentTask->data1A = countdown - 1;
        } else if (countdown == 1) {
            queueCdCommand(0, 1);
            gCurrentTask->data1A--;
        }
    }
    if (gCurrentTask->data18 >= 4) {
        if (state->promptState != 0) {
            if (state->promptState == 1) {
                initializeSpacedTaskGridPositions(0, entries[1].task, -0x2C000, 0x30000, 0, 0x10000);
                entries[1].unk04 = 22;
                state->promptState = 2;
                state->selectionDelay = 450;
            }
            setSpriteColor(128, 128, 128);
            addTilePacketWithBlend(-72, 44, 144, 54, 128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        } else {
            if (gFrameCounter & 8) {
                setSpriteColor(128, 128, 128);
                drawUppercaseProportionalText(-40, 62, 20, gTitleScreenStartPrompt, 2, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
            }
            if (gControllerPressedButtons[0] & 0x800) {
                playSoundEffect(0, 0, 32);
                state->promptState = 1;
            }
        }
        drawSpriteByIndexWithBlend(-72, 96, 53, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }
    drawFullScreenTexturedBackground();
}

void drawFullScreenTexturedBackground(void) {
    setSpriteColor(128, 128, 128);
    drawPagedTexturedRectangle(-160, -120, 320, 240, 0x85, 0x39F0,
                  &gMainOrderingTables[gActiveGraphicsBuffer], 0xFFF);
}

void createTitleLogoTask(s32 depth, s32 index) {
    TitleLogoResource *logo;
    SelectedTaskEntry *entry;

    createChildTaskWithReserve(updateTitleLogoTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    logo = (TitleLogoResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    gCreatedTask->data18 = 0;
    gCreatedTask->data1C = depth;
    gCreatedTask->data1E = index;
    logo->soundState = 0;
    gTitleLogoSoundTimer = 0;
    if (index % 10 == 0) {
        SelectedTaskEntry *entries;
        if (gCurrentTask->resource14 == 0xFFFF) {
            while (1) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource14];
        entry = entries + index / 10;
    } else {
        SelectedTaskEntry *entries;
        if (gCurrentTask->resource16 == 0xFFFF) {
            while (1) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
        entry = entries + index / 10;
    }
    entry->unk04 = 0;
    entry->unk06 = 1;
    entry->task = gCreatedTask;
}

void updateTitleLogoTask(void) {
    TitleLogoResource *logo;
    SelectedTaskEntry *entry;
    SelectedTaskEntry *entries;
    s32 logoX, logoY, subtitleX, subtitleY;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    logo = (TitleLogoResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }

    if (gControllerPressedButtons[0] & 0x8F0) {
        gCurrentTask->data18 = 7;
        if (logo->soundState == 0) {
            logo->soundState = 1;
        }
    }

    switch (gCurrentTask->data18) {
    case 0:
        logo->logoX = -0x70000;
        logo->logoY = -0x10E000;
        logo->logoScaleX = 4096;
        logo->logoScaleY = 4096;
        logo->subtitleScaleX = 4096;
        logo->subtitleScaleY = 4096;
        logo->brightness = 128;
        logo->flash = 0;
        logo->unk2F = 0;
        logo->blendMode = -1;
        logo->delay = 30;
        logo->logoVelocityY = 0;
        logo->subtitleX = 0xE8000;
        logo->subtitleY = -0x26000;
        gCurrentTask->data18 = 1;
        break;
    case 2:
        if (--logo->delay == 0) {
            gCurrentTask->data18 = 1;
        }
        break;
    case 1:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->logoVelocityY = -0x8000;
            logo->logoY = -0x1E000;
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->logoVelocityY = -0x4000;
            logo->logoY = -0x1E000;
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->logoVelocityY = -0x2000;
            logo->logoY = -0x1E000;
            gCurrentTask->data18 = 9;
        }
        break;
    case 9:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->soundState = 1;
            logo->logoY = -0x1E000;
            logo->subtitleVelocityX = 0;
            gCurrentTask->data18 = 10;
        }
        break;
    case 10:
        logo->subtitleVelocityX -= 0x1000;
        logo->subtitleX += logo->subtitleVelocityX;
        if (logo->subtitleX <= 0x68000) {
            gCurrentTask->data18 = 11;
        }
        break;
    case 11:
        logo->subtitleVelocityX -= 0x1000;
        logo->subtitleX += logo->subtitleVelocityX;
        logo->logoX += (logo->subtitleVelocityX * 32) / 80;
        logo->subtitleScaleX = (logo->subtitleX / 4096 + 144) * 4096 / 216;
        logo->subtitleScaleY = (0x40000 + logo->subtitleVelocityX / 5) / 64;
        logo->logoScaleX = ((logo->subtitleX - logo->logoX) / 4096 + 40) * 16;
        logo->logoScaleY = (0x68000 + (logo->subtitleVelocityX * 32) / 80) / 104;
        if (logo->subtitleX <= -0x8000) {
            logo->logoX = -0x90000;
            logo->subtitleVelocityX = -logo->subtitleVelocityX;
            logo->subtitleVelocityX += 0x1000;
            gCurrentTask->data18 = 12;
        }
        break;
    case 12:
        logo->subtitleVelocityX += 0x1000;
        logo->subtitleX += logo->subtitleVelocityX;
        logo->subtitleScaleX = (logo->subtitleX / 4096 + 144) * 4096 / 216;
        logo->subtitleScaleY = (0x40000 + logo->subtitleVelocityX / 5) / 64;
        logo->logoScaleX = ((logo->subtitleX - logo->logoX) / 4096 + 40) * 16;
        logo->logoScaleY = (0x68000 + (logo->subtitleVelocityX * 32) / 80) / 104;
        if (logo->subtitleX >= 0x68000) {
            logo->subtitleVelocityX = 0;
            gCurrentTask->data18 = 13;
        }
        break;
    case 13:
        logo->subtitleVelocityX -= 0x1000;
        logo->subtitleX += logo->subtitleVelocityX;
        logo->subtitleScaleX = (logo->subtitleX / 4096 + 144) * 4096 / 216;
        logo->subtitleScaleY = (0x40000 + logo->subtitleVelocityX / 5) / 64;
        logo->logoScaleX = ((logo->subtitleX - logo->logoX) / 4096 + 40) * 16;
        logo->logoScaleY = (0x68000 + (logo->subtitleVelocityX * 32) / 80) / 104;
        if (logo->subtitleX <= 0x48000) {
            s16 flash = 1;
            logo->subtitleScaleX = 4096;
            logo->subtitleScaleY = 4096;
            logo->logoScaleX = 4096;
            logo->logoScaleY = 4096;
            logo->brightness = 255;
            logo->flash = flash;
            logo->blendMode = flash;
            logo->subtitleX = 0x48000;
            gCurrentTask->data18 = 14;
        }
        break;
    case 14:
        logo->brightness -= 8;
        if (logo->brightness <= 128) {
            logo->brightness = 128;
            logo->blendMode = -1;
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        logo->logoX = -0x90000;
        logo->logoY = -0x1E000;
        logo->logoScaleX = 4096;
        logo->logoScaleY = 4096;
        logo->subtitleScaleX = 4096;
        logo->subtitleScaleY = 4096;
        logo->brightness = 128;
        logo->blendMode = -1;
        logo->subtitleX = 0x48000;
        logo->subtitleY = -0x26000;
        logo->unk2F = 1;
        entry->unk06 = 0;
        break;
    }

    if (entry->unk04 == 99) {
        deleteCurrentTaskSubtree();
        return;
    }
    if (logo->soundState == 1) {
        playSoundEffect(1, 0xA01, 72);
        logo->soundState = 2;
        gTitleLogoSoundTimer = 1;
    }
    if (gTitleLogoSoundTimer > 0) {
        gTitleLogoSoundTimer++;
    }

    logoX = logo->logoX / 4096;
    logoY = logo->logoY / 4096;
    subtitleX = logo->subtitleX / 4096;
    subtitleY = logo->subtitleY / 4096;
    if (logo->flash == 1) {
        setSpriteColor(logo->brightness, logo->brightness, logo->brightness);
        drawTransformedSpriteByIndex((s16)subtitleX, (s16)subtitleY, 0x1BA, 0, logo->blendMode,
                                    0, 32, logo->subtitleScaleX, logo->subtitleScaleY, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        drawTransformedSpriteByIndex((s16)logoX, (s16)logoY, 0x1BB, 0, logo->blendMode,
                                    0, 72, logo->logoScaleX, logo->logoScaleY, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)subtitleX, (s16)subtitleY, 0x1BA, 0, -1,
                                0, 32, logo->subtitleScaleX, logo->subtitleScaleY, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    drawTransformedSpriteByIndex((s16)logoX, (s16)logoY, 0x1BB, 0, -1,
                                0, 72, logo->logoScaleX, logo->logoScaleY, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    if (logo->flash == 1) {
        setSpriteColor(logo->brightness, logo->brightness, logo->brightness);
        drawSpriteByIndexWithBlend((s16)logoX, (s16)(logoY - 72), 0x1B7, 0, 1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        drawSpriteByIndexWithBlend((s16)(logoX + 128), (s16)(logoY - 72), 0x1B8, 0, 1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        drawSpriteByIndexWithBlend((s16)(logoX + 256), (s16)(logoY - 72), 0x1B9, 0, 1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }
}

void drawSelectedTaskSideMarkerSprite(void) {
    SelectedTaskSpriteResource *resource;
    SelectedTaskEntry *entries;
    Movement2D *position;
    s32 offset;
    s32 selection;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SelectedTaskSpriteResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((s16)resource->state > 0) {
        setSpriteColor(128, 128, 128);
        selection = resource->selection;
        switch (selection) {
        case 0:
        case 1:
            offset = 16;
            break;
        case 2:
            offset = 8;
            break;
        }
        position = (Movement2D *)&gTaskMemoryPool[entries[selection].task->resource14];
        drawSpriteByIndexWithBlend(position->x / 4096 + (s16)(offset - 32), position->y / 4096 - 4,
                      0x44, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], (u16)(resource->depth - 1));
    }
}

void drawRegisteredTaskGridSpriteWithDefaultBlend(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            if (parent->mode == 1 && parent->selectedIndex != gCurrentTask->data1A) {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)(gCurrentTask->data1A + parent->count));
            } else {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)gCurrentTask->data1A);
            }
            setSpriteColor(128, 128, 128);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            entry->flags = 0;
        }
    }
}

void initializePlayerModeSelectionTask(void) {
    PlayerModeSelectionResource *resource;

    gCurrentTask->data18 = 1;
    gCurrentTask->data1A = 0;
    gCurrentTaskParent->data1C++;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (PlayerModeSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    resource->unk00 = 0;
    if (playerCountAbsolute == 2) {
        resource->selection = playerCountAbsolute - 1;
    } else {
        resource->selection = 0;
    }
    resource->initialSelection = resource->selection;
    resource->unk06 = 0;
    resource->unk08 = 0;
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updatePlayerModeSelectionTask);
}

void updatePlayerModeSelectionTask(void) {
    PlayerModeSelectionResource *selection;
    RegisteredTaskEntry *entries;
    s32 ready;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    selection = (PlayerModeSelectionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        configureTaskStateTransition(11, 0, 30);
        setTaskBackgroundMode(2, -0x1000, 0x1000);
        createRegisteredPlayerModePanelTask(-120, -48, 4, 21);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if (hasTaskStateValue6(0)) {
            break;
        }
        createInputTimedSpriteTask(96, -8, gMenuInputPromptSequence, 2, 1, 0xB0, 0x40, 3, 31);
        gCurrentTask->data18 = 3;
        break;
    case 3:
        if ((s16)entries[3].flags > 0) {
            break;
        }
        if ((s16)entries[3].flags < 0) {
            goto cancel;
        }
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                            -0x78000, -0x60000, 0, -0x1E000, 0);
        gCurrentTask->data18++;
        break;
    case 4:
        if ((s16)entries[2].flags != 0) {
            break;
        }
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, D_80123B50, D_8013B0FC,
                                         0, 1, 0, 20, 248, -16, 0, 56, 2, 0,
                                         drawSelectedTaskMarkerSprite,
                                         drawRegisteredTaskTexturedQuadList, 4, 41);
        gCurrentTask->data18 = 5;
        break;
    case 5:
        if ((s16)entries[4].flags != 0) {
            break;
        }
        moveTaskChildrenWithSpacing(0, entries[4].task, -0x48000, -0x10000, -0x3C000, 0, 2);
        entries[4].command = 23;
        createRepeatedSoundTask(0, 0, 6, 2);
        gCurrentTask->data18 = 6;
        break;
    case 6:
        if ((s16)entries[4].flags != 0) {
            break;
        }
        entries[4].command = 20;
        configureColorCycleTask(1, 0, 128, 8, 0);
        gSelectionMenuFadeStarted = 0;
        gCurrentTask->data18 = 7;
        break;
    case 7:
        gCurrentTask->data1A = selection->selection + 1;
        ready = isTaskResourceTransitionComplete(0);
        if (ready == 1 && gSelectionMenuFadeStarted == 0) {
            entries[4].command = 22;
            gSelectionMenuFadeStarted = ready;
        }
        if ((s16)selection->unk06 > 0) {
            entries[4].command = 10;
            entries[4].flags = 1;
        }
        if ((s16)selection->unk06 != 0) {
            gCurrentTask->data18 = 72;
        }
        break;
    case 72:
        if ((s16)selection->unk06 == 0 || (s16)entries[4].flags != 0) {
            break;
        }
        if (gSelectionMenuFadeStarted == 0) {
            entries[4].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        if ((s16)selection->unk06 < 0) {
cancel:
            gCurrentTask->data18 = 10;
            break;
        }
        gCurrentTaskParent->data18 = 4;
        gMenuTextureResourceId = -1;
        queueMenuResourceReads((s16)gCurrentTaskParent->data18);
        playerCountAbsolute = selection->selection + 1;
        moveTaskChildrenWithSpacing(0, entries[4].task, -0x188000, -0x10000, -0x3C000, 0, -2);
        entries[4].command = 23;
        createRepeatedSoundTask(1, 2, 6, 2);
        gCurrentTask->data18 = 71;
        break;
    case 71:
        gCurrentTask->data18 = 8;
        break;
    case 8:
        if ((s16)entries[4].flags != 0) {
            break;
        }
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                            -0x78000, -0x20000, 0, 0x1E000, 0);
        gCurrentTask->data18 = 9;
        break;
    case 9:
        if ((s16)entries[2].flags != 0) {
            break;
        }
        createSecondaryDisplayTask(drawPlayerModeMessagePanelTask);
        gCurrentTask->data18 = 99;
        break;
    case 10:
        gCurrentTaskParent->data18 = 2;
        gMenuTextureResourceId = -1;
        configureTaskStateTransition(2, 0, 30);
        setMusicSequenceEnabled(0);
        gCurrentTask->data18 = 11;
        break;
    case 11:
        if (isTaskStateTransitionBelowMaximum(0)) {
            break;
        }
        configureTaskStateTransition(1, 0, 30);
        setSecondaryDisplayTaskMode(1);
        /* fall through */
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void createRegisteredPlayerModePanelTask(s16 x, s16 y, s32 depth, s32 index) {
    RegisteredTaskEntry *entry;

    createChildTaskWithReserve(updateRegisteredPlayerModePanelTask);
    gCreatedTask->data1C = depth;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[gCreatedTask->resource14], x * 4096, y * 4096);
    if (index % 10 == 0) {
        RegisteredTaskEntry *table;
        if (gCurrentTask->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource14];
        entry = table + index / 10;
    } else {
        RegisteredTaskEntry *table;
        if (gCurrentTask->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
        entry = table + index / 10;
    }
    entry->command = 1;
    entry->flags = 1;
    entry->task = gCreatedTask;
}

void updateRegisteredPlayerModePanelTask(void) {
    Movement2D *position;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    u16 remainder;
    u16 resource;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
    remainder = gCurrentTask->data1E % 10U;
    resource = gCurrentTaskParent->resource14;
    if (remainder == 0) {
        if (resource == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (entry->command == 99) {
        entry->flags = 0;
        deleteCurrentTaskSubtree();
    } else {
        entry->flags = updateMovement2D(position);
        setSpriteColor(128, 128, 128);
        setTexturedQuadColor(128, 128, 128);
        gCurrentTask->data1A = gCurrentTaskParent->data1A;
        drawWrappedGlyphText16x16((s16)(position->x / 4096 + 8), (s16)(position->y / 4096 + 8),
                                  14, gRegisteredPlayerModePanelTexts[gCurrentTask->data1A], 8, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawDefaultTexturedPanel((s16)(position->x / 4096), (s16)(position->y / 4096),
            240, 64, 5, 0, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

void drawSelectedTaskMarkerSprite(void) {
    SelectedTaskSpriteResource *resource;
    SelectedTaskEntry *entries;
    Movement2D *position;
    u16 state;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SelectedTaskSpriteResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (SelectedTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    state = resource->state;
    if ((s16)resource->state > 0) {
        if ((u32)(state - 1) < 2) {
            setSpriteColor(resource->r, resource->g, resource->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&gTaskMemoryPool[entries[resource->selection].task->resource14];
        drawSpriteByIndexWithBlend(position->x / 4096 - 30, position->y / 4096 + 20,
                      0x44, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], (u16)(resource->depth - 1));
    }
}

void drawRegisteredTaskTexturedQuadList(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    TextureColor *color;
    s16 texture;
    TexturedQuadListEntry *list;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    color = (TextureColor *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            setTexturedQuadColor(color->r, color->g, color->b);
            if (parent->mode == 1 && parent->selectedIndex != gCurrentTask->data1A) {
                texture = mapMenuByteIndex(parent->mapping,
                    (s16)(gCurrentTask->data1A + parent->count));
            } else {
                texture = mapMenuByteIndex(parent->mapping, (s16)gCurrentTask->data1A);
            }
            list = gRegisteredTaskQuadLists[texture];
            drawTexturedQuadList((s16)(position->x / 4096), (s16)(position->y / 4096),
                list, 1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            entry->flags = 0;
        }
    }
}

void createRepeatedSoundTask(s32 sound, s32 delay, s32 interval, s32 count) {
    RepeatedSoundResource *resource;

    createChildTaskWithReserve(updateRepeatedSoundTask);
    gCreatedTask->data18 = delay;
    gCreatedTask->data1A = interval;
    gCreatedTask->data1C = 0;
    gCreatedTask->data1E = count;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (RepeatedSoundResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->sound = sound;
}

void updateRepeatedSoundTask(void) {
    RepeatedSoundResource *resource;
    s16 sound;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RepeatedSoundResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    sound = resource->sound;
    if (gCurrentTask->data1E == 0) {
        deleteCurrentTaskSubtree();
    } else if (gCurrentTask->data18 != 0) {
        gCurrentTask->data18--;
    } else {
        if (gCurrentTask->data1C == 0) {
            switch (sound) {
            case 0:
                playSoundEffect(0, 0x200, 0x20);
                break;
            case 1:
                playSoundEffect(0, 0x500, 0x20);
                break;
            case 2:
                playSoundEffect(0, 0x300, 0x20);
                break;
            }
            gCurrentTask->data1E--;
            gCurrentTask->data1C = gCurrentTask->data1A;
            if (gCurrentTask->data1C == 0) {
                return;
            }
        }
        gCurrentTask->data1C--;
    }
}

void drawPlayerModeMessagePanelTask(void) {
    if (gCurrentTask->data18 == 99) {
        deleteCurrentTaskSubtree();
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16(-112, -24, 14, gPlayerModeMessageTexts[0], 8, -1,
                              &gMainOrderingTables[gActiveGraphicsBuffer], 3);
    drawDefaultTexturedPanel(-120, -32, 240, 64, 5, 0,
                            &gMainOrderingTables[gActiveGraphicsBuffer], 3);
}

void startUnlockNotification0Task(void) {
    GameTask *task = gCurrentTask;
    GameTask *parent = gCurrentTaskParent;

    task->data18 = 1;
    parent->data1C++;
    startUnlockNotification(0);
    setCurrentTaskCallback(waitUnlockNotification0TransitionTask);
}

void waitUnlockNotification0TransitionTask(void) {
    if (gTaskTransitionFlags & 0x200) {
        gCurrentTaskParent->data18 = 5;
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(1);
        gMenuTextureResourceId = -1;
        queueMode11TransitionResources();
        queueBackgroundTextureSectorCommands();
        queueMenuResourceReads((s16)gCurrentTaskParent->data18);
        setCurrentTaskCallback(finishUnlockNotification0TransitionTask);
        gCurrentTask->unk10 |= 2;
    }
}

void finishUnlockNotification0TransitionTask(void) {
    configureTaskStateTransition(2, 0, 30);
    gCurrentTaskParent->data1C--;
    deleteCurrentTaskSubtree();
}

void drawSound601SequenceSpriteTask(void) {
    if (playerCountAbsolute == 1) {
        drawSpriteByIndexWithBlend(-48, -12, 0x1E0, 6, -1, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 1);
    } else if (gCurrentTask->data18 == 0) {
        if (shouldRenderPlayerView(0)) {
            drawSpriteByIndexWithBlend(-40, -12, 0x1E0, 6, -1, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 1);
        }
    } else {
        if (shouldRenderPlayerView(1)) {
            drawSpriteByIndexWithBlend(-40, -12, 0x1E0, 6, -1, &gPlayer2OrderingTables[gActiveGraphicsBuffer], 1);
        }
    }
    if (gTaskTransitionFlags & 8) {
        deleteCurrentTaskSubtree();
    }
}

void beginSound601TaskSequence(void) {
    playSoundEffect(0, 0x601, 0x20);
    setCurrentTaskCallback(drawSound601SequenceSpriteTask);
}

void updateBoardReversePromptTask(void) {
    GameTask *task = gCurrentTask;
    u32 flags = gRacePlayers[task->data18].unk128;
    GameTask *drawTask;
    u16 frame;

    if ((flags & 0x40) || gRaceExitRequested) {
        deleteCurrentTaskSubtree();
    } else if (!gRaceRestartRequested) {
        if (flags & 0x400) {
            if (task->data1A < 30) {
                task->data1A++;
            }
        } else {
            task->data1A = 0;
        }
        task = gCurrentTask;
        if (gRacePlayers[task->data18].unk128 & 0x1000) {
            task->data1A = 0;
        }
        if (!gRacePauseState) {
            drawTask = gCurrentTask;
            if (drawTask->data1A >= 30) {
                frame = gFrameCounter;
                if (frame & 2) {
                    drawDebugText((s16)(drawTask->data1C - 52), (s16)drawTask->data1E,
                                  gBoardReversePromptPalette1);
                } else {
                    drawDebugText((s16)(drawTask->data1C - 52), (s16)drawTask->data1E,
                                  gBoardReversePromptPalette2);
                }
            }
        }
    }
}

void initializePlayerPromptTask(void) {
    if (playerCountAbsolute == 1) {
        gCurrentTask->data1C = 0;
        gCurrentTask->data1E = 0;
    } else {
        gCurrentTask->data1C = -32;
        gCurrentTask->data1E = gCurrentTask->data18 * 112 - 48;
    }
    setCurrentTaskCallback(updateBoardReversePromptTask);
}


void drawGrowingSprite179Task(void) {
    GameTask *task = gCurrentTask;
    if (task->data1C < 0x1000) {
        task->data1C += task->data1E;
        if (task->data1C >= 0x1000) {
            task->data1C = 0x1000;
        }
    }
    if (--gCurrentTask->data1A == 0) {
        deleteCurrentTaskSubtree();
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)gCurrentTask->data18, 0, 0x179, 6, -1,
        0x7FFF, 0x7FFF, (s16)gCurrentTask->data1C,
        (s16)gCurrentTask->data1C, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 1);
}

void drawGrowingSprite178Task(void) {
    GameTask *task = gCurrentTask;
    if (task->data1C < 0x1000) {
        task->data1C += task->data1E;
        if (task->data1C >= 0x1000) {
            task->data1C = 0x1000;
        }
    }
    if (--gCurrentTask->data1A == 0) {
        gCurrentTask->data1E = 0x400;
        gCurrentTask->data1C = 0;
        gCurrentTask->data1A = 30;
        setCurrentTaskCallback(drawGrowingSprite179Task);
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)gCurrentTask->data18, 0, 0x178, 6, -1,
        0x7FFF, 0x7FFF, (s16)gCurrentTask->data1C,
        (s16)gCurrentTask->data1C, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 1);
}

void waitThenStartSpriteScaleTransition(void) {
    GameTask *task = gCurrentTask;

    if (--task->data1A == 0) {
        task->data1E = 0x200;
        task->data1C = 0;
        task->data1A = 60;
        setCurrentTaskCallback(drawGrowingSprite178Task);
    }
}

void beginSpriteScaleTransitionDelay(void) {
    if (playerCountAbsolute == 1) {
        gCurrentTask->data18 = 0;
    } else {
        gCurrentTask->data18 = -30;
    }
    gCurrentTask->data1A = 45;
    setCurrentTaskCallback(waitThenStartSpriteScaleTransition);
}
