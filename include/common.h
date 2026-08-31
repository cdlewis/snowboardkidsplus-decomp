#ifndef COMMON_H
#define COMMON_H

/*
 * Master include for every translation unit under src (splat emits
 * `#include "common.h"` at the top of each generated C file).
 *
 * Grows to pull in real game headers (structs, globals, PsyQ SDK decls)
 * as decompilation progresses.
 */

#include "decomp/types.h"

/* -DSKIP_ASM (objdiff base builds): INCLUDE_ASM becomes a no-op so the
 * object holds only real decompiled C.  include_asm.h is splat-generated
 * and only defines these when not already defined. */
#ifdef SKIP_ASM
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#endif

#include "include_asm.h"

#endif /* COMMON_H */
