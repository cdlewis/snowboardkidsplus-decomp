#include "common.h"
#include "libsnd_private.h"

/* SDK wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

s16 SsVabOpenHead(u8 *address,s16 id) { return SsVabOpenHeadWithMode(address,id,0,0); }

s16 SsVabOpenHeadSticky(u8 *address,s16 id,u32 start) { return SsVabOpenHeadWithMode(address,id,1,start); }

s16 SsVabFakeHead(u8 *address,s16 id,u32 start) { return SsVabOpenHeadWithMode(address,id,1,start); }

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vs_vh", SsVabOpenHeadWithMode);
