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
#include "g_game.h"
#include "p_play.h"

int startepisode;
gamestart_t gamestart;
gameaction_t gameaction;
skill_t startskill;
int startplayer;
int controlmenumap;

int episode;
skill_t skilllevel;
gamestate_t gamestate;
uint8_t* demobuffer;

char demoname[20];

void G_InitPlayer(int playernum)
{
	player_t* player;

	if (playernum >= MAXPLAYERS) 
	{
		IO_Error("G_InitPlayer: bad player number\n");
		return;
	}
	sd->playeringame[playernum] = 1;

	memset(&playerobjs[playernum], 0, sizeof(player_t));
	playerobjs[playernum].readyweapon = wp_rifle;
	playerobjs[playernum].pendingweapon = wp_nochange;
	memset(&playerobjs[playernum].weaponowned, 0, sizeof(playerobjs[playernum].weaponowned)); //no idea why these are done when the entire thing is memset to 0 earlier..
	memset(&playerobjs[playernum].ammo, 0, sizeof(playerobjs[playernum].ammo));
	playerobjs[playernum].weaponowned[wp_knife] = 1;
	playerobjs[playernum].weaponowned[wp_rifle] = 1;
	playerobjs[playernum].ammo[am_clip] = 14;
	playerobjs[playernum].lives = 3;
	playerobjs[playernum].score = 0;
	playerobjs[playernum].nextextra = 10000;
	playerobjs[playernum].health = MAXBODY;
	playerobjs[playernum].armor = 7;
	memset(&playerobjs[playernum].items, 0, sizeof(playerobjs[playernum].items)); 
	return;
}

void G_WarpToMap(char* mapname)
{
	int i;

	Z_ClearZone(playzone);
	P_InitThinkers();
	P_InitActors();
	M_ClearMenus();
	memset(playerthingfound, 0, sizeof(int) * MAXPLAYERS);
	R_LoadMap(mapname);
	
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playerthingfound[i] == 0) 
		{
			IO_Error("G_WarpToMap: player %i not spawned on map %s\n", i, mapname);
		}
	}

	strncpy(sd->mapname, mapname, 8);
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
		IO_Error("G_RecordDemo: Can't allocate demo buffer");
		return;
	}
#endif
	demo_p = demobuffer;
	strncpy((char*)demobuffer, map, 8);
	demo_p += 8;
	demoend = demobuffer + 0x10000;
	demo = dm_recording;
	P_PlayLoop();
	if (D_WriteFile(demoname, demobuffer, (int)(demo_p - demobuffer)) == 0)
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
	demoaction = da_startgame;
	startepisode = episode;
	gamestart = gs_newgame;
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
	int i;

	skilllevel = sk_deadly;
	viewplayer = sd->consoleplayer;
	viewplayerangle = 0;
	
	for (i = 0; i < 4; i++)
	{
		if (sd->playeringame[i]) 
		{
			G_InitPlayer(i);
		}
	}

	G_WarpToMap(sd->mapname);
	demoaction = da_gameloop;
	gameaction = ga_runmap;
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
	int lump;

	startskill = sk_deadly;
	startepisode = 1;
	startplayer = 0;
	G_SetupNewGame();
	lump = W_GetNumForName(demoname);
	demobuffer = (uint8_t*)lumpinfo[lump].position;
	demo_p = demobuffer;
	G_WarpToMap((char*)demobuffer);
	demo_p = (void*)((char*)demo_p + 8);
	demoend = demobuffer + lumpinfo[lump].size;
	demo = dm_playback;
	P_PlayLoop();
	demo = dm_user;
	return;
}


void G_GameLoop(void)
{
	demoaction = da_gameloop;
	switch (gamestart) 
	{
	case gs_controlmap:
		G_SetupControls();
		return;
	case gs_newgame:
		G_SetupNewGame();
		break;
	case gs_demo:
		G_SetupDemo();
		return;
	case gs_warp:
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
			IO_Error("G_GameLoop: unknown gameaction");
		}
		D_FadeOut();
	}
}
