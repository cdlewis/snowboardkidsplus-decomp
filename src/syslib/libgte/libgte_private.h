#pragma once

#include "common.h"

typedef struct {
    s16 sine;
    s16 cosine;
} GteSinCos;

extern s16 rsin_tbl[1028];
extern s16 SQRT[192];
extern GteSinCos rcossin_tbl[4096];
