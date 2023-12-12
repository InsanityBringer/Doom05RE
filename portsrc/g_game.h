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

#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

typedef enum
{
	ga_runmap,
	ga_controlpanel,
	ga_completed,
	ga_died,
	ga_playing,
	ga_victory
} gameaction_t;

typedef enum
{
	gs_controlmap,
	gs_newgame,
	gs_demo,
	gs_netgame,
	gs_loadgame,
	gs_warp
} gamestart_t;

typedef struct
{
	int mappoint;
	int mapcompleted[16];
} gamestate_t;

extern gamestart_t gamestart;
extern gameaction_t gameaction;
extern gamestate_t gamestate;
extern int controlmenumap;
extern byte* demobuffer;

void G_PlayDemo(char* demoname);
void G_RecordDemo(char* map, char* demo);
void G_StartNewGame(int episode, int skill, int player);
void G_GameLoop();

void G_WarpToMap(char* mapname);

void G_WorldMap();

#endif
