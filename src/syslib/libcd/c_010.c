#include "common.h"

/* Streaming state is owned by the CD streaming data object. */
extern s32 StSTART_FLAG, StStartFrame, StEndFrame;

void StSetMask(u32 mask, u32 start, u32 end) {
    StSTART_FLAG = mask;
    StStartFrame = start;
    StEndFrame = end;
}
