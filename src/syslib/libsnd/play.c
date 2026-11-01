#include "common.h"

extern void _SsSeqPlay(s16 id, s16 sequence);

void _SsSndPlay(s16 id, s16 sequence) { _SsSeqPlay(id, sequence); }
