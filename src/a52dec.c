/*
 * a52dec.c
 * Copyright (C) 1999-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of a52dec, a free ATSC A-52 stream decoder.
 *
 * a52dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * a52dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <getopt.h>

#include "a52.h"
#include "audio_out.h"
#include "mm_accel.h"

#define BUFFER_SIZE 262144
static uint8_t buffer[BUFFER_SIZE];
static FILE * in_file;
static uint32_t frame_counter = 0;
static struct timeval tv_beg, tv_end, tv_start;
static int elapsed;
static int total_elapsed;
static int last_count = 0;
static int demux_track = 0;
static int disable_accel = 0;
static int disable_dynrng = 0;
static ao_open_t * output_open = NULL;
static ao_instance_t * output;
static sample_t * samples;

static void print_fps (int final) 
{
    int fps, tfps, frames;
	
    gettimeofday (&tv_end, NULL);

    if (frame_counter++ == 0) {
	tv_start = tv_beg = tv_end;
    }

    elapsed = (tv_end.tv_sec - tv_beg.tv_sec) * 100 +
	(tv_end.tv_usec - tv_beg.tv_usec) / 10000;
    total_elapsed = (tv_end.tv_sec - tv_start.tv_sec) * 100 +
	(tv_end.tv_usec - tv_start.tv_usec) / 10000;

    if (final) {
	if (total_elapsed) 
	    tfps = frame_counter * 10000 / total_elapsed;
	else
	    tfps = 0;

	fprintf (stderr,"\n%d frames decoded in %d.%02d "
		 "seconds (%d.%02d fps)\n", frame_counter,
		 total_elapsed / 100, total_elapsed % 100,
		 tfps / 100, tfps % 100);

	return;
    }

    if (elapsed < 50)	/* only display every 0.50 seconds */
	return;

    tv_beg = tv_end;
    frames = frame_counter - last_count;

    fps = frames * 10000 / elapsed;			/* 100x */
    tfps = frame_counter * 10000 / total_elapsed;	/* 100x */

    fprintf (stderr, "%d frames in %d.%02d sec (%d.%02d fps), "
	     "%d last %d.%02d sec (%d.%02d fps)\033[K\r", frame_counter,
	     total_elapsed / 100, total_elapsed % 100,
	     tfps / 100, tfps % 100, frames, elapsed / 100, elapsed % 100,
	     fps / 100, fps % 100);

    last_count = frame_counter;
}

static RETSIGTYPE signal_handler (int sig)
{
    print_fps (1);
    signal (sig, SIG_DFL);
    raise (sig);
}

static void print_usage (char * argv[])
{
    int i;
    ao_driver_t * drivers;

    fprintf (stderr, "usage: %s [-o <mode>] [-s[<track>]] [-c] [-r] <file>\n"
	     "\t-s\tuse program stream demultiplexer, track 0-7 or 0x80-0x87\n"
	     "\t-c\tuse c implementation, disables all accelerations\n"
	     "\t-r\tdisable dynamic range compression\n"
	     "\t-o\taudio output mode\n", argv[0]);

    drivers = ao_drivers ();
    for (i = 0; drivers[i].name; i++)
	fprintf (stderr, "\t\t\t%s\n", drivers[i].name);

    exit (1);
}

static void handle_args (int argc, char * argv[])
{
    int c;
    ao_driver_t * drivers;
    int i;

    drivers = ao_drivers ();
    while ((c = getopt (argc, argv, "s::cro:")) != -1)
	switch (c) {
	case 'o':
	    for (i = 0; drivers[i].name != NULL; i++)
		if (strcmp (drivers[i].name, optarg) == 0)
		    output_open = drivers[i].open;
	    if (output_open == NULL) {
		fprintf (stderr, "Invalid video driver: %s\n", optarg);
		print_usage (argv);
	    }
	    break;

	case 's':
	    demux_track = 0x80;
	    if (optarg != NULL) {
		char * s;

		demux_track = strtol (optarg, &s, 16);
		if (demux_track < 0x80)
		    demux_track += 0x80;
		if ((demux_track < 0x80) || (demux_track > 0x87) || (*s)) {
		    fprintf (stderr, "Invalid track number: %s\n", optarg);
		    print_usage (argv);
		}
	    }
	    break;

	case 'c':
	    disable_accel = 1;
	    break;

	case 'r':
	    disable_dynrng = 1;
	    break;

	default:
	    print_usage (argv);
	}

    /* -o not specified, use a default driver */
    if (output_open == NULL)
	output_open = drivers[0].open;

    if (optind < argc) {
	in_file = fopen (argv[optind], "rb");
	if (!in_file) {
	    fprintf (stderr, "%s - couldnt open file %s\n", strerror (errno),
		     argv[optind]);
	    exit (1);
	}
    } else
	in_file = stdin;
}

void a52_decode_data (uint8_t * start, uint8_t * end)
{
    static a52_state_t state;

    static uint8_t buf[3840];
    static uint8_t * bufptr = buf;
    static uint8_t * bufpos = buf + 7;
    int sample_rate;
    int bit_rate;
    int flags;

    while (start < end) {
	*bufptr++ = *start++;
	if (bufptr == bufpos) {
	    if (bufpos == buf + 7) {
		int length;

		length = a52_syncinfo (buf, &flags, &sample_rate, &bit_rate);
		if (!length) {
		    fprintf (stderr, "skip\n");
		    for (bufptr = buf; bufptr < buf + 6; bufptr++)
			bufptr[0] = bufptr[1];
		    continue;
		}
		bufpos = buf + length;
	    } else {
		sample_t level, bias;
		int i;

		if (ao_setup (output, sample_rate, &flags, &level, &bias))
		    goto error;
		flags |= A52_ADJUST_LEVEL;
		if (a52_frame (&state, buf, &flags, &level, bias))
		    goto error;
		if (disable_dynrng)
		    a52_dynrng (&state, NULL, NULL);
		for (i = 0; i < 6; i++) {
		    if (a52_block (&state, samples))
			goto error;
		    if (ao_play (output, flags, samples))
			goto error;
		}
		bufptr = buf;
		bufpos = buf + 7;
		print_fps (0);
		continue;
	    error:
		fprintf (stderr, "error\n");
		bufptr = buf;
		bufpos = buf + 7;
	    }
	}
    }
}

static void ps_loop (void)
{
    static int mpeg1_skip_table[16] = {
	     1, 0xffff,      5,     10, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
    };

    uint8_t * buf;
    uint8_t * end;
    uint8_t * tmp1;
    uint8_t * tmp2;
    int complain_loudly;

    complain_loudly = 1;
    buf = buffer;

    do {
	end = buf + fread (buf, 1, buffer + BUFFER_SIZE - buf, in_file);
	buf = buffer;

	while (buf + 4 <= end) {
	    /* check start code */
	    if (buf[0] || buf[1] || (buf[2] != 0x01)) {
		if (complain_loudly) {
		    fprintf (stderr, "missing start code at %#lx\n",
			     ftell (in_file) - (end - buf));
		    if ((buf[0] == 0) && (buf[1] == 0) && (buf[2] == 0))
			fprintf (stderr, "this stream appears to use "
				 "zero-byte padding before start codes,\n"
				 "which is not correct according to the "
				 "mpeg system standard.\n"
				 "mp1e was one encoder known to do this "
				 "before version 1.8.0.\n");
		    complain_loudly = 0;
		}
		buf++;
		continue;
	    }

	    switch (buf[3]) {
	    case 0xb9:	/* program end code */
		return;
	    case 0xba:	/* pack header */
		/* skip */
		if ((buf[4] & 0xc0) == 0x40)	/* mpeg2 */
		    tmp1 = buf + 14 + (buf[13] & 7);
		else if ((buf[4] & 0xf0) == 0x20)	/* mpeg1 */
		    tmp1 = buf + 12;
		else if (buf + 5 > end)
		    goto copy;
		else {
		    fprintf (stderr, "weird pack header\n");
		    exit (1);
		}
		if (tmp1 > end)
		    goto copy;
		buf = tmp1;
		break;
	    case 0xbd:	/* private stream 1 */
		tmp2 = buf + 6 + (buf[4] << 8) + buf[5];
		if (tmp2 > end)
		    goto copy;
		if ((buf[6] & 0xc0) == 0x80)	/* mpeg2 */
		    tmp1 = buf + 9 + buf[8];
		else {	/* mpeg1 */
		    for (tmp1 = buf + 6; *tmp1 == 0xff; tmp1++)
			if (tmp1 == buf + 6 + 16) {
			    fprintf (stderr, "too much stuffing\n");
			    buf = tmp2;
			    break;
			}
		    if ((*tmp1 & 0xc0) == 0x40)
			tmp1 += 2;
		    tmp1 += mpeg1_skip_table [*tmp1 >> 4];
		}
		if (*tmp1 == demux_track) {	/* a52 */
		    tmp1 += 4;
		    if (tmp1 < tmp2)
			a52_decode_data (tmp1, tmp2);
		}
		buf = tmp2;
		break;
	    default:
		if (buf[3] < 0xb9) {
		    fprintf (stderr,
			     "looks like a video stream, not system stream\n");
		    exit (1);
		}
		/* skip */
		tmp1 = buf + 6 + (buf[4] << 8) + buf[5];
		if (tmp1 > end)
		    goto copy;
		buf = tmp1;
		break;
	    }
	}

	if (buf < end) {
	copy:
	    /* we only pass here for mpeg1 ps streams */
	    memmove (buffer, buf, end - buf);
	}
	buf = buffer + (end - buf);

    } while (end == buffer + BUFFER_SIZE);
}

static void es_loop (void)
{
    int size;
		
    do {
	size = fread (buffer, 1, BUFFER_SIZE, in_file);
	a52_decode_data (buffer, buffer + size);
    } while (size == BUFFER_SIZE);
}

int main (int argc,char *argv[])
{
    uint32_t accel;

    fprintf (stderr, PACKAGE"-"VERSION
	     " (C) 2000-2001 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>\n");

    handle_args (argc, argv);

    accel = disable_accel ? 0 : MM_ACCEL_MLIB;

    output = ao_open (output_open);
    if (output == NULL) {
	fprintf (stderr, "Can not open output\n");
	return 1;
    }

    samples = a52_init (accel);
    if (samples == NULL) {
	fprintf (stderr, "A52 init failed\n");
	return 1;
    }

    signal (SIGINT, signal_handler);

    gettimeofday (&tv_beg, NULL);

    if (demux_track)
	ps_loop ();
    else
	es_loop ();

    ao_close (output);
    print_fps (1);
    return 0;
}
