#include "libspu_private.h"

void SpuGetVoiceEnvelope(s32 voice, s16 *envelope) { *envelope = SPURV(voice, volumex); }
