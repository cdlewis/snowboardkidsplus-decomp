#include "psx/libgpu.h"

void AddPrim(void *ordering_table, void *primitive) {
    P_TAG *ot = ordering_table;
    P_TAG *p = primitive;

    p->addr = ot->addr;
    ot->addr = (u32)p;
}
