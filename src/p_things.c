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

spawninfo_t spawninfo[33] =
{
	{2001,S_SHOT,4},
	{2002,S_MGUN,4},
	{2003,S_LAUN,4},
	{2005,S_CSAW,4},
	{2007,S_CLIP,4},
	{2008,S_SHEL,4},
	{2010,S_MISL,4},
	{2011,S_STIM,4},
	{2012,S_MEDI,4},
	{2013,S_SOUL,4},
	{2014,S_BON1,4},
	{2015,S_BON2,4},
	{2016,S_BON3,4},
	{2017,S_BON4,4},
	{2018,S_ARM1,4},
	{2019,S_ARM2,4},
	{2022,S_POW1,4},
	{2023,S_POW2,4},
	{2024,S_POW3,4},
	{2025,S_POW4,4},
	{2026,S_POW5,4},
	{2027,S_GBAR,2},
	{2028,S_COLU,2},
	{2037,S_RBAR,2},
	{2045,S_IGOG,4},
	{2046,S_MMIS,4},
	{2048,S_MBUL,4},
	{2049,S_MSHE,4},
	{10,S_ELEC,4},
	{5,S_GKEY,4},
	{6,S_SKEY,4},
	{7,S_BKEY,4},
	{-1,S_NULL,0},
};

actor_t actor;
actor_t* actorcap;

uint8_t* P_BlockOrg(int x, int y)
{
	int tx = (x - maporiginx) + -0x100000 >> 0x14;
	int ty = (y - maporiginy) + -0x100000 >> 0x14;

	return &blockmap[tx + mapwidth * ty];
}

int P_PlaceGetMarks(thing_t* rthing)
{
	uint8_t* map;

	map = P_BlockOrg(rthing->x, rthing->y);

	if (((((map[0] & 4) == 0) && ((map[1] & 4) == 0)) && ((map[mapwidth] & 4) == 0)) && ((map[mapwidth + 1] & 4) == 0)) 
	{
		map[0] |= 4;
		map[1] |= 4;
		map[mapwidth] |= 4;
		map[mapwidth + 1] |= 4;
		rthing->flags |= 1;
		return 1;
	}
	else 
	{
		return 0;
	}
}

void P_PlaceBlockMarks(thing_t* rthing)
{
	uint8_t* map;

	map = P_BlockOrg(rthing->x, rthing->y);

	map[0] |= 2;
	map[1] |= 2;
	map[mapwidth] |= 2;
	map[mapwidth + 1] |= 2;

	rthing->flags |= 8;
	return;
}

void P_RemoveGetMarks(thing_t* rthing)
{
	uint8_t* map;

	map = P_BlockOrg(rthing->x, rthing->y);

	map[0] &= 0xfb;
	map[1] &= 0xfb;
	map[mapwidth] &= 0xfb;
	map[mapwidth + 1] &= 0xfb;
	return;
}

void P_RemoveBlockMarks(thing_t* rthing)
{
	uint8_t* map;

	map = P_BlockOrg(rthing->x, rthing->y);

	map[0] &= 0xfd;
	map[1] &= 0xfd;
	map[mapwidth] &= 0xfd;
	map[mapwidth + 1] &= 0xfd;
	return;
}

void P_InitActors(void)
{
	actor.prev = &actor;
	actor.next = &actor;
	return;
}

void P_AddActor(actor_t* newactor)
{
	//[ISB] TODO: No check in the decompiled source, need to double check if this is an actual hazard in the original.
	if (actor.prev != NULL)
		(actor.prev)->next = newactor;

	newactor->next = &actor;
	newactor->prev = actor.prev;
	actor.prev = newactor;
	P_AddThinker((thinker_t*)newactor);
	return;
}

void P_RemoveActor(actor_t* newactor)
{
	if ((newactor->r->flags & 8) != 0) 
	{
		P_RemoveBlockMarks(newactor->r);
	}
	if ((newactor->r->flags & 1) != 0)
	{
		P_RemoveGetMarks(newactor->r);
	}
	newactor->next->prev = newactor->prev;
	newactor->prev->next = newactor->next;
	R_RemoveThing(newactor->r);
	P_RemoveThinker((thinker_t*)newactor);
	return;
}

void P_SetState(actor_t* actor, statenum_t state)
{
	if (state == S_NULL) 
	{
		P_RemoveActor(actor);
	}
	else
	{
		actor->state = &states[state];
		actor->tics = states[state].tics;
		actor->r->sprite = states[state].sprite;
		actor->r->frame = states[state].frame;
		if (states[state].action != (void*)NULL)
		{
			(*(void(*)(actor_t*))states[state].action)(actor);
		}
	}
	return;
}

void T_StateCycleMove(actor_t* actor)
{
	if (actor->tics != -1) 
	{
		actor->tics = actor->tics + -1;
		while (actor->tics == 0)
		{
			P_SetState(actor, actor->state->nextstate);
		}
	}
	return;
}

thing_t* P_GetNewThing(mapthing_t* mthing)
{
	thing_t* refthing;

	refthing = R_GetNewThing(mthing->sector);
	refthing->x = (int)mthing->origin.x << FRACBITS;
	refthing->y = (int)mthing->origin.y << FRACBITS;
	refthing->z = sectors[mthing->sector].floorheight;

	if (mthing->ang > 359) 
	{
		IO_Error("P_GetNewThing: bad angle for thing at (%i,%i)\n", mthing->origin.x, mthing->origin.y);
	}
	refthing->angle = ((int)mthing->ang << 0xd) / 360;
	refthing->flags = 0;

	return refthing;
}

actor_t* P_InitActor(mapthing_t* mthing, statenum_t state, int bblockflags)
{
	actorcap = (actor_t*)Z_Malloc(playzone, sizeof(actor_t));
	actorcap->r = P_GetNewThing(mthing);
	actorcap->r->specialdata = actorcap;
	actorcap->thinker.function = &T_StateCycleMove;
	actorcap->maporigin = P_BlockOrg(actorcap->r->x, actorcap->r->y);
	P_AddActor(actorcap);
	P_SetState(actorcap, state);

	if ((bblockflags & 2) == 0) 
	{
		if ((bblockflags & 4) != 0) 
		{
			if (P_PlaceGetMarks(actorcap->r) == 0)
			{
				IO_Error("P_InitActor: Overlapping getable objects at %i, %i\n", actorcap->r->x >> FRACBITS, actorcap->r->y >> FRACBITS);
			}
		}
	}
	else 
	{
		P_PlaceBlockMarks(actorcap->r);
	}

	return actorcap;
}

void P_SpawnPlayer(mapthing_t* mthing)
{
	thing_t* rthing;
	player_t* player;

	rthing = P_GetNewThing(mthing);
	playerthingfound[mthing->type-1] = 1;
	player = &playerobjs[mthing->type - 1];

	player->r = rthing;
	rthing->sprite = SPR_PLAY;
	rthing->frame = 0;
	rthing->specialdata = player;
	rthing->flags = 0xc; 

	playerobjs[mthing->type - 1].pendingweapon = wp_nochange;

	P_SetupPSprites(mthing->type - 1);

	P_PlaceBlockMarks(player->r);
	return;
}

void P_InitThing(mapthing_t* mthing)
{
	spawninfo_t* sp;

	if (mthing->type != 0)
	{
		if (mthing->type < 5)
		{
			P_SpawnPlayer(mthing);
			return;
		}
		if (mthing->type == 3001)
		{
			P_InitActor(mthing, S_TROO_STND, 2);
			actorcap->health = 20;
			return;
		}
		else if (mthing->type == 3002)
		{
			P_InitActor(mthing, S_SARG_STND, 2);
			actorcap->health = 40;
			return;
		}
		else if (mthing->type == 3003)
		{
			P_InitActor(mthing, S_BOSS_STND, 2);
			actorcap->health = 250;
			return;
		}
		else if (mthing->type == 3004)
		{
			P_InitActor(mthing, S_POSS_STND, 2);
			actorcap->health = 10;
			return;
		}
		else if (mthing->type == 3005)
		{
			P_InitActor(mthing, S_HEAD_STND, 2);
			actorcap->health = 100;
			return;
		}
	}
	sp = &spawninfo[0];
	for(;;)
	{
		if (sp->number == -1) 
		{
			//[ISB]
			//fprintf(stderr, "P_InitThing: unknown spawn number %d\n", mthing->type);
			return;
		}
		if ((int)mthing->type == sp->number) break;
		sp++;
	}
	P_InitActor(mthing, sp->state, sp->bmapflags);
	return;
}

