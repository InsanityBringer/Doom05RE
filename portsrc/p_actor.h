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

#include "p_local.h"

//why, ghidra, just why
typedef enum
{
	NUMSPRITES = 43,
	SPR_ARM1 = 24,
	SPR_ARM2 = 25,
	SPR_BKEY = 42,
	SPR_BON1 = 20,
	SPR_BON2 = 21,
	SPR_BON3 = 22,
	SPR_BON4 = 23,
	SPR_BOSS = 9,
	SPR_CLIP = 14,
	SPR_COLU = 32,
	SPR_CSAW = 13,
	SPR_ELEC = 39,
	SPR_GBAR = 31,
	SPR_GKEY = 40,
	SPR_HEAD = 8,
	SPR_IGOG = 35,
	SPR_LAUN = 12,
	SPR_MBUL = 37,
	SPR_MEDI = 18,
	SPR_MGUN = 11,
	SPR_MISL = 16,
	SPR_MMIS = 36,
	SPR_MSHE = 38,
	SPR_PBAY = 2,
	SPR_PLAY = 5,
	SPR_PMAC = 4,
	SPR_POSS = 6,
	SPR_POW1 = 26,
	SPR_POW2 = 27,
	SPR_POW3 = 28,
	SPR_POW4 = 29,
	SPR_POW5 = 30,
	SPR_PRIF = 3,
	SPR_PSHO = 1,
	SPR_RBAR = 33,
	SPR_SARG = 7,
	SPR_SHEL = 15,
	SPR_SHOT = 10,
	SPR_SKEY = 41,
	SPR_SOUL = 19,
	SPR_SPAR = 34,
	SPR_STIM = 17,
	SPR_TROO = 0
} spritenum_t;

typedef enum
{
	NUMSTATES = 144,
	S_ARM1 = 125,
	S_ARM2 = 126,
	S_AUTO = 18,
	S_AUTO1 = 19,
	S_AUTO2 = 20,
	S_AUTO3 = 21,
	S_AUTO4 = 22,
	S_AUTODOWN = 23,
	S_AUTOFLASH1 = 25,
	S_AUTOFLASH2 = 26,
	S_AUTOUP = 24,
	S_BAYONET = 2,
	S_BAYONET1 = 3,
	S_BAYONET2 = 4,
	S_BAYONET3 = 5,
	S_BAYONET4 = 6,
	S_BAYONETDOWN = 7,
	S_BAYONETUP = 8,
	S_BKEY = 143,
	S_BON1 = 121,
	S_BON2 = 122,
	S_BON3 = 123,
	S_BON4 = 124,
	S_BOSS_ATK1 = 102,
	S_BOSS_ATK2 = 103,
	S_BOSS_ATK3 = 104,
	S_BOSS_DIE1 = 106,
	S_BOSS_DIE2 = 107,
	S_BOSS_DIE3 = 108,
	S_BOSS_DIE4 = 109,
	S_BOSS_DIE5 = 110,
	S_BOSS_PAIN = 105,
	S_BOSS_RUN1 = 98,
	S_BOSS_RUN2 = 99,
	S_BOSS_RUN3 = 100,
	S_BOSS_RUN4 = 101,
	S_BOSS_STND = 97,
	S_CLIP = 115,
	S_COLU = 133,
	S_CSAW = 114,
	S_ELEC = 140,
	S_GBAR = 132,
	S_GKEY = 141,
	S_HEAD_ATK1 = 88,
	S_HEAD_ATK2 = 89,
	S_HEAD_ATK3 = 90,
	S_HEAD_DIE1 = 92,
	S_HEAD_DIE2 = 93,
	S_HEAD_DIE3 = 94,
	S_HEAD_DIE4 = 95,
	S_HEAD_DIE5 = 96,
	S_HEAD_PAIN = 91,
	S_HEAD_RUN1 = 84,
	S_HEAD_RUN2 = 85,
	S_HEAD_RUN3 = 86,
	S_HEAD_RUN4 = 87,
	S_HEAD_STND = 83,
	S_IGOG = 136,
	S_LAUN = 113,
	S_LIGHTDONE = 1,
	S_MBUL = 138,
	S_MEDI = 119,
	S_MGUN = 112,
	S_MISL = 117,
	S_MMIS = 137,
	S_MSHE = 139,
	S_NULL = 0,
	S_PLAYER = 41,
	S_POSS_ATK1 = 47,
	S_POSS_ATK2 = 48,
	S_POSS_ATK3 = 49,
	S_POSS_DIE1 = 51,
	S_POSS_DIE2 = 52,
	S_POSS_DIE3 = 53,
	S_POSS_DIE4 = 54,
	S_POSS_DIE5 = 55,
	S_POSS_PAIN = 50,
	S_POSS_RUN1 = 43,
	S_POSS_RUN2 = 44,
	S_POSS_RUN3 = 45,
	S_POSS_RUN4 = 46,
	S_POSS_STND = 42,
	S_POW1 = 127,
	S_POW2 = 128,
	S_POW3 = 129,
	S_POW4 = 130,
	S_POW5 = 131,
	S_RBAR = 134,
	S_RIFLASH1 = 16,
	S_RIFLASH2 = 17,
	S_RIFLE = 9,
	S_RIFLE1 = 10,
	S_RIFLE2 = 11,
	S_RIFLE3 = 12,
	S_RIFLE4 = 13,
	S_RIFLEDOWN = 14,
	S_RIFLEUP = 15,
	S_SARG_ATK1 = 74,
	S_SARG_ATK2 = 75,
	S_SARG_ATK3 = 76,
	S_SARG_DIE1 = 78,
	S_SARG_DIE2 = 79,
	S_SARG_DIE3 = 80,
	S_SARG_DIE4 = 81,
	S_SARG_DIE5 = 82,
	S_SARG_PAIN = 77,
	S_SARG_RUN1 = 70,
	S_SARG_RUN2 = 71,
	S_SARG_RUN3 = 72,
	S_SARG_RUN4 = 73,
	S_SARG_STND = 69,
	S_SGUN = 27,
	S_SGUN1 = 28,
	S_SGUN2 = 29,
	S_SGUN3 = 30,
	S_SGUN4 = 31,
	S_SGUN5 = 32,
	S_SGUN6 = 33,
	S_SGUN7 = 34,
	S_SGUN8 = 35,
	S_SGUN9 = 36,
	S_SGUNDOWN = 37,
	S_SGUNFLASH1 = 39,
	S_SGUNFLASH2 = 40,
	S_SGUNUP = 38,
	S_SHEL = 116,
	S_SHOT = 111,
	S_SKEY = 142,
	S_SOUL = 120,
	S_SPAR = 135,
	S_STIM = 118,
	S_TROO_ATK1 = 61,
	S_TROO_ATK2 = 62,
	S_TROO_ATK3 = 63,
	S_TROO_DIE1 = 65,
	S_TROO_DIE2 = 66,
	S_TROO_DIE3 = 67,
	S_TROO_DIE4 = 68,
	S_TROO_PAIN = 64,
	S_TROO_RUN1 = 57,
	S_TROO_RUN2 = 58,
	S_TROO_RUN3 = 59,
	S_TROO_RUN4 = 60,
	S_TROO_STND = 56
} statenum_t;

typedef struct
{
	int sprite;
	int frame;
	int tics;
	void (*action)();
	int nextstate;
	int misc1;
	int misc2;
} state_t;

extern char* sprnames[];
extern state_t states[];

typedef struct thinker_s
{
	struct thinker_s* prev;
	struct thinker_s* next;
	void* function;
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

extern actor_t actor;
extern actor_t* actorcap;

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

void P_RunAutoMap(char ch);

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
