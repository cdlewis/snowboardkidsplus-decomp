#include "game/memory_card.h"
#include "psx/libmcrd.h"

s32 pollMemoryCardRead(void) {
    s32 command, result;
    D_8013B608[D_8013AF38] = 0;
    D_8013B60C[D_8013AF38] = 0;
    switch (MemCardSync(1, &command, &result)) {
    case -1:
        MemCardReadFile(D_8013AF34, D_8013AF40, &gMemoryCardSaveHeader, 0, gMemoryCardSaveCapacities[D_8013AF48]);
        break;
    case 0: break;
    case 1:
        if (command == 3) {
            switch (result) {
            case 0:
                if (validateAndCopyMemoryCardSave()) return 1;
                return 5;
            case 1: return 2;
            case 2: return 3;
            case 3: MemCardAccept(D_8013AF34); break;
            case 5: return 4;
            }
        }
        break;
    }
    return 0;
}
