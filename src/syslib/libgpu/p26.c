#include "psx/libgpu.h"

void SetTile(P_TAG *packet) { setlen(packet, 3); packet->code = 0x60; }
