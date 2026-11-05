#pragma once

#include "decomp/types.h"

struct DIRENTRY {
    char name[20];
    s32 attr;
    s32 size;
    struct DIRENTRY *next;
    s32 head;
    char system[4];
};

s32 MemCardGetDirentry(s32 chan, char *name, struct DIRENTRY *dir,
                       s32 *files, s32 ofs, s32 max);
