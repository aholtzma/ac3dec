# # Process this file with automake to produce Makefile.in.
AUTOMAKE_OPTIONS = 1.3 foreign

CFLAGS = -Wall -Werror -O3 -g

bin_PROGRAMS = ac3dec 

ac3dec_LDADD= -L./libac3 -lac3 -lm 

noinst_HEADERS = output.h
ac3dec_SOURCES = ac3dec.c output_linux.c
ac3dec_DEPENDENCIES = libac3/libac3.a

EXTRA_DIST = README Changelog TODO

SUBDIRS = libac3 tools .

