#ifndef LIBSND_PRIVATE_H
#define LIBSND_PRIVATE_H

#include "psx/libsnd.h"
#include "decomp/types.h"

/* Shared voice-record view beginning at the first field accessed here. */
typedef struct {
    s16 field0;
    u8 reserved02[2];
    s16 field4;
    u8 reserved06[10];
    s16 seq_id;
    u8 reserved12[11];
    u8 field1d;
    u8 reserved1e[24];
} VoiceSlice;
extern VoiceSlice D_8013C7C8[];

typedef struct {
    u8 prior, mode, vol, pan, center, shift, min, max;
    u8 vibW, vibT, porW, porT, pbmin, pbmax, reserved1, reserved2;
    u16 adsr1, adsr2;
    s16 prog, vag, reserved[4];
} VagAtr;

typedef struct {
    u8 tones, mvol, prior, mode, mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1, reserved2;
} ProgAtr; /* size: 0x10 */

typedef struct {
    u16 attack_rate, decay_rate, sustain_level, sustain_rate, release_rate;
    s16 attack_mode, sustain_mode;
    s16 release_mode;
    s16 sustain_direction;
} Envelope;

s16 SsUtGetVagAtr(s16 vab, s16 program, s16 tone, VagAtr *attr);
s16 SsUtSetVagAtr(s16 vab, s16 program, s16 tone, VagAtr *attr);
s16 SsUtGetProgAtr(s16 vab, s16 program, ProgAtr *attr);
void _SsUtResolveADSR(u16 adsr1, u16 adsr2, Envelope *env);
void _SsUtBuildADSR(Envelope *env, u16 *adsr1, u16 *adsr2);

/* Sequence layout adapted from PsyZ; see THIRD_PARTY.md. */
struct SeqStruct {
    /* 0x00 */ u8* seq_ptr;
    /* 0x04 */ u8* read_pos;
    /* 0x08 */ u8* next_sep_pos;
    /* 0x0C */ u8* loop_pos;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u8 play_mode;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ u8 running_status;
    /* 0x17 */ u8 channel_idx;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 unk19;
    /* 0x1A */ u8 fn_idx;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ u8 l_count;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u8 rhythm_n;
    /* 0x25 */ u8 rhythm_d;
    /* 0x26 */ u8 vab_id;
    /* 0x27 */ u8 panpot[16];
    /* 0x37 */ u8 programs[16];
    /* 0x47 */ u8 unk47;
    /* 0x48 */ short vol_l;
    /* 0x4A */ short vol_r;
    /* 0x4C */ s16 unk4C;
    /* 0x4E */ s16 unk4E;
    /* 0x50 */ s16 resolution;
    /* 0x52 */ s16 unk52;
    /* 0x54 */ s16 unk54;
    /* 0x56 */ s16 unk56;
    /* 0x58 */ u16 voll;
    /* 0x5A */ u16 volr;
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
    /* 0x60 */ short vol[16];
    /* 0x80 */ s16 channel_mute;
    /* 0x82 */ u8 unk82;
    /* 0x83 */ u8 unk83;
    /* 0x84 */ s32 unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 tempo;
    /* 0x90 */ s32 delta_value;
    /* 0x94 */ u32 unk94;
    /* 0x98 */ unsigned int flags;
    /* 0x9C */ int v_time_l;
    /* 0xA0 */ int v_time_r;
    /* 0xA4 */ u32 unkA4;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
};

extern struct SeqStruct *D_8013BF10[];

/* Shared VM state belongs to the SDK's vm_g data object, still in split data. */
extern s16 _svm_stereo_mono;
extern s16 _svm_damper;
s16 _SsVmGetSeqVol(s16 id, s16 *left, s16 *right);
void _SsClose(s16 id);
void _SsSndStop(s16 id, s16 sequence);
void Snd_SetPlayMode(s16 id, s16 sequence, s8 mode, s16 count);
s16 SsVabOpenHeadWithMode(u8 *address, s16 id, s16 mode, u32 start);

void _SsStart(s32 use_vsync);
void _SsVmDamperOff(void);
void _SsVmDamperOn(void);

#endif
