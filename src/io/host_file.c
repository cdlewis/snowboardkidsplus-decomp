#include "text_encoding.h"
#include "game/io/host_file.h"
#include "game/graphics/graphics.h"
#include "psx/libc.h"
#include "psx/libsn.h"

const char gPcHostOpenErrorFormat[28]
    __attribute__((section(".rodata.host_file_errors"), aligned(4))) = _("Error opening file %s＼n");
const char gPcHostTransferErrorFormat[36]
    __attribute__((section(".rodata.host_file_errors"), aligned(4))) = _("Failed to read all of file %s＼n");

static const u32 sPcHostErrorPadding
    __attribute__((section(".rodata.host_file_errors"), aligned(4))) = 0;
extern char gPcHostAssetPathFormat[];

int readHostFile(char *path, char *buffer) {
    int length;
    int fd;

    fd = PCopen(path, 0, 0);
    if (fd == -1) {
        printf(gPcHostOpenErrorFormat, path);
    } else {
        length = PClseek(fd, 0, 2);
        PClseek(fd, 0, 0);
        if (PCread(fd, buffer, length) == length) {
            PCclose(fd);
            return 0;
        }
        printf(gPcHostTransferErrorFormat, path);
    }
    return -1;
}

int writePcHostFile(char *name, char *buffer, int length) {
    int fd;
    fd = PCopen(name, 1, 0);
    if (fd == -1) {
        fd = PCcreat(name, 0);
        if (fd == -1) {
            printf(gPcHostOpenErrorFormat, name);
            return -1;
        }
    }
    if (PCwrite(fd, buffer, length) != length) {
        printf(gPcHostTransferErrorFormat, name);
        return -1;
    }
    PCclose(fd);
    return 0;
}

s16 readPcHostAsset(s16 mode, char *name, void *buffer, s16 *position) {
    char path[256];
    s16 result;

    if (name == NULL) return -1;
    sprintf(path, gPcHostAssetPathFormat, name);
    switch (mode) {
    case 0:
        result = readHostFile(path, buffer);
        break;
    case 1:
        result = readHostFile(path, buffer);
        uploadTimImageWithOffset(buffer, position);
        break;
    case 2:
        result = readHostFile(path, buffer);
        uploadTimArchive(buffer, position);
        break;
    default:
        result = -1;
        break;
    }
    return result;
}
