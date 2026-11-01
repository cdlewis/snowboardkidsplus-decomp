#include "libsnd_private.h"
void _SsSetNrpnVabAttr14(s16 a, s16 b, s16 c, VagAtr attr, s16 d, unsigned int value) {
    Envelope envelope; /* The original passes this uninitialized to the builder. */
    SsUtGetVagAtr(a, b, c, &attr);
    attr.porW = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(a, b, c, &attr);
}
