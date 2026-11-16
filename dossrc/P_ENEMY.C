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

void P_DamageEnemy(actor_t* actor, int damage)
{
	actor->health -= damage;
	P_RemoveActor(actor);
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
