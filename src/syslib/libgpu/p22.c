#include "psx/libgpu.h"

void SetSprt(P_TAG *packet) { setlen(packet, 4); packet->code = 0x64; }
