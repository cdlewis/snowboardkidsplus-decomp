#include "common.h"
#include "libspu_private.h"

union SpuUnion *_spu_RXX
    __attribute__((section(".data.spu_registers"), aligned(4))) =
    (union SpuUnion *)0x1F801C00;
#define SPU_DMA_REGS __attribute__((section(".data.spu_dma"), aligned(4)))
volatile u32 *spuDmaAddressRegister SPU_DMA_REGS = (volatile u32 *)0x1F8010C0;
volatile u32 *spuDmaBlockRegister SPU_DMA_REGS = (volatile u32 *)0x1F8010C4;
volatile u32 *spuDmaControlRegister SPU_DMA_REGS = (volatile u32 *)0x1F8010C8;
s32 spuDmaDirection __attribute__((section(".data.spu_dma_busy"), aligned(4))) = 0;
static volatile u32 *dma_dpcr
    __attribute__((section(".data.spu_bus"), aligned(4))) = (volatile u32 *)0x1F8010F0;
static volatile u32 *spu_delay
    __attribute__((section(".data.spu_bus"), aligned(4))) = (volatile u32 *)0x1F801014;
u16 _spu_tsa __attribute__((section(".data.spu_transfer_addr"), aligned(2))) = 0;
static u16 spu_tsa_padding __attribute__((section(".data.spu_transfer_addr"), aligned(2))) = 0;
s32 _spu_mem_mode_plus
    __attribute__((section(".data.spu_shift"), aligned(4))) = 3;

#ifdef NON_MATCHING
extern int _spu_addrMode, _spu_IRQCallback;
extern u32 _spu_mem_mode, _spu_mem_mode_unit, _spu_mem_mode_unitM;
extern u16 _spu_RQ[];
extern u8 spuTransferScratch[];
extern void (*volatile _spu_transferCallback)(void);
extern const char spuTimeoutFormat[], spuResetWaitName[];
extern int printf(const char *, ...);
extern void _spu_Fw1ts(void), func_800A8E24(void *, int);
/* Best observed instruction-sequence similarity: 82.53% (137/166 words;
 * GCC 2.7.2). */
int _spu_init(int hot) {
    unsigned int i;
    *dma_dpcr |= 0xb0000;
    _spu_transMode = _spu_addrMode = 0;
    _spu_tsa = 0;
    _spu_RXX->rxx.main_vol.left = 0;
    _spu_RXX->rxx.main_vol.right = 0;
    _spu_RXX->rxx.spucnt = 0;
    _spu_Fw1ts();
    _spu_RXX->rxx.main_vol.left = 0;
    _spu_RXX->rxx.main_vol.right = 0;
    i = 0;
    while (_spu_RXX->rxx.spustat & 0x7ff) {
        if (++i > 0xf00) { printf(spuTimeoutFormat, spuResetWaitName); break; }
    }
    _spu_mem_mode = 2; _spu_mem_mode_plus = 3; _spu_mem_mode_unit = 8; _spu_mem_mode_unitM = 7;
    _spu_RXX->rxx.data_trans = 4;
    _spu_RXX->rxx.rev_vol.left = 0; _spu_RXX->rxx.rev_vol.right = 0;
    _spu_RXX->rxx.key_off[0] = 0xffff; _spu_RXX->rxx.key_off[1] = 0xffff;
    _spu_RXX->rxx.rev_mode[0] = 0; _spu_RXX->rxx.rev_mode[1] = 0;
    for (i = 0; i < 10; i++) _spu_RQ[i] = 0;
    if (!hot) {
        _spu_tsa = 0x200;
        _spu_RXX->rxx.chan_fm[0] = _spu_RXX->rxx.chan_fm[1] = 0;
        _spu_RXX->rxx.noise_mode[0] = _spu_RXX->rxx.noise_mode[1] = 0;
        _spu_RXX->rxx.cd_vol.left = _spu_RXX->rxx.cd_vol.right = 0;
        _spu_RXX->rxx.ex_vol.left = _spu_RXX->rxx.ex_vol.right = 0;
        func_800A8E24(spuTransferScratch, 16);
        for (i = 0; i < 24; i++) {
            _spu_RXX->rxx.voice[i].volume.left = 0;
            _spu_RXX->rxx.voice[i].volume.right = 0;
            _spu_RXX->rxx.voice[i].pitch = 0x3fff;
            _spu_RXX->rxx.voice[i].addr = 0x200;
            _spu_RXX->rxx.voice[i].adsr[0] = 0;
            _spu_RXX->rxx.voice[i].adsr[1] = 0;
        }
        _spu_RXX->rxx.key_on[0] = 0xffff; _spu_RXX->rxx.key_on[1] = 0xff;
        _spu_Fw1ts(); _spu_Fw1ts(); _spu_Fw1ts(); _spu_Fw1ts();
        _spu_RXX->rxx.key_off[0] = 0xffff; _spu_RXX->rxx.key_off[1] = 0xff;
        _spu_Fw1ts(); _spu_Fw1ts(); _spu_Fw1ts(); _spu_Fw1ts();
    }
    _spu_inTransfer = 1;
    _spu_RXX->rxx.spucnt = 0xc000;
    _spu_transferCallback = 0; _spu_IRQCallback = 0;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_init);
#endif

#ifdef NON_MATCHING
extern void _spu_Fw1ts(void);
extern const char spuTimeoutFormat[], spuWriteReadyWaitName[], spuDmaWriteWaitName[];
extern int printf(const char *, ...);
/* Best observed instruction-sequence similarity: 74.56% (85/114 words;
 * GCC 2.7.2). */
void func_800A8E24(void *data, int size) {
    u16 *source = data;
    int n, i;
    u16 status = _spu_RXX->rxx.spustat & 0x7ff;
    _spu_RXX->rxx.trans_addr = _spu_tsa;
    _spu_Fw1ts();
    while (size) {
        n = (unsigned)size < 65 ? size : 64;
        for (i = 0; i < n; i += 2) _spu_RXX->rxx.trans_fifo = *source++;
        _spu_RXX->rxx.spucnt = (_spu_RXX->rxx.spucnt & ~0x30) | 0x10;
        _spu_Fw1ts();
        i = 0;
        while (_spu_RXX->rxx.spustat & 0x400) {
            if (++i > 0xf00) { printf(spuTimeoutFormat, spuWriteReadyWaitName); break; }
        }
        size -= n;
        _spu_Fw1ts();
        _spu_Fw1ts();
    }
    _spu_RXX->rxx.spucnt &= ~0x30;
    i = 0;
    while ((_spu_RXX->rxx.spustat & 0x7ff) != status) {
        if (++i > 0xf00) { printf(spuTimeoutFormat, spuDmaWriteWaitName); break; }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", func_800A8E24);
#endif

#ifdef NON_MATCHING
extern void (*volatile _spu_transferCallback)(void);
extern void _spu_Fw1ts(void);
extern int DeliverEvent(u32, u32);
/* Best observed instruction-sequence similarity: 55.32% (26/47 words). */
void _spu_FiDMA(void) {
    unsigned int i = 0;
    volatile SPU_RXX *regs = &_spu_RXX->rxx;
    if (spuDmaDirection == 0)
        _spu_Fw1ts();
    regs->spucnt &= ~0x30;
    while ((regs->spucnt & 0x30) && ++i <= 0xf00) {}
    if (_spu_transferCallback)
        _spu_transferCallback();
    else
        DeliverEvent(0xf0000009, 0x20);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_FiDMA);
#endif

extern void _spu_Fw1ts(void);

void _spu_Fr_(u32 data, u16 addr, u16 count) {
    _spu_RXX->rxx.trans_addr = addr;
    _spu_Fw1ts();
    _spu_RXX->rxx.spucnt |= 0x30;
    _spu_Fw1ts();
    _spu_FsetDelayR();
    *spuDmaAddressRegister = data;
    *spuDmaBlockRegister = (count << 16) | 0x10;
    spuDmaDirection = 1;
    *spuDmaControlRegister = 0x01000200;
}

#ifdef NON_MATCHING
extern u32 spuPendingDmaAddress, spuPendingDmaBlocks;
/* Best observed instruction-sequence similarity: 68.12% (109/160 words;
 * GCC 2.7.2). */
int _spu_t(int op, ...) {
    u32 *args = __builtin_next_arg(op);
    unsigned int i;
    u16 mode;
    u32 ctrl;
    switch (op) {
    case 2:
        _spu_tsa = *args >> _spu_mem_mode_plus;
        _spu_RXX->rxx.trans_addr = _spu_tsa;
        return 0;
    case 1:
        spuDmaDirection = 0;
        i = 0;
        while (_spu_RXX->rxx.trans_addr != _spu_tsa) if (++i > 0xf00) return -2;
        _spu_RXX->rxx.spucnt = (_spu_RXX->rxx.spucnt & ~0x30) | 0x20;
        return 0;
    case 0:
        spuDmaDirection = 1;
        i = 0;
        while (_spu_RXX->rxx.trans_addr != _spu_tsa) if (++i > 0xf00) return -2;
        _spu_RXX->rxx.spucnt |= 0x30;
        return 0;
    case 3:
        mode = spuDmaDirection == 1 ? 0x30 : 0x20;
        i = 0;
        while ((_spu_RXX->rxx.spucnt & 0x30) != mode) if (++i > 0xf00) return -2;
        if (spuDmaDirection == 1) _spu_FsetDelayR(); else _spu_FsetDelayW();
        spuPendingDmaAddress = args[0];
        spuPendingDmaBlocks = (args[1] >> 6) + ((args[1] & 63) != 0);
        *spuDmaAddressRegister = spuPendingDmaAddress;
        *spuDmaBlockRegister = (spuPendingDmaBlocks << 16) | 0x10;
        ctrl = spuDmaDirection == 1 ? 0x01000200 : 0x01000201;
        *spuDmaControlRegister = ctrl;
        return 0;
    default: return 0;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_t);
#endif

extern int _spu_t(int, ...);
extern void func_800A8E24(void *, int);

int _spu_Fw(void *data, int size) {
    if (_spu_transMode == 0) {
        _spu_t(2, _spu_tsa << _spu_mem_mode_plus);
        _spu_t(1);
        _spu_t(3, data, size);
    } else {
        func_800A8E24(data, size);
    }
    return size;
}

int _spu_Fr(void *data, int size) {
    _spu_t(2, _spu_tsa << _spu_mem_mode_plus);
    _spu_t(0);
    _spu_t(3, data, size);
    return size;
}

void _spu_FsetRXX(unsigned offset, unsigned value, unsigned mode) {
    if (mode == 0) {
        _spu_RXX->raw[offset] = value;
    } else {
        _spu_RXX->raw[offset] = value >> _spu_mem_mode_plus;
    }
}

#ifdef NON_MATCHING
extern u32 _spu_mem_mode, _spu_mem_mode_unit, _spu_mem_mode_unitM;

/* Best observed instruction-sequence similarity: 85.37% (35/41 words). */
u32 _spu_FsetRXXa(s32 offset, u32 unit) {
    u16 value;

    if (_spu_mem_mode && (unit % _spu_mem_mode_unit)) {
        unit += _spu_mem_mode_unit;
        unit &= ~_spu_mem_mode_unitM;
    }
    value = unit >> _spu_mem_mode_plus;
    switch (offset) {
    case -1:
        return value & 0xFFFF;
    case -2:
        return unit;
    default:
        _spu_RXX->raw[offset] = value;
        return unit;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/spu", _spu_FsetRXXa);
#endif

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

void _spu_Fw1ts(void) {
    volatile int i, delay;

    delay = 13;
    for (i = 0; i < 60; i++) delay *= 13;
}
