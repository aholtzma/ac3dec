/*
 * audio_out.c
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

#include <inttypes.h>
#include <stdlib.h>

#include "a52.h"
#include "audio_out.h"

extern ao_open_t ao_oss_open;
extern ao_open_t ao_ossdolby_open;
extern ao_open_t ao_oss4_open;
extern ao_open_t ao_oss6_open;
extern ao_open_t ao_solaris_open;
extern ao_open_t ao_solarisdolby_open;
extern ao_open_t ao_null_open;
extern ao_open_t ao_null4_open;
extern ao_open_t ao_null6_open;
extern ao_open_t ao_float_open;

static ao_driver_t audio_out_drivers[] = {
#ifdef LIBAO_OSS
    {"oss", ao_oss_open},
    {"ossdolby", ao_ossdolby_open},
    {"oss4", ao_oss4_open},
    {"oss6", ao_oss6_open},
#endif
#ifdef LIBAO_SOLARIS
    {"solaris", ao_solaris_open},
    {"solarisdolby", ao_solarisdolby_open},
#endif
    {"null", ao_null_open},
    {"null4", ao_null4_open},
    {"null6", ao_null6_open},
    {"float", ao_float_open},
    {NULL, NULL}
};

ao_driver_t * ao_drivers (void)
{
    return audio_out_drivers;
}
