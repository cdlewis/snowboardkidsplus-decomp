/* TIM block parsing reconstructed from this SDK object's assembly oracle. */
#include "psx/libgs.h"

void GsGetTimInfo(u32 *tim, GsIMAGE *image) {
    u32 *next;
    image->pmode = *tim;
    if ((image->pmode >> 3) & 1) {
        tim++;
        next = tim + *tim / 4;
        tim++;
        image->cx = ((u16 *)tim)[0];
        image->cy = ((u16 *)tim)[1];
        tim++;
        image->cw = ((u16 *)tim)[0];
        image->ch = ((u16 *)tim)[1];
        tim++;
        image->clut = tim;
        next++;
        image->px = ((u16 *)next)[0];
        image->py = ((u16 *)next)[1];
        next++;
        image->pw = ((u16 *)next)[0];
        image->ph = ((u16 *)next)[1];
        next++;
        image->pixel = next;
    } else {
        tim += 2;
        image->px = ((u16 *)tim)[0];
        image->py = ((u16 *)tim)[1];
        tim++;
        image->pw = ((u16 *)tim)[0];
        image->ph = ((u16 *)tim)[1];
        tim++;
        image->pixel = tim;
    }
}
