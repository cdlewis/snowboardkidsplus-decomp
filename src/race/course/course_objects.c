#include "../../graphics/render_effects_internal.h"

struct VisualPlacementHeightData gVisualPlacementHeightData __attribute__((section(".data.visual_placement_heights"))) = {
    0,
    {0, 56, 88, 96, -16, -40, -64, -88,
     -112, -136, -160, -184, -208, -232, -256, -280},
    0
};

CourseSectorRange gCourseResourceSectors[10][3] __attribute__((section(".data.course_sector_resources"))) = {
    {{0x0C9D, 0x000F}, {0x0CAC, 0x003E}, {0x0CEA, 0x005A}},
    {{0x0D48, 0x0010}, {0x0D58, 0x004E}, {0x0DA6, 0x0060}},
    {{0x0E09, 0x000B}, {0x0E14, 0x0037}, {0x0E4B, 0x0060}},
    {{0x0EAC, 0x0011}, {0x0EBD, 0x0055}, {0x0F12, 0x0060}},
    {{0x0F73, 0x000C}, {0x0F7F, 0x0039}, {0x0FB8, 0x0060}},
    {{0x101C, 0x0011}, {0x102D, 0x0055}, {0x1082, 0x0060}},
    {{0x10E3, 0x000C}, {0x10EF, 0x003F}, {0x112E, 0x0062}},
    {{0x11A4, 0x0005}, {0x11A9, 0x0018}, {0x11C1, 0x003C}},
    {{0x11FE, 0x0004}, {0x1202, 0x0017}, {0x1219, 0x0046}},
    {{0x1265, 0x0005}, {0x126A, 0x0019}, {0x1283, 0x005E}}
};

CourseSectorRange gCourseMetadataSectors[10] __attribute__((section(".data.course_sector_resources"))) = {
    {0x0D44, 0x0001},
    {0x0E06, 0x0001},
    {0x0EAB, 0x0001},
    {0x0F72, 0x0001},
    {0x1018, 0x0001},
    {0x10E2, 0x0001},
    {0x1190, 0x0001},
    {0x11FD, 0x0001},
    {0x1264, 0x0001},
    {0x12E4, 0x0001}
};

CourseSectorRange gCourseOptionalResourceSectors[10] __attribute__((section(".data.course_sector_resources"))) = {
    {0x0D45, 0x0003},
    {0x0E07, 0x0002},
    {0x0000, 0x0000},
    {0x0000, 0x0000},
    {0x1019, 0x0003},
    {0x0000, 0x0000},
    {0x1191, 0x0003},
    {0x0000, 0x0000},
    {0x125F, 0x0005},
    {0x12E1, 0x0003}
};

u8 gCourseMetadataBuffer[0x800] __attribute__((section(".data.course_sector_resources"), aligned(4))) = {0};

EmitterPlacement gProximityEmitterPlacements[2]
    __attribute__((section(".data.proximity_emitter_placements"))) = {
    {0x2D8, 0x66D0, 0xA4E8, 0x2C, 0},
    {0x65E0, 0xAB40, 0xEFA0, 0x52, 0},
};

MovingVisualPlacement gMovingVisualPlacements[9]
    __attribute__((section(".data.moving_visual_placements"))) = {
    {24, 0, -2487616, 5385268, -2052416, 5742644},
    {26, 0, -2352960, 6135860, -2678592, 5676084},
    {24, 0, -3052352, 6169652, -2539328, 6403636},
    {29, 0, -2818880, 6921780, -3435840, 6783540},
    {29, 0, -3475776, 7003188, -3229504, 7422516},
    {29, 0, -3573496, 6820636, -3384720, 6964300},
    {27, 0, -2870196, 6550352, -3431836, 6553112},
    {26, 0, -3162140, 6098112, -2780108, 5860364},
    {24, 0, -2426136, 5658860, -2373948, 5250364},
};

OrbitingTaskPlacement gOrbitingTaskPlacements[2]
    __attribute__((section(".data.orbiting_task_placements"))) = {
    {99, 640, 23253, 42781},
    {99, 832, 22656, 45410},
};

u32 gObjectSortSavedStackPointer
    __attribute__((section(".sdata.object_sort_saved_stack"))) = 0;

u32 gAlternateObjectSortSavedStackPointer
    __attribute__((section(".sdata.alternate_object_sort_saved_stack"))) = 0;

TaskVisualFrame gPlacedObjectVisualFrame
    __attribute__((section(".data.placed_object_visual_frame"))) = {
    192, 0, 64, 64, 0xA, 0x3F28,
};

TaskVisualFrame *gCourseVisualFrames
    __attribute__((section(".sdata.course_visual_frames"))) = 0;

CourseTriggerRecord gCourse0Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {49, 7, 0},
    {82, 7, 0},
    {102, 3, 0},
    {139, 1, 0},
    {142, 7, 0},
    {153, 7, 0},
    {166, 7, 0},
    {-1, 0, 0},
};

CourseTriggerRecord gCourse1Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {39, 7, 0},
    {52, 7, 0},
    {103, 7, 0},
    {-1, 0, 0},
};

CourseTriggerRecord gCourse2Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {6, 7, 0},
    {33, 1, 0},
    {44, 1, 0},
    {66, 1, 0},
    {79, 1, 0},
    {106, 1, 0},
    {121, 1, 0},
    {136, 1, 0},
    {140, 1, 0},
    {-1, 0, 0},
};

CourseTriggerRecord gCourse3Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {16, 7, 0},
    {19, 7, 0},
    {34, 3, 0},
    {72, 1, 0},
    {129, 3, 0},
    {140, 1, 0},
    {157, 1, 0},
    {183, 3, 0},
    {272, 0, 0},
    {257, 0, 0},
    {246, 1, 0},
    {234, 1, 0},
    {226, 0, 0},
    {-1, 7, 0},
};

CourseTriggerRecord gCourse4Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {34, 1, 0},
    {70, 1, 0},
    {86, 2, 0},
    {105, 2, 0},
    {137, 3, 0},
    {150, 0, 0},
    {156, 3, 0},
    {186, 3, 0},
    {-1, 7, 0},
};

CourseTriggerRecord gCourse5Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {30, 3, 0},
    {46, 3, 0},
    {119, 0, 0},
    {145, 0, 0},
    {180, 3, 0},
    {183, 3, 0},
    {261, 3, 0},
    {294, 3, 0},
    {303, 3, 0},
    {-1, 7, 0},
};

CourseTriggerRecord gCourse6Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {26, 3, 0},
    {41, 0, 0},
    {76, 3, 0},
    {97, 7, 0},
    {132, 3, 0},
    {175, 3, 0},
    {224, 3, 0},
    {-1, 7, 0},
};

CourseTriggerRecord gCourse8Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {21, 7, 0},
    {31, 7, 0},
    {50, 0, 0},
    {-1, 7, 0},
};

CourseTriggerRecord gCourse9Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {23, 7, 0},
    {56, 7, 0},
    {-1, 7, 0},
};

CourseTriggerRecord *gCourseTriggers[]
    __attribute__((section(".data.course_triggers"))) = {
    gCourse0Triggers, gCourse1Triggers, gCourse2Triggers, gCourse3Triggers,
    gCourse4Triggers, gCourse5Triggers, gCourse6Triggers, gEmptyCourseTriggers,
    gCourse8Triggers, gCourse9Triggers,
};

void queueCourseResourceSectorReads(void) {
    queueCdSectorCommand(0, gCourseResourceSectors[D_8013AFCE][0].sector, gCourseResourceSectors[D_8013AFCE][0].count, (void *)0x801C0000, 0);
    queueCdSectorCommand(0, gCourseResourceSectors[D_8013AFCE][1].sector, gCourseResourceSectors[D_8013AFCE][1].count, (void *)0x801C8800, 0);
    queueCdSectorCommand(2, gCourseResourceSectors[D_8013AFCE][2].sector, gCourseResourceSectors[D_8013AFCE][2].count, (void *)0x80180000, 0);
    queueCdSectorCommand(0, gCourseMetadataSectors[D_8013AFCE].sector, gCourseMetadataSectors[D_8013AFCE].count, gCourseMetadataBuffer, 0);
    if ((u16)gCourseOptionalResourceSectors[D_8013AFCE].sector != 0) {
        queueCdSectorCommand(0, gCourseOptionalResourceSectors[D_8013AFCE].sector, gCourseOptionalResourceSectors[D_8013AFCE].count, (void *)0x801F0800, 0);
    }
    if (D_8013AFCE == 6) {
        queueCdSectorCommand(16, 0x1194, 16, (void *)0x80180000, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_80058BD4);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_80059254);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_80059290);

void drawTaskObject(u16 index) {
    GsDOBJ2 *object;
    GsDOBJ2 *objects;
    gObjectSortSavedStackPointer = SetSp(0x1F8003FC);
    objects = D_80129A80;
    object = &objects[index];
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    if (func_80059254()) {
        GsSortObject4(object, &D_800EC0B8[D_8013AF78], 2, (u32 *)0x1F800000);
    } else {
        GsSortObject4(object, &D_800EC018[D_8013AF78], 2, (u32 *)0x1F800000);
    }
    SetSp(gObjectSortSavedStackPointer);
}

void drawTaskObjectAlternateLayer(u16 index) {
    GsDOBJ2 *object;
    GsDOBJ2 *objects;
    gAlternateObjectSortSavedStackPointer = SetSp(0x1F8003FC);
    objects = D_80129A80;
    object = &objects[index];
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    if (func_80059254()) {
        GsSortObject4(object, &D_800EC130[D_8013AF78], 2, (u32 *)0x1F800000);
    } else {
        GsSortObject4(object, &D_800EC090[D_8013AF78], 2, (u32 *)0x1F800000);
    }
    SetSp(gAlternateObjectSortSavedStackPointer);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_80059484);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_80059958);

void initializeSelectedCourseOffsetTask(void) {
    CourseOffsetResource *resource;
    SelectedCourseRecord *records;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    records = D_80123D80;
    resource->rotation.vx = 0;
    resource->rotation.vy = records[D_8013AFCE].unk16;
    resource->rotation.vz = 0;
    D_8013AF94->vx = 0;
    D_8013AF94->vy = 0;
    D_8013AF94->vz = -0x330;
    RotMatrix(&resource->rotation, D_8013AF98);
    ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
    resource->position.vx = records[D_8013AFCE].unk18[3] + (D_8013AF88->vx << 8);
    resource->position.vy = records[D_8013AFCE].unk18[4];
    resource->position.vz = records[D_8013AFCE].unk18[5] + (D_8013AF88->vz << 8);
    if (D_8013AFCE != 8) {
        D_8013AE88->data1C = 4;
    } else {
        D_8013AE88->data1C = 0x19;
    }
    D_8013AE88->data1E = 0x16;
    setCurrentTaskCallback(updateSelectedCourseOffsetTask);
}

void updateSelectedCourseOffsetTask(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        RotMatrix(&resource->rotation, D_8013AF98);
        D_8013AF90->vx = 0;
        D_8013AF90->vy = 0;
        D_8013AF90->vz = 0x1000;
        ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
        resource->position.vx += D_8013AF88->vx;
        resource->position.vz += D_8013AF88->vz;
        resource->coordinate.coord.t[0] = resource->position.vx >> 8;
        resource->coordinate.coord.t[1] = resource->position.vy >> 8;
        resource->coordinate.coord.t[2] = resource->position.vz >> 8;
        if (D_8013AE88->data1E != 0) {
            D_8013AE88->data1E--;
        }
    }
    if (D_8013AE88->data1E == 0) {
        if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
            D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
            func_8001E3E4(&resource->rotation, &resource->coordinate);
            drawTaskObject(D_8013AE88->data1C);
        }
    }
    if (resource->linkedTask->data1A == 6) {
        setCurrentTaskCallback(updatePlayerFollowingOrbitTask);
    }
}

void updatePlayerFollowingOrbitTask(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (D_80120E90[D_8013AE88->data18].unk12C != 7) {
            D_8013AF94->vx = -100;
            D_8013AF94->vy = 0;
            D_8013AF94->vz = 0;
            RotMatrix(&resource->rotation, D_8013AF98);
            ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
            resource->position.vx = resource->coordinate.coord.t[0] + D_8013AF88->vx;
            resource->position.vz = resource->coordinate.coord.t[2] + D_8013AF88->vz;
            D_8013AE88->data1A = 0x40;
            setCurrentTaskCallback(updateOrbitingTaskObject);
            updateOrbitingTaskObject();
            return;
        }
        resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[0];
        resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[1];
        resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[2];
        if (resource->linkedTask->data1A == 7) {
            resource->rotation.vy = D_80120E90[D_8013AE88->data18].unk110;
        }
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
}

void updateOrbitingTaskObject(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->rotation.vy -= 0x20;
        D_8013AF94->vx = 100;
        D_8013AF94->vy = 0;
        D_8013AF94->vz = 0;
        RotMatrix(&resource->rotation, D_8013AF98);
        ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
        resource->coordinate.coord.t[0] = resource->position.vx + D_8013AF88->vx;
        resource->coordinate.coord.t[2] = resource->position.vz + D_8013AF88->vz;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
    if (!D_8013AFD3) {
        if (--D_8013AE88->data1A == 0) {
            D_8013AE88->data1A = 0x40;
            setCurrentTaskCallback(updateMovingTaskObject);
        }
    }
}

void updateMovingTaskObject(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        RotMatrix(&resource->rotation, D_8013AF98);
        D_8013AF90->vx = 0;
        D_8013AF90->vy = 0;
        D_8013AF90->vz = 8;
        ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
        resource->coordinate.coord.t[0] += D_8013AF88->vx;
        resource->coordinate.coord.t[2] += D_8013AF88->vz;
        D_8013AE88->data1A--;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
    if (D_8013AE88->data1A == 0) {
        func_800149A8();
    }
}

void computeSelectedTriangleAngles(s16 *pitch, s16 *yaw) {
    VECTOR normal;
    s32 x, y, z;
    computeSelectedTriangleNormal(&normal);
    *yaw = func_8004A124(0, 0, normal.vx, normal.vy);
    x = normal.vx;
    y = normal.vy;
    z = normal.vz;
    x /= 16;
    y /= 16;
    z /= 16;
    *pitch = -func_8004A124(0, 0, z, SquareRoot0(x*x + y*y));
}

void updateCameraRotationMatrices(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = cameras[playerIndex].yaw;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, &D_8012A6B0);
    D_8013AF90->vx = cameras[func_80059254()].pitch;
    D_8013AF90->vy = cameras[func_80059254()].yaw;
    D_8013AF90->vz = 0;
    RotMatrixYXZ(D_8013AF90, &D_8012A6D0);
}


void prepareTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frame, u8 usePitch) {
    if (usePitch == 0) {
        *D_8013AF9C = D_8012A6B0;
    } else {
        *D_8013AF9C = D_8012A6D0;
    }
    resource->frame = frame;
    ScaleMatrix(D_8013AF9C, (VECTOR *)&resource->scaleX);
    D_8012967C.r0 = resource->r0;
    D_8012967C.g0 = resource->g0;
    D_8012967C.b0 = resource->b0;
    D_8012967C.tpage = resource->frame->tpage;
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005A788);

void setTaskVisualFrame(TaskVisualResource *resource, TaskVisualFrame *frames, s32 scale) {
    u16 index;
    resource->scaleX = scale;
    resource->scaleY = scale;
    resource->scaleZ = scale;
    index = resource->framePosition >> 8;
    resource->vertices[0].x = (-frames[index].width - 1) / 2;
    resource->vertices[0].y = -frames[index].height;
    resource->vertices[0].z = 0;
    resource->vertices[1].x = (frames[index].width - 1) / 2;
    resource->vertices[1].y = -frames[index].height;
    resource->vertices[1].z = 0;
    resource->vertices[2].x = (-frames[index].width - 1) / 2;
    resource->vertices[2].y = -1;
    resource->vertices[2].z = 0;
    resource->vertices[3].x = (frames[index].width - 1) / 2;
    resource->vertices[3].y = -1;
    resource->vertices[3].z = 0;
    resource->frame = &frames[index];
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005AC4C);

void initializePickupVisualTask(void) {
    TaskVisualResource *resource;
    u16 i;
    if ((s16)getModeFilteredGameOptionFlag2() == 0) {
        func_800149A8();
    } else {
        for (i = D_8013B2B5; i < D_8013B2B5 + D_8013B2B6; i++) {
            if (D_8013B2BC[i].flags & 0x40) {
                D_8013B2BC[i].flags &= 0xBF;
            }
        }
        if (D_8013AE88->resource14 != 0xFFFF) {
            while (1) {}
        }
        D_8013AE88->resource14 = allocateTaskResource();
        resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->unkC6 = 0x600;
        resource->unkC8 = 0x80;
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->framePosition = 0;
        resource->scaleX = 0x1800;
        resource->scaleY = 0x1800;
        resource->scaleZ = 0x1800;
        setCurrentTaskCallback(func_8005B038);
    }
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005B038);

void initializeObjectVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x100;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->scaleX = 0x3800;
    resource->scaleY = 0x3800;
    resource->scaleZ = 0x3800;
    setCurrentTaskCallback(func_8005B460);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005B460);

void initializeInteractionVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x100;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->scaleX = 0x2000;
    resource->scaleY = 0x2000;
    resource->scaleZ = 0x2000;
    setCurrentTaskCallback(func_8005B6C0);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005B6C0);

void initializeCourseVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    if (D_8013AFCE == 2) {
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->scaleX = 0x6000;
        resource->scaleY = 0x6000;
        resource->scaleZ = 0x6000;
        resource->unkC6 = 0;
        resource->unkC8 = 0;
        resource->framePosition = 0;
        gCourseVisualFrames = D_8013B29C;
    } else {
        resource->unkC6 = 0x200;
        resource->unkC8 = 0x40;
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->scaleX = 0x2000;
        resource->scaleY = 0x2000;
        resource->scaleZ = 0x2000;
        resource->framePosition = 0;
        gCourseVisualFrames = D_801294AC;
    }
    setCurrentTaskCallback(updateCourseVisualTask);
}

void updateCourseVisualTask(void) {
    TaskVisualResource *resource;
    u8 i;
    s32 height;
    s16 *heightOffsets;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    prepareTaskVisualQuad(resource, gCourseVisualFrames, 0);
    for (i = D_8013B2B9; i < D_8013B2B9 + D_8013B2BA; i++) {
        if (isPointWithinCameraForwardRange(D_8013B2BC[i].x, D_8013B2BC[i].z, (s16)func_80059254())) {
            resource->coordinate.coord.t[0] = D_8013B2BC[i].x;
            resource->coordinate.coord.t[1] = D_8013B2BC[i].y;
            resource->coordinate.coord.t[2] = D_8013B2BC[i].z;
            height = D_8013B2BC[i].flags & 0xF;
            if (height != 0) {
                heightOffsets = gVisualPlacementHeightData.offsets;
                resource->coordinate.coord.t[1] -= heightOffsets[height];
            }
            func_8005A788(resource, D_80129540[D_8013B2BC[i].unk01], 2);
        }
    }
    if (D_8013AFD3 == 0) {
        if (D_8013AFCE == 9) resource->framePosition += resource->unkC8;
        if (resource->framePosition >= resource->unkC6) resource->framePosition &= 0xFF;
    }
}

void initializePlacedObjectTask(void) {
    PlacedObjectResource *resource;
    TaskVisualResource *visual;
    if ((s16)getStateAdjustedGameOptionFlag1() == 0) {
        if ((u32)(D_8013B2BC[D_8013AE88->data18].unk01 - 2) < 2) {
            func_800149A8();
            return;
        }
    }
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlacedObjectResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->groundCoordinate);
    GsInitCoordinate2(&resource->groundCoordinate, &resource->coordinate);
    D_8013AE88->data1A = D_8013B2BC[D_8013AE88->data18].unk01;
    resource->geometryIndex = D_8013B2BC[D_8013AE88->data18].geometryIndex;
    resource->rotation.vx = 0;
    resource->rotation.vy = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle;
    resource->rotation.vz = 0;
    resource->groundCoordinate.coord.t[0] = D_8013B2BC[D_8013AE88->data18].x;
    resource->groundCoordinate.coord.t[2] = D_8013B2BC[D_8013AE88->data18].z;
    resource->groundCoordinate.coord.t[1] = D_8013B2BC[D_8013AE88->data18].y =
        func_8004A764(resource->geometryIndex, resource->groundCoordinate.coord.t[0],
                      resource->groundCoordinate.coord.t[2]);
    resource->position.x = resource->groundCoordinate.coord.t[0] << 8;
    resource->position.y = resource->groundCoordinate.coord.t[1] << 8;
    resource->position.z = resource->groundCoordinate.coord.t[2] << 8;
    computeSelectedTriangleAngles(&resource->groundRotation.vx, &resource->groundRotation.vz);
    resource->groundRotation.vy = 0;
    if ((u32)(D_8013AE88->data1A - 2) < 2) {
        if (D_8013AE88->resource16 == 0xFFFF) {
            while (1) {}
        }
        visual = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource16];
        GsInitCoordinate2(0, &visual->coordinate);
        visual->coordinate.coord.t[0] = resource->groundCoordinate.coord.t[0];
        visual->coordinate.coord.t[1] = resource->groundCoordinate.coord.t[1] - 0x50;
        visual->coordinate.coord.t[2] = resource->groundCoordinate.coord.t[2];
        visual->unkC6 = 0;
        visual->unkC8 = 0;
        visual->framePosition = 0;
        visual->r0 = visual->g0 = visual->b0 = 0x80;
        visual->code = POLY_FT4_CODE;
        setTaskVisualFrame(visual, &gPlacedObjectVisualFrame, 0x1200);
    }
    setCurrentTaskCallback(func_8005BF00);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005BF00);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005C51C);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005C7AC);

void updateBobbingTrailEmitterTask(void) {
    BobbingTrailEmitterResource *resource;
    EmitterTrailResource *trail;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (BobbingTrailEmitterResource *)&D_800D9504[D_8013AE88->resource14];
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        if (!D_8013AFD3) {
            func_8002BF88(&resource->position, 0x7000, 0x18000, 2);
            resource->heading.value += resource->headingStep;
            if (resource->heading.halves.low > resource->baseHeading.halves.low + 0x140) resource->headingStep = -8;
            if (resource->heading.halves.low < resource->baseHeading.halves.low - 0x140) resource->headingStep = 8;
            resource->phase += 0x40;
            resource->rotation.vy = resource->heading.halves.low;
            resource->coordinate.coord.t[1] = 0xB654 - (rsin(resource->phase) >> 9);
            if (--resource->countdown == 0) {
                resource->countdown = 15;
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xA00, 0x18);
                if ((s16)func_800148B4(initializeVisualTrailTask) == 1) {
                    D_8013AE8C->data18 = 0xFF;
                    D_8013AE8C->resource14 = allocateTaskResource();
                    trail = (EmitterTrailResource *)&D_800D9504[D_8013AE8C->resource14];
                    GsInitCoordinate2(0, &trail->coordinate);
                    trail->geometryIndex = resource->geometryIndex;
                    trail->coordinate.coord.t[0] = resource->coordinate.coord.t[0];
                    trail->coordinate.coord.t[1] = resource->coordinate.coord.t[1] - 0x100;
                    trail->coordinate.coord.t[2] = resource->coordinate.coord.t[2];
                    trail->angle = (resource->heading.value + 0x800) & 0xFFF;
                }
            }
        }
        D_80129A80[10].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        D_8013AF88->vx = 0x3000;
        D_8013AF88->vy = 0x3000;
        D_8013AF88->vz = 0x3000;
        ScaleMatrix(&resource->coordinate.coord, D_8013AF88);
        drawTaskObject(10);
    }
}

void initializeProximityEmitterTask(void) {
    ProximityEmitterResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ProximityEmitterResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->coordinate.coord.t[0] = gProximityEmitterPlacements[D_8013AE88->data18].x;
    resource->coordinate.coord.t[1] = gProximityEmitterPlacements[D_8013AE88->data18].y;
    resource->coordinate.coord.t[2] = gProximityEmitterPlacements[D_8013AE88->data18].z;
    resource->geometryIndex = gProximityEmitterPlacements[D_8013AE88->data18].geometryIndex;
    resource->unkC0 = 0;
    resource->angle = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle + 0x400;
    resource->unkC4 = 0;
    resource->countdown = 1;
    setCurrentTaskCallback(updateProximityEmitterTask);
}

void updateProximityEmitterTask(void) {
    ProximityEmitterResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ProximityEmitterResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
            if (--resource->countdown == 0) {
                resource->countdown = 0x20;
                func_800148B4(initializeProximityParticleTask);
            }
        }
    }
}

void initializeProximityParticleTask(void) {
    EmittedParticleResource *resource;
    ProximityEmitterResource *parent;
    s32 xOffset, zOffset;
    s32 sine, cosine;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (EmittedParticleResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = (ProximityEmitterResource *)&D_800D9504[D_8013AE98->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->geometryIndex = parent->geometryIndex;
    resource->rotation = 0;
    resource->angle = parent->angle;
    resource->unkC4 = 0;
    xOffset = ((u8)((u8)randomNextSecondary() % 60) - 30) * 10;
    zOffset = ((u8)((u8)randomNextSecondary() % 60) - 30) * 10;
    resource->positionX = (parent->coordinate.coord.t[0] + xOffset) << 8;
    resource->positionY = parent->coordinate.coord.t[1] << 8;
    resource->positionZ = (parent->coordinate.coord.t[2] + zOffset) << 8;
    sine = rsin(resource->angle);
    cosine = rcos(resource->angle);
    resource->velocityX = (sine * 2560) / 4096;
    resource->velocityZ = (cosine * 2560) / 4096;
    resource->velocityY = -4096;
    resource->gravity = 80;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 90;
    setCurrentTaskCallback(func_8005CEF0);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005CEF0);

void initializeMovingVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x800;
    resource->unkC8 = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG;
    resource->framePosition = 0;
    resource->scaleX = 0x1000;
    resource->scaleY = 0x1000;
    resource->scaleZ = 0x1000;
    resource->unkC4 = gMovingVisualPlacements[D_8013AE88->data18].geometryIndex;
    resource->position.x = gMovingVisualPlacements[D_8013AE88->data18].startX;
    resource->position.z = gMovingVisualPlacements[D_8013AE88->data18].startZ;
    resource->velocityY = func_8004A124(resource->position.x, resource->position.z,
        gMovingVisualPlacements[D_8013AE88->data18].endX, gMovingVisualPlacements[D_8013AE88->data18].endZ);
    resource->unkDC = 0x10;
    resource->unkE0 = 0x80;
    resource->unkD8 = 0;
    resource->bounceCount = 0x400;
    resource->groundY = 0;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
    resource->position.y = func_8004A764(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    setCurrentTaskCallback(func_8005D394);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005D394);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005D828);

void updateActivePlayerRegionCollisions(void) {
    u16 i;
    PlayerCollisionRegion *region;
    RacePlayer *players;
    if (!D_8013AFD3) {
        i = 0;
        region = &D_80129A18[D_8013AE88->data18];
        players = D_80120E90;
        do {
            if (players[i].isActive != 0) {
                func_8005D828(&players[i], region);
            }
            i++;
        } while (i < 4);
    }
}

void initializeCollectibleTask(void) {
    PickupResource *resource;
    s32 height;
    s16 *heightOffsets;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PickupResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->geometryIndex = D_8013B2C0[D_8013AE88->data18].geometryIndex;
    resource->rotation.vx = 0;
    resource->rotation.vy = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle;
    resource->rotation.vz = 0;
    resource->coordinate.coord.t[0] = D_8013B2C0[D_8013AE88->data18].x;
    resource->coordinate.coord.t[2] = D_8013B2C0[D_8013AE88->data18].z;
    height = func_8004A764(resource->geometryIndex, resource->coordinate.coord.t[0], resource->coordinate.coord.t[2]);
    D_8013B2C0[D_8013AE88->data18].y = height;
    resource->coordinate.coord.t[1] = height;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    if (gameOptionPlayerModeAbsolute == 1) {
        resource->collisionRange = 0x2C00;
    } else {
        resource->collisionRange = 0x3400;
    }
    setCurrentTaskCallback(updateCollectibleTask);
}

void updateCollectibleTask(void) {
    PickupResource *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PickupResource *)&D_800D9504[D_8013AE88->resource14];
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[10].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(10);
        if (!D_8013AFD3) {
            func_8002BF88(&resource->position, 0x2800, 0x8000, 2);
            for (i = 0; i < D_8013B1D4; i++) {
                if (arePositionsWithinXZRange(&resource->position, &D_8012450C[i].position, resource->collisionRange)) {
                    *D_8012450C[i].active = 0;
                    D_80120E90[D_8012450C[i].playerIndex].unk95E++;
                    playSoundEffect(1, 0xF00, 0x20);
                    createNineIndexedChildTasks(0x15, 1);
                    D_8013AE88->callback = deleteCurrentTask;
                    D_8013AE88->unk10 |= 1;
                    break;
                }
            }
        }
    }
}

void initializeFallingTaskObject(void) {
    FallingTaskResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (FallingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    D_8013AE88->data1A = D_8013B2BC[D_8013AE88->data18].unk01;
    resource->geometryIndex = D_80120E90[D_8013AE88->data18].unk92E;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->position.x = D_80120E90[D_8013AE88->data18].x;
    resource->position.y = D_80120E90[D_8013AE88->data18].y - 0x2000;
    resource->position.z = D_80120E90[D_8013AE88->data18].z;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    resource->verticalVelocity = -0xA00;
    resource->groundHeight = func_8004A764(resource->geometryIndex, resource->position.x >> 8, resource->position.z >> 8) << 8;
    computeSelectedTriangleAngles(&resource->groundPitch, &resource->groundRoll);
    setCurrentTaskCallback(updateFallingTaskObject);
}

void updateFallingTaskObject(void) {
    FallingTaskResource *resource;
    u8 landed = 0;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->verticalVelocity += 0x180;
        resource->position.y += resource->verticalVelocity;
        if (resource->position.y > resource->groundHeight) {
            resource->position.y = resource->groundHeight;
            resource->rotation.vx = resource->groundPitch;
            resource->rotation.vz = resource->groundRoll;
            landed = 1;
        }
        resource->coordinate.coord.t[1] = resource->position.y >> 8;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[9].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(9);
    }
    if (landed) {
        D_8013AE88->data1A = 0x708;
        setCurrentTaskCallback(updateLandedTaskObject);
    }
}

void updateLandedTaskObject(void) {
    FallingTaskResource *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (--D_8013AE88->data1A == 0) {
            func_800149A8();
            return;
        }
        for (i = 0; i < D_8013AFD2; i++) {
            if (addPlayerFlagsIfWithinRadius(&resource->position, 0xC00, 4, i)) {
                func_800149A8();
                return;
            }
        }
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[9].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(9);
    }
}

void initializeOrbitingTaskObject(void) {
    OrbitingTaskResource *resource;
    u16 i;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (OrbitingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->groundCoordinate);
    GsInitCoordinate2(&resource->groundCoordinate, &resource->coordinate);
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->phase = (0x1000 / D_8013AE88->data1A) * D_8013AE88->data1C;
    resource->geometryIndex = gOrbitingTaskPlacements[D_8013AE88->data18].geometryIndex;
    resource->groundCoordinate.coord.t[0] = gOrbitingTaskPlacements[D_8013AE88->data18].x;
    resource->groundCoordinate.coord.t[2] = gOrbitingTaskPlacements[D_8013AE88->data18].z;
    resource->groundCoordinate.coord.t[1] = func_8004A764(resource->geometryIndex,
        resource->groundCoordinate.coord.t[0], resource->groundCoordinate.coord.t[2]);
    computeSelectedTriangleAngles(&resource->groundRotation.vx, &resource->groundRotation.vz);
    resource->groundRotation.vy = 0;
    for (i = 0; i < D_8013AFD2; i++) {
        resource->playerCooldowns[i] = 0;
    }
    setCurrentTaskCallback(func_8005E728);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005E728);

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005EBC4);

void updateAndDrawCurrentTaskObject(void) {
    func_8001E3E4(&D_8012A5C0[D_8013AE88->data18],
                  &D_80129C60[D_8013AE88->data18]);
    drawTaskObject(D_8013AE88->data18);
}

void copySlot26TransformToCurrentTaskObject(void) {
    D_80129C60[D_8013AE88->data18].coord.t[0] = D_80129C60[26].coord.t[0];
    D_80129C60[D_8013AE88->data18].coord.t[1] = D_80129C60[26].coord.t[1];
    D_80129C60[D_8013AE88->data18].coord.t[2] = D_80129C60[26].coord.t[2];
    D_8012A5C0[D_8013AE88->data18].vx = D_8012A5C0[26].vx;
    D_8012A5C0[D_8013AE88->data18].vy = D_8012A5C0[26].vy;
    D_8012A5C0[D_8013AE88->data18].vz = D_8012A5C0[26].vz;
    setCurrentTaskCallback(updateAndDrawCurrentTaskObject);
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005EFA0);

void updateCurrentPlayerCourseTrigger(void) {
    s32 i = 0;
    CourseTriggerRecord *record;
    s32 x, y, z;
    u16 angle;
    s32 sine, cosine;
    s32 mask;
    if (currentPlayerAbsolute->unk128 & 0x1000) currentPlayerAbsolute->unk948 = 0;
    currentPlayerAbsolute->unk950[0] = 0;
    record = gCourseTriggers[D_8013AFCE];
    for (;; record++, i++) {
        if (record->geometryIndex == -1) return;
        mask = 1 << i;
        if (!(currentPlayerAbsolute->unk948 & mask) && record->geometryIndex >= currentPlayerAbsolute->unk92E && record->geometryIndex - 3 < currentPlayerAbsolute->unk92E) {
            getGeometryPlacementTransform(record->geometryIndex, &x, &y, &z, &angle);
            x -= currentPlayerAbsolute->x;
            y -= currentPlayerAbsolute->y;
            z -= currentPlayerAbsolute->z;
            sine = rsin(-(s16)angle);
            cosine = rcos(-(s16)angle);
            z = (-sine * x + cosine * z) / 4096;
            if (z > 0 && z < 0x1E000) {
                currentPlayerAbsolute->unk948 |= mask;
                currentPlayerAbsolute->unk950[0] = 1;
                currentPlayerAbsolute->unk950[3] = record->value;
                return;
            }
        }
    }
}

void updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex) {
    s32 delta;
    RacePlayer *player;

    delta = D_8012BBB4[D_8013AFCE][tableIndex][sampleIndex] * 20;
    player = &D_80120E90[playerIndex];
    delta -= player->unk94C;
    if (delta > 30) {
        delta = 30;
    }
    if (delta < -30) {
        delta = -30;
    }
    player->unk94C += delta;
}

void resetSelectedRecordStates(void) {
    SelectedStateRecord *record = D_8012C6F4[D_8013AFCE];

    for (;;) {
        if (record->state == -2) {
            return;
        }
        record->state = -1;
        record++;
    }
}

void captureSelectedPlayerStates(void) {
    s8 split = 0;
    s32 i = 0;
    SelectedStateRecord *record = D_8012C6F4[D_8013AFCE];
    for (;; record++, i++) {
        GameTask *task;
        u16 inputIndex;
        if (record->state == -2) goto done;
        if (record->state == -1 && record->unk02 == D_80120E90[0].unk92E) {
            inputIndex = D_8013AEDC[0];
            record->state = inputIndex - 1;
            if (D_8013AED4[0][D_8013AEDC[0]-1].repeatCount) {
                record->state = inputIndex;
                split = 1;
            }
            D_8013B2E8[i] = D_80120E90[0];
            task = D_80120E90[0].task;
            D_8013B2E4[i].callback = task->callback;
            D_8013B2E4[i].data18 = task->data18;
            D_8013B2E4[i].data1A = task->data1A;
            D_8013B2E4[i].data1C = task->data1C;
            D_8013B2E4[i].data1E = task->data1E;
        }
    }
done:
    if (split) {
        D_8013AED4[0][D_8013AEDC[0]-1].repeatCount--;
        D_8013AED4[0][D_8013AEDC[0]].repeatCount = 0;
        D_8013AED4[0][D_8013AEDC[0]].horizontal = D_8013AED4[0][D_8013AEDC[0]-1].horizontal;
        D_8013AED4[0][D_8013AEDC[0]].vertical = D_8013AED4[0][D_8013AEDC[0]-1].vertical;
        D_8013AED4[0][D_8013AEDC[0]].buttons = D_8013AED4[0][D_8013AEDC[0]-1].buttons;
        D_8013AED4[0][D_8013AEDC[0]].pressedButtons = D_8013AED4[0][D_8013AEDC[0]-1].pressedButtons;
        D_8013AEDC[0]++;
    }
}

s32 restoreSelectedPlayerState(void) {
    s32 i = 0;
    SelectedStateRecord *record = D_8012C6F4[D_8013AFCE];
    for (;;) {
        if (record->state == -2) return 0;
        if (record->state != -1) {
            GameTask *task = D_80120E90[0].task;
            D_80120E90[0] = D_8013B2E8[i];
            D_80120E90[0].task = task;
            task->callback = D_8013B2E4[i].callback;
            task->data18 = D_8013B2E4[i].data18;
            task->data1A = D_8013B2E4[i].data1A;
            task->data1C = D_8013B2E4[i].data1C;
            task->data1E = D_8013B2E4[i].data1E;
            D_8013AEDC[0] = record->state;
            D_8013AEE0[0] = D_8013AED4[0][D_8013AEDC[0]].repeatCount;
            record->state = -1;
            D_801241E8[0].x = record->unk04;
            D_801241E8[0].y = record->unk08;
            D_801241E8[0].z = record->unk0C;
            D_8013B20C = record->unk10;
            D_801241E8[0].unk16 = 8;
            return 1;
        }
        record++;
        i++;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/course/course_objects", func_8005FAE0);
