#include "psx/libgs.h"

extern void DrawOTag(u32 *tag);

void GsDrawOt(GsOT *ot) { DrawOTag(ot->tag); }
