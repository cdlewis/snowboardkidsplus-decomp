#include "psx/libgpu.h"

void SetPolyFT4(P_TAG *packet) { setlen(packet, 9); packet->code = 0x2c; }
