#include "common.h"

extern void SetBackColor(s32 red, s32 green, s32 blue);

void GsSetAmbient(s32 red, s32 green, s32 blue) {
    SetBackColor(red >> 4, green >> 4, blue >> 4);
}
