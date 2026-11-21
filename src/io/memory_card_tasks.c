#include "card_movie_internal.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/io/memory_card.h"
#include "psx/libmcrd.h"

void *gMemoryCardTaskBuffer = NULL;
u32 gMemoryCardSaveBufferSize = 0;

s32 pollMemoryCardWrite(void) {
    s32 command, result;
    D_8013B60C[D_8013AF38] = 0;
    if (D_8013B608[D_8013AF38] == 1) {
        switch (MemCardCreateFile(D_8013AF34, D_8013AF40, (s8)gMemoryCardSaveHeader.blockCount)) {
        case 0: readMemoryCardDirectory();
            D_8013B608[D_8013AF38] = 0; break;
        case 1: return 2;
        case 2: return 3;
        case 4: return 4;
        case 6: D_8013B608[D_8013AF38] = 0; break;
        case 7: return 5;
        default: return 0;
        }
    }
    switch (MemCardSync(1, &command, &result)) {
    case -1:
        MemCardWriteFile(D_8013AF34, D_8013AF40, &gMemoryCardSaveHeader, 0, gMemoryCardSaveCapacities[D_8013AF48]);
        break;
    case 0: break;
    case 1:
        if (command == 4) {
            switch (result) {
            case 0: return 1;
            case 1: return 2;
            case 2: return 3;
            case 3: MemCardAccept(D_8013AF34);
            case 5: D_8013B608[D_8013AF38] = 1; break;
            }
        }
        break;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/io/memory_card_tasks", func_8001983C);

void resetMemoryCardTaskState(void) {
    D_8013AF3E = 0;
}

INCLUDE_ASM("asm/nonmatchings/io/memory_card_tasks", func_800198C4);

INCLUDE_ASM("asm/nonmatchings/io/memory_card_tasks", func_800198EC);

void beginMemoryCardBufferRead(void) {
    if (tryAcceptMemoryCard() == 1) {
        selectMemoryCardLoadFile(0, gMemoryCardTaskBuffer);
        setCurrentTaskCallback(pollMemoryCardBufferReadTask);
    }
}

void pollMemoryCardBufferReadTask(void) {
    switch (pollMemoryCardAvailability()) {
    case 0: D_8013AF3E = 1; return;
    case 5: D_8013AF3E = 2; return;
    case 2: D_8013AF3E = 6; break;
    case 1: D_8013AF3E = 5; break;
    case 3: D_8013AF3E = 7; break;
    case 4: D_8013AF3E = 8; break;
    case 6: D_8013AF3E = 9; break;
    case 7: D_8013AF3E = 10; break;
    case 8: D_8013AF3E = 11; break;
    }
    deleteCurrentTaskSubtree();
}

s32 startMemoryCardReadTask(void) {
    if (D_8013AF3E == 5) {
        createChildTaskWithReserve(pollMemoryCardReadTask);
        D_8013AF3E = 3;
        return 1;
    }
    return 0;
}

void pollMemoryCardReadTask(void) {
    s32 result = pollMemoryCardRead();
    if (result != 0) {
        switch (result) {
        case 1: D_8013AF3E = 0; break;
        case 2: D_8013AF3E = 7; break;
        case 3: D_8013AF3E = 8; break;
        case 4: D_8013AF3E = 11; break;
        case 5: D_8013AF3E = 12; break;
        }
        deleteCurrentTaskSubtree();
    }
}

s32 startMemoryCardSaveStatusTask(void *buffer, u32 size) {
    if (D_8013AF3E == 0) {
        createChildTaskWithReserve(waitForMemoryCardAcceptTask);
        D_8013AF3E = 1;
        gMemoryCardTaskBuffer = buffer;
        gMemoryCardSaveBufferSize = size;
        return 1;
    }
    return 0;
}

void waitForMemoryCardAcceptTask(void) {
    if (tryAcceptMemoryCard() == 1) {
        captureMemoryCardSaveIcon();
        setCurrentTaskCallback(pollMemoryCardStatusTask);
    }
}

void pollMemoryCardStatusTask(void) {
    switch (pollMemoryCardAvailability()) {
    case 0: D_8013AF3E = 1; return;
    case 5: D_8013AF3E = 2; return;
    case 2: D_8013AF3E = 6; break;
    case 8: D_8013AF3E = 11; break;
    case 1: D_8013AF3E = 5; break;
    case 3: D_8013AF3E = 7; break;
    case 4: D_8013AF3E = 8; break;
    case 6: D_8013AF3E = 9; break;
    case 7: D_8013AF3E = 10; break;
    }
    deleteCurrentTaskSubtree();
}

s32 startMemoryCardWriteTask(void) {
    if (D_8013AF3E == 5) {
        createChildTaskWithReserve(prepareMemoryCardWriteTask);
        D_8013AF3E = 4;
        return 1;
    }
    return 0;
}

void prepareMemoryCardWriteTask(void) {
    prepareMemoryCardSave(0, gMemoryCardTaskBuffer, gMemoryCardSaveBufferSize);
    setCurrentTaskCallback(pollMemoryCardWriteTask);
}

void pollMemoryCardWriteTask(void) {
    s32 result = pollMemoryCardWrite();
    if (result != 0) {
        switch (result) {
        case 1: D_8013AF3E = 0; break;
        case 2: D_8013AF3E = 7; break;
        case 3: D_8013AF3E = 8; break;
        case 4: D_8013AF3E = 9; break;
        case 5: D_8013AF3E = 10; break;
        }
        deleteCurrentTaskSubtree();
    }
}
