#include "common.h"
#include "psx/libetc.h"

s32 D_80132E74 __attribute__((section(".data.spu_started"), aligned(4))) = 0;
extern s32 D_80132E0C;
extern void _spu_FiDMA(void);
extern DmaCallback _SpuDataCallback(DmaCallback);
extern int EnterCriticalSection(void), ExitCriticalSection(void);
extern int OpenEvent(u32, int, int, void *), EnableEvent(int);

#ifdef NON_MATCHING
#include "libspu_private.h"
struct SpuInitValue { s32 a; s16 b, c; s32 d, e; };
extern u16 D_80132E40[];
extern s32 D_80132E18, D_80132E1C, D_80132E20, D_80132EF4;
extern s32 D_80132E10, D_80132E3C, D_80132E38, D_80132E70;
extern struct SpuInitValue D_80132E28;
extern void _spu_init(int), SpuStart(void);
/* Best observed instruction-sequence similarity: 81.97% (50/61 words). */
void _SpuInit(int mode) {
    int i;
    ResetCallback();
    _spu_init(mode);
    if (!mode)
        for (i = 23; i >= 0; i--)
            D_80132E40[i] = 0xc000;
    SpuStart();
    D_80132E18 = 0;
    D_80132E1C = 0;
    D_80132E28.a = 0;
    D_80132E28.b = 0;
    D_80132E28.c = 0;
    D_80132E28.d = 0;
    D_80132E28.e = 0;
    D_80132E20 = D_80132EF4;
    _spu_FsetRXX(0xd1, D_80132EF4, 0);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = 0;
    _spu_trans_mode = 0;
    _spu_transMode = 0;
    D_80132E10 = 0;
    D_80132E3C = 0;
    D_80132E38 = 0;
    D_80132E70 = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_ini", _SpuInit);
#endif

void SpuStart(void) {
    if (D_80132E74)
        return;
    D_80132E74 = 1;
    EnterCriticalSection();
    _SpuDataCallback(_spu_FiDMA);
    D_80132E0C = OpenEvent(0xf0000009, 0x20, 0x2000, 0);
    EnableEvent(D_80132E0C);
    ExitCriticalSection();
}
