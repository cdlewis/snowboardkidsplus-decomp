#pragma once

#include "psx/libpress.h"

s32 ResetCallback(void);
s32 printf(const char *format, ...);
void MDEC_reset(s32 mode);
void MDEC_in(u32 *buffer, s32 size);
void MDEC_out(u32 *buffer, s32 size);
s32 MDEC_in_sync(void);
s32 MDEC_out_sync(void);
u32 MDEC_status(void);
s32 mdecTimeout(const char *name);

#define MDEC_CONTROL(value) (*mdec1 = (value))
#define MDEC_COMMAND(value) (*mdec0 = (value))
#define MDEC_READ_STATUS() (*mdec1)
#define MDEC_DMA_READ(channel, reg) (*mdec_d##channel##_##reg)
#define MDEC_DMA_WRITE(channel, reg, value) (*mdec_d##channel##_##reg = (value))
#define MDEC_DMA_ENABLE() (*mdec_d_pcr |= 0x88)
#define MDEC_ADDRESS(buffer) ((u32)(buffer))
