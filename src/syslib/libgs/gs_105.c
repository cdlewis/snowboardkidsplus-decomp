#include "common.h"

#ifdef NON_MATCHING
typedef struct {
    u32 vertices, nvertices, normals, nnormals, primitives, nprimitives, scale;
} TmdObject;

/* Best observed instruction-sequence similarity: 40.00% (12/30 words;
 * GCC 2.8.0). */
void GsMapModelingData(u32 *data) {
    int i, count;
    TmdObject *base, *object;

    if (*data & 1)
        return;
    *data |= 1;
    data++;
    count = *data++;
    base = (TmdObject *)data;
    object = base;
    for (i = 0; i < count; i++, object++) {
        object->vertices += (u32)base;
        object->primitives += (u32)base;
        object->normals += (u32)base;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_105", GsMapModelingData);
#endif
