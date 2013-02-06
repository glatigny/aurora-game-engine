#!/bin/make -f
# Quick and dirty Makefile for zlib.
# Generates zlib.lib in ../lib

CFLAGS=-O2 -w
LIBFLAGS=

SRC += png.c
SRC += pngerror.c
SRC += pngget.c
SRC += pngmem.c
SRC += pngread.c
SRC += pngrio.c
SRC += pngrtran.c
SRC += pngrutil.c
SRC += pngset.c
SRC += pngtrans.c
SRC += pngwio.c
SRC += pngwrite.c
SRC += pngwtran.c
SRC += pngwutil.c

../lib/libpng.lib: $(addsuffix .obj,$(basename $(SRC)))
	lib.exe $(LIBFLAGS) -out:$@ $^

%.obj: %.c *.h Makefile
	cl.exe -c $(CFLAGS) -I../zlib -Fo$@ $<
