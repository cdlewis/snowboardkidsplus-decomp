#include "common.h"
#include "libsnd_private.h"

extern void func_800B0604(void), func_800B0624(void);

void _SsSetNrpnVabAttr1(s16 vab_id, s16 program, s16 tone, VagAtr attr, s16 function, u8 value) {
    SsUtGetVagAtr(vab_id, program, tone, &attr);
    attr.mode = value;
    SsUtSetVagAtr(vab_id, program, tone, &attr);
    if (value == 0)
        func_800B0604();
    else if (value == 4)
        func_800B0624();
}
