#include "decomp/types.h"

u8 gPlayerGroupedSelectionStates[2]
    __attribute__((section(".sdata.player_grouped_selection_states"))) = {0};

const u8 gPlayerGroupedSelectionStatesPadding[6]
    __attribute__((section(".data.player_grouped_selection_states_padding"))) = {0};
