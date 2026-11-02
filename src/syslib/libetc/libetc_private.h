#pragma once

#include "psx/libetc.h"

/* Dispatch storage and initialization remain owned by the interrupt setup
 * routines below the public wrappers in intr.c, which are still assembly.
 */
typedef struct InterruptDispatch {
    const char *version;
    DmaCallback (*dma)(s32 channel, DmaCallback callback);
    DmaCallback (*interrupt)(s32 channel, DmaCallback callback);
    s32 (*start)(void);
    s32 (*stop)(void);
    DmaCallback (*vsync)(s32 channel, DmaCallback callback);
    s32 (*restart)(void);
    s16 *state;
} InterruptDispatch;

extern InterruptDispatch *gInterruptDispatch;
extern volatile u16 *gInterruptMaskRegister;
/* The context flag is at offset 2 in the shared interrupt-state object. */
extern u16 gInterruptCallbackContext;
