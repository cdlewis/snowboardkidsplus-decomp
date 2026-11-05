#include "common.h"
#include "game/controller.h"
#include "game/task.h"

u16 gTaskResourceFreeBits[16] __attribute__((section(".data.task_resource_bitmap"))) = {0};

u8 gControllerFlags[4] __attribute__((section(".data"))) = {0, 0, 0, 0};

s16 D_8013AF08[2] __attribute__((section(".data.controller_values"))) = {0, 0};

void func_800131B4(void) {
}

INCLUDE_ASM("asm/nonmatchings/main", func_800131BC);

INCLUDE_ASM("asm/nonmatchings/main", func_80013278);

INCLUDE_ASM("asm/nonmatchings/main", func_80013294);

INCLUDE_ASM("asm/nonmatchings/main", func_8001330C);

INCLUDE_ASM("asm/nonmatchings/main", func_800133EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800134E4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001351C);

INCLUDE_ASM("asm/nonmatchings/main", func_80013540);

INCLUDE_ASM("asm/nonmatchings/main", func_800136DC);

INCLUDE_ASM("asm/nonmatchings/main", func_80013798);

INCLUDE_ASM("asm/nonmatchings/main", func_800137E0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001383C);

INCLUDE_ASM("asm/nonmatchings/main", func_800138AC);

INCLUDE_ASM("asm/nonmatchings/main", func_80013DD0);

INCLUDE_ASM("asm/nonmatchings/main", func_80013EA4);

INCLUDE_ASM("asm/nonmatchings/main", func_80013ED8);

INCLUDE_ASM("asm/nonmatchings/main", func_80013F28);

INCLUDE_ASM("asm/nonmatchings/main", func_80013F98);

INCLUDE_ASM("asm/nonmatchings/main", func_8001467C);

INCLUDE_ASM("asm/nonmatchings/main", func_800147B8);

INCLUDE_ASM("asm/nonmatchings/main", func_80014838);

INCLUDE_ASM("asm/nonmatchings/main", func_80014874);

INCLUDE_ASM("asm/nonmatchings/main", func_800148B4);

INCLUDE_ASM("asm/nonmatchings/main", func_800148F4);

INCLUDE_ASM("asm/nonmatchings/main", func_80014930);

INCLUDE_ASM("asm/nonmatchings/main", func_8001496C);

INCLUDE_ASM("asm/nonmatchings/main", func_800149A8);

INCLUDE_ASM("asm/nonmatchings/main", func_800149E0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014A2C);

INCLUDE_ASM("asm/nonmatchings/main", func_80014B14);

void setCurrentTaskCallback(TaskCallback callback) {
    D_8013AE88->callback = callback;
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014B58);

void deleteCurrentTask(void) {
    func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014BAC);

void releaseTaskResource(u16 resource) {
    u32 index = resource >> 4;
    resource %= 16;
    gTaskResourceFreeBits[index] |= 1 << resource;
}

void setControllerFlag0(u16 controller) {
    gControllerFlags[controller] |= 1;
}

void clearControllerFlag0(u16 controller) {
    u8 *flags = controller + gControllerFlags;
    *flags &= ~1;
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014CBC);

void setControllerFlag5(u16 controller) {
    gControllerFlags[controller] |= 0x20;
    D_8013AF08[controller] = -1;
}

void clearControllerFlag5(u16 controller) {
    u8 *flags = controller + gControllerFlags;
    *flags &= ~0x20;
    D_8013AF08[controller] = -1;
}

s32 isControllerFlag5Clear(u16 controller) {
    s32 flags = gControllerFlags[controller];
    flags &= 0x20;
    if (flags) {
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014D60);

INCLUDE_ASM("asm/nonmatchings/main", func_80014FCC);

INCLUDE_ASM("asm/nonmatchings/main", func_800150A4);

INCLUDE_ASM("asm/nonmatchings/main", func_80015154);

INCLUDE_ASM("asm/nonmatchings/main", func_800153EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800154D0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001553C);

INCLUDE_ASM("asm/nonmatchings/main", func_800157DC);

INCLUDE_ASM("asm/nonmatchings/main", func_80015924);

INCLUDE_ASM("asm/nonmatchings/main", func_8001596C);

INCLUDE_ASM("asm/nonmatchings/main", func_80015A30);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C94);

INCLUDE_ASM("asm/nonmatchings/main", func_80015CF8);
