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
#ifndef __P_ACTOR_H__
#define __P_ACTOR_H__

// [ISB] I need a better name for this header.
// The debug information doesn't always have the thinkers after the playloop, but if it is there, it's always after states. 

#include "p_local.h"
#include "states.h" //[ISB] the old id utility statescr makes a states.c and states.h, so replicate that. But think_t always follows the state_t type in the debug information. 

// think_t is a function pointer to a routine to handle an actor
typedef void (*think_t) ();

typedef struct thinker_s
{
	struct thinker_s* prev;
	struct thinker_s* next;
	think_t function;
	int tag;
} thinker_t;

typedef struct actor_s
{
	thinker_t thinker;
	thing_t* r;
	struct actor_s* prev;
	struct actor_s* next;
	state_t* state;
	int tics;
	byte* maporigin;
	int health;
} actor_t;

extern actor_t actorcap;
extern actor_t* actor;

void P_AnimatePlanePics();

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

void P_RunAutoMap(byte ch);

void P_PlayerGameThink(framecmd_t* cmd);

//status
void P_DrawWeapon();
void P_DrawAmmo();
void P_DrawHealth();
void P_DrawArmor();
void P_DrawCards();
void P_DrawTime();

//psprites
void P_SetupPSprites(int pnum);
void P_DrawPlayerShapes(int pnum);
void P_MovePlayerShapes();

//thinkers
void P_AddThinker(thinker_t* thinker);
void P_RemoveThinker(thinker_t* thinker);

void P_InitThing(mapthing_t* mthing);

void P_AddActor(actor_t* newactor);
void P_RemoveActor(actor_t* actor);
void P_SetState(actor_t* actor, statenum_t state);

void P_DamagePlayer(int player, int damage);
void P_DamageEnemy(actor_t* actor, int damage);

//blockmap
byte* P_BlockOrg(int x, int y);
int P_PlaceGetMarks(thing_t* rthing);
void P_PlaceBlockMarks(thing_t* rthing);
void P_RemoveGetMarks(thing_t* rthing);
void P_RemoveBlockMarks(thing_t* rthing);

#endif
