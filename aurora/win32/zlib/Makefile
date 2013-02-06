#!/bin/make -f
# Quick and dirty Makefile for zlib.
# Generates zlib.lib in ../lib

CFLAGS=-O2 -w
LIBFLAGS=

SRC += adler32.c
SRC += compress.c
SRC += crc32.c
SRC += deflate.c
SRC += gzio.c
SRC += infback.c
SRC += inffast.c
SRC += inflate.c
SRC += inftrees.c
SRC += trees.c
SRC += uncompr.c
SRC += zutil.c

../lib/zlib.lib: $(addsuffix .obj,$(basename $(SRC)))
	lib.exe $(LIBFLAGS) -out:$@ $^

%.obj: %.c *.h Makefile
	cl.exe -c $(CFLAGS) -Fo$@ $<
