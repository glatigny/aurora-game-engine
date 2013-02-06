ifndef PLATFORM
ifdef WINEBUILD
WIN32 := 1
endif # WINEBUILD

ifdef WIN32
PLATFORM := win32
else
PLATFORM := linux
endif # WIN32

endif # PLATFORM

ifndef ARCH
ifdef WIN32
ARCH := i686
else # WIN32
ARCH := $(shell uname -m)
endif # WIN32
endif # ARCH

include $(MK_PATH)/pspec/$(PLATFORM)-$(ARCH).mk

