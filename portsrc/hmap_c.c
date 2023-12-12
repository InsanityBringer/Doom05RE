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

void R_MapRowHigh(void)
{
	byte* iVar1;
	byte* local_14;
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
	iVar1 = &colormaps[mr_colormap * 256];
	local_14 = screenbuffer + planewidthlookup[mr_y + windowy] + (mr_x1 + windowx >> 2);
	local_10 = ((mr_x1 + windowx) & 3U) * 16000;

	//feed the pipeline early. This is needed to replicate a bug where the first pixel per span is duplicated
	yp = (pos >> 4) & 0xFC0U;
	xp = (pos >> 26) & 63;

	do
	{
		local_14[local_10] = iVar1[mr_picture[(xp + yp)]];

		yp = (pos >> 4) & 0xFC0U;
		xp = (pos >> 26) & 63;
		pos += step;

		if (local_10 == 48000)
		{
			local_14++;
			local_10 = 0;
		}
		else
		{
			local_10 += 16000;
		}
		local_8++;
	} while (local_8 != mr_x2);
	return;
}

void R_MapRowMed(void)
{
	byte* iVar1;
	byte* local_14;
	int local_10;
	int local_8;

	int xp, yp;

	int pos, step;

	pos = ((mr_xfrac) & 0xFFFF0000) | ((mr_yfrac >> 0x10) & 0xFFFF);
	step = ((mr_xstep) & 0xFFFF0000) | ((mr_ystep >> 0x10) & 0xFFFF);

	local_8 = mr_x1;
	iVar1 = &colormaps[mr_colormap * 256];
	local_14 = screenbuffer + (int)(planewidthlookup[mr_y + windowy] + (mr_x1 * 2 + windowx >> 2));
	local_10 = ((mr_x1 * 2 + windowx) & 3U) * 16000;

	yp = (pos >> 4) & 0xFC0U;
	xp = (pos >> 26) & 63;

	do
	{
		local_14[local_10] = iVar1[mr_picture[(xp + yp)]];

		yp = (pos >> 4) & 0xFC0U;
		xp = (pos >> 26) & 63;
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

void R_MapRowLow(void)
{
	byte* iVar1;
	byte* local_14;
	int local_10;
	int local_8;

	int xp, yp;

	int pos, step;

	pos = ((mr_xfrac) & 0xFFFF0000) | ((mr_yfrac >> 0x10) & 0xFFFF);
	step = ((mr_xstep) & 0xFFFF0000) | ((mr_ystep >> 0x10) & 0xFFFF);

	local_8 = mr_x1;
	iVar1 = &colormaps[mr_colormap * 256];
	local_14 = screenbuffer + (int)(planewidthlookup[mr_y * 2 + windowy] + (mr_x1 * 2 + windowx >> 2));
	local_10 = ((mr_x1 * 2 + windowx) & 3U) * 16000;
	yp = (pos >> 4) & 0xFC0U;
	xp = (pos >> 26) & 63;

	do
	{

		local_14[local_10] = iVar1[mr_picture[(xp + yp)]];

		yp = (pos >> 4) & 0xFC0U;
		xp = (pos >> 26) & 63;
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

extern byte* dm_dest;
extern int dm_x1, dm_x2;
extern fixed_t dm_xfrac, dm_yfrac;
extern fixed_t dm_xstep, dm_ystep;
extern byte* dm_picture;

void D_PolygonRow()
{
    byte* local_14;
    int local_10;
    int local_8;

    local_8 = dm_x2;
    local_14 = dm_dest + (dm_x2 >> 2);

    local_10 = ((dm_x2) & 3U) * 16000;

    do
    {
        local_14[local_10] = dm_picture[(((dm_yfrac >> 0x10)) << 9) + ((dm_xfrac >> 0x10))];
        dm_xfrac += dm_xstep;
        dm_yfrac += dm_ystep;
        if (local_10 == 48000)
        {
            local_14++;
            local_10 = 0;
        }
        else
        {
            local_10 += 16000;
        }
        local_8++;
    } while (local_8 != dm_x1);
    return;
}

