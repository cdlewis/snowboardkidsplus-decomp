#pragma once

#include "game/engine/task.h"
#include "psx/libgs.h"

typedef struct CourseOffsetResource {
    GsCOORDINATE2 coordinate;
    VECTOR position;
    SVECTOR rotation;
    GameTask *linkedTask;
} CourseOffsetResource;
