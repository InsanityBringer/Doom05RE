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
#include "p_actor.h"
#include "r_local.h"

typedef struct
{
	int number;
	statenum_t state;
	int bmapflags;
} spawninfo_t;

spawninfo_t spawninfo[33] =
{
	{2001,S_SHOT,BMF_GETTABLE},
	{2002,S_MGUN,BMF_GETTABLE},
	{2003,S_LAUN,BMF_GETTABLE},
	{2005,S_CSAW,BMF_GETTABLE},
	{2007,S_CLIP,BMF_GETTABLE},
	{2008,S_SHEL,BMF_GETTABLE},
	{2010,S_MISL,BMF_GETTABLE},
	{2011,S_STIM,BMF_GETTABLE},
	{2012,S_MEDI,BMF_GETTABLE},
	{2013,S_SOUL,BMF_GETTABLE},
	{2014,S_BON1,BMF_GETTABLE},
	{2015,S_BON2,BMF_GETTABLE},
	{2016,S_BON3,BMF_GETTABLE},
	{2017,S_BON4,BMF_GETTABLE},
	{2018,S_ARM1,BMF_GETTABLE},
	{2019,S_ARM2,BMF_GETTABLE},
	{2022,S_POW1,BMF_GETTABLE},
	{2023,S_POW2,BMF_GETTABLE},
	{2024,S_POW3,BMF_GETTABLE},
	{2025,S_POW4,BMF_GETTABLE},
	{2026,S_POW5,BMF_GETTABLE},
	{2027,S_GBAR,BMF_SOLID},
	{2028,S_COLU,BMF_SOLID},
	{2037,S_RBAR,BMF_SOLID},
	{2045,S_IGOG,BMF_GETTABLE},
	{2046,S_MMIS,BMF_GETTABLE},
	{2048,S_MBUL,BMF_GETTABLE},
	{2049,S_MSHE,BMF_GETTABLE},
	{10,S_ELEC,BMF_GETTABLE},
	{5,S_GKEY,BMF_GETTABLE},
	{6,S_SKEY,BMF_GETTABLE},
	{7,S_BKEY,BMF_GETTABLE},
	{-1,S_NULL,0},
};

actor_t actorcap;
actor_t* actor;

byte* P_BlockOrg(int x, int y)
{
	int tx = (x - maporiginx) - MAPBLOCKSIZE >> MAPBLOCKSHIFT;
	int ty = (y - maporiginy) - MAPBLOCKSIZE >> MAPBLOCKSHIFT;

	return &blockmap[tx + mapwidth * ty];
}

boolean P_PlaceGetMarks(thing_t* rthing)
{
	byte* map = P_BlockOrg(rthing->x, rthing->y);

	if (!(map[0] & BMF_GETTABLE) && !(map[1] & BMF_GETTABLE) && !(map[mapwidth] & BMF_GETTABLE) && !(map[mapwidth + 1] & BMF_GETTABLE))
	{
		map[0] |= BMF_GETTABLE;
		map[1] |= BMF_GETTABLE;
		map[mapwidth] |= BMF_GETTABLE;
		map[mapwidth + 1] |= BMF_GETTABLE;
		rthing->flags |= TF_GETTABLE;
		return true;
	}

	return false;
}

void P_PlaceBlockMarks(thing_t* rthing)
{
	byte* map = P_BlockOrg(rthing->x, rthing->y);

	map[0] |= BMF_SOLID;
	map[1] |= BMF_SOLID;
	map[mapwidth] |= BMF_SOLID;
	map[mapwidth + 1] |= BMF_SOLID;

	rthing->flags |= TF_SOLID;
}

void P_RemoveGetMarks(thing_t* rthing)
{
	byte* map = P_BlockOrg(rthing->x, rthing->y);

	map[0] &= ~BMF_GETTABLE;
	map[1] &= ~BMF_GETTABLE;
	map[mapwidth] &= ~BMF_GETTABLE;
	map[mapwidth + 1] &= ~BMF_GETTABLE;
}

void P_RemoveBlockMarks(thing_t* rthing)
{
	byte* map = P_BlockOrg(rthing->x, rthing->y);

	map[0] &= ~BMF_SOLID;
	map[1] &= ~BMF_SOLID;
	map[mapwidth] &= ~BMF_SOLID;
	map[mapwidth + 1] &= ~BMF_SOLID;
}

void P_InitActors(void)
{
	actorcap.prev = &actorcap;
	actorcap.next = &actorcap;
}

void P_AddActor(actor_t* actor)
{
	//BUG: This doesn't check that actor.prev is valid in the original
#ifndef __WATCOMC__
	if (actorcap.prev != NULL)
		(actorcap.prev)->next = actor;
#endif

	actor->next = &actorcap;
	actor->prev = actorcap.prev;
	actorcap.prev = actor;
	P_AddThinker(&actor->thinker);
}

void P_RemoveActor(actor_t* actor)
{
	if (actor->r->flags & TF_SOLID)
		P_RemoveBlockMarks(actor->r);
	
	if (actor->r->flags & TF_GETTABLE)
		P_RemoveGetMarks(actor->r);
	
	actor->next->prev = actor->prev;
	actor->prev->next = actor->next;
	R_RemoveThing(actor->r);
	P_RemoveThinker(&actor->thinker);
}

void P_SetState(actor_t* actor, statenum_t state)
{
	state_t* st;

	if (state == S_NULL)
	{
		P_RemoveActor(actor);
		return;
	}

	st = &states[state];
	actor->state = st;
	actor->tics = st->tics;
	actor->r->sprite = st->sprite;
	actor->r->frame = st->frame;
	if (st->action)
		st->action(actor);
}

void T_StateCycleMove(actor_t* actor)
{
	if (actor->tics != -1)
	{
		actor->tics--;

		// you can cycle through multiple states in a tic
		while (actor->tics == 0)
			P_SetState(actor, actor->state->nextstate);
	}
}

thing_t* P_GetNewThing(mapthing_t* mthing)
{
	thing_t* refthing = R_GetNewThing(mthing->sector);
	refthing->x = (int)mthing->origin.x << FRACBITS;
	refthing->y = (int)mthing->origin.y << FRACBITS;
	refthing->z = sectors[mthing->sector].floorheight;

	if (mthing->ang > 359)
		IO_Error("P_GetNewThing: bad angle for thing at (%i,%i)", mthing->origin.x, mthing->origin.y);
	
	refthing->angle = ((int)mthing->ang << 13) / 360;
	refthing->flags = 0;

	return refthing;
}

actor_t* P_InitActor(mapthing_t* mthing, statenum_t state, int bblockflags)
{
	actor = (actor_t*)Z_Malloc(playzone, sizeof(actor_t));
	actor->r = P_GetNewThing(mthing);
	actor->r->specialdata = actor;
	actor->thinker.function = &T_StateCycleMove;
	actor->maporigin = P_BlockOrg(actor->r->x, actor->r->y);
	P_AddActor(actor);
	P_SetState(actor, state);

	if (bblockflags & BMF_GETTABLE)
	{
		if (!P_PlaceGetMarks(actor->r))
			IO_Error("P_InitActor: Overlapping getable objects at %i, %i", actor->r->x >> FRACBITS, actor->r->y >> FRACBITS);
	}
	else if (bblockflags & BMF_SOLID)
	{
		P_PlaceBlockMarks(actor->r);
	}

	return actor;
}

void P_SpawnPlayer(mapthing_t* mthing)
{
	thing_t* rthing;
	player_t* player;

	rthing = P_GetNewThing(mthing);
	playerthingfound[mthing->type - 1] = true;
	player = &playerobjs[mthing->type - 1];

	player->r = rthing;
	rthing->sprite = SPR_PLAY;
	rthing->frame = 0;
	rthing->specialdata = player;
	rthing->flags = TF_PLAYER | TF_SOLID;

	playerobjs[mthing->type - 1].pendingweapon = wp_nochange;

	P_SetupPSprites(mthing->type - 1);

	P_PlaceBlockMarks(player->r);
}

void P_InitThing(mapthing_t* mthing)
{
	spawninfo_t* sp;

	switch (mthing->type) //[ISB] don't know for sure if it's a switch, need to do testing.
	{
	case 1:
	case 2:
	case 3:
	case 4:
		P_SpawnPlayer(mthing);
		return;
	case 3001:
		P_InitActor(mthing, S_TROO_STND, BMF_SOLID);
		actor->health = 20;
		return;
	case 3002:
		P_InitActor(mthing, S_SARG_STND, BMF_SOLID);
		actor->health = 40;
		return;
	case 3003:
		P_InitActor(mthing, S_BOSS_STND, BMF_SOLID);
		actor->health = 250;
		return;
	case 3004:
		P_InitActor(mthing, S_POSS_STND, BMF_SOLID);
		actor->health = 10;
		return;
	case 3005:
		P_InitActor(mthing, S_HEAD_STND, BMF_SOLID);
		actor->health = 100;
		return;
	}

	sp = &spawninfo[0];
	for (;;)
	{
		if (sp->number == -1)
		{
			//[ISB]
			//fprintf(stderr, "P_InitThing: unknown spawn number %d\n", mthing->type);
			return;
		}
		if (mthing->type == sp->number) break;
		sp++;
	}
	P_InitActor(mthing, sp->state, sp->bmapflags);
}
