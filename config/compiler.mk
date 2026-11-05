# Verified compiler and assembler settings. The Makefile owns the build rules.
# SDK function selections preserve the existing matches; this refactor does not
# establish different original compilers or alter translation-unit boundaries.

MAIN_GAME_UNITS := src/main.c src/main_remaining.c \
    src/game/unk_8001E63C.c \
    src/game/unk_8002B3F4.c \
    src/game/unk_800497B8.c \
    src/game/unk_80070274.c \
    src/game/unk_8007BE9C.c \
    src/game/unk_8007CC78.c

G8_UNITS := \
    $(MAIN_GAME_UNITS) \
    src/engine/game_options.c \
    src/engine/random.c
$(foreach unit,$(G8_UNITS),$(eval UNIT_G_$(unit) := 8))

GAME_TEXT_UNITS := $(MAIN_GAME_UNITS)
$(foreach unit,$(GAME_TEXT_UNITS),$(eval UNIT_REORDER_$(unit) := 1))

# Preserve the task-reference reset's store before its return (PSY-Q scheduling).
UNIT_REORDER_src/game/unk_80070274.c := 0

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

UNIT_FUNCTIONS_src/main.c := deleteCurrentTask
FN_FLAGS_src/main.c/deleteCurrentTask := -fno-delayed-branch
UNIT_FUNCTIONS_src/main_remaining.c := stopMusicSequence pauseMusicSequence resumeMusicSequence
UNIT_FUNCTIONS_src/game/unk_800497B8.c := initPrimitiveTemplates
FN_FLAGS_src/game/unk_800497B8.c/initPrimitiveTemplates := -fno-delayed-branch
$(foreach function,$(UNIT_FUNCTIONS_src/main_remaining.c),$(eval FN_FLAGS_src/main_remaining.c/$(function) := -fno-delayed-branch))
UNIT_FUNCTIONS_src/engine/random.c := resetGameplayRng
FN_FLAGS_src/engine/random.c/resetGameplayRng := -fno-delayed-branch
UNIT_FUNCTIONS_src/syslib/libsnd/ut_rdel.c := SsUtSetReverbDelay
FN_FLAGS_src/syslib/libsnd/ut_rdel.c/SsUtSetReverbDelay := -mno-split-addresses -fno-delayed-branch
UNIT_FUNCTIONS_src/syslib/libsnd/ut_rdep.c := SsUtSetReverbDepth
FN_FLAGS_src/syslib/libsnd/ut_rdep.c/SsUtSetReverbDepth := -mno-split-addresses -fno-delayed-branch

# Partial implementations apply only to separate comparison/scratch objects.
ifeq ($(COMPARISON):$(NON_MATCHING),1:1)
endif
