/*
 * extract_ac3.c
 *
 *
 * Aaron Holtzman - June 199
 *
 * Extracts an AC-3 audio stream from an MPEG-2 system stream
 * and writes it to stdout
 *
 * Ideas and bitstream syntax info borrowed from code written 
 * by Nathan Laredo (laredo@gnu.org)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define dump_current(a) \
	fprintf(stderr, "%9d: %02x %02x %02x %02x %02x %02x %02x %02x\n", \
		a - mpeg_data, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);

static unsigned char *mpeg_data = 0;
static unsigned char *mpeg_data_end = 0;

int map_file(char file_name[])
{
	struct stat st;
	int f;
	int mpeg_data_size;

	if ((f = open(file_name, O_RDONLY)) < 0) 
	{
		fprintf(stderr,"File not found\n");
		exit(1);
	}

	if (fstat(f, &st) < 0) 
	{
		fprintf(stderr,"File not found\n");
		exit(1);
	}

	mpeg_data_size = st.st_size - 4;

	if ((mpeg_data = mmap(0, mpeg_data_size, PROT_READ, MAP_SHARED, f, 0)) == MAP_FAILED) 
	{
		fprintf(stderr,"%s : mmap failed\n",strerror(errno));
		exit(1);
	}

	mpeg_data_end = mpeg_data + mpeg_data_size;

	return f;
}


void strip_ac3(void)
{
	int pes_bytes;
	int header_bytes;
	unsigned char *cur_pos;
	 
	cur_pos = mpeg_data;

	if (!(cur_pos[0] == 0    && cur_pos[1] == 0 && cur_pos[2] == 1 &&
		    cur_pos[3] == 0xba && (cur_pos[4] & 0xc0) == 0x40)) 
	{
		fprintf(stderr, "Non-program streams not handled - exiting\n\n");
		exit(1);
	}

	while (cur_pos < mpeg_data_end) 
	{
		/* Search for the next AC-3 block */
		while (!(cur_pos[0] == 0 && cur_pos[1] == 0 && cur_pos[2] == 1 && cur_pos[3] == 0xbd) &&
			cur_pos < mpeg_data_end)
			cur_pos++;

		if (cur_pos >= mpeg_data_end)
			break;

		pes_bytes = (cur_pos[4]<<8) + cur_pos[5] + 6;

		if(pes_bytes > 2200)
		{
			//FIXME if we have a really large pes size, then
			//we've probably just encounter some video that looks like
			//AC-3, so skip it.
			//
			//We really should parse all types of pes to avoid this problem
			cur_pos++;
			continue;
		}
		header_bytes = cur_pos[8] + 13;
		cur_pos += header_bytes;

		//fprintf(stderr,"Wrote 1 AC-3 frame of length %d\n",
		//		pes_bytes - header_bytes);
		fwrite(cur_pos,1,pes_bytes - header_bytes,stdout);
		
		cur_pos += pes_bytes - header_bytes;
	}
	return;
}

int main(int argc, char *argv[])
{
	int f;

	if (argc < 2) {
		fprintf(stderr, "usage: %s mpeg_stream\n", argv[0]);
		exit(1);
	}

	f = map_file(argv[1]);
	strip_ac3();
	close(f);

	return 0;
}		
