#include "psx/libetc.h"

/* Keep this four-byte data input separate from the SDK text padding.
 * SetVideoMode returns the previous mode. */
s32 gVideoMode __attribute__((section(".data.vmode"))) = 0;

s32 SetVideoMode(s32 mode) {
    s32 previous = gVideoMode;
    gVideoMode = mode;
    return previous;
}

s32 GetVideoMode(void) { return gVideoMode; }
