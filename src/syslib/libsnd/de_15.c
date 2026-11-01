#include "libsnd_private.h"

extern s16 SsUtSetReverbType(s16 type);

void _SsSetNrpnVabAttr15(s16 a, s16 b, s16 c, VagAtr attr, s16 d, u8 value) {
    SsUtSetReverbType(value);
}
