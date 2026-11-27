#include "libspu_private.h"

#define REVERB_DATA __attribute__((section(".data.spu_reverb_start_addresses"), aligned(4)))

s32 _spu_rev_startaddr[10] REVERB_DATA = {
    0xFFFE, 0xFB28, 0xFC18, 0xF6F8, 0xF204,
    0xEA44, 0xE128, 0xCFF8, 0xCFF8, 0xF880,
};

/* S_RMP0's original data section includes eight trailing zero bytes. */
static u32 sSpuReverbStartAddressPadding[2] REVERB_DATA = {0, 0};
