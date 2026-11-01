#include "libsnd_private.h"

void _SsSetNrpnVabAttr12(s16 a, s16 b, s16 c, VagAtr attr, s16 d, u8 value) {
    Envelope envelope; /* The original leaves the other fields uninitialized. */
    SsUtGetVagAtr(a, b, c, &attr);
    if ((u8)(value - 1) < 63)
        envelope.sustain_direction = 0;
    else if ((u8)(value - 64) < 64)
        envelope.sustain_direction = 1;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(a, b, c, &attr);
}
