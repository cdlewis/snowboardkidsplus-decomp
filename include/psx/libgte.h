#pragma once

#include "decomp/types.h"
#include "../../lib/psyq-sdk/4.2/INCLUDE/INLINE_C.H"

/* Psy-Q emits DMPSX placeholders for these commands. Use the mnemonics
 * provided by gte_macros.inc for GNU as and keep the SDK delay slots. */
#undef gte_rtps
#undef gte_rtpt
#undef gte_nccs
#undef gte_nclip
#undef gte_avsz3
#undef gte_avsz4

#define gte_rtps() __asm__ volatile ("nop; nop; rtps")
#define gte_rtpt() __asm__ volatile ("nop; nop; rtpt")
#define gte_nccs() __asm__ volatile ("nop; nop; nccs")
#define gte_nclip() __asm__ volatile ("nop; nop; nclip")
#define gte_avsz3() __asm__ volatile ("nop; nop; avsz3")
#define gte_avsz4() __asm__ volatile ("nop; nop; avsz4")

typedef struct CVECTOR {
    u8 r, g, b, cd;
} CVECTOR;

void DpqColor(CVECTOR *color, s32 depthCue, CVECTOR *out);

typedef struct {
    s32 vx, vy, vz, pad;
} VECTOR;

VECTOR *OuterProduct0(VECTOR *first, VECTOR *second, VECTOR *result);
long SquareRoot0(long value);
