#ifndef GAME_OFFSET_ARCHIVE_H
#define GAME_OFFSET_ARCHIVE_H

#include "decomp/types.h"

/* On-disc archive: count followed by count offsets relative to this header.
 * Actual archives have variable length; the final array is a C89 tail array.
 */
typedef struct OffsetArchive {
    u32 count;
    u32 offsets[1];
} OffsetArchive;

void *getOffsetArchiveEntry(OffsetArchive *archive, s32 index);

#endif
