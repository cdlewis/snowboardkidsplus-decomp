#pragma once

#include "decomp/types.h"

typedef struct CVECTOR {
    u8 r, g, b, cd;
} CVECTOR;

/* Psy-Q inline_c.h: store the GTE depth cue interpolation value. */
#define gte_stdp(r0) \
    __asm__ volatile("swc2 $8, 0(%0)" : : "r"(r0) : "memory")

void DpqColor(CVECTOR *color, s32 depthCue, CVECTOR *out);

typedef struct {
    s32 vx, vy, vz, pad;
} VECTOR;

VECTOR *OuterProduct0(VECTOR *first, VECTOR *second, VECTOR *result);
long SquareRoot0(long value);

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_ldv3( r0, r1, r2 ) __asm__ volatile ( \
    "lwc2    $0, 0( %0 );" \
    "lwc2    $1, 4( %0 );" \
    "lwc2    $2, 0( %1 );" \
    "lwc2    $3, 4( %1 );" \
    "lwc2    $4, 0( %2 );" \
    "lwc2    $5, 4( %2 )" \
    : \
    : "r"( r0 ), "r"( r1 ), "r"( r2 ) )

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_rtpt() __asm__ volatile ( \
    "nop;" \
    "nop;" \
    "rtpt" )

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_stsxy3( r0, r1, r2 ) __asm__ volatile ( \
    "swc2    $12, 0( %0 );" \
    "swc2    $13, 0( %1 );" \
    "swc2    $14, 0( %2 )" \
    : \
    : "r"( r0 ), "r"( r1 ), "r"( r2 ) \
    : "memory" )

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_nclip() __asm__ volatile ( \
    "nop;" \
    "nop;" \
    "nclip" )

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_stopz( r0 ) __asm__ volatile ( \
    "swc2    $24, 0( %0 )" \
    : \
    : "r"( r0 ) \
    : "memory" )

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_stflg( r0 ) __asm__ volatile ( \
    "cfc2    $12, $31;" \
    "nop;" \
    "sw    $12, 0( %0 )" \
    : \
    : "r"( r0 ) \
    : "$12", "memory" )

/* Psy-Q 3.6 INLINE_C.H. */
#define gte_stszotz( r0 ) __asm__ volatile ( \
    "mfc2    $12, $19;" \
    "nop;" \
    "sra    $12, $12, 2;" \
    "sw    $12, 0( %0 )" \
    : \
    : "r"( r0 ) \
    : "$12", "memory" )

/* Psy-Q INLINE_C.H. */
#define gte_ldv0(r0) __asm__ volatile ( \
    "lwc2 $0, 0(%0);" \
    "lwc2 $1, 4(%0)" \
    : : "r"(r0))

/* Psy-Q INLINE_C.H. */
#define gte_rtps() __asm__ volatile ( \
    "nop;" \
    "nop;" \
    "rtps")

/* Psy-Q INLINE_C.H. */
#define gte_stsxy(r0) __asm__ volatile ( \
    "swc2 $14, 0(%0)" \
    : : "r"(r0) : "memory")
