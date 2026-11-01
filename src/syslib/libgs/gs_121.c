#include "common.h"

extern void InitGeom(void), SetFarColor(int, int, int), SetGeomOffset(int, int);
extern s16 D_8013F756, D_8013F754;

void gte_init(void) {
    InitGeom();
    SetFarColor(0, 0, 0);
    SetGeomOffset(0, 0);
    D_8013F756 = 0;
    D_8013F754 = 0;
}
