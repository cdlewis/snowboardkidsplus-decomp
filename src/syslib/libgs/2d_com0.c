#include "common.h"

struct SpriteState {
    u8 unknown[24];
    u32 expand;
};
extern void _mk_xpndsp(void *, struct SpriteState *, void *);
extern void _mk_normsp(void *, struct SpriteState *, void *);

void _mk_spr_packet(void *packet, struct SpriteState *sprite, void *data) {
    if (sprite->expand)
        _mk_xpndsp(packet, sprite, data);
    else
        _mk_normsp(packet, sprite, data);
}

INCLUDE_ASM("asm/nonmatchings/syslib/libgs/2d_com0", _mk_xpndsp);

INCLUDE_ASM("asm/nonmatchings/syslib/libgs/2d_com0", _mk_normsp);
