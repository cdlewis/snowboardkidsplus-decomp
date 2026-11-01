#include "libsnd_private.h"
void _SsSetNrpnVabAttr4(s16 a, s16 b, s16 c, VagAtr attr, s16 d, u8 value) {
    Envelope envelope;
    SsUtGetVagAtr(a, b, c, &attr);
    _SsUtResolveADSR(attr.adsr1, attr.adsr2, &envelope);
    envelope.attack_mode = 0;
    envelope.attack_rate = value;
    _SsUtBuildADSR(&envelope, &attr.adsr1, &attr.adsr2);
    SsUtSetVagAtr(a, b, c, &attr);
}
