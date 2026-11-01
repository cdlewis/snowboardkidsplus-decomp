#include "psx/libspu.h"

void SsSetMVol(s16 left, s16 right) {
    SpuCommonAttr attr;

    attr.mask = 3;
    attr.mvol.left = left * 129;
    attr.mvol.right = right * 129;
    SpuSetCommonAttr(&attr);
}
