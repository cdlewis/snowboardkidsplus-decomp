#include "common.h"

#define PAD_DATA __attribute__((section(".data.pad_state"), aligned(4)))

int is_pad_init PAD_DATA = 0;
volatile int pad_data_port PAD_DATA = 0x1F801040;
volatile int pad_interrupt_status PAD_DATA = 0x1F801070;

void SetInitPadFlag(int value) { is_pad_init = value; }

int ReadInitPadFlag(void) { return is_pad_init; }

extern void StartPAD2(void);
extern void ChangeClearPAD(int);
extern void EnablePAD(void);
extern void DisablePAD(void);
extern void StopPAD2(void);
extern int func_800D6E64(void);
extern void _remove_ChgclrPAD(void);
extern void EnterCriticalSection(void);
extern void _patch_pad(void);
extern void ExitCriticalSection(void);
extern int func_800D6DEC(void);
extern void PAD_init2(char *, int, char *, int);
extern void InitPAD2(char *, int, char *, int);

int PAD_init(char *a, int alen, char *b, int blen) {
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    func_800D6DEC();
    PAD_init2(a, alen, b, blen);
    is_pad_init = 1;
    return 1;
}

int InitPAD(char *a, int alen, char *b, int blen) {
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    func_800D6DEC();
    InitPAD2(a, alen, b, blen);
    is_pad_init = 1;
    return 1;
}

int StartPAD(void) {
    StartPAD2();
    ChangeClearPAD(0);
    EnablePAD();
    return 1;
}

int StopPAD(void) {
    int result;

    DisablePAD();
    StopPAD2();
    result = func_800D6E64();
    is_pad_init = 0;
    return result;
}

typedef struct {
    void *next;
    int (*handler)(void);
    int (*test)(void);
    void *unused;
} PadIntrEntry;
extern PadIntrEntry padInterruptEntry;
extern void SysDeqIntRP(int, void *);

#ifdef NON_MATCHING
extern int func_800D6E9C(void), func_800D6F04(void);
extern void SysEnqIntRP(int, void *);
/* Best observed instruction-sequence similarity: 93.33% (28/30 words;
 * GCC 2.7.2). */
int func_800D6DEC(void) {
    EnterCriticalSection();
    padInterruptEntry.handler = func_800D6E9C;
    padInterruptEntry.test = func_800D6F04;
    padInterruptEntry.next = 0;
    padInterruptEntry.unused = 0;
    SysDeqIntRP(1, &padInterruptEntry);
    SysEnqIntRP(1, &padInterruptEntry);
    ExitCriticalSection();
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/pad", func_800D6DEC);
#endif

int func_800D6E64(void) {
    EnterCriticalSection();
    SysDeqIntRP(1, &padInterruptEntry);
    ExitCriticalSection();
    return 1;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 92.31% (24/26 words;
 * GCC 2.7.2). */
int func_800D6E9C(void) {
    volatile int delay;
    *(volatile u16 *)(pad_data_port + 10) = 0;
    delay = 10;
    while (delay-- != 0) { }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/pad", func_800D6E9C);
#endif

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 58.82% (10/17 words;
 * GCC 2.8.0). */
int func_800D6F04(void) {
    volatile u32 *regs = (volatile u32 *)pad_interrupt_status;
    if (regs[1] & 1) {
        if (regs[0] & 1) return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/pad", func_800D6F04);
#endif
