# This file should be included from inside libaurora.

AURORA_ENGINE_PATH = .
MK_PATH = $(AURORA_ENGINE_PATH)/mk

CONFIG_MAKEFILE = $(MK_PATH)/config.mk
$(CONFIG_MAKEFILE):: $(MK_PATH)/lib.mk
$(MK_PATH)/pspec.mk: $(MK_PATH)/lib.mk
$(MK_PATH)/common.mk: $(MK_PATH)/lib.mk

#ifeq (,$(findstring clean,$(MAKECMDGOALS)))
#ifeq (,$(findstring mrproper,$(MAKECMDGOALS)))
sinclude $(CONFIG_MAKEFILE)
#endif
#endif

include $(MK_PATH)/pspec.mk
include $(MK_PATH)/common.mk

ARCHIVE = $(LIBAURORA)

###########################################################
# Variables
#

# Include paths
ifdef GAMESPEC_PATH
ifdef FACTORY_EXTENSION
INCLUDE_DIRS += $(GAMESPEC_PATH)/$(dir $(FACTORY_EXTENSION))
endif
ifdef XH_EXTENSION
INCLUDE_DIRS += $(GAMESPEC_PATH)/$(dir $(XH_EXTENSION))
endif
endif

# Module-specific files
SRC += $(foreach s, $(AURORA_MODULES), $(SRC_$(s)))

ifdef WIN32
SRC += $(WIN32_SRC)
SRC += $(foreach s, $(AURORA_MODULES), $(WIN32_SRC_$(s)))
endif

# File lists
SRCLIST = $(addprefix src/,$(SRC))
PRE = $(addsuffix .i,$(basename $(SRCLIST)))
ASM = $(addsuffix .s,$(basename $(SRCLIST)))
OBJ = $(SUPP_OBJ) $(addsuffix $(OBJEXT),$(basename $(SRCLIST)))


###########################################################
# Rules

lib: $(ARCHIVE)

dep:
	@rm -f Makefile.dep
	@"$(MAKE)" Makefile.dep

doc:
	@echo " [DOXYGEN] Generating documentation"
	@mkdir -p doc/doxygen
	@cd src; doxygen


clean::
	@echo " [RM] $(notdir $(ARCHIVE))"
	@rm -f $(ARCHIVE)
	@echo " [RM] *$(OBJEXT)"
	@rm -f $(OBJ)
	@echo " [RM] Makefile.dep"
	@rm -f Makefile.dep


mrproper:: clean
	@echo " [RM] h_list.h"
	@rm -f src/h_list.h;
	@echo " [RM] xh_list.xh"
	@rm -f src/xh_list.xh;
	@echo " [RM] $(notdir $(CONFIG_MAKEFILE))"
	@rm -f $(CONFIG_MAKEFILE)
	@echo " [RM] $(notdir $(CONFIG_HEADER))"
	@rm -f $(CONFIG_HEADER)

test: lib

$(OBJ): src/h_list.h src/xh_list.xh

$(ARCHIVE): $(OBJ) Makefile
	@echo " [AR] $(notdir $@)"
	@$(AR) $(ARFLAGS) $(AROUTFLAG)$@ $(OBJ)

src/h_list.h: $(MK_PATH)/lib.mk Makefile $(CONFIG_MAKEFILE)
	@echo " [CUSTOM] $(notdir $@)"
	@rm -f $@
	@cd src; for f in $(addsuffix .h,$(basename $(SRC))); do [ -e "$${f%.h}.xh" ] && echo "#include \"$$f\"" >> h_list.h || true ; done
	@echo '#ifdef FACTORY_EXTENSION' >> $@
	@echo '#include FACTORY_EXTENSION' >> $@
	@echo '#endif' >> $@
	
src/xh_list.xh: $(MK_PATH)/lib.mk Makefile $(CONFIG_MAKEFILE)
	@echo " [CUSTOM] $(notdir $@)"
	@rm -f $@
	@for f in $(addsuffix .xh,$(basename $(SRC))); do \
		if [ -e "src/$$f" ]; then \
			echo -e "#define HEADER $${f%%.xh}\n#define HELPER LIST_HELPER\n#include \"include_helper.h\"\n" >> $@ ;\
		else \
			true ;\
		fi ;\
	done
	@echo '#ifdef XH_EXTENSION' >> $@
	@echo '#include XH_EXTENSION' >> $@
	@echo '#endif' >> $@
	@echo '#undef LIST_HELPER' >> $@

Makefile: $(MK_PATH)/lib.mk
ifdef GAMESPEC_PATH
$(MK_PATH)/config.mk:: $(GAMESPEC_PATH)/aoe.mk
else
$(MK_PATH)/config.mk:: $(MK_PATH)/default_config.mk
	@echo " [CP] config.mk"
	@cp $(MK_PATH)/default_config.mk $(MK_PATH)/config.mk
endif

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring mrproper,$(MAKECMDGOALS)))
Makefile.dep:: Makefile src/h_list.h src/xh_list.xh $(CONFIG_HEADER) $(TOOLS)
	@echo " [DEP] $(notdir $@)"
	@$(TOOLS) make_dep $(OBJEXT) "$@" $(DEP_INCLUDE_FLAGS) $(SRCLIST)
sinclude Makefile.dep
endif
endif

tinyxml/tinystr$(OBJEXT) tinyxml/tinyxml$(OBJEXT) tinyxml/tinyxmlerror$(OBJEXT) tinyxml/tinyxmlparser$(OBJEXT): tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp Makefile


.PHONY: all lib doc clean mrproper dep dosdevices

