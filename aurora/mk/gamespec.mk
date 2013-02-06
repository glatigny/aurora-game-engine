# This file is included by ../aoe.mk.
#
# This makefile builds the game-specific part of the engine.
# If you wish to replace this file by your own build system, you must ensure
# that the file config.mk is correctly generated (see the $(CONFIG_MAKEFILE)
# rule below to know what to put inside this file).
#
# Once config.mk is generated, just call make in the aurora library path and
# you will get libaurora.a / libaurora.lib, depending on your plateform.

###########################################################
# Makefile dependencies
#

MK_PATH = $(AURORA_ENGINE_PATH)/mk

# Include common parts of the makefile
include $(MK_PATH)/pspec.mk

aoe.mk: Makefile
$(MK_PATH)/gamespec.mk: aoe.mk
$(MK_PATH)/pspec.mk: $(MK_PATH)/gamespec.mk
$(MK_PATH)/common.mk: $(MK_PATH)/gamespec.mk

# Update the config makefile
CONFIG_MAKEFILE = $(MK_PATH)/config.mk
$(CONFIG_MAKEFILE):: $(MK_PATH)/gamespec.mk aoe.mk
	@echo " [CUSTOM] $(notdir $@)"
	@rm -f $@
	@echo "# Automatically generated file. Please do not modify." >> $@
	@echo "AURORA_WINDOW_TITLE = $(AURORA_WINDOW_TITLE)" >> $@
	@echo "AURORA_PROJECT_NAME = $(AURORA_PROJECT_NAME)" >> $@
	@echo "AURORA_ENGINE_VENDOR = $(AURORA_ENGINE_VENDOR)" >> $@
	@echo "AURORA_ENGINE_VERSION = $(AURORA_ENGINE_VERSION)" >> $@
	@echo "AURORA_ENGINE_REVISION = $(AURORA_ENGINE_REVISION)" >> $@
	@echo "AURORA_ENGINE_URL = $(AURORA_ENGINE_URL)" >> $@
	@echo "AURORA_ENGINE_PATH ?= $(AURORA_ENGINE_PATH)" >> $@
	@echo "GAMESPEC_PATH = $(GAMESPEC_PATH)" >> $@
	@[ -n "$(FACTORY_EXTENSION)" ] && echo "FACTORY_EXTENSION = $(FACTORY_EXTENSION)" >> $@ || true
	@[ -n "$(FACTORY_EXTENSION)" ] && echo "XH_EXTENSION = $(XH_EXTENSION)" >> $@ || true
	@echo "AURORA_DEFINES = $(AURORA_DEFINES)" >> $@
	@echo "AURORA_MODULES = $(AURORA_MODULES)" >> $@
	@[ -n "$(AOE_MAX_MEMORY)" ] && echo "AOE_MAX_MEMORY = $(AOE_MAX_MEMORY)" >> $@ || true
	@[ -n "$(AOE_MAX_VIDEO_MEMORY)" ] && echo "AOE_MAX_VIDEO_MEMORY = $(AOE_MAX_VIDEO_MEMORY)" >> $@ || true
ifdef OPTIMIZE
	@echo "OPTIMIZE = 1" >> $@
endif
ifdef PROFILING
	@echo "PROFILING = 1" >> $@
endif
ifdef DEBUG_SYMBOLS
	@echo "DEBUG_SYMBOLS = 1" >> $@
endif

include $(MK_PATH)/common.mk

ifdef WINEBUILD
ifndef WINEPREFIX
# Prepare and use default build directory
WINEPREFIX := $(PWD)/win32/build
endif
$(shell $(TOOLS) dosdevices)
export WINEPREFIX
endif

###########################################################
# Standard variables
#

ifdef WIN32
prefix ?= $(ProgramFiles)/$(PROJECT)
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)
datarootdir ?= $(prefix)
datadir ?= $(prefix)
else
prefix ?= /usr/local
exec_prefix ?= $(prefix)
libexecdir ?= $(exec_prefix)/libexec
bindir ?= $(exec_prefix)/bin
datarootdir ?= $(prefix)/share
datadir ?= $(datarootdir)
endif


###########################################################
# Variables needed by the aurora build system
#

# Main files
BIN = $(PROJECT)$(BINEXT)

# Add module-specific files to the source list
SRC += $(foreach s, $(AURORA_MODULES), $(SRC_$(s)))

# File lists
SRCLIST = $(addprefix src/,$(SRC))
PRE = $(addsuffix .i,$(basename $(SRCLIST)))
ASM = $(addsuffix .s,$(basename $(SRCLIST)))
OBJ = $(SUPP_OBJ) $(addsuffix $(OBJEXT),$(basename $(SRCLIST)))

# Windows-specific files
ifdef WIN32
VCPROJ = $(PROJECT).vcproj
ICO = $(PROJECT).ico
endif

# Setup directories
INCLUDE_DIRS += src
STARTSCRIPTINSTALLDIR = $(bindir)
BININSTALLDIR = $(libexecdir)/$(PROJECT)
DATAINSTALLDIR = $(datadir)/$(PROJECT)

# Cflags requirements
ifdef WIN32
CFLAGS += $(foreach m,$(AURORA_MODULES),$(WIN32_$(m)_CFLAGS))
else
CFLAGS += $(foreach m,$(AURORA_MODULES),$($(m)_CFLAGS))
endif

# Linking requirements
WIN32_AURORA_LDFLAGS += $(foreach m,$(AURORA_MODULES),$(WIN32_$(m)_LDFLAGS))
AURORA_LDFLAGS += $(foreach m,$(AURORA_MODULES),$($(m)_LDFLAGS))

###########################################################
# Rules

bin: $(VCPROJ) $(BIN)

ifdef WIN32

%.dll: win32/dll/%.dll
	@echo " [CP] $@"
	@-cp -f "$<" "$@"
	@touch "$@"

DLL += ogg.dll
DLL += SDL.dll
DLL += vorbis.dll
DLL += vorbisenc.dll
DLL += vorbisfile.dll
DLL += zlib1.dll

mrproper::
	@echo " [RM] DLL"
	@rm -f $(DLL)
	@echo " [RM] $(basename $(notdir $(BIN))).pdb"
	@rm -f "$(basename $(BIN)).pdb"
	@echo " [RM] $(basename $(notdir $(BIN))).ilk"
	@rm -f "$(basename $(BIN)).ilk"

dll: $(DLL)

run: $(BIN) $(DLL)
ifdef WINEBUILD
	@wine $(BIN)
else
	@$(BIN)
endif

else # WIN32

dll: ;
run: $(BIN)
	@./$(BIN)

endif # WIN32

final: bin
ifndef DEBUG_SYMBOLS
	@echo " [FINAL] The generated binary is optimized."
else
	@echo " [FINAL] The generated binary is optimized but not stripped."
endif
ifneq (,$(findstring NDEBUG,$(AURORA_DEFINES)))
	@echo " [FINAL] The generated binary has assertions disabled."
else
	@echo " [FINAL] The generated binary has assertions enabled."
endif

profile: bin
	@echo " [FINAL] The generated binary is for profiling."
ifneq (,$(findstring NDEBUG,$(AURORA_DEFINES)))
	@echo " [FINAL] The generated binary has assertions disabled."
else
	@echo " [FINAL] The generated binary has assertions enabled."
endif

ifndef WIN32
install: $(BIN)
	@echo " [INSTALL] $(BIN)"
	@$(TOOLS) install "$(BIN)" "$(BININSTALLDIR)"
	@$(TOOLS) genstartscript "$(BIN)" "$(STARTSCRIPTINSTALLDIR)/$(BIN)"
	@$(TOOLS) installdir data "$(DATAINSTALLDIR)"
else
mrproper::
	@[ -e syslog.txt ] && echo " [RM] syslog.txt" && rm -f syslog.txt || true
install:
	@echo " [INSTALL] ??? What ?"
	@echo "make install under Windows makes no sense at all."
	@echo "One day, this command will generate then call the installer."
endif

doc:
	@echo " [DOXYGEN] Generating documentation"
	@cd $(AURORA_ENGINE_PATH); "$(MAKE)" doc

dep:
	@rm -f Makefile.dep
	@"$(MAKE)" Makefile.dep
	@cd $(AURORA_ENGINE_PATH); "$(MAKE)" dep


clean::
	@echo " [RM] *$(OBJEXT)"
	@rm -f $(OBJ)
	@echo " [CLEAN] rc"
	@rm -f $(PROJECT).rc $(PROJECT).res
	@cd $(AURORA_ENGINE_PATH); "$(MAKE)" clean
	@echo " [RM] Makefile.dep"
	@rm -f Makefile.dep
	@[ -d win32/build/dosdevices ] && rm -f win32/build/dosdevices/* || true


mrproper:: $(CONFIG_HEADER) $(CONFIG_MAKEFILE) clean
	@cd $(AURORA_ENGINE_PATH); "$(MAKE)" mrproper
	@echo " [RM] $(BIN)"
	@rm -f $(BIN)
	@echo " [RM] config.mk"
	@rm -f $(CONFIG_MAKEFILE)


eolclean: mrproper
	@echo " [TOOL] clean_eol"
	@"$(TOOLS)" clean_eol .


ifdef WIN32
$(PROJECT).rc: $(MK_PATH)/gamespec.mk
	@echo " [RC] $(notdir $@)"
	@rm -f $@
	@echo -ne "#include \"windows.h\"\\r\\n" > $@
	@[ -f "$(PROJECT).ico" ]&& echo -ne "ICO1 ICON $(PROJECT).ico\\r\\n" || true >> $@
	@echo -ne "VS_VERSION_INFO VERSIONINFO\\r\\n FILEVERSION 0,${AURORA_ENGINE_VERSION},${AURORA_ENGINE_REVISION},0\\r\\n PRODUCTVERSION 0,${AURORA_ENGINE_VERSION},${AURORA_ENGINE_REVISION},0\\r\\n FILEFLAGSMASK 0x17L\\r\\n#ifdef _DEBUG\\r\\n FILEFLAGS 0x1L\\r\\n#else\\r\\n FILEFLAGS 0x0L\\r\\n#endif\\r\\n" >> $@
	@echo -ne "FILEOS 0x4L\\r\\n FILETYPE 0x1L\\r\\n FILESUBTYPE 0x0L\\r\\nBEGIN\\r\\nBLOCK \"StringFileInfo\"\\r\\nBEGIN\\r\\n" >> $@
	@echo -ne "BLOCK \"000004b0\"\\r\\nBEGIN\\r\\nVALUE \"Comments\", \"${AURORA_ENGINE_VENDOR}\"\\r\\nVALUE \"FileDescription\", \"${AURORA_PROJECT_NAME}\"\\r\\nVALUE \"FileVersion\", \"0, ${AURORA_ENGINE_VERSION}, ${AURORA_ENGINE_REVISION}, 0\"\\r\\n" >> $@
	@echo -ne "VALUE \"LegalCopyright\", \"(c) ${AURORA_ENGINE_VENDOR}\" VALUE \"OriginalFilename\", \"${PROJECT}.exe\"\\r\\nVALUE \"ProductName\", \"${AURORA_PROJECT_NAME}\"\\r\\nVALUE \"ProductVersion\", \"0, ${AURORA_ENGINE_VERSION}, ${AURORA_ENGINE_REVISION}, 0\"\\r\\nEND\\r\\n" >> $@
	@echo -ne "BLOCK \"040c04b0\"\\r\\nBEGIN\\r\\nVALUE \"Comments\", \"${AURORA_ENGINE_VENDOR}\"\\r\\nVALUE \"FileDescription\", \"${AURORA_PROJECT_NAME}\"\\r\\nVALUE \"FileVersion\", \"0, ${AURORA_ENGINE_VERSION}, ${AURORA_ENGINE_REVISION}, 0\"\\r\\n" >> $@
	@echo -ne "VALUE \"LegalCopyright\", \"(c) ${AURORA_ENGINE_VENDOR}\" VALUE \"OriginalFilename\", \"${PROJECT}.exe\"\\r\\nVALUE \"ProductName\", \"${AURORA_PROJECT_NAME}\"\\r\\nVALUE \"ProductVersion\", \"0, ${AURORA_ENGINE_VERSION}, ${AURORA_ENGINE_REVISION}, 0\"\\r\\nEND\\r\\n" >> $@
	@echo -ne "END\\r\\nBLOCK \"VarFileInfo\"\\r\\nBEGIN\\r\\nVALUE \"Translation\", 0x0, 1200, 0x40c, 1200\\r\\nEND\\r\\n" >> $@
	@echo -ne "END\\r\\n" >> $@
    
%.res: %.rc
	@echo " [RES] $(notdir $@)"
	@$(RC) -fo $@ -r $<

vcproj: $(VCPROJ)

$(VCPROJ): $(TOOLS) $(AURORA_ENGINE_PATH)/Makefile Makefile
	@$(TOOLS) vcproj "$(PROJECT)" "$(VCPROJ)" "." "src" "$(AURORA_ENGINE_PATH)" "src" "data" 

mrproper::
	@echo " [RM] $(notdir $(VCPROJ))"
	@rm -f $(VCPROJ) $(VCPROJ).${USERDOMAIN}.${USERNAME}.user
	@echo " [RM] $(basename $(notdir $(VCPROJ))).suo"
	@rm -f "$(basename $(VCPROJ)).suo"
	@echo " [RM] $(basename $(notdir $(VCPROJ))).sln"
	@rm -f "$(basename $(VCPROJ)).sln"
	@echo " [RM] $(basename $(notdir $(VCPROJ))).ncb"
	@rm -f "$(basename $(VCPROJ)).ncb"

# Binary (exe) generation for Windows
$(BIN): $(LIBAURORA) $(OBJ) $(PROJECT).res
	@echo " [LD] $(notdir $@)"
	@$(LD) $(LDFLAGS) $(LDOUTFLAG)$@ $^

else # !WIN32

# Binary generation for GNU
$(BIN):: $(OBJ) $(LIBAURORA)
	@echo " [LD] $(notdir $@)"
	$(LD) $(LDFLAGS) $(LDOUTFLAG)$@ $^

ifdef OPTIMIZE
ifndef DEBUG_SYMBOLS
$(BIN)::
	@echo " [STRIP] $(notdir $@)"
	@strip "$@"
endif # DEBUG_SYMBOLS
endif # OPTIMIZE

endif # WIN32


# Library generation rule
lib: $(LIBAURORA)
libaurora_make: $(CONFIG_HEADER) $(CONFIG_MAKEFILE)
	@cd "$(AURORA_ENGINE_PATH)"; "$(MAKE)"
$(LIBAURORA): libaurora_make $(MK_PATH)/pspec.mk
	@echo " [CHECK] $(notdir $@)"

# Factory extension
ifdef FACTORY_EXTENSION
ifdef XH_EXTENSION

$(FACTORY_EXTENSION): $(MK_PATH)/gamespec.mk Makefile
	@echo " [CUSTOM] $(notdir $@)"
	@rm -f $@
	@touch $@
	@for f in $(addsuffix .h,$(basename $(SRC))); do [ -e "src/$${f%.h}.xh" ] && echo "#include \"$$f\"" >> $@ || true ; done
	
$(XH_EXTENSION): $(MK_PATH)/gamespec.mk Makefile
	@echo " [CUSTOM] $(notdir $@)"
	@rm -f $@
	@touch $@
	@for f in $(addsuffix .xh,$(basename $(SRC))); do \
		if [ -e "src/$$f" ]; then \
			echo -e "#define HEADER $${f%%.xh}\n#define HELPER LIST_HELPER\n#include \"include_helper.h\"\n" >> $@ ;\
		else \
			true ;\
		fi ;\
	done

$(CONFIG_MAKEFILE):: $(FACTORY_EXTENSION) $(XH_EXTENSION)
mrproper::
	@echo " [RM] $(notdir $(FACTORY_EXTENSION))"
	@rm -f $(FACTORY_EXTENSION)
	@echo " [RM] $(notdir $(XH_EXTENSION))"
	@rm -f $(XH_EXTENSION)

endif # XH_EXTENSION
endif # FACTORY_EXTENSION


# Dependency makefile
ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring mrproper,$(MAKECMDGOALS)))
Makefile.dep:: Makefile $(CONFIG_HEADER) $(TOOLS)
	@echo " [DEP] $(notdir $@)"
	@$(TOOLS) make_dep $(OBJEXT) "$@" $(DEP_INCLUDE_FLAGS) $(SRCLIST)
sinclude Makefile.dep
endif
endif

.PHONY: chdir all bin dep final profile lib libaurora_make doc clean mrproper eolclean tinyxml vcproj bin dosdevices dll
