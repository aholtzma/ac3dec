CFLAGS = @CFLAGS@ @A52DEC_CFLAGS@

bin_PROGRAMS = a52dec extract_a52
a52dec_SOURCES = a52dec.c getopt.c
a52dec_LDADD = $(top_builddir)/liba52/liba52.la $(top_builddir)/libao/libao.la
extract_a52_SOURCES = extract_a52.c getopt.c

EXTRA_DIST = configure.incl getopt.h
