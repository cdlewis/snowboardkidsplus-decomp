#include "psx/libspu.h"
void SsSetSerialAttr(s8 serial, s8 attr, s8 value) {
    SpuCommonAttr common;
    if (serial == 0) {
        if (attr == 0) {
            common.mask = 0x200;
            common.cd.mix = value;
        }
        if (attr == 1) {
            common.mask = 0x100;
            common.cd.reverb = value;
        }
    }
    if (serial == 1) {
        if (attr == 0) {
            common.mask = 0x2000;
            common.ext.mix = value;
        }
        if (attr == 1) {
            common.mask = 0x1000;
            common.ext.reverb = value;
        }
    }
    SpuSetCommonAttr(&common);
}
