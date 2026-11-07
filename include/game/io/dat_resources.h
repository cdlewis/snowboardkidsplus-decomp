#pragma once

#include "decomp/types.h"
#include "game/engine/offset_archive.h"
#include "game/io/dat_entries.h"

/* Generated addresses come from symbol_addrs.txt. These are overlapping RAM
 * views with different lifetimes, not storage in the executable. Fixed-address
 * expressions preserve the original lui/ori loads (extern objects use addiu).
 */
#define DAT_SCRATCH ((u8 *)DAT_SCRATCH_ADDRESS)
#define DAT_STARTUP_MODELS ((OffsetArchive *)DAT_STARTUP_MODEL_BUFFER_ADDRESS)
#define DAT_STARTUP_TEXTURES ((OffsetArchive *)DAT_STARTUP_TEXTURE_BUFFER_ADDRESS)
#define DAT_PLAYER_MODEL_BUFFERS ((u32 (*)[4096])DAT_PLAYER_MODEL_BUFFERS_ADDRESS)
#define DAT_PLAYER_MODEL_BLOCKS ((u32 (*)[512])DAT_PLAYER_MODEL_BLOCKS_ADDRESS)
#define DAT_BACKGROUND_MAP ((u8 *)DAT_BACKGROUND_MAP_ADDRESS)
#define DAT_BACKGROUND_TEXTURE_FIRST ((u32 *)DAT_BACKGROUND_TEXTURE_FIRST_ADDRESS)
#define DAT_BACKGROUND_TEXTURE_SECOND ((u32 *)DAT_BACKGROUND_TEXTURE_SECOND_ADDRESS)
#define DAT_TEXTURE_ARCHIVE ((OffsetArchive *)DAT_TEXTURE_ARCHIVE_BUFFER_ADDRESS)
#define DAT_SOUND_HEADER_0 ((u8 *)DAT_SOUND_HEADER_0_ADDRESS)
#define DAT_SOUND_HEADER_1 ((u8 *)DAT_SOUND_HEADER_1_ADDRESS)
#define DAT_SOUND_HEADER_2 ((u8 *)DAT_SOUND_HEADER_2_ADDRESS)
#define DAT_MUSIC_SEQUENCE ((u8 *)DAT_MUSIC_SEQUENCE_ADDRESS)
#define DAT_COURSE_FIRST ((u8 *)DAT_COURSE_FIRST_ADDRESS)
#define DAT_COURSE_SECOND ((u8 *)DAT_COURSE_SECOND_ADDRESS)
#define DAT_COURSE_OPTIONAL ((u8 *)DAT_COURSE_OPTIONAL_ADDRESS)
#define DAT_SCENE_IMAGE ((u32 *)DAT_SCENE_IMAGE_ADDRESS)
#define DAT_INDEXED_RESOURCE ((u8 *)DAT_INDEXED_RESOURCE_ADDRESS)
#define DAT_GRID_MODEL_BUFFERS ((u8 *)DAT_GRID_MODEL_BUFFERS_ADDRESS)
#define DAT_GRID_TEXTURE_BUFFERS ((u8 *)DAT_GRID_TEXTURE_BUFFERS_ADDRESS)
#define DAT_ANIMATION_ARCHIVE ((OffsetArchive *)DAT_SCRATCH_ADDRESS)
/* SceneEventWorkspace is declared by the scene owner in scene_task_types.h. */
#define DAT_SCENE_WORKSPACE ((SceneEventWorkspace *)DAT_SCRATCH_ADDRESS)
/* SaveRecord is declared by the save-data owner. */
#define SAVE_RECORD_WORKSPACE ((SaveRecord *)SAVE_RECORD_WORKSPACE_ADDRESS)

/* Movie decoding reuses the resource arena while gameplay is suspended. */
#define MOVIE_VLC_BUFFER_0 ((u32 *)MOVIE_VLC_BUFFER_0_ADDRESS)
#define MOVIE_VLC_BUFFER_1 ((u32 *)MOVIE_VLC_BUFFER_1_ADDRESS)
#define MOVIE_IMAGE_BUFFER_0 ((u32 *)MOVIE_IMAGE_BUFFER_0_ADDRESS)
#define MOVIE_IMAGE_BUFFER_1 ((u32 *)MOVIE_IMAGE_BUFFER_1_ADDRESS)
#define MOVIE_VLC_TABLE ((u16 *)MOVIE_VLC_TABLE_ADDRESS)
