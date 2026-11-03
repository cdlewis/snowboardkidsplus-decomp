#include "common.h"
#include "libetc_private.h"


s32 ResetCallback(void) { return gInterruptDispatch->start(); }

DmaCallback InterruptCallback(s32 channel, DmaCallback cb) { return gInterruptDispatch->interrupt(channel, cb); }

DmaCallback DMACallback(s32 channel, DmaCallback cb) { return gInterruptDispatch->dma(channel, cb); }

DmaCallback VSyncCallback(DmaCallback cb) { return gInterruptDispatch->vsync(4, cb); }

DmaCallback VSyncCallbacks(s32 channel, DmaCallback cb) { return gInterruptDispatch->vsync(channel, cb); }

s32 StopCallback(void) { return gInterruptDispatch->stop(); }

s32 RestartCallback(void) { return gInterruptDispatch->restart(); }

s32 CheckCallback(void) { return gInterruptCallbackContext; }

s32 GetIntrMask(void) { return *gInterruptMaskRegister; }

s32 SetIntrMask(s32 mask) { s32 prev = *gInterruptMaskRegister; *gInterruptMaskRegister = mask; return prev; }

#ifdef NON_MATCHING
struct IntrState;
extern struct IntrState interruptState;
extern volatile u16 *interruptStatusRegister;
extern volatile u32 *interruptDmaPriorityRegister;
extern void intr_memclr(s32 *, int), func_800B5A34(void);
extern int setjmp(void *), HookEntryInt(u32 *), ExitCriticalSection(void), _96_remove(void);
extern void *startIntrVSync(void), *startIntrDMA(void);
/* Best observed instruction-sequence similarity: 50.00% (30/60 words;
 * GCC 2.8.0, -mno-split-addresses). */
void *func_800B595C(void) {
    u8 *s = (u8 *)&interruptState;
    if (*(u16 *)s) return 0;
    *gInterruptMaskRegister = 0;
    *interruptStatusRegister = *gInterruptMaskRegister;
    *interruptDmaPriorityRegister = 0x33333333;
    intr_memclr((s32 *)s, 0x41a);
    if (setjmp(s + 0x38)) func_800B5A34();
    *(u32 *)(s + 0x3c) = (u32)(s + 0x1018);
    HookEntryInt((u32 *)(s + 0x38));
    *(u16 *)s = 1;
    gInterruptDispatch->vsync = startIntrVSync();
    gInterruptDispatch->dma = startIntrDMA();
    _96_remove();
    ExitCriticalSection();
    return s;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B595C);
#endif

#ifdef NON_MATCHING
typedef struct {
    u16 active, context;
    DmaCallback callbacks[11];
    u16 enabled;
} IntrCallbackState;
extern int interruptRetryCount;
extern const char unexpectedInterruptFormat[], interruptTimeoutFormat[];
extern int printf(const char *, ...);
extern void ReturnFromException(void);
/* Best observed instruction-sequence similarity: 69.83% (81/116 words;
 * GCC 2.8.0, -mno-split-addresses). */
void func_800B5A34(void) {
    IntrCallbackState *s = (IntrCallbackState *)&interruptState;
    u16 pending;
    int i;
    if (!s->active) {
        printf(unexpectedInterruptFormat, *interruptStatusRegister);
        ReturnFromException();
    }
    s->context = 1;
    pending = s->enabled & *interruptStatusRegister & *gInterruptMaskRegister;
    while (pending) {
        for (i = 0; pending && i < 11; i++, pending >>= 1) {
            if (pending & 1) {
                *interruptStatusRegister = ~(1 << i);
                if (s->callbacks[i]) s->callbacks[i]();
            }
        }
        pending = s->enabled & *interruptStatusRegister & *gInterruptMaskRegister;
    }
    if (*interruptStatusRegister & *gInterruptMaskRegister) {
        if (interruptRetryCount++ > 0x800) {
            printf(interruptTimeoutFormat, *interruptStatusRegister, *gInterruptMaskRegister);
            interruptRetryCount = 0;
            *interruptStatusRegister = 0;
        }
    } else interruptRetryCount = 0;
    s->context = 0;
    ReturnFromException();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5A34);
#endif

#ifdef NON_MATCHING
extern void ChangeClearPAD(int), ChangeClearRCnt(int, int);
/* Best observed instruction-sequence similarity: 72.09% (62/86 words;
 * GCC 2.7.2). */
DmaCallback func_800B5C04(int channel, DmaCallback callback) {
    IntrCallbackState *s = (IntrCallbackState *)&interruptState;
    DmaCallback prev = s->callbacks[channel];
    int mask;
    if (callback == prev) return prev;
    if (s->active) {
        mask = *gInterruptMaskRegister;
        *gInterruptMaskRegister = 0;
        if (callback) {
            s->callbacks[channel] = callback;
            mask |= 1 << channel;
            s->enabled |= 1 << channel;
        } else {
            s->callbacks[channel] = 0;
            mask &= ~(1 << channel);
            s->enabled &= ~(1 << channel);
        }
        if (channel == 0) { ChangeClearPAD(!callback); ChangeClearRCnt(3, !callback); }
        if (channel == 4) ChangeClearRCnt(0, !callback);
        if (channel == 5) ChangeClearRCnt(1, !callback);
        if (channel == 6) ChangeClearRCnt(2, !callback);
        *gInterruptMaskRegister = mask;
    }
    return prev;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5C04);
#endif

struct IntrState {
    u16 active;
    u8 unknown02[0x30];
    u16 saved_mask;
    u32 saved_dpcr;
    u32 jump[16];
};
extern struct IntrState interruptState;
extern volatile u32 *interruptDmaPriorityRegister;

#ifdef NON_MATCHING
extern volatile u16 *interruptStatusRegister;
extern int EnterCriticalSection(void), ResetEntryInt(void);
/* Best observed instruction-sequence similarity: 65.12% (28/43 words). */
void *func_800B5D4C(void) {
    struct IntrState *s = &interruptState;
    if (!s->active)
        return 0;
    EnterCriticalSection();
    s->saved_mask = *gInterruptMaskRegister;
    s->saved_dpcr = *interruptDmaPriorityRegister;
    *gInterruptMaskRegister = 0;
    *interruptStatusRegister = *gInterruptMaskRegister;
    *interruptDmaPriorityRegister &= 0x77777777;
    ResetEntryInt();
    s->active = 0;
    return s;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libetc/intr", func_800B5D4C);
#endif

extern int HookEntryInt(u32 *), ExitCriticalSection(void);

void *func_800B5DEC(void) {
    struct IntrState *s = &interruptState;
    if (s->active)
        return 0;
    HookEntryInt(s->jump);
    s->active = 1;
    *gInterruptMaskRegister = s->saved_mask;
    *interruptDmaPriorityRegister = s->saved_dpcr;
    ExitCriticalSection();
    return s;
}

void intr_memclr(s32 *p, int n) {
    int i = n - 1;
    if (n != 0) {
        do {
            *p = 0;
            i--;
            p++;
        } while (i != -1);
    }
}
