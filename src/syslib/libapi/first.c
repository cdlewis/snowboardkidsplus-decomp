#include "common.h"

#ifdef NON_MATCHING
typedef int (*FirstHandler)(void *, void *, void *);
typedef struct {
    char *name;
    u8 unknown[0x30];
    FirstHandler first;
    u8 rest[0x18];
} Device;
extern FirstHandler D_80141578;
extern char D_80141580[];
extern int strcmp(const char *, const char *);
extern int func_800D6010(int *, void *, void *);
extern void *firstfile2(const char *, void *);

/* Best observed instruction-sequence similarity: 77.67% (80/103 words;
 * GCC 2.7.2). */
void *firstfile(const char *path, void *entry) {
    const signed char *s = (const signed char *)path;
    char *out = D_80141580;
    Device *device, *end;
    int found = 0;
    while (*s >= ';') *out++ = *s++;
    *out = 0;
    device = *(Device **)0x150;
    end = device + (*(unsigned *)0x154 / 80);
    while (device < end) {
        if (device->name && !strcmp(device->name, D_80141580)) {
            D_80141578 = device->first;
            found = 1;
            break;
        }
        device++;
    }
    if (!found) return 0;
    device = *(Device **)0x150;
    end = device + (*(unsigned *)0x154 / 80);
    while (device < end) {
        if (device->name && !strcmp(device->name, D_80141580)) {
            device->first = (FirstHandler)func_800D6010;
            break;
        }
        device++;
    }
    return firstfile2(path, entry);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/first", firstfile);
#endif

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 90.77% (59/65 words;
 * GCC 2.7.2). */
int func_800D6010(int *context, void *name, void *entry) {
    Device *device, *end;
    FirstHandler handler;
    if (!*context) *context = 1;
    device = *(Device **)0x150;
    end = device + (*(unsigned *)0x154 / 80);
    handler = D_80141578;
    while (device < end) {
        if (device->name && !strcmp(device->name, D_80141580)) {
            device->first = handler;
            break;
        }
        device++;
    }
    return D_80141578(context, name, entry);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/first", func_800D6010);
#endif
