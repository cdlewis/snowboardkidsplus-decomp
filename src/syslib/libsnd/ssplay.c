#include "common.h"
#include "libsnd_private.h"


void SsSeqPlay(s16 id,s8 mode,s16 count) { Snd_SetPlayMode(id,0,mode,count); }

void SsSepPlay(s16 id,s16 sequence,s8 mode,s16 count) { Snd_SetPlayMode(id,sequence,mode,count); }
