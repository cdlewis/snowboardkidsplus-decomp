#include "common.h"

typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX;
extern int rsin(int), rcos(int);
extern MATRIX *MulMatrix(MATRIX *, MATRIX *);

void gte_rotate_z_matrix(MATRIX *m, int angle) {
    MATRIX r;
    int scaled = angle / 360;
    int c = rcos(scaled);
    int s = rsin(scaled);
    if (angle) {
        r.m[0][0] = c; r.m[0][1] = -s; r.m[0][2] = 0;
        r.m[1][0] = s; r.m[1][1] = c; r.m[1][2] = 0;
        r.m[2][0] = 0; r.m[2][1] = 0; r.m[2][2] = 0x1000;
        r.t[0] = 0; r.t[1] = 0; r.t[2] = 0;
        MulMatrix(m, &r);
    }
}
