#include "game/io/memory_card.h"
#include "psx/libmcrd.h"

s32 pollMemoryCardRead(void) {
    s32 command, result;
    gMemoryCardCreateFilePending[gMemoryCardPortIndex] = 0;
    gMemoryCardChanged[gMemoryCardPortIndex] = 0;
    switch (MemCardSync(1, &command, &result)) {
    case -1:
        MemCardReadFile(gMemoryCardChannel, gMemoryCardSelectedFileName, &gMemoryCardSaveHeader, 0, gMemoryCardSaveCapacities[gMemoryCardSelectedFileType]);
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
            case 3: MemCardAccept(gMemoryCardChannel); break;
            case 5: return 4;
            }
        }
        break;
    }
    return 0;
}
