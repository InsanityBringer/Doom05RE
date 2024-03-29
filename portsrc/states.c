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
// generated by statescr

#include "states.h"
#include <stdlib.h>

char* sprnames[NUMSPRITES] = {
"TROO", "PSHO", "PBAY", "PRIF", "PMAC", "PLAY", "POSS", "SARG", "HEAD", "BOSS",
"SHOT", "MGUN", "LAUN", "CSAW", "CLIP", "SHEL", "MISL", "STIM", "MEDI", "SOUL", 
"BON1", "BON2", "BON3", "BON4", "ARM1", "ARM2", "POW1", "POW2", "POW3", "POW4", 
"POW5", "GBAR", "COLU", "RBAR", "SPAR", "IGOG", "MMIS", "MBUL", "MSHE", "ELEC", 
"GKEY", "SKEY", "BKEY"
};

void A_Light0();
void A_WeaponReady();
void A_FireGun();
void A_Refire();
void A_Lower();
void A_Raise();
void A_Light1();
void A_Light2();
void A_Look();
void A_Chase();
void A_Attack();
void A_Scream();

state_t states[NUMSTATES] = {
{SPR_TROO,0,-1,NULL,S_NULL,0,0}, //S_NULL
{SPR_PSHO,4,0,A_Light0,S_NULL,0,0}, //S_LIGHTDONE
{SPR_PBAY,0,1,A_WeaponReady,S_BAYONET,16,68}, //S_BAYONET
{SPR_PBAY,1,7,NULL,S_BAYONET2,16,36}, //S_BAYONET1
{SPR_PBAY,2,7,A_FireGun,S_BAYONET3,0,0}, //S_BAYONET2
{SPR_PBAY,1,7,NULL,S_BAYONET4,0,0}, //S_BAYONET3
{SPR_PBAY,1,7,A_Refire,S_BAYONET,0,0}, //S_BAYONET4
{SPR_PBAY,0,1,A_Lower,S_BAYONETDOWN,0,0}, //S_BAYONETDOWN
{SPR_PBAY,0,1,A_Raise,S_BAYONETUP,0,0}, //S_BAYONETUP
{SPR_PRIF,0,1,A_WeaponReady,S_RIFLE,16,68}, //S_RIFLE
{SPR_PRIF,0,5,NULL,S_RIFLE2,16,36}, //S_RIFLE1
{SPR_PRIF,0,8,A_FireGun,S_RIFLE3,0,0}, //S_RIFLE2
{SPR_PRIF,0,7,NULL,S_RIFLE4,0,0}, //S_RIFLE3
{SPR_PRIF,0,7,A_Refire,S_RIFLE,0,0}, //S_RIFLE4
{SPR_PRIF,0,1,A_Lower,S_RIFLEDOWN,0,0}, //S_RIFLEDOWN
{SPR_PRIF,0,1,A_Raise,S_RIFLEUP,0,0}, //S_RIFLEUP
{SPR_PSHO,4,4,A_Light1,S_RIFLASH2,128,99}, //S_RIFLASH1
{SPR_PSHO,5,4,A_Light2,S_LIGHTDONE,121,90}, //S_RIFLASH2
{SPR_PMAC,0,1,A_WeaponReady,S_AUTO,16,68}, //S_AUTO
{SPR_PMAC,0,5,NULL,S_AUTO2,16,36}, //S_AUTO1
{SPR_PMAC,0,8,A_FireGun,S_AUTO3,0,0}, //S_AUTO2
{SPR_PMAC,0,7,NULL,S_AUTO4,0,0}, //S_AUTO3
{SPR_PMAC,0,7,A_Refire,S_AUTO,0,0}, //S_AUTO4
{SPR_PMAC,0,1,A_Lower,S_AUTODOWN,0,0}, //S_AUTODOWN
{SPR_PMAC,0,1,A_Raise,S_AUTOUP,0,0}, //S_AUTOUP
{SPR_PSHO,4,4,A_Light1,S_AUTOFLASH2,128,99}, //S_AUTOFLASH1
{SPR_PSHO,5,4,A_Light2,S_LIGHTDONE,121,90}, //S_AUTOFLASH2
{SPR_PSHO,0,1,A_WeaponReady,S_SGUN,16,68}, //S_SGUN
{SPR_PSHO,0,3,NULL,S_SGUN2,16,36}, //S_SGUN1
{SPR_PSHO,0,8,A_FireGun,S_SGUN3,0,0}, //S_SGUN2
{SPR_PSHO,1,7,NULL,S_SGUN4,0,0}, //S_SGUN3
{SPR_PSHO,2,7,NULL,S_SGUN5,0,0}, //S_SGUN4
{SPR_PSHO,3,7,NULL,S_SGUN6,0,0}, //S_SGUN5
{SPR_PSHO,2,7,NULL,S_SGUN7,0,0}, //S_SGUN6
{SPR_PSHO,1,7,NULL,S_SGUN8,0,0}, //S_SGUN7
{SPR_PSHO,0,3,NULL,S_SGUN9,0,0}, //S_SGUN8
{SPR_PSHO,0,7,A_Refire,S_SGUN,0,0}, //S_SGUN9
{SPR_PSHO,0,1,A_Lower,S_SGUNDOWN,0,0}, //S_SGUNDOWN
{SPR_PSHO,0,1,A_Raise,S_SGUNUP,0,0}, //S_SGUNUP
{SPR_PSHO,4,4,A_Light1,S_SGUNFLASH2,128,99}, //S_SGUNFLASH1
{SPR_PSHO,5,4,A_Light2,S_LIGHTDONE,121,90}, //S_SGUNFLASH2
{SPR_PLAY,0,-1,NULL,S_NULL,0,0}, //S_PLAYER
{SPR_POSS,0,10,A_Look,S_POSS_STND,0,0}, //S_POSS_STND
{SPR_POSS,0,10,A_Chase,S_POSS_RUN2,0,0}, //S_POSS_RUN1
{SPR_POSS,1,10,A_Chase,S_POSS_RUN3,0,0}, //S_POSS_RUN2
{SPR_POSS,2,10,A_Chase,S_POSS_RUN4,0,0}, //S_POSS_RUN3
{SPR_POSS,3,10,A_Chase,S_POSS_RUN1,0,0}, //S_POSS_RUN4
{SPR_POSS,4,10,NULL,S_POSS_ATK2,0,0}, //S_POSS_ATK1
{SPR_POSS,5,10,A_Attack,S_POSS_RUN1,0,0}, //S_POSS_ATK2
{SPR_POSS,4,10,NULL,S_POSS_RUN1,0,0}, //S_POSS_ATK3
{SPR_POSS,6,10,NULL,S_POSS_RUN1,0,0}, //S_POSS_PAIN
{SPR_POSS,7,10,A_Scream,S_POSS_DIE2,0,0}, //S_POSS_DIE1
{SPR_POSS,8,10,NULL,S_POSS_DIE3,0,0}, //S_POSS_DIE2
{SPR_POSS,9,10,NULL,S_POSS_DIE4,0,0}, //S_POSS_DIE3
{SPR_POSS,10,10,NULL,S_POSS_DIE5,0,0}, //S_POSS_DIE4
{SPR_POSS,11,-1,NULL,S_NULL,0,0}, //S_POSS_DIE5
{SPR_TROO,0,10,A_Look,S_TROO_STND,0,0}, //S_TROO_STND
{SPR_TROO,0,10,A_Chase,S_TROO_RUN2,0,0}, //S_TROO_RUN1
{SPR_TROO,1,10,A_Chase,S_TROO_RUN3,0,0}, //S_TROO_RUN2
{SPR_TROO,2,10,A_Chase,S_TROO_RUN4,0,0}, //S_TROO_RUN3
{SPR_TROO,3,10,A_Chase,S_TROO_RUN1,0,0}, //S_TROO_RUN4
{SPR_TROO,4,10,NULL,S_TROO_ATK2,0,0}, //S_TROO_ATK1
{SPR_TROO,5,10,NULL,S_TROO_ATK3,0,0}, //S_TROO_ATK2
{SPR_TROO,6,10,A_Attack,S_TROO_RUN1,0,0}, //S_TROO_ATK3
{SPR_TROO,7,10,NULL,S_TROO_RUN1,0,0}, //S_TROO_PAIN
{SPR_TROO,8,10,A_Scream,S_TROO_DIE2,0,0}, //S_TROO_DIE1
{SPR_TROO,9,10,NULL,S_TROO_DIE3,0,0}, //S_TROO_DIE2
{SPR_TROO,10,10,NULL,S_TROO_DIE4,0,0}, //S_TROO_DIE3
{SPR_TROO,11,-1,NULL,S_NULL,0,0}, //S_TROO_DIE4
{SPR_SARG,0,10,A_Look,S_SARG_STND,0,0}, //S_SARG_STND
{SPR_SARG,0,10,A_Chase,S_SARG_RUN2,0,0}, //S_SARG_RUN1
{SPR_SARG,1,10,A_Chase,S_SARG_RUN3,0,0}, //S_SARG_RUN2
{SPR_SARG,2,10,A_Chase,S_SARG_RUN4,0,0}, //S_SARG_RUN3
{SPR_SARG,3,10,A_Chase,S_SARG_RUN1,0,0}, //S_SARG_RUN4
{SPR_SARG,4,10,NULL,S_SARG_ATK2,0,0}, //S_SARG_ATK1
{SPR_SARG,5,10,NULL,S_SARG_ATK3,0,0}, //S_SARG_ATK2
{SPR_SARG,6,10,A_Attack,S_SARG_RUN1,0,0}, //S_SARG_ATK3
{SPR_SARG,7,10,NULL,S_SARG_RUN1,0,0}, //S_SARG_PAIN
{SPR_SARG,8,10,A_Scream,S_SARG_DIE2,0,0}, //S_SARG_DIE1
{SPR_SARG,9,10,NULL,S_SARG_DIE3,0,0}, //S_SARG_DIE2
{SPR_SARG,10,10,NULL,S_SARG_DIE4,0,0}, //S_SARG_DIE3
{SPR_SARG,11,10,NULL,S_SARG_DIE5,0,0}, //S_SARG_DIE4
{SPR_SARG,12,-1,NULL,S_NULL,0,0}, //S_SARG_DIE5
{SPR_HEAD,0,10,A_Look,S_HEAD_STND,0,0}, //S_HEAD_STND
{SPR_HEAD,0,10,A_Chase,S_HEAD_RUN2,0,0}, //S_HEAD_RUN1
{SPR_HEAD,1,10,A_Chase,S_HEAD_RUN3,0,0}, //S_HEAD_RUN2
{SPR_HEAD,2,10,A_Chase,S_HEAD_RUN4,0,0}, //S_HEAD_RUN3
{SPR_HEAD,3,10,A_Chase,S_HEAD_RUN1,0,0}, //S_HEAD_RUN4
{SPR_HEAD,4,10,NULL,S_HEAD_ATK2,0,0}, //S_HEAD_ATK1
{SPR_HEAD,5,10,NULL,S_HEAD_ATK3,0,0}, //S_HEAD_ATK2
{SPR_HEAD,6,10,A_Attack,S_HEAD_RUN1,0,0}, //S_HEAD_ATK3
{SPR_HEAD,7,10,NULL,S_HEAD_RUN1,0,0}, //S_HEAD_PAIN
{SPR_HEAD,8,10,A_Scream,S_HEAD_DIE2,0,0}, //S_HEAD_DIE1
{SPR_HEAD,9,10,NULL,S_HEAD_DIE3,0,0}, //S_HEAD_DIE2
{SPR_HEAD,10,10,NULL,S_HEAD_DIE4,0,0}, //S_HEAD_DIE3
{SPR_HEAD,11,10,NULL,S_HEAD_DIE5,0,0}, //S_HEAD_DIE4
{SPR_HEAD,12,-1,NULL,S_NULL,0,0}, //S_HEAD_DIE5
{SPR_BOSS,0,10,A_Look,S_BOSS_STND,0,0}, //S_BOSS_STND
{SPR_BOSS,0,10,A_Chase,S_BOSS_RUN2,0,0}, //S_BOSS_RUN1
{SPR_BOSS,1,10,A_Chase,S_BOSS_RUN3,0,0}, //S_BOSS_RUN2
{SPR_BOSS,2,10,A_Chase,S_BOSS_RUN4,0,0}, //S_BOSS_RUN3
{SPR_BOSS,3,10,A_Chase,S_BOSS_RUN1,0,0}, //S_BOSS_RUN4
{SPR_BOSS,4,10,NULL,S_BOSS_ATK2,0,0}, //S_BOSS_ATK1
{SPR_BOSS,5,10,NULL,S_BOSS_ATK3,0,0}, //S_BOSS_ATK2
{SPR_BOSS,6,10,A_Attack,S_BOSS_RUN1,0,0}, //S_BOSS_ATK3
{SPR_BOSS,7,10,NULL,S_BOSS_RUN1,0,0}, //S_BOSS_PAIN
{SPR_BOSS,8,10,A_Scream,S_BOSS_DIE2,0,0}, //S_BOSS_DIE1
{SPR_BOSS,9,10,NULL,S_BOSS_DIE3,0,0}, //S_BOSS_DIE2
{SPR_BOSS,10,10,NULL,S_BOSS_DIE4,0,0}, //S_BOSS_DIE3
{SPR_BOSS,11,10,NULL,S_BOSS_DIE5,0,0}, //S_BOSS_DIE4
{SPR_BOSS,12,-1,NULL,S_NULL,0,0}, //S_BOSS_DIE5
{SPR_SHOT,0,-1,NULL,S_NULL,0,0}, //S_SHOT
{SPR_MGUN,0,-1,NULL,S_NULL,0,0}, //S_MGUN
{SPR_LAUN,0,-1,NULL,S_NULL,0,0}, //S_LAUN
{SPR_CSAW,0,-1,NULL,S_NULL,0,0}, //S_CSAW
{SPR_CLIP,0,-1,NULL,S_NULL,0,0}, //S_CLIP
{SPR_SHEL,0,-1,NULL,S_NULL,0,0}, //S_SHEL
{SPR_MISL,0,-1,NULL,S_NULL,0,0}, //S_MISL
{SPR_STIM,0,-1,NULL,S_NULL,0,0}, //S_STIM
{SPR_MEDI,0,-1,NULL,S_NULL,0,0}, //S_MEDI
{SPR_SOUL,0,-1,NULL,S_NULL,0,0}, //S_SOUL
{SPR_BON1,0,-1,NULL,S_NULL,0,0}, //S_BON1
{SPR_BON2,0,-1,NULL,S_NULL,0,0}, //S_BON2
{SPR_BON3,0,-1,NULL,S_NULL,0,0}, //S_BON3
{SPR_BON4,0,-1,NULL,S_NULL,0,0}, //S_BON4
{SPR_ARM1,0,-1,NULL,S_NULL,0,0}, //S_ARM1
{SPR_ARM2,0,-1,NULL,S_NULL,0,0}, //S_ARM2
{SPR_POW1,0,-1,NULL,S_NULL,0,0}, //S_POW1
{SPR_POW2,0,-1,NULL,S_NULL,0,0}, //S_POW2
{SPR_POW3,0,-1,NULL,S_NULL,0,0}, //S_POW3
{SPR_POW4,0,-1,NULL,S_NULL,0,0}, //S_POW4
{SPR_POW5,0,-1,NULL,S_NULL,0,0}, //S_POW5
{SPR_GBAR,0,-1,NULL,S_NULL,0,0}, //S_GBAR
{SPR_COLU,0,-1,NULL,S_NULL,0,0}, //S_COLU
{SPR_RBAR,0,-1,NULL,S_NULL,0,0}, //S_RBAR
{SPR_SPAR,0,-1,NULL,S_NULL,0,0}, //S_SPAR
{SPR_IGOG,0,-1,NULL,S_NULL,0,0}, //S_IGOG
{SPR_MMIS,0,-1,NULL,S_NULL,0,0}, //S_MMIS
{SPR_MBUL,0,-1,NULL,S_NULL,0,0}, //S_MBUL
{SPR_MSHE,0,-1,NULL,S_NULL,0,0}, //S_MSHE
{SPR_ELEC,0,-1,NULL,S_NULL,0,0}, //S_ELEC
{SPR_GKEY,0,-1,NULL,S_NULL,0,0}, //S_GKEY
{SPR_SKEY,0,-1,NULL,S_NULL,0,0}, //S_SKEY
{SPR_BKEY,0,-1,NULL,S_NULL,0,0}, //S_BKEY
};
