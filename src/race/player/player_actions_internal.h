#pragma once

/* Original unit addressing policy; include before public headers. */
#include "../race_internal.h"

extern u16 gPlayerSoundVariantsA[2] __attribute__((section(".data.player_sound_variants")));

extern u16 gPlayerSoundVariantsB[2] __attribute__((section(".data.player_sound_variants")));

extern const char gDemonstrationText[16]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4)));

extern const char gDemonstrationStartText[12]
    __attribute__((section(".rodata.demonstration_overlay"), aligned(4)));

extern TaskVisualFrame gPlayerFlagVisualFrames[6]
    __attribute__((section(".data.player_flag_visual_frames"), aligned(4)));

extern TaskVisualFrame gPlayerSideVisualFrames[4]
    __attribute__((section(".data.player_side_visual_frames"), aligned(4)));

extern GsRVIEW2 gPlayerCameraViews[2]
    __attribute__((section(".data.player_camera_views")));

extern RaceTime D_8013B154
    __attribute__((section(".data.menu_sprite_palettes_prefix"), aligned(4)));

extern MenuSpritePaletteGroups gMenuSpritePaletteGroups
    __attribute__((section(".data.menu_sprite_palettes"), aligned(4)));

extern RaceTime D_80124248[10]
    __attribute__((section(".data.course_hit_time_limits"), aligned(4)));

extern u8 D_8012429C[12]
    __attribute__((section(".data.course_count_targets"), aligned(4)));

extern u8 D_801242A8[12]
    __attribute__((section(".data.course_hit_targets"), aligned(4)));

/* Each input sequence ends with 0xFFFF. */
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

extern TaskVisualFrame gFallingPairedVisualFrames[7]
    __attribute__((section(".data.falling_paired_visual_frames"), aligned(4)));

extern u8 gFallingPairedVisualAnimation[6]
    __attribute__((section(".data.falling_paired_visual_animation"), aligned(2)));

extern TaskVisualFrame gPlayerDualResourceEffectFrame
    __attribute__((section(".data.player_dual_resource_effect_frame"), aligned(4)));

extern TaskVisualFrame gPlayerVisualPairFrame
    __attribute__((section(".data.player_visual_pair_frame"), aligned(4)));

extern TaskVisualFrame gPlayerVisualPair6780Frame
    __attribute__((section(".data.player_visual_pair_6780_frame"), aligned(4)));

extern TaskVisualFrame gPlayerVisualPair5E80Frame
    __attribute__((section(".data.player_visual_pair_5e80_frame"), aligned(4)));

extern s16 D_8013B1D4 __attribute__((section(".data.finish_slot_state")));

extern const char gAutoPilotLabel[] __attribute__((section(".rodata.auto_pilot_label"), aligned(4)));

extern const char gCompleteBonusLabel[20] __attribute__((section(".rodata.complete_bonus_label"), aligned(4)));

extern const char gTotalMoneyLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4)));

extern const char gTrickPrizeLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4)));

extern const char gMakeBonusLabel[16] __attribute__((section(".rodata.money_labels"), aligned(4)));

extern const char gResultRankPrizeLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4)));

extern const char gResultGetCoinLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4)));

extern const char gResultTotalMoneyLabel[16] __attribute__((section(".rodata.result_prize_labels"), aligned(4)));

extern const char gResultValueFormat[4] __attribute__((section(".data.result_row_labels"), aligned(4)));
extern const char gResultValueMultiplierLabel[4] __attribute__((section(".data.result_row_labels"), aligned(4)));
extern const char gPlayerNumberSuffixLabel[4] __attribute__((section(".data.result_row_labels"), aligned(4)));
extern const char gComputerPlayerLabel[4] __attribute__((section(".data.result_row_labels"), aligned(4)));

extern RaceTime gRaceElapsedTime;

u16 arePositionsWithinXZRange(const Position3D *a, const Position3D *b, s32 threshold);
u16 isPositionWithinPlayerCollisionRange(const Position3D *position, s32 radius, s32 height, s16 playerIndex);
u16 addPlayerFlagsIfWithinRadius(Position3D *position, s32 radius, s16 flags, u16 playerIndex);
void addPlayerFlagsWithinRadius(const Position3D *position, s32 radius, u16 flags);
void updateRacePlayerFinishSlotsTask(void);
s32 requestCurrentPlayerAction6(void);
s32 requestCurrentPlayerAction6NarrowAngle(void);
s32 requestCurrentPlayerAction7(void);
s32 checkCurrentPlayerFlags43040OrAngleRange(void);
s32 checkCurrentPlayerFlags345001(void);
s32 requestCurrentPlayerAction17(void);
s32 checkCurrentPlayerFlags275000OrField134(void);
s16 checkCurrentPlayerSurfaceType(void);
s16 countCurrentPlayerInputGroups(void);
void clearCurrentPlayerTaskState(void);
void updateCurrentPlayerDirectionalAnimation(void);
void func_8002E3C0(void);
void addPlayerCappedCounters(u16 playerIndex, s32 amount);
void playCurrentPlayerSoundEffect(s16 index);
void startPlayerTimerTask(RacePlayer *player);
void loadCurrentPlayerModelBlock(void);
void resetGameOptionState(void);
void queueBackgroundTextureSectorCommands(void);
void uploadBufferedTexturePair(void);
void uploadTextureArchiveEntry(s16 x, s16 y, s16 clutX, s16 clutY, s32 index);
void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map);
void queueStartupSectorReads(void);
void uploadTimImageAt(s16 x, s16 y, s16 clutX, s16 clutY, u32 *tim);
void initializeBackgroundCameraTask(void);
void initializeViewAndSelectTaskCallback(void);
void beginMode2TaskSequence(void);
void beginMode2SequenceAndFadeMusic(void);
void queueMode11TransitionResources(void);
