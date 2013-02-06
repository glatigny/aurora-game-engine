# This file is a template for config.mk.
# It will be used whenever config.mk is missing during library compilation.
# You can also use it as a base for aoe.mk : just uncomment the last part.

# Constants
AURORA_WINDOW_TITLE = Standalone aurora library
AURORA_PROJECT_NAME = Standalone aurora library
AURORA_ENGINE_VENDOR = Aurora Studio
AURORA_ENGINE_VERSION = 0
AURORA_ENGINE_REVISION = 8
AURORA_ENGINE_URL = http://engine.aurora-studio.fr
AURORA_PROJECT_URL = http://engine.aurora-studio.fr/standalone
AURORA_ENGINE_PATH = .
# Conventions for URL :
#  url/data.aop/main.aos : startup file if the local one is not found
#  url/project/update.aos : update script for the given project
#
#  The URL alone will show a nice HTML page telling about the engine.

# Features
#AURORA_DEFINES += NADVICE            # Disable runtime advice.
#AURORA_DEFINES += NWARNING           # Disable runtime warnings.
#AURORA_DEFINES += NERROR             # Disable runtime error checks.
#AURORA_DEFINES += NDEBUG             # Disable runtime assertions.
#AURORA_DEFINES += SYSLOG             # Enable logging framework. You must enable this to enable other log options.
#AURORA_DEFINES += PERFLOG            # Enable performance logging framework.
#AURORA_DEFINES += SCRIPTLOG          # Enable AOS execution flow reflection.
#AURORA_DEFINES += FLOAT_COORD        # Use floating point for coordinates.
#AURORA_DEFINES += DOUBLE_COORD       # Use double precision floating point for coordinates (implies FLOAT_COORD).
#AURORA_DEFINES += FLOAT_DURATION     # Use floating point for durations.
#AURORA_DEFINES += DOUBLE_DURATION    # Use double precision floating point for durations (implies DOUBLE_DURATION).
#AURORA_DEFINES += OPENGL_ENABLE_FBO  # Use FBO for offscreen drawing operations.
#AURORA_DEFINES += GPU_IMAGEPROCESS   # Enable hardware-accelerated image processing of renderables (like shaders).
#AURORA_DEFINES += DEBUG_KEYS         # Enable debug keys like StateStep or StatePlay.
#AURORA_DEFINES += MOUSE_GRAB         # Allow the mouse pointer to be grabbed.
#AURORA_DEFINES += CUSTOM_KEY_EVENTS  # Enables custom key events.
#AURORA_DEFINES += ALLOW_REGULAR_NEW  # Allows the "new" keyword to be used instead of AOENEW.
#AURORA_DEFINES += KEEP_AOD_SOURCES   # Keep access to the XML form of the objects (needed for editors).
#AURORA_DEFINES += TRASH_DEPRECATED   # Disallow deprecated code to run by removing forbidden words using the preprocessor. Nasty way to do it, but it should mostly work.
#AURORA_DEFINES += MEMTRACKING        # Track memory allocations (required to enforce memory limits).
#AURORA_DEFINES += TIME_BASED         # Time-based execution, with elapsedTime in milliseconds.
#AURORA_DEFINES += POINTER_TRACKING   # Track pointers in script engine to allow reloading.
#AURORA_DEFINES += DISPLAY_COLLISIONS # Display collision rects when doing collision tests.

# Limits
#AOE_MAX_MEMORY = 128                 # Maximum memory allocation in MB (excluding 3rd party libraries)
#AOE_MAX_VIDEO_MEMORY = 256           # Maximum video memory allocation in MB

# Modules
#AURORA_MODULES += DIRECTX            # Enable DirectX support
#AURORA_MODULES += LOG_WINDOW         # Enable the syslog window on Windows.
#AURORA_MODULES += OPENGL             # OpenGL support
#AURORA_MODULES += SDL                # SDL/GL support
#AURORA_MODULES += MOUSE              # Mouse support in both DirectX and OpenGL
#AURORA_MODULES += JOYSTICK           # Joystick support in both DirectInput and SDL
#AURORA_MODULES += NET                # Network support
#AURORA_MODULES += TCP                # TCP over UNIX sockets support
#AURORA_MODULES += OPENAL             # OpenAL engine
#AURORA_MODULES += SCRIPT_DEBUGGER    # Step by step debugger for the script engine
#AURORA_MODULES += INTROSPECTION      # Ability to query class members at runtime

# VFS modules
#  VFS file access modules
#AURORA_MODULES += POSIX              # Posix file support
#AURORA_MODULES += PACKAGE            # Package file support
#AURORA_MODULES += CURL               # libcurl for HTTP, HTTPS and FTP support in VFS
#AURORA_MODULES += PCM                # PCM audio
#AURORA_MODULES += VIDCAP             # Webcam / video capture support
#AURORA_MODULES += MIC                # Microphone / audio input

#  VFS decoders
#AURORA_MODULES += PNG                # PNG loading support
#AURORA_MODULES += XML                # XML loading support
#AURORA_MODULES += VORBIS             # Vorbis sound format support

#  VFS encoders
#AURORA_MODULES += PNG_SAVE           # PNG saving support

#  Threads
#AURORA_MODULES += THREADS            # Threads support
#AURORA_DEFINES += BG_PRELOAD         # Background preload resources in resource manager.

# Compilation options
#PROFILING = 1                        # Generate an optimized binary but keep debugging information. This setting does not disable assertions.
#OPTIMIZE = 1                         # Generate an optimized binary. This setting does not disable assertions.
#DEBUG_SYMBOLS = 1                    # Generate debug symbols. This setting does not disable assertions nor enables 'debug' mode. It just generates symbol tables/pdb files.

##################################
# Settings for gamespec aoe.mk : #
# 
# # Uncomment this part to generate a valid aoe.mk.
# 
# # Name of the main executable
# PROJECT = aurora_engine
# 
# # Path to the aurora library from the top-level makefile.
# AURORA_ENGINE_PATH = aurora
# 
# # Gamespec path from aurora directory
# GAMESPEC_PATH = ..
# 
# # Extension paths relative to top-level makefile
# #FACTORY_EXTENSION = src/factory_extension.h
# #XH_EXTENSION = src/xh_extension.xh
# 
# include $(AURORA_ENGINE_PATH)/mk/gamespec.mk
# 
##################################
