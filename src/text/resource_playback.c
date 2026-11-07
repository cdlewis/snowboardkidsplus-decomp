#include "resource_playback_internal.h"

s16 gTextEntryCdCommands[230] __attribute__((section(".data.text_entry_cd_commands"))) = {
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

CdResourceLocation gSecondaryResourceLocations[108] __attribute__((section(".data.secondary_resource_tables"))) = {
    {0x1B49, 0x0026},
    {0x136B, 0x0026},
    {0x1391, 0x0026},
    {0x13B7, 0x0026},
    {0x13DD, 0x0026},
    {0x1403, 0x0026},
    {0x1429, 0x0026},
    {0x144F, 0x0026},
    {0x1475, 0x0026},
    {0x149B, 0x0026},
    {0x14C1, 0x0026},
    {0x14E7, 0x0026},
    {0x150D, 0x0026},
    {0x1533, 0x0026},
    {0x1559, 0x0026},
    {0x157F, 0x0026},
    {0x15A5, 0x0026},
    {0x15CB, 0x0026},
    {0x15F1, 0x0026},
    {0x1617, 0x0026},
    {0x163D, 0x0026},
    {0x1663, 0x0026},
    {0x1689, 0x0026},
    {0x16AF, 0x0026},
    {0x16D5, 0x0026},
    {0x16FB, 0x0026},
    {0x1721, 0x0026},
    {0x1747, 0x0026},
    {0x176D, 0x0026},
    {0x1793, 0x0026},
    {0x17B9, 0x0026},
    {0x17DF, 0x0026},
    {0x1805, 0x0026},
    {0x182B, 0x0026},
    {0x1851, 0x0026},
    {0x1877, 0x0026},
    {0x189D, 0x0026},
    {0x18C3, 0x0026},
    {0x18E9, 0x0026},
    {0x190F, 0x0026},
    {0x1935, 0x0026},
    {0x195B, 0x0026},
    {0x1981, 0x0026},
    {0x19A7, 0x0026},
    {0x19CD, 0x0026},
    {0x19F3, 0x0026},
    {0x1A19, 0x0026},
    {0x1A3F, 0x0026},
    {0x1A65, 0x0026},
    {0x1A8B, 0x0026},
    {0x1AB1, 0x0026},
    {0x1AD7, 0x0026},
    {0x1AFD, 0x0026},
    {0x1B23, 0x0026},
    {0x1B49, 0x0026},
    {0x1B6F, 0x0026},
    {0x1B95, 0x0026},
    {0x1BBB, 0x0026},
    {0x1BE1, 0x0026},
    {0x1C07, 0x0026},
    {0x1C2D, 0x0026},
    {0x1C53, 0x0026},
    {0x1C79, 0x0026},
    {0x1C9F, 0x0026},
    {0x1CC5, 0x0026},
    {0x1CEB, 0x0026},
    {0x1D11, 0x0026},
    {0x1D37, 0x0026},
    {0x1D5D, 0x0026},
    {0x1D83, 0x0026},
    {0x1DA9, 0x0026},
    {0x1DCF, 0x0026},
    {0x1DF5, 0x0026},
    {0x1E1B, 0x0026},
    {0x1E41, 0x0026},
    {0x1E67, 0x0026},
    {0x1E8D, 0x0026},
    {0x1EB3, 0x0026},
    {0x1ED9, 0x0026},
    {0x1EFF, 0x0026},
    {0x1F25, 0x0026},
    {0x1F4B, 0x0026},
    {0x1F71, 0x0026},
    {0x1F97, 0x0026},
    {0x1FBD, 0x0026},
    {0x1FE3, 0x0026},
    {0x2009, 0x0026},
    {0x202F, 0x0026},
    {0x2055, 0x0026},
    {0x207B, 0x0026},
    {0x20A1, 0x0026},
    {0x20C7, 0x0026},
    {0x20ED, 0x0026},
    {0x2113, 0x0026},
    {0x2139, 0x0026},
    {0x215F, 0x0026},
    {0x2185, 0x0026},
    {0x21AB, 0x0026},
    {0x21D1, 0x0026},
    {0x21F7, 0x0026},
    {0x221D, 0x0026},
    {0x2243, 0x0026},
    {0x2269, 0x0026},
    {0x228F, 0x0026},
    {0x22B5, 0x0026},
    {0x22DB, 0x0026},
    {0x2301, 0x0026},
    {0x2327, 0x0026},
};

u8 gSecondaryResourceModeOffsets[12] __attribute__((section(".data.secondary_resource_tables"))) = {
    0, 31, 40, 46, 53, 62, 67, 78, 85, 93, 101, 108
};

GameTask *gSecondaryResourceDisplayTask __attribute__((section(".sdata.secondary_resource_display_task"))) = NULL;

void setSecondaryResourceEntryTask(TextEntry *entry) {
    SecondaryOwner *owner;
    SecondaryResource *resource;
    s16 index;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (SecondaryOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL || entry->unk06 == 0) {
        owner->task = NULL;
        gSecondaryResourceDisplayTask = NULL;
    } else {
        func_80014874(initializeSecondaryResourceTask);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (SecondaryResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        owner->task = D_8013AE8C;
        if (entry->unk08 == 0) resource->deadline = 0;
        else resource->deadline = owner->tick + entry->unk0A;
        owner->flags |= 1;
        if (entry->unk06 != owner->resource) {
            owner->resource = entry->unk06;
            index = gSecondaryResourceModeOffsets[D_8013AF3A] + entry->unk06 - 1;
            resetPendingCdRequests();
            queueCdSectorCommand(0, gSecondaryResourceLocations[index].sector, gSecondaryResourceLocations[index].count, (void *)0x80184060, 0);
        }
    }
}

void initializeSecondaryResourceTask(void) {
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {
        }
    }
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(updateSecondaryResourceTask);
    D_8013AE88->unk10 |= 2;
}

void updateSecondaryResourceTask(void) {
    SecondaryOwner *owner;
    SecondaryResource *resource;
    TextEntry *entry;
    if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
    owner = (SecondaryOwner *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (SecondaryResource *)&D_800D9504[D_8013AE88->resource14];
    entry = resource->entry;
    if (owner->task == NULL) {
        gSecondaryResourceDisplayTask = NULL;
        owner->flags &= ~1;
    }
    if (owner->task == D_8013AE88) {
        switch (D_8013AE88->data18) {
        case 0:
            owner->flags &= ~1;
            uploadTimImageAt(320, 256, 768, 234, (u32 *)0x80184060);
            D_8013B59C = 0;
            switch (entry->unk08) {
            case 0:
                configureTaskStateTransition(0, 5, 0);
                D_8013B59E = 1;
                break;
            case 1:
                configureTaskStateTransition(1, 5, entry->unk0A);
                D_8013B59E = 1;
                D_8013B59C = entry->unk0A;
                break;
            case 2:
                configureTaskStateTransition(2, 5, entry->unk0A);
                D_8013B59E = 0;
                D_8013B59C = entry->unk0A;
                break;
            case 3:
                configureTaskStateTransition(4, 5, entry->unk0A);
                D_8013B59E = 1;
                D_8013B59C = entry->unk0A;
                break;
            case 4:
                configureTaskStateTransition(5, 5, entry->unk0A);
                D_8013B59E = 0;
                D_8013B59C = entry->unk0A;
                break;
            }
            func_80014930(&gGameTasks[4], drawSecondaryTextBackgroundTask);
            gSecondaryResourceDisplayTask = D_8013AE8C;
            D_8013AE88->data18 = 1;
            break;
        case 1:
            if (owner->tick > resource->deadline) {
                owner->task = NULL;
                if (owner->completionTick < resource->deadline) owner->completionTick = resource->deadline;
                func_800149A8();
            }
            break;
        }
    } else {
        func_800149A8();
    }
}

void drawSecondaryTextBackgroundTask(void) {
    if (gSecondaryResourceDisplayTask != D_8013AE88) {
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

void setSecondaryTextEntryTask(TextEntry *entry) {
    TextTaskResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextTaskResource *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL) {
        resource->task = NULL;
    } else {
        func_80014874(func_800A6E88);
        D_8013AE8C->resource14 = allocateTaskResource();
        ((TextTaskResource *)&D_800D9504[D_8013AE8C->resource14])->entry = entry;
        D_8013AE8C->data18 = 0;
        resource->task = D_8013AE8C;
    }
}

INCLUDE_ASM("asm/nonmatchings/text/resource_playback", func_800A6E88);

void setTextEntryMusicTask(TextEntry *entry) {
    MusicOwnerState *owner;
    MusicResource *resource;

    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    owner = (MusicOwnerState *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL) {
        owner->musicTask = NULL;
    } else if (entry->unk06 == 0) {
        stopMusicSequence();
        owner->musicTask = NULL;
    } else {
        func_80014874(finishMusicTaskAtDeadline);
        owner->musicTask = D_8013AE8C;
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (MusicResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        switch (entry->unk08) {
        case 0:
            if (entry->unk14 == 0) {
                func_80017E7C(0);
            } else {
                playMusicSequence((s16)entry->unk14, (u8)entry->unk14, 1);
            }
            resource->deadline = 0;
            break;
        case 1:
            if (entry->unk0A == 0) {
                func_80017ED4(0x100, 0);
                resource->deadline = owner->tick + 15;
            } else {
                func_80017ED4((s16)(0x400 - entry->unk0A), 0);
                resource->deadline = owner->tick + entry->unk0A;
            }
            break;
        case 2:
            if (entry->unk0A == 0) {
                fadeOutMusicSequence(0x100);
                resource->deadline = owner->tick + 15;
            } else {
                fadeOutMusicSequence((s16)(0x400 - entry->unk0A));
                resource->deadline = owner->tick + entry->unk0A;
            }
            break;
        }
    }
}

void finishMusicTaskAtDeadline(void) {
    MusicOwnerState *owner;
    TaskResourceRecord *state;
    if (D_8013AE98->resource16 == 0xFFFF) {
        while (1) {}
    }
    owner = (MusicOwnerState *)&D_800D9504[D_8013AE98->resource16];
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

void setTextEntryCdAudioTask(TextEntry *entry) {
    CdAudioOwner *owner;
    CdAudioResource *resource;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (CdAudioOwner *)&D_800D9504[D_8013AE88->resource16];
    if (entry == NULL || entry->unk06 == 0) {
        SsSetSerialVol(0, 0, 0);
        resetPendingCdRequests();
        owner->task = NULL;
    } else {
        resetPendingCdRequests();
        if (entry->unk14 == 0) SsSetSerialVol(0, 103, 103);
        else SsSetSerialVol(0, entry->unk14, entry->unk14);
        queueCdCommand(gTextEntryCdCommands[entry->unk06], 1);
        func_80014874(updateTextEntryCdAudioTask);
        owner->task = D_8013AE8C;
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (CdAudioResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->entry = entry;
        resource->deadline = owner->tick + entry->unk0A;
    }
}

void updateTextEntryCdAudioTask(void) {
    CdAudioPlaybackOwner *owner;
    CdAudioResource *resource;
    TextEntry *entry;
    if (D_8013AE98->resource16 == 0xFFFF) { while (1) {} }
    owner = (CdAudioPlaybackOwner *)&D_800D9504[D_8013AE98->resource16];
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (CdAudioResource *)&D_800D9504[D_8013AE88->resource14];
    entry = resource->entry;
    if (owner->task == D_8013AE88) {
        if (entry->unk06 == 0) owner->task = NULL;
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

void processTextEntrySound(TextEntry *entry) {
    TextPlaybackState *owner;
    TextSoundSlot *slot;
    u16 soundKey;
    s16 volume;
    s16 i;
    if (D_8013AE88->resource16 == 0xFFFF) { while (1) {} }
    owner = (TextPlaybackState *)&D_800D9504[D_8013AE88->resource16];
    if (entry != NULL && entry->unk06 != 0) {
        soundKey = (entry->unk06 - 1) << 8;
        volume = 32;
        if (entry->unk14 != 0) volume = entry->unk14;
        switch (entry->unk08) {
        case 0:
            gTextOneShotVoice = playSoundEffect(1, (soundKey >> 8) << 8, volume);
            gTextOneShotSoundKey = soundKey;
            break;
        case 1:
            for (i = 0; i < 8; i++) {
                slot = &gTextSoundSlots[i];
                if (slot->voice == 0xFF) {
                    slot->voice = playOwnedSoundEffect(0, 1, (soundKey >> 8) << 8, volume);
                    slot->soundKey = soundKey;
                    break;
                }
            }
            break;
        case 2:
            for (i = 0; i < 8; i++) {
                slot = &gTextSoundSlots[i];
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
    owner->tasks[4] = NULL;
}

void stopTextSoundVoices(void) {
    s16 i;
    for (i = 0; i < 8; i++) {
        if (gTextSoundSlots[i].voice != 0xFF) {
            func_800187B8(gTextSoundSlots[i].voice);
            gTextSoundSlots[i].voice = 0xFF;
            gTextSoundSlots[i].soundKey = 0;
        }
    }
}

void beginAudioFadeCountdownTask(void) {
    AudioTransitionState *state;
    s16 i;
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    state = (AudioTransitionState *)&D_800D9504[D_8013AE88->resource16];
    for (i = 0; i < 5; i++) {
        state->unk2C[i] = 0;
    }
    gSecondaryResourceDisplayTask = NULL;
    func_8001814C();
    SsSeqSetVol(gMusicSequenceId, 64, 64);
    fadeOutMusicSequence(0x200);
    resetPendingCdRequests();
    SsSetSerialVol(0, 103, 103);
    configureTaskStateTransition(0, 5, 0);
    D_8013AE88->data18 = 10;
    setCurrentTaskCallback(signalFlag9WhenTaskCountdownExpires);
}

void signalFlag9WhenTaskCountdownExpires(void) {
    if (--D_8013AE88->data18 == 0) {
        D_8013AFE4 |= 0x200;
        func_800149A8();
    }
}
