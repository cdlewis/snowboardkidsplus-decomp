#include "game/graphics/graphics.h"
#include "psx/libetc.h"

u32 gSynchronizedFrameCount __attribute__((section(".sdata.synchronized_frame_count"))) = 0;

void waitForFrameHcount(u32 minimumHcount) {
    DrawSync(0);
    while ((u32)VSync(1) < minimumHcount) {
    }
    VSync(0);
    gSynchronizedFrameCount++;
}
