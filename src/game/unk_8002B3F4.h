#pragma once

#include "common.h"
#include "game/global_flags.h"
#include "game/angles.h"
#include "game/coordinate.h"
#include "game/movement.h"
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/game_task_scheduler.h"
#include "game/race_player.h"
#include "game/race_camera.h"
#include "game/coordinate.h"
#include "game/race_time.h"
#include "game/selected_records.h"
#include "game/selected_record.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/game_options.h"
#include "game/controller.h"
#include "game/cd_queue.h"
#include "game/controller_effects.h"
#include "game/controller.h"
#include "psx/libsnd.h"
#include "psx/libgte.h"

typedef struct MenuTransitionResource {
    u8 unk00[0xE];
    s16 unk0E;
} MenuTransitionResource;
typedef struct FallingObjectResource {
    GsCOORDINATE2 coordinate, parent;
    Position3D velocity;
    s32 heightLimit;
    u8 unkB0[0x10];
    SVECTOR rotation, parentRotation;
    u8 unkD0[0x30];
} FallingObjectResource;

typedef struct SelectedTaskSpriteResource {
    u8 unk00[0x30];
    u16 state;
    s16 selection;
    u8 unk34[0x1C];
    u16 depth;
    u8 unk52[0x10];
    u8 r, g, b;
    u8 unk65[0x9B];
} SelectedTaskSpriteResource;

typedef struct SelectedTaskEntry {
    GameTask *task;
    s16 unk04, unk06;
} SelectedTaskEntry;

typedef struct ExpandingCoordinateEffectResource {
    GsCOORDINATE2 coordinate, parent;
    Position3D position;
    s32 unkAC;
    VECTOR scale;
    SVECTOR rotation;
    s16 pitch, unkCA, yaw, unkCE, selectedIndex;
    u8 unkD2[0x2E];
} ExpandingCoordinateEffectResource;
typedef struct PlayerParticleEntry {
    Position3D position;
    s32 framePosition;
} PlayerParticleEntry;

typedef struct PlayerParticleResource {
    PlayerParticleEntry entries[16];
} PlayerParticleResource;

typedef struct FallingVisualResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x24];
    s32 height;
    u8 unk78[0x50];
    s16 unkC8, framePosition;
    u8 r0, g0, b0, code;
    s32 unkD0, velocity, unkD8;
    s32 animationTick;
    s32 delay, triggered;
    u8 unkE8[0x18];
} FallingVisualResource;

typedef struct PlayerModeSelectionResource {
    u16 unk00;
    u16 selection;
    u16 initialSelection;
    u16 unk06;
    u16 unk08;
    u8 unk0A[0xF6];
} PlayerModeSelectionResource;

typedef struct OrbitingPlayerVisualResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x20];
    Position3D offset;
    s32 radius;
    u8 unk80[0x44];
    s16 unkC4, frameLimit, frameStep, framePosition;
    u8 r0, g0, b0, code;
    s32 unkD0, scale, angle, countdown;
    u8 unkE0[0x20];
} OrbitingPlayerVisualResource;

typedef struct RisingPlayerEffectResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    Position3D offset;
    u8 unkAC[4];
    VECTOR scale;
    SVECTOR rotation;
    u8 unkC8[0x10];
    s32 unkD8;
    s32 velocity;
    u8 unkE0[0x20];
} RisingPlayerEffectResource;

typedef struct PlayerEffectResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x60];
    VECTOR scale;
    SVECTOR rotation;
    u8 unkC8[0x38];
} PlayerEffectResource;

typedef struct ScaledCoordinateResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x60];
    VECTOR scale;
    u8 unkC0[0x40];
} ScaledCoordinateResource;

typedef struct FallingSpriteEffect {
    s16 active;
    s16 x, y;
    s16 frame;
    s16 unk08;
    s16 velocityX, velocityY;
    s16 unk0E, unk10;
    s16 scale;
    s16 palette;
} FallingSpriteEffect;

typedef struct TimeRecordHeaderResource {
    u8 unk00[6];
    s16 brightness;
    u8 unk08[0xF8];
} TimeRecordHeaderResource;

typedef struct TimeRecordDetailCountdownResource {
    u8 unk00[8];
    s16 timer;
    s16 unk0A;
    u8 unk0C[8];
    s16 unk14;
    u8 unk16[0xEA];
} TimeRecordDetailCountdownResource;

typedef struct MenuTimeResource {
    s16 selection;
    s16 unk02;
    s16 brightness;
    u8 unk06[0xFA];
} MenuTimeResource;

typedef struct RepeatedSoundResource {
    s16 sound;
    u8 unk02[0xFE];
} RepeatedSoundResource;

typedef struct SpriteAnimationResource {
    s16 x;
    s16 y;
    u16 frame;
    u16 unk06;
    u8 player;
    u8 checkPlayer;
    u8 alternateLayer;
    u8 unk0B[0xF5];
} SpriteAnimationResource;

typedef struct SlidingTimeResource {
    u16 timer;
    u16 unk02;
    s32 x;
    s32 y;
    u8 unk0C[8];
    s32 speed;
    RaceTime time;
    s8 hasExtraTime;
    u8 unk1D[0xE3];
} SlidingTimeResource;

typedef struct SlidingNumberResource {
    u16 timer;
    u16 unk02;
    s32 x;
    s32 y;
    u8 unk0C[8];
    s32 speed;
    s32 value;
    s16 player;
    u8 unk1E[0xE2];
} SlidingNumberResource;

typedef struct UnpackedWordBytes {
    u8 unk0;
    u8 unk1;
    u16 unk2;
} UnpackedWordBytes;


typedef struct MenuSpritePaletteGroups {
    u8 unk00[3];
    u8 kind[8];
    u8 mappedIconPalettes[9];
} MenuSpritePaletteGroups;

typedef struct PlayerNumberResource {
    u16 timer;
    u16 unk02;
    s32 x;
    s32 y;
    u8 unk0C[8];
    s32 speed;
    s32 value;
    s16 showExtraValue;
    s16 player;
    u8 unk20[0xE0];
} PlayerNumberResource;

typedef struct PanelResource {
    RaceTime time[2];
    s16 brightness;
    s16 timer;
    s16 unk0C;
    s16 unk0E[2];
    u8 unk12[0xEE];
} PanelResource;

/* The fade callbacks use a full task resource record. */
typedef struct TaskFadeResource {
    s16 brightness;
    s16 timer;
    s8 playerOrder[4];
    u8 unk08[0xF8];
} TaskFadeResource;

typedef struct ResultDisplayResource {
    s16 brightness;
    s16 timer;
    s16 unk04;
    s16 unk06;
    s16 unk08;
    u8 unk0A[0xF6];
} ResultDisplayResource;

typedef struct PanelInputResource {
    u8 unk00[0xA];
    s16 timer;
    u8 unk0C[0xF4];
} PanelInputResource;

typedef struct StatsMenuFadeResource {
    s16 unk00;
    s16 brightness;
    s16 timer;
    s16 unk06;
    s16 trickPrize;
    s16 makeBonus;
    s16 completeBonus;
    u8 unk0E[0xF2];
} StatsMenuFadeResource;

typedef struct RaceResultsFadeResource {
    s16 unk00;
    RaceTime difference;
    s16 brightness;
    s16 timer;
    s16 unk0A;
    s16 timeBonus;
    s16 bonusTimer;
    s16 displayTimer;
    s16 scoreBonus;
    s16 hasBonus;
    u8 unk16[0xEA];
} RaceResultsFadeResource;

typedef struct MenuSoundDelayResource {
    s16 unk00;
    s16 brightness;
    s16 timer;
    s16 unk06;
    u8 unk08[6];
    s16 unk0E;
    u8 unk10[0xF0];
} MenuSoundDelayResource;

typedef struct TaskMenuFadeResource {
    s16 unk00;
    s16 brightness;
    s16 secondaryBrightness;
    s16 timer;
    u8 unk08[0xF8];
} TaskMenuFadeResource;

typedef struct MenuCountdownResource {
    s16 unk00, unk02;
    s16 timer;
    s16 unk06;
    u8 unk08[0xF8];
} MenuCountdownResource;

typedef struct MenuFadeInResource {
    s16 unk00;
    s16 brightness;
    s16 timer;
    s16 unk06;
    s16 unk08, unk0A, unk0C, unk0E;
    u8 unk10[0xF0];
} MenuFadeInResource;

typedef struct SlidingDisplayResource {
    s32 position;
    u8 unk04[0xC];
    s32 velocity;
    u16 timer;
    u8 unk16[0xEA];
} SlidingDisplayResource;


typedef struct TaskMotionResource {
    s32 position;
    u8 unk04[12];
    s32 velocity;
    u16 delay;
    u8 unk16[0xEA];
} TaskMotionResource;

typedef struct SpriteMotionResource {
    u16 timer;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    s32 unk10;
    s32 unk14;
    s16 variant;
    u8 padding[0xE6];
} SpriteMotionResource;

typedef struct TrailVisualResource {
    GsCOORDINATE2 coordinate;
    Position3D position;
    u8 unk5C[0x6A];
    s16 unkC6, unkC8, framePosition;
    u8 r0, g0, b0, code;
    s32 scale, unkD4, angle, lifetime;
    u8 unkE0[0x20];
} TrailVisualResource;

typedef struct BouncingVisualResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x24];
    s32 offsetY;
    u8 unk78[0x50];
    s16 timer, framePosition;
    u8 r0, g0, b0, code;
    s32 unkD0, velocity, unkD8, frame, delay, soundPlayed;
    u8 unkE8[0x18];
} BouncingVisualResource;

typedef struct SlidingVisualResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x24];
    s32 unk74;
    u8 unk78[0x50];
    s16 timer;
    s16 framePosition;
    u8 r0, g0, b0, code;
    s32 step;
    s32 velocity;
    u8 unkD8[0x28];
} SlidingVisualResource;



extern u16 gPlayerSoundVariantsA[2] __attribute__((section(".data.player_sound_variants")));

extern u16 gPlayerSoundVariantsB[2] __attribute__((section(".data.player_sound_variants")));

extern const char gDemonstrationText[16]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4)));

extern const char gDemonstrationStartText[12]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4)));

extern GsRVIEW2 gPlayerCameraViews[2]
    __attribute__((section(".data.player_camera_views")));

extern MenuSpritePaletteGroups gMenuSpritePaletteGroups
    __attribute__((section(".data.menu_sprite_palettes"), aligned(4)));

extern u16 gControllerInputSequences[3][26]
    __attribute__((section(".data.controller_input_sequences")));

extern u8 gResultLabelPalettes[12]
    __attribute__((section(".data.result_label_tables")));

extern u8 gResultLabelXOffsets[12]
    __attribute__((section(".data.result_label_tables")));

extern u8 gResultIconPalettes[4]
    __attribute__((section(".data.result_icon_palettes"), aligned(4)));

extern const u16 gPlayerModeMessageText[] __attribute__((section(".data.player_mode_messages")));

extern const u16 gPlayerModeAlternateMessageText[] __attribute__((section(".data.player_mode_messages")));

extern const u16 *gPlayerModeMessageTexts[] __attribute__((section(".data.player_mode_message_texts")));

extern TaskVisualFrame gPlayerSideVisualFrames[4]
    __attribute__((section(".data.player_side_visual_frames"), aligned(4)));

extern TaskVisualFrame gPlayerFlagVisualFrames[6]
    __attribute__((section(".data.player_flag_visual_frames"), aligned(4)));

extern TaskVisualFrame gFallingPairedVisualFrames[7]
    __attribute__((section(".data.falling_paired_visual_frames"), aligned(4)));

extern u8 gFallingPairedVisualAnimation[6]
    __attribute__((section(".data.falling_paired_visual_animation"), aligned(2)));

extern TaskVisualFrame gPlayerDualResourceEffectFrame
    __attribute__((section(".data.player_dual_resource_effect_frame"), aligned(4)));

extern TaskVisualFrame gPlayerVisualPair5E80Frame
    __attribute__((section(".data.player_visual_pair_5e80_frame"), aligned(4)));

extern u8 D_8013B18D __attribute__((section(".data")));

extern s16 D_8013B10C;
extern s16 D_8013B10E;
extern u8 D_80123640[];

extern s16 D_8013B0E4;
extern u16 D_8013B0E6;
extern s32 D_8013B0E8;
extern TaskVisualFrame gPlayerVisualPairFrame
    __attribute__((section(".data.player_visual_pair_frame"), aligned(4)));

extern TaskVisualFrame gPlayerVisualPair6780Frame
    __attribute__((section(".data.player_visual_pair_6780_frame"), aligned(4)));



extern u16 D_8013AFAA __attribute__((section(".data")));
extern s32 D_8013AF84 __attribute__((section(".data")));
extern u8 D_8013AFE0 __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));


void func_80048340(void);
void func_80031C98(void);
void func_80048AA4(void);
void updatePlayerSideVisualTask(void);
void func_80045C7C(void);
void initializePlayerVisualHistory(TaskVisualResource *resource, VECTOR *vectors, s32 value, s32 scale);
s16 func_80046228(Position3D *position, s32 distance, s16 angle, s16 player, s16 *targetAngle);
void updateHomingTaskVisualMovement(TaskVisualResource *resource, VECTOR *history, s32 distance, s32 maxTurn);
void initializeBouncingTaskVisual(void);
void updateBouncingTaskVisual(void);
void updateAcceleratingTaskVisualCountdown(void);
void updatePlayerVisualPairTask(void);
void updatePlayerVisualPairTask5E80(void);
void updatePlayerVisualPairTask6780(void);


void drawSlidingRaceTimeDisplay(SlidingTimeResource *resource);
void func_800497B8(void);
void updateFallingPairedVisualTask(void);
s32 calculateRaceTimeDifference(const RaceTime *a, const RaceTime *b, RaceTime *difference);

void drawRaceTime(const RaceTime *value, s32 x, s32 y, s16 mode, GsOT *ot, u16 depth);

/* Only the first byte of this shared state is used by this task flow. */
extern u8 D_80120F90[];
extern const char D_80010A94[];
extern const char gAutoPilotLabel[] __attribute__((section(".rodata.auto_pilot_label"), aligned(4)));

extern const char gCompleteBonusLabel[20] __attribute__((section(".rodata.complete_bonus_label"), aligned(4)));

extern const char gTotalMoneyLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4)));

extern const char gTrickPrizeLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4)));

extern const char gMakeBonusLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4)));

extern const char gResultRankPrizeLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4)));

extern const char gResultGetCoinLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4)));

extern const char gResultTotalMoneyLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4)));

extern u8 D_80124284[];
extern const char gResultValueFormat[];
extern const char gResultValueMultiplierLabel[];
extern const char gPlayerNumberSuffixLabel[];
extern const char gComputerPlayerLabel[];
extern const char D_8013B138[];
extern RaceTime gRaceElapsedTime;


extern int rand(void);

void uploadTimImageAt(s16 x, s16 y, s16 clutX, s16 clutY, u32 *tim);
s16 func_8004A124(s32 x1, s32 z1, s32 x2, s32 z2);
u8 func_8004AAA0(s16 index, s32 x, s32 z);
void func_8003EE70(void);
void func_8002F5F8(void);
void func_8002F548(void);
void func_8002F65C(void);
void func_8003DEE0(void);
void drawSound601SequenceSpriteTask(void);
void drawGrowingSprite178Task(void);
void func_80033C2C(void);
void updateOneShotTaskVisual(void);
void drawSpriteMotionDisplay(SpriteMotionResource *resource);
void slideInSpriteMotionTask(void);
void waitSpriteExitMotionTask(void);
void slideInSlidingDisplayTask(void);
void func_8001816C(void);
void SetFogNearFar(s32 near, s32 far, s32 h);
void queueMode11TransitionResources(void);
void dispatchMode11TransitionTask(void);
void func_80030108(void);
void initializeBackgroundCameraTask(void);
void func_800302F8(void);
void updateRepeatedSoundTask(void);
void waitUnlockNotification0TransitionTask(void);
void func_80032F3C(void);
void queueBackgroundTextureSectorCommands(void);
void func_8002FA58(s16 resource);
void initializePlayerNumberDisplayTask(void);
void slideOutPlayerNumberDisplayTask(void);
void func_80034664(PlayerNumberResource *resource);
void slideOutSlidingNumberDisplayTask(void);
void initializeSlidingNumberDisplayTask(void);
void slideInSlidingNumberDisplayTask(void);
void drawSlidingNumberDisplay(SlidingNumberResource *resource);
void advancePlayerSpriteAnimationTask(void);
void drawPlayerSpriteAnimation(SpriteAnimationResource *resource);
s16 func_8003D904(u16 player);
void slideOutSlidingDisplayTask(void);
void drawSlidingDisplaySprite(SlidingDisplayResource *resource);
void slideRaceTimeDisplayOut(void);
void func_80033100(void);
void updatePlayerScaledObjectTask(void);
void updateRisingPlayerEffectTask(void);
void func_80049404(void);
void func_8004531C(void);
void updatePlayerFlagTaskVisual(void);
void drawPlayerFlagTaskVisual(TaskVisualResource *resource);
void func_80044BC0(void);
void func_800450BC(void);
void func_80046044(void);
void updateDualResource4C80EffectTask(void);
void func_80047D94(void);
void updatePlayerDualResourceEffectTask(void);
void updatePlayerResultOrder(TaskFadeResource *resource);
void func_80036BA4(TaskResourceRecord *resource);
void waitPlayerResultsInputTask(void);
void drawPlayerResultRowValues(s16 row, s16 x, s16 y, TaskFadeResource *resource);
void drawPlayerResultRows(TaskFadeResource *resource);
void fadeInPlayerResultsTask(void);
void fadeInLapTimeLabelTask(void);
void fadeInTimeMenuTask(void);
void drawTaskTimeMenuAndSetFlag10OnInput(void);
void waitTaskTimeRecordCountdownAndPlaySound(void);
void updateTaskTimeRecordCountdownPhase1AndPlaySound(void);
void func_8003A158(void);
void drawTaskTimeRecordsAndSetFlag10OnInput(void);
void drawCurrentTaskTimeRecordDetails(void);
void func_80036454(void);
void waitResultDisplayInputTask(void);
void waitResultNumberDisplayTask(void);
void waitResultDisplayConfirmationTask(void);
void waitResultDisplayTask(void);
void func_8002BC2C(void);
void func_8002B820(void);
void drawResultDisplayLabels(ResultDisplayResource *resource);
void func_800401DC(s16 x, s16 y, s16 kind, GsOT *ot, u16 depth);
void drawResultPrizeLabels(ResultDisplayResource *resource);
void drawFirstPlayerResultCounterAndIcon(ResultDisplayResource *resource);
void drawMenuTimeWithBrightness(MenuTimeResource *resource);
void drawTimeRecordRows(MenuTimeResource *resource);
void drawLapTimeLabel(TaskMenuFadeResource *resource);
void func_80038090(TaskResourceRecord *resource);
void func_80038270(TaskResourceRecord *resource);
void func_80038C70(void);
void drawCountRecordRows(TaskResourceRecord *resource);
void drawHitPrizeMoneyLabels(MenuFadeInResource *resource);
void tickMenuSoundDelayStage0Task(void);
void waitTaskMenuCountdownAndPlaySound(void);
void drawTaskMenuAndSetFlag10OnInput(void);
void func_80038B34(void);
void startTaskMenuCountdownAndPlaySound(void);
void func_8003BE58(TaskResourceRecord *resource);
void updatePanelExitInputTask(void);
void advanceCurrentTaskPanelTimer(void);
void tickRaceResultsDelayTask(void);
void tickRaceResultsSoundDelayTask(void);
void drawTimeRecordHeader(TimeRecordHeaderResource *resource);
void func_80039564(TaskResourceRecord *resource);
void func_80039B34(TaskResourceRecord *resource);
void func_800398F4(TaskResourceRecord *resource);
void func_8003AAE0(TaskResourceRecord *resource);
void func_8003AD48(TaskResourceRecord *resource);
void drawStatsRecordRows(TaskResourceRecord *resource);
void drawStatsMenuMoneyLabels(StatsMenuFadeResource *resource);
void waitForStatsScoreCountStartTask(void);
void advanceTaskStatsCountdownAndPlaySound(void);
void resetTaskStatsCountdown(void);
void countStatsMenuMoneyTask(void);
void drawCurrentTaskStatsSelection(void);
void waitTaskStatsCountdownAndPlaySound(void);
void waitResultFlagsInputTask(void);

void initializeMenuTransitionResourcesTask(void);
void waitMode2TransitionTask(void);

void initializeRaceTimeComparisonPanel(void);
void updateFallingRotatingObjectTask(void);
