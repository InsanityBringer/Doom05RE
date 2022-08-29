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
#include "r_local.h"

void R_MapRowHighColor(void)
{
	uint16_t* iVar1;
	uint8_t* local_14;
	int local_10;
	int local_8;

	int xp, yp;

	int pos, step;

	/*if ((((mr_x1 < 0) || (viewwidth + 1 < mr_x2)) || (mr_x2 <= mr_x1)) || (viewheight <= mr_y))
	{
		IO_Error("MapRow: mr_x1 = %i  mr_x2 = %i mr_y = %i", mr_x1, mr_x2, mr_y);
		return;
	}
	if (mr_picture == NULL)
	{
		IO_Error("MapRow: NULL picture");
		return;
	}*/

	pos = ((mr_xfrac) & 0xFFFF0000) | ((mr_yfrac >> 0x10) & 0xFFFF);
	step = ((mr_xstep) & 0xFFFF0000) | ((mr_ystep >> 0x10) & 0xFFFF);

	local_8 = mr_x1;
	iVar1 = &wordcolormaps[mr_colormap * 256];
	local_14 = screenbuffer + (int)(planewidthlookup[mr_y + windowy] + (mr_x1 * 2 + windowx >> 2));
	local_10 = ((mr_x1 * 2 + windowx) & 3U) * 16000;

	do
	{
		yp = (pos >> 4) & 0xFC0U;
		xp = (pos >> 26) & 63;

		local_14[local_10] = iVar1[mr_picture[(xp + yp)]] & 255;
		local_14[local_10 + 16000] = (iVar1[mr_picture[(xp + yp)]] >> 8) & 255;

		pos += step;

		if (local_10 == 32000)
		{
			local_14++;
			local_10 = 0;
		}
		else
		{
			local_10 = 32000;
		}
		local_8++;
	} while (local_8 != mr_x2);
	return;
}