#include "libsnd_private.h"
void _SsSetNrpnVabAttr3(s16 a, s16 b, s16 c, VagAtr attr, s16 d, unsigned int value) {
    SsUtGetVagAtr(a, b, c, &attr);
    attr.max = value;
    SsUtSetVagAtr(a, b, c, &attr);
}
