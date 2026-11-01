#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 97.14% (34/35 words;
 * GCC 2.8.0). The loop branch also targets the preceding multiply. */
u32 _spu_2pitch(u32 center, u32 note) {
    u32 p0 = center << 12;
    u32 factor = 0x103B;
    u32 p1 = center * factor;
    s32 s = note / 32;
    s32 f = note % 32;
    s32 i;

    for (i = 0; i < s; i++) {
        p0 = center * factor;
        factor *= 0x103B;
        factor >>= 12;
        p1 = center * factor;
    }
    return (p0 + ((p1 - p0) >> 5) * f) >> 12;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_n2p", _spu_2pitch);
#endif

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 33.33% (26/78 words). */
u16 _spu_note2pitch(u16 center, u16 centerfine, u16 note, u16 fine) {
    int diff = (note * 128 + fine) - (center * 128 + centerfine);
    int n = diff < 0 ? -diff : diff;
    int octave = n / 1536;
    int rem = n % 1536;
    u32 pitch, factor = 0x103b, low, high;
    int i;
    if (diff >= 0) pitch = 0x1000 << octave;
    else {
        if (rem) { octave++; rem = 1536 - rem; }
        pitch = 0x1000 >> octave;
    }
    pitch = (u16)pitch;
    n = rem < 0 ? -rem : rem;
    low = pitch << 12;
    high = pitch * factor;
    for (i = 0; i < n / 32; i++) {
        low = pitch * factor;
        factor = (factor * 0x103b) >> 12;
        high = pitch * factor;
    }
    pitch = (low + (((high - low) >> 5) * (n & 31))) >> 12;
    if (pitch >= 0x4000) pitch = 0x3fff;
    return pitch;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_n2p", _spu_note2pitch);
#endif

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 12.00% (9/75 words; target includes padding). */
u32 _spu_pitch2note(u16 center, u16 fine, u16 pitch) {
    int bit, position = 0, octave, i, j, fraction = 0x600;
    u32 base, factor = 0x1000;
    for (bit = 15; bit >= 0; bit--) {
        if (pitch & (1 << bit)) { position = bit; break; }
    }
    octave = position - 12;
    base = 1 << position;
    for (i = 0; i < 48; i++) {
        u32 low = base * factor;
        u32 high, step, a = 0, b;
        factor = (factor * 0x103b) >> 12;
        high = base * factor;
        step = (high - low) >> 5;
        b = step;
        for (j = 0; j < 32; j++) {
            if (pitch >= ((low + a) >> 12) && pitch < ((low + b) >> 12)) {
                fraction = i * 32 + j;
                goto done;
            }
            a += step;
            b += step;
        }
    }
done:
    return ((center + fraction / 128 + octave * 12) << 8) | (fine + fraction % 128);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libspu/s_n2p", _spu_pitch2note);
#endif
