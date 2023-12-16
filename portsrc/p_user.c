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
#include "w_wad.h"
#include "g_game.h"
#include "p_actor.h"

typedef enum
{
    ps_smoke,
    ps_flash,
    ps_weapon,
    ps_shell1,
    ps_shell2,
    ps_shell3,
    ps_shell4,
    ps_gore1,
    ps_gore2,
    ps_gore3,
    ps_gore4,
    NUMPSPRITES
} psprnum_t; //[ISB] shell casings, and on-screen gore effects? How times change...

typedef struct
{
    state_t* state;
    fixed_t sx;
    fixed_t sy;
    fixed_t speedx;
    fixed_t speedy;
    int tics;
} pspdef_t;

#define LOWERSPEED		FRACUNIT*6
#define RAISESPEED		FRACUNIT*6

#define WEAPONBOTTOM	144*FRACUNIT
#define WEAPONTOP		68*FRACUNIT

int playerobjfound[4];
player_t* player;
player_t playerobjs[4];

int debugmove;

fixed_t swingx, swingy;

pspdef_t psprites[MAXPLAYERS][NUMPSPRITES];

#define FRICTION 0xe800
#define STOPSPEED 0x3FF

#define MAXBOB 0x260000
#define VIEWHEIGHT (40<<FRACBITS)

fixed_t clipgap = 48 << FRACBITS;
fixed_t clipstepup = 24 << FRACBITS;

line_t* clipline;
int clipbox[4];

void P_BringUpWeapon(void);

void P_SetPsprite(psprnum_t position, statenum_t stnum)
{
    state_t* state;
    pspdef_t* psp;

    psp = &psprites[playernum][position];

    do
    {
        if (!stnum)
        {
            psp->state = NULL;
            break;		// object removed itself
        }

        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;  // could be 0
        if (state->misc1)
        {   // coordinate set
            psp->sx = state->misc1 << FRACBITS;
            psp->sy = state->misc2 << FRACBITS;
        }
        // call action routine
        if (state->action != NULL)
        {
            state->action(psp);
        }
        stnum = state->nextstate;
    } while (psp->tics == 0);	// an initial state of 0 could cycle through
}

void P_SetupPSprites(int pnum)
{
    int i;

    for (i = 0; i < NUMPSPRITES; i++)
    {
        psprites[pnum][i].state = NULL;
    }

    player = &playerobjs[pnum];
    playernum = pnum;
    playerobjs[pnum].pendingweapon = playerobjs[pnum].readyweapon;
    P_BringUpWeapon();
}

void P_CalcSwing(void)
{
    fixed_t	swing;
    int		angle;

    // OPTIMIZE: tablify this

    swing = player->bob;

    angle = (NUMANGLES / 70 * processedframe) & ANGLEMASK;
    swingx = FixedMul(swing, sines[angle]);

    angle = (NUMANGLES / 70 * processedframe + NUMANGLES / 2) & ANGLEMASK;
    swingy = -FixedMul(swingx, sines[angle]);
}

void P_DrawPlayerShapes(int pnum)
{
    int i;
    pspdef_t* psp;

    player = &playerobjs[pnum];
    playernum = pnum;
    P_CalcSwing();
    psp = &psprites[pnum];
    for (i = 0; i <= ps_weapon; i++)
    {
        if (psp->state != NULL)
            R_DrawPlayerShape(psp->state->sprite, psp->state->frame, psp->sx + swingx >> FRACBITS, psp->sy + swingy >> FRACBITS);
        
        psp++;
    }
    //BUG: This is done in two steps so that the unused layers wouldn't swing, but i isn't set to ps_shell1 initially
    //This causes the game to render the weapon psprite layers from the next player. 
    for (i = 0; i < NUMPSPRITES; i++)
    {
        if (psp->state != NULL)
            R_DrawPlayerShape(psp->state->sprite, psp->state->frame, psp->sx >> FRACBITS, psp->sy >> FRACBITS);
        
        psp++;
    }
}

void P_MovePlayerShapes(void)
{
    int i;
    pspdef_t* psp;
    state_t* state;

    for (playernum = 0; playernum < MAXPLAYERS; playernum++)
    {
        if (sd->playeringame[playernum] != 0)
        {
            player = &playerobjs[playernum];
            psp = &psprites[playernum * NUMPSPRITES];

            for (i = 0; i < NUMPSPRITES; i++, psp++)
            {
                if (state = psp->state)
                {
                    if (i != ps_weapon || player->health != 0)
                    {
                        if (psp->tics != -1)
                        {
                            psp->tics--;
                            if (!psp->tics)
                            {
                                P_SetPsprite(i, psp->state->nextstate);
                            }
                        }
                    }
                }
            }
        }
    }
}

void P_BringUpWeapon(void)
{
    statenum_t new;

    switch (player->pendingweapon)
    {
    case wp_knife:
        new = S_BAYONETUP;
        break;
    case wp_rifle:
        new = S_RIFLEUP;
        break;
    case wp_shotgun:
        new = S_SGUNUP;
        break;
    case wp_auto:
        new = S_AUTOUP;
        break;
    case wp_missile:
    case wp_chainsaw:
    case wp_claw:
    case wp_bfg:
        new = S_SGUNUP;
        break;
    default:
        IO_Error("A_ChangeWeapon: bad weapon number\n");
        break;
    }

    player->pendingweapon = wp_nochange;
    P_SetPsprite(ps_weapon, new);
    psprites[playernum][ps_weapon].sy = WEAPONBOTTOM;
}

void P_FireWeapon(void)
{
    statenum_t new;

    switch (player->readyweapon)
    {
    case wp_knife:
        new = S_BAYONET1;
        break;
    case wp_rifle:
        if (player->ammo[am_clip] < 1)
            return;
        
        new = S_RIFLE1;
        break;
    case wp_shotgun:
        if (player->ammo[am_shell] < 1)
            return;
        
        new = S_SGUN1;
        break;
    case wp_auto:
        if (player->ammo[am_clip] < 1)
            return;
        
        new = S_AUTO1;
        break;
    case wp_missile:
        if (player->ammo[am_misl] < 1)
            return;
        
        new = S_SGUN1;
        break;
    case wp_chainsaw:
        new = S_SGUN1;
        break;
    case wp_claw:
        if (player->ammo[am_soul] < 1)
            return;
        
        new = S_SGUN1;
        break;
    case wp_bfg:
        if (player->ammo[am_cell] < 1)
            return;
        
        new = S_SGUN1;
        break;
    default:
        IO_Error("A_FireWeapon: bad weapon number\n");
        break;
    }

    P_SetPsprite(ps_weapon, new);
}

void A_WeaponReady(pspdef_t* psp)
{
    statenum_t new;

    if ((player->pendingweapon == wp_nochange) && (player->health != 0))
    {
        if (player->firedown != 0)
        {
            P_FireWeapon();
        }
    }
    else
    {
        switch (player->readyweapon)
        {
        case wp_knife:
            new = S_BAYONETDOWN;
            break;
        case wp_rifle:
            new = S_RIFLEDOWN;
            break;
        case wp_shotgun:
            new = S_SGUNDOWN;
            break;
        case wp_auto:
            new = S_AUTODOWN;
            break;
        case wp_missile:
        case wp_chainsaw:
        case wp_claw:
        case wp_bfg:
            new = S_SGUNDOWN;
            break;
        default:
            IO_Error("A_ChangeWeapon: bad weapon number\n");
            break;
        }
        P_SetPsprite(ps_weapon, new);
    }
}

void A_Refire(pspdef_t* psp)
{
    if (((player->firedown != 0) && (player->pendingweapon == wp_nochange)) && (player->health != 0))
    {
        P_FireWeapon();
    }
}

void A_Lower(pspdef_t* psp)
{
    psp->sy += LOWERSPEED;

    if (psp->sy < WEAPONBOTTOM)
        return;

    if (player->health == 0)
    {
        P_SetPsprite(ps_weapon, S_NULL);
    }
    else
    {
        player->readyweapon = player->pendingweapon;
        if (playernum == sd->consoleplayer)
        {
            P_DrawWeapon();
            P_DrawAmmo();
        }
        P_BringUpWeapon();
    }
}

void A_Raise(pspdef_t* psp)
{
    statenum_t new;

    psp->sy = psp->sy - RAISESPEED;
    if (psp->sy > WEAPONTOP)
        return;

    psp->sy = WEAPONTOP;

    switch (player->readyweapon)
    {
    case wp_knife:
        new = S_BAYONET;
        break;
    case wp_rifle:
        new = S_RIFLE;
        break;
    case wp_shotgun:
        new = S_SGUN;
        break;
    case wp_auto:
        new = S_AUTO;
        break;
    case wp_missile:
    case wp_chainsaw:
    case wp_claw:
    case wp_bfg:
        new = S_SGUN;
        break;
    default:
        IO_Error("A_ChangeWeapon: bad weapon number\n");
        break;
    }

    P_SetPsprite(ps_weapon, new);
}

void A_FireGun(pspdef_t* psp)
{
    statenum_t new;

    switch (player->readyweapon)
    {
    case wp_knife:
    case wp_chainsaw:
        new = S_NULL;
        break;
    case wp_rifle:
        player->ammo[am_clip]--;
        P_PlayerShoot();
        new = S_RIFLASH1;
        break;
    case wp_shotgun:
        player->ammo[am_shell]--;
        P_PlayerShoot();
        new = S_SGUNFLASH1;
        break;
    case wp_auto:
        player->ammo[am_clip]--;
        P_PlayerShoot();
        new = S_AUTOFLASH1;
        break;
    case wp_missile:
        player->ammo[am_misl]--;
        new = S_SGUNFLASH1;
        break;
    case wp_claw:
        player->ammo[am_soul]--;
        new = S_SGUNFLASH1;
        break;
    case wp_bfg:
        player->ammo[am_cell]--;
        new = S_SGUNFLASH1;
        break;
    default:
        IO_Error("A_FireGun: bad weapon number");
        break;
    }

    if (playernum == sd->consoleplayer)
        P_DrawAmmo();
    
    P_SetPsprite(ps_flash, new);
}

void A_Light0(pspdef_t* psp)
{
    extralight = 0;
}

void A_Light1(pspdef_t* psp)
{
    extralight = 1;
}

void A_Light2(pspdef_t* psp)
{
    extralight = 2;
}

void P_UseFrontLines(sector_t* sector)
{
    int i, side;
    line_t* line;
    vertex_t vv1, vv2;
    fixed_t frac, midz;

    if (sector->validcheck == validcheck)
        return;

    sector->validcheck = validcheck;

    for (i = 0; i < sector->linecount; i++)
    {
        line = &lines[sector->lines[i]];
        if (line->flags & ML_TWOSIDED || line->special)
        {
            R_TransformVertex(&points[line->p1], &vv1);
            R_TransformVertex(&points[line->p2], &vv2);

            //find which side is facing the view,
            //and the fraction at which the ray down the center hit. 
            if (vv1.tx <= 0 && vv2.tx > 0)
            {
                frac = FixedDiv(-vv1.tx, vv2.tx - vv1.tx);
                side = 0;
            }
            else if (vv1.tx > 0 && vv2.tx <= 0)
            {
                if (!(line->flags & ML_TWOSIDED))
                    continue;

                frac = FixedDiv(vv1.tx, vv1.tx - vv2.tx);
                side = 1;
            }
            else
                continue; //line doesn't cross center of screen

            midz = vv1.tz + FixedMul(vv2.tz - vv1.tz, frac);

            if (midz > USERANGE || midz < 0)
                continue;

            if (!line->special)
                P_UseFrontLines(&sectors[sides[line->side[side ^ 1]].sector]);
            else
                P_PlayerUseSpecialLine(line, side);
        }
    }
}

void P_Use(void)
{
    sector_t* sector = &sectors[player->r->sector];

    viewx = player->r->x;
    viewy = player->r->y;
    viewsin = sines[player->r->angle];
    viewcos = cosines[player->r->angle];
    validcheck++;

    P_UseFrontLines(sector);
}

void P_Thrust(fixed_t angle, fixed_t speed)
{
    angle &= ANGLEMASK;
    if (debugmove == 0)
    {
        player->momx += FixedMul(speed, cosines[angle]);
        player->momy += FixedMul(speed, sines[angle]);
    }
    else
    {
        player->momx = FixedMul(speed << 3, cosines[angle]);
        player->momy = FixedMul(speed << 3, sines[angle]);
    }
}

void P_ChangeWeapon(weapontype_t newweapon)
{
    if (newweapon <= wp_bfg && player->weaponowned[newweapon] != 0)
    {
        player->pendingweapon = newweapon;
    }
}

void P_SlowPlayer(void)
{
    if (debugmove == 0)
    {
        if ((player->momx < -STOPSPEED) || (STOPSPEED < player->momx))
        {
            player->momx = FixedMul(player->momx, FRICTION);
        }
        else
        {
            player->momx = 0;
        }
        if ((player->momy < -STOPSPEED) || (STOPSPEED < player->momy))
        {
            player->momy = FixedMul(player->momy, FRICTION);
        }
        else
        {
            player->momy = 0;
        }
    }
    else
    {
        player->momx = player->momy = 0;
    }
}

int P_CheckSolidLines(int sectornum)
{
    point_t v1, v2;
    sector_t* sector;
    fixed_t wallintercept, clipintercept;
    int i;

    sector = &sectors[sectornum];
    if (sector->validcheck == validcheck)
    {
        return 1;
    }

    sector->validcheck = validcheck;

    for (i = 0; i < sector->linecount; i++)
    {
        clipline = &lines[sector->lines[i]];
        if ((((clipline->bbox[1] <= clipbox[3]) && (clipbox[1] <= clipline->bbox[3])) &&
            (clipbox[0] <= clipline->bbox[2])) && (clipline->bbox[0] <= clipbox[2]))
        {
            if (clipline->slopetype == ls_slope)
            {
                v1 = points[clipline->p1];
                v2 = points[clipline->p2];

                wallintercept = clipline->yintercept;
                clipintercept = wallintercept + FixedMul(clipline->slope, clipbox[1]);
                if (clipintercept < clipbox[0] || clipintercept > clipbox[2])
                {
                    clipintercept = wallintercept + FixedMul(clipline->slope, clipbox[3]);
                    if (clipintercept < clipbox[0] || clipintercept > clipbox[2])
                    {
                        clipintercept = FixedDiv(clipbox[0] - wallintercept, clipline->slope);
                        if (clipintercept < clipbox[1] || clipintercept > clipbox[3])
                        {
                            clipintercept = FixedDiv(clipbox[2] - wallintercept, clipline->slope);
                            if (clipintercept < clipbox[1] || clipintercept > clipbox[3])
                                continue;
                        }
                    }
                }
            }

            if (clipline->flags & ML_BLOCKING)
                return 0;

            if (clipline->flags & ML_TWOSIDED)
            {
                short sec;
                fixed_t gap, step;

                if (sides[clipline->side[0]].sector == sectornum)
                    sec = sides[clipline->side[1]].sector;
                else
                    sec = sides[clipline->side[0]].sector;

                gap = sectors[sec].ceilingheight - sector->floorheight;
                if (gap < clipgap)
                    return 0;

                step = sectors[sec].floorheight - sector->floorheight;
                if (step > clipstepup)
                    return 0;

                if (!P_CheckSolidLines(sec))
                    return 0;
            }
        }
    }

    return 1;
}

int P_CheckPosition(fixed_t ox, fixed_t oy, fixed_t size, int basesector)
{
    fixed_t leftblock, rightblock, topblock, bottomblock;
    byte* blmap;
    fixed_t x, y;

    clipbox[1] = ox - size;
    clipbox[3] = ox + size;
    clipbox[2] = oy + size;
    clipbox[0] = oy - size;

    blmap = P_BlockOrg(ox, oy);

    if ((*blmap & BMF_GETTABLE) != 0)
    {
        P_GetThingAt(blmap);
    }
    if ((blmap[1] & BMF_GETTABLE) != 0)
    {
        P_GetThingAt(blmap + 1);
    }
    if ((blmap[mapwidth] & BMF_GETTABLE) != 0)
    {
        P_GetThingAt(blmap + mapwidth);
    }
    if ((blmap[mapwidth + 1] & BMF_GETTABLE) != 0)
    {
        P_GetThingAt(blmap + mapwidth + 1);
    }

    if (((((blmap[0] & BMF_SOLID) == 0) && ((blmap[1] & BMF_SOLID) == 0)) && ((blmap[mapwidth] & BMF_SOLID) == 0)) && ((blmap[mapwidth + 1] & BMF_SOLID) == 0))
    {
        rightblock = clipbox[0] - maporiginy >> MAPBLOCKSHIFT;
        leftblock = clipbox[2] - maporiginy >> MAPBLOCKSHIFT;
        x = rightblock;
        while (x <= leftblock)
        {
            topblock = clipbox[1] - maporiginx >> MAPBLOCKSHIFT;
            bottomblock = clipbox[3] - maporiginx >> MAPBLOCKSHIFT;
            y = topblock;
            while (y <= bottomblock)
            {
                if (((blockmap[x * mapwidth + y]) & BMF_CHECKLINES) != 0)
                {
                    validcheck++;
                    return P_CheckSolidLines(basesector);
                }
                y++;
            }
            x++;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

void P_ClipPlayerMove(void)
{
    fixed_t tryx;
    fixed_t tryy;

    tryx = player->r->x + player->momx;
    tryy = player->r->y + player->momy;

    if (P_CheckPosition(tryx, tryy, PLAYERRADIUS, player->r->sector) == 0)
    {
        tryx = player->r->x + player->momx;
        tryy = player->r->y;
        if (P_CheckPosition(tryx, tryy, PLAYERRADIUS, player->r->sector) == 0)
        {
            tryx = player->r->x;
            tryy = player->r->y + player->momy;
            if (P_CheckPosition(tryx, tryy, PLAYERRADIUS, player->r->sector) == 0)
            {
                player->momx = player->momy = 0;
                return;
            }
        }
    }

    player->r->sector = P_CrossSectorBounds(player->r->sector, player->r->x, player->r->y, tryx, tryy);
    player->r->x = tryx;
    player->r->y = tryy;
}

void P_MovePlayer(void)
{
    if ((player->momx != 0) || (player->momy != 0))
    {
        P_RemoveBlockMarks(player->r);
        P_ClipPlayerMove();
        P_PlaceBlockMarks(player->r);
    }
}

void P_CalcBob(void)
{
    int angle;
    fixed_t bob;

    if (debugmove)
    {
        player->viewz = player->r->z;
        player->bob = 0;
    }
    else
    {
        player->r->z = sectors[player->r->sector].floorheight;
        player->bob = FixedMul(player->momx, player->momx) + FixedMul(player->momy, player->momy);
        player->bob >>= 1;
        if (player->bob > MAXBOB)
            player->bob = MAXBOB;

       angle = processedframe * (NUMANGLES / 70) & (NUMANGLES / 2 - 1);
       bob = FixedMul(player->bob / 2, sines[angle]);
       player->viewz = player->r->z + VIEWHEIGHT + bob;
    }
}

void P_DeadThink(framecmd_t* cmd)
{
    fixed_t delta;

    P_MovePlayer();
    P_SlowPlayer();
    delta = player->viewz - player->r->z;
    if (delta <= 20 * FRACUNIT)
    {
        if (delta <= 8 * FRACUNIT)
        {
            if ((cmd->buttons & BT_ATTACK) != 0)
            {
                gameaction = ga_died;
            }
        }
        else
        {
            player->viewz = player->viewz - FRACUNIT / 4;
        }
    }
    else
    {
        player->viewz = player->viewz - FRACUNIT;
    }
}

void P_PlayerGameThink(framecmd_t* cmd)
{
    weapontype_t weaponchange;

    if (player->health == 0)
    {
        P_DeadThink(cmd);
    }
    else
    {
        if (sectors[player->r->sector].special != 0)
        {
            P_PlayerInSpecialSector();
        }
        if ((player->strafedown == 0) && ((cmd->buttons & BT_USE) != 0))
        {
            P_Use();
        }
        player->strafedown = (cmd->buttons & BT_USE);
        weaponchange = (cmd->buttons >> BT_WEAPONSHIFT) & BT_WEAPONMASK;
        if (weaponchange != wp_nochange)
        {
            P_ChangeWeapon(weaponchange);
        }
        player->firedown = (cmd->buttons & BT_ATTACK);

        if ((cmd->buttons & BT_USE) == 0)
        {
            player->r->angle = (player->r->angle - FixedMul(cmd->xmove, FRACUNIT * 2)) & ANGLEMASK;
        }
        else if (cmd->xmove != 0)
        {
            P_Thrust(player->r->angle - (NUMANGLES / 4), cmd->xmove * 640);
        }

        if (cmd->ymove != 0)
        {
            P_Thrust(player->r->angle, cmd->ymove * 640);
        }
        P_MovePlayer();
        P_SlowPlayer();
        P_CalcBob();
    }
}
