#include "common.h"
#include "libpad_private.h"

extern int _padCmdGetStatus(PadCommand *);
extern int _padCmdSetMap(PadCommand *, int);
extern int _padCmdGetDescR0(PadCommand *, int);
extern int _padCmdGetDescR1(PadCommand *, int);
extern int _padCmd4B(PadCommand *);




void _padSetAct(PadCommand *p, u8 *data, int len) {
    p->actuators = data;
    p->actuator_length = len;
}

void _padSetCmd(PadCommand *p, int cmd, u8 *data, int len) {
    p->command = cmd;
    p->command_data = data;
    p->command_length = len;
}

void _padSendAtLoadInfo(PadCommand *p) {
    switch (p->state) {
    case 2: _padCmdGetStatus(p); break;
    case 3: _padCmdSetMap(p, p->mode_index); break;
    case 4: _padCmdGetDescR1(p, p->request_index); break;
    }
}

#ifdef NON_MATCHING
extern int _padGetActSize(PadCommand *);
extern int _padLoadActInfo(PadCommand *, void *);
/* Best observed instruction-sequence similarity: 81.93% (68/83 words;
 * GCC 2.8.0, -O2). */
int _padRecvAtLoadInfo(PadCommand *p) {
    switch (p->state) {
    case 2:
        p->mode_count = (*(u8 * volatile *)((u8 *)p + 0x3c))[3];
        p->mode_index = (*(u8 * volatile *)((u8 *)p + 0x3c))[4];
        *(u16 *)((u8 *)p + 0xe6) = 0;
        p->actuator_count = (*(u8 * volatile *)((u8 *)p + 0x3c))[5];
        p->unknownEA[0] = (*(u8 * volatile *)((u8 *)p + 0x3c))[6];
        p->actuator_size = 0;
        break;
    case 3:
        *(u16 *)((u8 *)p + 0xe6) = ((*(u8 **)((u8 *)p + 0x3c))[4] << 8)
                                   + (*(u8 **)((u8 *)p + 0x3c))[5];
        p->request_index = 0;
        break;
    case 4: {
        int size = p->actuator_size;
        unsigned int index = p->request_index + 1;
        p->request_index = index;
        p->actuator_size = size + (((*(u8 **)((u8 *)p + 0x3c))[4] + 3) & 0x1fc) + 8;
        if ((u8)index < p->unknownEA[0]) return 0;
        if (_padGetActSize(p) > 128) {
            p->state = 0xfe;
            p->unknown49[0] = 2;
        } else {
            p->state = 0xff;
            _padLoadActInfo(p, (u8 *)p + 0x63);
            p->state = 2;
        }
        return 0;
    }
    }
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padcmd", _padRecvAtLoadInfo);
#endif

int _padGetActSize(PadCommand *p) {
    int nmode = p->mode_count;
    int nact = p->actuator_count;
    int accum = p->actuator_size;
    int a = ((nmode + 1) >> 1) << 2;
    int b = ((nact * 5 + 3) & 0xffc) + 4;
    return a + b + accum;
}

#ifdef NON_MATCHING
extern int (*_padFuncChkEng)(PadCommand *);
extern void _padLoadActInfo_snd(PadCommand *);
extern int func_800D2E88(PadCommand *);
/* Best observed instruction-sequence similarity: 61.11% (33/54 words;
 * GCC 2.8.0, -O2). */
int _padLoadActInfo(PadCommand *p, void *memory) {
    u32 base;
    if (memory) {
        if (*(u32 *)((u8 *)p + 4)) return 0;
        if (!_padFuncChkEng(p)) {
            p->unknown49[0] = 4;
            p->state = 1;
            p->send = _padLoadActInfo_snd;
            p->receive = func_800D2E88;
            base = (((int)memory + 3) >> 2) << 2;
            *(u32 *)((u8 *)p + 0) = base;
            p->request_index = 0;
            base += ((p->mode_count + 1) >> 1) << 2;
            *(u32 *)((u8 *)p + 4) = base;
            base += (p->actuator_count * 5 + 3) & 0xffc;
            *(u32 *)((u8 *)p + 8) = base;
            return 1;
        }
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padcmd", _padLoadActInfo);
#endif

void _padLoadActInfo_snd(PadCommand *p) {
    switch (p->state) {
    case 2:
        _padCmdSetMap(p, p->request_index);
        break;
    case 3:
        _padCmdGetDescR0(p, p->request_index);
        break;
    case 4:
        if (p->request_step == 0)
            _padCmdGetDescR1(p, p->request_index);
        else
            _padCmd4B(p);
        break;
    }
}

#ifdef NON_MATCHING
typedef struct { u8 size; u8 pad[3]; u8 *data; } PadBlock;
typedef struct {
    u16 *modes; u8 *descriptors; PadBlock *blocks;
    u8 pad0c[0x30]; u8 *reply; u8 pad40[6];
    u8 state, index, remaining, status; u8 pad4a[0x99];
    u8 mode_count; u8 padE4[5]; u8 actuator_count, block_count;
} PadLoad;
extern u8 *padCommandWriteCursor;
/* Best observed instruction-sequence similarity: 78.34% (123/157 words;
 * GCC 2.8.0, -O2). */
int func_800D2E88(PadCommand *command) {
    PadLoad *p = (PadLoad *)command;
    PadBlock *block;
    u8 *src, *desc;
    int n;
    switch (p->state) {
    case 2:
        p->modes[p->index] = (p->reply[4] << 8) + p->reply[5];
        if (++p->index < p->mode_count) return 0;
        p->index = 0;
        return 1;
    case 3:
        desc = p->descriptors + p->index * 5;
        desc[0] = p->reply[4]; desc[1] = p->reply[5] & 0x7f;
        desc[2] = p->reply[6]; desc[3] = p->reply[7]; desc[4] = p->reply[5] >> 7;
        if (++p->index < p->actuator_count) return 0;
        p->index = 0; p->remaining = 0;
        return 1;
    case 4:
        block = &p->blocks[p->index];
        if (!p->remaining) {
            p->remaining = p->reply[4];
            block->size = p->remaining;
            if (!p->index) block->data = (u8 *)(p->blocks + p->block_count);
            else block->data = block[-1].data + ((block[-1].size + 3) & 0x1fc);
            padCommandWriteCursor = block->data;
            src = p->reply + 5; n = 4;
        } else { src = p->reply + 2; n = 6; }
        while (n--) {
            if (!p->remaining) break;
            *padCommandWriteCursor++ = *src++;
            p->remaining--;
        }
        if (p->remaining) return 0;
        if (++p->index >= p->block_count) { p->status = 6; p->state = 0xfe; return 0; }
        p->remaining = 0;
        return 0;
    }
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padcmd", func_800D2E88);
#endif

extern int (*_padFuncChkEng)(PadCommand *);
extern void _padSetActAlign_snd(PadCommand *);
extern int func_800D3180(PadCommand *);
int _padSetActAlign(PadCommand *p, u8 *data) {
    if (_padFuncChkEng(p) == 0) {
        p->state = 1;
        p->send = _padSetActAlign_snd;
        p->alignment = data;
        p->receive = func_800D3180;
        return 1;
    }
    return 0;
}

void _padSetActAlign_snd(PadCommand *p) {
    p->command = 0x4d;
    p->command_length = 6;
    p->command_data = p->alignment;
}

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 84.62% (44/52 words;
 * GCC 2.8.0, -O2). */
int func_800D3180(PadCommand *p) {
    int i, j, count, needed;
    u8 *align;
    for (i = 0; i < p->actuator_count; i++) {
        align = p->alignment;
        count = 0;
        for (j = 5; j >= 0; j--) if (*align++ == i) count++;
        needed = (*(u8 **)((u8 *)p + 4))[i * 5 + 2];
        if (!needed) needed = 1;
        align = p->alignment;
        for (j = 0; j < 6; j++) {
            if (*align++ == i) {
                if (count < needed) { p->alignment_data[j] = 0xff; count--; }
                else p->alignment_data[j] = i;
            }
        }
    }
    p->state = 0xfe;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padcmd", func_800D3180);
#endif

#ifdef NON_MATCHING
extern void _padSetMainMode_snd(PadCommand *);
extern int _padSetMainMode_rcv(PadCommand *);
/* Best observed instruction-sequence similarity: 76.32% (29/38 words, -O2). */
int _padSetMainMode(PadCommand *p, u8 mode, u8 lock) {
    if (_padFuncChkEng(p) == 0) {
        p->state = 1;
        p->send = _padSetMainMode_snd;
        p->receive = _padSetMainMode_rcv;
        p->mode_data[0] = mode;
        p->mode_data[1] = lock;
        p->mode_status = mode == p->mode_index;
        return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libpad/padcmd", _padSetMainMode);
#endif

void _padSetMainMode_snd(PadCommand *p) {
    int state = p->state;
    if (state == 2) goto case2;
    if (state == 3) goto case3;
    goto end;
case2:
    p->command = 0x44;
    p->command_data = p->mode_data;
    p->command_length = state;
    goto end;
case3:
    p->command = 0x4d;
    p->command_data = p->alignment_data;
    p->command_length = 6;
end: ;
}

extern void (*_padFuncClrInfo)(PadCommand *);

int _padSetMainMode_rcv(PadCommand *p) {
    if (p->mode_status != 0) {
        if (p->state == 2)
            return 1;
        p->state = 0xfe;
        return 0;
    }
    _padFuncClrInfo(p);
    return 0;
}

int _padCmdParaMode(PadCommand *p, int mode) {
    int r;
    p->command = 0x43;
    p->command_data = &p->parameter;
    r = 1;
    p->parameter = mode;
    p->command_length = r;
    return r;
}

int _padCmdGetStatus(PadCommand *p) {
    int r = 0x45;
    p->command = r;
    p->command_data = 0;
    p->command_length = 0;
    return r;
}

int _padCmdSetMap(PadCommand *p, int mode) {
    int r;
    p->command = 0x4c;
    p->command_data = &p->parameter;
    r = 1;
    p->parameter = mode;
    p->command_length = r;
    return r;
}

int _padCmdGetDescR0(PadCommand *p, int mode) {
    int r;
    p->command = 0x46;
    p->command_data = &p->parameter;
    r = 1;
    p->parameter = mode;
    p->command_length = r;
    return r;
}

int _padCmdGetDescR1(PadCommand *p, int mode) {
    int r;
    p->command = 0x47;
    p->command_data = &p->parameter;
    r = 1;
    p->parameter = mode;
    p->command_length = r;
    return r;
}

int _padCmd4B(PadCommand *p) {
    int r = 0x4b;
    p->command = r;
    p->command_data = 0;
    p->command_length = 0;
    return r;
}
