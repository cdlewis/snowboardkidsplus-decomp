/* Adapted from PsyZ decomp; see THIRD_PARTY.md and licenses/psyz-decomp.txt. */
#include "common.h"
#include "libspu_private.h"

union SpuUnion *_spu_RXX
    __attribute__((section(".data.spu_registers"), aligned(4))) =
    (union SpuUnion *)0x1F801C00;
static volatile u32 *dma_dpcr
    __attribute__((section(".data.spu_bus"), aligned(4))) = (volatile u32 *)0x1F8010F0;
static volatile u32 *spu_delay
    __attribute__((section(".data.spu_bus"), aligned(4))) = (volatile u32 *)0x1F801014;
s32 _spu_mem_mode_plus
    __attribute__((section(".data.spu_shift"), aligned(4))) = 3;

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_init);

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", func_800A8E24);

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_FiDMA);

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_Fr_);

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_t);

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_Fw);

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_Fr);

void _spu_FsetRXX(unsigned offset, unsigned value, unsigned mode) {
    if (mode == 0) {
        _spu_RXX->raw[offset] = value;
    } else {
        _spu_RXX->raw[offset] = value >> _spu_mem_mode_plus;
    }
}

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_FsetRXXa);

u32 _spu_FgetRXXa(s32 offset, s32 mode) {
    u16 value = _spu_RXX->raw[offset];
    if (mode == -1) {
        return value;
    } else {
        return value << _spu_mem_mode_plus;
    }
}

void _spu_FsetPCR(s32 mode) {
    *dma_dpcr &= 0xFFF8FFFF;
    if (mode) {
        *dma_dpcr |= 0x30000;
    } else {
        *dma_dpcr |= 0x50000;
    }
}

void _spu_FsetDelayW(void) {
    *spu_delay = (*spu_delay & 0xF0FFFFFF) | 0x20000000;
}

void _spu_FsetDelayR(void) {
    *spu_delay = (*spu_delay & 0xF0FFFFFF) | 0x22000000;
}

INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_Fw1ts);
