#include "psx/libspu.h"
void SsSetSerialVol(s8 serial, s16 left, s16 right) {
    SpuCommonAttr attr;
    if (serial == 0) {
        attr.mask = 0xc0;
        if (left >= 128) left = 127;
        if (right >= 128) right = 127;
        attr.cd.volume.left = left * 258;
        attr.cd.volume.right = right * 258;
    }
    if (serial == 1) {
        attr.mask = 0xc00;
        if (left >= 128) left = 127;
        if (right >= 128) right = 127;
        attr.ext.volume.left = left * 258;
        attr.ext.volume.right = right * 258;
    }
    SpuSetCommonAttr(&attr);
}
