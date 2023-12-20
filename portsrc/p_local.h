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

#ifndef __P_LOCAL_H__
#define __P_LOCAL_H__

//The play loop is dependent on the refresh
#include "r_local.h"

#define USERANGE (40*FRACUNIT)

#define NUMPLAYERFRAMES 32
#define PLAYERFRAMEMASK (NUMPLAYERFRAMES-1)

// player radius for movement checking
#define	PLAYERRADIUS	16*FRACUNIT

typedef struct
{
	signed char xmove;
	signed char ymove;
	byte buttons;
	byte keyscan;
} framecmd_t;

#define BT_USE 1
#define BT_ATTACK 2
#define BT_WEAPONMASK 0xF
#define BT_WEAPONSHIFT 2

typedef struct
{
	int sector;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	int angle;
} framepos_t;

typedef struct
{
	int ident;
	int startgame;
	int commint;
	int consoleplayer;
	byte playeringame[MAXPLAYERS];
	int playercmdframe[MAXPLAYERS];
	framecmd_t playercmd[MAXPLAYERS * NUMPLAYERFRAMES];
	int mousex;
	int mousey;
	int mousebuttons;
	int mousepresent;
	int oldmousex;
	int oldmousey;
	int lastframe;
	framepos_t framepos[2];
	int viewpos;
	char mapname[9];
} shared_t;

typedef enum
{
	wp_knife,
	wp_rifle,
	wp_shotgun,
	wp_auto,
	wp_missile,
	wp_chainsaw,
	wp_claw,
	wp_bfg,

	wp_nochange = 15
} weapontype_t;

#define MAXCLIP 199
#define MAXSHELL 99
#define MAXCELL 9
#define MAXSOUL 66
#define MAXMISL 9

typedef enum
{
	am_clip,
	am_shell,
	am_cell,
	am_soul,
	am_misl,
	NUMAMMO
} ammotype_t;

typedef enum
{
	dm_user,
	dm_recording,
	dm_playback
} demostate_t;

typedef enum
{
	it_bluecard,
	it_yellowcard,
	it_redcard
} item_t;

#define MAXBODY 15
#define MAXARMOR 15

typedef struct
{
	int viewz;
	struct thing_s* r;
	int momx;
	int momy;
	int momz;
	int bob;
	int lives;
	int score;
	int nextextra;
	int health;
	int armor;
	int itemtime;
	int items[3];
	weapontype_t readyweapon;
	weapontype_t pendingweapon;
	int weaponowned[8];
	int ammo[NUMAMMO];
	int firedown;
	int strafedown;
} player_t;

#define SBARHEIGHT 32

extern shared_t* sd;

extern int processedframe;

extern memzone_t* playzone;

extern player_t* player;
extern player_t playerobjs[];
extern boolean playerthingfound[MAXPLAYERS];
extern int playernum;
extern int viewplayer;
extern int viewplayerangle;
extern int debugmove;

extern boolean automapup;
extern int goldshift;
extern int redshift;

extern byte* demoend;
extern demostate_t demo;
extern byte* demo_p;

extern int snumber0;
extern int wnumber0;
extern fixed_t amaporgy;
extern fixed_t amaporgx;
extern int cardnumber;
extern pic_t* healthbar;
extern pic_t* armorbar;
extern pic_t* timebar;
extern pic_t* blankbar;

//playsim
void P_Startup();
void P_InitThinkers();
void P_PlayLoop();

void P_SpawnSpecialSectors();

void P_EnterAutoMap();

void P_InitThing(mapthing_t* mthing);

//trace
void P_PlayerShoot();
int P_CrossSectorBounds(int sector, fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

//statues
void P_DrawPlayScreen(); //Needed by platform-specific video code. 

//things/actors
void P_InitActors();

#endif
