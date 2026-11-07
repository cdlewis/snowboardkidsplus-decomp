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
s32 MemCardAccept(s32 chan);
s32 MemCardReadFile(s32 chan, char *name, void *buffer, s32 offset, s32 bytes);
s32 MemCardSync(s32 mode, s32 *cmds, s32 *result);
void MemCardInit(long val);
void MemCardStart(void);
