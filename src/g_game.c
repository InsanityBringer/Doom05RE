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

int startepisode;
gamestart_t gamestart;
gameaction_t gameaction;
int startskill;
int startplayer;
int controlmenumap;

int episode;
int skilllevel;
int gamestate;
uint8_t* demobuffer;

char demoname[20];

void G_InitPlayer(int playernum)
{
	player_t* local_1c;

	if (playernum >= MAXPLAYERS) 
	{
		IO_Error("G_InitPlayer: bad player number\n");
		return;
	}
	sd->playeringame[playernum] = 1;

	memset(&playerobjs[playernum], 0, sizeof(player_t));
	playerobjs[playernum].readyweapon = wp_rifle;
	playerobjs[playernum].pendingweapon = wp_nochange;
	playerobjs[playernum].weaponowned[wp_knife] = 1;
	playerobjs[playernum].weaponowned[wp_rifle] = 1;
	playerobjs[playernum].ammo[am_clip] = 14;
	playerobjs[playernum].lives = 3;
	playerobjs[playernum].score = 0;
	playerobjs[playernum].nextextra = 10000;
	playerobjs[playernum].health = MAXBODY;
	playerobjs[playernum].armor = 7;
	return;
}

void G_WarpToMap(char* mapname)
{
	char* local_20;
	int iVar1;

	local_20 = mapname;
	Z_ClearZone(playzone);
	P_InitThinkers();
	P_InitActors();
	M_ClearMenus();
	memset(playerthingfound, 0, sizeof(int) * MAXPLAYERS);
	R_LoadMap(local_20);
	iVar1 = 0;
	while (iVar1 < MAXPLAYERS)
	{
		if (playerthingfound[iVar1] == 0) 
		{
			IO_Error("G_WarpToMap: player %i not spawned on map %s\n", iVar1, local_20);
		}
		iVar1++;
	}
	strncpy(sd->mapname, local_20, 8);
	return;
}

void G_PlayerDied(player_t* player)
{
	//once again, it is but a dream
	return;
}

void G_PlayDemo(char* name)
{
	strcpy(&demoname[0], name);
	demoaction = 8;
	gamestart = 2;
}

void G_SetupNewGame(void);
void G_RecordDemo(char* map, char* demoname)
{
	int iVar1;
	G_StartNewGame(1, 4, 0);

	//[ISB] In order to make the game not freeze when recording demos, this is needed to set some vital state.
	//however, it's missing in the original code
	//G_SetupNewGame();

	demoaction = da_gameloop;
	gameaction = ga_runmap;
	G_WarpToMap(map);
	demobuffer = (uint8_t*)malloc(0x10000);
#ifdef ISB_LINT
	if (demobuffer == NULL)
	{
		IO_Error("G_RecordDemo: Can't allocate demo buffer\n");
		return;
	}
#endif
	demo_p = demobuffer;
	strncpy((char*)demobuffer, map, 8);
	demo_p += 8;
	demoend = demobuffer + 0x10000;
	demo = 1;
	P_PlayLoop();
	iVar1 = D_WriteFile(demoname, demobuffer, (int)(demo_p - demobuffer));
	if (iVar1 == 0)
	{
		IO_Error("Error saving demo\n");
	}
	free(demobuffer);
	return;
}


void G_StartSavedGame(int save)
{
	//it is but a dream
}

void G_StartNewGame(int episode, int skill, int player)
{
	demoaction = 8;
	startepisode = episode;
	gamestart = 1;
	startskill = skill;
	startplayer = player;
	return;
}

void G_SetupNewGame(void)
{
	char mapname[9];
	skilllevel = startskill;
	sd->consoleplayer = startplayer;
	viewplayer = startplayer;
	viewplayerangle = 0;
	memset(&sd->playeringame[0], 0, MAXPLAYERS * sizeof(uint8_t));
	G_InitPlayer(startplayer);
	sprintf(mapname, "E1M%d", startepisode);
	G_WarpToMap(mapname);
	gameaction = 0;
	return;
}

void G_SetupNetGame(void)
{
	//it is also but a dream. 
	int local_8;

	skilllevel = 4;
	viewplayer = sd->consoleplayer;
	viewplayerangle = 0;
	local_8 = 0;
	while (local_8 < 4)
	{
		if (sd->playeringame[local_8]) 
		{
			G_InitPlayer(local_8);
		}
		local_8 = local_8 + 1;
	}
	G_WarpToMap(sd->mapname);
	demoaction = 9;
	gameaction = 0;
	return;
}

void G_SetupControls(void)
{
	memset((char*)&sd->playeringame, 0, 4);
	G_InitPlayer(0);
	G_WarpToMap("MENUMAP");
	M_StartControlPanel();
	controlmenumap = 1;
	P_PlayLoop();
	controlmenumap = 0;
	return;
}

void G_SetupDemo(void)
{
	int iVar1;

	startskill = 4;
	startepisode = 1;
	startplayer = 0;
	G_SetupNewGame();
	iVar1 = W_GetNumForName(demoname);
	demobuffer = (uint8_t*)lumpinfo[iVar1].position;
	demo_p = demobuffer;
	G_WarpToMap((char*)demobuffer);
	demo_p = (void*)((char*)demo_p + 8);
	demoend = demobuffer + lumpinfo[iVar1].size;
	demo = 2;
	P_PlayLoop();
	demo = 0;
	return;
}


void G_GameLoop(void)
{
	int iVar1;

	demoaction = 9;
	switch (gamestart) 
	{
	case 0:
		G_SetupControls();
		return;
	case 1:
		G_SetupNewGame();
		break;
	case 2:
		G_SetupDemo();
		return;
	case 5:
		G_SetupNewGame();
	}
	
	while (1)
	{
		if (demoaction != da_gameloop) 
		{
			return;
		}
		else if (gameaction == ga_runmap)
		{
			P_PlayLoop();
		}
		else if (gameaction == ga_died)
		{
			G_InitPlayer(0);
			G_WorldMap();
		}
		else if (gameaction == ga_completed)
		{
			G_WorldMap();
		}
		else
		{
			IO_Error("G_GameLoop: unknown gameaction\n");
		}
		D_FadeOut();
	}
}
