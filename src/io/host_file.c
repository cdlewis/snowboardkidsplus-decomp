#include "../text/resource_playback_internal.h"

int readHostFile(char *path, char *buffer) {
    int length;
    int fd;

    fd = PCopen(path, 0, 0);
    if (fd == -1) {
        printf(D_80012310, path);
    } else {
        length = PClseek(fd, 0, 2);
        PClseek(fd, 0, 0);
        if (PCread(fd, buffer, length) == length) {
            PCclose(fd);
            return 0;
        }
        printf(D_8001232C, path);
    }
    return -1;
}

int writePcHostFile(char *name, char *buffer, int length) {
    int fd;
    fd = PCopen(name, 1, 0);
    if (fd == -1) {
        fd = PCcreat(name, 0);
        if (fd == -1) {
            printf(D_80012310, name);
            return -1;
        }
    }
    if (PCwrite(fd, buffer, length) != length) {
        printf(D_8001232C, name);
        return -1;
    }
    PCclose(fd);
    return 0;
}

s16 readPcHostAsset(s16 mode, char *name, void *buffer, s16 *position) {
    char path[256];
    s16 result;

    if (name == NULL) return -1;
    sprintf(path, D_8013B5C8, name);
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
