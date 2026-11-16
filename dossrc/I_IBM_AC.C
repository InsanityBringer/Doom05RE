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

#include <stdarg.h>
#include <stdio.h>
#include <i86.h>
#include <conio.h>
#include <dos.h>
#include <graph.h> //no idea why

#include "doomdef.h"
#include "w_wad.h"
#include "i_local.h"
#include "p_local.h"
#include "r_data.h"

void IO_BlitBlocks(void)
{
	int iVar1;
	int iVar2;
	int* puVar3;
	int i;

	outp(0x3c4, 2);
	iVar2 = 199;
	do
	{
		while (update[iVar2] == 0)
		{
			iVar2 = iVar2 - 1;
			if (iVar2 < 0)
			{
				return;
			}
		}
		update[iVar2]--;
		iVar1 = ublocksource[iVar2];
		puVar3 = (int*)(buffercrtc + iVar1);
		//TODO: unroll
		//TODO: investigate if transfer horizontally (despite the penalty to setting the mapmask) will make this faster.
		outp(0x3c5, 1);
		for (i = 0; i < 1600; i += 80)
		{
			puVar3[i / 4] = *(int*)(&screenbuffer[iVar1] + (i));
		}

		outp(0x3c5, 2);
		for (i = 0; i < 1600; i += 80)
		{
			puVar3[i / 4] = *(int*)(&screenbuffer[iVar1] + 16000 + (i));
		}

		outp(0x3c5, 4);
		for (i = 0; i < 1600; i += 80)
		{
			puVar3[i / 4] = *(int*)(&screenbuffer[iVar1] + 32000 + (i));
		}

		outp(0x3c5, 8);
		for (i = 0; i < 1600; i += 80)
		{
			puVar3[i / 4] = *(int*)(&screenbuffer[iVar1] + 48000 + (i));
		}
		iVar2--;
	} while (-1 < iVar2);
}
