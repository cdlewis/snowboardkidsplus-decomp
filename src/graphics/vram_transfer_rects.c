#include "psx/libgpu.h"

RECT gReadPixelRect __attribute__((section(".data.read_pixel_rect"))) = {0, 0, 1, 1};

RECT gWritePixelRect __attribute__((section(".sdata.write_pixel_rect"))) = {0, 0, 1, 1};

RECT gScanlineCopyRect __attribute__((section(".data.scanline_copy_rects"))) = {640, 256, 64, 1};

RECT gScanlineSeedRect __attribute__((section(".data.scanline_copy_rects"))) = {704, 144, 64, 1};

RECT D_8013B260 __attribute__((section(".data.scanline_copy_rects"))) = {640, 256, 64, 1};
