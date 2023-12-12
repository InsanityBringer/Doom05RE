//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2020 by SaladBadger.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "r_data.h"

byte* dest;
unsigned short* colormap;
unsigned short temp;

unsigned int frac;
unsigned int step;
int offset;

#define DRAWPIXEL *dest = colormap[sp_source[(frac >> 25)]] & 0xFF;\
			      dest[16000] = (colormap[sp_source[(frac >> 25)]] >> 8) & 0xFF;\
						dest += 0x50;\
						frac += step;\

void R_RawScaleHighColor()
{
	int count, i;

	colormap = &wordcolormaps[sp_colormap * 256];
	dest = planewidthlookup[sp_y1 + windowy] + highcollumntable[sp_x * 2 + windowx];
	count = (sp_y2 - sp_y1);

	frac = sp_frac << 9;
	step = sp_fracstep << 9;

	for (i = 0; i <= count; i++)
	{
		DRAWPIXEL
	}
}
