SHELL = /bin/bash

# Set optimize mode
ifndef OPTIMIZE
ifneq (,$(findstring final,$(MAKECMDGOALS)))
OPTIMIZE := 1
endif
else # OPTIMIZE
ifndef PROFILING
ifneq (,$(findstring profile,$(MAKECMDGOALS)))
PROFILING := 1
endif
endif # PROFILING
endif # OPTIMIZE

# Set UNIT_TEST
ifneq (,$(findstring test,$(MAKECMDGOALS)))
UNIT_TEST := 1
endif

ifdef OPTIMIZE
#  OPTIMIZED MODE
CFLAGS = $(FINAL_CFLAGS)
CXXFLAGS = $(FINAL_CXXFLAGS)
LDFLAGS = $(FINAL_LDFLAGS)
else # OPTIMIZE
ifdef PROFILING
#  PROFILING MODE
CFLAGS = $(PROF_CFLAGS)
CXXFLAGS = $(PROF_CXXFLAGS)
LDFLAGS = $(PROF_LDFLAGS)
else # PROFILING
#  DEBUG MODE
CFLAGS = $(DEBUG_CFLAGS)
CXXFLAGS = $(DEBUG_CXXFLAGS)
LDFLAGS = $(DEBUG_LDFLAGS)
endif # PROFILING
endif # OPTIMIZE

ifneq (,$(findstring NDEBUG,$(AURORA_DEFINES)))
CFLAGS += -DNDEBUG
endif

INCLUDE_DIRS += $(AURORA_ENGINE_PATH)/src
INCLUDE_DIRS += $(AURORA_ENGINE_PATH)/tinyxml

LIBAURORA = $(AURORA_ENGINE_PATH)/libaurora$(LIBEXT)
CONFIG_HEADER = $(AURORA_ENGINE_PATH)/src/config.h
GLOBAL_HEADERS += $(CONFIG_HEADER)
GLOBAL_HEADERS += $(AURORA_ENGINE_PATH)/src/aurora_consts.h
GLOBAL_HEADERS += $(AURORA_ENGINE_PATH)/src/globals.h

%$(OBJEXT):: %.c $(GLOBAL_HEADERS)
%$(OBJEXT):: %.cpp $(GLOBAL_HEADERS)
TOOLS = $(MK_PATH)/tools.sh


$(CONFIG_HEADER): $(CONFIG_MAKEFILE)
	@echo " [CUSTOM] $(notdir $@)"
	@rm -f "$@"
	@echo "/* Automatically generated configuration file. Please do not modify. */" >> "$@"
	@echo "#define AURORA_WINDOW_TITLE \"$(AURORA_WINDOW_TITLE)\"" >> "$@"
	@echo "#define AURORA_PROJECT_NAME \"$(AURORA_PROJECT_NAME)\"" >> "$@"
	@echo "#define AURORA_ENGINE_VENDOR \"$(AURORA_ENGINE_VENDOR)\"" >> "$@"
	@echo "#define AURORA_ENGINE_VERSION \"$(AURORA_ENGINE_VERSION)\"" >> "$@"
	@echo "#define AURORA_ENGINE_REVISION \"$(AURORA_ENGINE_REVISION)\"" >> "$@"
	@echo "#define AURORA_ENGINE_URL \"$(subst \\,\\\\,$(AURORA_ENGINE_URL))\"" >> "$@"
	@echo "#define AURORA_PROJECT_URL \"$(subst \\,\\\\,$(AURORA_PROJECT_URL))\"" >> "$@"
	@for def in AURORA_ENGINE $(AURORA_DEFINES); do echo "#define $${def}" >> "$@"; done
	@for mod in LIBAURORA $(AURORA_MODULES); do echo "#define $${mod}_SUPPORT" >> "$@"; done
	@[ -n "$(FACTORY_EXTENSION)" -a -n "$(GAMESPEC_PATH)" ] && echo "#define FACTORY_EXTENSION \"../$(GAMESPEC_PATH)/$(FACTORY_EXTENSION)\"" >> "$@" || true
	@[ -n "$(XH_EXTENSION)" -a -n "$(GAMESPEC_PATH)" ] && echo "#define XH_EXTENSION \"../$(GAMESPEC_PATH)/$(XH_EXTENSION)\"" >> "$@" || true
	@[ -n "$(AOE_MAX_MEMORY)" ] && echo "#define AOE_MAX_MEMORY ( $(AOE_MAX_MEMORY) * 1048576 )" >> $@ || true
	@[ -n "$(AOE_MAX_VIDEO_MEMORY)" ] && echo "#define AOE_MAX_VIDEO_MEMORY ( $(AOE_MAX_VIDEO_MEMORY) * 1048576 )" >> $@ || true
ifdef WIN32
	@echo "#ifndef WIN32" >> "$@"
	@echo "#define WIN32" >> "$@"
	@echo "#endif" >> "$@"
endif

%.mk:
	@echo " [CHECK] $(notdir $@)"
	@touch "$@"
