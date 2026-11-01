#include "libsnd_private.h"

extern u16 D_8013CFDA, D_8013CEC0, D_8013CEC2, D_8013C7B8, D_8013C7BA;
void _SsVmKeyOffNow(int mode) {
    int upper, lower;
    u16 voice = D_8013CFDA;
    if (voice < 16) {
        lower = 1 << voice;
        upper = 0;
    } else {
        lower = 0;
        upper = 1 << (voice - 16);
    }
    D_8013C7C8[voice].field1d = 0;
    D_8013C7C8[voice].field4 = 0;
    D_8013C7C8[voice].field0 = 0;
    D_8013CEC0 |= lower;
    D_8013CEC2 |= upper;
    D_8013C7B8 &= ~D_8013CEC0;
    D_8013C7BA &= ~D_8013CEC2;
}
