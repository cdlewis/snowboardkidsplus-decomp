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
extern u16 D_8013AF78 __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013AFD3 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));
extern u32 D_8013AFE4 __attribute__((section(".data")));
extern u8 D_8013AFD7 __attribute__((section(".data")));
extern u8 D_8013B1AA __attribute__((section(".data")));
extern u8 D_8013B1AB __attribute__((section(".data")));

extern const u8 D_80123B50[];
extern const u8 D_8013B0FC[];
extern TexturedQuadListEntry D_80123850[];
extern TexturedQuadListEntry D_801239D0[];
extern TexturedQuadListEntry D_80123910[];
extern TexturedQuadListEntry D_80123A90[];

extern s32 D_8013B100;
extern u8 D_8013B110;

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
    if (D_8013B109 != 1) {
        deleteCurrentTaskSubtree();
        return;
    }
    switch (D_8013B108) {
    case 2:
        createChildTaskWithReserve(scheduleMenuTransitionResourcesTask);
        D_8013AE88->unk10 |= 1;
        break;
    case 25:
        createChildTaskWithReserve(initializeTaskResourcePairWithOneTickTransition);
        D_8013AE88->unk10 |= 1;
        break;
    case 27:
        createChildTaskWithReserve(initializeMode1Task);
        D_8013AE88->unk10 |= 1;
        D_8013B109 = 3;
        D_8013AFD7 = 0;
        break;
    case 28:
        createChildTaskWithReserve(initializeSelectionMenuTask);
        D_8013AE88->unk10 |= 1;
        D_8013B109 = 3;
        D_8013AFD7 = 0;
        break;
    case 26:
        createChildTaskWithReserve(initializeUnlockMenuResourceTask);
        D_8013AE88->unk10 |= 1;
        D_8013B108 = 2;
        break;
    default:
        createChildTaskWithReserve(initializeBackgroundCameraTask);
        D_8013AE8C->data18 = D_8013B108;
        D_8013AE88->unk10 |= 1;
        D_8013B0F0 = 0;
        break;
    }
}

void initializeMode11TransitionTask(void) {
    configureTaskStateTransition(11, 0, 30);
    setSecondaryDisplayTaskMode(1);
    queueMode11TransitionResources();
    setCurrentTaskCallback(dispatchMode11TransitionTask);
    D_8013AE88->unk10 |= 2;
}

void dispatchGameOptionTransitionTask(void) {
    D_8013AF84 = 0x100;
    initPlayerCamera(0);
    initializeDefaultSceneLighting();
    switch (D_8013B109) {
    case 0:
        playMovie(2);
        resetPendingCdRequests();
        D_8013B109 = 1;
        D_8013B0F0 = 0;
        break;
    case 1:
        createChildTaskWithReserve(initializeMode11TransitionTask);
        D_8013AE88->unk10 |= 1;
        break;
    case 2:
        configureTaskStateTransition(1, 0, 30);
        setSecondaryDisplayTaskMode(1);
        queueCdSectorCommand(0, DAT_STARTUP_MODELS_SECTOR, DAT_STARTUP_MODELS_SECTOR_COUNT, DAT_STARTUP_MODELS, 0);
        queueCdSectorCommand(0, DAT_STARTUP_TEXTURES_SECTOR, DAT_STARTUP_TEXTURES_SECTOR_COUNT, DAT_STARTUP_TEXTURES, 0);
        D_8013B109 = 1;
        D_8013B0F0 = 0;
        D_8013AE88->callback = deleteCurrentTaskWhenCdCommandQueueIdle;
        break;
    case 3:
        D_8013B109 = 1;
        D_8013B0F0 = 0;
        deleteCurrentTaskSubtree();
        break;
    }
}

void scheduleMenuTransitionResourcesTask(void) {
    setCurrentTaskCallback(initializeMenuTransitionResourcesTask);
}

void initializeMenuTransitionResourcesTask(void) {
    MenuTransitionResource *resource;
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    resource = (MenuTransitionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
    resource->unk0E = 0;
    setTaskBackgroundMode(0, 0, 0);
    initializeNewGameSaveData();
    if (D_8013B0F0 == 0) {
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(3);
        queueCdSectorCommand(2, DAT_MODE2_TEXTURES_SECTOR, DAT_MODE2_TEXTURES_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B0F0 = 1;
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(waitMode2TransitionTask);
        D_8013AE88->unk10 |= 2;
    } else {
        setCurrentTaskCallback(updateTitleScreenMenuTask);
    }
}

void waitMode2TransitionTask(void) {
    switch (D_8013AE88->data18) {
    case 0:
        configureTaskStateTransition(2, 0, 30);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013AE88->data18 = 0;
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (ModeSelectionState *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    input = &state->input;

    switch (D_8013AE88->data18) {
    case 0:
        createControllerInputSequenceTasks();
        setMusicSequenceEnabled(0);
        setSecondaryDisplayTaskMode(0);
        configureTaskStateTransition(11, 0, 30);
        D_8013B100 = 0;
        D_8013AE88->data1A = 15;
        D_8013AE88->data18 = 1;
        break;
    case 1:
        createTitleLogoTask(4, 1);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (entries[0].unk06 != 0) {
            break;
        }
        input->unk00 = 0;
        input->confirmed = 0;
        input->previousSelection = input->selection = D_8013B110;
        createRegisteredSpacedChildTasks(1, 0x8B0, 0, (const u8 *)gTitleMenuSprites, gTitleMenuSpriteMapping,
                                        0, 1, 0, 0, 316, 48, 0, 15, 3, 0,
                                        drawSelectedTaskSideMarkerSprite,
                                        drawRegisteredTaskGridSpriteWithDefaultBlend, 3, 11);
        D_8013AE88->data18 = 3;
        break;
    case 3:
        if (entries[1].unk06 != 0) {
            break;
        }
        entries[1].unk04 = 23;
        state->selectionDelay = 150;
        state->idleFrames = 0;
        state->selectionFrames = 0;
        D_8013AE88->data18 = 4;
        break;
    case 4:
        if (D_8013AE88->data1A == 0) {
            if (input->selection != input->previousSelection) {
                input->previousSelection = input->selection;
                state->selectionFrames = 0;
            }
            if (state->idleFrames >= 1290) {
                if (state->selectionFrames >= state->selectionDelay) {
                    D_8013B10A++;
                    if (D_8013B10A >= 10) {
                        D_8013B10A = 0;
                    }
                    if (D_8013B10A & 1) {
                        D_8013B109 = 2;
                        D_8013AFD7 = (D_8013B10A >> 1) + 1;
                    } else {
                        D_8013B109 = 0;
                        D_8013AFD7 = 0;
                    }
                    D_8013B110 = 0;
                    D_8013B108 = 2;
                    configureTaskStateTransition(2, 0, 30);
                    input->confirmed = 1;
                    D_8013AE88->data18 = 7;
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
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        if (isTaskStateTransitionBelowMaximum(0)) {
            break;
        }
        switch (input->selection) {
        case 0:
            D_8013B108 = 3;
            break;
        case 1:
            D_8013B108 = 25;
            break;
        case 2:
            D_8013B108 = 26;
            break;
        }
        resetPendingCdRequests();
        D_8013B110 = input->selection;
        D_8013AE88->data18 = 8;
        break;
    case 7:
        if (!isTaskStateTransitionBelowMaximum(0)) {
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
    default:
        resetPendingCdRequests();
        deleteCurrentTaskSubtree();
        return;
    }

    if (D_8013B100 < 41) {
        if (D_8013B100 > 0) {
            D_8013B100++;
        }
    } else {
        countdown = D_8013AE88->data1A;
        if (countdown > 1) {
            D_8013AE88->data1A = countdown - 1;
        } else if (countdown == 1) {
            queueCdCommand(0, 1);
            D_8013AE88->data1A--;
        }
    }
    if (D_8013AE88->data18 >= 4) {
        if (state->promptState != 0) {
            if (state->promptState == 1) {
                initializeSpacedTaskGridPositions(0, entries[1].task, -0x2C000, 0x30000, 0, 0x10000);
                entries[1].unk04 = 22;
                state->promptState = 2;
                state->selectionDelay = 450;
            }
            setSpriteColor(128, 128, 128);
            addTilePacketWithBlend(-72, 44, 144, 54, 128, 128, 128, 2, &D_800EBFF0[D_8013AF78], 4);
        } else {
            if (gFrameCounter & 8) {
                setSpriteColor(128, 128, 128);
                drawUppercaseProportionalText(-40, 62, 20, gTitleScreenStartPrompt, 2, -1, &D_800EBFF0[D_8013AF78], 4);
            }
            if (gControllerPressedButtons[0] & 0x800) {
                playSoundEffect(0, 0, 32);
                state->promptState = 1;
            }
        }
        drawSpriteByIndexWithBlend(-72, 96, 53, 10, -1, &D_800EBFF0[D_8013AF78], 4);
    }
    drawFullScreenTexturedBackground();
}

void drawFullScreenTexturedBackground(void) {
    setSpriteColor(128, 128, 128);
    drawPagedTexturedRectangle(-160, -120, 320, 240, 0x85, 0x39F0,
                  &D_800EBFF0[D_8013AF78], 0xFFF);
}

void createTitleLogoTask(s32 depth, s32 index) {
    TitleLogoResource *logo;
    SelectedTaskEntry *entry;

    createChildTaskWithReserve(updateTitleLogoTask);
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    logo = (TitleLogoResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    logo->soundState = 0;
    D_8013B100 = 0;
    if (index % 10 == 0) {
        SelectedTaskEntry *entries;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = entries + index / 10;
    } else {
        SelectedTaskEntry *entries;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = entries + index / 10;
    }
    entry->unk04 = 0;
    entry->unk06 = 1;
    entry->task = D_8013AE8C;
}

void updateTitleLogoTask(void) {
    TitleLogoResource *logo;
    SelectedTaskEntry *entry;
    SelectedTaskEntry *entries;
    s32 logoX, logoY, subtitleX, subtitleY;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    logo = (TitleLogoResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }

    if (gControllerPressedButtons[0] & 0x8F0) {
        D_8013AE88->data18 = 7;
        if (logo->soundState == 0) {
            logo->soundState = 1;
        }
    }

    switch (D_8013AE88->data18) {
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
        D_8013AE88->data18 = 1;
        break;
    case 2:
        if (--logo->delay == 0) {
            D_8013AE88->data18 = 1;
        }
        break;
    case 1:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->logoVelocityY = -0x8000;
            logo->logoY = -0x1E000;
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->logoVelocityY = -0x4000;
            logo->logoY = -0x1E000;
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->logoVelocityY = -0x2000;
            logo->logoY = -0x1E000;
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        logo->logoVelocityY += 0x1000;
        logo->logoY += logo->logoVelocityY;
        if (logo->logoY >= -0x1E000) {
            logo->soundState = 1;
            logo->logoY = -0x1E000;
            logo->subtitleVelocityX = 0;
            D_8013AE88->data18 = 10;
        }
        break;
    case 10:
        logo->subtitleVelocityX -= 0x1000;
        logo->subtitleX += logo->subtitleVelocityX;
        if (logo->subtitleX <= 0x68000) {
            D_8013AE88->data18 = 11;
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
            D_8013AE88->data18 = 12;
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
            D_8013AE88->data18 = 13;
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
            D_8013AE88->data18 = 14;
        }
        break;
    case 14:
        logo->brightness -= 8;
        if (logo->brightness <= 128) {
            logo->brightness = 128;
            logo->blendMode = -1;
            D_8013AE88->data18 = 7;
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
        D_8013B100 = 1;
    }
    if (D_8013B100 > 0) {
        D_8013B100++;
    }

    logoX = logo->logoX / 4096;
    logoY = logo->logoY / 4096;
    subtitleX = logo->subtitleX / 4096;
    subtitleY = logo->subtitleY / 4096;
    if (logo->flash == 1) {
        setSpriteColor(logo->brightness, logo->brightness, logo->brightness);
        drawTransformedSpriteByIndex((s16)subtitleX, (s16)subtitleY, 0x1BA, 0, logo->blendMode,
                                    0, 32, logo->subtitleScaleX, logo->subtitleScaleY, 0, &D_800EBFF0[D_8013AF78], 4);
        drawTransformedSpriteByIndex((s16)logoX, (s16)logoY, 0x1BB, 0, logo->blendMode,
                                    0, 72, logo->logoScaleX, logo->logoScaleY, 0, &D_800EBFF0[D_8013AF78], 4);
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)subtitleX, (s16)subtitleY, 0x1BA, 0, -1,
                                0, 32, logo->subtitleScaleX, logo->subtitleScaleY, 0, &D_800EBFF0[D_8013AF78], 4);
    drawTransformedSpriteByIndex((s16)logoX, (s16)logoY, 0x1BB, 0, -1,
                                0, 72, logo->logoScaleX, logo->logoScaleY, 0, &D_800EBFF0[D_8013AF78], 4);
    if (logo->flash == 1) {
        setSpriteColor(logo->brightness, logo->brightness, logo->brightness);
        drawSpriteByIndexWithBlend((s16)logoX, (s16)(logoY - 72), 0x1B7, 0, 1, &D_800EBFF0[D_8013AF78], 4);
        drawSpriteByIndexWithBlend((s16)(logoX + 128), (s16)(logoY - 72), 0x1B8, 0, 1, &D_800EBFF0[D_8013AF78], 4);
        drawSpriteByIndexWithBlend((s16)(logoX + 256), (s16)(logoY - 72), 0x1B9, 0, 1, &D_800EBFF0[D_8013AF78], 4);
    }
}

void drawSelectedTaskSideMarkerSprite(void) {
    SelectedTaskSpriteResource *resource;
    SelectedTaskEntry *entries;
    Movement2D *position;
    s32 offset;
    s32 selection;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SelectedTaskSpriteResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
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
                      0x44, 0, -1, &D_800EBFF0[D_8013AF78], (u16)(resource->depth - 1));
    }
}

void drawRegisteredTaskGridSpriteWithDefaultBlend(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            if (parent->mode == 1 && parent->selectedIndex != D_8013AE88->data1A) {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)D_8013AE88->data1A);
            }
            setSpriteColor(128, 128, 128);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}

void initializePlayerModeSelectionTask(void) {
    PlayerModeSelectionResource *resource;

    D_8013AE88->data18 = 1;
    D_8013AE88->data1A = 0;
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskMemoryBlock();
    resource = (PlayerModeSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    resource->unk00 = 0;
    if (gameOptionPlayerModeAbsolute == 2) {
        resource->selection = gameOptionPlayerModeAbsolute - 1;
    } else {
        resource->selection = 0;
    }
    resource->initialSelection = resource->selection;
    resource->unk06 = 0;
    resource->unk08 = 0;
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updatePlayerModeSelectionTask);
}

void updatePlayerModeSelectionTask(void) {
    PlayerModeSelectionResource *selection;
    RegisteredTaskEntry *entries;
    s32 ready;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    selection = (PlayerModeSelectionResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 1:
        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        configureTaskStateTransition(11, 0, 30);
        setTaskBackgroundMode(2, -0x1000, 0x1000);
        createRegisteredPlayerModePanelTask(-120, -48, 4, 21);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (hasTaskStateValue6(0)) {
            break;
        }
        createInputTimedSpriteTask(96, -8, D_80123840, 2, 1, 0xB0, 0x40, 3, 31);
        D_8013AE88->data18 = 3;
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
        D_8013AE88->data18++;
        break;
    case 4:
        if ((s16)entries[2].flags != 0) {
            break;
        }
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, D_80123B50, D_8013B0FC,
                                         0, 1, 0, 20, 248, -16, 0, 56, 2, 0,
                                         drawSelectedTaskMarkerSprite,
                                         drawRegisteredTaskTexturedQuadList, 4, 41);
        D_8013AE88->data18 = 5;
        break;
    case 5:
        if ((s16)entries[4].flags != 0) {
            break;
        }
        moveTaskChildrenWithSpacing(0, entries[4].task, -0x48000, -0x10000, -0x3C000, 0, 2);
        entries[4].command = 23;
        createRepeatedSoundTask(0, 0, 6, 2);
        D_8013AE88->data18 = 6;
        break;
    case 6:
        if ((s16)entries[4].flags != 0) {
            break;
        }
        entries[4].command = 20;
        configureColorCycleTask(1, 0, 128, 8, 0);
        D_8013B41F = 0;
        D_8013AE88->data18 = 7;
        break;
    case 7:
        D_8013AE88->data1A = selection->selection + 1;
        ready = isTaskResourceTransitionComplete(0);
        if (ready == 1 && D_8013B41F == 0) {
            entries[4].command = 22;
            D_8013B41F = ready;
        }
        if ((s16)selection->unk06 > 0) {
            entries[4].command = 10;
            entries[4].flags = 1;
        }
        if ((s16)selection->unk06 != 0) {
            D_8013AE88->data18 = 72;
        }
        break;
    case 72:
        if ((s16)selection->unk06 == 0 || (s16)entries[4].flags != 0) {
            break;
        }
        if (D_8013B41F == 0) {
            entries[4].command = 22;
            D_8013B41F = 1;
        }
        if ((s16)selection->unk06 < 0) {
cancel:
            D_8013AE88->data18 = 10;
            break;
        }
        D_8013AE98->data18 = 4;
        D_8013B10C = -1;
        queueMenuResourceReads((s16)D_8013AE98->data18);
        D_8013AFCD = selection->selection + 1;
        moveTaskChildrenWithSpacing(0, entries[4].task, -0x188000, -0x10000, -0x3C000, 0, -2);
        entries[4].command = 23;
        createRepeatedSoundTask(1, 2, 6, 2);
        D_8013AE88->data18 = 71;
        break;
    case 71:
        D_8013AE88->data18 = 8;
        break;
    case 8:
        if ((s16)entries[4].flags != 0) {
            break;
        }
        configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                            -0x78000, -0x20000, 0, 0x1E000, 0);
        D_8013AE88->data18 = 9;
        break;
    case 9:
        if ((s16)entries[2].flags != 0) {
            break;
        }
        createSecondaryDisplayTask(drawPlayerModeMessagePanelTask);
        D_8013AE88->data18 = 99;
        break;
    case 10:
        D_8013AE98->data18 = 2;
        D_8013B10C = -1;
        configureTaskStateTransition(2, 0, 30);
        setMusicSequenceEnabled(0);
        D_8013AE88->data18 = 11;
        break;
    case 11:
        if (isTaskStateTransitionBelowMaximum(0)) {
            break;
        }
        configureTaskStateTransition(1, 0, 30);
        setSecondaryDisplayTaskMode(1);
        /* fall through */
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void createRegisteredPlayerModePanelTask(s16 x, s16 y, s32 depth, s32 index) {
    RegisteredTaskEntry *entry;

    createChildTaskWithReserve(updateRegisteredPlayerModePanelTask);
    D_8013AE8C->data1C = depth;
    D_8013AE8C->data1E = index;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    initMovement2D((Movement2D *)&gTaskMemoryPool[D_8013AE8C->resource14], x * 4096, y * 4096);
    if (index % 10 == 0) {
        RegisteredTaskEntry *table;
        if (D_8013AE88->resource14 == 0xFFFF) {
            while (1) {}
        }
        table = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource14];
        entry = table + index / 10;
    } else {
        RegisteredTaskEntry *table;
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        table = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
        entry = table + index / 10;
    }
    entry->command = 1;
    entry->flags = 1;
    entry->task = D_8013AE8C;
}

void updateRegisteredPlayerModePanelTask(void) {
    Movement2D *position;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    u16 remainder;
    u16 resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    remainder = D_8013AE88->data1E % 10U;
    resource = D_8013AE98->resource14;
    if (remainder == 0) {
        if (resource == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (entry->command == 99) {
        entry->flags = 0;
        deleteCurrentTaskSubtree();
    } else {
        entry->flags = updateMovement2D(position);
        setSpriteColor(128, 128, 128);
        setTexturedQuadColor(128, 128, 128);
        D_8013AE88->data1A = D_8013AE98->data1A;
        drawWrappedGlyphText16x16((s16)(position->x / 4096 + 8), (s16)(position->y / 4096 + 8),
                                  14, gRegisteredPlayerModePanelTexts[D_8013AE88->data1A], 8, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDefaultTexturedPanel((s16)(position->x / 4096), (s16)(position->y / 4096),
            240, 64, 5, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void drawSelectedTaskMarkerSprite(void) {
    SelectedTaskSpriteResource *resource;
    SelectedTaskEntry *entries;
    Movement2D *position;
    u16 state;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SelectedTaskSpriteResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    entries = (SelectedTaskEntry *)&gTaskMemoryPool[D_8013AE88->resource16];
    state = resource->state;
    if ((s16)resource->state > 0) {
        if ((u32)(state - 1) < 2) {
            setSpriteColor(resource->r, resource->g, resource->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&gTaskMemoryPool[entries[resource->selection].task->resource14];
        drawSpriteByIndexWithBlend(position->x / 4096 - 30, position->y / 4096 + 20,
                      0x44, 0, -1, &D_800EBFF0[D_8013AF78], (u16)(resource->depth - 1));
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
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    color = (TextureColor *)&gTaskMemoryPool[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&gTaskMemoryPool[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            setTexturedQuadColor(color->r, color->g, color->b);
            if (parent->mode == 1 && parent->selectedIndex != D_8013AE88->data1A) {
                texture = mapMenuByteIndex(parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                texture = mapMenuByteIndex(parent->mapping, (s16)D_8013AE88->data1A);
            }
            list = gRegisteredTaskQuadLists[texture];
            drawTexturedQuadList((s16)(position->x / 4096), (s16)(position->y / 4096),
                list, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}

void createRepeatedSoundTask(s32 sound, s32 delay, s32 interval, s32 count) {
    RepeatedSoundResource *resource;

    createChildTaskWithReserve(updateRepeatedSoundTask);
    D_8013AE8C->data18 = delay;
    D_8013AE8C->data1A = interval;
    D_8013AE8C->data1C = 0;
    D_8013AE8C->data1E = count;
    D_8013AE8C->resource14 = allocateTaskMemoryBlock();
    resource = (RepeatedSoundResource *)&gTaskMemoryPool[D_8013AE8C->resource14];
    resource->sound = sound;
}

void updateRepeatedSoundTask(void) {
    RepeatedSoundResource *resource;
    s16 sound;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RepeatedSoundResource *)&gTaskMemoryPool[D_8013AE88->resource14];
    sound = resource->sound;
    if (D_8013AE88->data1E == 0) {
        deleteCurrentTaskSubtree();
    } else if (D_8013AE88->data18 != 0) {
        D_8013AE88->data18--;
    } else {
        if (D_8013AE88->data1C == 0) {
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
            D_8013AE88->data1E--;
            D_8013AE88->data1C = D_8013AE88->data1A;
            if (D_8013AE88->data1C == 0) {
                return;
            }
        }
        D_8013AE88->data1C--;
    }
}

void drawPlayerModeMessagePanelTask(void) {
    if (D_8013AE88->data18 == 99) {
        deleteCurrentTaskSubtree();
    }
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16(-112, -24, 14, gPlayerModeMessageTexts[0], 8, -1,
                              &D_800EBFF0[D_8013AF78], 3);
    drawDefaultTexturedPanel(-120, -32, 240, 64, 5, 0,
                            &D_800EBFF0[D_8013AF78], 3);
}

void startUnlockNotification0Task(void) {
    GameTask *task = D_8013AE88;
    GameTask *parent = D_8013AE98;

    task->data18 = 1;
    parent->data1C++;
    startUnlockNotification(0);
    setCurrentTaskCallback(waitUnlockNotification0TransitionTask);
}

void waitUnlockNotification0TransitionTask(void) {
    if (D_8013AFE4 & 0x200) {
        D_8013AE98->data18 = 5;
        configureTaskStateTransition(11, 0, 30);
        setSecondaryDisplayTaskMode(1);
        D_8013B10C = -1;
        queueMode11TransitionResources();
        queueBackgroundTextureSectorCommands();
        queueMenuResourceReads((s16)D_8013AE98->data18);
        setCurrentTaskCallback(finishUnlockNotification0TransitionTask);
        D_8013AE88->unk10 |= 2;
    }
}

void finishUnlockNotification0TransitionTask(void) {
    configureTaskStateTransition(2, 0, 30);
    D_8013AE98->data1C--;
    deleteCurrentTaskSubtree();
}

void drawSound601SequenceSpriteTask(void) {
    if (gameOptionPlayerModeAbsolute == 1) {
        drawSpriteByIndexWithBlend(-48, -12, 0x1E0, 6, -1, &D_800EC018[D_8013AF78], 1);
    } else if (D_8013AE88->data18 == 0) {
        if (shouldRenderPlayerView(0)) {
            drawSpriteByIndexWithBlend(-40, -12, 0x1E0, 6, -1, &D_800EC018[D_8013AF78], 1);
        }
    } else {
        if (shouldRenderPlayerView(1)) {
            drawSpriteByIndexWithBlend(-40, -12, 0x1E0, 6, -1, &D_800EC0B8[D_8013AF78], 1);
        }
    }
    if (D_8013AFE4 & 8) {
        deleteCurrentTaskSubtree();
    }
}

void beginSound601TaskSequence(void) {
    playSoundEffect(0, 0x601, 0x20);
    setCurrentTaskCallback(drawSound601SequenceSpriteTask);
}

void updateBoardReversePromptTask(void) {
    GameTask *task = D_8013AE88;
    u32 flags = gRacePlayers[task->data18].unk128;
    GameTask *drawTask;
    u16 frame;

    if ((flags & 0x40) || D_8013B1AA) {
        deleteCurrentTaskSubtree();
    } else if (!D_8013B1AB) {
        if (flags & 0x400) {
            if (task->data1A < 30) {
                task->data1A++;
            }
        } else {
            task->data1A = 0;
        }
        task = D_8013AE88;
        if (gRacePlayers[task->data18].unk128 & 0x1000) {
            task->data1A = 0;
        }
        if (!D_8013AFD3) {
            drawTask = D_8013AE88;
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
    if (D_8013AFCD == 1) {
        D_8013AE88->data1C = 0;
        D_8013AE88->data1E = 0;
    } else {
        D_8013AE88->data1C = -32;
        D_8013AE88->data1E = D_8013AE88->data18 * 112 - 48;
    }
    setCurrentTaskCallback(updateBoardReversePromptTask);
}


void drawGrowingSprite179Task(void) {
    GameTask *task = D_8013AE88;
    if (task->data1C < 0x1000) {
        task->data1C += task->data1E;
        if (task->data1C >= 0x1000) {
            task->data1C = 0x1000;
        }
    }
    if (--D_8013AE88->data1A == 0) {
        deleteCurrentTaskSubtree();
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)D_8013AE88->data18, 0, 0x179, 6, -1,
        0x7FFF, 0x7FFF, (s16)D_8013AE88->data1C,
        (s16)D_8013AE88->data1C, 0, &D_800EBFF0[D_8013AF78], 1);
}

void drawGrowingSprite178Task(void) {
    GameTask *task = D_8013AE88;
    if (task->data1C < 0x1000) {
        task->data1C += task->data1E;
        if (task->data1C >= 0x1000) {
            task->data1C = 0x1000;
        }
    }
    if (--D_8013AE88->data1A == 0) {
        D_8013AE88->data1E = 0x400;
        D_8013AE88->data1C = 0;
        D_8013AE88->data1A = 30;
        setCurrentTaskCallback(drawGrowingSprite179Task);
    }
    setSpriteColor(128, 128, 128);
    drawTransformedSpriteByIndex((s16)D_8013AE88->data18, 0, 0x178, 6, -1,
        0x7FFF, 0x7FFF, (s16)D_8013AE88->data1C,
        (s16)D_8013AE88->data1C, 0, &D_800EBFF0[D_8013AF78], 1);
}

void waitThenStartSpriteScaleTransition(void) {
    GameTask *task = D_8013AE88;

    if (--task->data1A == 0) {
        task->data1E = 0x200;
        task->data1C = 0;
        task->data1A = 60;
        setCurrentTaskCallback(drawGrowingSprite178Task);
    }
}

void beginSpriteScaleTransitionDelay(void) {
    if (D_8013AFCD == 1) {
        D_8013AE88->data18 = 0;
    } else {
        D_8013AE88->data18 = -30;
    }
    D_8013AE88->data1A = 45;
    setCurrentTaskCallback(waitThenStartSpriteScaleTransition);
}
