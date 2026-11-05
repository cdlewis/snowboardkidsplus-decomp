.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
.SECONDARY:

# Compact, colored output, matching snowboardkids-decomp.
ifndef QUIET
PRINTF = @printf
else
PRINTF = @true
endif
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
.SILENT:
endif

NO_COL := \033[0m
GREEN := \033[0;32m
YELLOW := \033[0;33m
PINK := \033[0;35m
CYAN := \033[0;36m

ROOT := $(CURDIR)
PY ?= .venv/bin/python
SPLAT ?= .venv/bin/splat
CROSS := $(or $(SKP_CROSS),mips-linux-gnu-)
AS := $(CROSS)as
LD := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
AR := $(CROSS)ar
CPP := $(or $(SKP_CPP),cpp)
CC1 := $(or $(SKP_CC1),tools/cc1psx.py)
SDK_CC1 := $(or $(SKP_SDK_CC1),tools/gcc/gcc-2.8.1-psx/cc1)
MASPSX := $(or $(SKP_MASPSX),tools/maspsx/maspsx.py)
ASPSX_VERSION := $(or $(SKP_ASPSX_VERSION),2.67)
SDK_ASPSX_VERSION := $(or $(SKP_SDK_ASPSX_VERSION),2.77)
G_VALUE := $(or $(SKP_G_VALUE),0)
BUILD_DIR ?= build
COMPARISON ?= 0
NON_MATCHING ?= 0
EXTRA_CC_FLAGS ?=

CPP_FLAGS := -nostdinc -undef -D__GNUC__=2 -D__OPTIMIZE__ -Dmips -D__mips__ -D__psx__ -DPSX -DINCLUDE_ASM_USE_MACRO_INC=1 -I$(ROOT)/include
AS_FLAGS := -EL -march=r3000 -mtune=r3000 -I $(ROOT)/include -I $(ROOT)/src -I $(ROOT)/build

include config/compiler.mk

# Isolated matching tools query these same settings.
unit_g = $(or $(UNIT_G_$(1)),$(G_VALUE))
unit_cc_g = $(or $(UNIT_CC_G_$(1)),$(call unit_g,$(1)))
unit_aspsx = $(or $(UNIT_ASPSX_$(1)),$(if $(filter src/syslib/%,$(1)),$(SDK_ASPSX_VERSION),$(ASPSX_VERSION)))
unit_cc1 = $(or $(UNIT_CC1_$(1)),$(if $(filter src/syslib/%,$(1)),$(SDK_CC1),$(CC1)))
cc1_dependencies = $(1) $(if $(filter tools/cc1psx.py,$(1)),tools/gcc/gcc-2.7.2-cdk/cc1)
unit_ccflags = -quiet -O2 -G$(call unit_cc_g,$(1)) -fgnu-linker -mcpu=3000 $(UNIT_CCFLAGS_$(1)) $(EXTRA_CC_FLAGS)
unit_asflags = $(AS_FLAGS) $(if $(filter 1,$(UNIT_PAD_$(1))),,-no-pad-sections) -G$(call unit_g,$(1)) $(if $(filter 1,$(UNIT_REORDER_$(1))),-O2)
unit_maspsx = --aspsx-version=$(call unit_aspsx,$(1)) --expand-div -G$(call unit_g,$(1)) $(if $(filter 1,$(UNIT_REORDER_$(1))),--gas-reorder) $(if $(filter 1,$(UNIT_INDEXED_$(1))),--gas-indexed-loads)
unit_functions = $(if $(ONLY_FUNCTION),$(filter $(ONLY_FUNCTION),$(UNIT_FUNCTIONS_$(1))),$(UNIT_FUNCTIONS_$(1)))
function_cc1 = $(or $(FN_CC1_$(1)/$(2)),$(call unit_cc1,$(1)))
unit_source = $(if $(SOURCE),$(SOURCE),$(1))
unit_output = $(BUILD_DIR)/$(patsubst %.c,%,$(1))

ifeq ($(NON_MATCHING),1)
ifneq ($(COMPARISON),1)
ifneq ($(MAKECMDGOALS),objects)
$(error NON_MATCHING=1 requires COMPARISON=1; partial objects must not enter the executable)
endif
endif
CPP_FLAGS += -DNON_MATCHING
endif

# Splat supplies exact linker inputs. Remaking this included dependency file
# restarts Make, discovering newly generated assembly after a clean checkout.
SPLIT_INPUTS := snowboardkidsplus.yaml symbol_addrs.txt reloc_addrs.txt disc/SLPS_018.23 Makefile
SPLIT_OUTPUTS := linkers/slps_018.23.ld linkers/slps_018.23.d linkers/undefined_funcs_auto.txt linkers/undefined_syms_auto.txt include/include_asm.h include/macro.inc include/labels.inc include/gte_macros.inc
.PHONY: all verify objects all-objects expected split extract clean hooks report badges scratch print-settings FORCE
ifeq ($(COMPARISON),1)
ifeq ($(abspath $(BUILD_DIR)),$(ROOT)/build)
$(error comparison objects require a separate BUILD_DIR, such as build/compare)
endif
all: all-objects
else
ifeq ($(filter print-settings,$(MAKECMDGOALS)),)
ifneq ($(strip $(UNIT) $(SOURCE) $(ONLY_FUNCTION) $(EXTRA_CC_FLAGS)),)
$(error scratch overrides require COMPARISON=1 and a separate BUILD_DIR)
endif
endif
all: verify
endif

ifneq ($(COMPARISON),1)
ifeq ($(filter clean extract hooks print-settings scratch objects badges,$(MAKECMDGOALS)),)
include linkers/slps_018.23.d
endif
endif

# Splat preserves unchanged outputs; refresh the included .d timestamp so Make
# does not restart forever after an input change that leaves its contents intact.
$(SPLIT_OUTPUTS) &: $(SPLIT_INPUTS)
	$(PRINTF) "[$(CYAN) splat  $(NO_COL)]  Extracting snowboardkidsplus.yaml\n"
	$(SPLAT) split snowboardkidsplus.yaml
	# Make depends on archives; archive:member selectors are only valid for ld.
	sed -i -E 's/(\.a):[^[:space:]:]+/\1/g' linkers/slps_018.23.d
	@touch $(SPLIT_OUTPUTS)

split: $(SPLIT_OUTPUTS)

extract:
	$(PRINTF) "[$(CYAN)extract $(NO_COL)]  Extracting disc executable and SNB/SNB.DAT\n"
	$(PY) tools/extract_iso.py "Snobow Kids Plus (Japan)/Snobow Kids Plus (Japan).bin" disc

disc/SLPS_018.23:
	$(MAKE) extract

# Invalidate objects when resolved environment/command-line settings change.
# Preserve the timestamp on repeated invocations with the same configuration.
CONFIG_VARIABLES = PY CPP CC1 SDK_CC1 AS LD OBJCOPY MASPSX ASPSX_VERSION SDK_ASPSX_VERSION G_VALUE CPP_FLAGS AS_FLAGS COMPARISON NON_MATCHING SOURCE ONLY_FUNCTION EXTRA_CC_FLAGS $(sort $(filter UNIT_% FN_%,$(.VARIABLES)))
$(BUILD_DIR)/.directory:
	@mkdir -p $(@D)
	@touch $@

$(BUILD_DIR)/.config: FORCE | $(BUILD_DIR)/.directory
	$(file >$@.tmp,$(foreach name,$(CONFIG_VARIABLES),$(name)=$($(name))$(newline)))
	@cmp -s $@.tmp $@ && rm $@.tmp || mv $@.tmp $@

define newline


endef
FORCE:

C_SOURCES := $(if $(UNIT),$(UNIT),$(shell find src -name '*.c' -type f | sort))
LINK_OBJECTS := $(shell sed -n 's/^[[:space:]]*\(build\/[^[:space:]]*\.[oa]\).*/\1/p' linkers/slps_018.23.d 2>/dev/null)
SDK_LIBRARIES := $(sort $(filter %.a,$(LINK_OBJECTS)))
C_OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
HASM_SOURCES := $(shell find src -name '*.s' -type f | sort)
HASM_OBJECTS := $(addprefix $(BUILD_DIR)/,$(HASM_SOURCES:.s=.o))
ASM_OBJECTS := $(filter build/asm/%,$(LINK_OBJECTS))

# Splat selects each archive member and section in snowboardkidsplus.yaml.
SDK_OBJECT_NAMES := $(shell $(PY) tools/psyq_sdk.py --list-objects)
SDK_LIBRARY_INPUTS := $(shell $(PY) tools/psyq_sdk.py --list-library-inputs)
SDK_NAMES := $(sort $(foreach object,$(SDK_OBJECT_NAMES),$(firstword $(subst /, ,$(object)))))
build/sdk/psyq42/%.o: tools/psyq_sdk.py tools/decomp.py snowboardkidsplus.yaml lib/psyq-sdk/PROVENANCE.json symbol_addrs.txt $(wildcard $(SDK_LIBRARY_INPUTS) tools/psyq-sdk/psyq-obj-parser)
	$(PY) tools/psyq_sdk.py --object $* --output $@ --objcopy $(OBJCOPY)

define sdk_archive_rule
build/sdk/psyq42/$(1).a: $(addprefix build/sdk/psyq42/,$(addsuffix .o,$(filter $(1)/%,$(SDK_OBJECT_NAMES)))) snowboardkidsplus.yaml lib/psyq-sdk/PROVENANCE.json
	rm -f $$@.tmp
	$(AR) rcsD $$@.tmp $$(filter %.o,$$^)
	mv $$@.tmp $$@
endef
$(foreach library,$(SDK_NAMES),$(eval $(call sdk_archive_rule,$(library))))

# Alternate compilations are separate targets. The merger selects existing
# .ent/.end regions without invoking compilers or changing instructions.
define alternate_rule
$(call unit_output,$(1)).$(2).s: $(call unit_output,$(1)).i $(call cc1_dependencies,$(call function_cc1,$(1),$(2)))
	$(call function_cc1,$(1),$(2)) $(call unit_ccflags,$(1)) $(FN_FLAGS_$(1)/$(2)) $$< -o $$@
endef

define c_rule
$(call unit_output,$(1)).i: $(call unit_source,$(1)) Makefile config/compiler.mk $(BUILD_DIR)/.config
	@mkdir -p $$(@D)
	$(PRINTF) "[$(GREEN)   c    $(NO_COL)]  $(call unit_source,$(1))\n"
	$(CPP) $(CPP_FLAGS) -I$(ROOT)/$(dir $(1)) -MMD -MP -MT $$@ -MF $(call unit_output,$(1)).d $(abspath $(call unit_source,$(1))) -o $$@

$(call unit_output,$(1)).base.s: $(call unit_output,$(1)).i $(call cc1_dependencies,$(call unit_cc1,$(1)))
	$(call unit_cc1,$(1)) $(call unit_ccflags,$(1)) $$< -o $$@

$(call unit_output,$(1)).s: $(call unit_output,$(1)).base.s $(foreach name,$(call unit_functions,$(1)),$(call unit_output,$(1)).$(name).s) $(if $(call unit_functions,$(1)),tools/merge_functions.py)
	$(if $(call unit_functions,$(1)),$(PY) tools/merge_functions.py $$< $$@ $(foreach name,$(call unit_functions,$(1)),$(name)=$(call unit_output,$(1)).$(name).s),cp $$< $$@)

$(call unit_output,$(1)).masp.s: $(call unit_output,$(1)).s $(MASPSX) $(wildcard tools/maspsx/maspsx/*.py)
	$(PY) $(MASPSX) $(call unit_maspsx,$(1)) < $$< > $$@

$(call unit_output,$(1)).o: $(call unit_output,$(1)).masp.s
	$(AS) $(call unit_asflags,$(1)) --MD $(call unit_output,$(1)).as.d -o $$@ $$<

$(foreach name,$(call unit_functions,$(1)),$(eval $(call alternate_rule,$(1),$(name))))
endef
$(foreach unit,$(filter-out src/syslib/%,$(C_SOURCES)),$(foreach name,$(call unit_functions,$(unit)),$(if $(filter-out $(call unit_cc1,$(unit)),$(call function_cc1,$(unit),$(name))),$(error function-specific compiler selection is restricted to SDK units))))
$(foreach unit,$(filter-out src/syslib/%,$(C_SOURCES)),$(if $(filter-out $(CC1),$(call unit_cc1,$(unit))),$(error game units must share CC1; $(unit) selects $(call unit_cc1,$(unit)))))
$(foreach unit,$(C_SOURCES),$(eval $(call c_rule,$(unit))))

$(BUILD_DIR)/asm/%.o: asm/%.s Makefile config/compiler.mk $(BUILD_DIR)/.config
	@mkdir -p $(@D)
	$(PRINTF) "[$(GREEN)   as   $(NO_COL)]  $<\n"
	$(AS) $(AS_FLAGS) -no-pad-sections -G$(G_VALUE) --MD $@.d -o $@ $<

$(BUILD_DIR)/src/%.o: src/%.s Makefile config/compiler.mk $(BUILD_DIR)/.config
	@mkdir -p $(@D)
	$(PRINTF) "[$(GREEN)  hasm  $(NO_COL)]  $<\n"
	$(AS) $(AS_FLAGS) -no-pad-sections -G$(G_VALUE) --MD $@.d -o $@ $<

ifneq ($(COMPARISON),1)
build/slps_018.23.elf: $(C_OBJECTS) $(HASM_OBJECTS) $(ASM_OBJECTS) linkers/slps_018.23.ld linkers/undefined_funcs_auto.txt linkers/undefined_syms_auto.txt
	$(PRINTF) "[$(PINK) linker $(NO_COL)]  Linking $@\n"
	$(LD) -EL -nostdlib -T linkers/slps_018.23.ld -T linkers/undefined_funcs_auto.txt -T linkers/undefined_syms_auto.txt -Map build/slps_018.23.map -o $@ --whole-archive --start-group $(SDK_LIBRARIES) --end-group --no-whole-archive

build/slps_018.23.exe: build/slps_018.23.elf
	$(PRINTF) "[$(CYAN) objcpy $(NO_COL)]  $<\n"
	$(OBJCOPY) -O binary $< $@

verify: build/slps_018.23.exe
	$(PRINTF) "[$(GREEN) verify $(NO_COL)]  Checking executable SHA1\n"
	$(PY) tools/verify_image.py
	$(PY) tools/psyq_sdk.py --verify

endif

# Comparison objects cannot overwrite the objects used by the verified image.
objects:
	+$(MAKE) COMPARISON=1 BUILD_DIR=build/compare all-objects
all-objects: $(C_OBJECTS) $(HASM_OBJECTS)
scratch: $(C_OBJECTS)

ifneq ($(COMPARISON),1)
expected: verify
	$(PRINTF) "[$(CYAN) expect $(NO_COL)]  Updating verified target objects\n"
	$(PY) tools/update_expected.py
else
verify expected:
	$(error executable verification and snapshots require the normal build)
endif

print-settings:
	@printf '%s\n' 'CC1=$(call unit_cc1,$(UNIT))' 'CPP=$(CPP)' 'AS=$(AS)' 'LD=$(LD)' 'CROSS=$(CROSS)' 'CPP_FLAGS=$(CPP_FLAGS)' 'AS_FLAGS=$(call unit_asflags,$(UNIT))' 'ORACLE_AS_FLAGS=$(AS_FLAGS) -no-pad-sections -G$(call unit_g,$(UNIT))' 'CC1_FLAGS=$(call unit_ccflags,$(UNIT))' 'G_VALUE=$(call unit_g,$(UNIT))' 'UNIT_CC_FLAGS=$(UNIT_CCFLAGS_$(UNIT))' 'FUNCTION_FLAGS=$(FN_FLAGS_$(UNIT)/$(ONLY_FUNCTION))' 'FUNCTION_CC1=$(call function_cc1,$(UNIT),$(ONLY_FUNCTION))' 'CC_G_VALUE=$(call unit_cc_g,$(UNIT))' 'ASPSX_VERSION=$(call unit_aspsx,$(UNIT))' 'REORDER=$(UNIT_REORDER_$(UNIT))' 'INDEXED=$(UNIT_INDEXED_$(UNIT))' 'PAD=$(UNIT_PAD_$(UNIT))'

clean:
	$(PRINTF) "[$(YELLOW) clean  $(NO_COL)]  Removing generated files\n"
	rm -rf build asm
	rm -f $(SPLIT_OUTPUTS)

hooks:
	$(PRINTF) "[$(CYAN) hooks  $(NO_COL)]  Installing Git hooks\n"
	tools/install-git-hooks.sh

report: verify
	$(PRINTF) "[$(CYAN) report $(NO_COL)]  Generating progress report\n"
	$(PY) -m mapfile_parser objdiff_report

badges:
	./tools/build-and-verify.sh
	$(PY) tools/update_badges.py

-include $(C_OBJECTS:.o=.d) $(C_OBJECTS:.o=.as.d) $(addsuffix .d,$(HASM_OBJECTS) $(ASM_OBJECTS))
