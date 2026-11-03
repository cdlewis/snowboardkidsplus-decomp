#include "common.h"

extern void InitGeom(void), SetFarColor(int, int, int), SetGeomOffset(int, int);
extern s16 GsORGOFSY, GsORGOFSX;

void gte_init(void) {
    InitGeom();
    SetFarColor(0, 0, 0);
    SetGeomOffset(0, 0);
    GsORGOFSY = 0;
    GsORGOFSX = 0;
}
