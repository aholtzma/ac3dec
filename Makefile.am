# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O0 -g 

bin_PROGRAMS = decode

decode_LDADD= -lm

decode_SOURCES = bitstream.c decode.c imdct.c mantissa.c exponent.c stats.c\
	bit_allocate.c uncouple.c output.c parse.c crc.c debug.c ring_buffer.c\
	matlab.c
noinst_HEADERS = bitstream.h decode.h imdct.h ac3.h stats.h mantissa.h\
	exponent.h bit_allocate.h uncouple.h output.h parse.h crc.h debug.h\
	ring_buffer.h matlab.h

EXTRA_decode_SOURCES = output_linux.c output_solaris.c

EXTRA_DIST = README

SUBDIRS = . test tools

