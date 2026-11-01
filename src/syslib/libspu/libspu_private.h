#ifndef LIBSPU_PRIVATE_H
#define LIBSPU_PRIVATE_H

#include "psx/libspu.h"

#define NUM_VOICES 24
typedef struct tagSpuVoiceRegister {
    /* 0x00 */ SpuVolume volume;
    /* 0x04 */ u16 pitch;
    /* 0x06 */ u16 addr;
    /* 0x08 */ u16 adsr[2];
    /* 0x0C */ u16 volumex;
    /* 0x0E */ u16 loop_addr;
} SPU_VOICE_REG; // size:0x10

typedef struct tagSpuControl {
    /* 0x1F801C00 0x000 */ SPU_VOICE_REG voice[NUM_VOICES];
    /* 0x1F801D80 0x180 */ SpuVolume main_vol;
    /* 0x1F801D84 0x184 */ SpuVolume rev_vol;
    /* 0x1F801D88 0x188 */ u16 key_on[2];
    /* 0x1F801D8C 0x18C */ u16 key_off[2];
    /* 0x1F801D90 0x190 */ u16 chan_fm[2];
    /* 0x1F801D94 0x194 */ u16 noise_mode[2];
    /* 0x1F801D98 0x198 */ u16 rev_mode[2];
    /* 0x1F801D9C 0x19C */ u32 chan_on;
    /* 0x1F801DA0 0x1A0 */ u16 unk;
    /* 0x1F801DA2 0x1A2 */ u16 rev_work_addr;
    /* 0x1F801DA4 0x1A4 */ u16 irq_addr;
    /* 0x1F801DA6 0x1A6 */ u16 trans_addr;
    /* 0x1F801DA8 0x1A8 */ u16 trans_fifo;
    /* 0x1F801DAA 0x1AA */ u16 spucnt;
    /* 0x1F801DAC 0x1AC */ u16 data_trans;
    /* 0x1F801DAE 0x1AE */ u16 spustat; // read-only on hardware
    /* 0x1F801DB0 0x1B0 */ SpuVolume cd_vol;
    /* 0x1F801DB4 0x1B4 */ SpuVolume ex_vol;
    /* 0x1F801DB8 0x1B8 */ SpuVolume main_volx;
    /* 0x1F801DBC 0x1BC */ SpuVolume unk_vol;
    /* 0x1F801DC0 0x1C0 */ u16 dAPF1;
    /* 0x1F801DC2 0x1C2 */ u16 dAPF2;
    /* 0x1F801DC4 0x1C4 */ u16 vIIR;
    /* 0x1F801DC6 0x1C6 */ u16 vCOMB1;
    /* 0x1F801DC8 0x1C8 */ u16 vCOMB2;
    /* 0x1F801DCA 0x1CA */ u16 vCOMB3;
    /* 0x1F801DCC 0x1CC */ u16 vCOMB4;
    /* 0x1F801DCE 0x1CE */ u16 vWALL;
    /* 0x1F801DD0 0x1D0 */ u16 vAPF1;
    /* 0x1F801DD2 0x1D2 */ u16 vAPF2;
    /* 0x1F801DD4 0x1D4 */ u16 mLSAME;
    /* 0x1F801DD6 0x1D6 */ u16 mRSAME;
    /* 0x1F801DD8 0x1D8 */ u16 mLCOMB1;
    /* 0x1F801DDA 0x1DA */ u16 mRCOMB1;
    /* 0x1F801DDC 0x1DC */ u16 mLCOMB2;
    /* 0x1F801DDE 0x1DE */ u16 mRCOMB2;
    /* 0x1F801DE0 0x1E0 */ u16 dLSAME;
    /* 0x1F801DE2 0x1E2 */ u16 dRSAME;
    /* 0x1F801DE4 0x1E4 */ u16 mLDIFF;
    /* 0x1F801DE6 0x1E6 */ u16 mRDIFF;
    /* 0x1F801DE8 0x1E8 */ u16 mLCOMB3;
    /* 0x1F801DEA 0x1EA */ u16 mRCOMB3;
    /* 0x1F801DEC 0x1EC */ u16 mLCOMB4;
    /* 0x1F801DEE 0x1EE */ u16 mRCOMB4;
    /* 0x1F801DF0 0x1F0 */ u16 dLDIFF;
    /* 0x1F801DF2 0x1F2 */ u16 dRDIFF;
    /* 0x1F801DF4 0x1F4 */ u16 mLAPF1;
    /* 0x1F801DF6 0x1F6 */ u16 mRAPF1;
    /* 0x1F801DF8 0x1F8 */ u16 mLAPF2;
    /* 0x1F801DFA 0x1FA */ u16 mRAPF2;
    /* 0x1F801DFC 0x1FC */ u16 vLIN;
    /* 0x1F801DFE 0x1FE */ u16 vRIN;
} SPU_RXX;

union SpuUnion {
    volatile SPU_RXX rxx;
    volatile u16 raw[0x100];
};


typedef struct {
    u32 addr, size;
} SPU_MALLOC;

extern union SpuUnion *_spu_RXX;
extern u16 _spu_tsa;
extern u32 _spu_FsetRXXa(s32, u32);
extern s32 _spu_mem_mode_plus;
extern s32 _spu_AllocBlockNum, _spu_AllocLastNum;
extern SPU_MALLOC *_spu_memList;
#define SPURV(n, field) (((SPU_VOICE_REG *)_spu_RXX)[n].field)

s32 _SpuIsInAllocateArea(u32 address);
s32 _SpuIsInAllocateArea_(u32 address);
void _spu_FsetRXX(u32 offset, u32 value, u32 mode);
u32 _spu_FgetRXXa(s32 offset, s32 mode);
void _spu_FsetPCR(s32 mode);
void _spu_FsetDelayW(void);
void _spu_FsetDelayR(void);

/* Shared state is owned by the SDK spu data object, still in split data. */
extern s32 _spu_inTransfer;
extern s32 _spu_trans_mode;
extern s32 _spu_transMode;
u32 _SpuSetAnyVoice(s32 on_off, u32 voice_bit, s32 first, s32 second);
void _spu_setInTransfer(s32 value);
s32 _spu_getInTransfer(void);

#endif
