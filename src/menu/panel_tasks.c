#include "panel_tasks_internal.h"
#include "game/menu/task_widgets.h"
#include "game/menu/selected_sprite.h"
#include "game/engine/game_task_scheduler.h"
#include "grouped_selection_internal.h"
#include "game/audio/music.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"
#include "game/race/player/player_resource_textures.h"
#include "game/save/save_data.h"
#include "psx/libetc.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

#define GS_MODEL_OBJECT_HIDDEN 0x80000000u
#define TASK_POSITION(task) ((Movement2D *)&gTaskMemoryPool[(task)->resource14])

#define CHILDREN_READY(entries) (!*(s16 *)&(entries)[0].flags && !*(s16 *)&(entries)[1].flags && \
                                 !*(s16 *)&(entries)[2].flags && !*(s16 *)&(entries)[3].flags && \
                                 !*(s16 *)&(entries)[4].flags)
#define CHILD_MOVEMENT(entry) ((Movement2D *)&gTaskMemoryPool[(entry).task->resource14])
#define MODEL_PREVIEW_MOVEMENT(entry) ((Movement2D *)&gTaskMemoryPool[(entry).task->resource16])
#define BOARD_SHOP_PANELS_READY(panels) \
    (!(panels)[0].flags && !(panels)[1].flags && !(panels)[2].flags && !(panels)[3].flags)
#define BOARD_PURCHASE_CHILDREN_READY(entries) \
    ((s16)(entries)[0].flags == 0 && (s16)(entries)[1].flags == 0 && \
     (s16)(entries)[2].flags == 0 && (s16)(entries)[4].flags == 0)


typedef struct GroupedSelectionDescriptor {
    const u8 *mapping;
    u8 spacingX;
    u8 spacingY;
    u8 count;
    u8 unk07;
    const u8 *label;
    u32 unk0C;
    u8 *tripletIndices;
} GroupedSelectionDescriptor;

typedef struct GridModelPreviewState {
    s16 unk00;
    s16 rotationStep;
    s16 unk04;
    s16 unk06;
} GridModelPreviewState;

typedef struct MoneyMenuSelection {
    s16 mode;
    s16 selectedIndex;
    s16 previousIndex;
    s16 choiceResult;
} MoneyMenuSelection;

enum BoardShopChoice {
    BOARD_SHOP_BUY,
    BOARD_SHOP_PAINT,
    BOARD_SHOP_LEAVE
};

enum PlayerTextureMenuItem {
    TEXTURE_MENU_NEXT_PAGE,
    TEXTURE_MENU_PREVIOUS_PAGE,
    TEXTURE_MENU_DONE,
    TEXTURE_MENU_FIRST_PAINT
};

extern MenuRgbColor gMenuColorCycleColor __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

static const u32 sBoardShopModeJumpTablePadding
    __attribute__((section(".rodata.board_shop_mode_padding"))) = 0;

static const u32 sBoardSelectionDetailsJumpTablePadding
    __attribute__((section(".rodata.board_selection_details_padding"))) = 0;

extern const GridSpriteEntry gBoardMenuSprites[18];
extern GroupedSelectionDescriptor gBoardMenuSelectionDescriptors[3];
extern volatile u8 shopMenuRememberedSelectionGp;
extern const GridSpriteEntry gMoneyPanelSelectionSprites[3];
extern const u8 gMoneyPanelSelectionTextMapping[4];

extern char gShopCurrencySuffix[];

extern const s16 gGridModelPreviewPose0And2[54], gSharedGridModelPreviewPose[54], gGridModelPreviewPose4[54];
extern const s16 gGridModelPreviewPose8[54], gGridModelPreviewPose9[54], gGridModelPreviewPose7[54];

static const GridSpriteEntry sShopMenuSprites[4]
    __attribute__((section(".data.shop_menu_sprites"), aligned(4))) = {
    {0xD6, 0, 0, 0},
    {0xD5, 0, 0, 0},
    {0xD7, 0, 0, 0},
    {0xC6, 0, 0, 0},
};

const u16 gShopBoardMenuDescriptionText[46]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x0092, 0x00A1, 0x00B8, 0x009D, 0x0093, 0x004E, 0x005A,
    0x002C, 0x0083, 0x002C, 0x007E, 0x00BE, 0x00EA, 0x008E, 0x00A1,
    0x00B9, 0xC007, 0x0038, 0xC006, 0x0083, 0x002C, 0x007E, 0x00AA,
    0x007D, 0x0075, 0x0043, 0x006F, 0x00BE, 0x00AE, 0x00BF, 0x009B,
    0x0094, 0x009E, 0x00BA, 0xC007, 0x009B, 0x00A5, 0x00C0, 0x00CD,
    0x0098, 0x00B0, 0x009E, 0x0033, 0x0000, 0x0000,
};

const u16 gShopCharacterBoardMenuDescriptionText[50]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x0092, 0x00A1, 0x00B8, 0x009D, 0x0093, 0x004E, 0x005A,
    0x002C, 0x0083, 0x002C, 0x007E, 0x0100, 0x00AA, 0x0044, 0x003C,
    0x0042, 0x00BE, 0x00EA, 0x008E, 0x00A1, 0x00B9, 0xC007, 0x0038,
    0xC006, 0x00EA, 0x008E, 0x00A1, 0x0044, 0x003C, 0x0042, 0x00A7,
    0x0118, 0x00C0, 0x0095, 0x00A1, 0x00B9, 0x009E, 0x00BA, 0xC007,
    0x009B, 0x00A5, 0x00C0, 0x00CD, 0x0098, 0x00B0, 0x009E, 0x0033,
    0x0000, 0x0000,
};

const u16 gShopPaletteMenuDescriptionText[38]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0048, 0x003F, 0x0068, 0x0049, 0x0051, 0x002C, 0x00AA,
    0xC006, 0x0097, 0x00B1, 0x00AA, 0x011A, 0xC007, 0x00BE, 0x800A,
    0xC006, 0x0006, 0x0001, 0x0001, 0x0011, 0x00CD, 0x00AE, 0x00BF,
    0x009B, 0x0094, 0x009E, 0x00BA, 0xC007, 0x009B, 0x00A5, 0x00C0,
    0x00CD, 0x0098, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gShopExitDescriptionText[22]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0096, 0x00E4, 0x00BE, 0x00CD, 0x00A4, 0x0038, 0x0073,
    0x002C, 0x0062, 0x0063, 0x0057, 0x0040, 0x002C, 0x00A7, 0x00B4,
    0x00CE, 0x00B9, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gShopSubmenuReturnDescriptionText[12]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0096, 0x00E4, 0x00A7, 0x00B4, 0x00CE, 0x00B9, 0x00B0,
    0x009E, 0x0033, 0x0000, 0x0000,
};

const u16 gShopGreetingText[12]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0034, 0x0093, 0x00B8, 0x008E, 0x009D, 0x008F, 0x0093,
    0x0025, 0x0035, 0x0000, 0x0000,
};

const u16 gShopInsufficientFundsText[14]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0034, 0x0096, 0x00FC, 0x00C0, 0x00A1, 0x00B9, 0x00A6,
    0x0093, 0x00B7, 0x0033, 0x0035, 0x0000, 0x0000,
};

const u16 gShopBoardAlreadyOwnedText[16]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0034, 0x00A0, 0x00BB, 0x00AB, 0x0094, 0x00B9, 0x0098,
    0x00BB, 0x00A6, 0x00BF, 0x00CA, 0x0033, 0x0035, 0x0000, 0x0000,
};

const u16 gShopBoardBuyDescriptionText[40]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x0092, 0x00A1, 0x00B8, 0x009D, 0x0093, 0x0083, 0x002C,
    0x007E, 0x00BE, 0x00EA, 0x0093, 0x00B0, 0x009E, 0x0033, 0xC007,
    0x009E, 0x00CD, 0x00A7, 0x00B4, 0x008E, 0x00A4, 0x0093, 0x00BA,
    0x0083, 0x002C, 0x007E, 0x00AB, 0x00EA, 0x0094, 0x009B, 0x00A5,
    0x00C0, 0x00CD, 0x0098, 0x00B0, 0x009F, 0x00BF, 0x0033, 0x0000,
};

const u16 gShopBoardPaintDescriptionText[38]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x0083, 0x002C, 0x007E, 0x00AA, 0x0087, 0x0043, 0x006F,
    0x0055, 0x00BE, 0x0097, 0x0095, 0x00B0, 0x009E, 0x0033, 0xC007,
    0x00E3, 0x00E4, 0x00E5, 0x00AA, 0x004C, 0x002C, 0x0080, 0x004E,
    0x00A6, 0x00AA, 0x00CD, 0x0096, 0x00FC, 0x00AB, 0x0097, 0x0097,
    0x00B9, 0x00B0, 0x009F, 0x00BF, 0x0033, 0x0000,
};

const u16 gShopNewBoardStockGreetingText[32]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0034, 0xC006, 0x0096, 0x00B4, 0x009D, 0x00BC, 0x0093,
    0x0083, 0x002C, 0x007E, 0xC007, 0x00C0, 0x00AB, 0x0093, 0x008E,
    0x00A1, 0x00B7, 0x0033, 0x00EA, 0x008E, 0x00A4, 0x0093, 0x0097,
    0x00A6, 0x0093, 0x0097, 0x0093, 0x0031, 0x0035, 0x0000, 0x0000,
};

const u16 gShopCharacterBoardBuyDescriptionText[22]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x0092, 0x00A1, 0x00B8, 0x009D, 0x0093, 0x0044, 0x003C,
    0x0042, 0x00BE, 0x00EA, 0x0094, 0xC007, 0x009B, 0x00A5, 0x00C0,
    0x00CD, 0x0098, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gShopCharacterBoardEquipDescriptionText[32]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x00EA, 0x008E, 0x00A1, 0x0044, 0x003C, 0x0042, 0x00AA,
    0x00DD, 0x0097, 0x00B8, 0x011B, 0x0098, 0x00A6, 0x0044, 0x003C,
    0x0042, 0x00A7, 0x0118, 0x00C0, 0x0095, 0x00BA, 0xC007, 0x009B,
    0x00A5, 0x00C0, 0x00CD, 0x0098, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gShopCharacter8BoardMenuDescriptionText[28]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x00C5, 0x00BF, 0x00A9, 0x00BF, 0x00A6, 0x00C0, 0x00B8,
    0x0038, 0x004D, 0x005A, 0x0080, 0x006F, 0x0100, 0x00AA, 0x0083,
    0x002C, 0x007E, 0x00AB, 0x0096, 0x0093, 0x00A4, 0x00B0, 0x009F,
    0x00BF, 0x0033, 0x0000, 0x0000,
};

const u16 gShopCharacter9BoardMenuDescriptionText[26]
    __attribute__((section(".data.shop_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x00C5, 0x00BF, 0x00A9, 0x00BF, 0x00A6, 0x00C0, 0x00B8,
    0x0038, 0x0047, 0x0045, 0x007D, 0x0100, 0x00AA, 0x0083, 0x002C,
    0x007E, 0x00AB, 0x0096, 0x0093, 0x00A4, 0x00B0, 0x009F, 0x00BF,
    0x0033, 0x0000,
};
static const u16 *sShopMenuTexts[15] __attribute__((section(".data.shop_menu_texts"))) = {
    gShopBoardMenuDescriptionText, gShopCharacterBoardMenuDescriptionText, gShopPaletteMenuDescriptionText, gShopExitDescriptionText, gShopSubmenuReturnDescriptionText,
    gShopGreetingText, gShopInsufficientFundsText, gShopBoardAlreadyOwnedText, gShopBoardBuyDescriptionText, gShopBoardPaintDescriptionText,
    gShopNewBoardStockGreetingText, gShopCharacterBoardBuyDescriptionText, gShopCharacterBoardEquipDescriptionText, gShopCharacter8BoardMenuDescriptionText, gShopCharacter9BoardMenuDescriptionText,
};

static const GridSpriteEntry sBoardShopModeSprites[3]
    __attribute__((section(".data.board_shop_sprites"), aligned(4))) = {
    {0xC4, 0, 0, 0},
    {0xC5, 0, 0, 0},
    {0xC7, 0, 0, 0},
};

static const s8 sShopCharacterTextureIndices[12]
    __attribute__((section(".data.board_shop_sprites"), aligned(4))) = {
    3, 1, 4, 5, 2, 9, 8, 10, 11, 12, 0, 0,
};

static const u8 sBoardShopModeTextMapping[4]
    __attribute__((section(".data.board_shop_mode_text_mapping"), aligned(4))) = {8, 9, 4, 0};
static const u8 sBoardShopModeSpriteMapping[4]
    __attribute__((section(".data.shop_submenu_mappings"), aligned(4))) = {0, 1, 2, 0};
static const u8 sShopSubmenuTextureMapping[4]
    __attribute__((section(".data.shop_submenu_mappings"), aligned(4))) = {0xFF, 0xFF, 6, 0};

static const u8 sShopMenuSpriteMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {0, 1, 2, 3};
static const u8 sShopMenuTextMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {0, 1, 2, 3};
static const u8 sShopMenuCharacter8TextMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {13, 1, 2, 3};
static const u8 sShopMenuCharacter9TextMapping[4]
    __attribute__((section(".data.shop_menu_mappings"), aligned(4))) = {14, 1, 2, 3};

static const u8 sShopMenuParentStates[4]
    __attribute__((section(".data.shop_menu_destinations"), aligned(4))) = {12, 13, 14, 6};
static const u8 sShopMenuTextureMapping[4]
    __attribute__((section(".data.shop_menu_destinations"), aligned(4))) = {7, 7, 7, 6};

/* The remembered choice follows the adjacent three-entry menu's sprite mapping. */
static const u8 sMoneyPanelSelectionSpriteMapping[3]
    __attribute__((section(".data.shop_menu_selection"))) = {0, 1, 2};
volatile u8 gShopMenuRememberedSelection __attribute__((section(".data.shop_menu_selection"))) = 0;

volatile u8 gBoardShopRememberedSelection __attribute__((section(".sdata"))) = 0;
u8 gMoneyPanelRememberedSelection __attribute__((section(".sdata"))) = 0;

static const GridSpriteEntry sPlayerTextureSelectionSprites[19]
    __attribute__((section(".data.player_texture_selection_sprites"), aligned(4))) = {
    {0x10C, 0, 0, 0},
    {0x10D, 0, 0, 0},
    {0x10E, 0, 0, 0},
    {0x10F, 0, 0xFF, 0},
    {0x110, 0, 0xFF, 0},
    {0x111, 0, 0xFF, 0},
    {0x112, 0, 0xFF, 0},
    {0x113, 0, 0xFF, 0},
    {0x114, 0, 0xFF, 0},
    {0x115, 0, 0xFF, 0},
    {0x116, 0, 0xFF, 0},
    {0x117, 0, 0xFF, 0},
    {0x118, 0, 0xFF, 0},
    {0x119, 0, 0xFF, 0},
    {0x11A, 0, 0xFF, 0},
    {0x11B, 0, 0xFF, 0},
    {0x11C, 0, 0xFF, 0},
    {0x11D, 0, 0xFF, 0},
    {0x11E, 0, 0xFF, 0},
};

/* Each page has eight paints, a page turn, and Done, followed by two padding bytes. */
static const u8 sPlayerTextureSelectionPageItems[2][12]
    __attribute__((section(".data.player_texture_selection_page_items"), aligned(4))) = {
    {3, 4, 5, 6, 7, 8, 9, 10, TEXTURE_MENU_NEXT_PAGE, TEXTURE_MENU_DONE},
    {11, 12, 13, 14, 15, 16, 17, 18, TEXTURE_MENU_PREVIOUS_PAGE, TEXTURE_MENU_DONE},
};

static const u8 *const sPlayerTextureSelectionPages[2]
    __attribute__((section(".data.player_texture_selection_pages"))) = {
    sPlayerTextureSelectionPageItems[0], sPlayerTextureSelectionPageItems[1],
};

static RecordFlagMenuEntries sShopDefaultBoardEntries
    __attribute__((section(".data.shop_default_board_entries"))) = {
    gDefaultBoardMenuItems, 0, 20, 3, NULL, 0, 0
};

static const u8 sShopBoardGroupUnlockFlags[4]
    __attribute__((section(".data.shop_board_group_unlock_flags"), aligned(4))) = {1, 0, 2, 0};

static const u8 sBoardPurchaseSpriteMapping[4]
    __attribute__((section(".data.board_purchase_mappings"), aligned(4))) = {1, 2, 3, 5};
static const u8 sBoardPurchaseFullSpriteMapping[8]
    __attribute__((section(".data.board_purchase_mappings"), aligned(4))) = {1, 2, 3, 4, 5, 0, 0, 0};

const s16 *gGridModelPreviewPoses[10]
    __attribute__((section(".data.grid_model_preview_poses"))) = {
    gGridModelPreviewPose0And2, gSharedGridModelPreviewPose, gGridModelPreviewPose0And2, gSharedGridModelPreviewPose, gGridModelPreviewPose4,
    gSharedGridModelPreviewPose, gSharedGridModelPreviewPose, gGridModelPreviewPose7, gGridModelPreviewPose8, gGridModelPreviewPose9
};

PanelModeLabel gRegisteredPanelMode0Label __attribute__((section(".data.registered_panel_mode_labels"))) = {2, 0};
PanelModeLabel gRegisteredPanelMode1Label __attribute__((section(".data.registered_panel_mode_labels"))) = {3, 0};

u8 gBoardSelectionLevels[12] __attribute__((section(".data.board_selection_levels"))) = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 1, 2, 3
};

const u8 gBoardStyleFreestyleText[16]
    __attribute__((section(".rodata.board_style_text"), aligned(4))) = {
    0x83, 0x74, 0x83, 0x8A, 0x81, 0x7C, 0x83, 0x58, 0x83, 0x5E, 0x83, 0x43, 0x83, 0x8B, 0, 0
};

const u8 gBoardStyleAllRoundText[16]
    __attribute__((section(".rodata.board_style_text"), aligned(4))) = {
    0x83, 0x49, 0x81, 0x7C, 0x83, 0x8B, 0x83, 0x89, 0x83, 0x45, 0x83, 0x93, 0x83, 0x68, 0, 0
};

const u8 gBoardStyleAlpineText[12]
    __attribute__((section(".rodata.board_style_text"), aligned(4))) = {
    0x83, 0x41, 0x83, 0x8B, 0x83, 0x79, 0x83, 0x93, 0, 0, 0, 0
};

const u8 gBoardStyleSpecialText[12]
    __attribute__((section(".rodata.board_style_text"), aligned(4))) = {
    0x83, 0x58, 0x83, 0x79, 0x83, 0x56, 0x83, 0x83, 0x83, 0x8B, 0, 0
};

s8 *gBoardStyleText[4] __attribute__((section(".data.board_style_text"))) = {
    (s8 *)gBoardStyleFreestyleText, (s8 *)gBoardStyleAllRoundText,
    (s8 *)gBoardStyleAlpineText, (s8 *)gBoardStyleSpecialText
};

u8 gBoardSelectionStyles[12] __attribute__((section(".data.board_selection_styles"))) = {
    0, 1, 2, 0, 1, 2, 0, 1, 2, 3, 3, 3
};

const u8 gSelectedBoardLevelLabel[12] __attribute__((section(".rodata.selected_board_details"), aligned(4))) = {
    0x82, 0x6B, 0x82, 0x64, 0x82, 0x75, 0x82, 0x64, 0x82, 0x6B, 0, 0
};

const u8 gSelectedBoardPricePlaceholder[12] __attribute__((section(".rodata.selected_board_details"), aligned(4))) = {
    0x81, 0x7C, 0x81, 0x7C, 0x81, 0x7C, 0x81, 0x7C, 0x81, 0x7C, 0, 0
};

const u32 gGroupedSelectionBoundsTablePadding
    __attribute__((section(".rodata.grouped_selection_bounds_padding"))) = 0;

const u8 gBoardLevelLabel[16] __attribute__((section(".rodata.board_level_label"), aligned(4))) = {
    0x82, 0x6B, 0x82, 0x64, 0x82, 0x75, 0x82, 0x64,
    0x82, 0x6B, 0, 0, 0, 0, 0, 0
};

/* Read the encoded byte before converting it to a signed screen offset. */
static volatile u8 sBoardPriceVerticalOffsets[12]
    __attribute__((section(".data.board_price_vertical_offsets"), aligned(4))) = {
    0xF8, 0xF8, 0xF6, 0xF8, 0xE8, 0xF6, 0xF6, 0xEC, 0xFC, 0xF9, 0, 0
};

u16 gCurrentTaskPanelText[8] __attribute__((section(".data.current_task_panel_text"))) = {
    0xFA, 0x94, 0xB1, 0x9F, 0x98, 0x31, 0, 0
};

const u16 gTaskSelectionPanelLabel[6] __attribute__((section(".data.task_selection_panel_text"), aligned(4))) = {
    0x18, 0x157, 0x2D, 0x8020, 0xBF, 0
};

const u16 gTaskSelectionPanelText[8] __attribute__((section(".data.task_selection_panel_text"), aligned(4))) = {
    0xFA, 0x94, 0xB1, 0x9E, 0xA2, 0x33, 0, 0
};

ResourceSectorRange gGridResourceSecondSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges"))) = {
    {
        {DAT_GRID_TEXTURE_0_0_SECTOR, DAT_GRID_TEXTURE_0_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_1_SECTOR, DAT_GRID_TEXTURE_0_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_2_SECTOR, DAT_GRID_TEXTURE_0_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_3_SECTOR, DAT_GRID_TEXTURE_0_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_0_4_SECTOR, DAT_GRID_TEXTURE_0_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_1_0_SECTOR, DAT_GRID_TEXTURE_1_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_1_SECTOR, DAT_GRID_TEXTURE_1_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_2_SECTOR, DAT_GRID_TEXTURE_1_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_3_SECTOR, DAT_GRID_TEXTURE_1_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_1_4_SECTOR, DAT_GRID_TEXTURE_1_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_2_0_SECTOR, DAT_GRID_TEXTURE_2_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_1_SECTOR, DAT_GRID_TEXTURE_2_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_2_SECTOR, DAT_GRID_TEXTURE_2_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_3_SECTOR, DAT_GRID_TEXTURE_2_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_2_4_SECTOR, DAT_GRID_TEXTURE_2_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_3_0_SECTOR, DAT_GRID_TEXTURE_3_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_1_SECTOR, DAT_GRID_TEXTURE_3_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_2_SECTOR, DAT_GRID_TEXTURE_3_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_3_SECTOR, DAT_GRID_TEXTURE_3_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_3_4_SECTOR, DAT_GRID_TEXTURE_3_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_4_0_SECTOR, DAT_GRID_TEXTURE_4_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_1_SECTOR, DAT_GRID_TEXTURE_4_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_2_SECTOR, DAT_GRID_TEXTURE_4_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_3_SECTOR, DAT_GRID_TEXTURE_4_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_4_4_SECTOR, DAT_GRID_TEXTURE_4_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_5_0_SECTOR, DAT_GRID_TEXTURE_5_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_1_SECTOR, DAT_GRID_TEXTURE_5_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_2_SECTOR, DAT_GRID_TEXTURE_5_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_3_SECTOR, DAT_GRID_TEXTURE_5_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_5_4_SECTOR, DAT_GRID_TEXTURE_5_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_6_0_SECTOR, DAT_GRID_TEXTURE_6_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_1_SECTOR, DAT_GRID_TEXTURE_6_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_2_SECTOR, DAT_GRID_TEXTURE_6_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_3_SECTOR, DAT_GRID_TEXTURE_6_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_6_4_SECTOR, DAT_GRID_TEXTURE_6_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_7_0_SECTOR, DAT_GRID_TEXTURE_7_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_1_SECTOR, DAT_GRID_TEXTURE_7_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_2_SECTOR, DAT_GRID_TEXTURE_7_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_3_SECTOR, DAT_GRID_TEXTURE_7_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_7_4_SECTOR, DAT_GRID_TEXTURE_7_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_8_0_SECTOR, DAT_GRID_TEXTURE_8_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_1_SECTOR, DAT_GRID_TEXTURE_8_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_2_SECTOR, DAT_GRID_TEXTURE_8_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_3_SECTOR, DAT_GRID_TEXTURE_8_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_8_4_SECTOR, DAT_GRID_TEXTURE_8_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_TEXTURE_9_0_SECTOR, DAT_GRID_TEXTURE_9_0_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_1_SECTOR, DAT_GRID_TEXTURE_9_1_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_2_SECTOR, DAT_GRID_TEXTURE_9_2_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_3_SECTOR, DAT_GRID_TEXTURE_9_3_SECTOR_COUNT},
        {DAT_GRID_TEXTURE_9_4_SECTOR, DAT_GRID_TEXTURE_9_4_SECTOR_COUNT},
    }
};

ResourceSectorRange gGridResourceFirstSectorRanges[10][5] __attribute__((section(".data.grid_resource_sector_ranges"))) = {
    {
        {DAT_GRID_MODEL_0_0_SECTOR, DAT_GRID_MODEL_0_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_1_SECTOR, DAT_GRID_MODEL_0_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_2_SECTOR, DAT_GRID_MODEL_0_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_3_SECTOR, DAT_GRID_MODEL_0_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_0_4_SECTOR, DAT_GRID_MODEL_0_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_1_0_SECTOR, DAT_GRID_MODEL_1_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_1_SECTOR, DAT_GRID_MODEL_1_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_2_SECTOR, DAT_GRID_MODEL_1_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_3_SECTOR, DAT_GRID_MODEL_1_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_1_4_SECTOR, DAT_GRID_MODEL_1_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_2_0_SECTOR, DAT_GRID_MODEL_2_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_1_SECTOR, DAT_GRID_MODEL_2_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_2_SECTOR, DAT_GRID_MODEL_2_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_3_SECTOR, DAT_GRID_MODEL_2_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_2_4_SECTOR, DAT_GRID_MODEL_2_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_3_0_SECTOR, DAT_GRID_MODEL_3_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_1_SECTOR, DAT_GRID_MODEL_3_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_2_SECTOR, DAT_GRID_MODEL_3_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_3_SECTOR, DAT_GRID_MODEL_3_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_3_4_SECTOR, DAT_GRID_MODEL_3_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_4_0_SECTOR, DAT_GRID_MODEL_4_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_1_SECTOR, DAT_GRID_MODEL_4_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_2_SECTOR, DAT_GRID_MODEL_4_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_3_SECTOR, DAT_GRID_MODEL_4_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_4_4_SECTOR, DAT_GRID_MODEL_4_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_5_0_SECTOR, DAT_GRID_MODEL_5_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_1_SECTOR, DAT_GRID_MODEL_5_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_2_SECTOR, DAT_GRID_MODEL_5_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_3_SECTOR, DAT_GRID_MODEL_5_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_5_4_SECTOR, DAT_GRID_MODEL_5_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_6_0_SECTOR, DAT_GRID_MODEL_6_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_1_SECTOR, DAT_GRID_MODEL_6_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_2_SECTOR, DAT_GRID_MODEL_6_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_3_SECTOR, DAT_GRID_MODEL_6_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_6_4_SECTOR, DAT_GRID_MODEL_6_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_7_0_SECTOR, DAT_GRID_MODEL_7_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_1_SECTOR, DAT_GRID_MODEL_7_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_2_SECTOR, DAT_GRID_MODEL_7_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_3_SECTOR, DAT_GRID_MODEL_7_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_7_4_SECTOR, DAT_GRID_MODEL_7_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_8_0_SECTOR, DAT_GRID_MODEL_8_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_1_SECTOR, DAT_GRID_MODEL_8_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_2_SECTOR, DAT_GRID_MODEL_8_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_3_SECTOR, DAT_GRID_MODEL_8_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_8_4_SECTOR, DAT_GRID_MODEL_8_4_SECTOR_COUNT},
    },
    {
        {DAT_GRID_MODEL_9_0_SECTOR, DAT_GRID_MODEL_9_0_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_1_SECTOR, DAT_GRID_MODEL_9_1_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_2_SECTOR, DAT_GRID_MODEL_9_2_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_3_SECTOR, DAT_GRID_MODEL_9_3_SECTOR_COUNT},
        {DAT_GRID_MODEL_9_4_SECTOR, DAT_GRID_MODEL_9_4_SECTOR_COUNT},
    }
};

POLY_F3 gNumberAdjustmentArrow __attribute__((section(".data.number_adjustment_arrow"))) = {
    4U << 24, 0, 0, 0, POLY_F3_CODE, 0, 0, 0, 0, 0, 0
};

static inline RegisteredTaskEntry *getRegisteredPanelEntry(u32 resource, s32 index) {
    RegisteredTaskEntry *entries = (RegisteredTaskEntry *)&gTaskMemoryPool[resource];
    return &entries[index / 10];
}

void createRegisteredStaticTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                         const u16 *text, s32 mode, s32 depth, s32 index) {
    RegisteredTextPanelResource *resource;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(drawRegisteredTextPanelTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (RegisteredTextPanelResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->index = index;
    resource->depth = depth;
    resource->mode = mode;
    resource->x = x;
    resource->y = y;
    resource->textX = textX;
    resource->textY = textY;
    resource->text = text;
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTask->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = gCreatedTask;
}

void drawRegisteredTextPanelTask(void) {
    RegisteredTextPanelResource *state;
    RegisteredTaskEntry *entry;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    state = (RegisteredTextPanelResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (state->index % 10 == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTaskParent->resource14, state->index);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTaskParent->resource16, state->index);
    }
    if (entry->task != gCurrentTask || entry->command == 99) {
        deleteCurrentTaskSubtree();
    } else {
        setSpriteColor(128, 128, 128);
        drawWrappedGlyphText8x16(state->textX, state->textY, 18, state->text, 7, -1,
                                 &gMainOrderingTables[gActiveGraphicsBuffer], state->depth);
        if (state->mode == 0) {
            drawWrappedGlyphText8x16((s16)(state->textX + 32), state->textY, 18, &gRegisteredPanelMode0Label.glyph, 7, -1,
                                     &gMainOrderingTables[gActiveGraphicsBuffer], state->depth);
        } else if (state->mode == 1) {
            drawWrappedGlyphText8x16((s16)(state->textX + 32), state->textY, 18, &gRegisteredPanelMode1Label.glyph, 7, -1,
                                     &gMainOrderingTables[gActiveGraphicsBuffer], state->depth);
        }
        setTexturedQuadColor(128, 128, 128);
        drawDefaultTexturedPanel(state->x, state->y, 152, 40, 5, 0,
                                &gMainOrderingTables[gActiveGraphicsBuffer], state->depth);
    }
}

void createRegisteredDetailedTextPanelTask(s16 x, s16 y, s16 textX, s16 textY,
                                   const u16 *text, s32 value, s32 color, s32 mode,
                                   const u16 *content, s32 depth, s32 index) {
    RegisteredDetailedPanelResource *resource;
    RegisteredTaskEntry *entry;
    createChildTaskWithReserve(updateRegisteredDetailedTextPanelTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (RegisteredDetailedPanelResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->unk00 = 0;
    resource->unk02 = 0;
    resource->unk04 = 0;
    resource->unk06 = 0;
    resource->unk08 = 0;
    resource->x = x;
    resource->y = y;
    resource->textX = textX;
    resource->textY = textY;
    resource->text = text;
    resource->value = value;
    resource->color = color;
    resource->mode = mode;
    resource->content = content;
    resource->depth = depth;
    resource->index = index;
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = 0;
    gCreatedTask->data1A = 0;
    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTask->resource16, index);
    }
    entry->command = 0;
    entry->flags = 0;
    entry->task = gCreatedTask;
}

void updateRegisteredDetailedTextPanelTask(void) {
    RegisteredDetailedPanelResource *state;
    ValuePanelEntry *children;
    RegisteredTaskEntry *entry;
    s16 result;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    state = (RegisteredDetailedPanelResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    children = (ValuePanelEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (state->index % 10 == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTaskParent->resource14, state->index);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTaskParent->resource16, state->index);
    }

    if (entry->task == gCurrentTask && entry->command != 99) {
        switch (gCurrentTask->data18) {
        case 0:
            createRegisteredStaticTextPanelTask(state->x, state->y, state->textX, state->textY,
                                                state->text, state->value, state->depth + 1, 1);
            if (state->mode == 0) {
                entry->flags = 2;
                gCurrentTask->data18 = 3;
            } else {
                createInputTimedSpriteTask(state->x + 132, state->y + 18,
                                           gMenuInputPromptSequence, 2, (u8)state->color,
                                           0xB0, 0x40, state->depth, 11);
                gCurrentTask->data18 = 1;
            }
            break;
        case 1:
            result = children[1].active;
            if (result > 0) {
                break;
            }
            if (result < 0) {
                entry->flags = 1;
                gCurrentTask->data18 = 4;
            } else if (state->content == NULL) {
                entry->flags = 2;
                gCurrentTask->data18 = 4;
            } else {
                createRegisteredMovingPanelTask(state->color, (const s16 *)state->content,
                                                state->x + 28, state->y + 23, 0, 16,
                                                state->x + 28, state->y + 39,
                                                state->depth + 2, 21);
                gCurrentTask->data18 = 2;
            }
            break;
        case 2:
            result = children[2].active;
            if (result >= 0) {
                if (result == 0) {
                    entry->flags = 2;
                } else if (result == 1) {
                    entry->flags = 3;
                } else {
                    entry->flags = 4;
                }
                gCurrentTask->data18 = 4;
            }
            break;
        case 3:
            break;
        case 4:
            goto finish;
        }
    } else {
    finish:
        deleteCurrentTaskSubtree();
    }
}

void initializeCountedTaskResourcePair(void) {
    gCurrentTask->data18 = 0;
    gCurrentTaskParent->data1C++;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateShopMenuTask);
    gCurrentTask->unk10 |= 2;
}

void updateShopMenuTask(void) {
    MoneyMenuSelection *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s32 nextState;
    s16 selectedIndex;
    GameTask *parent;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 0: {
        s16 mode = 1;
        setMusicSequenceEnabled(mode);
        selection->mode = 0;
        if (shopMenuRememberedSelectionGp != 0) {
            selection->selectedIndex = shopMenuRememberedSelectionGp - 1;
        } else {
            selection->selectedIndex = shopMenuRememberedSelectionGp;
        }
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)sShopMenuSprites, sShopMenuSpriteMapping,
                                         NULL, 0, 0, 20, -308, -96, 0, 24, 4, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        if (gRacePlayers[0].unk009 == 8) {
            createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sShopMenuCharacter8TextMapping, 4, 11);
        } else if (gRacePlayers[0].unk009 == 9) {
            createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sShopMenuCharacter9TextMapping, 4, 11);
        } else {
            createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sShopMenuTextMapping, 4, 11);
        }
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskMoneyPanel, 4, 21);
        createRegisteredTexturePairTask(172, -72, sShopMenuTextureMapping, 4, selection->selectedIndex, 1, 5, 31);
        gCurrentTask->data18 = 2;
        break;
    }
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (selection->choiceResult == 0 || (s16)entries[0].flags != 0) {
            break;
        }
        if (gSelectionMenuFadeStarted == 0) {
            entries[0].command = 22;
            entries[1].command = 22;
            entries[2].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        if (selection->choiceResult < 0) {
            GameTask *cancelParent = gCurrentTaskParent;
            shopMenuRememberedSelectionGp = 0;
            cancelParent->data18 = 6;
            goto startExit;
        }
        selectedIndex = selection->selectedIndex;
        parent = gCurrentTaskParent;
        nextState = sShopMenuParentStates[selectedIndex];
        parent->data18 = nextState;
        if ((gRacePlayers[0].unk009 == 8 || gRacePlayers[0].unk009 == 9) && nextState == 12) {
            entries[0].command = 21;
            selection->choiceResult = 0;
            break;
        }
        if (gCurrentTaskParent->data18 == 6) {
            shopMenuRememberedSelectionGp = 0;
        } else {
            shopMenuRememberedSelectionGp = selection->selectedIndex + 1;
        }
        gCurrentTask->data18 = 8;
        break;
    case 8:
        entries[0].command = 10;
        gCurrentTask->data18 = 9;
        break;
    case 9:
        if ((s16)entries[0].flags != 0) {
            break;
        }
    startExit:
        gCurrentTask->data18 = 5;
        break;
    case 5:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 6;
        }
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            gCurrentTask->data18 = 7;
        }
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

/* Keep the array incomplete at the call site for absolute address scheduling. */
extern char gMoneyPanelCurrencyLabel[] __attribute__((section(".data")));

void drawCurrentTaskMoneyPanel(void) {
    Movement2D *position;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 32, position->y / 4096 - 22,
                     0xB6, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                             6, 0, gPlayerSaveRecords[0].money, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gMoneyPanelCurrencyLabel, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        if (gCurrentTask->data1A == 1) {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }
}

void initializeAlternateCountedTaskResourcePair(void) {
    gCurrentTask->data18 = 0;
    gCurrentTaskParent->data1C++;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateBoardShopModeMenuTask);
    gCurrentTask->unk10 |= 2;
}

void updateBoardShopModeMenuTask(void) {
    MoneyMenuSelection *selection;
    RegisteredPanelSlot *panels;
    s16 player;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];

    player = 0;
    switch (gCurrentTask->data18) {
    case 0: {
        s16 mode = 1;

        setMusicSequenceEnabled(mode);
        selection->mode = 0;
        if (gBoardShopRememberedSelection) {
            selection->selectedIndex = gBoardShopRememberedSelection - 1;
        } else {
            selection->selectedIndex = gBoardShopRememberedSelection;
        }
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)sBoardShopModeSprites,
                                         sBoardShopModeSpriteMapping, NULL, 0, 0, 20, -308, -80, 0, 24, 3, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, sBoardShopModeTextMapping, 4, 11);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSaveRecordMoneyPanelWithCharacterSprites, 4, 21);
        uploadTextureArchiveEntry(320, 0, 768, 0xE9, sShopCharacterTextureIndices[gRacePlayers[0].unk009]);
        uploadTextureArchiveEntry(320, 256, 768, 0xEA, 0);
        createRegisteredTexturePairTask(172, -72, sShopSubmenuTextureMapping, 3, selection->selectedIndex, 1, 5, 31);
        gCurrentTask->data18 = 2;
        break;
    }
    case 2:
        if (BOARD_SHOP_PANELS_READY(panels)) {
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x80000, -0x50000, 0x1E000, 0, 1);
            configureMovement2D(CHILD_MOVEMENT(panels[1]), -0x8C000, 0x8000, 0x1E000, 0, 4);
            if (!gBoardShopRememberedSelection) {
                s16 index;
                s16 newStock = 0;

                /* Each special board has an unlock bit three places above its purchase bit. */
                for (index = 0; index < 3; index++) {
                    if ((gPlayerSaveRecords[player].flags & (0x800U << (index + 3))) &&
                        !(gPlayerSaveRecords[player].flags & (0x800U << index))) {
                        newStock = 1;
                    }
                }
                if (newStock) {
                    setTaskSecondaryResourceData(panels[1].task, sShopMenuTexts[10]);
                } else {
                    setTaskSecondaryResourceData(panels[1].task, sShopMenuTexts[5]);
                }
            }
            configureMovement2D(CHILD_MOVEMENT(panels[2]), -0x8000, -0x48000, -0x1E000, 0, 4);
            configureMovement2D(CHILD_MOVEMENT(panels[3]), -0x8000, -0x48000, -0x1E000, 0, 4);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (BOARD_SHOP_PANELS_READY(panels)) {
            panels[0].command = 20;
            panels[1].command = 20;
            panels[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (!gBoardShopRememberedSelection) {
            playSoundEffect(0, 2, 32);
        }
        gCurrentTask->data18 = 5;
        break;
    case 5:
        if (isTaskResourceTransitionComplete(0) == 1 && !gSelectionMenuFadeStarted) {
            panels[0].command = 21;
            panels[1].command = 22;
            panels[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = 1;
        }
        if (selection->selectedIndex != selection->previousIndex) {
            setTaskSecondaryResourceData(panels[1].task, NULL);
        }
        if (selection->choiceResult && !panels[0].flags) {
            if (!gSelectionMenuFadeStarted) {
                panels[0].command = 21;
                panels[1].command = 22;
                panels[2].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (selection->choiceResult < 0) {
                gCurrentTaskParent->data18 = 11;
            } else {
                switch (selection->selectedIndex) {
                case BOARD_SHOP_BUY:
                    gCurrentTaskParent->data18 = 16;
                    break;
                case BOARD_SHOP_PAINT:
                    gCurrentTaskParent->data18 = 17;
                    break;
                case BOARD_SHOP_LEAVE:
                    gCurrentTaskParent->data18 = 11;
                    break;
                }
            }
            if (gCurrentTaskParent->data18 == 11) {
                gBoardShopRememberedSelection = 0;
            } else {
                gBoardShopRememberedSelection = selection->selectedIndex + 1;
            }
            if (selection->choiceResult < 0) {
                gCurrentTask->data18 = 11;
            } else {
                gCurrentTask->data18 = 9;
            }
        }
        break;
    case 9:
        panels[0].command = 10;
        gCurrentTask->data18 = 10;
        break;
    case 10:
        if (!panels[0].flags) {
            gCurrentTask->data18 = 11;
        }
        break;
    case 11:
        if (gCurrentTaskParent->data18 == 11) {
            gCurrentTask->data18 = 12;
        } else {
            gCurrentTask->data18 = 6;
        }
        break;
    case 12:
        playSoundEffect(0, 0x202, 32);
        gCurrentTask->data18 = 6;
        break;
    case 6:
        if (BOARD_SHOP_PANELS_READY(panels)) {
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            moveTaskChildrenWithSpacing(0, panels[0].task, -0x134000, -0x50000, 0x1E000, 0x1E000, -1000);
            panels[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(panels[1]), -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(panels[2]), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(panels[3]), 0xAC000, -0x48000, -0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        if (BOARD_SHOP_PANELS_READY(panels)) {
            gCurrentTask->data18 = 8;
        }
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskSaveRecordMoneyPanelWithCharacterSprites(void) {
    Movement2D *position;
    u16 recordIndex;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    recordIndex = gCurrentTask->data18;
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB9, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        {
            s32 amountX = (s16)(position->x / 4096 + 74);
            s32 amountY = (s16)(position->y / 4096 + 152);
            SaveRecord *records = gPlayerSaveRecords;
            drawDecimalNumber8x8(amountX, amountY, 6, 0,
                                 records[(s16)recordIndex].money,
                                 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gMoneyPanelCurrencyLabel, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        if (gCurrentTask->data1A == 1) {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }

    setSpriteColor(128, 128, 128);
    if (sShopCharacterTextureIndices[gRacePlayers[0].unk009] != 0) {
        s32 offsetX;
        s32 sprite;

        if (gRacePlayers[0].unk009 == 5) {
            offsetX = 40;
            sprite = 0x208;
        } else {
            offsetX = 48;
            sprite = 0x169;
        }
        drawSpriteByIndexWithBlend(offsetX + position->x / 4096, position->y / 4096,
                      sprite, 0, 0, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C + 2);
    }
    drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                  0x168, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C + 2);
}

void initializeCountedTaskResourcePairEC50(void) {
    gCurrentTask->data18 = 0;
    gCurrentTaskParent->data1C++;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateMoneyPanelSelectionTask);
    gCurrentTask->unk10 |= 2;
}

void updateMoneyPanelSelectionTask(void) {
    MoneyMenuSelection *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s16 mode;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 0:
        mode = 1;
        setMusicSequenceEnabled(mode);
        selection->mode = 0;
        if (gMoneyPanelRememberedSelection != 0) {
            selection->selectedIndex = gMoneyPanelRememberedSelection - 1;
        } else {
            selection->selectedIndex = 0;
        }
        selection->previousIndex = selection->selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)gMoneyPanelSelectionSprites, sMoneyPanelSelectionSpriteMapping,
                                         NULL, 0, 0, 20, -308, -80, 0, 24, 3, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-320, 8, sShopMenuTexts, gMoneyPanelSelectionTextMapping, 4, 11);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskMoneyPanelWithCharacterSprites, 4, 21);
        uploadTextureArchiveEntry(320, 0, 768, 0xE9, sShopCharacterTextureIndices[gRacePlayers[0].unk009]);
        uploadTextureArchiveEntry(320, 256, 768, 0xEA, 0);
        createRegisteredTexturePairTask(172, -72, sShopSubmenuTextureMapping, 3, selection->selectedIndex, 1, 5, 31);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x50000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 4);
            if (gMoneyPanelRememberedSelection == 0) {
                setTaskSecondaryResourceData(entries[1].task, sShopMenuTexts[5]);
            }
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 4);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 4);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (gMoneyPanelRememberedSelection == 0) {
            playSoundEffect(0, 2, 32);
        }
        gCurrentTask->data18 = 5;
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (selection->selectedIndex != selection->previousIndex) {
            setTaskSecondaryResourceData(entries[1].task, NULL);
        }
        if (selection->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (selection->choiceResult < 0) {
                goto returnToParentMenu;
            }
            switch (selection->selectedIndex) {
            case 0:
                gCurrentTaskParent->data18 = 18;
                break;
            case 1:
                gCurrentTaskParent->data18 = 19;
                break;
            case 2:
returnToParentMenu:
                gCurrentTaskParent->data18 = 11;
                break;
            }
            if (gCurrentTaskParent->data18 == 11) {
                gMoneyPanelRememberedSelection = 0;
            } else {
                gMoneyPanelRememberedSelection = selection->selectedIndex + 1;
            }
            if (selection->choiceResult < 0) {
                goto finishChoice;
            }
            gCurrentTask->data18 = 9;
        }
        break;
    case 9:
        entries[0].command = 10;
        gCurrentTask->data18 = 10;
        break;
    case 10:
        if ((s16)entries[0].flags == 0) {
finishChoice:
            gCurrentTask->data18 = 11;
        }
        break;
    case 11:
        if (gCurrentTaskParent->data18 == 11) {
            gCurrentTask->data18 = 12;
            break;
        }
        goto beginExit;
    case 12:
        playSoundEffect(0, 0x202, 32);
beginExit:
        gCurrentTask->data18 = 6;
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x50000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            gCurrentTask->data18 = 8;
        }
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskMoneyPanelWithCharacterSprites(void) {
    Movement2D *position;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB7, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                             6, 0, gPlayerSaveRecords[0].money, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gMoneyPanelCurrencyLabel, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        if (gCurrentTask->data1A == 1) {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }

    setSpriteColor(128, 128, 128);
    if (sShopCharacterTextureIndices[gRacePlayers[0].unk009] != 0) {
        s32 offsetX;
        s32 sprite;

        if (gRacePlayers[0].unk009 == 5) {
            offsetX = 40;
            sprite = 0x208;
        } else {
            offsetX = 48;
            sprite = 0x169;
        }
        drawSpriteByIndexWithBlend(offsetX + position->x / 4096, position->y / 4096,
                      sprite, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C + 2);
    }
    drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                  0x168, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C + 2);
}

char gMoneyPanelCurrencyLabel[4] __attribute__((section(".data"), aligned(4))) = "\x82" "f";

void initializeCountedGroupedSelectionBoundsTask(void) {
    RECT *bounds;
    s16 selection;
    gCurrentTaskParent->data1C++;
    gCurrentTask->data18 = 0;
    if (gPlayerSaveRecords[0].flags & 0x1C000) {
        gCurrentTask->data1A = 1;
    } else {
        gCurrentTask->data1A = 0;
    }
    if (gCurrentTask->resource14 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    selection = getGroupedIndexBase(0);
    gMenuSelectionStates[0].selection = selection;
    gMenuSelectionStates[0].previousSelection = selection;
    gMenuModelDrawAreas[0].x = 0;
    gMenuModelDrawAreas[0].y = 0;
    gMenuModelDrawAreas[0].w = -1;
    gMenuModelDrawAreas[0].h = -1;
    gMenuModelDrawAreas[1].x = 0;
    bounds = &gMenuModelDrawAreas[1];
    bounds->y = 0;
    bounds->w = -1;
    bounds->h = -1;
    setCurrentTaskCallback(updateGroupedSelectionBoundsTask);
}

void updateGroupedSelectionBoundsTask(void) {
    FlagSelectedMenuState *input;
    RegisteredTaskEntry *entries;
    MenuSelectionState *selection;
    GroupedSelectionDescriptor *descriptor;
    s32 complete;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    selection = &gMenuSelectionStates[0];

    switch (gCurrentTask->data18) {
    case 0: {
        GameTask *task;
        setMusicSequenceEnabled(1);
        descriptor = &gBoardMenuSelectionDescriptors[gCurrentTask->data1A];
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gBoardMenuSprites, descriptor->mapping,
                                        NULL, 0, 0, 20, -304, -96, descriptor->spacingX,
                                        descriptor->spacingY, descriptor->count, 0xB0,
                                        drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-316, 8, gSelectionPanelTexts, descriptor->label, 4, 11);
        createRegisteredPanelTask(0, selection->selection, 244, 0, 384, 21);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSaveRecordMoneyPanel, 4, 31);
        createRegisteredTripletTask(-296, 48, 0, 16, gSelectionPanelStatTriplets, descriptor->tripletIndices, 3, 41);
        task = gCurrentTask;
        input->mode = 0;
        input->selectedIndex = 0;
        input->previousIndex = 0;
        input->choiceResult = 0;
        task->data18 = 2;
        break;
    }
    case 2:
        if (CHILDREN_READY(entries)) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D(CHILD_MOVEMENT(entries[1]), -0x88000, 0x8000, 0x1E000, 0x1E000, 5);
            configureRegisteredChildMovement(0, entries[2].task, 0x40000, 0, 0x1E000, 0x1E000, 5);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D(CHILD_MOVEMENT(entries[4]), -0x74000, 0x30000, 0x1E000, 0x1E000, 5);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (CHILDREN_READY(entries)) {
            GameTask *task;

            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 1;
            entries[3].command = 20;
            entries[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = gCurrentTask;
            gSelectionMenuFadeStarted = 0;
            task->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[3].command = 22;
            entries[4].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (input->selectedIndex != input->previousIndex) {
            selection->previousSelection = selection->selection;
            selection->selection = getTaskResourceGroupedIndex(entries[0].task, input->selectedIndex);
            if (selection->selection == 9) {
                selection->selection += getSaveRecordFlagGroupIndex(0, 1);
            }
            if (selection->selection < 0) {
                selection->selection = selection->previousSelection;
            } else if (selection->selection != selection->previousSelection) {
                if (input->previousIndex < input->selectedIndex) {
                    entries[2].command = 11;
                } else {
                    entries[2].command = 12;
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && *(s16 *)&entries[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[3].command = 22;
                entries[4].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (input->choiceResult < 0) {
                gCurrentTaskParent->data18 = 12;
                goto startExit;
            }
            gCurrentTask->data18 = 13;
        }
        break;
    case 13: {
        GameTask *task = gCurrentTask;
        entries[0].command = 10;
        task->data18 = 14;
        break;
    }
    case 14:
        if (*(s16 *)&entries[0].flags == 0) {
            gCurrentTask->data18 = 5;
        }
        break;
    case 5: {
        GameTask *task = gCurrentTask;
        descriptor = &gBoardMenuSelectionDescriptors[task->data1A];
        if (descriptor->mapping[input->selectedIndex] == 17) {
            boardShopRememberedSelectionAbsolute = 0;
            gCurrentTaskParent->data18 = 11;
            playSoundEffect(0, 0x202, 32);
startExit:
            gCurrentTask->data18 = 6;
        } else {
            task->data18 = 9;
        }
        break;
    }
    case 9:
        createChildTaskWithReserve(initializeTaskResourcePair);
        gCreatedTask->data18 = 0;
        descriptor = &gBoardMenuSelectionDescriptors[gCurrentTask->data1A];
        gCreatedTask->data1A = mapCourseMenuByteIndex(descriptor->mapping, input->selectedIndex);
        gCurrentTask->data18 = 10;
        break;
    case 11:
        gCurrentTaskParent->data18 = 12;
        goto finishExit;
    case 6:
        if (CHILDREN_READY(entries)) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(entries[1]), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            configureRegisteredChildMovement(0, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[4]), -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        if (CHILDREN_READY(entries)) {
finishExit:
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
        gCurrentTask->data18 = 12;
        break;
    case 10:
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void initializeTaskResourcePair(void) {
    gCurrentTask->data18 = 0;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateShopBoardPurchaseMenuTask);
}

void updateShopBoardPurchaseMenuTask(void) {
    MoneyMenuSelection *parentSelection;
    RegisteredPanelSlot *parentPanels;
    MoneyMenuSelection *selection;
    RegisteredPanelSlot *panels;
    MenuSelectionState *groupedSelection;
    RecordFlagMenuEntries entryStorage;
    u8 padding[8];
    RecordFlagMenuEntries *entries;
    s32 complete;
    u32 price;
    s16 value;
    s16 i;
    s32 recordIndex;

    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parentSelection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
    parentPanels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    selection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    recordIndex = 0;
    groupedSelection = &gMenuSelectionStates[0];

    switch (gCurrentTask->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x1BC000, -0x60000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (parentPanels[0].flags == 0) {
            parentPanels[0].command = 23;
            parentPanels[1].command = 23;
            parentPanels[4].command = 23;
            configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        if (parentPanels[0].flags == 0) {
            s32 depth;
            s16 gridMode;
            if (gCurrentTask->data1A == 4) {
                SaveRecord *record;
                entries = &entryStorage;
                i = 0;
                record = &gPlayerSaveRecords[0];
                entries->unk_05 = 20;
                entries->unk_04 = 0;
                entries->entries = gRecordFlagMenuItemBuffers[0];
                entries->count = 0;
                entries->textIndices = gRecordFlagMenuTextIndexBuffers[0];
                entries->unk_10 = 0;
                for (; i < 3; i++) {
                    if (record->flags & (0x800U << (i + 3))) {
                        entries->entries[entries->count] = i + 11;
                        entries->textIndices[entries->count] = i + 5;
                        entries->count++;
                    }
                }
                gridMode = 1;
            } else {
                entries = &sShopDefaultBoardEntries;
                gridMode = 1;
            }
            createRegisteredSpacedChildTasks(gridMode, 0xB0, 0x40, (const u8 *)gBoardMenuSprites,
                                             entries->entries, NULL, 0, 0, 0, -444, -96,
                                             entries->unk_04, entries->unk_05, entries->count, 222,
                                             drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 5, 1);
            if (gCurrentTask->data1A == 4) {
                createRegisteredTextPanelTask(-136, 8, gSelectionPanelTexts, entries->textIndices, 4, 11);
            } else {
                createRegisteredTextPanelTask(-136, 8, NULL, entries->textIndices, 4, 11);
            }
            depth = 3;
            createRegisteredCallbackTask(0, 0, 0, drawCurrentTaskSelectedBoardDetails, depth, 21);
            createRegisteredCallbackTask(0, 0, 0, drawCurrentTaskSelectedBoardPrice, depth, 31);
            selection->mode = 0;
            selection->selectedIndex = getGroupedIndexOffsetPlus216(groupedSelection->selection);
            value = findIndexedHalfwordRecord((const IndexedHalfwordRecord *)gBoardMenuSprites, entries->entries,
                entries->count, selection->selectedIndex);
            selection->selectedIndex = value;
            selection->previousIndex = value;
            selection->choiceResult = 0;
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            panels[2].flags == 0 && panels[3].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x74000, -0x60000, 0, 0);
            configureVerticalTaskGridMovement(0, panels[0].task, -0x74000, -0x4C000, 0x8000, 0x8000, 2);
            initMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14], -0x1C8000, 0x8000);
            goto showSelection;
        }
        break;
    case 4:
        if (panels[0].flags == 0 && parentPanels[1].flags == 0) {
            GameTask *task;

            panels[0].command = 20;
            panels[1].command = 20;
            panels[2].command = 20;
            panels[3].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = gCurrentTask;
            gSelectionMenuFadeStarted = 0;
            task->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            panels[2].command = 22;
            panels[3].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (hasTaskResource16Value(panels[1].task) != 0) {
            panels[2].command = 23;
        } else if (gSelectionMenuFadeStarted != 0) {
            panels[2].command = 22;
        } else {
            panels[2].command = 20;
        }
        if (parentPanels[2].flags == 0 && selection->choiceResult != 0) {
            gCurrentTask->data18 = 6;
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(gCurrentTask->data1A, panels[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
                selection->previousIndex = selection->selectedIndex;
            } else {
                parentPanels[2].command = 13;
                selection->previousIndex = selection->selectedIndex;
                setTaskSecondaryResourceData(panels[1].task, NULL);
            }
        } else if (gCurrentTask->data18 != 5 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 22;
            panels[1].command = 22;
            panels[2].command = 22;
            panels[3].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        break;
    case 6:
        if (selection->choiceResult != 0 && panels[0].flags == 0) {
            if (selection->choiceResult < 0) {
                gCurrentTask->data18 = 9;
            } else {
                gCurrentTask->data18 = 14;
            }
        }
        break;
    case 9: {
        GameTask *task;

        configureTaskGridMovement(0, panels[0].task, -0x74000, -0x60000, 0x1E000, 0x1E000, 0);
        task = gCurrentTask;
        panels[0].command = 23;
        task->data18 = 10;
        break;
    }
    case 10:
        if (panels[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x1B4000, -0x60000, 0, 0);
            moveTaskChildrenWithSpacing(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 11;
        }
        break;
    case 11:
        if (parentPanels[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 12;
        }
        break;
    case 12:
        if (parentPanels[0].flags == 0) {
            GameTask *task;

            initMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14], -0x1C8000, 0x8000);
            initMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14], -0x88000, 0x8000);
            parentPanels[2].command = 1;
            parentPanels[0].command = 20;
            parentPanels[1].command = 20;
            parentPanels[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = gCurrentTask;
            gSelectionMenuFadeStarted = 0;
            task->data18 = 13;
        }
        break;
    case 13:
        parentSelection->choiceResult = 0;
        gCurrentTaskParent->data18 = 4;
        goto finish;
    case 14: {
        s16 board = groupedSelection->selection;
        u8 flagIndex = gShopBoardUnlockFlagIndices[board];
        SaveRecord *record = &gPlayerSaveRecords[recordIndex];

        if (record->flags & (4U << flagIndex)) {
            GameTask *task;

            setTaskSecondaryResourceData(panels[1].task, sShopMenuTexts[7]);
            task = gCurrentTask;
            panels[0].command = 21;
            parentPanels[2].command = 1;
            selection->choiceResult = 0;
            task->data18 = 5;
        } else if ((price = gShopBoardPrices[board], record->money < price)) {
            setTaskSecondaryResourceData(panels[1].task, sShopMenuTexts[6]);
            panels[0].command = 21;
            parentPanels[2].command = 1;
            selection->choiceResult = 0;
            playSoundEffect(0, 0x302, 32);
            gCurrentTask->data18 = 5;
        } else {
            parentPanels[2].command = 2;
            createValuePanelTask(-120, -48, &selection->mode, 2, 41);
            playSoundEffect(0, 0x502, 32);
            gCurrentTask->data18 = 15;
        }
        break;
    }
    case 15:
        if (panels[4].flags == 0) {
            if (selection->choiceResult != 1) {
                parentPanels[2].command = 1;
showSelection:
                gCurrentTask->data18 = 4;
            } else {
                u32 *balance = &gPlayerSaveRecords[recordIndex].money;
                u32 target;
                GameTask *transition;
                GameTask *task;
                s16 board = groupedSelection->selection;

                target = *balance;
                price = gShopBoardPrices[board];
                target -= price;
                transition = createValueTransitionTask(balance, target, 100);
                task = gCurrentTask;

                panels[5].task = transition;
                task->data18 = 16;
            }
        }
        break;
    case 16: {
        GameTask *transition;
        s16 group;
        s16 groupComplete;
        s32 shift;

        if (gFrameCounter & 1) {
            playSoundEffect(1, 2, 32);
        }
        transition = panels[5].task;
        if (transition->data1A == 1) {
            SaveRecord *records = gPlayerSaveRecords;
            SaveRecord *record = records;
            s32 player;
            u8 flagIndex;
            group = 0;
            player = 0;
            transition->data18 = 1;
            flagIndex = gShopBoardUnlockFlagIndices[groupedSelection->selection];
            record[recordIndex].flags |= 4U << flagIndex;
            for (; group < 3; group++) {
                groupComplete = 1;
                for (i = 0; i < 3; i++) {
                    u32 flags;
                    shift = group * 3 + i;
                    flags = gPlayerSaveRecords[(s16)player].flags;
                    if ((flags & (4 << shift)) == 0) {
                        groupComplete = 0;
                        break;
                    }
                }
                if (groupComplete) {
                    setPairedStateIndexedFlag(sShopBoardGroupUnlockFlags[group]);
                }
            }
            panels[5].task = createCountdownTask(15);
            playSoundEffect(1, 2, 32);
            gCurrentTask->data18 = 17;
        }
        break;
    }
    case 17: {
        u16 done = panels[5].task->data1A;
        if (done == 1) {
            GameTask *task;

            playSoundEffect(0, 0x102, 32);
            task = gCurrentTask;
            panels[5].task->data18 = done;
            task->data18 = 7;
        }
        break;
    }
    case 7:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x1B4000, -0x4C000, 0x1E000, 0x1E000, -1000);
        panels[0].command = 23;
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[1].task->resource14],
                            -0x1C8000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[2].task->resource14],
                            -0x140000, 0, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[panels[3].task->resource14],
                            0x140000, 0, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(0, parentPanels[0].task, -0x1BC000, -0x60000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[1].task->resource14],
                            -0x1C8000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureRegisteredChildMovement(0, parentPanels[2].task, 0x180000, 0, 0x1E000, 0, -1000);
        configureRegisteredChildMovement(1, parentPanels[2].task, 0x180000, 0, 0x1E000, 0, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[3].task->resource14],
                            0x138000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&gTaskMemoryPool[parentPanels[4].task->resource14],
                            -0x1B4000, 0x30000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 8;
        break;
    case 8:
        if (panels[0].flags == 0 && panels[1].flags == 0 &&
            panels[2].flags == 0 && panels[3].flags == 0 &&
            parentPanels[0].flags == 0 && parentPanels[1].flags == 0 &&
            parentPanels[2].flags == 0 && parentPanels[3].flags == 0 && parentPanels[4].flags == 0) {
            gCurrentTaskParent->data18 = 11;
            goto finish;
        }
        break;
    default:
finish:
        deleteCurrentTaskSubtree();
        break;
    }
}

extern char gSaveRecordMoneyPanelCurrencyLabel[]
    __attribute__((section(".data.save_record_money_panel_currency_label")));

void drawCurrentTaskSaveRecordMoneyPanel(void) {
    Movement2D *position;
    u16 recordIndex;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    recordIndex = gCurrentTask->data18;
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB9, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        {
            s32 amountX = (s16)(position->x / 4096 + 74);
            s32 amountY = (s16)(position->y / 4096 + 152);
            SaveRecord *records = gPlayerSaveRecords;
            drawDecimalNumber8x8(amountX, amountY, 6, 0,
                                 records[(s16)recordIndex].money,
                                 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gSaveRecordMoneyPanelCurrencyLabel, 0, -1,
                           &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        if (gCurrentTask->data1A == 1) {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }
}

char gSaveRecordMoneyPanelCurrencyLabel[4]
    __attribute__((section(".data.save_record_money_panel_currency_label"), aligned(4))) = "\x82" "f";

void drawCurrentTaskSelectedBoardDetails(void) {
    Movement2D *position;
    MenuSelectionState *entry;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    entry = &gMenuSelectionStates[(s16)gCurrentTask->data18];
    if (gCurrentTask->data1A != 0) {
        s16 board = entry->selection;
        u32 price = gShopBoardPrices[board];
        s32 priceOffset = 4;

        if (board < 9) {
            s32 textIndex = gBoardSelectionStyles[board];
            priceOffset = 0;
            drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 8,
                10, gBoardStyleText[textIndex], 6, -1,
                &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 24,
                10, (s8 *)gSelectedBoardLevelLabel, 6, -1,
                &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawDecimalNumber8x16(position->x / 4096 - 64, position->y / 4096 + 24,
                1, 0, gBoardSelectionLevels[entry->selection], 6, -1,
                &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
        if (price == 0) {
            s32 x = (s16)(position->x / 4096 - 104);
            s32 y = position->y / 4096;
            s32 textOffset = priceOffset + 8;
            drawWrappedDoubleByteText(x, y + textOffset + 32,
                10, (s8 *)gSelectedBoardPricePlaceholder, 6, -1,
                &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            u32 remaining = price;
            s32 count = 0;
            s32 nextCount;
            s16 digits;
            s32 textOffset;

            do {
                nextCount = ++count;
                remaining /= 10;
            } while (remaining != 0);
            digits = nextCount;
            {
                s32 x = (s16)(position->x / 4096 - 104);
                s32 y = position->y / 4096;
                textOffset = priceOffset + 8;
                drawDecimalNumber8x16(x, y + textOffset + 32,
                    digits, 0, price, 6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
            {
                s32 x = position->x / 4096;
                s32 currencyOffset = digits * 8 - 104;
                drawWrappedDoubleByteText(x + currencyOffset,
                    position->y / 4096 + textOffset + 32,
                    1, (s8 *)gSaveRecordMoneyPanelCurrencyLabel, 6, -1,
                    &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
        }
    }
}

void drawCurrentTaskSelectedBoardPrice(void) {
    Movement2D *position;
    s16 player;
    MenuSelectionState *entry;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    player = gCurrentTask->data18;
    entry = &gMenuSelectionStates[player];
    if (gCurrentTask->data1A != 0) {
        s16 board = entry->selection;
        u8 flagIndex = gShopBoardUnlockFlagIndices[board];
        u32 price = gShopBoardPrices[board];
        SaveRecord *records = gPlayerSaveRecords;
        if (records[player].flags & (4U << flagIndex)) {
            drawSpriteByIndexWithBlend(position->x / 4096 + 36, position->y / 4096 - 8, 0xC3, 15, -1,
                         &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            u32 remaining = price;
            s16 digits = 0;
            s32 xOffset;
            s32 x;

            while (remaining != 0) {
                remaining /= 10;
                digits++;
            }
            if (digits < 5) {
                digits = 5;
            }
            x = position->x / 4096;
            xOffset = digits * 4 - 64;
            drawDecimalNumber8x8(x - xOffset, position->y / 4096 - 4,
                                 digits, 0, price, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawWrappedText8x8(position->x / 4096 - xOffset + digits * 8, position->y / 4096 - 4,
                               2, gSaveRecordMoneyPanelCurrencyLabel, 0, -1,
                               &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            if (gCurrentTask->data1A == 1) {
                addTilePacketWithBlend(position->x / 4096 - xOffset - 4, position->y / 4096 - 8,
                              digits * 8 + 16, 16, gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            } else {
                addTilePacketWithBlend(position->x / 4096 - xOffset - 4, position->y / 4096 - 8,
                              digits * 8 + 16, 16, 128, 128, 128, 2,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
        }
    }
}

void initializeCountedSelectionBoundsTask(void) {
    RECT *bounds;
    gCurrentTaskParent->data1C++;
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 0;
    if (gCurrentTask->resource14 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    gMenuSelectionStates[0].selection = 0;
    gMenuSelectionStates[0].previousSelection = 0;
    gMenuModelDrawAreas[0].x = 0;
    gMenuModelDrawAreas[0].y = 0;
    gMenuModelDrawAreas[0].w = -1;
    gMenuModelDrawAreas[0].h = -1;
    gMenuModelDrawAreas[1].x = 0;
    bounds = &gMenuModelDrawAreas[1];
    bounds->y = 0;
    bounds->w = -1;
    bounds->h = -1;
    setCurrentTaskCallback(updateBoardSelectionMenuTask);
}

void updateBoardSelectionMenuTask(void) {
    MoneyMenuSelection *selection;
    RegisteredTaskEntry *entries;
    MenuSelectionState *groupedSelection;
    GroupedSelectionDescriptor *descriptor;
    s32 complete;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    selection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    groupedSelection = &gMenuSelectionStates[0];

    switch (gCurrentTask->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        descriptor = &gBoardMenuSelectionDescriptors[gCurrentTask->data1A];
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gBoardMenuSprites,
                                         descriptor->mapping, NULL, 0, 0, 20, -304, -96,
                                         descriptor->spacingX, descriptor->spacingY,
                                         descriptor->count, 176,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredTextPanelTask(-316, 8, gSelectionPanelTexts, descriptor->label, 4, 11);
        createRegisteredPanelTask(0, groupedSelection->selection, 244, 0, 384, 21);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSpriteBA, 4, 31);
        createRegisteredTripletTask(-296, 48, 0, 16, gSelectionPanelStatTriplets, descriptor->tripletIndices, 3, 41);
        selection->mode = 0;
        selection->selectedIndex = 0;
        selection->previousIndex = 0;
        selection->choiceResult = 0;
        gCurrentTask->data18 = 2;
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x88000, 0x8000, 0x1E000, 0x1E000, 5);
            configureRegisteredChildMovement(0, entries[2].task, 0x40000, 0, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0x1E000, 5);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[4].task->resource14],
                                -0x74000, 0x30000, 0x1E000, 0x1E000, 5);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 1;
            entries[3].command = 20;
            entries[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[3].command = 22;
            entries[4].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceGroupedIndex(entries[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
            } else if (groupedSelection->previousSelection != groupedSelection->selection) {
                if (selection->previousIndex >= selection->selectedIndex) {
                    entries[2].command = 12;
                } else {
                    entries[2].command = 11;
                }
            }
            selection->previousIndex = selection->selectedIndex;
        } else if (selection->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[3].command = 22;
                entries[4].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (selection->choiceResult < 0) {
                gCurrentTaskParent->data18 = 12;
                goto beginExit;
            }
            gCurrentTask->data18 = 14;
        }
        break;
    case 14:
        entries[0].command = 10;
        gCurrentTask->data18 = 15;
        break;
    case 15:
        if ((s16)entries[0].flags == 0) {
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        descriptor = &gBoardMenuSelectionDescriptors[gCurrentTask->data1A];
        if (descriptor->mapping[selection->selectedIndex] == 17) {
            boardShopRememberedSelectionAbsolute = 0;
            gCurrentTaskParent->data18 = 11;
            playSoundEffect(0, 0x202, 32);
beginExit:
            gCurrentTask->data18 = 6;
        } else {
            gCurrentTask->data18 = 9;
        }
        break;
    case 9:
        createChildTaskWithReserve(initializeTaskResourcesAndClearData18);
        gCreatedTask->data18 = 0;
        descriptor = &gBoardMenuSelectionDescriptors[gCurrentTask->data1A];
        gCreatedTask->data1A = mapCourseMenuByteIndex(descriptor->mapping, selection->selectedIndex);
        gCurrentTask->data18 = 10;
        break;
    case 10:
        break;
    case 11:
        gCurrentTaskParent->data18 = 12;
        goto finishExit;
    case 12:
        boardShopRememberedSelectionAbsolute = 0;
        gCurrentTaskParent->data18 = 11;
        playSoundEffect(0, 0x202, 32);
        gCurrentTask->data18 = 13;
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
            configureRegisteredChildMovement(0, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureRegisteredChildMovement(1, entries[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[4].task->resource14],
                                -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0 &&
            (s16)entries[4].flags == 0) {
finishExit:
            gCurrentTask->data18 = 8;
        }
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void initializeTaskResourcesAndClearData18(void) {
    gCurrentTask->data18 = 0;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setCurrentTaskCallback(updateBoardSelectionDetailsMenuTask);
}

void updateBoardSelectionDetailsMenuTask(void) {
    MoneyMenuSelection *parentSelection;
    RegisteredPanelSlot *parentPanels;
    MoneyMenuSelection *selection;
    RegisteredPanelSlot *panels;
    MenuSelectionState *groupedSelection;
    RecordFlagMenuEntries entryStorage;
    RecordFlagMenuEntries *entries;
    RegisteredPanelSlot *detailPanels;
    s32 complete;
    s16 value;

    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parentSelection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
    parentPanels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    selection = (MoneyMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    groupedSelection = &gMenuSelectionStates[0];

    switch (gCurrentTask->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (parentPanels[0].flags == 0) {
            parentPanels[0].command = 23;
            parentPanels[1].command = 23;
            parentPanels[4].command = 23;
            configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        if (parentPanels[0].flags == 0) {
            entries = &entryStorage;
            buildRecordFlagMenuEntries(0, gCurrentTaskParent->data1A, parentSelection->selectedIndex, 0, 20, entries);
            createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gBoardMenuSprites,
                                             entries->entries, NULL, 0, 0, 20, -304, -96,
                                             entries->unk_04, entries->unk_05, entries->count, 222,
                                             drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 5, 1);
            createRegisteredTextPanelTask(-136, 8, NULL, entries->textIndices, 4, 11);
            createRegisteredCallbackTask(0, 0, 0, drawCurrentTaskBoardStyleAndLevel, 3, 21);
            selection->mode = 0;
            value = getGroupedIndexOffsetPlus216(groupedSelection->selection);
            selection->selectedIndex = value;
            value = findIndexedHalfwordRecord((const IndexedHalfwordRecord *)gBoardMenuSprites, entries->entries, entries->count, value);
            selection->selectedIndex = value;
            selection->previousIndex = value;
            selection->choiceResult = 0;
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x74000, -0x60000, 0, 0);
            configureVerticalTaskGridMovement(0, panels[0].task, -0x74000, -0x4C000, 0x8000, 0x8000, 2);
            initMovement2D(TASK_POSITION(parentPanels[1].task), -0x13C000, 0x8000);
            goto showSelection;
        }
        break;
    case 4:
        if (panels[0].flags == 0 && parentPanels[1].flags == 0) {
            panels[0].command = 20;
            panels[1].command = 20;
            panels[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 21;
            panels[1].command = 22;
            panels[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (parentPanels[2].flags == 0) {
            if (selection->choiceResult > 0) {
                parentPanels[2].command = 2;
            }
            if (selection->choiceResult != 0) {
                gCurrentTask->data18 = 6;
            }
        }
        if (groupedSelection->selection == groupedSelection->previousSelection &&
            selection->selectedIndex != selection->previousIndex) {
            groupedSelection->previousSelection = groupedSelection->selection;
            groupedSelection->selection = getTaskResourceAdjustedGroupedIndex(gCurrentTask->data1A, panels[0].task, selection->selectedIndex);
            if (groupedSelection->selection < 0) {
                groupedSelection->selection = groupedSelection->previousSelection;
                selection->previousIndex = selection->selectedIndex;
            } else {
                parentPanels[2].command = 13;
                selection->previousIndex = selection->selectedIndex;
                setTaskSecondaryResourceData(panels[1].task, NULL);
            }
        } else if (gCurrentTask->data18 != 5 && gSelectionMenuFadeStarted == 0) {
            panels[0].command = 22;
            panels[1].command = 22;
            panels[2].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        break;
    case 6:
        if (selection->choiceResult != 0 && panels[0].flags == 0) {
            if (selection->choiceResult < 0) {
                gCurrentTask->data18 = 10;
            } else {
                gCurrentTask->data18 = 21;
            }
        }
        break;
    case 10:
        configureTaskGridMovement(0, panels[0].task, -0x74000, -0x60000, 0x1E000, 0x1E000, 0);
        panels[0].command = 23;
        gCurrentTask->data18 = 11;
        break;
    case 11:
        if (panels[0].flags == 0) {
            initializeSpacedTaskGridPositions(0, panels[0].task, -0x128000, -0x60000, 0, 0);
            moveTaskChildrenWithSpacing(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 12;
        }
        break;
    case 12:
        if (parentPanels[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 13;
        }
        break;
    case 13:
        if (parentPanels[0].flags == 0) {
            initMovement2D(TASK_POSITION(panels[1].task), -0x13C000, 0x8000);
            initMovement2D(TASK_POSITION(parentPanels[1].task), -0x88000, 0x8000);
            parentPanels[2].command = 1;
            parentPanels[0].command = 20;
            parentPanels[1].command = 20;
            parentPanels[4].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 14;
        }
        break;
    case 14:
        parentSelection->choiceResult = 0;
        gCurrentTaskParent->data18 = 4;
        deleteCurrentTaskSubtree();
        break;
    case 21:
        panels[0].command = 10;
        gCurrentTask->data18 = 22;
        break;
    case 22:
        if (panels[0].flags == 0) {
            gCurrentTask->data18 = 15;
        }
        break;
    case 15:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x128000, -0x4C000, 0x1E000, 0x1E000, -1000);
        panels[0].command = 23;
        configureMovement2D(TASK_POSITION(panels[1].task), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(panels[2].task), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(1, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 16;
        break;
    case 16:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 && parentPanels[0].flags == 0) {
            TaskMemoryBlock *resources;
            createChildTaskWithReserve(initializePlayerTextureSelectionTask);
            gCreatedTask->resource14 = allocateTaskMemoryBlock();
            resources = gTaskMemoryPool;
            gCreatedTask->resource16 = allocateTaskMemoryBlock();
            detailPanels = (RegisteredPanelSlot *)&resources[gCreatedTask->resource16];
            detailPanels[0].task = parentPanels[2].task;
            detailPanels[0].command = 0;
            detailPanels[0].flags = 0;
            detailPanels[1].task = parentPanels[3].task;
            detailPanels[1].command = 0;
            detailPanels[1].flags = 0;
            panels[4].task = gCreatedTask;
            gCurrentTask->data18 = 17;
        }
        break;
    case 17:
        if (panels[4].task->data1E != 0) {
            parentPanels[2].command = panels[4].task->data1E;
            panels[4].task->data1E = 1;
        } else {
            panels[4].task->data1E = parentPanels[2].flags;
        }
        break;
    case 18:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x74000, -0x4C000, 0x1E000, 0x1E000, 0);
        panels[0].command = 23;
        configureMovement2D(TASK_POSITION(panels[1].task), -0x88000, 0x8000, 0x1E000, 0x1E000, 0);
        configureMovement2D(TASK_POSITION(panels[2].task), 0, 0, 0x1E000, 0, 0);
        configureTaskGridMovement(1, parentPanels[0].task, -0x7C000, -0x60000, 0x1E000, 0x1E000, 0);
        selection->choiceResult = 0;
        parentPanels[2].command = 1;
        gCurrentTask->data18 = 19;
        break;
    case 19:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 && parentPanels[0].flags == 0) {
showSelection:
            gCurrentTask->data18 = 4;
        }
        break;
    case 20:
        gCurrentTaskParent->data18 = 12;
        deleteCurrentTaskSubtree();
        break;
    case 7:
        moveTaskChildrenWithSpacing(0, panels[0].task, -0x128000, -0x4C000, 0x1E000, 0x1E000, -1000);
        panels[0].command = 23;
        configureMovement2D(TASK_POSITION(panels[1].task), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(panels[2].task), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(0, parentPanels[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(parentPanels[1].task), -0x13C000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureRegisteredChildMovement(0, parentPanels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
        configureRegisteredChildMovement(1, parentPanels[2].task, 0xF4000, 0, 0x1E000, 0, -1000);
        configureMovement2D(TASK_POSITION(parentPanels[3].task), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(TASK_POSITION(parentPanels[4].task), -0x128000, 0x30000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 8;
        break;
    case 8:
        if (panels[0].flags == 0 && panels[1].flags == 0 && panels[2].flags == 0 &&
            parentPanels[0].flags == 0 && parentPanels[1].flags == 0 && parentPanels[2].flags == 0 &&
            parentPanels[3].flags == 0 && parentPanels[4].flags == 0) {
            gCurrentTaskParent->data18 = 11;
            deleteCurrentTaskSubtree();
        }
        break;
    default:
        gCurrentTask->data18 = 9;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskBoardStyleAndLevel(void) {
    Movement2D *position;
    MenuSelectionState *entry;
    s32 textIndex;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    entry = &gMenuSelectionStates[(s16)gCurrentTask->data18];
    if (gCurrentTask->data1A != 0) {
        textIndex = gBoardSelectionStyles[entry->selection];
        drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 8,
            10, gBoardStyleText[textIndex], 6, -1,
            &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawWrappedDoubleByteText(position->x / 4096 - 104, position->y / 4096 + 24,
            10, (s8 *)gBoardLevelLabel, 6, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawDecimalNumber8x16(position->x / 4096 - 64, position->y / 4096 + 24,
            1, 0, gBoardSelectionLevels[entry->selection], 6, -1,
            &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

void initializePlayerTextureSelectionTask(void) {
    gCurrentTask->data18 = 0;
    gCurrentTask->data1E = 0;
    setCurrentTaskCallback(updatePlayerTextureSelectionTask);
}

static inline void uploadPlayerTexturePreview(s16 index) {
    OffsetArchive *textures = DAT_STARTUP_TEXTURES;
    /* Archive word zero holds the count, followed by the texture offsets. */
    s16 wordIndex = gMenuPlayerTextureIds[index] + 1;
    uploadTimImageAt(576, 256, 768, 238,
                     (u32 *)((s16 *)textures + ((s32)textures->offsets[wordIndex - 1] >> 1)));
}

void updatePlayerTextureSelectionTask(void) {
    FlagSelectedMenuState *menu;
    RegisteredPanelSlot *panels;
    MenuSelectionState *board;
    MenuModelSelectionPair *texture;
    SpacedGroupResource *group;
    s16 item;
    s32 complete;
    u8 padding[0x20];

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    menu = (FlagSelectedMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    panels = (RegisteredPanelSlot *)&gTaskMemoryPool[gCurrentTask->resource16];
    board = &gMenuSelectionStates[0];
    texture = &gMenuPlayerTextureSelections[0];

    switch (gCurrentTask->data18) {
    case 0: {
        const u8 *saved = gPlayerResourceTextureSelections.entries[0];
        /* This signed subscript keeps the row base in the original address register. */
        s16 initial = findByteIndexOrZero(gMenuPlayerTextureIds, *(saved - -(s32)board->selection), 16);

        menu->mode = 0;
        if (initial >= 8) {
            menu->selectedIndex = initial - 8;
            gCurrentTask->data1A = 1;
        } else {
            menu->selectedIndex = initial;
            gCurrentTask->data1A = 0;
        }
        menu->previousIndex = menu->selectedIndex;
        menu->choiceResult = 0;
        texture->selection = initial;
        texture->previousSelection = initial;
        gCurrentTask->data18 = 1;
        break;
    }
    case 1: {
        const u8 *mapping = sPlayerTextureSelectionPages[gCurrentTask->data1A];

        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)sPlayerTextureSelectionSprites,
                                         mapping, NULL, 0, 0, 20, -304, -96,
                                         0, 20, 10, 0x11F, drawSelectedTaskSprite,
                                         drawRegisteredTaskGridSpriteWithColor, 4, 21);
        createRegisteredCallbackTask(0, 180, 64, drawCurrentTaskFramedSpritePair16A, 3, 31);
        uploadPlayerTexturePreview(texture->selection);
        gCurrentTask->data18 = 2;
        break;
    }
    case 2:
        if (panels[2].flags == 0 && panels[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, panels[2].task, -0x7C000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D(CHILD_MOVEMENT(panels[3]), 0, 0x40000, 0x1E000, 0, 10);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (panels[2].flags == 0 && panels[3].flags == 0) {
            panels[2].command = 20;
            panels[3].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            panels[2].command = 21;
            panels[3].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (menu->selectedIndex != menu->previousIndex) {
            menu->previousIndex = menu->selectedIndex;
            group = (SpacedGroupResource *)&gTaskMemoryPool[panels[2].task->resource14];
            item = mapMenuByteIndex(group->mapping, menu->selectedIndex) - TEXTURE_MENU_FIRST_PAINT;
            if ((u16)item >= 16) {
                break;
            }
            texture->selection = item;
            uploadPlayerTexturePreview(texture->selection);
        }
        if (gControllerPressedButtons[0] & PADLleft) {
            if (gCurrentTask->data1A == 0) {
                gCurrentTask->data1A = 1;
            } else {
                gCurrentTask->data1A--;
            }
            gCurrentTask->data18 = 10;
            break;
        }
        if (menu->choiceResult == 0 || panels[2].flags != 0) {
            break;
        }
        if (gSelectionMenuFadeStarted == 0) {
            panels[2].command = 22;
            panels[3].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        if (menu->choiceResult < 0) {
            gCurrentTask->data18 = 13;
        } else {
            group = (SpacedGroupResource *)&gTaskMemoryPool[panels[2].task->resource14];
            item = mapMenuByteIndex(group->mapping, menu->selectedIndex);
            if (item == TEXTURE_MENU_NEXT_PAGE && gCurrentTask->data1A == 0) {
                gCurrentTask->data1A++;
                gCurrentTask->data18 = 10;
            } else if (item == TEXTURE_MENU_PREVIOUS_PAGE && gCurrentTask->data1A != 0) {
                gCurrentTask->data1A--;
                gCurrentTask->data18 = 10;
            } else if (item == TEXTURE_MENU_DONE) {
                gCurrentTask->data18 = 15;
            } else if ((u16)(item - TEXTURE_MENU_FIRST_PAINT) < 16 && texture->selection != texture->previousSelection) {
                gCurrentTask->data1E = 20;
                gCurrentTask->data18 = 8;
            } else {
                gCurrentTask->data18 = 9;
            }
        }
        break;
    case 8:
        if (gCurrentTask->data1E != 0) {
            gCurrentTask->data1E = 0;
        } else {
            gCurrentTask->data18 = 9;
        }
        break;
    case 9:
        panels[2].command = 21;
        menu->choiceResult = 0;
        gCurrentTask->data18 = 4;
        break;
    case 15:
        panels[2].command = 10;
        gCurrentTask->data18 = 16;
        break;
    case 16:
        if (panels[2].flags == 0) {
            gCurrentTask->data18 = 5;
        }
        break;
    case 13:
        moveTaskChildrenWithSpacing(0, panels[2].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        configureMovement2D(CHILD_MOVEMENT(panels[3]), 0xB4000, 0x40000, 0x1E000, 0, 0);
        gCurrentTask->data18 = 14;
        break;
    case 14:
        if (panels[2].flags == 0 && panels[3].flags == 0) {
            gCurrentTaskParent->data18 = 18;
            deleteCurrentTaskSubtree();
        }
        break;
    case 10:
        moveTaskChildrenWithSpacing(0, panels[2].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
        panels[2].command = 23;
        gCurrentTask->data18 = 11;
        break;
    case 11:
        if (panels[2].flags == 0) {
            group = (SpacedGroupResource *)&gTaskMemoryPool[panels[2].task->resource14];
            group->mapping = sPlayerTextureSelectionPages[gCurrentTask->data1A];
            moveTaskChildrenWithSpacing(0, panels[2].task, -0x80000, -0x60000, 0x1E000, 0, 1);
            panels[2].command = 23;
            panels[3].command = 23;
            group = (SpacedGroupResource *)&gTaskMemoryPool[panels[2].task->resource14];
            if (menu->selectedIndex < 8) {
                item = mapMenuByteIndex(group->mapping, menu->selectedIndex);
            } else {
                item = mapMenuByteIndex(group->mapping, 7);
            }
            texture->selection = item - TEXTURE_MENU_FIRST_PAINT;
            uploadPlayerTexturePreview(texture->selection);
            menu->choiceResult = 0;
            gCurrentTask->data18 = 3;
        }
        break;
    case 5:
        moveTaskChildrenWithSpacing(0, panels[2].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
        panels[2].command = 23;
        configureMovement2D(CHILD_MOVEMENT(panels[3]), 0xB4000, 0x40000, 0x1E000, 0x1E000, -1000);
        configureRegisteredChildMovement(0, panels[0].task, 0xF4000, 0, 0x1E000, 0, -1000);
        configureRegisteredChildMovement(1, panels[0].task, 0xF4000, 0, 0x1E000, 0, -1000);
        configureMovement2D(CHILD_MOVEMENT(panels[1]), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        gCurrentTask->data18 = 6;
        break;
    case 6:
        if (panels[2].flags == 0 && panels[3].flags == 0 && panels[0].flags == 0 && panels[1].flags == 0) {
            gCurrentTaskParent->data18 = 20;
            deleteCurrentTaskSubtree();
        }
        break;
    default:
        gCurrentTaskParent->data18 = 18;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskSpriteBA(void) {
    Movement2D *position;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                      0xBA, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

void drawCurrentTaskFramedSpritePair16A(void) {
    Movement2D *position;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096,
                      0x16A, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 64, position->y / 4096,
                      0x16B, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
    setTexturedQuadColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 4, position->y / 4096 - 4,
                           136, 36, 5, 1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C - 1);
}

void initializeCountedDisplayBoundsTask(void) {
    RECT *bounds;
    RECT *view;
    RECT *last;
    gCurrentTaskParent->data1C++;
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 0;
    if (gCurrentTask->resource14 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setSecondaryDisplayTaskMode(2);
    gMenuModelDrawAreas[0].x = 0;
    gMenuModelDrawAreas[0].y = 0;
    gMenuModelDrawAreas[0].w = -1;
    gMenuModelDrawAreas[0].h = -1;
    gMenuModelDrawAreas[1].x = 0;
    bounds = &gMenuModelDrawAreas[1];
    bounds->y = 0;
    bounds->w = -1;
    bounds->h = -1;
    gMenuModelDrawAreas[2].x = 24;
    view = &gMenuModelDrawAreas[2];
    view->y = 132;
    view->w = 120;
    view->h = 84;
    gMenuModelDrawAreas[3].x = 0;
    last = &gMenuModelDrawAreas[3];
    last->y = 0;
    last->w = -1;
    last->h = -1;
    setCurrentTaskCallback(updateBoardPurchaseMenuTask);
    gCurrentTask->unk10 |= 2;
}

void updateBoardPurchaseMenuTask(void) {
    FlagSelectedMenuState *input;
    RegisteredTaskEntry *entries;
    TaskPanelSelectionValue *selection;
    s16 panelValues[6];
    s32 result;
    u32 price;
    u16 player;
    s16 playerIndex;
    s16 character;
    s16 palette;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    playerIndex = (s16)gCurrentTask->data1A;
    player = gCurrentTask->data1A;
    character = gRacePlayers[playerIndex].unk009;
    palette = gRacePlayers[playerIndex].unk00D;
    selection = &gTaskPanelSelectionValues[playerIndex];

    switch (gCurrentTask->data18) {
    case 0: {
        s16 previewCharacter, previewPalette;
        s32 allOwned;
        s16 board;
        const u8 *mapping;
        s16 count;

        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        input->selectedIndex = 0;
        input->previousIndex = 0;
        input->mode = 0;
        input->choiceResult = 0;
        selection->value = input->selectedIndex + 1;
        selection->unk02 = selection->value;
        allOwned = 1;
        board = 0;
        do {
            result = hasStateGridFlag(&gPlayerSaveRecords[(s16)player], character, board);
            if (result != 1) {
                allOwned = 0;
            }
            board++;
        } while (board < 4);
        if (allOwned) {
            mapping = sBoardPurchaseFullSpriteMapping;
            count = 5;
        } else {
            mapping = sBoardPurchaseSpriteMapping;
            count = 4;
        }
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gBoardMenuSpriteEntries, mapping,
                                         NULL, 0, 0, 20, -304, -96, 0, 20, count, 0xB0,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSelectedPlayerMoneyPanel, 4, 11);
        createRegisteredCallbackTask(0, 180, 0, drawCurrentTaskBoardPrice, 4, 21);
        previewCharacter = character;
        previewPalette = palette;
        createGridModelPreviewTask(0, previewCharacter, selection->value, previewPalette,
                                   244, gPaletteSelectionMainPreviewY[previewCharacter] + 50, 384, 41);
        createRegisteredCallbackTask(0, -180, 0, drawCurrentTaskFramedSprite20C, 4, 31);
        createGridModelPreviewTask(2, previewCharacter, gRacePlayers[(s16)player].unk00A[0], previewPalette,
                                   -260, gPaletteSelectionSidePreviewY[previewCharacter] + 118, 448, 51);
        gCurrentTask->data18 = 2;
        break;
    }
    case 2:
        if (BOARD_PURCHASE_CHILDREN_READY(entries) && (s16)entries[3].flags == 0 && (s16)entries[5].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D(CHILD_MOVEMENT(entries[1]), -0x8000, -0x48000, 0x1E000, 0, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[2]), 0, 0, 0x1E000, 0, 6);
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[4]), 0x40000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), 0, 0, 0x1E000, 0x1E000, 6);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (BOARD_PURCHASE_CHILDREN_READY(entries) && (s16)entries[3].flags == 0) {
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[5]), -0x50000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[5].flags == 0) {
            GameTask *task;
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            entries[4].command = 1;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = gCurrentTask;
            gSelectionMenuFadeStarted = 0;
            task->data18 = 5;
        }
        break;
    case 5: {
        result = isTaskResourceTransitionComplete(0);
        if (result == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = result;
        }
        if (input->selectedIndex != input->previousIndex) {
            selection->unk02 = selection->value;
            if ((u16)mapSpacedGroupIndex(entries[0].task, input->selectedIndex) < 5U) {
                selection->value = input->selectedIndex + 1;
                if (selection->value != selection->unk02) {
                    entries[4].command = 13;
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (input->choiceResult < 0) {
                gCurrentTaskParent->data18 = 13;
                goto leave;
            }
            if (mapSpacedGroupIndex(entries[0].task, input->selectedIndex) == 5) {
                boardMenuRememberedSelectionAbsolute = 0;
                gCurrentTaskParent->data18 = 11;
                playSoundEffect(0, 0x202, 32);
                gCurrentTask->data18 = 15;
            } else {
                gCurrentTask->data18 = 11;
            }
        }
        break;
    }
    case 15: {
        GameTask *task = gCurrentTask;
        entries[0].command = 10;
        task->data18 = 16;
        break;
    }
    case 16:
        if ((s16)entries[0].flags == 0) {
leave:
            gCurrentTask->data18 = 6;
        }
        break;
    case 11: {
        s16 recordIndex = player;
        SaveRecord *record = &gPlayerSaveRecords[recordIndex];
        s16 boardCharacter = character;
        if (hasStateGridFlag(record, boardCharacter, selection->value) == 1) {
            gCurrentTask->data18 = 17;
        } else if ((price = gCharacterBoardPrices[boardCharacter * 5 + selection->value]) > record->money) {
            playSoundEffect(0, 0x302, 32);
            gCurrentTask->data18 = 17;
        } else {
            createRegisteredCallbackTask(recordIndex, 0, -28, drawCurrentTaskTextPanel, 2, 101);
            panelValues[0] = 0x125;
            panelValues[2] = 0x126;
            panelValues[4] = 0xB0;
            panelValues[1] = 0;
            panelValues[3] = 0;
            panelValues[5] = 0;
            createRegisteredMovingPanelTask(1, panelValues, 24, -13, 0, 16, 24, 19, 3, 111);
            playSoundEffect(0, 0x502, 32);
            gCurrentTask->data18 = 12;
        }
        break;
    }
    case 12:
        if ((s16)entries[11].flags >= 0) {
            entries[10].command = 99;
            if ((s16)entries[11].flags != 0) {
                gCurrentTask->data18 = 17;
            } else {
                price = gCharacterBoardPrices[character * 5 + selection->value];
                entries[12].task = createValueTransitionTask(
                    &gPlayerSaveRecords[(s16)player].money,
                    gPlayerSaveRecords[(s16)player].money - price, 100);
                gCurrentTask->data18 = 13;
            }
        }
        break;
    case 13: {
        GameTask *transition;
        if (gFrameCounter & 1) {
            playSoundEffect(1, 2, 32);
        }
        transition = entries[12].task;
        if (transition->data1A == 1) {
            s16 recordIndex;
            transition->data18 = 1;
            playSoundEffect(0, 0x102, 32);
            recordIndex = player;
            setStateGridFlag(&gPlayerSaveRecords[recordIndex], character, selection->value);
            createRegisteredCallbackTask(recordIndex, 0, -28, drawCurrentTaskSelectionPanel, 2, 101);
            entries[12].task = createCountdownTask(15);
            gCurrentTask->data18 = 14;
        }
        break;
    }
    case 14: {
        GameTask *countdown = entries[12].task;
        if (countdown->data1A == 1) {
            GameTask *task;
            countdown->data18 = 1;
            task = gCurrentTask;
            entries[10].command = 99;
            task->data18 = 17;
        }
        break;
    }
    case 17: {
        GameTask *task;
        input->choiceResult = 0;
        entries[0].command = 21;
        configureColorCycleTask(2, 64, 128, 4, 0);
        task = gCurrentTask;
        gSelectionMenuFadeStarted = 1;
        task->data18 = 5;
        break;
    }
    case 6:
        if (BOARD_PURCHASE_CHILDREN_READY(entries)) {
            GameTask *task;
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[5]), -0x104000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, -1000);
            task = gCurrentTask;
            entries[3].command = 23;
            task->data18 = 7;
        }
        break;
    case 7:
        if ((s16)entries[5].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(entries[1]), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[2]), 0xB4000, 0, 0x1E000, 0x1E000, -1000);
            configureMovement2D(MODEL_PREVIEW_MOVEMENT(entries[4]), 0xF4000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[3]), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
        if (BOARD_PURCHASE_CHILDREN_READY(entries) && (s16)entries[3].flags == 0) {
            gCurrentTask->data18 = 9;
        }
        break;
    case 9:
        gCurrentTask->data18 = 10;
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskSelectedPlayerMoneyPanel(void) {
    Movement2D *position;
    u16 recordIndex;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    recordIndex = gCurrentTaskParent->data1A;
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                     0xB7, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                     0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        {
            s32 amountX = (s16)(position->x / 4096 + 74);
            s32 amountY = (s16)(position->y / 4096 + 152);
            SaveRecord *records = gPlayerSaveRecords;
            drawDecimalNumber8x8(amountX, amountY, 6, 0,
                                 records[(s16)recordIndex].money,
                                 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
        drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                           2, gShopCurrencySuffix, 0, -1,
                           &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        if (gCurrentTask->data1A == 1) {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                          128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }
}

void drawCurrentTaskBoardPrice(void) {
    Movement2D *position;
    s16 player;
    s16 character;
    TaskPanelSelectionValue *selection;
    s32 x, y, fixedY;
    u8 offset;
    s16 priceX, priceY;
    u32 price, remaining;
    s16 digits;
    u32 halfWidth;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    player = gCurrentTaskParent->data1A;
    character = gRacePlayers[player].unk009;
    selection = &gTaskPanelSelectionValues[player];
    x = position->x / 4096;
    priceX = x;
    fixedY = position->y;
    if (fixedY < 0) {
        fixedY += 4095;
    }
    offset = sBoardPriceVerticalOffsets[character];
    fixedY >>= 12;
    y = (s8)offset + fixedY;
    priceY = y;

    if (gCurrentTask->data1A != 0) {
        SaveRecord *record = gPlayerSaveRecords + player;
        s16 column = selection->value;

        price = gCharacterBoardPrices[character * 5 + column];
        if (hasStateGridFlag(record, character, column) == 1) {
            drawSpriteByIndexWithBlend(x + 36, y + 8, 0xC3, 15, -1,
                          &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            remaining = price;
            digits = 0;
            while (remaining != 0) {
                remaining /= 10;
                digits++;
            }
            if (digits < 5) {
                digits = 5;
            }
            halfWidth = digits * 4;
            priceX -= halfWidth - 64;
            drawDecimalNumber8x8(priceX, priceY + 12, digits, 0, price, 0, -1,
                                 &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawWrappedText8x8(priceX + digits * 8, priceY + 12, 2, gShopCurrencySuffix, 0, -1,
                               &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            if (gCurrentTask->data1A == 1) {
                addTilePacketWithBlend(priceX - 4, priceY + 8, digits * 8 + 16, 16,
                              gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            } else {
                addTilePacketWithBlend(priceX - 4, priceY + 8, digits * 8 + 16, 16, 128, 128, 128, 2,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
        }
    }
}

s32 hasStateGridFlag(SaveRecord *state, s16 row, s16 column) {
    if (row < 5) {
        if (state->boardUnlockFlagsLow & (1 << (row * 5 + column))) {
            return 1;
        }
    } else if (row < 10) {
        if (state->boardUnlockFlagsHigh & (1 << ((row - 5) * 5 + column))) {
            return 1;
        }
    }
    return 0;
}

void setStateGridFlag(SaveRecord *state, s16 row, s16 column) {
    if (row < 5) {
        state->boardUnlockFlagsLow |= 1 << (row * 5 + column);
    } else if (row < 10) {
        state->boardUnlockFlagsHigh |= 1 << ((row - 5) * 5 + column);
    }
}

void drawCurrentTaskFramedSprite20C(void) {
    Movement2D *position;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    setTexturedQuadColor(128, 128, 128);
    setSpriteColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 140, position->y / 4096 + 8,
                           128, 92, 5, 1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawSpriteByIndexWithBlend(position->x / 4096 - 136, position->y / 4096 + 12,
                  0x20C, 1, 0, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void drawCurrentTaskTextPanel(void) {
    Movement2D *position;
    s32 x, y;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16((s16)(x + 16), (s16)(y + 12), 10, gCurrentTaskPanelText,
                              8, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void drawCurrentTaskSelectionPanel(void) {
    Movement2D *position;
    s32 x, y;
    TaskPanelSelectionValue *selection;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    selection = &gTaskPanelSelectionValues[(s16)gCurrentTaskParent->data1A];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16((s16)(x + 8), (s16)(y + 4), 10, gTaskSelectionPanelLabel,
                              8, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDecimalNumber16((s16)(x + 56), (s16)(y + 4), 1, 0, selection->value + 1,
                 8, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawWrappedGlyphText16x16((s16)(x + 8), (s16)(y + 20), 10, gTaskSelectionPanelText,
                              8, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void queueGridResourceSectorReads(s16 slot, s16 row, s16 column) {
    if (row < 10) {
        queueCdSectorCommand(0, gGridResourceFirstSectorRanges[row][column].sector,
                            gGridResourceFirstSectorRanges[row][column].sectorCount,
                            DAT_GRID_MODEL_BUFFERS + slot * 0x4000, 0);
        queueCdSectorCommand(0, gGridResourceSecondSectorRanges[row][column].sector,
                            gGridResourceSecondSectorRanges[row][column].sectorCount,
                            DAT_GRID_TEXTURE_BUFFERS + slot * 0x4000, 0);
    } else {
        queueCdSectorCommand(0, DAT_GRID_FALLBACK_SECTOR, DAT_GRID_FALLBACK_SECTOR_COUNT, DAT_GRID_MODEL_BUFFERS + slot * 0x4000, 0);
    }
}

void createGridModelPreviewTask(s16 playerIndex, s16 character, s16 modelSlot, s16 paletteRow,
                                s16 x, s16 y, s16 z, s32 index) {
    GridModelPreviewState *state;
    Movement2D *position;
    RegisteredTaskEntry *entry;
    u32 *source;
    u32 *destination;
    u32 *model;
    s32 i;
    u32 (*sourceBuffers)[4096];
    u32 (*destinationBuffers)[4096];
    RacePlayer *attributePlayer;
    RacePlayer *positionPlayer;

    createChildTaskWithReserve(updateGridModelPreviewTask);
    i = 0;
    gCreatedTask->data1A = modelSlot;
    gCreatedTask->data1C = playerIndex;
    gCreatedTask->data1E = index;
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    sourceBuffers = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
    destinationBuffers = DAT_PLAYER_MODEL_BUFFERS;
    currentPlayerAbsolute = &gRacePlayers[gCreatedTask->data1C];
    state = (GridModelPreviewState *)&gTaskMemoryPool[gCreatedTask->resource14];
    currentPlayerAbsolute->characterId = character;
    currentPlayerAbsolute->unk015[0] = modelSlot;
    currentPlayerAbsolute->unk015[3] = paletteRow;

    destination = destinationBuffers[gCreatedTask->data1C];
    source = sourceBuffers[gCreatedTask->data1A];
    for (; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[gCreatedTask->data1C];
    model++;
    GsMapModelingData(model);
    model++;
    currentPlayerAbsolute->modelPartCount = *(u8 *)model;
    model++;
    if (gCreatedTask->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)model, &gPlayerRootTextureOffsets[gCreatedTask->data1C]);
        for (i = 1; i < currentPlayerAbsolute->modelPartCount; i++) {
            offsetTmdObjectTextures(i, (TmdObject *)model, &gPlayerPartTextureOffsets[gCreatedTask->data1C]);
        }
    }

    GsInitCoordinate2(0, &currentPlayerAbsolute->modelCoordinate);
    for (i = 0; i < currentPlayerAbsolute->modelPartCount; i++) {
        GsDOBJ2 *object;

        GsLinkObject4(model, &currentPlayerAbsolute->modelObjects[i], i);
        object = &currentPlayerAbsolute->modelObjects[i];
        GsInitCoordinate2(&currentPlayerAbsolute->modelCoordinate, &currentPlayerAbsolute->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &currentPlayerAbsolute->model.modelCoordinates[i];
    }

    attributePlayer = currentPlayerAbsolute;
    if (attributePlayer->modelPartCount >= 2) {
        attributePlayer->modelObjects[0].attribute |= GS_MODEL_OBJECT_HIDDEN;
    }
    uploadTimImageWithOffset(((u32 (*)[4096])DAT_GRID_TEXTURE_BUFFERS)[gCreatedTask->data1A],
                             (s16 *)&gPlayerTextureOrigins[gCreatedTask->data1C]);

    positionPlayer = currentPlayerAbsolute;
    positionPlayer->modelCoordinate.coord.t[0] = x;
    positionPlayer->modelCoordinate.coord.t[1] = y;
    positionPlayer->modelCoordinate.coord.t[2] = z;
    positionPlayer->unk0B4 = 0;
    positionPlayer->unk0B6 = 0;
    positionPlayer->unk0B8 = 0;
    state->rotationStep = 0;
    if (character < 10) {
        loadCurrentPlayerModelPose(gGridModelPreviewPoses[character]);
    }

    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    position = (Movement2D *)&gTaskMemoryPool[gCreatedTask->resource16];
    initMovement2D(position, x << 12, y << 12);
    position->unk08 = z;

    if (index % 10 == 0) {
        if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTask->resource14, index);
    } else {
        if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
        entry = getRegisteredPanelEntry(gCurrentTask->resource16, index);
    }
    entry->command = 0;
    entry->flags = 1;
    entry->task = gCreatedTask;
    copyPlayerPaletteRowWithTransition(playerIndex, character, paletteRow, 0);
}

void reloadGridModelPreview(GameTask *task, s16 character, s16 modelSlot, s16 paletteRow,
                            s16 x, s16 y, s16 z) {
    GridModelPreviewState *state = (GridModelPreviewState *)&gTaskMemoryPool[task->resource14];
    s16 playerIndex = task->data1C;
    GridModelPreviewState saved;
    SVECTOR rotation;
    u32 *source;
    u32 *destination;
    u32 *model;
    s32 i;
    RacePlayer *rotationPlayer;
    RacePlayer *attributePlayer;
    RacePlayer *positionPlayer;

    task->data1A = modelSlot;
    saved = *state;
    currentPlayerAbsolute = &gRacePlayers[task->data1C];
    currentPlayerAbsolute->characterId = character;
    currentPlayerAbsolute->unk015[0] = modelSlot;
    currentPlayerAbsolute->unk015[3] = paletteRow;
    rotationPlayer = currentPlayerAbsolute;
    rotation.vx = rotationPlayer->unk0B4;
    rotation.vy = rotationPlayer->unk0B6;
    rotation.vz = rotationPlayer->unk0B8;

    {
        u32 (*sourceBuffers)[4096] = (u32 (*)[4096])DAT_GRID_MODEL_BUFFERS;
        u32 (*destinationBuffers)[4096] = DAT_PLAYER_MODEL_BUFFERS;

        destination = destinationBuffers[task->data1C];
        source = sourceBuffers[task->data1A];
    }
    for (i = 0; i < 4096; i++) {
        *destination++ = *source++;
    }

    model = DAT_PLAYER_MODEL_BUFFERS[task->data1C];
    model++;
    GsMapModelingData(model);
    model++;
    currentPlayerAbsolute->modelPartCount = *(u8 *)model;
    model++;
    if (task->data1C != 0) {
        offsetTmdObjectTextures(0, (TmdObject *)model, &gPlayerRootTextureOffsets[task->data1C]);
        for (i = 1; i < currentPlayerAbsolute->modelPartCount; i++) {
            offsetTmdObjectTextures(i, (TmdObject *)model, &gPlayerPartTextureOffsets[task->data1C]);
        }
    }

    GsInitCoordinate2(0, &currentPlayerAbsolute->modelCoordinate);
    for (i = 0; i < currentPlayerAbsolute->modelPartCount; i++) {
        GsDOBJ2 *object;

        GsLinkObject4(model, &currentPlayerAbsolute->modelObjects[i], i);
        object = &currentPlayerAbsolute->modelObjects[i];
        GsInitCoordinate2(&currentPlayerAbsolute->modelCoordinate, &currentPlayerAbsolute->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &currentPlayerAbsolute->model.modelCoordinates[i];
    }

    attributePlayer = currentPlayerAbsolute;
    if (attributePlayer->modelPartCount >= 2) {
        attributePlayer->modelObjects[0].attribute |= GS_MODEL_OBJECT_HIDDEN;
    }
    uploadTimImageWithOffset(((u32 (*)[4096])DAT_GRID_TEXTURE_BUFFERS)[task->data1A],
                             (s16 *)&gPlayerTextureOrigins[task->data1C]);

    positionPlayer = currentPlayerAbsolute;
    positionPlayer->modelCoordinate.coord.t[0] = x;
    positionPlayer->modelCoordinate.coord.t[1] = y;
    positionPlayer->modelCoordinate.coord.t[2] = z;
    positionPlayer->unk0B4 = rotation.vx;
    positionPlayer->unk0B6 = rotation.vy;
    positionPlayer->unk0B8 = rotation.vz;
    state->rotationStep = saved.rotationStep;
    if (character < 10) {
        loadCurrentPlayerModelPose(gGridModelPreviewPoses[character]);
    }
    initMovement2D((Movement2D *)&gTaskMemoryPool[task->resource16], x << 12, y << 12);
    copyPlayerPaletteRowWithTransition(playerIndex, character, paletteRow, 0);
}

void updateGridModelPreviewTask(void) {
    u8 padding[0x10];
    GridModelPreviewState *state;
    Movement2D *position;
    TaskPanelSelectionValue *selection;
    RegisteredTaskEntry *entry;

    currentPlayerAbsolute = &gRacePlayers[gCurrentTask->data1C];
    selection = &gTaskPanelSelectionValues[(s16)gCurrentTaskParent->data1A];
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    state = (GridModelPreviewState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource16];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(gCurrentTaskParent->resource14, gCurrentTask->data1E);
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entry = getModelPreviewEntry(gCurrentTaskParent->resource16, gCurrentTask->data1E);
    }

    switch (entry->command) {
    case 1:
        state->rotationStep = 1;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 2:
        state->rotationStep = -32;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 3:
        state->rotationStep = 0;
        entry->command = 0;
        entry->flags = 0;
        break;
    case 0:
        entry->flags = 0;
        break;
    case 13:
        reloadGridModelPreview(gCurrentTask, gRacePlayers[gCurrentTask->data1C].characterId,
                               selection->value, gRacePlayers[gCurrentTask->data1C].unk015[3],
                               position->x / 4096, position->y / 4096, position->unk08);
        entry->command = 0;
        entry->flags = 0;
        selection->unk02 = selection->value;
        break;
    case 99:
    default:
        entry->task = 0;
        entry->flags = 0;
        deleteCurrentTaskSubtree();
        return;
    }

    entry->flags |= updateMovement2D(position);
    currentPlayerAbsolute->modelCoordinate.coord.t[0] = position->x / 4096;
    currentPlayerAbsolute->modelCoordinate.coord.t[1] = position->y / 4096;
    currentPlayerAbsolute->modelCoordinate.coord.t[2] = position->unk08;
    if (state->rotationStep > 0) {
        currentPlayerAbsolute->unk0B6 = (-32 * gFrameCounter) & 0xFFF;
    } else {
        currentPlayerAbsolute->unk0B6 = 0x480;
    }
    setCoordinateRotation((SVECTOR *)&currentPlayerAbsolute->unk0B4, &currentPlayerAbsolute->modelCoordinate);
    sortPlayerModelWithDrawArea(gCurrentTask->data1C, gCurrentTask->data1C, &gMenuModelDrawAreas[gCurrentTask->data1C]);
}
