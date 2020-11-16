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
#pragma once

#include "doomdef.h"

typedef struct
{
    int picnum;
    int basepic;
    int numpics;
    int speed;
} anim_t;

void P_AnimatePlanePics();

//-----------------------------------------------------------------------------
// Doors
//-----------------------------------------------------------------------------

typedef enum
{
    normal,
    close30ThenOpen,
    close,
    open
} vldoor_e;

typedef struct  
{
    thinker_t thinker;
    vldoor_e type;
    sector_t* sector;
    fixed_t topheight;
    fixed_t speed;
    int direction;
    int topwait;
    int topcountdown;
} vldoor_t;

#define VDOORSPEED (FRACUNIT*2)
#define VDOORWAIT 150

//-----------------------------------------------------------------------------
// Floors
//-----------------------------------------------------------------------------

typedef struct  
{
    thinker_t thinker;
    sector_t* sector;
    int direction;
    int floordestheight;
    int speed;
} floormove_t;

#define FLOORSPEED FRACUNIT

//-----------------------------------------------------------------------------
// Platforms
//-----------------------------------------------------------------------------

typedef enum 
{
    down = 1,
    up = 0,
    waiting = 2
} plat_e;

typedef enum 
{
    downWaitUpStay = 1,
    raise = 0,
    raiseAndChange = 2
} plattype_e;

typedef struct  
{
    thinker_t thinker;
    sector_t* sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plattype_e type;
} plat_t;

#define PLATSPEED FRACUNIT
#define PLATWAIT 105

//-----------------------------------------------------------------------------
// Switches
//-----------------------------------------------------------------------------
void P_InitSwitchList();
void P_StartupPicAnims();

//-----------------------------------------------------------------------------
// Specials
//-----------------------------------------------------------------------------
void P_PlayerCrossSpecialLine(line_t* line);
void P_PlayerUseSpecialLine(line_t* line, int side);
void P_PlayerShootSpecialLine(line_t* line);

void P_PlayerInSpecialSector();

#define GLOWSPEED 8
#define	STROBEBRIGHT 5
#define	FASTDARK 15
#define SLOWDARK 35

typedef struct  
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;
} lightflash_t;

typedef struct 
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int darktime;
    int brighttime;
} lightstrobe_t;

void P_RunAutoMap(char ch);

void P_PlayerGameThink(framecmd_t* cmd);

//status
void P_DrawWeapon();
void P_DrawAmmo();
void P_DrawHealth();
void P_DrawArmor();
void P_DrawCards();
void P_DrawTime();
void P_DrawPlayScreen();

//psprites
void P_SetPSprite(int position, statenum_t stnum);
void P_SetupPSprites(int pnum);
void P_CalcSwing();
void P_DrawPlayerShapes(int pnum);
void P_MovePlayerShapes();
void P_BringUpWeapon();

//things/actors
void P_InitActors();
void P_InitThing(mapthing_t* mthing);

void P_AddActor(actor_t* newactor);
void P_RemoveActor(actor_t* actor);
void P_SetState(actor_t* actor, statenum_t state);

void P_DamagePlayer(int player, int damage);
void P_DamageEnemy(actor_t* actor, int damage);
