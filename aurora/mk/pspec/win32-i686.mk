# Windows / VC++

# Command-line tools
ifdef WINEBUILD
CC = wine cl.exe
CXX = $(CC)
LD = wine link.exe
AR = wine lib
CPP = wine cl.exe -E
RC = wine rc.exe
else # WINEBUILD
CC = cl.exe
CXX = $(CC)
LD = link.exe
AR = lib.exe
CPP = cl.exe -E
RC = rc.exe
endif # WINEBUILD
DEP = g++ -MM
LDOUTFLAG = -out:
AROUTFLAG = -out:

# Flags
ifdef WINEBUILD
# No debug under wine, MSPDB80.DLL is broken
DEBUG_CFLAGS = -DUNICODE -DWIN32 -EHsc -nologo -Wp64 -MTd
DEBUG_CXXFLAGS = $(DEBUG_CFLAGS)
DEBUG_LDFLAGS = $(WIN32_AURORA_LDFLAGS) $(WIN32_AURORA_LDFLAGS) /NODEFAULTLIB:libcmt

else # WINEBUILD

DEBUG_CFLAGS = -DUNICODE -DWIN32 -EHsc -nologo -MTd
DEBUG_CXXFLAGS = $(DEBUG_CFLAGS)
DEBUG_LDFLAGS = $(WIN32_AURORA_LDFLAGS) -debug $(WIN32_AURORA_LDFLAGS) /NODEFAULTLIB:libcmt

PROF_CFLAGS = -DUNICODE -DWIN32 -EHsc -nologo -Ox -Oy -TP -MT
PROF_CXXFLAGS = $(PROF_CFLAGS)
PROF_LDFLAGS = $(WIN32_AURORA_LDFLAGS) -debug -SUBSYSTEM:WINDOWS -OPT:REF -OPT:ICF -LTCG -MACHINE:X86

endif # WINEBUILD

FINAL_CFLAGS = -DUNICODE -DWIN32 -EHsc -nologo -Ox -Oy -TP -MT
FINAL_CXXFLAGS = $(FINAL_CFLAGS)
FINAL_LDFLAGS = $(WIN32_AURORA_LDFLAGS) -SUBSYSTEM:WINDOWS -OPT:REF -OPT:ICF -LTCG -MACHINE:X86 

ARFLAGS = 

INCLUDE_DIRS += win32/include
INCLUDE_DIRS += .

INCLUDE_FLAGS = $(addsuffix ",$(addprefix -I",$(INCLUDE_DIRS))) $(addsuffix ",$(addprefix -I",$(dir $(GLOBAL_HEADERS)))) $(addprefix -FI,$(notdir $(GLOBAL_HEADERS)))
DEP_INCLUDE_FLAGS = $(addprefix -I ,$(INCLUDE_DIRS)) $(addprefix -include ,$(GLOBAL_HEADERS))

ifndef WINEBUILD
ifdef DEBUG_SYMBOLS
DEBUG_CFLAGS += -Zi
PROF_CFLAGS += -Zi
FINAL_CFLAGS += -Zi
endif
endif

# Extensions
BINEXT = .exe
OBJEXT = .obj
LIBEXT = .lib
SHLIBEXT = .dll

# Module CFLAGS

# Module LDFLAGS
WIN32_DIRECTX_LDFLAGS += d3d9.lib d3dx9.lib dinput.lib dinput8.lib dxguid.lib advapi32.lib
WIN32_SDL_LDFLAGS += SDL.lib opengl32.lib
WIN32_PNG_LDFLAGS += libpng.lib zlib.lib
WIN32_VORBIS_LDFLAGS += vorbisenc.lib vorbisfile.lib ogg.lib
WIN32_OPENAL_LDFLAGS += OpenAL32.lib

# Windows-specific LDFLAGS
WIN32_AURORA_LDFLAGS += kernel32.lib user32.lib gdi32.lib ws2_32.lib winmm.lib

# Rules
%$(OBJEXT):: %.c
	@echo " [CC] $(notdir $<)"
	@$(CC) -c $(CFLAGS) $(INCLUDE_FLAGS) "$<" -Fo"$@"

%$(OBJEXT):: %.cpp
	@echo " [CXX] $(notdir $<)"
	@$(CXX) -c $(CXXFLAGS) $(INCLUDE_FLAGS) "$<" -Fo"$@"

%.i:: %.c
	@echo " [CC] $(notdir $<)"
	@$(CC) -I -c $(CFLAGS) $(INCLUDE_FLAGS) "$<" -Fo"$@"

%.i:: %.cpp
	@echo " [CXX] $(notdir $<)"
	@$(CXX) -I -c $(CXXFLAGS) $(INCLUDE_FLAGS) "$<" -Fo"$@"

mrproper::
	@echo " [RM] vc80.pdb"
	@rm -f vc80.pdb
