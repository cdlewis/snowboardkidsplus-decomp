#include "common.h"
#include "libsnd_private.h"

/* SDK wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssclose", _SsClose);

void SsSeqClose(s16 id) { _SsClose(id); }

void SsSepClose(s16 id) { _SsClose(id); }
