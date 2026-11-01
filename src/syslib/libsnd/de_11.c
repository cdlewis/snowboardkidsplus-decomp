#include "libsnd_private.h"
void _SsSetNrpnVabAttr11(s16 a, s16 b, s16 c, VagAtr attr, s16 d, u8 value) {
    Envelope envelope; /* The original leaves other fields uninitialized. */
    SsUtGetVagAtr(a, b, c, &attr);
    envelope.release_mode = 1;
    envelope.release_rate = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(a, b, c, &attr);
}
