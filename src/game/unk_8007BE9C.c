#include "common.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "psx/libsnd.h"

extern u8 D_8013AFE2 __attribute__((section(".data")));

INCLUDE_ASM("asm/nonmatchings/game/unk_8007BE9C", func_8007BE9C);

s32 getValidatedSelectionIndex(void) {
    switch (D_8013AFE2) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        return D_8013AFE2;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8007BE9C", func_8007C7C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8007BE9C", func_8007CB90);
