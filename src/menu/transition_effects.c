#include "common.h"
#include "game/graphics/graphics.h"
#include "game/menu/task_widgets.h"
#include "game/race/player/race_player.h"

typedef struct TaskValueState {
    GameTask *task;
    u16 unk04;
    u16 unk06;
} TaskValueState;

typedef struct ColorCycleResource {
    u8 value, step, minimum, maximum;
} ColorCycleResource;

extern TaskValueState D_8012D4A4[3];
extern GameTask *D_8013BDA8[4];
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));

MenuRgbColor gMenuColorCycleColor __attribute__((section(".sdata.menu_color_cycle"), aligned(4))) = {0, 0, 0};

s16 getTaskTransitionViewportIndex(s32 id);
void updateTaskStateTransition(void);

void configureTaskStateTransition(s32 mode, s32 id, u16 duration) {
    TaskValueState *state;
    s16 index;
    index = getTaskTransitionViewportIndex(id);
    switch (index) {
    case 0: break;
    case 1: id -= 0xFFC; break;
    case 2: id -= 0x1FFC; break;
    }
    state = &D_8012D4A4[index];
    if (duration == 0) state->unk04 = 0xFFFF;
    else state->unk04 = 0xFFFF / duration;
    switch (mode) {
    case 1: case 2: case 4: case 5: case 6: case 7:
    case 11: case 12: case 14: case 15:
        createChildTaskForParentWithReserve(&gGameTasks[4], updateTaskStateTransition);
        if (mode >= 10) gCreatedTask->data18 = mode - 10;
        else gCreatedTask->data18 = mode;
        gCreatedTask->data1A = index;
        gCreatedTask->data1C = id;
        gCreatedTask->data1E = 1;
        state->task = gCreatedTask;
        if (mode < 11) {
            if (duration == 0) {
                if (mode == 1 || mode == 4 || mode == 6) state->unk06 = 0xFFFF;
                else state->unk06 = 0;
            } else {
                if (mode == 1 || mode == 4 || mode == 6) state->unk06 = 0xFFFF;
                else state->unk06 = 0;
            }
        }
        break;
    case 0: case 3: case 8: case 9: case 10: case 13:
    default: state->task = NULL; break;
    }
}

void updateTaskStateTransition(void) {
    u16 index = gCurrentTask->data1A;
    TaskValueState *state = &D_8012D4A4[index];
    GsOT *ot;
    s16 draw;
    GameTask *task;

    if (state->task != gCurrentTask) {
        goto end_task;
    }
    /* configureTaskStateTransition assigns one of the three viewport indices. */
    switch (index) {
    case 0:
        ot = &D_800EBFF0[gActiveGraphicsBuffer];
        draw = 1;
        break;
    case 1:
        ot = &D_800EC018[gActiveGraphicsBuffer];
        draw = shouldRenderPlayerView(0);
        break;
    case 2:
        ot = &D_800EC0B8[gActiveGraphicsBuffer];
        draw = shouldRenderPlayerView(1);
        break;
    }

    if (draw) {
        u16 mode;
        GameTask *drawTask = gCurrentTask;
        mode = drawTask->data18;
        if (mode == 1 || mode == 2 || mode == 6 || mode == 7) {
            drawScreenDarkeningOverlay(state->unk06 >> 8, ot, gCurrentTask->data1C);
        } else if (mode == 4 || mode == 5) {
            drawScreenBrighteningOverlay(state->unk06 >> 8, ot, drawTask->data1C);
        }
    }

    if (gCurrentTask->data1E &&
        (!gRacePauseState || gCurrentTask->data18 == 6 || gCurrentTask->data18 == 7)) {
        if ((gCurrentTask->data18 == 1 || gCurrentTask->data18 == 6) && state->unk06 == 0) {
            goto end_task;
        }
        task = gCurrentTask;
        if ((task->data18 == 2 || task->data18 == 7) && state->unk06 == 0xFFFF) {
            goto stop_transition;
        }
        if (gCurrentTask->data18 == 4 && state->unk06 == 0) {
end_task:
            deleteCurrentTaskSubtree();
            return;
        }
        task = gCurrentTask;
        if (task->data18 == 5 && state->unk06 == 0xFFFF) {
stop_transition:
            task->data1E = 0;
            return;
        }
        if (gCurrentTask->data18 == 1 || gCurrentTask->data18 == 4 || gCurrentTask->data18 == 6) {
            if (state->unk06 < state->unk04) {
                state->unk06 = 0;
            } else {
                state->unk06 -= state->unk04;
            }
        }
        if (gCurrentTask->data18 == 2 || gCurrentTask->data18 == 5 || gCurrentTask->data18 == 7) {
            if (0xFFFF - state->unk06 < state->unk04) {
                state->unk06 = 0xFFFF;
            } else {
                state->unk06 += state->unk04;
            }
        }
    }
}

s16 getTaskTransitionViewportIndex(s32 id) {
    if (id >= 0x1000 && id < 0x2000) {
        return 1;
    }
    return 2 * (id >= 0x2000 && id < 0x3000);
}

s32 hasTaskStateValue6(s32 id) {
    return D_8012D4A4[getTaskTransitionViewportIndex(id)].unk06 != 0;
}

s32 isTaskStateTransitionBelowMaximum(s32 id) {
    return D_8012D4A4[getTaskTransitionViewportIndex(id)].unk06 < 0xFFFFu;
}

void clearColorCycleTaskReferences(void) {
    s16 i;
    for (i = 0; i < 4; ) {
        D_8013BDA8[i] = NULL;
        i = (s16)(i + 1);
    }
}

void configureColorCycleTask(s32 mode, s16 minimum, s16 maximum, s32 step, s16 index) {
    GameTask *task;
    ColorCycleResource *resource;
    s32 missing;
    u8 low = minimum;
    u8 high = maximum;
    u8 rate = step;
    if (index < 4) {
        task = D_8013BDA8[index];
        missing = task == NULL;
        if (mode != 0 && (mode < 0 ? 0 : (mode < 5))) {
            if (missing == 1) {
                createChildTaskForParentWithReserve(&gGameTasks[3], updateColorCycleTask);
                task = D_8013BDA8[index] = gCreatedTask;
                task->data1A = index;
                gCreatedTask->resource14 = allocateTaskMemoryBlock();
                resource = (ColorCycleResource *)&gTaskMemoryPool[gCreatedTask->resource14];
                if (mode == 1 || mode == 3) resource->value = minimum;
                else resource->value = maximum;
            } else {
                resource = (ColorCycleResource *)&gTaskMemoryPool[task->resource14];
                if (mode == 1) resource->value = minimum;
                if (mode == 2) resource->value = maximum;
            }
            task->data18 = mode;
            if (mode == 3) task->data18 = 1;
            if (mode == 4) task->data18 = 2;
            task->data1E = 0;
            resource->step = rate;
            resource->minimum = low;
            resource->maximum = high;
            gMenuColorCycleColor.r = resource->value;
            gMenuColorCycleColor.g = resource->value;
            gMenuColorCycleColor.b = resource->value;
        } else if (task != NULL) task->data18 = 0;
    }
}

void updateColorCycleTask(void) {
    ColorCycleResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (ColorCycleResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data18 == 0) {
        D_8013BDA8[gCurrentTask->data1A] = NULL;
        deleteCurrentTaskSubtree();
    } else {
        gCurrentTask->data1E = 0;
        switch (gCurrentTask->data18) {
        case 1:
            if (resource->step >= resource->maximum - resource->value) {
                resource->value = resource->maximum;
                gCurrentTask->data1E = 1;
                gCurrentTask->data18 = 2;
            } else { resource->value += resource->step; }
            break;
        case 2:
            if (resource->step >= resource->value - resource->minimum) {
                resource->value = resource->minimum;
                gCurrentTask->data1E = 1;
                gCurrentTask->data18 = 1;
            } else { resource->value -= resource->step; }
            break;
        }
        gMenuColorCycleColor.r = resource->value;
        gMenuColorCycleColor.g = resource->value;
        gMenuColorCycleColor.b = resource->value;
    }
}

s32 copyColorCycleTaskColor(s16 index, MenuRgbColor *color) {
    GameTask *task = D_8013BDA8[index];
    if (task == NULL) {
        return 0;
    } else {
        const ColorCycleResource *resource = (const ColorCycleResource *)&gTaskMemoryPool[task->resource14];
        color->r = resource->value;
        color->g = resource->value;
        color->b = resource->value;
        return 1;
    }
}

s32 isTaskResourceTransitionComplete(s16 index) {
    GameTask *task;
    task = *(index + D_8013BDA8);
    if (task != NULL && task->data1E == 1) {
        return 1;
    }
    return 0;
}

s32 hasColorCycleTaskReferences(void) {
    s32 found = 0;
    s16 i = 0;

    do {
        if (D_8013BDA8[i] != NULL) {
            found = 1;
        }
    } while (++i < 4);
    return found;
}

void drawScreenDarkeningOverlay(s32 intensity, GsOT *ot, u16 depth) {
    TILE *packet = (TILE *)GsGetWorkBase();
    u16 tpage;
    SetTile(packet);
    setXY0(packet, -170, -130);
    setWH(packet, 340, 260);
    setRGB0(packet, intensity, intensity, intensity);
    SetSemiTrans(packet, 1);
    AddPrim(ot->org + depth, packet);
    tpage = GetTPage(0, 2, 0, 0);
    SetDrawMode((DR_MODE *)(packet + 1), 1, 1, tpage, NULL);
    AddPrim(ot->org + depth, ++packet);
    func_800BF2C8((PACKET *)((DR_MODE *)packet + 1));
}

void drawScreenBrighteningOverlay(s32 intensity, GsOT *ot, u16 depth) {
    TILE *packet = (TILE *)GsGetWorkBase();
    u16 tpage;
    SetTile(packet);
    setXY0(packet, -170, -130);
    setWH(packet, 340, 260);
    setRGB0(packet, intensity, intensity, intensity);
    SetSemiTrans(packet, 1);
    AddPrim(ot->org + depth, packet);
    tpage = GetTPage(0, 1, 0, 0);
    SetDrawMode((DR_MODE *)(packet + 1), 1, 1, tpage, NULL);
    AddPrim(ot->org + depth, ++packet);
    func_800BF2C8((PACKET *)((DR_MODE *)packet + 1));
}
