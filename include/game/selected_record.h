#pragma once

#include "decomp/types.h"

typedef struct SelectedRecord {
    u16 vertexIndices[3];
    s16 unk6;
    s16 unk8;
    s16 unkA;
} SelectedRecord;

#ifdef SELECTED_RECORD_GP_ADDRESSING
extern s16 *D_8013B1EC;
extern SelectedRecord *D_8013B1F0;
extern s16 *D_8013B1F4;
extern s16 D_8013B1F8;
extern s16 D_8013B1FA;
extern s16 D_8013B1FC;
#else
extern SelectedRecord *D_8013B1F0 __attribute__((section(".data")));
#endif
extern s16 D_8013B1FE __attribute__((section(".data")));


void bindGeometryRecordData(s16 *data);
