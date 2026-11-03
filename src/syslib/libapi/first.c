#include "common.h"
#include "first_private.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 77.67% (80/103 words;
 * GCC 2.7.2). */
void *firstfile(const char *path, void *entry) {
    const signed char *s = (const signed char *)path;
    char *out = firstFileDeviceName;
    Device *device, *end;
    int found = 0;
    while (*s >= ';') *out++ = *s++;
    *out = 0;
    device = BIOS_DCB_BASE;
    end = device + (BIOS_DCB_BYTES / sizeof(Device));
    while (device < end) {
        if (device->name && !strcmp(device->name, firstFileDeviceName)) {
            firstFileHandler = device->first;
            found = 1;
            break;
        }
        device++;
    }
    if (!found) return 0;
    device = BIOS_DCB_BASE;
    end = device + (BIOS_DCB_BYTES / sizeof(Device));
    while (device < end) {
        if (device->name && !strcmp(device->name, firstFileDeviceName)) {
            device->first = (FirstHandler)firstfileDeviceCallback;
            break;
        }
        device++;
    }
    return firstfile2(path, entry);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libapi/first", firstfile);
#endif

int firstfileDeviceCallback(int *context, void *name, void *entry) {
    Device *device, *end;
    FirstHandler handler;
    if (!*context) *context = 1;
    device = BIOS_DCB_BASE;
    end = device + (BIOS_DCB_BYTES / sizeof(Device));
    handler = firstFileHandler;
    if (device < end) {
        Device *limit = end;
        do {
            if (device->name && !strcmp(device->name, firstFileDeviceName)) {
                device->first = handler;
                break;
            }
            device++;
        } while (device < limit);
    }
    return firstFileHandler(context, name, entry);
}
