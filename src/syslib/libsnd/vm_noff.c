#include "libsnd_private.h"

void vmNoiseOff(u8 voice) {
    D_8013C7C8[voice].field1d = 0;
    D_8013C7C8[voice].field0 = 0;
    D_8013C7C8[voice].field4 = 0;
}
