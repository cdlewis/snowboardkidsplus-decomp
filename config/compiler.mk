# Verified compiler and assembler settings. The Makefile owns the build rules.
# SDK function selections preserve the existing matches; this refactor does not
# establish different original compilers or alter translation-unit boundaries.

G8_UNITS := \
    src/main.c \
    src/main_remaining.c \
    src/engine/game_options.c \
    src/engine/random.c
$(foreach unit,$(G8_UNITS),$(eval UNIT_G_$(unit) := 8))

GAME_TEXT_UNITS := src/main.c src/main_remaining.c
$(foreach unit,$(GAME_TEXT_UNITS),$(eval UNIT_REORDER_$(unit) := 1))

GCC272_G0_REORDER_PAD_UNITS := \
    src/syslib/libcard/card.c \
    src/syslib/libgte/geo_00.c \
    src/syslib/libgte/geo_01.c \
    src/syslib/libgte/cor_05.c \
    src/syslib/libcard/init.c \
    src/syslib/libetc/vsync.c \
    src/syslib/libspu/s_sr.c \
    src/syslib/libsn/read.c \
    src/syslib/libapi/first.c \
    src/syslib/libsnd/midicc.c \
    src/syslib/libgs/gs_119.c \
    src/syslib/libgs/gs_120.c \
    src/syslib/libsnd/vs_mono.c \
    src/syslib/libsnd/vm_doff.c \
    src/syslib/libsnd/vm_don.c \
    src/syslib/libsnd/vm_key.c \
    src/syslib/libsnd/vm_prog.c \
    src/syslib/libsnd/vm_noff.c \
    src/syslib/libsnd/vm_nowof.c \
    src/syslib/libsnd/vm_n2p.c \
    src/syslib/libsnd/cc_98.c \
    src/syslib/libsnd/cc_99.c \
    src/syslib/libsnd/ssinit_c.c \
    src/syslib/libsnd/ut_keyv.c \
    src/syslib/libgs/gs_001.c \
    src/syslib/libgs/gs_010.c \
    src/syslib/libgs/gs_101.c \
    src/syslib/libgs/gs_121.c \
    src/syslib/libgs/gs_127.c \
    src/syslib/libgs/2d_com0.c \
    src/syslib/libspu/s_ini.c \
    src/syslib/libmath/ferr.c \
    src/syslib/libspu/s_m_init.c \
    src/syslib/libspu/s_m_util.c \
    src/syslib/libspu/s_gvv.c \
    src/syslib/libspu/s_gvex.c \
    src/syslib/libspu/s_dcb.c \
    src/syslib/libspu/spu.c \
    src/syslib/libspu/s_it.c \
    src/syslib/libspu/s_itc.c \
    src/syslib/libspu/s_w.c \
    src/syslib/libspu/s_stm.c \
    src/syslib/libspu/s_n2p.c \
    src/syslib/libspu/s_crwa.c \
    src/syslib/libetc/vmode.c \
    src/syslib/libsnd/sssv.c \
    src/syslib/libsnd/sssattr.c \
    src/syslib/libsnd/vm_stav.c \
    src/syslib/libsnd/playmode.c \
    src/syslib/libsnd/midinote.c \
    src/syslib/libsnd/ut_ako.c \
    src/syslib/libsnd/ssinit.c \
    src/syslib/libsnd/sstable.c \
    src/syslib/libsnd/ut_sva.c \
    src/syslib/libsnd/ut_gva.c \
    src/syslib/libsnd/midimeta.c \
    src/syslib/libsnd/tempo.c \
    src/syslib/libgs/2d_com1.c \
    src/syslib/libgs/2d_sp1.c \
    src/syslib/libspu/s_snv.c \
    src/syslib/libspu/s_srv.c \
    src/syslib/libsnd/ssstart.c \
    src/syslib/libpress/libpress.c \
    src/syslib/libetc/intr.c \
    src/syslib/libetc/intr_vb.c \
    src/syslib/libetc/intr_dma.c \
    src/syslib/libsnd/ssclose.c \
    src/syslib/libsnd/ssstop.c \
    src/syslib/libsnd/ssplay.c \
    src/syslib/libsnd/sspause.c \
    src/syslib/libsnd/ssreplay.c \
    src/syslib/libsnd/ssvol.c \
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
    src/syslib/libsnd/ut_rev.c \
    src/syslib/libsnd/vs_vh.c \
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
    src/syslib/libgs/gs_107.c \
    src/syslib/libgs/gs_110.c \
    src/syslib/libgs/gs_111.c \
    src/syslib/libgs/gs_113.c \
    src/syslib/libgs/gs_122.c \
    src/syslib/libcd/c_002.c \
    src/syslib/libcd/c_003.c \
    src/syslib/libcd/c_004.c \
    src/syslib/libcd/c_005.c \
    src/syslib/libcd/c_007.c \
    src/syslib/libcd/c_008.c \
    src/syslib/libcd/c_010.c \
    src/syslib/libcd/event.c \
    src/syslib/libcd/cdread2.c \
    src/syslib/libcd/cdreade.c \
    src/syslib/libcd/cdrom.c \
    src/syslib/libcd/iso9660.c \
    src/syslib/libds/dscb.c \
    src/syslib/libpad/padseqd.c \
    src/syslib/libpad/padportd.c \
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
    src/syslib/libc/memmove.c \
    src/syslib/libcd/c_011.c \
    src/syslib/libcd/c_009.c \
    src/syslib/libsnd/ut_rdel.c \
    src/syslib/libsnd/ut_rdep.c
$(foreach unit,$(REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC272_G0_INDEXED_REORDER_PAD_UNITS := \
    src/syslib/libsnd/midibend.c \
    src/syslib/libsnd/cc_0.c \
    src/syslib/libsnd/vs_vab.c \
    src/syslib/libsnd/vs_vtb.c \
    src/syslib/libsnd/vm_seq.c \
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
    src/syslib/libspu/s_stsa.c \
    src/syslib/libpad/padcmd.c
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

GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS := \
    src/syslib/libgpu/sys.c
$(foreach unit,$(GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.7.2-psx/cc1))
$(foreach unit,$(GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_CCFLAGS_$(unit) := -fno-builtin))
$(foreach unit,$(GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_INDEXED_$(unit) := 1))
$(foreach unit,$(GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC272_FNO_BUILTIN_G0_INDEXED_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

GCC280_G0_REORDER_PAD_UNITS := \
    src/syslib/libsnd/de_1.c \
    src/syslib/libsnd/cc_10.c \
    src/syslib/libsnd/ssend.c \
    src/syslib/libsnd/miditime.c \
    src/syslib/libgs/gs_005.c
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

GCC272_G0_REORDER_UNITS := \
    src/syslib/libapi/pad.c \
    src/syslib/libcd/bios.c \
    src/syslib/libcd/cdread.c
$(foreach unit,$(GCC272_G0_REORDER_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.7.2-psx/cc1))
$(foreach unit,$(GCC272_G0_REORDER_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC272_G0_REORDER_UNITS),$(eval UNIT_REORDER_$(unit) := 1))

REORDER_UNITS := \
    src/syslib/libspu/s_m_f.c \
    src/syslib/libsnd/ut_rfb.c \
    src/syslib/libcd/sys.c
$(foreach unit,$(REORDER_UNITS),$(eval UNIT_REORDER_$(unit) := 1))

GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS := \
    src/syslib/libspu/s_snc.c
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_CC1_$(unit) := tools/gcc/gcc-2.8.1-psx/cc1))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_CCFLAGS_$(unit) := -mno-split-addresses))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_G_$(unit) := 0))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_REORDER_$(unit) := 1))
$(foreach unit,$(GCC281_MNO_SPLIT_ADDRESSES_G0_REORDER_PAD_UNITS),$(eval UNIT_PAD_$(unit) := 1))

UNIT_FUNCTIONS_src/main.c := deleteCurrentTask
FN_FLAGS_src/main.c/deleteCurrentTask := -fno-delayed-branch
UNIT_FUNCTIONS_src/main_remaining.c := stopMusicSequence pauseMusicSequence resumeMusicSequence initPrimitiveTemplates
$(foreach function,$(UNIT_FUNCTIONS_src/main_remaining.c),$(eval FN_FLAGS_src/main_remaining.c/$(function) := -fno-delayed-branch))
UNIT_FUNCTIONS_src/engine/random.c := resetGameplayRng
FN_FLAGS_src/engine/random.c/resetGameplayRng := -fno-delayed-branch
UNIT_FUNCTIONS_src/syslib/libsnd/vm_seq.c := _SsVmGetSeqLVol _SsVmGetSeqRVol
FN_FLAGS_src/syslib/libsnd/vm_seq.c/_SsVmGetSeqLVol := -O1
FN_FLAGS_src/syslib/libsnd/vm_seq.c/_SsVmGetSeqRVol := -O1
UNIT_FUNCTIONS_src/syslib/libgs/gs_001.c := GsInitGraph2
FN_FLAGS_src/syslib/libgs/gs_001.c/GsInitGraph2 := -mno-split-addresses -fno-delayed-branch
FN_CC1_src/syslib/libgs/gs_001.c/GsInitGraph2 := tools/gcc/gcc-2.8.0-psx/cc1
UNIT_FUNCTIONS_src/syslib/libspu/spu.c := _spu_Fw1ts
FN_FLAGS_src/syslib/libspu/spu.c/_spu_Fw1ts :=
FN_CC1_src/syslib/libspu/spu.c/_spu_Fw1ts := tools/gcc/gcc-2.8.0-psx/cc1
UNIT_FUNCTIONS_src/syslib/libetc/intr.c := func_800B5DEC
FN_FLAGS_src/syslib/libetc/intr.c/func_800B5DEC := -mno-split-addresses
FN_CC1_src/syslib/libetc/intr.c/func_800B5DEC := tools/gcc/gcc-2.8.0-psx/cc1
UNIT_FUNCTIONS_src/syslib/libgpu/sys.c := _cwb DrawSyncCallback SetGraphDebug gpuGetClipStart gpuGetClipEnd func_800CD4CC MoveImage2 MoveImage ResetGraph
FN_FLAGS_src/syslib/libgpu/sys.c/ResetGraph := -mno-split-addresses
FN_CC1_src/syslib/libgpu/sys.c/ResetGraph := tools/gcc/gcc-2.8.0-psx/cc1
FN_FLAGS_src/syslib/libgpu/sys.c/_cwb := -fno-delayed-branch
FN_FLAGS_src/syslib/libgpu/sys.c/MoveImage2 := -mno-split-addresses
FN_CC1_src/syslib/libgpu/sys.c/MoveImage2 := tools/gcc/gcc-2.8.0-psx/cc1
FN_FLAGS_src/syslib/libgpu/sys.c/MoveImage := -mno-split-addresses
FN_CC1_src/syslib/libgpu/sys.c/MoveImage := tools/gcc/gcc-2.8.0-psx/cc1
FN_FLAGS_src/syslib/libgpu/sys.c/DrawSyncCallback := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/SetGraphDebug := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/gpuGetClipStart := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/gpuGetClipEnd := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/func_800CD4CC := -mno-split-addresses
FN_CC1_src/syslib/libgpu/sys.c/DrawSyncCallback := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/SetGraphDebug := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/gpuGetClipStart := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/gpuGetClipEnd := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/func_800CD4CC := tools/gcc/gcc-2.8.0-psx/cc1
UNIT_FUNCTIONS_src/syslib/libcd/cdread.c := func_800B8E68
FN_FLAGS_src/syslib/libcd/cdread.c/func_800B8E68 := -mno-split-addresses
FN_CC1_src/syslib/libcd/cdread.c/func_800B8E68 := tools/gcc/gcc-2.8.0-psx/cc1
UNIT_FUNCTIONS_src/syslib/libsnd/ut_rdel.c := SsUtSetReverbDelay
FN_FLAGS_src/syslib/libsnd/ut_rdel.c/SsUtSetReverbDelay := -mno-split-addresses -fno-delayed-branch
UNIT_FUNCTIONS_src/syslib/libsnd/ut_rdep.c := SsUtSetReverbDepth
FN_FLAGS_src/syslib/libsnd/ut_rdep.c/SsUtSetReverbDepth := -mno-split-addresses -fno-delayed-branch
UNIT_FUNCTIONS_src/syslib/libpad/padcmd.c := _padSetActAlign _padSetActAlign_snd _padLoadActInfo_snd _padSendAtLoadInfo
FN_FLAGS_src/syslib/libpad/padcmd.c/_padSetActAlign := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/_padSetActAlign_snd := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/_padLoadActInfo_snd := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/_padSendAtLoadInfo := -O2
UNIT_FUNCTIONS_src/syslib/libcd/sys.c := CdFlush CdSync CdReady CdDataSync CdSetDebug CdSyncCallback CdReadyCallback CdComstr CdIntstr CdDataCallback
FN_FLAGS_src/syslib/libcd/sys.c/CdFlush := -fno-delayed-branch
FN_FLAGS_src/syslib/libcd/sys.c/CdSync := -fno-delayed-branch
FN_FLAGS_src/syslib/libcd/sys.c/CdReady := -fno-delayed-branch
FN_FLAGS_src/syslib/libcd/sys.c/CdDataSync := -fno-delayed-branch
FN_FLAGS_src/syslib/libcd/sys.c/CdSetDebug := -mno-split-addresses
FN_FLAGS_src/syslib/libcd/sys.c/CdSyncCallback := -mno-split-addresses
FN_FLAGS_src/syslib/libcd/sys.c/CdReadyCallback := -mno-split-addresses
FN_FLAGS_src/syslib/libcd/sys.c/CdComstr :=
FN_FLAGS_src/syslib/libcd/sys.c/CdIntstr :=
FN_FLAGS_src/syslib/libcd/sys.c/CdDataCallback :=
FN_CC1_src/syslib/libcd/sys.c/CdComstr := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libcd/sys.c/CdIntstr := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libcd/sys.c/CdDataCallback := tools/gcc/gcc-2.7.2-psx/cc1

# Partial implementations apply only to separate comparison/scratch objects.
ifeq ($(COMPARISON):$(NON_MATCHING),1:1)
UNIT_FUNCTIONS_src/syslib/libgs/gs_001.c := GsInitGraph2 func_800BD3FC
FN_FLAGS_src/syslib/libgs/gs_001.c/func_800BD3FC := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libc/memmove.c := memmove
FN_FLAGS_src/syslib/libc/memmove.c/memmove := -fno-builtin
UNIT_FUNCTIONS_src/syslib/libspu/s_sk.c := SpuSetKey
FN_FLAGS_src/syslib/libspu/s_sk.c/SpuSetKey := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/vm_key.c := _SsVmKeyOff
FN_FLAGS_src/syslib/libsnd/vm_key.c/_SsVmKeyOff := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/vm_aloc2.c := _SsVmDoAllocate
FN_FLAGS_src/syslib/libsnd/vm_aloc2.c/_SsVmDoAllocate := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/vm_aloc1.c := _SsVmAlloc
FN_FLAGS_src/syslib/libsnd/vm_aloc1.c/_SsVmAlloc := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libgs/gs_004.c := GsSetOffset
FN_FLAGS_src/syslib/libgs/gs_004.c/GsSetOffset := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/midiread.c := _SsSeqPlay
FN_FLAGS_src/syslib/libsnd/midiread.c/_SsSeqPlay := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/ssopenq.c := SsSeqOpen
FN_FLAGS_src/syslib/libsnd/ssopenq.c/SsSeqOpen := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libspu/s_m_int.c := _spu_gcSPU
FN_FLAGS_src/syslib/libspu/s_m_int.c/_spu_gcSPU := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/sstick.c := SsSetTickMode
FN_FLAGS_src/syslib/libsnd/sstick.c/SsSetTickMode := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libgs/gs_123.c := Gssub_make_matrix
FN_FLAGS_src/syslib/libgs/gs_123.c/Gssub_make_matrix := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/vm_pb.c := _SsVmPitchBend
FN_FLAGS_src/syslib/libsnd/vm_pb.c/_SsVmPitchBend := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libpress/build.c := DecDCTvlcBuild
FN_FLAGS_src/syslib/libpress/build.c/DecDCTvlcBuild := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libetc/intr.c := func_800B5DEC func_800B595C func_800B5A34
FN_FLAGS_src/syslib/libetc/intr.c/func_800B595C := -mno-split-addresses
FN_FLAGS_src/syslib/libetc/intr.c/func_800B5A34 := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/ut_cp.c := SsUtChangePitch
FN_FLAGS_src/syslib/libsnd/ut_cp.c/SsUtChangePitch := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/vm_vsu.c := _SsVmVSetUp
FN_FLAGS_src/syslib/libsnd/vm_vsu.c/_SsVmVSetUp := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libgs/gs_002.c := GsSetDrawBuffOffset
FN_FLAGS_src/syslib/libgs/gs_002.c/GsSetDrawBuffOffset :=
UNIT_FUNCTIONS_src/syslib/libgs/gs_003.c := GsSetDrawBuffClip
FN_FLAGS_src/syslib/libgs/gs_003.c/GsSetDrawBuffClip := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/vm_n2p.c := note2pitch
FN_FLAGS_src/syslib/libsnd/vm_n2p.c/note2pitch := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libpad/padcmd.c := _padSetActAlign _padSetActAlign_snd _padLoadActInfo_snd _padSendAtLoadInfo _padSetMainMode _padLoadActInfo func_800D3180 _padRecvAtLoadInfo func_800D2E88
FN_FLAGS_src/syslib/libpad/padcmd.c/_padSetMainMode := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/_padLoadActInfo := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/func_800D3180 := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/_padRecvAtLoadInfo := -O2
FN_FLAGS_src/syslib/libpad/padcmd.c/func_800D2E88 := -O2
UNIT_FUNCTIONS_src/syslib/libsnd/ssend.c := SsEnd
FN_FLAGS_src/syslib/libsnd/ssend.c/SsEnd := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libcd/cdread.c := func_800B8E68 CdReadBreak
FN_FLAGS_src/syslib/libcd/cdread.c/CdReadBreak := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libpad/padseqd.c := func_800D44E0
FN_FLAGS_src/syslib/libpad/padseqd.c/func_800D44E0 :=
UNIT_FUNCTIONS_src/syslib/libsnd/ssvol.c := _SsSndSetVol SsSepSetVol
FN_FLAGS_src/syslib/libsnd/ssvol.c/_SsSndSetVol := -mno-split-addresses
FN_FLAGS_src/syslib/libsnd/ssvol.c/SsSepSetVol := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libgpu/sys.c := _cwb DrawSyncCallback SetGraphDebug gpuGetClipStart gpuGetClipEnd func_800CD4CC ResetGraph SetDrawEnv func_800CEAD8 func_800CED08 MoveImage MoveImage2
FN_FLAGS_src/syslib/libgpu/sys.c/ResetGraph := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/SetDrawEnv := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/func_800CEAD8 := -mno-split-addresses
FN_FLAGS_src/syslib/libgpu/sys.c/func_800CED08 := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libpad/padportd.c := func_800D3CF4
FN_FLAGS_src/syslib/libpad/padportd.c/func_800D3CF4 :=
UNIT_FUNCTIONS_src/syslib/libgs/gs_005.c := GsSetClip
FN_FLAGS_src/syslib/libgs/gs_005.c/GsSetClip := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libgs/gs_104.c := GsInit3D
FN_FLAGS_src/syslib/libgs/gs_104.c/GsInit3D := -mno-split-addresses -fno-delayed-branch
UNIT_FUNCTIONS_src/syslib/libcd/c_009.c := StGetNext
FN_FLAGS_src/syslib/libcd/c_009.c/StGetNext := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libspu/s_m_f.c := SpuFree
FN_FLAGS_src/syslib/libspu/s_m_f.c/SpuFree := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libetc/intr_dma.c := setIntrDMA
FN_FLAGS_src/syslib/libetc/intr_dma.c/setIntrDMA := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/ssstart.c := func_800AC754 _SsStart
FN_FLAGS_src/syslib/libsnd/ssstart.c/func_800AC754 := -O1
FN_FLAGS_src/syslib/libsnd/ssstart.c/_SsStart := -mno-split-addresses
UNIT_FUNCTIONS_src/syslib/libsnd/ut_rfb.c := SsUtSetReverbFeedback
FN_FLAGS_src/syslib/libsnd/ut_rfb.c/SsUtSetReverbFeedback := -mno-split-addresses -fno-delayed-branch
FN_CC1_src/syslib/libsnd/ssstart.c/_SsStart := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libcd/c_011.c/func_800BA430 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgs/gs_001.c/func_800BD3FC := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libcd/sys.c/CdControl := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libcd/sys.c/CdControlF := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libcd/sys.c/CdControlB := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libcd/cdreade.c/CdReadFile := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libc/memmove.c/memmove := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libapi/pad.c/func_800D6F04 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libds/dscb.c/DsSyncCallback := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libds/dscb.c/DsReadyCallback := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libds/dscb.c/DsStartCallback := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libspu/s_sk.c/SpuSetKey := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/vm_key.c/_SsVmKeyOff := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/vm_aloc2.c/_SsVmDoAllocate := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/vm_aloc1.c/_SsVmAlloc := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgs/gs_004.c/GsSetOffset := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/midiread.c/_SsSeqPlay := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/midiread.c/_SsSeqGetEof := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/ssopenq.c/SsSeqOpen := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libspu/s_m_int.c/_spu_gcSPU := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/sstick.c/SsSetTickMode := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgs/gs_123.c/Gssub_make_matrix := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/vm_pb.c/_SsVmPitchBend := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpress/build.c/DecDCTvlcBuild := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libetc/intr.c/func_800B595C := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libetc/intr.c/func_800B5A34 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libetc/intr.c/func_800B5C04 := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libsnd/ut_cp.c/SsUtChangePitch := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/vm_vsu.c/_SsVmVSetUp := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpad/padcmd.c/_padLoadActInfo := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpad/padcmd.c/func_800D3180 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpad/padcmd.c/_padRecvAtLoadInfo := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpad/padcmd.c/func_800D2E88 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgs/gs_002.c/GsSetDrawBuffOffset := tools/gcc/gcc-2.7.2-psx/cc1
FN_CC1_src/syslib/libgs/gs_003.c/GsSetDrawBuffClip := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/vm_n2p.c/note2pitch := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libcd/cdread.c/CdReadBreak := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpad/padseqd.c/func_800D44E0 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/ssvol.c/_SsSndSetVol := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libsnd/ssvol.c/SsSepSetVol := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/ResetGraph := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/SetDrawEnv := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/func_800CEAD8 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libgpu/sys.c/func_800CED08 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libpad/padportd.c/func_800D3CF4 := tools/gcc/gcc-2.8.0-psx/cc1
FN_CC1_src/syslib/libetc/intr_dma.c/setIntrDMA := tools/gcc/gcc-2.8.0-psx/cc1
endif
