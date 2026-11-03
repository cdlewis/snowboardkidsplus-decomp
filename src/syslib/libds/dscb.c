#include "common.h"
#include "psx/libetc.h"

#ifdef NON_MATCHING
typedef void (*DsCallback)(void);
extern DsCallback dsSyncCallback, dsReadyCallback, dsStartCallback;

/* Each: best observed instruction-sequence similarity 40.00% (2/5 words;
 * GCC 2.8.0). */
DsCallback DsSyncCallback(DsCallback callback) {
    DsCallback previous = dsSyncCallback;
    dsSyncCallback = callback;
    return previous;
}

DsCallback DsReadyCallback(DsCallback callback) {
    DsCallback previous = dsReadyCallback;
    dsReadyCallback = callback;
    return previous;
}

DsCallback DsStartCallback(DsCallback callback) {
    DsCallback previous = dsStartCallback;
    dsStartCallback = callback;
    return previous;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libds/dscb", DsSyncCallback);
INCLUDE_ASM("asm/nonmatchings/syslib/libds/dscb", DsReadyCallback);
INCLUDE_ASM("asm/nonmatchings/syslib/libds/dscb", DsStartCallback);
#endif

DmaCallback DsDataCallback(DmaCallback callback) { return DMACallback(3, callback); }
