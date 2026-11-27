#include "card_movie_internal.h"
#include "game/engine/system_runtime.h"
#include "game/engine/task.h"
#include "game/io/memory_card.h"
#include "psx/libmcrd.h"

s16 gMemoryCardTaskState __attribute__((section(".sdata.memory_card_task_state"), aligned(2))) = 0;

void *gMemoryCardTaskBuffer = NULL;
u32 gMemoryCardSaveBufferSize = 0;

s32 pollMemoryCardWrite(void) {
    s32 command, result;
    gMemoryCardChanged[gMemoryCardPortIndex] = 0;
    if (gMemoryCardCreateFilePending[gMemoryCardPortIndex] == 1) {
        switch (MemCardCreateFile(gMemoryCardChannel, gMemoryCardSelectedFileName, (s8)gMemoryCardSaveHeader.blockCount)) {
        case 0: readMemoryCardDirectory();
            gMemoryCardCreateFilePending[gMemoryCardPortIndex] = 0; break;
        case 1: return 2;
        case 2: return 3;
        case 4: return 4;
        case 6: gMemoryCardCreateFilePending[gMemoryCardPortIndex] = 0; break;
        case 7: return 5;
        default: return 0;
        }
    }
    switch (MemCardSync(1, &command, &result)) {
    case -1:
        MemCardWriteFile(gMemoryCardChannel, gMemoryCardSelectedFileName, &gMemoryCardSaveHeader, 0, gMemoryCardSaveCapacities[gMemoryCardSelectedFileType]);
        break;
    case 0: break;
    case 1:
        if (command == 4) {
            switch (result) {
            case 0: return 1;
            case 1: return 2;
            case 2: return 3;
            case 3: MemCardAccept(gMemoryCardChannel);
            case 5: gMemoryCardCreateFilePending[gMemoryCardPortIndex] = 1; break;
            }
        }
        break;
    }
    return 0;
}

s32 formatMemoryCard(void) {
    s32 *fileCount;

    switch (MemCardFormat(gMemoryCardChannel)) {
    case 1:
        return 1;
    default:
        fileCount = &gMemoryCardDirectoryEntryCounts[gMemoryCardPortIndex];
        *fileCount = 0;
        /* The original returns the address of the cleared count on this path. */
        return (s32)fileCount;
    case 0:
        return 0;
    case 2:
        return 2;
    }
}

void resetMemoryCardTaskState(void) {
    gMemoryCardTaskState = 0;
}

void setMemoryCardChannel(s32 channel) {
    gMemoryCardChannel = channel;
    switch (channel) {
    case 0x10:
        gMemoryCardPortIndex = 1;
        break;
    default:
        gMemoryCardPortIndex = 0;
        break;
    }
}

s32 startMemoryCardReadStatusTask(void *buffer) {
    if (gMemoryCardTaskState == 0) {
        createChildTaskWithReserve(beginMemoryCardBufferRead);
        gMemoryCardTaskState = 1;
        gMemoryCardTaskBuffer = buffer;
        return 1;
    }
    return 0;
}

void beginMemoryCardBufferRead(void) {
    if (tryAcceptMemoryCard() == 1) {
        selectMemoryCardLoadFile(0, gMemoryCardTaskBuffer);
        setCurrentTaskCallback(pollMemoryCardBufferReadTask);
    }
}

void pollMemoryCardBufferReadTask(void) {
    switch (pollMemoryCardAvailability()) {
    case 0: gMemoryCardTaskState = 1; return;
    case 5: gMemoryCardTaskState = 2; return;
    case 2: gMemoryCardTaskState = 6; break;
    case 1: gMemoryCardTaskState = 5; break;
    case 3: gMemoryCardTaskState = 7; break;
    case 4: gMemoryCardTaskState = 8; break;
    case 6: gMemoryCardTaskState = 9; break;
    case 7: gMemoryCardTaskState = 10; break;
    case 8: gMemoryCardTaskState = 11; break;
    }
    deleteCurrentTaskSubtree();
}

s32 startMemoryCardReadTask(void) {
    if (gMemoryCardTaskState == 5) {
        createChildTaskWithReserve(pollMemoryCardReadTask);
        gMemoryCardTaskState = 3;
        return 1;
    }
    return 0;
}

void pollMemoryCardReadTask(void) {
    s32 result = pollMemoryCardRead();
    if (result != 0) {
        switch (result) {
        case 1: gMemoryCardTaskState = 0; break;
        case 2: gMemoryCardTaskState = 7; break;
        case 3: gMemoryCardTaskState = 8; break;
        case 4: gMemoryCardTaskState = 11; break;
        case 5: gMemoryCardTaskState = 12; break;
        }
        deleteCurrentTaskSubtree();
    }
}

s32 startMemoryCardSaveStatusTask(void *buffer, u32 size) {
    if (gMemoryCardTaskState == 0) {
        createChildTaskWithReserve(waitForMemoryCardAcceptTask);
        gMemoryCardTaskState = 1;
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
    case 0: gMemoryCardTaskState = 1; return;
    case 5: gMemoryCardTaskState = 2; return;
    case 2: gMemoryCardTaskState = 6; break;
    case 8: gMemoryCardTaskState = 11; break;
    case 1: gMemoryCardTaskState = 5; break;
    case 3: gMemoryCardTaskState = 7; break;
    case 4: gMemoryCardTaskState = 8; break;
    case 6: gMemoryCardTaskState = 9; break;
    case 7: gMemoryCardTaskState = 10; break;
    }
    deleteCurrentTaskSubtree();
}

s32 startMemoryCardWriteTask(void) {
    if (gMemoryCardTaskState == 5) {
        createChildTaskWithReserve(prepareMemoryCardWriteTask);
        gMemoryCardTaskState = 4;
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
        case 1: gMemoryCardTaskState = 0; break;
        case 2: gMemoryCardTaskState = 7; break;
        case 3: gMemoryCardTaskState = 8; break;
        case 4: gMemoryCardTaskState = 9; break;
        case 5: gMemoryCardTaskState = 10; break;
        }
        deleteCurrentTaskSubtree();
    }
}
