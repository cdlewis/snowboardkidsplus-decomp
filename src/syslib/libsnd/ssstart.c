#include "common.h"
#include "libsnd_private.h"

/* Start wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssstart", _SsStart);

void SsStart(void) { _SsStart(1); }

void SsStart2(void) { _SsStart(0); }

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssstart", func_800AC754);

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssstart", func_800AC7A0);
