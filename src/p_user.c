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
#include "p_local.h"

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

pspdef_t psprites[MAXPLAYERS * NUMPSPRITES];

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

    psp = &psprites[playernum * NUMPSPRITES + position];
    do 
    {
        if (stnum == 0) 
        {
            psp->state = (state_t*)NULL;
            return;
        }
        state = &states[stnum];
        psp->state = state;
        psp->tics = states[stnum].tics;
        if (states[stnum].misc1 != 0)
        {
            psp->sx = states[stnum].misc1 << FRACBITS;
            psp->sy = states[stnum].misc2 << FRACBITS;
        }
        if (states[stnum].action != NULL)
        {
            ((void(*)())states[stnum].action)(psp);
        }
        stnum = state->nextstate;
    } while (psp->tics == 0);
    return;
}

void P_SetupPSprites(int pnum)
{
    int i;

    for (i = 0; i < NUMPSPRITES; i++) 
    {
        psprites[pnum * NUMPSPRITES + i].state = NULL;
    }

    player = &playerobjs[pnum];
    playernum = pnum;
    playerobjs[pnum].pendingweapon = playerobjs[pnum].readyweapon;
    P_BringUpWeapon();
    return;
}

void P_CalcSwing(void)
{
    //This is basically the same as the unused P_CalcSwing function in the release source. 
    swingx = FixedMul(player->bob, sines[processedframe * 117 & 0x1fff]);
    swingy = -FixedMul(swingx, sines[processedframe * 117 + 4096 & 0x1fff]);

    return;
}

void P_DrawPlayerShapes(int pnum)
{
    pspdef_t* psp;
    int i;

    player = &playerobjs[pnum];
    playernum = pnum;
    i = pnum;
    P_CalcSwing();
    psp = &psprites[i * NUMPSPRITES];
    for (i = 0; i < 3; i++) 
    {
        if (psp->state != NULL)
        {
            R_DrawPlayerShape(psp->state->sprite, psp->state->frame, psp->sx + swingx >> FRACBITS, psp->sy + swingy >> FRACBITS);
        }
        psp++;
    }
    for (i = 0; i < NUMPSPRITES; i++)
    {
        if (psp->state != NULL) 
        {
            R_DrawPlayerShape(psp->state->sprite, psp->state->frame, psp->sx >> FRACBITS, psp->sy >> FRACBITS);
        }
        psp++;
    }
    return;
}

void P_MovePlayerShapes(void)
{
    state_t* state;
    pspdef_t* psp;
    int i;

    playernum = 0;
    while (playernum < MAXPLAYERS)
    {
        if (sd->playeringame[playernum] != 0) 
        {
            player = &playerobjs[playernum];
            psp = &psprites[playernum * NUMPSPRITES];
            
            for (i = 0; i < NUMPSPRITES; i++, psp++)
            {
                if (((psp->state != 0) && (((i != ps_weapon || (player->health != 0)) && (psp->tics != -1)))))
                {
                    psp->tics--;
                    if (!psp->tics)
                    {
                        P_SetPsprite(i, psp->state->nextstate);
                    }
                }
            }
        }
        playernum++;
    }
    return;
}

void P_BringUpWeapon(void)
{
    statenum_t newpsp;

    if (player->pendingweapon < 8)
    {
        switch (player->pendingweapon)
        {
        case wp_knife:
            newpsp = S_BAYONETUP;
            break;
        case wp_rifle:
            newpsp = S_RIFLEUP;
            break;
        case wp_shotgun:
            newpsp = S_SGUNUP;
            break;
        case wp_auto:
            newpsp = S_AUTOUP;
            break;
        default:
            newpsp = S_SGUNUP;
            break;
        }
    }
    else
    {
        IO_Error("A_ChangeWeapon: bad weapon number\n");
    }
    player->pendingweapon = wp_nochange;
    P_SetPsprite(ps_weapon, newpsp);
    psprites[playernum * NUMPSPRITES + 2].sy = WEAPONBOTTOM;
    return;
}

void P_FireWeapon(void)
{
    statenum_t newpsp;

    if (player->readyweapon < 8) 
    {
        switch (player->readyweapon) 
        {
        case wp_knife:
            newpsp = S_BAYONET1;
            break;
        case wp_rifle:
            if (player->ammo[am_clip] < 1) 
            {
                return;
            }
            newpsp = S_RIFLE1;
            break;
        case wp_shotgun:
            if (player->ammo[am_shell] < 1) 
            {
                return;
            }
            newpsp = S_SGUN1;
            break;
        case wp_auto:
            if (player->ammo[am_clip] < 1) 
            {
                return;
            }
            newpsp = S_AUTO1;
            break;
        case wp_missile:
            if (player->ammo[am_misl] < 1) 
            {
                return;
            }
            newpsp = S_SGUN1;
            break;
        case wp_chainsaw:
            newpsp = S_SGUN1;
            break;
        case wp_claw:
            if (player->ammo[am_soul] < 1) 
            {
                return;
            }
            newpsp = S_SGUN1;
            break;
        case wp_bfg:
            if (player->ammo[am_cell] < 1) 
            {
                return;
            }
            newpsp = S_SGUN1;
        }
    }
    else 
    {
        IO_Error("A_FireWeapon: bad weapon number\n");
    }
    P_SetPsprite(ps_weapon, newpsp);
    return;
}

void A_WeaponReady(pspdef_t* psp)
{
    statenum_t newpsp;

    if ((player->pendingweapon == wp_nochange) && (player->health != 0)) 
    {
        if (player->firedown != 0)
        {
            P_FireWeapon();
        }
    }
    else
    {
        if (player->readyweapon < 8) 
        {
            switch (player->readyweapon) 
            {
            case wp_knife:
                newpsp = S_BAYONETDOWN;
                break;
            case wp_rifle:
                newpsp = S_RIFLEDOWN;
                break;
            case wp_shotgun:
                newpsp = S_SGUNDOWN;
                break;
            case wp_auto:
                newpsp = S_AUTODOWN;
                break;
            default:
                newpsp = S_SGUNDOWN;
            }
        }
        else 
        {
            IO_Error("A_ChangeWeapon: bad weapon number\n");
        }
        P_SetPsprite(ps_weapon, newpsp);
    }
    return;
}

void A_Refire(pspdef_t *psp)
{
    if (((player->firedown != 0) && (player->pendingweapon == wp_nochange)) && (player->health != 0))
    {
        P_FireWeapon();
    }
    return;
}

void A_Lower(pspdef_t* psp)
{
    short* psVar1;

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
    statenum_t newpsp;

    psp->sy = psp->sy - RAISESPEED;
    if (psp->sy > WEAPONTOP)
        return;

    psp->sy = WEAPONTOP;

    if (player->readyweapon < 8)
    {
        switch (player->readyweapon)
        {
        case wp_knife:
            newpsp = S_BAYONET;
            break;
        case wp_rifle:
            newpsp = S_RIFLE;
            break;
        case wp_shotgun:
            newpsp = S_SGUN;
            break;
        case wp_auto:
            newpsp = S_AUTO;
            break;
        default:
            newpsp = S_SGUN;
        }
    }
    else 
    {
        IO_Error("A_ChangeWeapon: bad weapon number\n");
    }
    P_SetPsprite(ps_weapon, newpsp);
}

void A_FireGun(pspdef_t* psp)
{
    statenum_t newpsp;

    if (player->readyweapon < 8)
    {
        switch (player->readyweapon)
        {
        default:
            newpsp = S_NULL;
            break;
        case wp_rifle:
            player->ammo[am_clip]--;
            P_PlayerShoot();
            newpsp = S_RIFLASH1;
            break;
        case wp_shotgun:
            player->ammo[am_shell]--;
            P_PlayerShoot();
            newpsp = S_SGUNFLASH1;
            break;
        case wp_auto:
            player->ammo[am_clip]--;
            P_PlayerShoot();
            newpsp = S_AUTOFLASH1;
            break;
        case wp_missile:
            player->ammo[am_misl]--;
            newpsp = S_SGUNFLASH1;
            break;
        case wp_claw:
            player->ammo[am_soul]--;
            newpsp = S_SGUNFLASH1;
            break;
        case wp_bfg:
            player->ammo[am_cell]--;
            newpsp = S_SGUNFLASH1;
            break;
        }
    }
    else
    {
        IO_Error("A_FireGun: bad weapon number");
    }
    if (playernum == sd->consoleplayer)
    {
        P_DrawAmmo();
    }
    P_SetPsprite(ps_flash, newpsp);
    return;
}

void A_Light0(pspdef_t * psp)
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
    fixed_t frac;
    line_t* line;
    int side;
    int i;

    vertex_t vv1, vv2;

    if (sector->validcheck != validcheck)
    {
        sector->validcheck = validcheck;
        
        for (i = 0; i < sector->linecount; i++)
        {
            line = &lines[sector->lines[i]];
            if ((line->flags & ML_TWOSIDED) || (line->special != 0))
            {
                R_TransformVertex(&points[line->p1], &vv2);
                R_TransformVertex(&points[line->p2], &vv1);
                if (((int)vv2.tx < 0) && (0 < vv1.tx)) 
                {
                    frac = FixedDiv(-vv2.tx, vv1.tx - vv2.tx);
                    side = 0;
                }
                else 
                {
                    if ((((int)vv2.tx < 1) || (-1 < vv1.tx)) || (!(line->flags & ML_TWOSIDED)))
                        continue;

                    frac = FixedDiv(vv2.tx, vv2.tx - vv1.tx);
                    side = 1;
                }

                frac = vv2.tz + FixedMul(vv1.tz - vv2.tz, frac);

                if ((frac <= 40*FRACUNIT) && (-1 < frac)) 
                {
                    if (line->special == 0) 
                    {
                        P_UseFrontLines(sectors + sides[line->side[side ^ 1]].sector);
                    }
                    else 
                    {
                        P_PlayerUseSpecialLine(line, side);
                    }
                }
            }
        }
    }
    return;
}

void P_Use(void)
{
    viewx = player->r->x;
    viewy = player->r->y;
    viewsin = sines[player->r->angle];
    viewcos = cosines[player->r->angle];
    validcheck++;
    P_UseFrontLines(&sectors[player->r->sector]);
    return;
}

void P_Thrust(fixed_t angle, fixed_t speed)
{
    angle = angle & 0x1fff;
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
    return;
}

void P_ChangeWeapon(weapontype_t newweapon)
{
    if ((newweapon < 8) && (player->weaponowned[newweapon] != 0))
    {
        player->pendingweapon = newweapon;
    }
    return;
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
    return;
}

int P_CheckSolidLines(int sectornum)
{
    short sec;
    fixed_t clipintercept;
    fixed_t lineintercept;
    int iVar4;
    int i;
    sector_t* sector;

    fixed_t temp;

    point_t p1, p2;

    sector = &sectors[sectornum];
    if (sector->validcheck != validcheck)
    {
        sector->validcheck = validcheck;
        
        for (i = 0; i < sector->linecount; i++)
        {
            clipline = &lines[sector->lines[i]];
            if ((((clipline->bbox[1] <= clipbox[3]) && (clipbox[1] <= clipline->bbox[3])) &&
                (clipbox[0] <= clipline->bbox[2])) && (clipline->bbox[0] <= clipbox[2])) 
            {
                if (clipline->slopetype == ls_slope) 
                {
                    p1 = points[clipline->p1];
                    p2 = points[clipline->p2];

                    lineintercept = clipline->yintercept;

                    iVar4 = lineintercept + FixedMul(clipline->slope, clipbox[1]);
                    temp = lineintercept + FixedMul(clipline->slope, clipbox[3]);

                    if ((iVar4 < clipbox[0] || clipbox[2] < iVar4) && ((temp < clipbox[0] || clipbox[2] < temp)))
                    {
                        clipintercept = clipbox[0] - lineintercept;

                        iVar4 = FixedDiv(clipintercept, clipline->slope);

                        if ((iVar4 < clipbox[1]) || (clipbox[3] < iVar4))
                        {
                            clipintercept = clipbox[2] - lineintercept;
                            lineintercept = FixedDiv(clipintercept, clipline->slope);
                            if ((lineintercept < clipbox[1]) || (clipbox[3] < lineintercept))
                                continue;
                        }
                    }
                }
                if ((clipline->flags & ML_BLOCKING)) 
                {
                    return 0;
                }
                if ((clipline->flags & ML_TWOSIDED)) 
                {
                    if ((int)sides[clipline->side[0]].sector == sectornum) 
                    {
                        sec = sides[clipline->side[1]].sector;
                    }
                    else 
                    {
                        sec = sides[clipline->side[0]].sector;
                    }
                    if (sectors[sec].ceilingheight - sector->floorheight < clipgap)
                    {
                        return 0;
                    }
                    if (clipstepup < sectors[sec].floorheight - sector->floorheight)
                    {
                        return 0;
                    }
                    if (P_CheckSolidLines(sec) == 0)
                    {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

int P_CheckPosition(fixed_t ox, fixed_t oy, fixed_t size, int basesector)
{
    fixed_t leftblock, rightblock, topblock, bottomblock;
    uint8_t* blmap;
    fixed_t x, y;

    clipbox[1] = ox - size;
    clipbox[3] = ox + size;
    clipbox[2] = oy + size;
    clipbox[0] = oy - size;

    blmap = P_BlockOrg(ox, oy);

    if ((*blmap & 4) != 0) 
    {
        P_GetThingAt(blmap);
    }
    if ((blmap[1] & 4) != 0) 
    {
        P_GetThingAt(blmap + 1);
    }
    if ((blmap[mapwidth] & 4) != 0)
    {
        P_GetThingAt(blmap + mapwidth);
    }
    if ((blmap[mapwidth + 1] & 4) != 0) 
    {
        P_GetThingAt(blmap + mapwidth + 1);
    }

    if (((((blmap[0] & 2) == 0) && ((blmap[1] & 2) == 0)) && ((blmap[mapwidth] & 2) == 0)) && ((blmap[mapwidth + 1] & 2) == 0)) 
    {
        rightblock = clipbox[0] - maporiginy >> 0x14;
        leftblock = clipbox[2] - maporiginy >> 0x14;
        x = rightblock;
        while (x <= leftblock)
        {
            topblock = clipbox[1] - maporiginx >> 0x14;
            bottomblock = clipbox[3] - maporiginx >> 0x14;
            y = topblock;
            while (y <= bottomblock) 
            {
                if (((blockmap[x * mapwidth + y]) & 1) != 0) 
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

    if (P_CheckPosition(tryx, tryy, 16 << FRACBITS, player->r->sector) == 0)
    {
        tryx = player->r->x + player->momx;
        tryy = player->r->y;
        if (P_CheckPosition(tryx, tryy, 16 << FRACBITS, player->r->sector) == 0)
        {
            tryx = player->r->x;
            tryy = player->r->y + player->momy;
            if (P_CheckPosition(tryx, tryy, 16 << FRACBITS, player->r->sector) == 0)
            {
                player->momx = player->momy = 0;
                return;
            }
        }
    }

    player->r->sector = P_CrossSectorBounds(player->r->sector, player->r->x, player->r->y, tryx, tryy);
    player->r->x = tryx;
    player->r->y = tryy;
    return;
}

void P_MovePlayer(void)
{
    if ((player->momx != 0) || (player->momy != 0)) 
    {
        P_RemoveBlockMarks(player->r);
        P_ClipPlayerMove();
        P_PlaceBlockMarks(player->r);
    }
    return;
}

void P_CalcBob(void)
{
    if (debugmove == 0) 
    {
        player->r->z = sectors[player->r->sector].floorheight;

        player->bob = FixedMul(player->momx, player->momx) + FixedMul(player->momy, player->momy);
        player->bob = player->bob >> 1;
        if (MAXBOB < player->bob)
        {
            player->bob = MAXBOB;
        }

        player->viewz = player->r->z + VIEWHEIGHT + FixedMul(player->bob / 2, sines[processedframe * 0x75 & 0xfff]);
    }
    else 
    {
        player->viewz = player->r->z;
        player->bob = 0;
    }
    return;
}

void P_DeadThink(framecmd_t* cmd)
{
    fixed_t delta;

    P_MovePlayer();
    P_SlowPlayer();
    delta = player->viewz - player->r->z;
    if (delta <= 20*FRACUNIT) 
    {
        if (delta <= 8*FRACUNIT)
        {
            if ((cmd->buttons & 2) != 0) 
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
    return;
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
        if ((player->strafedown == 0) && ((cmd->buttons & 1) != 0)) 
        {
            P_Use();
        }
        player->strafedown = (cmd->buttons & 1);
        weaponchange = (cmd->buttons >> 2) & 0xf;
        if (weaponchange != wp_nochange) 
        {
            P_ChangeWeapon(weaponchange);
        }
        player->firedown = (cmd->buttons & 2);

        if ((cmd->buttons & 1) == 0) 
        {
            player->r->angle = (player->r->angle - FixedMul(cmd->xmove, 0x20000)) & 0x1fff;
        }
        else if (cmd->xmove != 0)
        {
            P_Thrust(player->r->angle + -0x800, (int)(short)(char)cmd->xmove * 640);
        }

        if (cmd->ymove != 0) 
        {
            P_Thrust(player->r->angle, (int)(short)(char)cmd->ymove * 640);
        }
        P_MovePlayer();
        P_SlowPlayer();
        P_CalcBob();
    }
    return;
}
