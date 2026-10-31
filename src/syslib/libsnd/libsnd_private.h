#ifndef LIBSND_PRIVATE_H
#define LIBSND_PRIVATE_H

#include "psx/libsnd.h"
#include "decomp/types.h"

/* Shared VM state belongs to the SDK's vm_g data object, still in split data. */
extern s16 _svm_stereo_mono;
extern s16 _svm_damper;
void _SsStart(s32 use_vsync);
void _SsVmDamperOff(void);
void _SsVmDamperOn(void);

#endif
