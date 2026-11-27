#include "game/engine/game_loop.h"
#include "game/save/save_data.h"
#include "psx/scratchpad.h"
#include "psx/libetc.h"
#include "../../graphics/render_effects_internal.h"
#include "game/audio/music.h"
#include "game/engine/angles.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/random.h"
#include "game/engine/task.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/menu/selected_records.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_geometry.h"
#include "game/race/course/course_objects.h"
#include "game/race/course/course_offset.h"
#include "game/race/player/race_player.h"
#include "psx/libapi.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

#define COURSE_PLACEMENT_LIMIT 140
#define PLACEMENT_END 0xFF
#define PLACEMENT_FLAG_COLLECTED 0x40

typedef struct PlacedObjectBounceVisual {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x24];
    s32 height, unk78, velocity;
} PlacedObjectBounceVisual;

/* Fixed-point motion for a particle spawned by a proximity emitter. */
typedef struct ProximityParticleResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    Position3D position;
    s32 unkAC;
    s32 velocityX, velocityY, velocityZ, gravity;
    SVECTOR rotation;
    u8 unkC8[8];
    s16 geometryIndex;
    u8 unkD2[0x2E];
} ProximityParticleResource;

extern u8 gModeSequenceStage __attribute__((section(".data")));
extern u16 gGameStateFlags __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern VECTOR *gScratchVector0 __attribute__((section(".data")));
extern SVECTOR *gScratchShortVector1 __attribute__((section(".data")));
extern MATRIX *gScratchMatrix0 __attribute__((section(".data")));
extern u32 *gCurrentPlayerOrderingTable __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 gRaceRuleset __attribute__((section(".data")));
extern u8 gMainMenuSelectionIndex __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));
extern s16 *gCourseGeometryVertexData __attribute__((section(".data")));
extern s16 *gCourseGeometryRegionData __attribute__((section(".data")));

extern VECTOR *gScratchVector1 __attribute__((section(".data")));
extern u32 *D_8013AF70 __attribute__((section(".data")));
extern u32 *D_8013AF74 __attribute__((section(".data")));

extern s16 gPlayerCollectionEntryCount __attribute__((section(".data.finish_slot_state")));

extern VisualPlacement D_801259DC[], D_80125FCC[], D_8012648C[], D_801269CC[], D_80126E9C[];
extern VisualPlacement D_8012726C[], D_8012763C[], D_80127A4C[], D_80127C0C[];
extern VisualPlacement gEmptyVisualPlacements[1];
extern VisualPlacement D_8012807C[], D_801281CC[], D_8012831C[];
extern VisualPlacement D_801283CC[], D_8012865C[], D_801288EC[];
extern VisualPlacement D_80128A3C[], D_80128E0C[], D_801291DC[];

u8 gCpuSpeedModesSoloFirst[4] __attribute__((section(".data.cpu_speed_modes"))) = { 1, 0, 2, 0 };
u8 gCpuSpeedModesSoloSecond[4] __attribute__((section(".data.cpu_speed_modes"))) = { 0, 1, 2, 0 };
u8 gCpuSpeedModesSoloThird[4] __attribute__((section(".data.cpu_speed_modes"))) = { 0, 1, 2, 0 };
u8 gCpuSpeedModesSoloFourth[4] __attribute__((section(".data.cpu_speed_modes"))) = { 0, 2, 1, 0 };

TaskVisualFrame gObjectVisualFrameType8 __attribute__((section(".data.object_visual_frames"))) = {
    0x00, 0x00, 0x40, 0x80, 0x19, 0x7828
};
TaskVisualFrame gObjectVisualFrameType9 __attribute__((section(".data.object_visual_frames"))) = {
    0x40, 0x00, 0x40, 0x80, 0x19, 0x7868
};

TaskVisualFrame gInteractionVisualFrame __attribute__((section(".data.interaction_visual_frame"))) = {
    0xC0, 0x20, 0x20, 0x20, 0x1C, 0x3D7F
};
VisualPlacement *gInteractionVisualPlacements __attribute__((section(".sdata.interaction_visual_placements"))) = 0;

VisualPlacement *gCourseObjectPlacements[10] __attribute__((section(".data.course_object_placements"))) = {
    D_801259DC, D_80125FCC, D_8012648C, D_801269CC, D_80126E9C,
    D_8012726C, D_8012763C, gEmptyVisualPlacements, D_80127A4C, D_80127C0C,
};

VisualPlacement *gCourseChallengePlacements[3][10] __attribute__((section(".data.course_challenge_placements"))) = {
    {
        D_80128A3C, D_80128E0C, gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements,
        gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements, D_801291DC,
    },
    {
        D_8012807C, D_801281CC, gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements,
        gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements, D_8012831C,
    },
    {
        D_801283CC, D_8012865C, gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements,
        gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements, gEmptyVisualPlacements, D_801288EC,
    },
};

u8 D_8013B2B4 __attribute__((section(".sdata.course_object_visual_state"))) = 0;
u8 gPickupVisualPlacementStartIndex __attribute__((section(".sdata.course_object_visual_state"))) = 0;
u8 gPickupVisualPlacementCount __attribute__((section(".sdata.course_object_visual_state"))) = 0;
u8 gObjectVisualPlacementStartIndex __attribute__((section(".sdata.course_object_visual_state"))) = 0;
u8 gObjectVisualPlacementCount __attribute__((section(".sdata.course_object_visual_state"))) = 0;
u8 gCourseVisualPlacementStartIndex __attribute__((section(".sdata.course_object_visual_state"))) = 0;
u8 gCourseVisualPlacementCount __attribute__((section(".sdata.course_object_visual_state"))) = 0;
VisualPlacement *gCurrentCourseObjectPlacements __attribute__((section(".sdata.course_object_visual_state"))) = 0;
VisualPlacement *gCollectiblePlacements __attribute__((section(".sdata.course_object_visual_state"))) = 0;

TaskVisualFrame gPickupVisualFrames[6] __attribute__((section(".data.pickup_visual_frames"), aligned(4))) = {
    {0x80, 0x98, 0x20, 0x20, 0x000D, 0x3DB9},
    {0xA0, 0x98, 0x18, 0x20, 0x000D, 0x3DB9},
    {0xB8, 0x98, 0x10, 0x20, 0x000D, 0x3DB9},
    {0xC8, 0x98, 0x08, 0x20, 0x000D, 0x3DB9},
    {0xD0, 0x98, 0x10, 0x20, 0x000D, 0x3DB9},
    {0xE0, 0x98, 0x18, 0x20, 0x000D, 0x3DB9},
};

struct VisualPlacementHeightData gVisualPlacementHeightData __attribute__((section(".data.visual_placement_heights"))) = {
    0,
    {0, 56, 88, 96, -16, -40, -64, -88,
     -112, -136, -160, -184, -208, -232, -256, -280},
    0
};

CourseSectorRange gCourseResourceSectors[10][3] __attribute__((section(".data.course_sector_resources"))) = {
    {
        {DAT_COURSE_0_RESOURCE_0_SECTOR, DAT_COURSE_0_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_0_RESOURCE_1_SECTOR, DAT_COURSE_0_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_0_RESOURCE_2_SECTOR, DAT_COURSE_0_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_1_RESOURCE_0_SECTOR, DAT_COURSE_1_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_1_RESOURCE_1_SECTOR, DAT_COURSE_1_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_1_RESOURCE_2_SECTOR, DAT_COURSE_1_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_2_RESOURCE_0_SECTOR, DAT_COURSE_2_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_2_RESOURCE_1_SECTOR, DAT_COURSE_2_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_2_RESOURCE_2_SECTOR, DAT_COURSE_2_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_3_RESOURCE_0_SECTOR, DAT_COURSE_3_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_3_RESOURCE_1_SECTOR, DAT_COURSE_3_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_3_RESOURCE_2_SECTOR, DAT_COURSE_3_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_4_RESOURCE_0_SECTOR, DAT_COURSE_4_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_4_RESOURCE_1_SECTOR, DAT_COURSE_4_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_4_RESOURCE_2_SECTOR, DAT_COURSE_4_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_5_RESOURCE_0_SECTOR, DAT_COURSE_5_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_5_RESOURCE_1_SECTOR, DAT_COURSE_5_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_5_RESOURCE_2_SECTOR, DAT_COURSE_5_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_6_RESOURCE_0_SECTOR, DAT_COURSE_6_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_6_RESOURCE_1_SECTOR, DAT_COURSE_6_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_6_RESOURCE_2_SECTOR, DAT_COURSE_6_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_7_RESOURCE_0_SECTOR, DAT_COURSE_7_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_7_RESOURCE_1_SECTOR, DAT_COURSE_7_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_7_RESOURCE_2_SECTOR, DAT_COURSE_7_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_8_RESOURCE_0_SECTOR, DAT_COURSE_8_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_8_RESOURCE_1_SECTOR, DAT_COURSE_8_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_8_RESOURCE_2_SECTOR, DAT_COURSE_8_RESOURCE_2_SECTOR_COUNT},
    },
    {
        {DAT_COURSE_9_RESOURCE_0_SECTOR, DAT_COURSE_9_RESOURCE_0_SECTOR_COUNT},
        {DAT_COURSE_9_RESOURCE_1_SECTOR, DAT_COURSE_9_RESOURCE_1_SECTOR_COUNT},
        {DAT_COURSE_9_RESOURCE_2_SECTOR, DAT_COURSE_9_RESOURCE_2_SECTOR_COUNT},
    }
};

CourseSectorRange gCourseMetadataSectors[10] __attribute__((section(".data.course_sector_resources"))) = {
    {DAT_COURSE_METADATA_000_SECTOR, DAT_COURSE_METADATA_000_SECTOR_COUNT},
    {DAT_COURSE_METADATA_001_SECTOR, DAT_COURSE_METADATA_001_SECTOR_COUNT},
    {DAT_COURSE_METADATA_002_SECTOR, DAT_COURSE_METADATA_002_SECTOR_COUNT},
    {DAT_COURSE_METADATA_003_SECTOR, DAT_COURSE_METADATA_003_SECTOR_COUNT},
    {DAT_COURSE_METADATA_004_SECTOR, DAT_COURSE_METADATA_004_SECTOR_COUNT},
    {DAT_COURSE_METADATA_005_SECTOR, DAT_COURSE_METADATA_005_SECTOR_COUNT},
    {DAT_COURSE_METADATA_006_SECTOR, DAT_COURSE_METADATA_006_SECTOR_COUNT},
    {DAT_COURSE_METADATA_007_SECTOR, DAT_COURSE_METADATA_007_SECTOR_COUNT},
    {DAT_COURSE_METADATA_008_SECTOR, DAT_COURSE_METADATA_008_SECTOR_COUNT},
    {DAT_COURSE_METADATA_009_SECTOR, DAT_COURSE_METADATA_009_SECTOR_COUNT}
};

CourseSectorRange gCourseOptionalResourceSectors[10] __attribute__((section(".data.course_sector_resources"))) = {
    {DAT_COURSE_OPTIONAL_000_SECTOR, DAT_COURSE_OPTIONAL_000_SECTOR_COUNT},
    {DAT_COURSE_OPTIONAL_001_SECTOR, DAT_COURSE_OPTIONAL_001_SECTOR_COUNT},
    {0x0000, 0x0000},
    {0x0000, 0x0000},
    {DAT_COURSE_OPTIONAL_004_SECTOR, DAT_COURSE_OPTIONAL_004_SECTOR_COUNT},
    {0x0000, 0x0000},
    {DAT_COURSE_OPTIONAL_006_SECTOR, DAT_COURSE_OPTIONAL_006_SECTOR_COUNT},
    {0x0000, 0x0000},
    {DAT_COURSE_OPTIONAL_008_SECTOR, DAT_COURSE_OPTIONAL_008_SECTOR_COUNT},
    {DAT_COURSE_OPTIONAL_009_SECTOR, DAT_COURSE_OPTIONAL_009_SECTOR_COUNT}
};

u8 gCourseMetadataBuffer[0x800] __attribute__((section(".data.course_sector_resources"), aligned(4))) = {0};

EmitterPlacement gProximityEmitterPlacements[2]
    __attribute__((section(".data.proximity_emitter_placements"))) = {
    {0x2D8, 0x66D0, 0xA4E8, 0x2C, 0},
    {0x65E0, 0xAB40, 0xEFA0, 0x52, 0},
};

TaskVisualFrame gMovingVisualFrames[8][8]
    __attribute__((section(".data.moving_visual_frames"), aligned(4))) = {
    {
        {0x00, 0x84, 0x40, 0x40, 0x001A, 0x796B},
        {0x00, 0x84, 0x40, 0x40, 0x001A, 0x796B},
        {0x00, 0x84, 0x40, 0x42, 0x001A, 0x796B},
        {0x40, 0x84, 0x40, 0x42, 0x001A, 0x796B},
        {0x40, 0x84, 0x40, 0x40, 0x001A, 0x796B},
        {0x40, 0x84, 0x40, 0x40, 0x001A, 0x796B},
        {0x40, 0x84, 0x40, 0x42, 0x001A, 0x796B},
        {0x00, 0x84, 0x40, 0x42, 0x001A, 0x796B},
    },
    {
        {0x80, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0x80, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0x80, 0x42, 0x40, 0x42, 0x001A, 0x796B},
        {0xC0, 0x42, 0x40, 0x42, 0x001A, 0x796B},
        {0xC0, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0xC0, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0xC0, 0x42, 0x40, 0x42, 0x001A, 0x796B},
        {0x80, 0x42, 0x40, 0x42, 0x001A, 0x796B},
    },
    {
        {0x00, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0x00, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0x00, 0x42, 0x40, 0x42, 0x001A, 0x796B},
        {0x40, 0x42, 0x40, 0x42, 0x001A, 0x796B},
        {0x40, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0x40, 0x42, 0x40, 0x40, 0x001A, 0x796B},
        {0x40, 0x42, 0x40, 0x42, 0x001A, 0x796B},
        {0x00, 0x42, 0x40, 0x42, 0x001A, 0x796B},
    },
    {
        {0x80, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0x80, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0x80, 0x00, 0x40, 0x42, 0x001A, 0x796B},
        {0xC0, 0x00, 0x40, 0x42, 0x001A, 0x796B},
        {0xC0, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0xC0, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0xC0, 0x00, 0x40, 0x42, 0x001A, 0x796B},
        {0x80, 0x00, 0x40, 0x42, 0x001A, 0x796B},
    },
    {
        {0x00, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0x00, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0x00, 0x00, 0x40, 0x42, 0x001A, 0x796B},
        {0x40, 0x00, 0x40, 0x42, 0x001A, 0x796B},
        {0x40, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0x40, 0x00, 0x40, 0x40, 0x001A, 0x796B},
        {0x40, 0x00, 0x40, 0x42, 0x001A, 0x796B},
        {0x00, 0x00, 0x40, 0x42, 0x001A, 0x796B},
    },
    {
        {0x80, 0x00, 0x40, 0x40, 0x021A, 0x796B},
        {0x80, 0x00, 0x40, 0x40, 0x021A, 0x796B},
        {0x80, 0x00, 0x40, 0x42, 0x021A, 0x796B},
        {0xC0, 0x00, 0x40, 0x42, 0x021A, 0x796B},
        {0xC0, 0x00, 0x40, 0x40, 0x021A, 0x796B},
        {0xC0, 0x00, 0x40, 0x40, 0x021A, 0x796B},
        {0xC0, 0x00, 0x40, 0x42, 0x021A, 0x796B},
        {0x80, 0x00, 0x40, 0x42, 0x021A, 0x796B},
    },
    {
        {0x00, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0x00, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0x00, 0x42, 0x40, 0x42, 0x021A, 0x796B},
        {0x40, 0x42, 0x40, 0x42, 0x021A, 0x796B},
        {0x40, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0x40, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0x40, 0x42, 0x40, 0x42, 0x021A, 0x796B},
        {0x00, 0x42, 0x40, 0x42, 0x021A, 0x796B},
    },
    {
        {0x80, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0x80, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0x80, 0x42, 0x40, 0x42, 0x021A, 0x796B},
        {0xC0, 0x42, 0x40, 0x42, 0x021A, 0x796B},
        {0xC0, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0xC0, 0x42, 0x40, 0x40, 0x021A, 0x796B},
        {0xC0, 0x42, 0x40, 0x42, 0x021A, 0x796B},
        {0x80, 0x42, 0x40, 0x42, 0x021A, 0x796B},
    },
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

TaskVisualFrame *gObjectVisualFramesByType[2]
    __attribute__((section(".data.object_visual_frames_by_type"))) = {
    &gObjectVisualFrameType8, &gObjectVisualFrameType9,
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

u16 gPlacedObjectModelIds[8]
    __attribute__((section(".data.placed_object_model_ids"))) = {
    8, 9, 5, 6, 10, 10, 13, 0,
};

u8 gPlacedObjectEffectChoices[4][16]
    __attribute__((section(".data.placed_object_reward_choices"), aligned(4))) = {
    {4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 1, 2},
    {4, 4, 5, 5, 3, 3, 3, 3, 1, 1, 1, 1, 2, 2, 2, 2},
    {4, 5, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2},
    {4, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2},
};

u8 gPlacedObjectItemChoices[4][16]
    __attribute__((section(".data.placed_object_reward_choices"), aligned(4))) = {
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 2, 1},
    {4, 4, 4, 2, 6, 6, 6, 2, 2, 2, 2, 2, 2, 2, 1, 1},
    {4, 6, 1, 2, 2, 2, 2, 2, 6, 1, 1, 3, 3, 3, 3, 3},
    {6, 2, 2, 2, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 5},
};

void initializeCourseObjectTask(void);
void updateCourseObjectTask(void);
void initializeBobbingTrailEmitterTask(void);

void queueCourseResourceSectorReads(void) {
    queueCdSectorCommand(0, gCourseResourceSectors[gSelectedCourseIndex][0].sector, gCourseResourceSectors[gSelectedCourseIndex][0].count, DAT_COURSE_FIRST, 0);
    queueCdSectorCommand(0, gCourseResourceSectors[gSelectedCourseIndex][1].sector, gCourseResourceSectors[gSelectedCourseIndex][1].count, DAT_COURSE_SECOND, 0);
    queueCdSectorCommand(2, gCourseResourceSectors[gSelectedCourseIndex][2].sector, gCourseResourceSectors[gSelectedCourseIndex][2].count, DAT_SCRATCH, 0);
    queueCdSectorCommand(0, gCourseMetadataSectors[gSelectedCourseIndex].sector, gCourseMetadataSectors[gSelectedCourseIndex].count, gCourseMetadataBuffer, 0);
    if ((u16)gCourseOptionalResourceSectors[gSelectedCourseIndex].sector != 0) {
        queueCdSectorCommand(0, gCourseOptionalResourceSectors[gSelectedCourseIndex].sector, gCourseOptionalResourceSectors[gSelectedCourseIndex].count, DAT_COURSE_OPTIONAL, 0);
    }
    if (gSelectedCourseIndex == 6) {
        queueCdSectorCommand(16, DAT_COURSE_IMAGE_SECTOR, DAT_COURSE_IMAGE_SECTOR_COUNT, DAT_SCRATCH, 0);
    }
}

void initializeCourseObjects(void) {
    u8 i;
    const u32 *offset;
    u16 modelCount;
    u32 *model;
    s16 modelIndex;
    s32 course;

    if (gSelectedCourseIndex == 7) {
        createChildTaskForParentWithReserve(&gGameTasks[3], updateActivePlayerRegionCollisions);
        gCreatedTask->data18 = 1;
        return;
    }

    offset = RACE_EXTRA_MODEL_ARCHIVE->offsets;
    modelCount = (u16)RACE_EXTRA_MODEL_ARCHIVE->count;
    for (i = 0; i < modelCount; i++) {
        model = RACE_EXTRA_MODEL_ARCHIVE->offsets;
        model = (u32 *)((u8 *)model + *offset++);
        GsMapModelingData(model);
        /* Skip the flags and object count to reach the TMD object table. */
        model++;
        model++;
        modelIndex = i;
        GsLinkObject4(model, &gTaskObjectModels[modelIndex], 0);
        gTaskObjectModels[modelIndex].coord2 = &gCourseObjectCoordinates[modelIndex];
        gTaskObjectModels[modelIndex].tmd = model;
        gTaskObjectModels[modelIndex].attribute = 0;
        GsInitCoordinate2(0, &gCourseObjectCoordinates[modelIndex]);
    }

    for (i = 0; i < 4; i++) {
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeCourseObjectTask);
        gCreatedTask->data18 = i;
    }

    course = gSelectedCourseIndex;
    switch (course) {
    case 1:
        if (gMainMenuSelectionIndex == 0) {
            createChildTaskForParentWithReserve(&gGameTasks[4], initializeBobbingTrailEmitterTask);
        }
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeProximityEmitterTask);
        gCreatedTask->data18 = 0;
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeProximityEmitterTask);
        gCreatedTask->data18 = course;
        createChildTaskForParentWithReserve(&gGameTasks[3], updateActivePlayerRegionCollisions);
        gCreatedTask->data18 = 0;
        for (i = 0; i < 9; i++) {
            createChildTaskForParentWithReserve(&gGameTasks[4], initializeMovingVisualTask);
            gCreatedTask->data18 = i;
        }
        break;
    case 4:
        for (i = 0; i < 3; i++) {
            createChildTaskForParentWithReserve(&gGameTasks[4], initializeOrbitingTaskObject);
            gCreatedTask->data18 = 0;
            gCreatedTask->data1A = 3;
            gCreatedTask->data1C = i;
        }
        for (i = 0; i < 4; i++) {
            createChildTaskForParentWithReserve(&gGameTasks[4], initializeOrbitingTaskObject);
            gCreatedTask->data18 = 1;
            gCreatedTask->data1A = 4;
            gCreatedTask->data1C = i;
        }
        break;
    }

    gPickupVisualPlacementCount = gObjectVisualPlacementCount = gCourseVisualPlacementCount = 0;
    gPickupVisualPlacementStartIndex = gObjectVisualPlacementStartIndex = gCourseVisualPlacementStartIndex = 255;
    gCurrentCourseObjectPlacements = gCourseObjectPlacements[gSelectedCourseIndex];
    i = 0;
    do {
        VisualPlacement *placements = gCurrentCourseObjectPlacements;
        VisualPlacement *placement = (VisualPlacement *)(i * sizeof(*placement) + (u32)placements);
        s8 type = (s8)placement->unk01;
        if ((u8)type == PLACEMENT_END) {
            break;
        }
        if ((u8)type < 7) {
            if ((u8)type != 5 && !((type == 2 || type == 3) && gMainMenuSelectionIndex)) {
                tryCreateChildTaskForParentWith20SlotsReserved(&gGameTasks[4], initializePlacedObjectTask);
                gCreatedTask->resource14 = allocateTaskMemoryBlock();
                if ((u32)(gCurrentCourseObjectPlacements[i].unk01 - 2) < 2) {
                    gCreatedTask->resource16 = allocateTaskMemoryBlock();
                }
                gCreatedTask->data18 = i;
            }
        } else {
            switch (placement->unk01) {
            case 7:
                if (gMainMenuSelectionIndex == 0) {
                    if (i < gPickupVisualPlacementStartIndex) {
                        gPickupVisualPlacementStartIndex = i;
                    }
                    gPickupVisualPlacementCount++;
                }
                break;
            case 8:
            case 9:
                if (gMainMenuSelectionIndex != 1 || gRaceRuleset != 1) {
                    if (i < gObjectVisualPlacementStartIndex) {
                        gObjectVisualPlacementStartIndex = i;
                    }
                    gObjectVisualPlacementCount++;
                }
                break;
            default:
                if (i < gCourseVisualPlacementStartIndex) {
                    gCourseVisualPlacementStartIndex = i;
                }
                gCourseVisualPlacementCount++;
                break;
            }
        }
        i++;
    } while (i != COURSE_PLACEMENT_LIMIT);

    if (gMainMenuSelectionIndex == 1) {
        switch (gRaceRuleset) {
        case 0:
            gInteractionVisualPlacements = gCourseChallengePlacements[playerCountAbsolute][gSelectedCourseIndex];
            i = 0;
            if (gInteractionVisualPlacements[0].unk01 != PLACEMENT_END) {
                for (;;) {
                    VisualPlacement *placements = gInteractionVisualPlacements;
                    u8 index = i;
                    VisualPlacement *placement = (VisualPlacement *)(index * sizeof(*placement) + (u32)placements);
                    u8 type = placement->unk01;
                    if (type == PLACEMENT_END) {
                        break;
                    }
                    placement->flags &= ~PLACEMENT_FLAG_COLLECTED;
                    i++;
                }
                createChildTaskForParentWithReserve(&gGameTasks[4], initializeInteractionVisualTask);
            }
            break;
        case 1:
            i = 0;
            gCollectiblePlacements = gCourseChallengePlacements[0][gSelectedCourseIndex];
            for (;;) {
                if (gCollectiblePlacements[i].unk01 == PLACEMENT_END) {
                    goto createVisualTasks;
                }
                tryCreateChildTaskForParentWith20SlotsReserved(&gGameTasks[4], initializeCollectibleTask);
                gCreatedTask->data18 = i;
                i++;
            }
        }
    }

createVisualTasks:
    createChildTaskForParentWithReserve(&gGameTasks[4], initializePickupVisualTask);
    if (gObjectVisualPlacementCount) {
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeObjectVisualTask);
    }
    if (gCourseVisualPlacementCount) {
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeCourseVisualTask);
    }
}

u16 getRenderCameraIndex(void) {
    if (gGameStateFlags & 0x400) {
        if (gFrameCounter & 1) {
            return 1;
        }
    }
    return 0;
}

s16 advanceTaskVisualFrame(TaskVisualResource *resource) {
    resource->framePosition += resource->unkC8;
    if (resource->framePosition >= resource->unkC6) {
        resource->framePosition = (resource->framePosition - resource->unkC8) & 0xFF00;
        return 0;
    }
    return 1;
}

void drawTaskObject(u16 index) {
    GsDOBJ2 *object;
    GsDOBJ2 *objects;
    gObjectSortSavedStackPointer = SetSp((u32)getScratchAddr((sizeof(scratchpad) - sizeof(u32)) / sizeof(u32)));
    objects = gTaskObjectModels;
    object = &objects[index];
    GsGetLs(object->coord2, gScratchMatrix0);
    GsSetLsMatrix(gScratchMatrix0);
    if (getRenderCameraIndex()) {
        GsSortObject4(object, &gPlayer2OrderingTables[gActiveGraphicsBuffer], 2, (u32 *)getScratchAddr(0));
    } else {
        GsSortObject4(object, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 2, (u32 *)getScratchAddr(0));
    }
    SetSp(gObjectSortSavedStackPointer);
}

void drawTaskObjectAlternateLayer(u16 index) {
    GsDOBJ2 *object;
    GsDOBJ2 *objects;
    gAlternateObjectSortSavedStackPointer = SetSp((u32)getScratchAddr((sizeof(scratchpad) - sizeof(u32)) / sizeof(u32)));
    objects = gTaskObjectModels;
    object = &objects[index];
    GsGetLs(object->coord2, gScratchMatrix0);
    GsSetLsMatrix(gScratchMatrix0);
    if (getRenderCameraIndex()) {
        GsSortObject4(object, &gPlayer2AlternateLayerOrderingTables[gActiveGraphicsBuffer], 2, (u32 *)getScratchAddr(0));
    } else {
        GsSortObject4(object, &gPlayer1AlternateLayerOrderingTables[gActiveGraphicsBuffer], 2, (u32 *)getScratchAddr(0));
    }
    SetSp(gAlternateObjectSortSavedStackPointer);
}

void initializeCourseObjectTask(void) {
    if (gSelectedCourseIndex == 8 && gCurrentTask->data18 == 0) {
        gCurrentTask->data18 = 24;
    }

    switch (gCurrentTask->data18) {
    case 0:
    case 24:
        gCourseObjectRotations[gCurrentTask->data18].vy = gSelectedCourseRecords[gSelectedCourseIndex].unk16;
        gCurrentTask->data1C = 0x418;
        gCurrentTask->data1E = 0x240;
        gScratchShortVector0->vx = 0;
        gScratchShortVector0->vy = gCourseObjectRotations[gCurrentTask->data18].vy + gCurrentTask->data1C;
        gScratchShortVector0->vz = 0;
        gScratchShortVector1->vx = 0;
        gScratchShortVector1->vy = 0;
        gScratchShortVector1->vz = gCurrentTask->data1E;
        RotMatrix(gScratchShortVector0, gScratchMatrix0);
        ApplyMatrix(gScratchMatrix0, gScratchShortVector1, gScratchVector0);
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[0] =
            ((s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[0] >> 8) + gScratchVector0->vx;
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[1] = ((s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[1] >> 8);
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[2] =
            ((s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[2] >> 8) + gScratchVector0->vz;
        break;
    case 1:
    case 2:
        gCurrentTask->data1A = 0;
        gCourseObjectRotations[gCurrentTask->data18].vy = gSelectedCourseRecords[gSelectedCourseIndex].unk16 + 0x400;
        gCurrentTask->data1C = 0x128;
        gCurrentTask->data1E = 0xA0;
        gScratchShortVector0->vx = 0;
        gScratchShortVector0->vy = gCourseObjectRotations[gCurrentTask->data18].vy + gCurrentTask->data1C;
        gScratchShortVector0->vz = 0;
        gScratchShortVector1->vx = 0;
        gScratchShortVector1->vy = 0;
        gScratchShortVector1->vz = gCurrentTask->data1E;
        RotMatrix(gScratchShortVector0, gScratchMatrix0);
        ApplyMatrix(gScratchMatrix0, gScratchShortVector1, gScratchVector0);
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[0] =
            ((s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[0] >> 8) + gScratchVector0->vx;
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[1] = ((s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[1] >> 8);
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[2] =
            ((s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[2] >> 8) + gScratchVector0->vz;
        break;
    case 3:
        gCourseObjectRotations[gCurrentTask->data18].vy = gSelectedCourseRecords[gSelectedCourseIndex].unk14 + 0x400;
        gCurrentTask->data1C = 0x280;
        gCurrentTask->data1E = -0x78;
        gScratchShortVector0->vx = 0;
        gScratchShortVector0->vy = gCourseObjectRotations[gCurrentTask->data18].vy + gCurrentTask->data1C;
        gScratchShortVector0->vz = 0;
        gScratchShortVector1->vx = 0;
        gScratchShortVector1->vy = 0;
        gScratchShortVector1->vz = gCurrentTask->data1E;
        RotMatrix(gScratchShortVector0, gScratchMatrix0);
        ApplyMatrix(gScratchMatrix0, gScratchShortVector1, gScratchVector0);
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[0] =
            (gSelectedCourseRecords[gSelectedCourseIndex].unk08 >> 8) + gScratchVector0->vx;
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[1] = (gSelectedCourseRecords[gSelectedCourseIndex].unk0C >> 8) - 8;
        gCourseObjectCoordinates[gCurrentTask->data18].coord.t[2] =
            (gSelectedCourseRecords[gSelectedCourseIndex].unk10 >> 8) + gScratchVector0->vz;
        break;
    }

    setCurrentTaskCallback(updateCourseObjectTask);
}

void updateCourseObjectTask(void) {
    RECT rect;
    RacePlayer *players;
    RacePlayer *cameraPlayer;
    SelectedCourseRecord *records;
    GsCOORDINATE2 *coordinates;
    s16 cameraIndex;
    u16 objectIndex;

    if (!gRacePauseState && gCurrentTask->data18 == 2) {
        if (gTaskTransitionFlags & 4) {
            if (gCurrentTask->data1A != 0x400) {
                if (gCurrentTask->data1A == 0) {
                    rect.x = 640;
                    rect.y = 247;
                    rect.w = 16;
                    rect.h = 1;
                    MoveImage(&rect, 640, 246);
                }
                gCurrentTask->data1A += 0x80;
                gCourseObjectRotations[gCurrentTask->data18].vy += 0x80;
            }
        } else if (gCurrentTask->data1A != 0) {
            gCurrentTask->data1A -= 0x80;
            gCourseObjectRotations[gCurrentTask->data18].vy -= 0x80;
            if (gCurrentTask->data1A == 0) {
                rect.x = 640;
                rect.y = 255;
                rect.w = 16;
                rect.h = 1;
                MoveImage(&rect, 640, 246);
            }
        }
    }

    if (gSelectedCourseIndex != 9 && gCurrentTask->data18 == 3) {
        players = gRacePlayers;
        cameraPlayer = &players[getRenderCameraIndex()];
        records = gSelectedCourseRecords;
        if (cameraPlayer->unk92E < records[gSelectedCourseIndex].objectFirstSegment) {
            return;
        }
        if (players[getRenderCameraIndex()].unk92E > records[gSelectedCourseIndex].objectLastSegment) {
            return;
        }
    }

    cameraIndex = getRenderCameraIndex();
    coordinates = gCourseObjectCoordinates;
    objectIndex = gCurrentTask->data18;
    if (isPositionInCameraRange(coordinates[objectIndex].coord.t[0],
                                coordinates[objectIndex].coord.t[2], cameraIndex)) {
        objectIndex = gCurrentTask->data18;
        setCoordinateRotation(&gCourseObjectRotations[objectIndex], &coordinates[objectIndex]);
        drawTaskObjectAlternateLayer(gCurrentTask->data18);
    }
}

void initializeSelectedCourseOffsetTask(void) {
    CourseOffsetResource *resource;
    SelectedCourseRecord *records;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    records = gSelectedCourseRecords;
    resource->rotation.vx = 0;
    resource->rotation.vy = records[gSelectedCourseIndex].unk16;
    resource->rotation.vz = 0;
    gScratchShortVector1->vx = 0;
    gScratchShortVector1->vy = 0;
    gScratchShortVector1->vz = -0x330;
    RotMatrix(&resource->rotation, gScratchMatrix0);
    ApplyMatrix(gScratchMatrix0, gScratchShortVector1, gScratchVector0);
    resource->position.vx = records[gSelectedCourseIndex].unk18[3] + (gScratchVector0->vx << 8);
    resource->position.vy = records[gSelectedCourseIndex].unk18[4];
    resource->position.vz = records[gSelectedCourseIndex].unk18[5] + (gScratchVector0->vz << 8);
    if (gSelectedCourseIndex != 8) {
        gCurrentTask->data1C = 4;
    } else {
        gCurrentTask->data1C = 0x19;
    }
    gCurrentTask->data1E = 0x16;
    setCurrentTaskCallback(updateSelectedCourseOffsetTask);
}

void updateSelectedCourseOffsetTask(void) {
    CourseOffsetResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        RotMatrix(&resource->rotation, gScratchMatrix0);
        gScratchShortVector0->vx = 0;
        gScratchShortVector0->vy = 0;
        gScratchShortVector0->vz = 0x1000;
        ApplyMatrix(gScratchMatrix0, gScratchShortVector0, gScratchVector0);
        resource->position.vx += gScratchVector0->vx;
        resource->position.vz += gScratchVector0->vz;
        resource->coordinate.coord.t[0] = resource->position.vx >> 8;
        resource->coordinate.coord.t[1] = resource->position.vy >> 8;
        resource->coordinate.coord.t[2] = resource->position.vz >> 8;
        if (gCurrentTask->data1E != 0) {
            gCurrentTask->data1E--;
        }
    }
    if (gCurrentTask->data1E == 0) {
        if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
            gTaskObjectModels[gCurrentTask->data1C].coord2 = &resource->coordinate;
            setCoordinateRotation(&resource->rotation, &resource->coordinate);
            drawTaskObject(gCurrentTask->data1C);
        }
    }
    if (resource->linkedTask->data1A == 6) {
        setCurrentTaskCallback(updatePlayerFollowingOrbitTask);
    }
}

void updatePlayerFollowingOrbitTask(void) {
    CourseOffsetResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        if (gRacePlayers[gCurrentTask->data18].unk12C != 7) {
            gScratchShortVector1->vx = -100;
            gScratchShortVector1->vy = 0;
            gScratchShortVector1->vz = 0;
            RotMatrix(&resource->rotation, gScratchMatrix0);
            ApplyMatrix(gScratchMatrix0, gScratchShortVector1, gScratchVector0);
            resource->position.vx = resource->coordinate.coord.t[0] + gScratchVector0->vx;
            resource->position.vz = resource->coordinate.coord.t[2] + gScratchVector0->vz;
            gCurrentTask->data1A = 0x40;
            setCurrentTaskCallback(updateOrbitingTaskObject);
            updateOrbitingTaskObject();
            return;
        }
        resource->coordinate.coord.t[0] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[0];
        resource->coordinate.coord.t[1] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[1];
        resource->coordinate.coord.t[2] = gRacePlayers[gCurrentTask->data18].worldCoordinate.coord.t[2];
        if (resource->linkedTask->data1A == 7) {
            resource->rotation.vy = gRacePlayers[gCurrentTask->data18].unk110;
        }
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[gCurrentTask->data1C].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(gCurrentTask->data1C);
    }
}

void updateOrbitingTaskObject(void) {
    CourseOffsetResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        resource->rotation.vy -= 0x20;
        gScratchShortVector1->vx = 100;
        gScratchShortVector1->vy = 0;
        gScratchShortVector1->vz = 0;
        RotMatrix(&resource->rotation, gScratchMatrix0);
        ApplyMatrix(gScratchMatrix0, gScratchShortVector1, gScratchVector0);
        resource->coordinate.coord.t[0] = resource->position.vx + gScratchVector0->vx;
        resource->coordinate.coord.t[2] = resource->position.vz + gScratchVector0->vz;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[gCurrentTask->data1C].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(gCurrentTask->data1C);
    }
    if (!gRacePauseState) {
        if (--gCurrentTask->data1A == 0) {
            gCurrentTask->data1A = 0x40;
            setCurrentTaskCallback(updateMovingTaskObject);
        }
    }
}

void updateMovingTaskObject(void) {
    CourseOffsetResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        RotMatrix(&resource->rotation, gScratchMatrix0);
        gScratchShortVector0->vx = 0;
        gScratchShortVector0->vy = 0;
        gScratchShortVector0->vz = 8;
        ApplyMatrix(gScratchMatrix0, gScratchShortVector0, gScratchVector0);
        resource->coordinate.coord.t[0] += gScratchVector0->vx;
        resource->coordinate.coord.t[2] += gScratchVector0->vz;
        gCurrentTask->data1A--;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[gCurrentTask->data1C].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(gCurrentTask->data1C);
    }
    if (gCurrentTask->data1A == 0) {
        deleteCurrentTaskSubtree();
    }
}

void computeSelectedTriangleAngles(s16 *pitch, s16 *yaw) {
    VECTOR normal;
    s32 x, y, z;
    computeSelectedTriangleNormal(&normal);
    *yaw = getXZHeading(0, 0, normal.vx, normal.vy);
    x = normal.vx;
    y = normal.vy;
    z = normal.vz;
    x /= 16;
    y /= 16;
    z /= 16;
    *pitch = -getXZHeading(0, 0, z, SquareRoot0(x*x + y*y));
}

void updateCameraRotationMatrices(u16 playerIndex) {
    PlayerCamera *cameras = gPlayerCameras;
    gScratchShortVector0->vx = 0;
    gScratchShortVector0->vy = cameras[playerIndex].yaw;
    gScratchShortVector0->vz = 0;
    RotMatrix(gScratchShortVector0, &D_8012A6B0);
    gScratchShortVector0->vx = cameras[getRenderCameraIndex()].pitch;
    gScratchShortVector0->vy = cameras[getRenderCameraIndex()].yaw;
    gScratchShortVector0->vz = 0;
    RotMatrixYXZ(gScratchShortVector0, &D_8012A6D0);
}


void prepareTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frame, u8 usePitch) {
    if (usePitch == 0) {
        *D_8013AF9C = D_8012A6B0;
    } else {
        *D_8013AF9C = D_8012A6D0;
    }
    resource->frame = frame;
    ScaleMatrix(D_8013AF9C, (VECTOR *)&resource->scaleX);
    gTaskVisualQuad.r0 = resource->r0;
    gTaskVisualQuad.g0 = resource->g0;
    gTaskVisualQuad.b0 = resource->b0;
    gTaskVisualQuad.tpage = resource->frame->tpage;
}

void drawPreparedTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frames, u8 mode) {
    u16 i;
    POLY_FT4 *poly;
    long depthCue, flags;
    long depth;

    resource->frame = &frames[(u16)(resource->framePosition >> 8)];
    /* This texture-page bit reverses the quad horizontally. */
    if (resource->frame->tpage & 0x200) {
        resource->vertices[0].x = (resource->frame->width - 1) / 2;
        resource->vertices[0].y = -resource->frame->height;
        resource->vertices[0].z = 0;
        resource->vertices[1].x = (-resource->frame->width - 1) / 2;
        resource->vertices[1].y = -resource->frame->height;
        resource->vertices[1].z = 0;
        resource->vertices[2].x = (resource->frame->width - 1) / 2;
        resource->vertices[2].y = -1;
        resource->vertices[2].z = 0;
        resource->vertices[3].x = (-resource->frame->width - 1) / 2;
        resource->vertices[3].y = -1;
        resource->vertices[3].z = 0;
    } else {
        resource->vertices[0].x = (-resource->frame->width - 1) / 2;
        resource->vertices[0].y = -resource->frame->height;
        resource->vertices[0].z = 0;
        resource->vertices[1].x = (resource->frame->width - 1) / 2;
        resource->vertices[1].y = -resource->frame->height;
        resource->vertices[1].z = 0;
        resource->vertices[2].x = (-resource->frame->width - 1) / 2;
        resource->vertices[2].y = -1;
        resource->vertices[2].z = 0;
        resource->vertices[3].x = (resource->frame->width - 1) / 2;
        resource->vertices[3].y = -1;
        resource->vertices[3].z = 0;
    }

    *gScratchMatrix0 = *D_8013AF9C;
    for (i = 0; i < 4; i++) {
        ApplyMatrixSV(gScratchMatrix0, (SVECTOR *)&resource->vertices[i], (SVECTOR *)&resource->transformedVertices[i]);
    }
    resource->coordinate.flg = 0;
    GsGetLs(&resource->coordinate, gScratchMatrix0);
    GsSetLsMatrix(gScratchMatrix0);

    poly = &gTaskVisualQuad;
    poly->u0 = resource->frame->u;
    poly->v0 = resource->frame->v;
    poly->u1 = (u8)(resource->frame->u - 1) + resource->frame->width;
    poly->v1 = resource->frame->v;
    poly->u2 = resource->frame->u;
    poly->v2 = (u8)(resource->frame->v - 1) + resource->frame->height;
    poly->u3 = (u8)(resource->frame->u - 1) + resource->frame->width;
    poly->v3 = (u8)(resource->frame->v - 1) + resource->frame->height;
    setcode(poly, resource->code);
    poly->clut = resource->frame->clut;

    depth = RotTransPers4((SVECTOR *)&resource->transformedVertices[0],
                          (SVECTOR *)&resource->transformedVertices[1],
                          (SVECTOR *)&resource->transformedVertices[2],
                          (SVECTOR *)&resource->transformedVertices[3],
                          (long *)&poly->x0, (long *)&poly->x1,
                          (long *)&poly->x2, (long *)&poly->x3,
                          &depthCue, &flags);
    if (depth >= 0 && flags >= 0) {
        switch (mode) {
        case 0:
            addCopiedPrimitive(&gCurrentPlayerOrderingTable[depth >> 2], (u32 *)poly);
            break;
        case 1:
            addCopiedPrimitive(&D_8013AF70[depth >> 2], (u32 *)poly);
            break;
        case 2:
            addCopiedPrimitive(&D_8013AF74[depth >> 2], (u32 *)poly);
            break;
        }
    }
}

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

void drawCameraFacingTaskVisual(TaskVisualResource *resource, u8 mode) {
    u16 i;
    POLY_FT4 *poly;
    long depthCue, flags;
    long depth;
    if (mode == 0) {
        gScratchShortVector0->vx = 0;
        gScratchShortVector0->vy = gPlayerCameras[getRenderCameraIndex()].yaw;
        gScratchShortVector0->vz = 0;
    } else {
        gScratchShortVector0->vx = gPlayerCameras[getRenderCameraIndex()].pitch;
        gScratchShortVector0->vy = gPlayerCameras[getRenderCameraIndex()].yaw;
        gScratchShortVector0->vz = 0;
    }
    RotMatrixYXZ(gScratchShortVector0, gScratchMatrix0);
    ScaleMatrix(gScratchMatrix0, (VECTOR *)&resource->scaleX);
    for (i = 0; i < 4; i++) {
        ApplyMatrixSV(gScratchMatrix0, (SVECTOR *)&resource->vertices[i], (SVECTOR *)&resource->transformedVertices[i]);
    }
    resource->coordinate.flg = 0;
    GsGetLs(&resource->coordinate, gScratchMatrix0);
    GsSetLsMatrix(gScratchMatrix0);
    poly = &gTaskVisualQuad;
    poly->u0 = resource->frame->u;
    poly->v0 = resource->frame->v;
    poly->u1 = (u8)(resource->frame->u - 1) + resource->frame->width;
    poly->v1 = resource->frame->v;
    poly->u2 = resource->frame->u;
    poly->v2 = (u8)(resource->frame->v - 1) + resource->frame->height;
    poly->u3 = (u8)(resource->frame->u - 1) + resource->frame->width;
    poly->v3 = (u8)(resource->frame->v - 1) + resource->frame->height;
    setRGB0(poly, resource->r0, resource->g0, resource->b0);
    setcode(poly, resource->code);
    poly->tpage = resource->frame->tpage;
    poly->clut = resource->frame->clut;
    depth = RotTransPers4((SVECTOR *)&resource->transformedVertices[0], (SVECTOR *)&resource->transformedVertices[1], (SVECTOR *)&resource->transformedVertices[2], (SVECTOR *)&resource->transformedVertices[3], (long *)&poly->x0, (long *)&poly->x1, (long *)&poly->x2, (long *)&poly->x3, &depthCue, &flags);
    if (depth >= 0 && flags >= 0) {
        addCopiedPrimitive(&gCurrentPlayerOrderingTable[depth >> 2], (u32 *)poly);
    }
}

void initializePickupVisualTask(void) {
    TaskVisualResource *resource;
    u16 i;
    if ((s16)getModeFilteredGameOptionFlag2() == 0) {
        deleteCurrentTaskSubtree();
    } else {
        for (i = gPickupVisualPlacementStartIndex; i < gPickupVisualPlacementStartIndex + gPickupVisualPlacementCount; i++) {
            if (gCurrentCourseObjectPlacements[i].flags & 0x40) {
                gCurrentCourseObjectPlacements[i].flags &= 0xBF;
            }
        }
        if (gCurrentTask->resource14 != 0xFFFF) {
            while (1) {}
        }
        gCurrentTask->resource14 = allocateTaskMemoryBlock();
        resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->unkC6 = 0x600;
        resource->unkC8 = 0x80;
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->framePosition = 0;
        resource->scaleX = 0x1800;
        resource->scaleY = 0x1800;
        resource->scaleZ = 0x1800;
        setCurrentTaskCallback(updatePickupVisualTask);
    }
}

void updatePickupVisualTask(void) {
    TaskVisualResource *resource;
    u8 i, player, nearby;
    s32 height;
    s16 *heightOffsets;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    prepareTaskVisualQuad(resource, gPickupVisualFrames, 0);
    for (i = gPickupVisualPlacementStartIndex; i < gPickupVisualPlacementStartIndex + gPickupVisualPlacementCount; i++) {
        if (!(gCurrentCourseObjectPlacements[i].flags & 0x40)) {
            if (!isPointWithinCameraForwardRange(gCurrentCourseObjectPlacements[i].x, gCurrentCourseObjectPlacements[i].z, (s16)getRenderCameraIndex())) {
                continue;
            }
            resource->unkC4 = gCurrentCourseObjectPlacements[i].geometryIndex;
            resource->coordinate.coord.t[0] = gCurrentCourseObjectPlacements[i].x;
            resource->coordinate.coord.t[1] = gCurrentCourseObjectPlacements[i].y;
            resource->coordinate.coord.t[2] = gCurrentCourseObjectPlacements[i].z;
            height = gCurrentCourseObjectPlacements[i].flags & 0xF;
            if (height != 0) {
                heightOffsets = gVisualPlacementHeightData.offsets;
                resource->coordinate.coord.t[1] -= heightOffsets[height];
            }
            if (gRacePauseState == 0) {
                player = 0;
                resource->position.x = resource->coordinate.coord.t[0] << 8;
                resource->position.y = resource->coordinate.coord.t[1] << 8;
                resource->position.z = resource->coordinate.coord.t[2] << 8;
                for (; player < gActiveRacePlayerCount; player++) {
                    if (isPositionWithinPlayerCollisionRange(&resource->position, 0x1A00, 0x3800, player)) {
                        addPlayerCappedCounters(player, 100);
                        playPlayerSoundEffect(player, 0, 0x700, 0x20);
                        gCurrentCourseObjectPlacements[i].flags |= 0x40;
                        break;
                    }
                }
            }
            drawPreparedTaskVisualQuad(resource, gPickupVisualFrames, 1);
        } else {
            nearby = 0;
            for (player = 0; player < gActiveRacePlayerCount; player++) {
                resource->position.x = gCurrentCourseObjectPlacements[i].x << 8;
                resource->position.z = gCurrentCourseObjectPlacements[i].z << 8;
                nearby = arePositionsWithinXZRange(&resource->position, (const Position3D *)&gRacePlayers[player].x, 0x80000);
                if (nearby) {
                    break;
                }
            }
            if (!nearby) {
                gCurrentCourseObjectPlacements[i].flags &= 0xBF;
            }
        }
    }
    resource->framePosition += resource->unkC8;
    if (resource->framePosition >= resource->unkC6) {
        resource->framePosition &= 0xFF;
    }
}

void initializeObjectVisualTask(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x100;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->scaleX = 0x3800;
    resource->scaleY = 0x3800;
    resource->scaleZ = 0x3800;
    setCurrentTaskCallback(updateObjectVisualTask);
}

void updateObjectVisualTask(void) {
    TaskVisualResource *resource;
    u8 i;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    prepareTaskVisualQuad(resource, &gObjectVisualFrameType8, 0);
    for (i = gObjectVisualPlacementStartIndex; i < gObjectVisualPlacementStartIndex + gObjectVisualPlacementCount; i++) {
        if (isPointWithinCameraForwardRange(gCurrentCourseObjectPlacements[i].x, gCurrentCourseObjectPlacements[i].z, (s16)getRenderCameraIndex())) {
            resource->coordinate.coord.t[0] = gCurrentCourseObjectPlacements[i].x;
            resource->coordinate.coord.t[1] = gCurrentCourseObjectPlacements[i].y;
            resource->coordinate.coord.t[2] = gCurrentCourseObjectPlacements[i].z;
            if (gRacePauseState == 0) {
                resource->position.x = resource->coordinate.coord.t[0] << 8;
                resource->position.y = resource->coordinate.coord.t[1] << 8;
                resource->position.z = resource->coordinate.coord.t[2] << 8;
                if (gCurrentCourseObjectPlacements[i].unk01 == 8) {
                    resolvePlayerCollisionsWithCylinder(&resource->position, 0x4000, 0x12000, 2);
                } else {
                    resolvePlayerCollisionsWithCylinder(&resource->position, 0x2000, 0x12000, 2);
                }
            }
            drawPreparedTaskVisualQuad(resource, gObjectVisualFramesByType[gCurrentCourseObjectPlacements[i].unk01 - 8], 0);
        }
    }
}

void initializeInteractionVisualTask(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x100;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->scaleX = 0x2000;
    resource->scaleY = 0x2000;
    resource->scaleZ = 0x2000;
    setCurrentTaskCallback(updateInteractionVisualTask);
}

void updateInteractionVisualTask(void) {
    TaskVisualResource *resource;
    u8 i, player;
    s32 height;
    s16 *heightOffsets;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    prepareTaskVisualQuad(resource, &gInteractionVisualFrame, 1);
    for (i = 0; ; i++) {
        if (gInteractionVisualPlacements[i].unk01 == PLACEMENT_END) {
            return;
        }
        if (!(gInteractionVisualPlacements[i].flags & PLACEMENT_FLAG_COLLECTED)) {
            if (isPositionInCameraRange(gInteractionVisualPlacements[i].x, gInteractionVisualPlacements[i].z, (s16)getRenderCameraIndex())) {
                resource->unkC4 = gInteractionVisualPlacements[i].geometryIndex;
                resource->coordinate.coord.t[0] = gInteractionVisualPlacements[i].x;
                resource->coordinate.coord.t[1] = gInteractionVisualPlacements[i].y;
                resource->coordinate.coord.t[2] = gInteractionVisualPlacements[i].z;
                height = gInteractionVisualPlacements[i].flags & 0xF;
                if (height != 0) {
                    heightOffsets = gVisualPlacementHeightData.offsets;
                    resource->coordinate.coord.t[1] -= heightOffsets[height];
                }
                player = 0;
                if (gRacePauseState == 0) {
                    resource->position.x = resource->coordinate.coord.t[0] << 8;
                    resource->position.y = resource->coordinate.coord.t[1] << 8;
                    resource->position.z = resource->coordinate.coord.t[2] << 8;
                    for (; player < playerCountAbsolute; player++) {
                        if (isPositionWithinPlayerCollisionRange(&resource->position, 0x1C00, 0x5000, player)) {
                            gRacePlayers[player].collectedCourseCoinMarkerCount++;
                            if (gRacePlayers[player].unk0FC != 0) {
                                gRacePlayers[player].unk0FC = 0xF0;
                            } else {
                                startPlayerTimerTask(&gRacePlayers[player]);
                            }
                            queuePlayerControllerEffect(gRacePlayers[player].playerIndex, 0xD);
                            playSoundEffect(0, 0, 0x20);
                            gInteractionVisualPlacements[i].flags |= PLACEMENT_FLAG_COLLECTED;
                            break;
                        }
                    }
                }
                drawPreparedTaskVisualQuad(resource, &gInteractionVisualFrame, 0);
            }
        }
    }
}

void initializeCourseVisualTask(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    if (gSelectedCourseIndex == 2) {
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
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    prepareTaskVisualQuad(resource, gCourseVisualFrames, 0);
    for (i = gCourseVisualPlacementStartIndex; i < gCourseVisualPlacementStartIndex + gCourseVisualPlacementCount; i++) {
        if (isPointWithinCameraForwardRange(gCurrentCourseObjectPlacements[i].x, gCurrentCourseObjectPlacements[i].z, (s16)getRenderCameraIndex())) {
            resource->coordinate.coord.t[0] = gCurrentCourseObjectPlacements[i].x;
            resource->coordinate.coord.t[1] = gCurrentCourseObjectPlacements[i].y;
            resource->coordinate.coord.t[2] = gCurrentCourseObjectPlacements[i].z;
            height = gCurrentCourseObjectPlacements[i].flags & 0xF;
            if (height != 0) {
                heightOffsets = gVisualPlacementHeightData.offsets;
                resource->coordinate.coord.t[1] -= heightOffsets[height];
            }
            drawPreparedTaskVisualQuad(resource, D_80129540[gCurrentCourseObjectPlacements[i].unk01], 2);
        }
    }
    if (gRacePauseState == 0) {
        if (gSelectedCourseIndex == 9) resource->framePosition += resource->unkC8;
        if (resource->framePosition >= resource->unkC6) resource->framePosition &= 0xFF;
    }
}

void initializePlacedObjectTask(void) {
    PlacedObjectResource *resource;
    TaskVisualResource *visual;
    if ((s16)getStateAdjustedGameOptionFlag1() == 0) {
        if ((u32)(gCurrentCourseObjectPlacements[gCurrentTask->data18].unk01 - 2) < 2) {
            deleteCurrentTaskSubtree();
            return;
        }
    }
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlacedObjectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->groundCoordinate);
    GsInitCoordinate2(&resource->groundCoordinate, &resource->coordinate);
    gCurrentTask->data1A = gCurrentCourseObjectPlacements[gCurrentTask->data18].unk01;
    resource->geometryIndex = gCurrentCourseObjectPlacements[gCurrentTask->data18].geometryIndex;
    resource->rotation.vx = 0;
    resource->rotation.vy = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle;
    resource->rotation.vz = 0;
    resource->groundCoordinate.coord.t[0] = gCurrentCourseObjectPlacements[gCurrentTask->data18].x;
    resource->groundCoordinate.coord.t[2] = gCurrentCourseObjectPlacements[gCurrentTask->data18].z;
    resource->groundCoordinate.coord.t[1] = gCurrentCourseObjectPlacements[gCurrentTask->data18].y =
        getCourseHeightAtPoint(resource->geometryIndex, resource->groundCoordinate.coord.t[0],
                      resource->groundCoordinate.coord.t[2]);
    resource->position.x = resource->groundCoordinate.coord.t[0] << 8;
    resource->position.y = resource->groundCoordinate.coord.t[1] << 8;
    resource->position.z = resource->groundCoordinate.coord.t[2] << 8;
    computeSelectedTriangleAngles(&resource->groundRotation.vx, &resource->groundRotation.vz);
    resource->groundRotation.vy = 0;
    if ((u32)(gCurrentTask->data1A - 2) < 2) {
        if (gCurrentTask->resource16 == 0xFFFF) {
            while (1) {}
        }
        visual = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource16];
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
    setCurrentTaskCallback(updatePlacedObjectTask);
}

void updatePlacedObjectTask(void) {
    PlacedObjectResource *resource;
    PlacedObjectBounceVisual *visual;
    RacePlayer *player;
    RacePlayer *buyer;
    RacePlayer *itemPlayer;
    RacePlayer *players;
    u16 model;
    u16 i;
    u16 blocked;
    s32 height;
    s32 balance;
    u8 reward;
    u8 uses;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlacedObjectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (isPositionInCameraRange(resource->groundCoordinate.coord.t[0],
                                resource->groundCoordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        if ((u32)(gCurrentTask->data1A - 2) < 2) {
            if (gRacePlayers[getRenderCameraIndex()].unk01C >= 100) {
                if (gCurrentTask->data1A == 2) {
                    model = 17;
                } else {
                    model = 16;
                }
                height = resource->groundCoordinate.coord.t[1];
                gTaskObjectModels[model].coord2 = &resource->coordinate;
                resource->groundCoordinate.coord.t[1] += -0x100 + (rsin((gFrameCounter & 31) << 7) >> 8);
                gScratchShortVector0->vx = 0;
                gScratchShortVector0->vy = 0;
                gScratchShortVector0->vz = 0;
                setCoordinateRotation(gScratchShortVector0, &resource->groundCoordinate);
                setCoordinateRotation(&resource->rotation, &resource->coordinate);
                gScratchVector0->vx = 0x2000;
                gScratchVector0->vy = 0x2000;
                gScratchVector0->vz = 0x2000;
                ScaleMatrix(&resource->coordinate.coord, gScratchVector0);
                drawTaskObject(model);
                resource->groundCoordinate.coord.t[1] = height;
            }
            if (gCurrentTask->resource16 == 0xFFFF) {
                while (1) {}
            }
            drawCameraFacingTaskVisual((TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource16], 0);
        }
        gTaskObjectModels[gPlacedObjectModelIds[gCurrentTask->data1A]].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->groundRotation, &resource->groundCoordinate);
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        if (gCurrentTask->data1A == 5) {
            gScratchVector0->vx = 0x3000;
            gScratchVector0->vy = 0x3000;
            gScratchVector0->vz = 0x3000;
            ScaleMatrix(&resource->coordinate.coord, gScratchVector0);
        }
        drawTaskObject(gPlacedObjectModelIds[gCurrentTask->data1A]);
    }
    if (!gRacePauseState) {
        switch (gCurrentTask->data1A) {
        case 0:
            resolvePlayerCollisionsWithCylinder(&resource->position, 0x6000, 0x10000, 2);
            break;
        case 1:
            for (i = 0; i < gActiveRacePlayerCount; i++) {
                addPlayerFlagsIfWithinRadius(&resource->position, 0xC00, 4, i);
            }
            break;
        case 2:
        case 3:
            blocked = 0;
            i = 0;
            players = gRacePlayers;
            uses = 4;
        check_player:
            if (i < gActiveRacePlayerCount) {
                if (isPositionWithinPlayerCollisionRange(&resource->position, 0x2FC0, 0x5C00, (s16)i)) {
                    buyer = (RacePlayer *)(i * sizeof(*players) + (u32)players);
                    balance = buyer->unk01C;
                    blocked = 1;
                    if (balance < 100) {
                        if (!buyer->unk008) {
                            goto next_player;
                        }
                        buyer->unk01C = 0;
                    } else {
                        buyer->unk01C = balance - 100;
                    }
                    playPlayerSoundEffect(i, 1, 0x901, 0x20);
                    if (gCurrentTask->data1A == 2) {
                        player = (RacePlayer *)(i * sizeof(*players) + (u32)players);
                        reward = gPlacedObjectEffectChoices[player->unk935][randomNextObject(player) & 15];
                        player->unk940 = 3;
                        player->unk93F = uses;
                        player->unk93E = reward;
                    } else {
                        itemPlayer = (RacePlayer *)(i * sizeof(*players) + (u32)players);
                        itemPlayer->heldItem = gPlacedObjectItemChoices[itemPlayer->unk935][randomNextObject(itemPlayer) & 15];
                        if (gModeSequenceStage) {
                            itemPlayer->heldItem = 1;
                        }
                        if (gSelectedCourseIndex == 8 && itemPlayer->unk008 &&
                            (s8)itemPlayer->heldItem == uses && (u8)randomNextMain()) {
                            itemPlayer->heldItem = 6;
                        }
                        players[i].unk942 = 4;
                    }
                    createPositionedEffectTask(resource->position.x, resource->position.y, resource->position.z);
                    gCurrentTask->data1C = 0;
                    if (gCurrentTask->resource16 == 0xFFFF) {
                        while (1) {}
                    }
                    visual = (PlacedObjectBounceVisual *)&gTaskMemoryPool[gCurrentTask->resource16];
                    visual->velocity = -0x800;
                    visual->height = -0x5000;
                    setCurrentTaskCallback(updatePlacedObjectBounceTask);
                    return;
                }
            next_player:
                i++;
                goto check_player;
            }
            if (blocked) {
                resolvePlayerCollisionsWithCylinder(&resource->position, 0x2FC0, 0x5C00, 0x800);
            }
            break;
        case 5:
            resolvePlayerCollisionsWithCylinder(&resource->position, 0x6800, 0x18000, 2);
            break;
        }
    }
}

void updatePlacedObjectBounceTask(void) {
    PlacedObjectResource *resource;
    PlacedObjectBounceVisual *visual;
    u16 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (PlacedObjectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { while (1) {} }
    visual = (PlacedObjectBounceVisual *)&gTaskMemoryPool[gCurrentTask->resource16];
    if (!gRacePauseState) {
        visual->height += visual->velocity;
        switch (gCurrentTask->data1C) {
        case 0: case 1:
            visual->velocity += 0x140;
            if (visual->height >= 0) {
                visual->height = 0;
                if (gCurrentTask->data1C == 0) visual->velocity = -0x600;
                else { visual->velocity = 0; gCurrentTask->data1E = 4; }
                gCurrentTask->data1C++;
            }
            break;
        case 2:
            if (--gCurrentTask->data1E == 0) {
                visual->velocity = -0x500;
                startRotatingQuadEffectTask(gCurrentTask->data1A - 2, (VECTOR *)&resource->position, resource->groundRotation.vx, resource->rotation.vy, resource->groundRotation.vz);
                gCurrentTask->data1E = 16;
                gCurrentTask->data1C++;
            }
            break;
        case 3:
            if (--gCurrentTask->data1E == 0) gCurrentTask->data1C++;
            break;
        }
        if (gCurrentTask->data1C != 0 || visual->velocity > 0) {
            for (i = 0; i < gActiveRacePlayerCount; i++) {
                pushPlayerOutOfCylinder(&resource->position, 0x3000, 0x6000, (s16)i);
            }
        }
    }
    visual->coordinate.coord.t[1] = resource->groundCoordinate.coord.t[1] + (visual->height >> 8);
    drawCameraFacingTaskVisual((TaskVisualResource *)visual, 0);
    if (gCurrentTask->data1C == 4) setCurrentTaskCallback(updatePlacedObjectTask);
}

void initializeBobbingTrailEmitterTask(void) {
    BobbingTrailEmitterResource *resource;
    u16 angle;

    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (BobbingTrailEmitterResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);

    resource->geometryIndex = 0x59;
    resource->rotation.vx = 0;
    angle = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle;
    resource->rotation.vz = 0;
    resource->coordinate.coord.t[0] = 0x8AF7;
    resource->coordinate.coord.t[1] = 0xB658;
    resource->coordinate.coord.t[2] = 0xE7C2;
    resource->countdown = 15;
    resource->headingStep = 8;
    resource->phase = 0;
    resource->rotation.vy = angle;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->baseHeading.value = resource->rotation.vy;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    resource->heading.value = resource->baseHeading.value;
    setCurrentTaskCallback(updateBobbingTrailEmitterTask);
}

void updateBobbingTrailEmitterTask(void) {
    BobbingTrailEmitterResource *resource;
    EmitterTrailResource *trail;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (BobbingTrailEmitterResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        if (!gRacePauseState) {
            resolvePlayerCollisionsWithCylinder(&resource->position, 0x7000, 0x18000, 2);
            resource->heading.value += resource->headingStep;
            if (resource->heading.halves.low > resource->baseHeading.halves.low + 0x140) resource->headingStep = -8;
            if (resource->heading.halves.low < resource->baseHeading.halves.low - 0x140) resource->headingStep = 8;
            resource->phase += 0x40;
            resource->rotation.vy = resource->heading.halves.low;
            resource->coordinate.coord.t[1] = 0xB654 - (rsin(resource->phase) >> 9);
            if (--resource->countdown == 0) {
                resource->countdown = 15;
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xA00, 0x18);
                if ((s16)tryCreateChildTaskWith20SlotsReserved(initializeVisualTrailTask) == 1) {
                    gCreatedTask->data18 = 0xFF;
                    gCreatedTask->resource14 = allocateTaskMemoryBlock();
                    trail = (EmitterTrailResource *)&gTaskMemoryPool[gCreatedTask->resource14];
                    GsInitCoordinate2(0, &trail->coordinate);
                    trail->geometryIndex = resource->geometryIndex;
                    trail->coordinate.coord.t[0] = resource->coordinate.coord.t[0];
                    trail->coordinate.coord.t[1] = resource->coordinate.coord.t[1] - 0x100;
                    trail->coordinate.coord.t[2] = resource->coordinate.coord.t[2];
                    trail->angle = (resource->heading.value + 0x800) & 0xFFF;
                }
            }
        }
        gTaskObjectModels[10].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        gScratchVector0->vx = 0x3000;
        gScratchVector0->vy = 0x3000;
        gScratchVector0->vz = 0x3000;
        ScaleMatrix(&resource->coordinate.coord, gScratchVector0);
        drawTaskObject(10);
    }
}

void initializeProximityEmitterTask(void) {
    ProximityEmitterResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (ProximityEmitterResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->coordinate.coord.t[0] = gProximityEmitterPlacements[gCurrentTask->data18].x;
    resource->coordinate.coord.t[1] = gProximityEmitterPlacements[gCurrentTask->data18].y;
    resource->coordinate.coord.t[2] = gProximityEmitterPlacements[gCurrentTask->data18].z;
    resource->geometryIndex = gProximityEmitterPlacements[gCurrentTask->data18].geometryIndex;
    resource->unkC0 = 0;
    resource->angle = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle + 0x400;
    resource->unkC4 = 0;
    resource->countdown = 1;
    setCurrentTaskCallback(updateProximityEmitterTask);
}

void updateProximityEmitterTask(void) {
    ProximityEmitterResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ProximityEmitterResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
            if (--resource->countdown == 0) {
                resource->countdown = 0x20;
                tryCreateChildTaskWith20SlotsReserved(initializeProximityParticleTask);
            }
        }
    }
}

void initializeProximityParticleTask(void) {
    ProximityParticleResource *resource;
    ProximityEmitterResource *parent;
    s32 xOffset, zOffset;
    s32 sine, cosine;
    if (gCurrentTask->resource14 != 0xFFFF) { while (1) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (ProximityParticleResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTaskParent->resource14 == 0xFFFF) { while (1) {} }
    parent = (ProximityEmitterResource *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->geometryIndex = parent->geometryIndex;
    resource->rotation.vx = 0;
    resource->rotation.vy = parent->angle;
    resource->rotation.vz = 0;
    xOffset = ((u8)((u8)randomNextSecondary() % 60) - 30) * 10;
    zOffset = ((u8)((u8)randomNextSecondary() % 60) - 30) * 10;
    resource->position.x = (parent->coordinate.coord.t[0] + xOffset) << 8;
    resource->position.y = parent->coordinate.coord.t[1] << 8;
    resource->position.z = (parent->coordinate.coord.t[2] + zOffset) << 8;
    sine = rsin(resource->rotation.vy);
    cosine = rcos(resource->rotation.vy);
    resource->velocityX = (sine * 2560) / 4096;
    resource->velocityZ = (cosine * 2560) / 4096;
    resource->velocityY = -4096;
    resource->gravity = 80;
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 90;
    setCurrentTaskCallback(updateProximityParticleTask);
}

void updateProximityParticleTask(void) {
    ProximityParticleResource *resource;
    s32 groundY;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ProximityParticleResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        resource->rotation.vx += 0x40;
        resource->velocityY += resource->gravity;
        resource->position.x += resource->velocityX;
        resource->position.z += resource->velocityZ;
        resource->position.y += resource->velocityY;
        resource->coordinate.coord.t[0] = resource->position.x >> 8;
        resource->coordinate.coord.t[1] = resource->position.y >> 8;
        resource->coordinate.coord.t[2] = resource->position.z >> 8;

        if (gCurrentTask->data1A == 0) {
            if (resource->velocityY > 0) {
                resource->geometryIndex = getAdjacentGeometryRegionForPoint(resource->geometryIndex,
                    resource->coordinate.coord.t[0], resource->coordinate.coord.t[2]);
                groundY = getCourseHeightAtPoint(resource->geometryIndex, resource->coordinate.coord.t[0],
                    resource->coordinate.coord.t[2]) - 0x10;
                if (resource->coordinate.coord.t[1] > groundY) {
                    if (!gCurrentTask->data18 || !getCourseSurfaceAtPoint(resource->geometryIndex,
                            resource->coordinate.coord.t[0], resource->coordinate.coord.t[2])) {
                        resource->coordinate.coord.t[1] = groundY;
                        resource->position.y = groundY << 8;
                        resource->velocityY = -(resource->velocityY * 3 / 5);
                        playPositionalSoundEffect(resource->position.x, resource->position.y,
                            resource->position.z, 1, 0xA01, 0x20);
                        gCurrentTask->data18 = 1;
                        gCurrentTask->data1C = 6;
                    } else {
                        deleteCurrentTaskSubtree();
                        return;
                    }
                }
            }
        } else {
            gCurrentTask->data1A--;
        }
    }

    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0],
            resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        if (!gRacePauseState) {
            if (resource->velocityY > 0) {
                addPlayerFlagsWithinRadius(&resource->position, 0x5C00, 0x20);
            }
            if (gCurrentTask->data18) {
                if (gCurrentTask->data1C) {
                    resolvePlayerCollisionsWithCylinder(&resource->position, 0x4000, 0x6000, 2);
                    gCurrentTask->data1C--;
                }
            }
        }
        gTaskObjectModels[7].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(7);
    }
}

void initializeMovingVisualTask(void) {
    TaskVisualResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x800;
    resource->unkC8 = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG;
    resource->framePosition = 0;
    resource->scaleX = 0x1000;
    resource->scaleY = 0x1000;
    resource->scaleZ = 0x1000;
    resource->unkC4 = gMovingVisualPlacements[gCurrentTask->data18].geometryIndex;
    resource->position.x = gMovingVisualPlacements[gCurrentTask->data18].startX;
    resource->position.z = gMovingVisualPlacements[gCurrentTask->data18].startZ;
    resource->velocityY = getXZHeading(resource->position.x, resource->position.z,
        gMovingVisualPlacements[gCurrentTask->data18].endX, gMovingVisualPlacements[gCurrentTask->data18].endZ);
    resource->unkDC = 0x10;
    resource->unkE0 = 0x80;
    resource->unkD8 = 0;
    resource->bounceCount = 0x400;
    resource->groundY = 0;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
    resource->position.y = getCourseHeightAtPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    setCurrentTaskCallback(updateMovingVisualTask);
}

/* For this task, groundY selects the return endpoint, velocityY holds the
 * heading, and bounceCount holds the movement speed. */
void updateMovingVisualTask(void) {
    TaskVisualResource *resource;
    u16 cameraIndex;
    s16 turn;
    s32 dx, dz;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    cameraIndex = getRenderCameraIndex();
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0],
            resource->coordinate.coord.t[2], (s16)cameraIndex)) {
        if (!gRacePauseState) {
            if (resource->groundY) {
                turn = getXZHeading(resource->position.x, resource->position.z,
                    gMovingVisualPlacements[gCurrentTask->data18].startX,
                    gMovingVisualPlacements[gCurrentTask->data18].startZ);
            } else {
                turn = getXZHeading(resource->position.x, resource->position.z,
                    gMovingVisualPlacements[gCurrentTask->data18].endX,
                    gMovingVisualPlacements[gCurrentTask->data18].endZ);
            }
            turn = (turn - resource->velocityY) & 0xFFF;
            if (turn > 0x800) {
                turn |= 0xF000;
            }
            if (turn > resource->unkDC) {
                turn = resource->unkDC;
            }
            if (turn < -resource->unkDC) {
                turn = -resource->unkDC;
            }
            resource->velocityY += turn;
            resource->position.x += rsin(resource->velocityY) * resource->bounceCount / 0x1000;
            resource->position.z += rcos(resource->velocityY) * resource->bounceCount / 0x1000;
            resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4,
                resource->position.x >> 8, resource->position.z >> 8);
            resource->position.y = getCourseHeightAtPoint(resource->unkC4,
                resource->position.x >> 8, resource->position.z >> 8) << 8;

            if (resource->groundY) {
                dx = resource->position.x - gMovingVisualPlacements[gCurrentTask->data18].startX;
                dz = resource->position.z - gMovingVisualPlacements[gCurrentTask->data18].startZ;
            } else {
                dx = resource->position.x - gMovingVisualPlacements[gCurrentTask->data18].endX;
                dz = resource->position.z - gMovingVisualPlacements[gCurrentTask->data18].endZ;
            }
            if (dx < 0) {
                dx = -dx;
            }
            if (dz < 0) {
                dz = -dz;
            }
            if (dx < 0xC00 && dz < 0xC00) {
                resource->groundY ^= 1;
            }

            resolvePlayerCylinderCollisionsBySpeed(&resource->position, 0x1000, 0xC00, 0x1000, 4);
            resource->unkD8 = (resource->unkD8 + resource->unkE0) & 0xFFFF;
            if (resource->unkD8 == 0) {
                switch ((u8)((u8)randomNextSecondary() % 3U)) {
                case 0:
                    resource->bounceCount = 0x400;
                    resource->unkE0 = 0x80;
                    resource->unkDC = 0x10;
                    resource->unkC8 = 0x80;
                    break;
                case 1:
                    resource->bounceCount = 0x800;
                    resource->unkE0 = 0x100;
                    resource->unkDC = 0x20;
                    resource->unkC8 = 0x100;
                    break;
                case 2:
                    resource->bounceCount = 0x1000;
                    resource->unkE0 = 0x200;
                    resource->unkDC = 0x40;
                    resource->unkC8 = 0x200;
                    break;
                }
            }
            resource->framePosition += resource->unkC8;
            if (resource->framePosition >= resource->unkC6) {
                resource->framePosition &= 0xFF;
            }
            resource->coordinate.coord.t[0] = resource->position.x >> 8;
            resource->coordinate.coord.t[1] = resource->position.y >> 8;
            resource->coordinate.coord.t[2] = resource->position.z >> 8;
        }

        prepareTaskVisualQuad(resource, gMovingVisualFrames[0], 0);
        drawPreparedTaskVisualQuad(resource,
            gMovingVisualFrames[((resource->velocityY - gPlayerCameras[cameraIndex].yaw + 0x100) & 0xFFF) / 0x200], 0);
    }
}

void resolvePlayerRegionCollision(RacePlayer *player, PlayerCollisionRegion *region) {
    s32 dx, dz;
    s32 lateral, vertical, correction;
    s32 localX, localY;
    s16 height;
    s32 courseHeight, slopeHeight;

    if (gRacePauseState) {
        return;
    }
    if (player->unk12C == 0x1D) {
        player->unk128 |= 0x2000000;
        gScratchShortVector0->vx = region->pitch;
        gScratchShortVector0->vy = -region->yaw;
        gScratchShortVector0->vz = 0;
        RotMatrixYXZ(gScratchShortVector0, gScratchMatrix0);
        gScratchVector0->vx = player->x - region->endpoints[0].position.x;
        gScratchVector0->vy = 0;
        gScratchVector0->vz = player->z - region->endpoints[0].position.z;
        ApplyMatrixLV(gScratchMatrix0, gScratchVector0, gScratchVector1);
        if (gScratchVector1->vx != 0) {
            gScratchShortVector0->vx = 0;
            gScratchShortVector0->vy = region->yaw;
            gScratchShortVector0->vz = 0;
            RotMatrix(gScratchShortVector0, gScratchMatrix0);
            gScratchShortVector0->vx = -gScratchVector1->vx;
            gScratchShortVector0->vy = 0;
            gScratchShortVector0->vz = 0;
            ApplyMatrix(gScratchMatrix0, gScratchShortVector0, gScratchVector0);
            player->x += gScratchVector0->vx;
            player->z += gScratchVector0->vz;
        }
        gScratchShortVector0->vx = region->pitch;
        gScratchShortVector0->vy = -region->yaw;
        gScratchShortVector0->vz = 0;
        RotMatrixYXZ(gScratchShortVector0, gScratchMatrix0);
        dx = (player->x - region->endpoints[0].position.x) >> 5;
        dz = (player->z - region->endpoints[0].position.z) >> 5;
        gScratchVector0->vx = 0;
        gScratchVector0->vy = 0;
        gScratchVector0->vz = SquareRoot0(dx * dx + dz * dz) << 5;
        ApplyMatrixLV(gScratchMatrix0, gScratchVector0, gScratchVector1);
        slopeHeight = gScratchVector1->vy + 0x3000;
        player->y = region->endpoints[0].position.y - slopeHeight;
        courseHeight = player->unk92E;
        player->worldCoordinate.coord.t[1] = player->y >> 8;
        if (region->maxHeight < courseHeight) {
            if (gMainMenuSelectionIndex == 0) {
                playSoundEffect(0, 0x501, 0x40);
                addPlayerCappedCounters(player->playerIndex, 300);
                createActivePlayerSlidingNumberTask(300, player->playerIndex);
            }
            if (gMainMenuSelectionIndex == 1 && gRaceRuleset == 2) {
                playSoundEffect(0, 0x501, 0x40);
                player->trickAttackPointTotal *= 2;
                createTrickAttackMultiplierDisplayTask(player->playerIndex);
            }
            player->unk12C = 0x16;
        }
    } else {
        pushPlayerOutOfCylinder(&region->endpoints[0].position, 0xC00, 0x4000, player->playerIndex);
        pushPlayerOutOfCylinder(&region->endpoints[1].position, 0xC00, 0x4000, player->playerIndex);
        height = player->unk92E;
        if (height < region->minHeight || height > region->maxHeight) {
            return;
        }
        gScratchShortVector0->vx = region->pitch;
        gScratchShortVector0->vy = -region->yaw;
        gScratchShortVector0->vz = 0;
        RotMatrixYXZ(gScratchShortVector0, gScratchMatrix0);
        gScratchVector0->vx = player->x - region->endpoints[0].position.x;
        gScratchVector0->vy = player->y - region->endpoints[0].position.y - 0x2800;
        gScratchVector0->vz = player->z - region->endpoints[0].position.z;
        ApplyMatrixLV(gScratchMatrix0, gScratchVector0, gScratchVector1);
        localX = gScratchVector1->vx;
        if (localX < -0x1C00 - player->unk0AC || localX > player->unk0AC + 0x1C00) {
            return;
        }
        localY = gScratchVector1->vy;
        if (localY > 0) {
            return;
        }
        if (localY < -0x4800) {
            return;
        }
        if ((player->unk128 & 0x201) && !(player->unk128 & 0x2800) &&
            player->unk062 > 0 && localY < -0x2000 &&
            localX > -0x1C00 && localX < 0x1C00) {
            player->unk128 |= 0x2000000;
            player->unk12C = 0x1D;
            player->unk138 = region->yaw;
            player->unk13A = region->pitch;
            return;
        }
        lateral = gScratchVector1->vx;
        correction = 0;
        if (lateral >= 0) {
            s32 margin = player->unk0AC;
            if (lateral < margin + 0x1C00) {
                s32 edgeDistance = lateral - 0x1C00;
                correction = margin - edgeDistance;
            }
        } else if (lateral > -(player->unk0AC + 0x1C00)) {
            correction = -(player->unk0AC + 0x1C00) - lateral;
        }
        if (correction != 0) {
            gScratchShortVector0->vx = 0;
            gScratchShortVector0->vy = region->yaw;
            gScratchShortVector0->vz = 0;
            RotMatrix(gScratchShortVector0, gScratchMatrix0);
            gScratchShortVector0->vx = correction;
            gScratchShortVector0->vy = 0;
            gScratchShortVector0->vz = 0;
            ApplyMatrix(gScratchMatrix0, gScratchShortVector0, gScratchVector0);
            player->x += gScratchVector0->vx;
            player->z += gScratchVector0->vz;
        }
    }
}

void updateActivePlayerRegionCollisions(void) {
    u16 i;
    PlayerCollisionRegion *region;
    RacePlayer *players;
    if (!gRacePauseState) {
        i = 0;
        region = &D_80129A18[gCurrentTask->data18];
        players = gRacePlayers;
        do {
            if (players[i].isActive != 0) {
                resolvePlayerRegionCollision(&players[i], region);
            }
            i++;
        } while (i < 4);
    }
}

void initializeCollectibleTask(void) {
    PickupResource *resource;
    s32 height;
    s16 *heightOffsets;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (PickupResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->geometryIndex = gCollectiblePlacements[gCurrentTask->data18].geometryIndex;
    resource->rotation.vx = 0;
    resource->rotation.vy = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle;
    resource->rotation.vz = 0;
    resource->coordinate.coord.t[0] = gCollectiblePlacements[gCurrentTask->data18].x;
    resource->coordinate.coord.t[2] = gCollectiblePlacements[gCurrentTask->data18].z;
    height = getCourseHeightAtPoint(resource->geometryIndex, resource->coordinate.coord.t[0], resource->coordinate.coord.t[2]);
    gCollectiblePlacements[gCurrentTask->data18].y = height;
    resource->coordinate.coord.t[1] = height;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    if (playerCountAbsolute == 1) {
        resource->collisionRange = 0x2C00;
    } else {
        resource->collisionRange = 0x3400;
    }
    setCurrentTaskCallback(updateCollectibleTask);
}

void updateCollectibleTask(void) {
    PickupResource *resource;
    u16 i;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (PickupResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[10].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(10);
        if (!gRacePauseState) {
            resolvePlayerCollisionsWithCylinder(&resource->position, 0x2800, 0x8000, 2);
            for (i = 0; i < gPlayerCollectionEntryCount; i++) {
                if (arePositionsWithinXZRange(&resource->position, &gPlayerCollectionEntries[i].location.player.position, resource->collisionRange)) {
                    *gPlayerCollectionEntries[i].active = 0;
                    gRacePlayers[gPlayerCollectionEntries[i].location.player.playerIndex].scoreAttackPointTotal++;
                    playSoundEffect(1, 0xF00, 0x20);
                    createNineIndexedChildTasks(0x15, 1);
                    gCurrentTask->callback = deleteCurrentTask;
                    gCurrentTask->unk10 |= 1;
                    break;
                }
            }
        }
    }
}

void initializeFallingTaskObject(void) {
    FallingTaskResource *resource;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (FallingTaskResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    gCurrentTask->data1A = gCurrentCourseObjectPlacements[gCurrentTask->data18].unk01;
    resource->geometryIndex = gRacePlayers[gCurrentTask->data18].unk92E;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->position.x = gRacePlayers[gCurrentTask->data18].x;
    resource->position.y = gRacePlayers[gCurrentTask->data18].y - 0x2000;
    resource->position.z = gRacePlayers[gCurrentTask->data18].z;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    resource->verticalVelocity = -0xA00;
    resource->groundHeight = getCourseHeightAtPoint(resource->geometryIndex, resource->position.x >> 8, resource->position.z >> 8) << 8;
    computeSelectedTriangleAngles(&resource->groundPitch, &resource->groundRoll);
    setCurrentTaskCallback(updateFallingTaskObject);
}

void updateFallingTaskObject(void) {
    FallingTaskResource *resource;
    u8 landed = 0;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingTaskResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
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
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[9].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(9);
    }
    if (landed) {
        gCurrentTask->data1A = 0x708;
        setCurrentTaskCallback(updateLandedTaskObject);
    }
}

void updateLandedTaskObject(void) {
    FallingTaskResource *resource;
    u16 i;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingTaskResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (!gRacePauseState) {
        if (--gCurrentTask->data1A == 0) {
            deleteCurrentTaskSubtree();
            return;
        }
        for (i = 0; i < gActiveRacePlayerCount; i++) {
            if (addPlayerFlagsIfWithinRadius(&resource->position, 0xC00, 4, i)) {
                deleteCurrentTaskSubtree();
                return;
            }
        }
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        gTaskObjectModels[9].coord2 = &resource->coordinate;
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        drawTaskObject(9);
    }
}

void initializeOrbitingTaskObject(void) {
    OrbitingTaskResource *resource;
    u16 i;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (OrbitingTaskResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->groundCoordinate);
    GsInitCoordinate2(&resource->groundCoordinate, &resource->coordinate);
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->phase = (0x1000 / gCurrentTask->data1A) * gCurrentTask->data1C;
    resource->geometryIndex = gOrbitingTaskPlacements[gCurrentTask->data18].geometryIndex;
    resource->groundCoordinate.coord.t[0] = gOrbitingTaskPlacements[gCurrentTask->data18].x;
    resource->groundCoordinate.coord.t[2] = gOrbitingTaskPlacements[gCurrentTask->data18].z;
    resource->groundCoordinate.coord.t[1] = getCourseHeightAtPoint(resource->geometryIndex,
        resource->groundCoordinate.coord.t[0], resource->groundCoordinate.coord.t[2]);
    computeSelectedTriangleAngles(&resource->groundRotation.vx, &resource->groundRotation.vz);
    resource->groundRotation.vy = 0;
    for (i = 0; i < gActiveRacePlayerCount; i++) {
        resource->playerCooldowns[i] = 0;
    }
    setCurrentTaskCallback(updateOrbitingObstacleTask);
}

void updateOrbitingObstacleTask(void) {
    OrbitingTaskResource *resource;
    u8 active;
    u8 i;
    s32 x, y, z;
    u16 trackHeading;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    active = 0;
    i = active;
    resource = (OrbitingTaskResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (playerCountAbsolute != 0) {
        do {
            if ((u32)((u16)gRacePlayers[i].unk92E - 91) < 13) {
                active |= 1;
            }
            i++;
        } while (i < playerCountAbsolute);
    }
    if (!active) {
        return;
    }
    if (!gRacePauseState) {
        if (gCurrentTask->data18 == 0) {
            resource->phase += 24;
            resource->rotation.vy += 96;
        } else {
            resource->phase -= 24;
            resource->rotation.vy -= 96;
        }
        resource->groundCoordinate.coord.t[0] =
            gOrbitingTaskPlacements[gCurrentTask->data18].x
            + rsin(resource->phase) * gOrbitingTaskPlacements[gCurrentTask->data18].radius / 4096;
        resource->groundCoordinate.coord.t[2] =
            gOrbitingTaskPlacements[gCurrentTask->data18].z
            + rcos(resource->phase) * gOrbitingTaskPlacements[gCurrentTask->data18].radius / 4096;
        resource->groundCoordinate.coord.t[1] = getCourseHeightAtPoint(resource->geometryIndex,
            resource->groundCoordinate.coord.t[0], resource->groundCoordinate.coord.t[2]);
        resource->position.x = resource->groundCoordinate.coord.t[0] << 8;
        resource->position.y = resource->groundCoordinate.coord.t[1] << 8;
        resource->position.z = resource->groundCoordinate.coord.t[2] << 8;
        for (i = 0; i < gActiveRacePlayerCount; i++) {
            if (resource->playerCooldowns[i] == 0) {
                if (pushPlayerOutOfCylinder(&resource->position, 0x9000, 0x8000, i)) {
                    s16 heading, delta, limit;
                    playPlayerSoundEffect(i, 1, 0xC01, 0x20);
                    heading = getXZHeading(resource->position.x, resource->position.z,
                        gRacePlayers[i].x, gRacePlayers[i].z);
                    getGeometryPlacementTransform(gRacePlayers[i].unk92E, &x, &y, &z, &trackHeading);
                    delta = (heading - trackHeading) & 0xFFF;
                    limit = 0x200;
                    if (gRacePlayers[i].unk008) {
                        limit = 0x400;
                    }
                    if (delta > limit || delta < -limit) {
                        if (delta > 0x800) {
                            heading += limit;
                        } else {
                            heading -= limit;
                        }
                    } else {
                        heading = trackHeading;
                    }
                    gRacePlayers[i].unk060 = rsin(heading) * 3;
                    gRacePlayers[i].unk064 = rcos(heading) * 3;
                    gRacePlayers[i].unk050 = gRacePlayers[i].x - gRacePlayers[i].unk060;
                    gRacePlayers[i].unk058 = gRacePlayers[i].z - gRacePlayers[i].unk064;
                    resource->playerCooldowns[i] = 6;
                }
            } else {
                resource->playerCooldowns[i]--;
            }
        }
    }
    if (isPositionInCameraRange(resource->groundCoordinate.coord.t[0],
        resource->groundCoordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        setCoordinateRotation(&resource->groundRotation, &resource->groundCoordinate);
        setCoordinateRotation(&resource->rotation, &resource->coordinate);
        GsGetLs(&resource->coordinate, gScratchMatrix0);
        GsSetLsMatrix(gScratchMatrix0);
        if (getRenderCameraIndex()) {
            drawProjectedModelPrimitives(gOptionalCourseModels[0], &gPlayer2OrderingTables[gActiveGraphicsBuffer], 2, 1);
        } else {
            drawProjectedModelPrimitives(gOptionalCourseModels[0], &gPlayer1OrderingTables[gActiveGraphicsBuffer], 2, 1);
        }
    }
}

void initializeCourseUnlockScene(void) {
    s16 index;
    GeometryRegion *region;
    s32 regionOffset;
    CourseGeometryVertex *vertices;
    s32 headingTarget;
    s16 i;

    index = gSelectedCourseRecords[gSelectedCourseIndex].unk00;
    vertices = (CourseGeometryVertex *)gCourseGeometryVertexData;
    regionOffset = index * (s32)sizeof(*region);
    region = (GeometryRegion *)(regionOffset + (u32)gCourseGeometryRegionData);
    gCourseObjectCoordinates[26].coord.t[0] = ((vertices[region->vertices[0]].x + vertices[region->vertices[1]].x
        + vertices[region->vertices[2]].x + vertices[region->vertices[3]].x) / 4) * 5;
    gCourseObjectCoordinates[26].coord.t[2] = ((vertices[region->vertices[0]].z + vertices[region->vertices[1]].z
        + vertices[region->vertices[2]].z + vertices[region->vertices[3]].z) / 4) * 5;
    gCourseObjectCoordinates[26].coord.t[1] = getCourseHeightAtPoint(index,
        gCourseObjectCoordinates[26].coord.t[0], gCourseObjectCoordinates[26].coord.t[2]);
    gCourseObjectRotations[26].vz = 0;
    gCourseObjectRotations[26].vx = 0;
    headingTarget = (s32)gSelectedCourseRecords[gSelectedCourseIndex].unk18[10] >> 8;
    gCourseObjectRotations[26].vy = getXZHeading(gCourseObjectCoordinates[26].coord.t[0], gCourseObjectCoordinates[26].coord.t[2],
        headingTarget, headingTarget);

    createChildTaskWithReserve(copySlot26TransformToCurrentTaskObject);
    gCreatedTask->data18 = gCurrentTask->data18 + 26;
    gCurrentTask->data18 = 26;
    gPlayerCameras[0].pitch = 0x20;
    gPlayerCameras[0].unk10 = -0x180;
    gPlayerCameras[0].yaw = gCourseObjectRotations[26].vy;
    gPlayerCameras[0].unk16 = 9;
    i = 0;
    while (1) {
        if (i >= gActiveRacePlayerCount) {
            break;
        }
        gRacePlayers[i].task->callback = initializeCurrentPlayerUnlockScene;
        i++;
    }
    setCurrentTaskCallback(updateAndDrawCurrentTaskObject);
}

void updateAndDrawCurrentTaskObject(void) {
    setCoordinateRotation(&gCourseObjectRotations[gCurrentTask->data18],
                          &gCourseObjectCoordinates[gCurrentTask->data18]);
    drawTaskObject(gCurrentTask->data18);
}

void copySlot26TransformToCurrentTaskObject(void) {
    gCourseObjectCoordinates[gCurrentTask->data18].coord.t[0] = gCourseObjectCoordinates[26].coord.t[0];
    gCourseObjectCoordinates[gCurrentTask->data18].coord.t[1] = gCourseObjectCoordinates[26].coord.t[1];
    gCourseObjectCoordinates[gCurrentTask->data18].coord.t[2] = gCourseObjectCoordinates[26].coord.t[2];
    gCourseObjectRotations[gCurrentTask->data18].vx = gCourseObjectRotations[26].vx;
    gCourseObjectRotations[gCurrentTask->data18].vy = gCourseObjectRotations[26].vy;
    gCourseObjectRotations[gCurrentTask->data18].vz = gCourseObjectRotations[26].vz;
    setCurrentTaskCallback(updateAndDrawCurrentTaskObject);
}

void updateCpuRacerSpeedControl(void) {
    s32 i, j;
    s32 firstMode, secondMode;
    u8 playerMode;
    s32 order[4];

    if (gMainMenuSelectionIndex) {
        return;
    }

    /* The original builds this standings order before assigning speed modes. */
    order[0] = 0;
    order[1] = 1;
    order[2] = 2;
    order[3] = 3;
    for (i = 0; i < 3; i++) {
        for (j = i; j < 4; j++) {
            s32 first = order[i];
            s32 next = order[j];
            if (gRacePlayers[first].unk935 > gRacePlayers[next].unk935) {
                order[i] = next;
                order[j] = first;
            }
        }
    }

    i = 3;
    playerMode = playerCountAbsolute;
    secondMode = 2;
    firstMode = 1;
    if (playerMode == firstMode) {
        i = 0;
        if (gRacePlayers[0].unk935 == 0) {
            for (j = 0; j < 4; j++) {
                if (gRacePlayers[j].unk008) {
                    u8 mode = gCpuSpeedModesSoloFirst[i++];
                    gRacePlayers[j].speedControlTarget = 0;
                    gRacePlayers[j].speedControlMode = mode;
                }
            }
        }
        i = 0;
        if (gRacePlayers[0].unk935 == 1) {
            for (j = 0; j < 4; j++) {
                if (gRacePlayers[j].unk008) {
                    u8 mode = gCpuSpeedModesSoloSecond[i++];
                    gRacePlayers[j].speedControlTarget = 0;
                    gRacePlayers[j].speedControlMode = mode;
                }
            }
        }
        i = 0;
        if (gRacePlayers[0].unk935 == 2) {
            for (j = 0; j < 4; j++) {
                if (gRacePlayers[j].unk008) {
                    u8 mode = gCpuSpeedModesSoloThird[i++];
                    gRacePlayers[j].speedControlTarget = 0;
                    gRacePlayers[j].speedControlMode = mode;
                }
            }
        }
        i = 0;
        if (gRacePlayers[0].unk935 == 3) {
            for (j = 0; j < 4; j++) {
                if (gRacePlayers[j].unk008) {
                    u8 mode = gCpuSpeedModesSoloFourth[i++];
                    gRacePlayers[j].speedControlTarget = 0;
                    gRacePlayers[j].speedControlMode = mode;
                }
            }
        }
    } else {
        RacePlayer *players = gRacePlayers;
        if (players[0].unk935 <= players[1].unk935) {
            if (players[1].unk935 == i) {
                if (players[2].unk935 <= players[3].unk935) {
                    players[2].speedControlMode = secondMode;
                    players[3].speedControlMode = firstMode;
                } else {
                    players[2].speedControlMode = firstMode;
                    players[3].speedControlMode = secondMode;
                }
            } else {
                if (players[2].unk935 <= players[3].unk935) {
                    players[2].speedControlMode = firstMode;
                    players[3].speedControlMode = secondMode;
                } else {
                    players[2].speedControlMode = secondMode;
                    players[3].speedControlMode = firstMode;
                }
            }
            gRacePlayers[2].speedControlTarget = 1;
            gRacePlayers[3].speedControlTarget = 1;
        } else {
            if (players[0].unk935 == i) {
                if (players[2].unk935 <= players[3].unk935) {
                    goto assignSecondMode;
                }
            } else {
                if (players[2].unk935 > players[3].unk935) {
                    goto assignSecondMode;
                }
            }
            players[2].speedControlMode = firstMode;
            players[3].speedControlMode = secondMode;
            goto setTargetPlayer;

assignSecondMode:
            players[2].speedControlMode = secondMode;
            players[3].speedControlMode = firstMode;

setTargetPlayer:
            gRacePlayers[2].speedControlTarget = 0;
            gRacePlayers[3].speedControlTarget = 0;
        }
    }

    for (i = 0; i < gActiveRacePlayerCount; i++) {
        RacePlayer *player = &gRacePlayers[i];
        if (player->unk008) {
            switch (player->speedControlMode) {
            case 0:
                player->speedControlState = 0;
                break;
            case 1: {
                RacePlayer *target = &gRacePlayers[player->speedControlTarget];
                s32 dx, dz;
                player->speedControlState = 0;
                dx = target->x - player->x;
                dz = target->z - player->z;
                if (dx >= 0x140000 || dx <= -0x140000 || dz >= 0x140000 || dz <= -0x140000) {
                    if (gRacePlayers[player->speedControlTarget].unk935 < player->unk935) {
                        player->speedControlState = 1;
                    } else {
                        player->speedControlState = 2;
                    }
                }
                break;
            }
            case 2:
                player->speedControlState = 3;
                break;
            }
            if (player->characterId == 8 || player->characterId == 9) {
                player->speedControlState = 0;
            }
        }
    }
}

void updateCurrentPlayerCourseTrigger(void) {
    s32 i = 0;
    CourseTriggerRecord *record;
    s32 x, y, z;
    u16 angle;
    s32 sine, cosine;
    s32 mask;
    if (currentPlayerAbsolute->unk128 & 0x1000) currentPlayerAbsolute->unk948 = 0;
    currentPlayerAbsolute->unk950 = 0;
    record = gCourseTriggers[gSelectedCourseIndex];
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
                currentPlayerAbsolute->unk950 = 1;
                currentPlayerAbsolute->unk952[1] = record->value;
                return;
            }
        }
    }
}

s32 updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex) {
    s32 delta;
    RacePlayer *player;

    delta = D_8012BBB4[gSelectedCourseIndex][tableIndex][sampleIndex] * 20;
    player = &gRacePlayers[playerIndex];
    delta -= player->unk94C;
    if (delta > 30) {
        delta = 30;
    }
    if (delta < -30) {
        delta = -30;
    }
    return player->unk94C += delta;
}

void resetSelectedRecordStates(void) {
    SelectedStateRecord *record = D_8012C6F4[gSelectedCourseIndex];

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
    SelectedStateRecord *record = D_8012C6F4[gSelectedCourseIndex];
    for (;; record++, i++) {
        GameTask *task;
        u16 inputIndex;
        if (record->state == -2) goto done;
        if (record->state == -1 && record->unk02 == gRacePlayers[0].unk92E) {
            inputIndex = gControllerInputRecordIndices[0];
            record->state = inputIndex - 1;
            if (gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]-1].repeatCount) {
                record->state = inputIndex;
                split = 1;
            }
            D_8013B2E8[i] = gRacePlayers[0];
            task = gRacePlayers[0].task;
            gSelectedPlayerTaskSnapshots[i].callback = task->callback;
            gSelectedPlayerTaskSnapshots[i].data18 = task->data18;
            gSelectedPlayerTaskSnapshots[i].data1A = task->data1A;
            gSelectedPlayerTaskSnapshots[i].data1C = task->data1C;
            gSelectedPlayerTaskSnapshots[i].data1E = task->data1E;
        }
    }
done:
    if (split) {
        gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]-1].repeatCount--;
        gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]].repeatCount = 0;
        gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]].horizontal = gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]-1].horizontal;
        gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]].vertical = gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]-1].vertical;
        gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]].buttons = gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]-1].buttons;
        gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]].pressedButtons = gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]-1].pressedButtons;
        gControllerInputRecordIndices[0]++;
    }
}

s32 restoreSelectedPlayerState(void) {
    s32 i = 0;
    SelectedStateRecord *record = D_8012C6F4[gSelectedCourseIndex];
    for (;;) {
        if (record->state == -2) return 0;
        if (record->state != -1) {
            GameTask *task = gRacePlayers[0].task;
            gRacePlayers[0] = D_8013B2E8[i];
            gRacePlayers[0].task = task;
            task->callback = gSelectedPlayerTaskSnapshots[i].callback;
            task->data18 = gSelectedPlayerTaskSnapshots[i].data18;
            task->data1A = gSelectedPlayerTaskSnapshots[i].data1A;
            task->data1C = gSelectedPlayerTaskSnapshots[i].data1C;
            task->data1E = gSelectedPlayerTaskSnapshots[i].data1E;
            gControllerInputRecordIndices[0] = record->state;
            D_8013AEE0[0] = gControllerInputRecordBuffers[0][gControllerInputRecordIndices[0]].repeatCount;
            record->state = -1;
            gPlayerCameras[0].x = record->unk04;
            gPlayerCameras[0].y = record->unk08;
            gPlayerCameras[0].z = record->unk0C;
            D_8013B20C = record->unk10;
            gPlayerCameras[0].unk16 = 8;
            return 1;
        }
        record++;
        i++;
    }
}

s32 hasCapturedSelectedPlayerState(void) {
    const SelectedStateRecord *record = D_8012C6F4[gSelectedCourseIndex];

    for (;; record++) {
        if (record->state == -2) {
            goto noState;
        }
        if (record->state != -1) {
            return 1;
        }
    }
noState:
    return 0;
}
