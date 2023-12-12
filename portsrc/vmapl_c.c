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
byte* colormap;
unsigned short temp;

unsigned int frac;
unsigned int step;

#define DRAWPIXEL *dest = colormap[sp_source[(frac >> 25)]];\
						dest += 0xA0;\
						frac += step;\


void R_RawScaleLow()
{
	int count;
	int i;

	colormap = &colormaps[sp_colormap * 256];
	dest = planewidthlookup[sp_y1 * 2 + windowy] + highcollumntable[sp_x * 2 + windowx];
	count = (sp_y2 - sp_y1);

	frac = sp_frac << 9;
	step = sp_fracstep << 9;

	//TBH, the function table is a horrible idea. It's super inefficient and cache unfriendly. rip.
	//This can be unrolled for performance reasons, but eh
	for (i = 0; i <= count; i++)
	{
		DRAWPIXEL
	}
	//((void(*)())FunctionTable[count])();
}
