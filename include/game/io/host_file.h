#pragma once

#include "common.h"

int readHostFile(char *path, char *buffer);
int writePcHostFile(char *name, char *buffer, int length);
s16 readPcHostAsset(s16 mode, char *name, void *buffer, s16 *position);
