#include "common.h"

#ifdef NON_MATCHING
#include "psx/libcd.h"
typedef struct { CdlLOC pos; u32 size; char name[16]; } CdFile;
extern CdFile D_8013F610;
extern const char D_80012A50[];
extern char *strcpy(char *, const char *), *strcat(char *, const char *);
extern CdFile *CdSearchFile(CdFile *, const char *);
extern int CdControl(u8, u8 *, u8 *), CdRead(int, u32 *, int);

/* Best observed instruction-sequence similarity: 76.60% (72/94 words;
 * GCC 2.8.0). */
int CdReadFile(char *path, u32 *buffer, int size) {
    signed char name[80];
    int i, sectors;
    if (path) {
        strcpy((char *)name, path);
        for (i = 0; name[i]; i++) {
            if ((unsigned char)(name[i] - 'a') < 26) name[i] -= 32;
        }
        if (name[i - 2] != ';') strcat((char *)name, D_80012A50);
        if (!CdSearchFile(&D_8013F610, (char *)name)) return 0;
    }
    if (!size || D_8013F610.size < (u32)size) size = D_8013F610.size;
    if (!CdControl(2, (u8 *)&D_8013F610.pos, 0)) return 0;
    sectors = (size + 0x7ff) / 0x800;
    if (!CdRead(sectors, buffer, 0x80)) return 0;
    CdIntToPos(CdPosToInt(&D_8013F610.pos) + sectors, &D_8013F610.pos);
    D_8013F610.size -= sectors;
    return size;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdreade", CdReadFile);
#endif

#ifdef NON_MATCHING
typedef struct { u32 words[15]; } CdExec;
extern CdExec D_8013F628;
extern int CdReadFile(char *, u32 *, int);
extern int CdReadSync(int, u8 *);

/* Best observed instruction-sequence similarity: 79.63% (43/54 words). */
CdExec *CdReadExec(char *path) {
    u32 buffer[512];

    if (!CdReadFile(path, buffer, 0x800)) return 0;
    if (CdReadSync(0, 0)) return 0;
    if (!CdReadFile(0, (u32 *)buffer[6], 0)) return 0;
    if (CdReadSync(0, 0)) return 0;
    D_8013F628 = *(CdExec *)&buffer[4];
    D_8013F628.words[8] = 0;
    D_8013F628.words[9] = 0;
    return &D_8013F628;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libcd/cdreade", CdReadExec);
#endif
