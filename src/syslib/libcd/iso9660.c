#include "common.h"
#include "psx/libcd.h"

#ifdef NON_MATCHING
typedef struct { CdlLOC pos; u32 size; char name[16]; } CdFile;
extern int D_80134CD0, D_801349CC, CD_debug;
extern CdFile D_8013D210[];
extern const char D_80012810[], D_8001282C[], D_80012844[], D_80012860[], D_80012880[], D_8001288C[];
extern int func_800B8590(void), func_800B8854(int, char *), func_800B88F8(int);
extern int _cd_cmp_name(const char *, const char *), printf(const char *, ...);

/* Best observed instruction-sequence similarity: 74.73% (136/182 words;
 * GCC 2.7.2). */
CdFile *CdSearchFile(CdFile *file, const char *path) {
    char name[32], *out;
    const char *p;
    int dir = 1, depth = 0, i;
    if (D_80134CD0 != D_801349CC) {
        if (!func_800B8590()) return 0;
        D_80134CD0 = D_801349CC;
    }
    if (*path != '\\') return 0;
    name[0] = 0;
    p = path;
    do {
        out = name;
        while (*p && *p != '\\') *out++ = *p++;
        if (!*p) break;
        p++;
        *out = 0;
        dir = func_800B8854(dir, name);
        if (dir == -1) { name[0] = 0; break; }
        depth++;
    } while (depth < 8);
    if (depth >= 8) {
        if (CD_debug > 0) printf(D_80012810, path, depth);
        return 0;
    }
    if (!name[0]) {
        if (CD_debug > 0) printf(D_8001282C, path);
        return 0;
    }
    *out = 0;
    if (!func_800B88F8(dir)) {
        if (CD_debug > 0) printf(D_80012844);
        return 0;
    }
    if (CD_debug >= 2) printf(D_80012860, name);
    for (i = 0; i < 64 && D_8013D210[i].name[0]; i++) {
        if (_cd_cmp_name(D_8013D210[i].name, name)) {
            if (CD_debug >= 2) printf(D_80012880, name);
            *file = D_8013D210[i];
            return &D_8013D210[i];
        }
    }
    if (CD_debug > 0) printf(D_8001288C, name);
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/iso9660", CdSearchFile);
#endif

extern int strncmp(const char *a, const char *b, u32 count);

int _cd_cmp_name(const char *a, const char *b) {
    return (unsigned)strncmp(a, b, 12) < 1U;
}

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/iso9660", func_800B8590);

#ifdef NON_MATCHING
typedef struct { int parent; u32 unknown; char name[36]; } IsoEntry;
extern IsoEntry D_8013D814[];
extern int strcmp(const char *, const char *);
/* Best observed instruction-sequence similarity: 92.68% (38/41 words). */
int func_800B8854(int parent, char *name) {
    int i;
    for (i = 0; i < 128; i++) {
        if (!D_8013D814[i].parent) break;
        if (D_8013D814[i].parent == parent && !strcmp(name, D_8013D814[i].name))
            return i + 1;
    }
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/iso9660", func_800B8854);
#endif

INCLUDE_ASM("asm/nonmatchings/syslib/libcd/iso9660", func_800B88F8);

extern s32 CdControl(u8, u8 *, u8 *), CdRead(s32, u32 *, s32), CdReadSync(s32, u8 *);

s32 func_800B8B94(s32 count, s32 sector, u32 *buffer) {
    CdlLOC loc;
    CdIntToPos(sector, &loc);
    CdControl(2, (u8 *)&loc, 0);
    CdRead(count, buffer, 0x80);
    return CdReadSync(0, 0) == 0;
}
