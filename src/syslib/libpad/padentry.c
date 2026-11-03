#include "common.h"
#include "libpad_private.h"

extern int func_800D2200(void);
extern void func_800D2214(void);
extern void func_800D22E0(void);
extern PadCommand *(*_padFuncPort2Info)(int);
extern int _padSetActAlign(PadCommand *, u8 *);
extern int _padSetMainMode(PadCommand *, u8, u8);
extern void _padSetAct(PadCommand *, u8 *, int);

int PadChkVsync(void) { return func_800D2200(); }

void PadStartCom(void) { func_800D2214(); }

void PadStopCom(void) { func_800D22E0(); }

extern int _padModeMtap;
extern PadCommand *_padInfoDir;

int PadChkMtap(int port) {
    if (_padModeMtap) {
        return _padInfoDir[port >> 4].unknownE5[3] == 8;
    }
    return 0;
}

int PadGetState(int port) {
    PadCommand *p = _padFuncPort2Info(port);
    if ((*(u32 *)((u8 *)p + 0x34) & 0xffff0000) ||
        ((p != *(PadCommand **)((u8 *)p + 0x10) && *((u8 *)p + 0x38)) ||
         **(u8 **)((u8 *)p + 0x30))) {
        switch (p->unknown49[0]) {
        case 2: return 1;
        case 3: return 1;
        case 6: return 4;
        }
    }
    return p->unknown49[0];
}

int PadInfoMode(int port, int mode, int index) {
    PadCommand *p = _padFuncPort2Info(port);
    switch (mode) {
    case 1: return *((u8 *)p + 0xe8);
    case 2: return *(u16 *)((u8 *)p + 0xe6);
    case 3: return p->mode_index;
    case 4:
        if (index < 0) return p->mode_count;
        if (index < p->mode_count) return (*(u16 **)p)[index];
        else return 0;
    case 100: return *(int *)((u8 *)p + 0x4c);
    default: return 0;
    }
}

int PadInfoAct(int port, int index, int request) {
    PadCommand *p = _padFuncPort2Info(port);
    PadActuatorInfo *a;
    if (index < 0) return p->actuator_count;
    if (index < p->actuator_count) {
        a = &p->actuator_info[index];
        switch (request) {
        case 1: return a->data[0];
        case 2: return a->data[1];
        case 3: return a->data[2];
        case 4: return a->data[3];
        case 5: return a->data[4];
        }
    }
    return 0;
}

typedef struct {
    u8 count, pad[3];
    u8 *items;
} PadComb;

int PadInfoComb(int port, int index, int entry) {
    PadCommand *p = _padFuncPort2Info(port);
    PadComb *comb;
    if (index < 0)
        return p->unknownEA[0];
    if (index < p->unknownEA[0]) {
        comb = &((PadComb **)((u8 *)p + 8))[0][index];
        if (entry < 0)
            return comb->count;
        if (entry < comb->count)
            return comb->items[entry];
        return 0;
    } else {
        return 0;
    }
}

int PadSetActAlign(int port, u8 *data) {
    return _padSetActAlign(_padFuncPort2Info(port), data);
}

int PadSetMainMode(int port, int mode, int lock) {
    return _padSetMainMode(_padFuncPort2Info(port), mode, lock);
}

void PadSetAct(int port, u8 *data, int size) {
    _padSetAct(_padFuncPort2Info(port), data, size);
}
