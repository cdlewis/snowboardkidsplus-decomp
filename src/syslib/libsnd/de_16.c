#include "libsnd_private.h"
#include "psx/libsnd.h"


void _SsSetNrpnVabAttr16(s16 a, s16 b, s16 c, VagAtr attr, s16 d, u8 value) {
    SsUtSetReverbDepth(value, value);
}
