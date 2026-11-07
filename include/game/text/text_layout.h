#pragma once

#include "decomp/types.h"

s16 parseDecimalS16(const u8 *text, s16 *result);
s16 advanceTextLine(const s8 **text, s16 limit);
s16 countWrappedTextLines(const s8 *text, s16 width, s16 limit);
