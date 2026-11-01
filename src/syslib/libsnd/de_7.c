#include "libsnd_private.h"
void _SsSetNrpnVabAttr7(s16 a, s16 b, s16 c, VagAtr attr, s16 d, u8 value) {
    Envelope envelope;
    SsUtGetVagAtr(a, b, c, &attr);
    _SsUtResolveADSR(attr.adsr1, attr.adsr2, &envelope);
    envelope.sustain_level = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(a, b, c, &attr);
}
