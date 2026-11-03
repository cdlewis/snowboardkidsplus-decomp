#include "common.h"
#include "psx/libetc.h"

s32 _spu_isCalled __attribute__((section(".data.spu_started"), aligned(4))) = 0;
extern s32 _spu_EVdma;
extern void _spu_FiDMA(void);
extern DmaCallback _SpuDataCallback(DmaCallback);
extern int EnterCriticalSection(void), ExitCriticalSection(void);
extern int OpenEvent(u32, int, int, void *), EnableEvent(int);

#ifdef NON_MATCHING
#include "libspu_private.h"
struct SpuInitValue { s32 a; s16 b, c; s32 d, e; };
extern u16 _spu_voice_centerNote[];
extern s32 _spu_rev_flag, _spu_rev_reserve_wa, _spu_rev_offsetaddr, _spu_rev_startaddr;
extern s32 _spu_keystat, _spu_RQmask, _spu_RQvoice, _spu_env;
extern struct SpuInitValue spuInitialState;
extern void _spu_init(int), SpuStart(void);
/* Best observed instruction-sequence similarity: 81.97% (50/61 words). */
void _SpuInit(int mode) {
    int i;
    ResetCallback();
    _spu_init(mode);
    if (!mode)
        for (i = 23; i >= 0; i--)
            _spu_voice_centerNote[i] = 0xc000;
    SpuStart();
    _spu_rev_flag = 0;
    _spu_rev_reserve_wa = 0;
    spuInitialState.a = 0;
    spuInitialState.b = 0;
    spuInitialState.c = 0;
    spuInitialState.d = 0;
    spuInitialState.e = 0;
    _spu_rev_offsetaddr = _spu_rev_startaddr;
    _spu_FsetRXX(0xd1, _spu_rev_startaddr, 0);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = 0;
    _spu_trans_mode = 0;
    _spu_transMode = 0;
    _spu_keystat = 0;
    _spu_RQmask = 0;
    _spu_RQvoice = 0;
    _spu_env = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_ini", _SpuInit);
#endif

void SpuStart(void) {
    if (_spu_isCalled)
        return;
    _spu_isCalled = 1;
    EnterCriticalSection();
    _SpuDataCallback(_spu_FiDMA);
    _spu_EVdma = OpenEvent(0xf0000009, 0x20, 0x2000, 0);
    EnableEvent(_spu_EVdma);
    ExitCriticalSection();
}
