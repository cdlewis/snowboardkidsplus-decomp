#include "common.h"
#include "libpad_private.h"

#ifdef NON_MATCHING
typedef struct PadPort PadPort;
struct PadPort {
    u8 unk0[0xc]; void *unkc; PadPort *self; u8 unk14[0x1c];
    u8 *buffer; u8 unk34[8]; u8 *send; u8 *receive;
    u8 unk44[0x19]; u8 align[6]; u8 rest[0x8d];
};
extern u8 D_80141310[], D_80141280[], D_801412C8[];
extern int D_8013AD9C, D_8013ADB0;
extern PadCommand *D_8013AD98;
extern void (*D_8013AD64)(int), (*D_8013AD68)(PadCommand *);
extern int (*D_8013AD6C)(PadCommand *), (*D_8013AD74)(PadCommand *);
extern void (*D_8013AD70)(PadCommand *);
extern u8 *(*D_8013AD78)(int);
extern unsigned (*D_8013AD88)(PadCommand *);
extern void _padInitDirSeq(void), func_800D2000(void), bzero(void *, int);
extern void func_800D3BF0(int), func_800D3B88(PadCommand *);
extern int func_800D3CF4(PadCommand *), func_800D402C(PadCommand *);
extern void func_800D3DB0(PadCommand *);
extern u8 *func_800D4064(int);
extern unsigned _pad_shift(PadCommand *);
/* Best observed instruction-sequence similarity: 78.89% (71/90 words;
 * GCC 2.7.2). */
int PadInitDirect(u8 *a, u8 *b) {
    PadPort *p = (PadPort *)D_80141310;
    int i, j;
    D_8013AD9C = 0; D_8013ADB0 = 0;
    _padInitDirSeq();
    D_8013AD64 = func_800D3BF0; D_8013AD68 = func_800D3B88;
    D_8013AD6C = func_800D3CF4; D_8013AD70 = func_800D3DB0;
    D_8013AD74 = func_800D402C; D_8013AD78 = func_800D4064;
    D_8013AD98 = (PadCommand *)p; D_8013AD88 = _pad_shift;
    bzero(p, 0x1e0);
    p[0].buffer = a; p[1].buffer = b;
    for (i = 0; i < 2; i++, p++) {
        p->unkc = 0; p->self = p;
        p->buffer[0] = 0xff; p->buffer[1] = 0;
        p->send = D_80141280 + i * 0x23;
        p->receive = D_801412C8 + i * 0x23;
        for (j = 0; j < 6; j++) p->align[j] = 0xff;
    }
    func_800D2000();
    D_8013AD9C = 1;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padportd", PadInitDirect);
#endif

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 96.15% (25/26 words;
 * GCC 2.7.2). */
void func_800D3B88(PadCommand *p) {
    u8 *q;
    int i;
    if (p->unknown49[0]) {
        p->unknown49[0] = 0;
        p->state = 0;
        *(u16 *)((u8 *)p + 0xe6) = 0;
        p->send = 0;
        p->receive = 0;
        p->mode_count = 0;
        p->mode_index = 0;
        *(volatile u16 *)((u8 *)p + 0xe6) = 0;
        p->actuator_count = 0;
        p->unknownEA[0] = 0;
        *(u32 *)((u8 *)p + 0) = 0;
        *(u32 *)((u8 *)p + 4) = 0;
        *(u32 *)((u8 *)p + 8) = 0;
        q = p->alignment_data;
        for (i = 5; i >= 0; i--)
            *q++ = 0xff;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padportd", func_800D3B88);
#endif

#ifdef NON_MATCHING
extern u8 D_80141310[];
extern int D_8013ADA4, D_8013ADA8, D_8013ADB8, D_8013ADBC[];
extern u8 *D_8013ADFC;
extern void _dirFailAuto(PadCommand *);
extern unsigned _pad_shift(PadCommand *);
extern int func_800D2324(PadCommand *);
/* Best observed instruction-sequence similarity: 65.57% (40/61 words;
 * GCC 2.7.2). */
void func_800D3BF0(int result) {
    int done;
    do {
        PadCommand *p = &((PadCommand *)D_80141310)[D_8013ADA4];
        if (result != -9) {
            if (result == 0) D_8013ADBC[D_8013ADA4] = 0;
            else { _dirFailAuto(p); _pad_shift(p); }
        }
        D_8013ADA8 = 0;
        *(volatile u16 *)(D_8013ADFC + 10) = 0;
        D_8013ADA4++;
        done = 1;
        if (D_8013ADA4 <= D_8013ADB8)
            done = func_800D2324(&((PadCommand *)D_80141310)[D_8013ADA4]);
        result = 0xffff;
    } while (!done);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padportd", func_800D3BF0);
#endif

unsigned _pad_shift(PadCommand *p) {
    unsigned command = p->command;
    p->command = 0;
    p->last_command = command;
    return command;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 72.55% (37/51 words;
 * GCC 2.8.0). */
int func_800D3CF4(PadCommand *p) {
    int index = *((u8 *)p + 0x45) - 3;
    switch (p->command) {
    case 0:
        if (index < 6 && !*((u8 *)p + 0x57 + index)) return 0;
        if (index < p->actuator_length) return p->actuators[index];
        return 0;
    case 0x4d:
        if (index < p->command_length) return p->command_data[index];
        return 0xff;
    default:
        if (index < p->command_length) return p->command_data[index];
        return 0;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padportd", func_800D3CF4);
#endif

#ifdef NON_MATCHING
extern int D_8013ADAC;
extern void bzero(void *, int);
/* Best observed instruction-sequence similarity: 45.91% (73/159 words;
 * GCC 2.7.2). */
void func_800D3DB0(PadCommand *p) {
    u8 *flags = p->unknown54 + 3;
    u16 mode;
    int i, j, n, enabled, cost;
    u8 *desc;
    bzero(flags, 6);
    mode = *(u16 *)&p->unknownE5[1];
    if (mode && p->actuators) {
        n = p->actuator_length < 6 ? p->actuator_length : 6;
        for (i = 0; i < p->actuator_count; i++) {
            desc = *(u8 **)((u8 *)p + 4) + i * 5;
            enabled = 0;
            for (j = 0; j < n; j++) {
                if (p->alignment_data[j] == i && (p->actuators[j] & (desc[2] ? 0xff : 1))) { enabled = 1; break; }
            }
            if (enabled) {
                cost = D_8013ADAC + desc[3];
                if (cost < 61) D_8013ADAC = cost;
                else enabled = 0;
            }
            if (enabled) for (j = 0; j < n; j++) if (p->alignment_data[j] == i) flags[j] = 1;
        }
    } else if ((p->unknownE5[3] == 4 || p->unknownE5[3] == 5 || p->unknownE5[3] == 7) && !mode && p->actuator_length >= 2) {
        if ((p->actuators[0] & 0xc0) == 0x40 && (p->actuators[1] & 1) && D_8013ADAC + 10 < 61) {
            flags[1] = flags[0] = 1; D_8013ADAC += 10;
        }
    } else if (p->unknownE5[3] == 3) flags[0] = 1;
    else if (!mode) for (i = 5; i >= 0; i--) flags[i] = 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padportd", func_800D3DB0);
#endif

extern u8 D_80141310[];

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 78.57% (11/14 words). */
int func_800D402C(PadCommand *port) {
    int i, result = 0x10;
    for (i = 0; i < 2; i++, result += 0x10)
        if (port == (PadCommand *)D_80141310 + i)
            return result;
    return 0xff;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padportd", func_800D402C);
#endif

u8 *func_800D4064(int port) {
    u8 *p = D_80141310;
    if (port & 0xf0)
        p += 0xf0;
    return p;
}
