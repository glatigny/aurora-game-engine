# Flags
DEBUG_CFLAGS = -O0 -mcpu=G4 -pipe -rdynamic -D_GLIBCXX_DEBUG -Wall -Wno-multichar
DEBUG_CXXFLAGS = $(DEBUG_CFLAGS)
DEBUG_LDFLAGS = -lz -lpng -lm $(AURORA_LDFLAGS)

PROF_CFLAGS = -O2 -mcpu=G4 -pipe -rdynamic -ffast-math -Wall -Wno-multichar
PROF_CXXFLAGS = $(PROF_CFLAGS)
PROF_LDFLAGS = -lz -lpng -lm $(AURORA_LDFLAGS)

FINAL_CFLAGS = -O2 -mcpu=G4 -fomit-frame-pointer -pipe -ffast-math -Wall -Wno-multichar
FINAL_CXXFLAGS = $(FINAL_CFLAGS)
FINAL_LDFLAGS = -lz -lpng -lm -O $(AURORA_LDFLAGS)

include $(MK_PATH)/pspec/linux-common.mk
