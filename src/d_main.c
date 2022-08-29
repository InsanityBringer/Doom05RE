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
#include "p_play.h"
#include "s_sound.h"
#include "m_menu.h"

//TODO: this needs to be removed for the likely to happen port back to dos
#include <SDL_main.h>

char* wadnames[] = { "DOOM.WAD", "CONFIG.LMP", NULL };

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
