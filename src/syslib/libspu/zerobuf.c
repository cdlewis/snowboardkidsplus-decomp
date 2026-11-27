#include "libspu_private.h"

s8 _spu_zerobuf[1024] __attribute__((section(".data.spu_zero_buffer"), aligned(4))) = {0};
