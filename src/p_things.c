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
	return (((x - maporiginx) + -0x100000 >> 0x14) +
		mapwidth * ((y - maporiginy) + -0x100000 >> 0x14) +
		blockmap);
}

int P_PlaceGetMarks(thing_t* rthing)
{
	uint8_t* pbVar1;
	thing_t* local_24;
	int local_20;

	local_24 = rthing;
	pbVar1 = P_BlockOrg(rthing->x, rthing->y);
	if (((((*pbVar1 & 4) == 0) && ((pbVar1[1] & 4) == 0)) && ((pbVar1[mapwidth] & 4) == 0))
		&& ((pbVar1[mapwidth + 1] & 4) == 0)) 
	{
		*pbVar1 |= 4;
		pbVar1[1] |= 4;
		pbVar1[mapwidth] |= 4;
		pbVar1[mapwidth + 1] |= 4;
		local_24->flags |= 1;
		local_20 = 1;
	}
	else 
	{
		local_20 = 0;
	}
	return local_20;
}

void P_PlaceBlockMarks(thing_t* rthing)
{
	uint8_t* pbVar1;
	thing_t* local_20;

	local_20 = rthing;
	pbVar1 = P_BlockOrg(rthing->x, rthing->y);
	*pbVar1  |= 2;
	pbVar1[1] |= 2;
	pbVar1[mapwidth] |= 2;
	pbVar1[mapwidth + 1] |= 2;
	local_20->flags |= 8;
	return;
}

void P_RemoveGetMarks(thing_t* rthing)
{
	uint8_t* pbVar1;

	pbVar1 = P_BlockOrg(rthing->x, rthing->y);
	*pbVar1 &= 0xfb;
	pbVar1[1] &= 0xfb;
	pbVar1[mapwidth] &= 0xfb;
	pbVar1[mapwidth + 1] &= 0xfb;
	return;
}

void P_RemoveBlockMarks(thing_t* rthing)
{
	uint8_t* pbVar1;

	pbVar1 = P_BlockOrg(rthing->x, rthing->y);
	*pbVar1 &= 0xfd;
	pbVar1[1] &= 0xfd;
	pbVar1[mapwidth] &= 0xfd;
	pbVar1[mapwidth + 1] &= 0xfd;
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
			//[ISB] hrm, so this doesn't work at all because a param isn't passed. Heh.
			//TODO: Could be RE error, so double check.
			(*(void(*)())states[state].action)();
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
	thing_t* ptVar1;

	ptVar1 = R_GetNewThing(mthing->sector);
	ptVar1->x = (int)mthing->origin.x << FRACBITS;
	ptVar1->y = (int)mthing->origin.y << FRACBITS;
	ptVar1->z = sectors[mthing->sector].floorheight;
	if (359 < mthing->ang) 
	{
		IO_Error("P_GetNewThing: bad angle for thing at (%i,%i)\n", mthing->origin.x, mthing->origin.y);
	}
	ptVar1->angle = ((int)mthing->ang << 0xd) / 360;
	ptVar1->flags = 0;
	return ptVar1;
}

actor_t* P_InitActor(mapthing_t* mthing, statenum_t state, int bblockflags)
{
	thing_t* ptVar1;
	uint8_t* pbVar2;
	int iVar3;
	mapthing_t* local_20;

	actorcap = (actor_t*)Z_Malloc(playzone, sizeof(actor_t));
	ptVar1 = P_GetNewThing(mthing);
	actorcap->r = ptVar1;
	actorcap->r->specialdata = actorcap;
	(actorcap->thinker).function = &T_StateCycleMove;
	pbVar2 = P_BlockOrg(actorcap->r->x, actorcap->r->y);
	actorcap->maporigin = pbVar2;
	P_AddActor(actorcap);
	P_SetState(actorcap, state);
	if ((bblockflags & 2) == 0) 
	{
		if ((bblockflags & 4) != 0) 
		{
			iVar3 = P_PlaceGetMarks(actorcap->r);
			if (iVar3 == 0)
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
	short sVar1;
	thing_t* ptVar2;
	int iVar3;
	player_t* local_20;

	ptVar2 = P_GetNewThing(mthing);
	playerthingfound[mthing->type-1] = 1;
	local_20 = &playerobjs[mthing->type - 1];

	local_20->r = ptVar2;
	ptVar2->sprite = SPR_PLAY;
	ptVar2->frame = 0;
	ptVar2->specialdata = local_20;
	ptVar2->flags = 0xc;

	//TODO: I'm honestly not sure what this is referencing. All player setup is done elsewhere
	//(&goldshift)[(int)sVar1 * 0x20] = 0xf;

	P_SetupPSprites((int)mthing->type - 1);

	P_PlaceBlockMarks(local_20->r);
	return;
}

void P_InitThing(mapthing_t* mthing)
{
	unsigned short uVar1;
	spawninfo_t* local_1c;

	uVar1 = mthing->type;
	if (uVar1 != 0)
	{
		if (uVar1 < 5)
		{
			P_SpawnPlayer(mthing);
			return;
		}
		if (uVar1 == 3001) 
		{
			P_InitActor(mthing, S_TROO_STND, 2);
			actorcap->health = 20;
			return;
		}
		else if (uVar1 == 3002) 
		{
			P_InitActor(mthing, S_SARG_STND, 2);
			actorcap->health = 40;
			return;
		}
		else if (uVar1 == 3003) 
		{
			P_InitActor(mthing, S_BOSS_STND, 2);
			actorcap->health = 250;
			return;
		}
		else if (uVar1 == 3004)
		{
			P_InitActor(mthing, S_POSS_STND, 2);
			actorcap->health = 10;
			return;
		}
		else if (uVar1 == 3005) 
		{
			P_InitActor(mthing, S_HEAD_STND, 2);
			actorcap->health = 100;
			return;
		}
	}
	local_1c = &spawninfo[0];
	for(;;)
	{
		if (local_1c->number == -1) 
		{
			//[ISB]
			//fprintf(stderr, "P_InitThing: unknown spawn number %d\n", mthing->type);
			return;
		}
		if ((int)mthing->type == local_1c->number) break;
		local_1c = local_1c + 1;
	}
	P_InitActor(mthing, local_1c->state, local_1c->bmapflags);
	return;
}

