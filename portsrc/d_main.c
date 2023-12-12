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
#include "w_wad.h"
#include "p_local.h"
#include "s_sound.h"
#include "m_menu.h"

#include <SDL_main.h>

char* wadnames[] = { "DOOM.WAD", "CONFIG.LMP", NULL };

extern void P_DrawPlayerShapes(int pnum);
void TimeSpin(void)
{
	int start, i, tics;

	start = IO_GetTime();
	do
	{
		IO_DoEvents(); //[ISB] I should use a timer thread....
		i = IO_GetTime();
	} while (i == start);
	start = start + 1;

	for (i = 0; i < 8192; i += 64)
	{
		R_RenderView(viewsector, viewx, viewy, viewz, i);
		P_DrawPlayerShapes(viewplayer);
		IO_UpdateScreen();
		IO_DoEvents();
	}
	i = IO_GetTime();
	tics = i - start;
	IO_Error("Time:%i  (%f ips)", tics, (double)(8960.0f / (float)tics));
}

int main(int argc, char** argv)
{
	my_argc = argc;
	my_argv = argv;

	printf("Loading WADFile...\n");
	W_InitMultipleFiles(wadnames);

	printf("Initializing...\n");
	S_Startup();
	D_LoadConfig();
	IO_Startup();
	R_Startup();
	V_Startup();
	M_Startup();
	P_Startup();

	D_DemoLoop(); //never returns
	IO_Error("Fell out of main?\n");
}
