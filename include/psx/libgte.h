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

/* Psy-Q 4.2 INLINE_C.H. */
#define gte_stsz3(first, second, third) __asm__ volatile ( \
    "swc2 $17, 0(%0);" \
    "swc2 $18, 0(%1);" \
    "swc2 $19, 0(%2)" \
    : : "r"(first), "r"(second), "r"(third) : "memory")

/* Psy-Q 4.2 INLINE_C.H. */
#define gte_avsz3() __asm__ volatile ("nop; nop; avsz3")
#define gte_stotz(result) \
    __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(result) : "memory")

/* Psy-Q 4.2 INLINE_C.H: write projected coordinates to triangle packets. */
#define gte_stsxy3_f3(packet) __asm__ volatile ( \
    "swc2 $12, 8(%0);" \
    "swc2 $13, 12(%0);" \
    "swc2 $14, 16(%0)" \
    : : "r"(packet) : "memory")

#define gte_stsxy3_g3(packet) __asm__ volatile ( \
    "swc2 $12, 8(%0);" \
    "swc2 $13, 16(%0);" \
    "swc2 $14, 24(%0)" \
    : : "r"(packet) : "memory")

#define gte_stsxy3_ft3(packet) __asm__ volatile ( \
    "swc2 $12, 8(%0);" \
    "swc2 $13, 16(%0);" \
    "swc2 $14, 24(%0)" \
    : : "r"(packet) : "memory")

#define gte_stsxy3_gt3(packet) __asm__ volatile ( \
    "swc2 $12, 8(%0);" \
    "swc2 $13, 20(%0);" \
    "swc2 $14, 32(%0)" \
    : : "r"(packet) : "memory")

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

/* Psy-Q INLINE_C.H. */
#define gte_ldrgb(r0) __asm__ volatile ( \
    "lwc2 $6, 0(%0)" \
    : : "r"(r0))

/* Psy-Q INLINE_C.H. */
#define gte_nccs() __asm__ volatile ( \
    "nop;" \
    "nop;" \
    "nccs")

/* Psy-Q INLINE_C.H. */
#define gte_strgb(r0) __asm__ volatile ( \
    "swc2 $22, 0(%0)" \
    : : "r"(r0) : "memory")
