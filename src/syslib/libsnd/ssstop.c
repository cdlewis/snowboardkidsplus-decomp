#include "common.h"
#include "libsnd_private.h"

/* SDK wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssstop", _SsSndStop);

void SsSeqStop(s16 id) { _SsSndStop(id, 0); }

void SsSepStop(s16 id,s16 sequence) { _SsSndStop(id, sequence); }
