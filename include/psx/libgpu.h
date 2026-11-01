#ifndef PSX_LIBGPU_H
#define PSX_LIBGPU_H

#include "decomp/types.h"

typedef struct RECT { s16 x, y, w, h; } RECT;

typedef struct P_TAG {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

typedef struct DR_ENV { u32 tag; u32 code[15]; } DR_ENV;
typedef struct DR_TWIN { u32 tag; u32 code[2]; } DR_TWIN;
typedef struct DR_AREA { u32 tag; u32 code[2]; } DR_AREA;
typedef struct DR_OFFSET { u32 tag; u32 code[2]; } DR_OFFSET;
typedef struct DR_PRIO { u32 tag; u32 code[2]; } DR_PRIO;
typedef struct DR_MODE { u32 tag; u32 code[2]; } DR_MODE;
typedef struct DR_STP { u32 tag; u32 code[2]; } DR_STP;
typedef struct DR_TPAGE { u32 tag; u32 code[1]; } DR_TPAGE;
typedef struct DR_MOVE { u32 tag; u32 code[5]; } DR_MOVE;

/* GetDrawEnv copies 0x5C bytes in this SDK, including the packed commands. */
typedef struct DRAWENV {
    RECT clip;
    s16 ofs[2];
    RECT tw;
    u16 tpage;
    u8 dtd, dfe, isbg, r0, g0, b0;
    DR_ENV dr_env;
} DRAWENV;

typedef struct DISPENV {
    RECT disp, screen;
    u8 isinter, isrgb24, pad0, pad1;
} DISPENV;

#define setlen(p, length) (((P_TAG *)(p))->len = (u8)(length))
#define getlen(p) ((u8)(((P_TAG *)(p))->len))
#define getTPage(tp, abr, x, y) \
    ((((tp) & 3) << 7) | (((abr) & 3) << 5) | (((y) & 0x100) >> 4) | \
     (((x) & 0x3FF) >> 6) | (((y) & 0x200) << 2))
#define getClut(x, y) (((y) << 6) | (((x) >> 4) & 0x3F))

u16 GetTPage(s32 tp, s32 abr, s32 x, s32 y);
u16 GetClut(s32 x, s32 y);
void AddPrim(void *ordering_table, void *primitive);
void SetSemiTrans(void *primitive, s32 enable);
s32 LoadImage(RECT *rect, u32 *pixels);
u16 LoadTPage(u32 *pixels, s32 tp, s32 abr, s32 x, s32 y, s32 w, s32 h);
u16 LoadClut(u32 *clut, s32 x, s32 y);
u16 LoadClut2(u32 *clut, s32 x, s32 y);
DRAWENV *SetDefDrawEnv(DRAWENV *env, s32 x, s32 y, s32 w, s32 h);
DISPENV *SetDefDispEnv(DISPENV *env, s32 x, s32 y, s32 w, s32 h);
DRAWENV *GetDrawEnv(DRAWENV *env);
DISPENV *GetDispEnv(DISPENV *env);
s32 GetGraphDebug(void);
s32 GetODE(void);
void DrawPrim(void *primitive);
void SetTexWindow(DR_TWIN *packet, RECT *window);
void SetDrawArea(DR_AREA *packet, RECT *area);
void SetDrawOffset(DR_OFFSET *packet, u16 *offset);
void SetPriority(DR_PRIO *packet, s32 pbc, s32 pbw);
void SetDrawStp(DR_STP *packet, s32 stp);
void SetDrawTPage(DR_TPAGE *packet, s32 dfe, s32 dtd, s32 tpage);
void SetDrawMove(DR_MOVE *packet, RECT *rect, s32 x, s32 y);
void SetDrawMode(DR_MODE *packet, s32 dfe, s32 dtd, s32 tpage, RECT *window);

#endif
