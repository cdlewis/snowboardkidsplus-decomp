#include "common.h"
#include "libpad_private.h"

extern void func_800D41A0(void);
#ifdef NON_MATCHING
extern int func_800D40CC(PadCommand *);
#else
extern void func_800D40CC(void);
#endif
extern int func_800D44E0(PadCommand *);
extern void (*D_8013AD7C)(void), (*D_8013AD84)(void);
extern int (*D_8013AD80)(PadCommand *);

void _padInitDirSeq(void) {
    D_8013AD7C = (void (*)(void))func_800D40CC;
    D_8013AD80 = func_800D44E0;
    D_8013AD84 = func_800D41A0;
}

#ifdef NON_MATCHING
extern int _padCmdParaMode(PadCommand *, int);
extern void _padSendAtLoadInfo(PadCommand *);
/* Best observed instruction-sequence similarity: 79.25% (42/53 words). */
int func_800D40CC(PadCommand *p) {
    if (**(u8 **)((u8 *)p + 0x3c) == 0xf3 && !*((u8 *)p + 0xe8)) {
        _padCmdParaMode(p, 0);
    } else {
        switch (p->state) {
        case 0: case 0xff: break;
        case 1: _padCmdParaMode(p, 1); break;
        case 0xfe: _padCmdParaMode(p, 0); break;
        default:
            if (p->send) p->send(p);
            else _padSendAtLoadInfo(p);
            break;
        }
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padseqd", func_800D40CC);
#endif

INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padseqd", func_800D41A0);

extern void (*D_8013AD68)(PadCommand *);

void _dirFailAuto(PadCommand *p) {
    (*(int *)((u8 *)p + 0x4c))++;
    if (p->state) {
        if (p->state == 1) {
            if (p->unknown49[1] < 2) { p->unknown49[1]++; return; }
            p->unknown49[0] = 2;
            p->state = 0xff;
            return;
        } else {
            if (p->unknown49[1] < 4) { p->unknown49[1]++; return; }
            if (p->unknown49[0]) D_8013AD68(p);
        }
    }
    if (**(u8 **)((u8 *)p + 0x3c) != 0xf3) {
        (*(u8 **)((u8 *)p + 0x30))[0] = 0xff;
        (*(u8 **)((u8 *)p + 0x30))[1] = 0;
        *((u8 *)p + 0xe8) = 0;
    }
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 57.14% (8/14 words,
 * including target padding; GCC 2.8.0). */
int func_800D44E0(PadCommand *pad) {
    if (*(u16 *)&pad->unknownE5[1] && pad->state == 0xff)
        return 0;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padseqd", func_800D44E0);
#endif
