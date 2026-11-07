#pragma once

#include "decomp/types.h"

/* On-disc archive: count followed by count offsets relative to this header.
 * Actual archives have variable length; the final array is a C89 tail array.
 */
typedef struct OffsetArchive {
    u32 count;
    u32 offsets[1];
} OffsetArchive;

void *getOffsetArchiveEntry(OffsetArchive *archive, s32 index);

/* Load each TIM entry into VRAM; stop and return zero for an invalid TIM. */
s32 loadTimArchive(OffsetArchive *archive);

/* Upload a TIM image and palette; return zero for an invalid magic word. */
s32 loadTimImage(u32 *tim);
