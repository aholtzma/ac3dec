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


static unsigned char *mpeg_data = 0;
static unsigned char *mpeg_data_end = 0;
static unsigned char *cur_pos;

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

	cur_pos = mpeg_data;
	mpeg_data_end = mpeg_data + mpeg_data_size;

	return f;
}

inline void increment_position(long x)
{
	cur_pos += x;

	if(cur_pos >= mpeg_data_end)
	{
		fprintf(stderr, "Error: unexpected end of stream\n");
		exit(1);
	}

}

inline int next_24_bits(long x)
{
	if (cur_pos[0] != ((x >> 16) & 0xff))
		return 0;
	if (cur_pos[1] != ((x >>  8) & 0xff))
		return 0;
	if (cur_pos[2] != ((x      ) & 0xff))
		return 0;

	return 1;
}

inline int next_32_bits(long x)
{
	if (cur_pos[0] != ((x >> 24) & 0xff))
		return 0;
	if (cur_pos[1] != ((x >> 16) & 0xff))
		return 0;
	if (cur_pos[2] != ((x >>  8) & 0xff))
		return 0;
	if (cur_pos[3] != ((x      ) & 0xff))
		return 0;

	return 1;
}

void parse_pes(void)
{
	unsigned long data_length;
	unsigned long header_length;


	//The header length is the PES_header_data_length byte plus 6 for the packet
	//start code and packet size, 3 for the PES_header_data_length and two
	//misc bytes, and finally 4 bytes for the mystery AC3 packet tag 
	header_length = cur_pos[8] + 6 + 3 + 4 ;
	data_length =(cur_pos[4]<<8) + cur_pos[5];


	//If we have AC-3 audio then output it
	if(cur_pos[3] == 0xbd)
	{
		//Debuggin printfs
		//fprintf(stderr,"start of pes curpos[] = %02x%02x%02x%02x\n",
		//	cur_pos[0],cur_pos[1],cur_pos[2],cur_pos[3]);
		//fprintf(stderr,"header_length = %d data_length = %x\n",
		//	header_length, data_length);
		//fprintf(stderr,"extra crap 0x%02x%02x%02x%02x data size 0x%0lx\n",cur_pos[header_length-4],
		//	cur_pos[header_length-3],cur_pos[header_length-2],cur_pos[header_length-1],data_length);

		//Make sure it isn't a strange 0x2xxx xxxx packet
		if((cur_pos[header_length-4] & 0x80 ))
			fwrite(&cur_pos[header_length],1,data_length - (header_length - 6),stdout);
	}

	//The packet size is data_length plus 6 bytes to account for the
	//packet start code and the data_length itself.
	increment_position(data_length + 6);
}

void parse_pack(void)
{
	unsigned long skip_length;

	/* Deal with the pack header */
	/* The first 13 bytes are junk. The fourteenth byte 
	 * contains the number of stuff bytes */
	skip_length = cur_pos[13] & 0x7;
	increment_position(14 + skip_length);

	/* Deal with the system header if it exists */
	if(next_32_bits(0x000001bb))
	{
		/* Bytes 5 and 6 contain the length of the header minus 6 */
		skip_length = (cur_pos[4] << 8) +  cur_pos[5];
		increment_position(6 + skip_length);
	}

	while(next_24_bits(0x000001) && !next_32_bits(0x000001ba))
	{
		parse_pes();
	}
}

int main(int argc, char *argv[])
{
	int f;

	if (argc < 2) {
		fprintf(stderr, "usage: %s mpeg_stream\n", argv[0]);
		exit(1);
	}

	f = map_file(argv[1]);

	if(!next_32_bits(0x000001ba))
	{
		fprintf(stderr, "Non-program streams not handled - exiting\n\n");
		exit(1);
	}

	do
	{
		parse_pack();
	} 
	while(next_32_bits(0x000001ba));

	fprintf(stderr,"curpos[] = %x%x%x%x\n",cur_pos[0],cur_pos[1],cur_pos[2],cur_pos[3]);

	if(!next_32_bits(0x000001b9))
	{
		fprintf(stderr, "Error: expected end of stream code\n");
		exit(1);
	}

	close(f);
	return 0;
}		
