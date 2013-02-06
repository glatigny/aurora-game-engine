# Command-line tools
CC ?= gcc
CXX ?= g++
LD = $(CXX)
AR ?= ar
CPP = gcc -E
DEP = g++ -MM
LDOUTFLAG = -o
AROUTFLAG = 

ARFLAGS = rc

INCLUDE_FLAGS = $(addprefix -I ,$(INCLUDE_DIRS)) $(addprefix -include ,$(GLOBAL_HEADERS))
DEP_INCLUDE_FLAGS = $(addprefix -I ,$(INCLUDE_DIRS)) $(addprefix -include ,$(GLOBAL_HEADERS))

ifdef DEBUG_SYMBOLS
DEBUG_CFLAGS += -g3
PROF_CFLAGS += -g3
FINAL_CFLAGS += -g3
endif

# Extensions
BINEXT = 
OBJEXT = .o
LIBEXT = .a
SHLIBEXT = .so

# Rules
%.s:: %.i
	@echo " [CXX] $(notdir $<)"
	@$(CXX) -S $(CFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%$(OBJEXT):: %.s
	@echo " [CXX] $(notdir $<)"
	@$(CXX) -c $(CFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%$(OBJEXT):: %.i
	@echo " [CXX] $(notdir $<)"
	@$(CXX) -c $(CFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%$(OBJEXT):: %.c
	@echo " [CC] $(notdir $<)"
	@$(CC) -c $(CFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%$(OBJEXT):: %.cpp
	@echo " [CXX] $(notdir $<)"
	@$(CXX) -c $(CXXFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%.i:: %.c
	@echo " [CPP] $(notdir $<)"
	@$(CC) -E $(CFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%.i:: %.cpp
	@echo " [CPP] $(notdir $<)"
	@$(CXX) -E $(CXXFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%.s:: %.c
	@echo " [CC][=>AS] $(notdir $<)"
	@$(CC) -S $(CFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"

%.s:: %.cpp
	@echo " [CXX][=>AS] $(notdir $<)"
	@$(CXX) -S $(CXXFLAGS) $(INCLUDE_FLAGS) "$<" -o "$@"


# POSIX CFLAGS
OPENGL_CFLAGS += $(shell pkg-config --exists gl glu && pkg-config --cflags gl glu)
SDL_CFLAGS += $(shell pkg-config --exists sdl && pkg-config --cflags sdl)

# POSIX LDFLAGS
OPENGL_LDFLAGS += $(shell ( pkg-config --exists gl glu && pkg-config --libs gl glu ) || echo "-lGL -lGLU")
SDL_LDFLAGS += $(shell ( pkg-config --exists sdl && pkg-config --libs sdl ) || echo "-lSDL")
VORBIS_LDFLAGS += $(shell ( pkg-config --exists vorbisfile && pkg-config --libs vorbisfile ) || echo "-lvorbisfile -lvorbis -lm -logg" )
OPENAL_LDFLAGS += $(shell ( pkg-config --exists openal && pkg-config --libs openal ) || echo "-lopenal" )
THREADS_LDFLAGS += -lpthread

