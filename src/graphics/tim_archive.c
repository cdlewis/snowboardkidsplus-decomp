#include "render_effects_internal.h"

void unusedVariadicStub(s32 unused0, s32 unused1, s32 unused2, ...) {
}

s32 loadTimArchive(OffsetArchive *archive) {
    u32 i;
    u32 count = archive->count;
    for (i = 1; i < count + 1; i++) {
        if (!loadTimImage((u32 *)((u8 *)archive + archive->offsets[i - 1]))) {
            return 0;
        }
    }
    return 1;
}

s32 loadTimImage(u32 *tim) {
    GsIMAGE image;
    if (*tim != 0x10) {
        return 0;
    }
    GsGetTimInfo(tim + 1, &image);
    LoadImage((RECT *)&image.px, image.pixel);
    DrawSync(0);
    LoadImage((RECT *)&image.cx, image.clut);
    DrawSync(0);
    return 1;
}
