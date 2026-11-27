#include "common.h"

#include "psx/libgs.h"

void GsSetWorkBase(PACKET *value) {
    GsOUT_PACKET_P = value;
}
