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
#include "p_local.h"

int screenfaded;
int playernum;
int processedframe;

memzone_t* playzone;

void* demo_p;

int automapup;
int redshift;
int goldshift;
shared_t* sd;

thinker_t* currentthinker;

uint8_t* playpalette;
int currentpalette;

int viewplayer;
int viewplayerangle;

player_t* player;
int playerthingfound[MAXPLAYERS];
thinker_t thinkercap;
player_t playerobjs[MAXPLAYERS];

uint8_t* demoend;
int demo;
uint8_t* demo_p;

#define STARTREDPALS 1
#define STARTGOLDPALS 8
#define NUMREDPALS 6
#define NUMGOLDPALS	2

void P_SetShiftPalette(void)
{
	int iVar1;

	if (redshift == 0)
	{
		if (goldshift == 0) 
		{
			iVar1 = 0;
		}
		else
		{
			iVar1 = goldshift >> 2;
			if (NUMGOLDPALS < iVar1)
			{
				iVar1 = NUMGOLDPALS;
			}
			iVar1 = iVar1 + STARTGOLDPALS;
		}
	}
	else 
	{
		iVar1 = redshift >> 2;
		if (NUMREDPALS < iVar1) 
		{
			iVar1 = NUMREDPALS;
		}
		iVar1 = iVar1 + STARTREDPALS;
	}
	if ((iVar1 != currentpalette) && (screenfaded == 0)) 
	{
		currentpalette = iVar1;
		IO_SetPalette(&playpalette[iVar1 * 0x300]);
	}
	return;
}

void P_SetPlayPalette(void)
{
	redshift = 0;
	goldshift = 0;
	currentpalette = 0;
	return;
}

void P_InitThinkers(void)
{
	thinkercap.prev = &thinkercap;
	thinkercap.next = &thinkercap;
	return;
}

void P_AddThinker(thinker_t* thinker)
{
	(thinkercap.prev)->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
	return;
}

void P_RemoveThinker(thinker_t* thinker)
{
	thinker->next->prev = thinker->prev;
	thinker->prev->next = thinker->next;
	Z_Free(thinker);
	return;
}

void P_RunThinkers(void)
{
	thinker_t* temp_79f3315cca1;

	currentthinker = thinkercap.next;
	temp_79f3315cca1 = currentthinker;
	while (currentthinker = temp_79f3315cca1, currentthinker != &thinkercap) 
	{
		temp_79f3315cca1 = currentthinker->next;
		if (temp_79f3315cca1->prev != currentthinker) 
		{
			IO_Error("P_RunThinkers: bad link in list\n");
		}
		if (currentthinker->function != NULL) 
		{
			((void(*)())currentthinker->function)(currentthinker);
		}
	}
	return;
}

void P_CheckDebugKeys(void)
{
	if (keydown[KEY_M] != 0) 
	{
		debugmove = debugmove ^ 1;
		keydown[KEY_M] = 0;
	}
	if ((keydown[KEY_T] != 0) && (keydown[KEY_DEBUG] != 0)) 
	{
		TimeSpin();
	}
	if (keydown[KEY_Q] != 0) 
	{
		/*if (keydown[KEY_DEBUG])
		{
			D_SpinScreenOut();
			IO_Quit();
		}*/

		R_SetDetail(0);
		keydown[KEY_Q] = 0;
	}
	if (keydown[KEY_W] != 0)
	{
		R_SetDetail(1);
		keydown[KEY_W] = 0;
	}
	if (keydown[KEY_E] != 0) 
	{
		R_SetDetail(2);
		keydown[KEY_E] = 0;
	}
	/*if (keydown[KEY_R]) //[ISB]
	{
		R_SetDetail(3);
	}*/
	if (keydown[KEY_LBRACKET] != 0) 
	{
		R_SizeDown();
		keydown[KEY_LBRACKET] = 0;
	}
	if (keydown[KEY_RBRACKET] != 0) 
	{
		R_SizeUp();
		keydown[KEY_RBRACKET] = 0;
	}
	if ((demo == 1) && (keydown[KEY_D] != 0)) 
	{
		gameaction = 2;
		keydown[KEY_D] = 0;
	}

	return;
}

void P_ProcessFrames(void)
{
	void* pvVar1;
	int uVar2;

	playernum = 0;
	while (playernum < 4) 
	{
		if (sd->playeringame[playernum] != 0 && (sd->playercmdframe[playernum] <= processedframe))
		{
			playernum = 0;
		}
		playernum++;
	}
	while (1)
	{
		playernum = 0;
		while (playernum < 4)
		{
			if (sd->playeringame[playernum] != 0 && (sd->playercmdframe[playernum] <= processedframe)) 
			{
				return;
			}
			playernum++;
		}
		processedframe++;
		uVar2 = processedframe & 0x1f;
		M_CheckInput(&sd->playercmd[sd->consoleplayer * 0x20 + uVar2]);
		
		if (automapup != 0) 
		{
			P_RunAutoMap(sd->playercmd[sd->consoleplayer * 0x20 + uVar2].keyscan);
		}
		playernum = 0;
		while (playernum < 4) 
		{
			if (sd->playeringame[playernum] != 0) 
			{
				player = playerobjs + playernum;
				if (demo == 2) 
				{
					P_PlayerGameThink((framecmd_t*)demo_p);
					demo_p = (void*)((int)demo_p + 4);
					if (demo_p == demoend) 
					{
						gameaction = ga_completed;
						return;
					}
				}
				else 
				{
					P_PlayerGameThink(&sd->playercmd[playernum * 0x20 + uVar2]);
					pvVar1 = demo_p;
					if (demo == 1) 
					{
						if (demo_p == demoend)
						{
							gameaction = ga_completed;
							return;
						}
						demo_p = (void*)((int)demo_p + 4);
						*(framecmd_t*)pvVar1 = sd->playercmd[playernum * 0x20 + uVar2];
					}
				}
			}
			playernum++;
		}
		if (gameaction != ga_playing) break;
		P_MovePlayerShapes();
		P_RunThinkers();
		if (redshift != 0) 
		{
			redshift--;
		}
		if (goldshift != 0) 
		{
			goldshift--;
		}
	}
	return;
}

void P_PlayLoop(void)
{
	int* piVar1;
	uint8_t* palette;
	thing_t* local_20;
	int iVar2;

	screenfaded = 1;
	P_SetPlayPalette();
	automapup = 0;
	R_SetViewSize(config.viewsize, config.hdetail, 1);
	P_DrawPlayScreen();
	iVar2 = sd->consoleplayer;
	sd->playercmdframe[iVar2] = -1;
	processedframe = sd->playercmdframe[iVar2];
	iVar2 = 0;
	while (iVar2 < 4) 
	{
		sd->playercmdframe[iVar2] = -1;
		iVar2 = iVar2 + 1;
	}
	gameaction = ga_playing;
	do
	{
		IO_NewFrame();
		P_ProcessFrames();
		P_CheckDebugKeys();
		P_SetShiftPalette();
		P_AnimatePlanePics();
		if (automapup == 0) 
		{
			local_20 = playerobjs[viewplayer].r;
			R_RenderView(local_20->sector, local_20->x, local_20->y, playerobjs[viewplayer].viewz, local_20->angle + viewplayerangle & 0x1fff);
			P_DrawPlayerShapes(viewplayer);
		}
		M_DrawSelf();
		IO_UpdateScreen();
		if (screenfaded != 0) 
		{
			screenfaded = 0;
			palette = W_GetName("PLAYPAL");
			D_FadeIn(palette);
			D_Synchronize();
		}
	} while ((gameaction == ga_playing) && (demoaction == da_gameloop));
	return;
}

void P_Startup()
{
	playzone = Z_AllocateZone(0x60000);
	R_InitSprites(sprnames);
	P_StartupPicAnims();
	P_InitSwitchList();
	thinkercap.next = &thinkercap;
	thinkercap.prev = &thinkercap;
	playpalette = W_GetName("PLAYPAL");
	wnumber0 = W_GetNumForName("I_SMNUM0");
	snumber0 = W_GetNumForName("I_LGNUM0");
	cardnumber = W_GetNumForName("I_IPIC00");
	healthbar = (pic_t*)W_GetName("I_HBAR");
	armorbar = (pic_t*)W_GetName("I_ABAR");
	timebar = (pic_t*)W_GetName("I_TBAR");
	blankbar = (pic_t*)W_GetName("I_NBAR");
}
