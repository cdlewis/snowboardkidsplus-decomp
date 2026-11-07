# Verified compiler and assembler settings. The Makefile owns the build rules.
# All game C uses GCC 2.7.2-cdk (cygnus-2.7.2-970404) and maspsx 2.67.
# SDK selections retain their independently verified compiler/assembler settings.
# UNIT_CC_G may differ from UNIT_G to reproduce CCPSX's compiler -G8 and
# assembler -G0 defaults. Explicit .data externs also retain absolute addressing
# through tools/cc1psx.py, including in units that otherwise use small data.

MAIN_GAME_UNITS := src/main.c \
    src/io/cd_queue.c \
    src/engine/task_scheduler.c \
    src/io/controller_input.c src/graphics/texture_upload.c \
    src/race/player/player_animation.c \
    src/audio/sound_manager.c \
    src/io/memory_card.c \
    src/io/memory_card_tasks.c \
    src/io/movie_stream.c \
    src/engine/frame_sync.c src/io/memory_card_read.c \
    src/engine/game_loop.c src/engine/task_transitions.c \
    src/race/player/player_update.c \
    src/race/player/player_input.c \
    src/race/player/player_actions.c \
    src/graphics/background_resources.c \
    src/menu/mode_transitions.c \
    src/race/ui/race_panels.c \
    src/menu/record_screens.c \
    src/race/player/player_results.c \
    src/race/camera/race_camera.c \
    src/unclassified/race_8003EE70.c \
    src/race/ui/race_hud.c \
    src/race/effects/coordinate_effects.c \
    src/unclassified/graphics_800497B8.c \
    src/race/course/course_geometry.c \
    src/unclassified/graphics_8004D330.c \
    src/graphics/model_renderer.c \
    src/race/course/course_objects.c \
    src/graphics/primitive_draw.c \
    src/graphics/tim_archive.c \
    src/engine/linear_random.c \
    src/race/effects/player_effects.c \
    src/io/controller_effects.c \
    src/graphics/sprite_draw.c \
    src/menu/task_widgets.c \
    src/unclassified/menu_8007BE9C.c \
    src/unclassified/menu_8007CC78.c \
    src/menu/save_record_tasks.c

G8_UNITS := \
    $(MAIN_GAME_UNITS) \
    src/menu/panel_tasks.c \
    src/race/player/player_model_preview.c \
    src/menu/grid_selection.c \
    src/race/player/player_palette.c \
    src/menu/resource_tasks.c \
    src/text/text_sequence.c \
    src/race/camera/task_camera.c \
    src/menu/selection_tasks.c \
    src/io/controller_playback.c \
    src/menu/unlock_menu.c \
    src/menu/number_adjustment.c \
    src/text/text_entries.c \
    src/audio/music_sequence.c \
    src/text/text_playback.c \
    src/text/resource_playback.c \
    src/menu/unlock_notification.c \
    src/io/host_file.c \
    src/engine/game_options.c \
    src/engine/random.c
$(foreach unit,$(G8_UNITS),$(eval UNIT_G_$(unit) := 8))

GAME_TEXT_UNITS := $(MAIN_GAME_UNITS)
$(foreach unit,$(GAME_TEXT_UNITS),$(eval UNIT_REORDER_$(unit) := 1))

# These modules retain stores and stack restores before returns in the original instruction stream.
UNIT_REORDER_src/main.c := 0
UNIT_REORDER_src/io/cd_queue.c := 0
UNIT_REORDER_src/engine/task_scheduler.c := 0
UNIT_REORDER_src/io/controller_input.c := 0
UNIT_REORDER_src/graphics/texture_upload.c := 0
UNIT_REORDER_src/race/player/player_animation.c := 0
UNIT_REORDER_src/audio/sound_manager.c := 0
UNIT_REORDER_src/io/memory_card.c := 0
UNIT_REORDER_src/io/memory_card_tasks.c := 0
UNIT_REORDER_src/io/movie_stream.c := 0
UNIT_REORDER_src/engine/frame_sync.c := 0
UNIT_REORDER_src/io/memory_card_read.c := 0
UNIT_REORDER_src/engine/game_loop.c := 0
UNIT_REORDER_src/engine/task_transitions.c := 0
UNIT_REORDER_src/race/player/player_update.c := 0
UNIT_REORDER_src/race/player/player_input.c := 0
UNIT_REORDER_src/race/player/player_actions.c := 0
UNIT_REORDER_src/graphics/background_resources.c := 0
UNIT_REORDER_src/menu/mode_transitions.c := 0
UNIT_REORDER_src/race/ui/race_panels.c := 0
UNIT_REORDER_src/menu/record_screens.c := 0
UNIT_REORDER_src/race/player/player_results.c := 0
UNIT_REORDER_src/race/camera/race_camera.c := 0
UNIT_REORDER_src/unclassified/race_8003EE70.c := 0
UNIT_REORDER_src/race/ui/race_hud.c := 0
UNIT_REORDER_src/race/effects/coordinate_effects.c := 0
UNIT_REORDER_src/unclassified/graphics_800497B8.c := 0
UNIT_REORDER_src/race/course/course_geometry.c := 0
UNIT_REORDER_src/unclassified/graphics_8004D330.c := 0
UNIT_REORDER_src/graphics/model_renderer.c := 0
UNIT_REORDER_src/race/course/course_objects.c := 0
UNIT_REORDER_src/graphics/primitive_draw.c := 0
UNIT_REORDER_src/graphics/tim_archive.c := 0
UNIT_REORDER_src/engine/linear_random.c := 0
UNIT_G_src/engine/linear_random_state.c := 8
UNIT_REORDER_src/engine/linear_random_state.c := 0
UNIT_REORDER_src/race/effects/player_effects.c := 0
UNIT_REORDER_src/io/controller_effects.c := 0
UNIT_REORDER_src/graphics/sprite_draw.c := 0
UNIT_REORDER_src/menu/task_widgets.c := 0
UNIT_REORDER_src/unclassified/menu_8007CC78.c := 0
UNIT_REORDER_src/menu/save_record_tasks.c := 0

GCC272_G0_REORDER_PAD_UNITS := \
    src/syslib/libcard/card.c \
    src/syslib/libgte/geo_00.c \
    src/syslib/libgte/geo_01.c \
    src/syslib/libcard/init.c \
    src/syslib/libetc/vsync.c \
    src/syslib/libgs/gs_119.c \
    src/syslib/libsnd/vs_mono.c \
    src/syslib/libsnd/vm_doff.c \
    src/syslib/libsnd/vm_don.c \
    src/syslib/libsnd/vm_prog.c \
    src/syslib/libsnd/vm_noff.c \
    src/syslib/libsnd/vm_nowof.c \
    src/syslib/libsnd/cc_98.c \
    src/syslib/libsnd/ssinit_c.c \
    src/syslib/libgs/gs_101.c \
    src/syslib/libgs/gs_121.c \
    src/syslib/libgs/gs_127.c \
    src/syslib/libmath/ferr.c \
    src/syslib/libspu/s_m_init.c \
    src/syslib/libspu/s_m_util.c \
    src/syslib/libspu/s_gvv.c \
    src/syslib/libspu/s_gvex.c \
    src/syslib/libspu/s_dcb.c \
    src/syslib/libspu/s_it.c \
    src/syslib/libspu/s_itc.c \
    src/syslib/libspu/s_w.c \
    src/syslib/libspu/s_stm.c \
    src/syslib/libetc/vmode.c \
    src/syslib/libsnd/sssv.c \
    src/syslib/libsnd/sssattr.c \
    src/syslib/libspu/s_snv.c \
    src/syslib/libspu/s_srv.c \
    src/syslib/libpress/libpress.c \
    src/syslib/libetc/intr_vb.c \
    src/syslib/libsnd/ssstop.c \
    src/syslib/libsnd/ssplay.c \
    src/syslib/libsnd/sspause.c \
    src/syslib/libsnd/ssreplay.c \
    src/syslib/libsnd/sssmv.c \
    src/syslib/libsnd/play.c \
    src/syslib/libsnd/pause.c \
    src/syslib/libsnd/replay.c \
    src/syslib/libsnd/next.c \
    src/syslib/libsnd/de_0.c \
    src/syslib/libsnd/de_2.c \
    src/syslib/libsnd/de_3.c \
    src/syslib/libsnd/de_4.c \
    src/syslib/libsnd/de_5.c \
    src/syslib/libsnd/de_6.c \
    src/syslib/libsnd/de_7.c \
    src/syslib/libsnd/de_8.c \
    src/syslib/libsnd/de_9.c \
    src/syslib/libsnd/de_10.c \
    src/syslib/libsnd/de_11.c \
    src/syslib/libsnd/de_12.c \
    src/syslib/libsnd/de_13.c \
    src/syslib/libsnd/de_14.c \
    src/syslib/libsnd/de_15.c \
    src/syslib/libsnd/de_16.c \
    src/syslib/libsnd/vs_vtc.c \
    src/syslib/libgpu/ext.c \
    src/syslib/libgpu/p00.c \
    src/syslib/libgpu/p01.c \
    src/syslib/libgpu/p06.c \
    src/syslib/libgpu/p10.c \
    src/syslib/libgpu/p17.c \
    src/syslib/libgpu/p22.c \
    src/syslib/libgpu/p26.c \
    src/syslib/libgpu/p33.c \
    src/syslib/libgpu/p34.c \
    src/syslib/libgs/matrix.c \
    src/syslib/libgs/gs_110.c \
    src/syslib/libgs/gs_111.c \
    src/syslib/libgs/gs_113.c \
    src/syslib/libgs/gs_122.c \
    src/syslib/libcd/c_002.c \
    src/syslib/libcd/c_003.c \
    src/syslib/libcd/c_005.c \
    src/syslib/libcd/c_007.c \
    src/syslib/libcd/c_008.c \
    src/syslib/libcd/c_010.c \
    src/syslib/libcd/event.c \
    src/syslib/libcd/cdread2.c \
    src/syslib/libcd/cdrom.c \
    src/syslib/libpad/padentry.c
$(foreach unit,$(GCC272_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.7.2-psx/cc1))
$(foreach unit,$(GCC272_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC272_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC272_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))
UNIT_CCFLAGS_src/syslib/libgs/gs_127.c := -fno-delayed-branch

PAD_UNITS := \
    src/syslib/libgte/sincos.c
$(foreach unit,$(PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

REORDER_PAD_UNITS := \
    src/syslib/libsnd/ut_rdel.c \
    src/syslib/libsnd/ut_rdep.c
$(foreach unit,$(REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC272_G0_INDEXED_REORDER_PAD_UNITS := \
    src/syslib/libsnd/midibend.c \
    src/syslib/libsnd/cc_0.c \
    src/syslib/libsnd/vs_vab.c \
    src/syslib/libsnd/vs_vtb.c \
    src/syslib/libsnd/cc_7.c \
    src/syslib/libsnd/cc_11.c \
    src/syslib/libsnd/cc_64.c \
    src/syslib/libsnd/cc_91.c \
    src/syslib/libsnd/cc_100.c \
    src/syslib/libsnd/cc_101.c \
    src/syslib/libsnd/cc_121.c \
    src/syslib/libsnd/midiprog.c \
    src/syslib/libsnd/ut_gpa.c \
    src/syslib/libapi/counter.c \
    src/syslib/libsnd/ut_vvol.c
$(foreach unit,$(GCC272_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.7.2-psx/cc1))
$(foreach unit,$(GCC272_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC272_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_INDEXED_$(unit) := 1))
$(foreach unit,$(GCC272_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC272_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC272_O1_G0_REORDER_PAD_UNITS := \
    src/syslib/libspu/s_stsa.c
$(foreach unit,$(GCC272_O1_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.7.2-psx/cc1))
$(foreach unit,$(GCC272_O1_G0_REORDER_PAD_UNITS),$(eval UNIT_CCFLAGS_$(unit) := -O1))
$(foreach unit,$(GCC272_O1_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC272_O1_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC272_O1_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC281_G0_REORDER_PAD_UNITS := \
    src/syslib/libsnd/ccadsr.c
$(foreach unit,$(GCC281_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.8.1-psx/cc1))
$(foreach unit,$(GCC281_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC281_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC281_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC280_G0_REORDER_PAD_UNITS := \
    src/syslib/libsnd/de_1.c
$(foreach unit,$(GCC280_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.8.0-psx/cc1))
$(foreach unit,$(GCC280_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC280_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC280_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC280_MNO_SPLIT_ADDRESSES__FNO_DELAYED_BRANCH_G0_REORDER_PAD_UNITS := \
    src/syslib/libgs/gs_103.c
$(foreach unit,$(GCC280_MNO_SPLIT_ADDRESSES__FNO_DELAYED_BRANCH_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.8.0-psx/cc1))
$(foreach unit,$(GCC280_MNO_SPLIT_ADDRESSES__FNO_DELAYED_BRANCH_G0_REORDER_PAD_UNITS),$(eval UNIT_CCFLAGS_$(unit) := -mno-split-addresses -fno-delayed-branch))
$(foreach unit,$(GCC280_MNO_SPLIT_ADDRESSES__FNO_DELAYED_BRANCH_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC280_MNO_SPLIT_ADDRESSES__FNO_DELAYED_BRANCH_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC280_MNO_SPLIT_ADDRESSES__FNO_DELAYED_BRANCH_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS := \
    src/syslib/libspu/s_snc.c
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.8.1-psx/cc1))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_CCFLAGS_$(unit) := -mno-split-addresses))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

# Apply these settings to each complete SDK unit in one compiler invocation.
UNIT_CCFLAGS_src/syslib/libsnd/ut_rdel.c := -mno-split-addresses -fno-delayed-branch
UNIT_CCFLAGS_src/syslib/libsnd/ut_rdep.c := -mno-split-addresses -fno-delayed-branch
