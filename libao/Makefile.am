lib_LTLIBRARIES = libao.la
libao_la_SOURCES = audio_out.c audio_out_oss.c audio_out_solaris.c \
		   audio_out_null.c audio_out_float.c
libao_la_LIBADD = @LIBAO_LIBS@
libao_la_LDFLAGS = -no-undefined

EXTRA_DIST = configure.incl
