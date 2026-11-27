#include "game/menu/task_widgets.h"
#include "task_widget_data.h"

/* Separate definitions preserve the widget consumers' original address scheduling. */
u32 gTexturePairDimMask __attribute__((section(".sdata.texture_pair_dim_mask"))) = 0;

TexturedQuadListEntry *gPlayerPanelQuadLists[2]
    __attribute__((section(".data.player_panel_quad_lists"))) = {
    gPlayerPanelQuadsPalette0, gPlayerPanelQuadsPalette1
};
