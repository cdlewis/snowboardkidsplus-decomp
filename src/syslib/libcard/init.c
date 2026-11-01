#include "common.h"

extern void ChangeClearPAD(int), EnterCriticalSection(void), ExitCriticalSection(void);
extern int ReadInitPadFlag(void);
extern void InitCARD2(int), _copy_memcard_patch(void), _patch_card(void), _patch_card2(void);
extern void StartCARD2(void), StopCARD2(void), _ExitCard(void);

void InitCARD(int mode) {
    ChangeClearPAD(0);
    EnterCriticalSection();
    if (!ReadInitPadFlag()) mode = 0;
    InitCARD2(mode);
    _copy_memcard_patch();
    _patch_card();
    _patch_card2();
    ExitCriticalSection();
}

void StartCARD(void) {
    EnterCriticalSection();
    StartCARD2();
    ChangeClearPAD(0);
    ExitCriticalSection();
}

void StopCARD(void) {
    StopCARD2();
    _ExitCard();
}
