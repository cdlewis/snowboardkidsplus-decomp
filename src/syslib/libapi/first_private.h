#pragma once

#include "decomp/types.h"

typedef int (*FirstHandler)(void *, void *, void *);
typedef struct {
    char *name;
    u8 unknown[0x30];
    FirstHandler first;
    u8 rest[0x18];
} Device;

/* Fixed BIOS Device Control Block table locations.
 * The size is in bytes; a device record occupies 0x50 bytes. */
#define BIOS_DCB_BASE (*(Device **)0x150)
#define BIOS_DCB_BYTES (*(u32 *)0x154)

extern FirstHandler firstFileHandler;
extern char firstFileDeviceName[];
extern int strcmp(const char *, const char *);
extern int firstfileDeviceCallback(int *, void *, void *);
extern void *firstfile2(const char *, void *);
