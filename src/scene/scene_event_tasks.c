#include "scene_event_tasks_internal.h"

s16 gSceneCdCommands[230] __attribute__((section(".data.scene_cd_commands"))) = {
    0x0000, 0x0010, 0x0011, 0x0077, 0x00BE, 0x0047, 0x00A5, 0x0000, 0x00E1, 0x008E,
    0x0046, 0x00D0, 0x00C7, 0x00BD, 0x001B, 0x00D9, 0x00E0, 0x008D, 0x003B, 0x005E,
    0x0031, 0x005D, 0x00BC, 0x00AC, 0x009E, 0x00CF, 0x0012, 0x00C6, 0x00B8, 0x009D,
    0x003F, 0x008C, 0x00B7, 0x00CE, 0x00D6, 0x00E7, 0x0094, 0x005F, 0x00B6, 0x00D2,
    0x0030, 0x0013, 0x0045, 0x00B5, 0x003E, 0x0052, 0x001D, 0x00AB, 0x00C5, 0x004E,
    0x00B4, 0x004D, 0x0068, 0x0076, 0x0016, 0x0026, 0x00D8, 0x0020, 0x0082, 0x0062,
    0x003A, 0x00B3, 0x0051, 0x00D5, 0x008B, 0x00A4, 0x0093, 0x0043, 0x0092, 0x00C4,
    0x001C, 0x0018, 0x00B2, 0x00DF, 0x002B, 0x009C, 0x0067, 0x00ED, 0x00E6, 0x00DE,
    0x00E5, 0x00E4, 0x00CD, 0x00A3, 0x0017, 0x0063, 0x0024, 0x00A0, 0x0059, 0x0095,
    0x004A, 0x0058, 0x009F, 0x0049, 0x0048, 0x0021, 0x0083, 0x0041, 0x0057, 0x0056,
    0x0080, 0x0066, 0x00E3, 0x00EA, 0x0055, 0x00DC, 0x0089, 0x005B, 0x0098, 0x007F,
    0x002E, 0x0035, 0x007A, 0x007E, 0x00E9, 0x0061, 0x0091, 0x00EC, 0x0081, 0x002A,
    0x0042, 0x001F, 0x0050, 0x002F, 0x00D4, 0x0039, 0x006D, 0x0075, 0x00D1, 0x0000,
    0x00EE, 0x001E, 0x0029, 0x0014, 0x0038, 0x0015, 0x00BB, 0x0033, 0x004C, 0x00A8,
    0x0037, 0x0036, 0x006B, 0x00A2, 0x008A, 0x00A1, 0x00F0, 0x00B1, 0x006F, 0x0028,
    0x009A, 0x0099, 0x00AA, 0x00EB, 0x005C, 0x009B, 0x006E, 0x00DD, 0x00C3, 0x003C,
    0x0022, 0x0025, 0x00DB, 0x0073, 0x0027, 0x00E2, 0x002D, 0x0019, 0x0088, 0x002C,
    0x0079, 0x0032, 0x005A, 0x0040, 0x00B0, 0x0072, 0x0097, 0x0065, 0x0078, 0x0060,
    0x00BA, 0x00CC, 0x004F, 0x00D7, 0x0034, 0x0090, 0x008F, 0x0087, 0x00CB, 0x00AF,
    0x001A, 0x00D3, 0x006A, 0x00CA, 0x00A7, 0x00C0, 0x0044, 0x0071, 0x0054, 0x0086,
    0x0000, 0x007D, 0x004B, 0x00AE, 0x0023, 0x00E8, 0x0070, 0x0069, 0x0085, 0x00AD,
    0x00C9, 0x0084, 0x00BF, 0x00B9, 0x00A6, 0x00C8, 0x0096, 0x0064, 0x0053, 0x00DA,
    0x00A9, 0x007C, 0x00C2, 0x007B, 0x006C, 0x0074, 0x00C1, 0x003D, 0x0000, -1,
};

CdResourceLocation gSceneImageSectorRanges[108] __attribute__((section(".data.scene_image_sector_ranges"))) = {
    {DAT_SCENE_IMAGE_000_SECTOR, DAT_SCENE_IMAGE_000_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_001_SECTOR, DAT_SCENE_IMAGE_001_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_002_SECTOR, DAT_SCENE_IMAGE_002_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_003_SECTOR, DAT_SCENE_IMAGE_003_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_004_SECTOR, DAT_SCENE_IMAGE_004_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_005_SECTOR, DAT_SCENE_IMAGE_005_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_006_SECTOR, DAT_SCENE_IMAGE_006_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_007_SECTOR, DAT_SCENE_IMAGE_007_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_008_SECTOR, DAT_SCENE_IMAGE_008_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_009_SECTOR, DAT_SCENE_IMAGE_009_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_010_SECTOR, DAT_SCENE_IMAGE_010_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_011_SECTOR, DAT_SCENE_IMAGE_011_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_012_SECTOR, DAT_SCENE_IMAGE_012_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_013_SECTOR, DAT_SCENE_IMAGE_013_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_014_SECTOR, DAT_SCENE_IMAGE_014_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_015_SECTOR, DAT_SCENE_IMAGE_015_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_016_SECTOR, DAT_SCENE_IMAGE_016_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_017_SECTOR, DAT_SCENE_IMAGE_017_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_018_SECTOR, DAT_SCENE_IMAGE_018_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_019_SECTOR, DAT_SCENE_IMAGE_019_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_020_SECTOR, DAT_SCENE_IMAGE_020_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_021_SECTOR, DAT_SCENE_IMAGE_021_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_022_SECTOR, DAT_SCENE_IMAGE_022_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_023_SECTOR, DAT_SCENE_IMAGE_023_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_024_SECTOR, DAT_SCENE_IMAGE_024_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_025_SECTOR, DAT_SCENE_IMAGE_025_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_026_SECTOR, DAT_SCENE_IMAGE_026_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_027_SECTOR, DAT_SCENE_IMAGE_027_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_028_SECTOR, DAT_SCENE_IMAGE_028_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_029_SECTOR, DAT_SCENE_IMAGE_029_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_030_SECTOR, DAT_SCENE_IMAGE_030_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_031_SECTOR, DAT_SCENE_IMAGE_031_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_032_SECTOR, DAT_SCENE_IMAGE_032_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_033_SECTOR, DAT_SCENE_IMAGE_033_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_034_SECTOR, DAT_SCENE_IMAGE_034_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_035_SECTOR, DAT_SCENE_IMAGE_035_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_036_SECTOR, DAT_SCENE_IMAGE_036_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_037_SECTOR, DAT_SCENE_IMAGE_037_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_038_SECTOR, DAT_SCENE_IMAGE_038_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_039_SECTOR, DAT_SCENE_IMAGE_039_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_040_SECTOR, DAT_SCENE_IMAGE_040_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_041_SECTOR, DAT_SCENE_IMAGE_041_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_042_SECTOR, DAT_SCENE_IMAGE_042_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_043_SECTOR, DAT_SCENE_IMAGE_043_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_044_SECTOR, DAT_SCENE_IMAGE_044_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_045_SECTOR, DAT_SCENE_IMAGE_045_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_046_SECTOR, DAT_SCENE_IMAGE_046_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_047_SECTOR, DAT_SCENE_IMAGE_047_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_048_SECTOR, DAT_SCENE_IMAGE_048_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_049_SECTOR, DAT_SCENE_IMAGE_049_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_050_SECTOR, DAT_SCENE_IMAGE_050_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_051_SECTOR, DAT_SCENE_IMAGE_051_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_052_SECTOR, DAT_SCENE_IMAGE_052_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_053_SECTOR, DAT_SCENE_IMAGE_053_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_000_SECTOR, DAT_SCENE_IMAGE_000_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_055_SECTOR, DAT_SCENE_IMAGE_055_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_056_SECTOR, DAT_SCENE_IMAGE_056_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_057_SECTOR, DAT_SCENE_IMAGE_057_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_058_SECTOR, DAT_SCENE_IMAGE_058_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_059_SECTOR, DAT_SCENE_IMAGE_059_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_060_SECTOR, DAT_SCENE_IMAGE_060_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_061_SECTOR, DAT_SCENE_IMAGE_061_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_062_SECTOR, DAT_SCENE_IMAGE_062_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_063_SECTOR, DAT_SCENE_IMAGE_063_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_064_SECTOR, DAT_SCENE_IMAGE_064_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_065_SECTOR, DAT_SCENE_IMAGE_065_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_066_SECTOR, DAT_SCENE_IMAGE_066_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_067_SECTOR, DAT_SCENE_IMAGE_067_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_068_SECTOR, DAT_SCENE_IMAGE_068_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_069_SECTOR, DAT_SCENE_IMAGE_069_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_070_SECTOR, DAT_SCENE_IMAGE_070_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_071_SECTOR, DAT_SCENE_IMAGE_071_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_072_SECTOR, DAT_SCENE_IMAGE_072_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_073_SECTOR, DAT_SCENE_IMAGE_073_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_074_SECTOR, DAT_SCENE_IMAGE_074_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_075_SECTOR, DAT_SCENE_IMAGE_075_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_076_SECTOR, DAT_SCENE_IMAGE_076_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_077_SECTOR, DAT_SCENE_IMAGE_077_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_078_SECTOR, DAT_SCENE_IMAGE_078_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_079_SECTOR, DAT_SCENE_IMAGE_079_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_080_SECTOR, DAT_SCENE_IMAGE_080_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_081_SECTOR, DAT_SCENE_IMAGE_081_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_082_SECTOR, DAT_SCENE_IMAGE_082_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_083_SECTOR, DAT_SCENE_IMAGE_083_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_084_SECTOR, DAT_SCENE_IMAGE_084_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_085_SECTOR, DAT_SCENE_IMAGE_085_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_086_SECTOR, DAT_SCENE_IMAGE_086_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_087_SECTOR, DAT_SCENE_IMAGE_087_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_088_SECTOR, DAT_SCENE_IMAGE_088_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_089_SECTOR, DAT_SCENE_IMAGE_089_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_090_SECTOR, DAT_SCENE_IMAGE_090_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_091_SECTOR, DAT_SCENE_IMAGE_091_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_092_SECTOR, DAT_SCENE_IMAGE_092_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_093_SECTOR, DAT_SCENE_IMAGE_093_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_094_SECTOR, DAT_SCENE_IMAGE_094_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_095_SECTOR, DAT_SCENE_IMAGE_095_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_096_SECTOR, DAT_SCENE_IMAGE_096_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_097_SECTOR, DAT_SCENE_IMAGE_097_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_098_SECTOR, DAT_SCENE_IMAGE_098_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_099_SECTOR, DAT_SCENE_IMAGE_099_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_100_SECTOR, DAT_SCENE_IMAGE_100_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_101_SECTOR, DAT_SCENE_IMAGE_101_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_102_SECTOR, DAT_SCENE_IMAGE_102_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_103_SECTOR, DAT_SCENE_IMAGE_103_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_104_SECTOR, DAT_SCENE_IMAGE_104_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_105_SECTOR, DAT_SCENE_IMAGE_105_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_106_SECTOR, DAT_SCENE_IMAGE_106_SECTOR_COUNT},
    {DAT_SCENE_IMAGE_107_SECTOR, DAT_SCENE_IMAGE_107_SECTOR_COUNT},
};

u8 gSceneImageModeOffsets[12] __attribute__((section(".data.scene_image_sector_ranges"))) = {
    0, 31, 40, 46, 53, 62, 67, 78, 85, 93, 101, 108
};

GameTask *gSceneImageDisplayTask __attribute__((section(".sdata.scene_image_display_task"))) = NULL;

void setSceneImageEventTask(SceneEvent *entry) {
    SceneImageOwner *owner;
    SceneImageResource *resource;
    s16 index;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (SceneImageOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL || entry->resourceId == 0) {
        owner->imageTask = NULL;
        gSceneImageDisplayTask = NULL;
    } else {
        func_80014874(initializeSceneImageTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SceneImageResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        owner->imageTask = D_8013AE8C;
        if (entry->mode == 0) resource->deadline = 0;
        else resource->deadline = owner->tick + entry->duration;
        owner->flags |= 1;
        if (entry->resourceId != owner->imageId) {
            owner->imageId = entry->resourceId;
            index = gSceneImageModeOffsets[D_8013AF3A] + entry->resourceId - 1;
            resetPendingCdRequests();
            queueCdSectorCommand(0, gSceneImageSectorRanges[index].sector, gSceneImageSectorRanges[index].count, DAT_SCENE_IMAGE, 0);
        }
    }
}

void initializeSceneImageTask(void) {
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {
        }
    }
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(updateSceneImageTask);
    D_8013AE88->unk10 |= 2;
}

void updateSceneImageTask(void) {
    SceneImageOwner *owner;
    SceneImageResource *resource;
    SceneEvent *entry;
    if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
    owner = (SceneImageOwner *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (SceneImageResource *)&D_800D9504[D_8013AE88->resource14];
    entry = resource->entry;
    if (owner->imageTask == NULL) {
        gSceneImageDisplayTask = NULL;
        owner->flags &= ~1;
    }
    if (owner->imageTask == D_8013AE88) {
        switch (D_8013AE88->data18) {
        case 0:
            owner->flags &= ~1;
            uploadTimImageAt(320, 256, 768, 234, DAT_SCENE_IMAGE);
            D_8013B59C = 0;
            switch (entry->mode) {
            case 0:
                configureTaskStateTransition(0, 5, 0);
                D_8013B59E = 1;
                break;
            case 1:
                configureTaskStateTransition(1, 5, entry->duration);
                D_8013B59E = 1;
                D_8013B59C = entry->duration;
                break;
            case 2:
                configureTaskStateTransition(2, 5, entry->duration);
                D_8013B59E = 0;
                D_8013B59C = entry->duration;
                break;
            case 3:
                configureTaskStateTransition(4, 5, entry->duration);
                D_8013B59E = 1;
                D_8013B59C = entry->duration;
                break;
            case 4:
                configureTaskStateTransition(5, 5, entry->duration);
                D_8013B59E = 0;
                D_8013B59C = entry->duration;
                break;
            }
            func_80014930(&gGameTasks[4], drawSceneImageTask);
            gSceneImageDisplayTask = D_8013AE8C;
            D_8013AE88->data18 = 1;
            break;
        case 1:
            if (owner->tick > resource->deadline) {
                owner->imageTask = NULL;
                if (owner->completionTick < resource->deadline) owner->completionTick = resource->deadline;
                func_800149A8();
            }
            break;
        }
    } else {
        func_800149A8();
    }
}

void drawSceneImageTask(void) {
    if (gSceneImageDisplayTask != D_8013AE88) {
        func_800149A8();
    } else {
        if (D_8013B59F) {
            setSpriteColor(128, 128, 128);
            func_8007240C(-140, 44, 280, 56, 128, 128, 128, 2, &D_800EBFF0[D_8013AF78], 8);
        }
        setSpriteColor(128, 128, 128);
        drawPagedTexturedRectangle(-160, -120, 320, 240, 149, 0x3AB0, &D_800EBFF0[D_8013AF78], 8);
    }
}

void setSceneTextEventTask(SceneEvent *entry) {
    SceneTextTaskResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    resource = (SceneTextTaskResource *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL) {
        resource->task = NULL;
    } else {
        func_80014874(updateSceneTextTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        ((SceneTextTaskResource *)&D_800D9504[D_8013AE8C->resource14])->entry = entry;
        D_8013AE8C->data18 = 0;
        resource->task = D_8013AE8C;
    }
}

INCLUDE_ASM("asm/nonmatchings/scene/scene_event_tasks", updateSceneTextTask);

void setSceneMusicEventTask(SceneEvent *entry) {
    SceneMusicOwner *owner;
    SceneMusicResource *resource;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    owner = (SceneMusicOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL) {
        owner->musicTask = NULL;
    } else if (entry->resourceId == 0) {
        stopMusicSequence();
        owner->musicTask = NULL;
    } else {
        func_80014874(finishSceneMusicTaskAtDeadline);
        owner->musicTask = D_8013AE8C;
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SceneMusicResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        switch (entry->mode) {
        case 0:
            if (entry->parameter14 == 0) {
                func_80017E7C(0);
            } else {
                playMusicSequence((s16)entry->parameter14, (u8)entry->parameter14, 1);
            }
            resource->deadline = 0;
            break;
        case 1:
            if (entry->duration == 0) {
                func_80017ED4(0x100, 0);
                resource->deadline = owner->tick + 15;
            } else {
                func_80017ED4((s16)(0x400 - entry->duration), 0);
                resource->deadline = owner->tick + entry->duration;
            }
            break;
        case 2:
            if (entry->duration == 0) {
                fadeOutMusicSequence(0x100);
                resource->deadline = owner->tick + 15;
            } else {
                fadeOutMusicSequence((s16)(0x400 - entry->duration));
                resource->deadline = owner->tick + entry->duration;
            }
            break;
        }
    }
}

void finishSceneMusicTaskAtDeadline(void) {
    SceneMusicOwner *owner;
    TaskResourceRecord *state;
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {}
    }
    owner = (SceneMusicOwner *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = &D_800D9504[D_8013AE88->resource14];
    if (owner->musicTask == D_8013AE88) {
        if (owner->tick < state->unk04) {
            return;
        }
        owner->musicTask = NULL;
        if (owner->completionTick < state->unk04) {
            owner->completionTick = state->unk04;
        }
    }
    func_800149A8();
}

void setSceneCdAudioEventTask(SceneEvent *entry) {
    SceneCdAudioOwner *owner;
    SceneCdAudioResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (SceneCdAudioOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL || entry->resourceId == 0) {
        SsSetSerialVol(0, 0, 0);
        resetPendingCdRequests();
        owner->task = NULL;
    } else {
        resetPendingCdRequests();
        if (entry->parameter14 == 0) SsSetSerialVol(0, 103, 103);
        else SsSetSerialVol(0, entry->parameter14, entry->parameter14);
        queueCdCommand(gSceneCdCommands[entry->resourceId], 1);
        func_80014874(updateSceneCdAudioTask);
        owner->task = D_8013AE8C;
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SceneCdAudioResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        resource->deadline = owner->tick + entry->duration;
    }
}

void updateSceneCdAudioTask(void) {
    SceneCdAudioPlaybackOwner *owner;
    SceneCdAudioResource *resource;
    SceneEvent *entry;
    if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
    owner = (SceneCdAudioPlaybackOwner *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (SceneCdAudioResource *)&D_800D9504[D_8013AE88->resource14];
    entry = resource->entry;
    if (owner->task == D_8013AE88) {
        if (entry->resourceId == 0) owner->task = NULL;
        if (owner->task == NULL) {
            SsSetSerialVol(0, 0, 0);
            resetPendingCdRequests();
        } else if (!(owner->flags & 2) && (D_8013AEE8[0] & 0xB0)) {
            SsSetSerialVol(0, 0, 0);
            resetPendingCdRequests();
            owner->task = NULL;
        } else {
            if (owner->tick < resource->deadline) return;
            SsSetSerialVol(0, 0, 0);
            resetPendingCdRequests();
            owner->task = NULL;
            if (owner->completionTick < resource->deadline) owner->completionTick = resource->deadline;
        }
    }
    func_800149A8();
}

void processSceneSoundEvent(SceneEvent *entry) {
    ScenePlaybackState *owner;
    SceneSoundSlot *slot;
    u16 soundKey;
    s16 volume;
    s16 i;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (ScenePlaybackState *)&D_800D9504[D_8013AE88->resource16];
    if (entry != NULL && entry->resourceId != 0) {
        soundKey = (entry->resourceId - 1) << 8;
        volume = 32;
        if (entry->parameter14 != 0) volume = entry->parameter14;
        switch (entry->mode) {
        case 0:
            gSceneOneShotVoice = playSoundEffect(1, (soundKey >> 8) << 8, volume);
            gSceneOneShotSoundKey = soundKey;
            break;
        case 1:
            for (i = 0; i < 8; i++) {
                slot = &gSceneSoundSlots[i];
                if (slot->voice == 0xFF) {
                    slot->voice = playOwnedSoundEffect(0, 1, (soundKey >> 8) << 8, volume);
                    slot->soundKey = soundKey;
                    break;
                }
            }
            break;
        case 2:
            for (i = 0; i < 8; i++) {
                slot = &gSceneSoundSlots[i];
                if (slot->voice != 0xFF && slot->soundKey == soundKey) {
                    func_800187B8(slot->voice);
                    slot->voice = 0xFF;
                    slot->soundKey = 0;
                    break;
                }
            }
            break;
        }
    }
    owner->tasks[SCENE_EVENT_SOUND] = NULL;
}

void stopSceneSoundVoices(void) {
    s16 i;
    for (i = 0; i < 8; i++) {
        if (gSceneSoundSlots[i].voice != 0xFF) {
            func_800187B8(gSceneSoundSlots[i].voice);
            gSceneSoundSlots[i].voice = 0xFF;
            gSceneSoundSlots[i].soundKey = 0;
        }
    }
}

void beginScenePlaybackExitTask(void) {
    SceneExitState *state;
    s16 i;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (SceneExitState *)&D_800D9504[D_8013AE88->resource16];
    for (i = 0; i < 5; i++) {
        state->tasks[i] = NULL;
    }
    gSceneImageDisplayTask = NULL;
    func_8001814C();
    SsSeqSetVol(gMusicSequenceId, 64, 64);
    fadeOutMusicSequence(0x200);
    resetPendingCdRequests();
    SsSetSerialVol(0, 103, 103);
    configureTaskStateTransition(0, 5, 0);
    D_8013AE88->data18 = 10;
    setCurrentTaskCallback(finishScenePlaybackExitTask);
}

void finishScenePlaybackExitTask(void) {
    if (--D_8013AE88->data18 == 0) {
        D_8013AFE4 |= 0x200;
        func_800149A8();
    }
}
