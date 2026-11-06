#pragma once

#include "decomp/types.h"

s16 SsSeqOpen(u32 *sequence, s16 bankId);
void SsUtSetReverbDepth(s16 left, s16 right);
void SsSeqClose(s16 id);
void SsSepClose(s16 id);
void SsSeqStop(s16 id);
void SsSeqPause(s16 id);
void SsSeqReplay(s16 id);
void SsSepStop(s16 id, s16 sequence);
void SsSeqSetVol(s16 id, s16 left, s16 right);
void SsSeqPlay(s16 id, s8 mode, s16 count);
void SsSepPlay(s16 id, s16 sequence, s8 mode, s16 count);
s16 SsVabOpenHead(u8 *address, s16 id);
s16 SsVabOpenHeadSticky(u8 *address, s16 id, u32 start);
s16 SsVabFakeHead(u8 *address, s16 id, u32 start);
s16 SsVabTransBody(u8 *body, s16 id);
s16 SsVabTransCompleted(s16 immediate);

void SsStart(void);
void SsEnd(void);
void SsStart2(void);
void SsSetMono(void);
void SsSetStereo(void);
void SsSetMVol(s16 left, s16 right);

void SsSetSerialVol(s8 serial, s16 left, s16 right);

s16 SsUtKeyOffV(s16 voice);
