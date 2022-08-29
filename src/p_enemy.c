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

void P_DamageEnemy(actor_t* actor, int damage)
{
	actor->health -= damage;
	P_RemoveActor(actor);
	return;

	//[ISB] for fun
	/*statenum_t painstate = S_NULL;
	statenum_t deathstate = S_NULL;

	if (actor->health <= 0) return;

	actor->health -= damage;

	switch (actor->r->sprite)
	{
	case SPR_TROO:
		painstate = S_TROO_PAIN;
		deathstate = S_TROO_DIE1;
		break;
	case SPR_SARG:
		painstate = S_SARG_PAIN;
		deathstate = S_SARG_DIE1;
		break;
	case SPR_HEAD:
		painstate = S_HEAD_PAIN;
		deathstate = S_HEAD_DIE1;
		break;
	case SPR_POSS:
		painstate = S_POSS_PAIN;
		deathstate = S_POSS_DIE1;
		break;
	case SPR_BOSS:
		painstate = S_BOSS_PAIN;
		deathstate = S_BOSS_DIE1;
		break;
	}

	if (painstate != S_NULL)
	{
		if (actor->health > 0)
			P_SetState(actor, painstate);
		else
		{
			P_SetState(actor, deathstate);
			P_RemoveBlockMarks(actor->r);
		}
	}*/
}

//the ultimate disappointment
void A_Look(actor_t *actor)
{
}

void A_Chase(actor_t* actor)
{
}

void A_Attack(actor_t* actor)
{
}

void A_Scream(actor_t* actor)
{
}
