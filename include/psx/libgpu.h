#pragma once

#include "decomp/types.h"

typedef struct RECT { s16 x, y, w, h; } RECT;

typedef struct P_TAG {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} P_TAG;

typedef struct SPRT {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} SPRT;

/* Fixed-size 8 by 8 textured sprite. */
typedef struct SPRT_8 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} SPRT_8;

#define SPRT_8_CODE 0x74

/* Fixed-size 16 by 16 textured sprite. */
typedef struct SPRT_16 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
} SPRT_16;

#define SPRT_16_CODE 0x7C

typedef struct POLY_FT3 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} POLY_FT3;
#define POLY_FT3_CODE 0x24

typedef struct POLY_FT4 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;
#define POLY_FT4_CODE 0x2C

typedef struct LINE_F2 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1;
} LINE_F2;

typedef struct POLY_F4 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2, x3, y3;
} POLY_F4;
#define POLY_F4_CODE 0x28

typedef struct POLY_F3 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2;
} POLY_F3;
#define POLY_F3_CODE 0x20

typedef struct POLY_G3 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} POLY_G3;
#define POLY_G3_CODE 0x30

typedef struct POLY_G4 {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;
#define POLY_G4_CODE 0x38

typedef struct TILE {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0, w, h;
} TILE;


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
#define setcode(p, value) (((P_TAG *)(p))->code = (u8)(value))
#define getaddr(p) (((P_TAG *)(p))->addr)
#define setaddr(p, address) (((P_TAG *)(p))->addr = (u32)(address))
#define addPrim(ot, p) (setaddr(p, getaddr(ot)), setaddr(ot, p))
#define setRGB0(p, r, g, b) ((p)->r0 = (r), (p)->g0 = (g), (p)->b0 = (b))
#define setRGB1(p, r, g, b) ((p)->r1 = (r), (p)->g1 = (g), (p)->b1 = (b))
#define setRGB2(p, r, g, b) ((p)->r2 = (r), (p)->g2 = (g), (p)->b2 = (b))
#define setRGB3(p, r, g, b) ((p)->r3 = (r), (p)->g3 = (g), (p)->b3 = (b))
#define setXY0(p, x, y) ((p)->x0 = (x), (p)->y0 = (y))
#define setWH(p, width, height) ((p)->w = (width), (p)->h = (height))
#define setXY2(p, x_0, y_0, x_1, y_1) \
    ((p)->x0 = (x_0), (p)->y0 = (y_0), (p)->x1 = (x_1), (p)->y1 = (y_1))
#define setXY3(p, x_0, y_0, x_1, y_1, x_2, y_2) \
    ((p)->x0 = (x_0), (p)->y0 = (y_0), (p)->x1 = (x_1), (p)->y1 = (y_1), \
     (p)->x2 = (x_2), (p)->y2 = (y_2))
#define getlen(p) ((u8)(((P_TAG *)(p))->len))
#define getTPage(tp, abr, x, y) \
    ((((tp) & 3) << 7) | (((abr) & 3) << 5) | (((y) & 0x100) >> 4) | \
     (((x) & 0x3FF) >> 6) | (((y) & 0x200) << 2))
#define getClut(x, y) (((y) << 6) | (((x) >> 4) & 0x3F))

u16 GetTPage(s32 tp, s32 abr, s32 x, s32 y);
u16 GetClut(s32 x, s32 y);
void AddPrim(void *ordering_table, void *primitive);
void SetPolyFT4(P_TAG *packet);
void SetSemiTrans(void *primitive, s32 enable);
s32 LoadImage(RECT *rect, u32 *pixels);
s32 StoreImage(RECT *rect, u32 *pixels);
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
void SetSprt(P_TAG *packet);
void SetDrawTPage(DR_TPAGE *packet, s32 dfe, s32 dtd, s32 tpage);
void SetDrawMove(DR_MOVE *packet, RECT *rect, s32 x, s32 y);
void SetDrawMode(DR_MODE *packet, s32 dfe, s32 dtd, s32 tpage, RECT *window);

s32 DrawSync(s32 mode);

int MoveImage(RECT *rect, int x, int y);
