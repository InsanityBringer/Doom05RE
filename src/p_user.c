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

void P_SetPSprite(int position, statenum_t stnum)
{
    int local_20;
    state_t* local_1c;
    pspdef_t* ppVar1;

    ppVar1 = &psprites[playernum * NUMPSPRITES + position];
    local_20 = stnum;
    do 
    {
        if (local_20 == 0) 
        {
            ppVar1->state = (state_t*)NULL;
            return;
        }
        local_1c = &states[local_20];
        ppVar1->state = local_1c;
        ppVar1->tics = states[local_20].tics;
        if (states[local_20].misc1 != 0)
        {
            ppVar1->sx = states[local_20].misc1 << FRACBITS;
            ppVar1->sy = states[local_20].misc2 << FRACBITS;
        }
        if (states[local_20].action != NULL)
        {
            ((void(*)())states[local_20].action)(ppVar1);
        }
        local_20 = local_1c->nextstate;
    } while (ppVar1->tics == 0);
    return;
}

void P_SetupPSprites(int pnum)
{
    int iVar1;

    iVar1 = 0;
    while (iVar1 < NUMPSPRITES) 
    {
        psprites[pnum * NUMPSPRITES + iVar1].state = NULL;
        iVar1 = iVar1 + 1;
    }
    player = &playerobjs[pnum];
    playernum = pnum;
    playerobjs[pnum].pendingweapon = playerobjs[pnum].readyweapon;
    P_BringUpWeapon();
    return;
}

void P_CalcSwing(void)
{
    swingx = FixedMul(player->bob, sines[processedframe * 117 & 0x1fff]);
    swingy = -FixedMul(swingx, sines[processedframe * 117 + 4096 & 0x1fff]);

    return;
}

void P_DrawPlayerShapes(int pnum)
{
    pspdef_t* local_EAX_179;
    pspdef_t* local_20;
    int local_24;

    player = &playerobjs[pnum];
    playernum = pnum;
    local_24 = pnum;
    P_CalcSwing();
    local_20 = &psprites[local_24 * NUMPSPRITES];
    local_24 = 0;
    while (local_24 < 3) 
    {
        if (local_20->state != NULL)
        {
            R_DrawPlayerShape(local_20->state->sprite, local_20->state->frame, local_20->sx + swingx >> FRACBITS, local_20->sy + swingy >> FRACBITS);
        }
        local_24 = local_24 + 1;
        local_20 = local_20 + 1;
    }
    local_24 = 0;
    while (local_24 < NUMPSPRITES)
    {
        if (local_20->state != NULL) 
        {
            R_DrawPlayerShape(local_20->state->sprite, local_20->state->frame, local_20->sx >> FRACBITS, local_20->sy >> FRACBITS);
        }
        local_24 = local_24 + 1;
        local_20 = local_20 + 1;
    }
    return;
}

void P_MovePlayerShapes(void)
{
    state_t* local_EAX_121;
    pspdef_t* local_20;
    int position;

    playernum = 0;
    while (playernum < 4)
    {
        if (sd->playeringame[playernum] != 0) 
        {
            player = &playerobjs[playernum];
            local_20 = &psprites[playernum * NUMPSPRITES];
            position = 0;
            while (position < NUMPSPRITES) 
            {
                if (((local_20->state != NULL) &&
                    (((position != 2 || (player->health != 0)) && (local_20->tics != -1)))) &&
                    (local_20->tics = local_20->tics + -1, local_20->tics == 0))
                {
                    P_SetPSprite(position, local_20->state->nextstate);
                }
                position++;
                local_20++;
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
    P_SetPSprite(2, newpsp);
    psprites[playernum * NUMPSPRITES + 2].sy = 0x900000;
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
    P_SetPSprite(2, newpsp);
    return;
}

void A_WeaponReady(pspdef_t* psp)
{
    statenum_t local_8;

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
                local_8 = S_BAYONETDOWN;
                break;
            case wp_rifle:
                local_8 = S_RIFLEDOWN;
                break;
            case wp_shotgun:
                local_8 = S_SGUNDOWN;
                break;
            case wp_auto:
                local_8 = S_AUTODOWN;
                break;
            default:
                local_8 = S_SGUNDOWN;
            }
        }
        else 
        {
            IO_Error("A_ChangeWeapon: bad weapon number\n");
        }
        P_SetPSprite(2, local_8);
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

    psp->sy += (6 << FRACBITS);
    if (0x8fffff < psp->sy)
    {
        if (player->health == 0)
        {
            P_SetPSprite(2, S_NULL);
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
    return;
}

void A_Raise(pspdef_t* psp)
{
    statenum_t local_8;

    psp->sy = psp->sy + -0x60000;
    if (psp->sy < 0x440001)
    {
        psp->sy = 0x440000;
        if (player->readyweapon < 8)
        {
            switch (player->readyweapon)
            {
            case wp_knife:
                local_8 = S_BAYONET;
                break;
            case wp_rifle:
                local_8 = S_RIFLE;
                break;
            case wp_shotgun:
                local_8 = S_SGUN;
                break;
            case wp_auto:
                local_8 = S_AUTO;
                break;
            default:
                local_8 = S_SGUN;
            }
        }
        else 
        {
            IO_Error("A_ChangeWeapon: bad weapon number\n");
        }
        P_SetPSprite(2, local_8);
    }
    return;
}

void A_FireGun(pspdef_t* psp)
{
    statenum_t local_8;

    if (player->readyweapon < 8)
    {
        switch (player->readyweapon)
        {
        default:
            local_8 = S_NULL;
            break;
        case wp_rifle:
            player->ammo[am_clip]--;
            P_PlayerShoot();
            local_8 = S_RIFLASH1;
            break;
        case wp_shotgun:
            player->ammo[am_shell]--;
            P_PlayerShoot();
            local_8 = S_SGUNFLASH1;
            break;
        case wp_auto:
            player->ammo[am_clip]--;
            P_PlayerShoot();
            local_8 = S_AUTOFLASH1;
            break;
        case wp_missile:
            player->ammo[am_misl]--;
            local_8 = S_SGUNFLASH1;
            break;
        case wp_claw:
            player->ammo[am_soul]--;
            local_8 = S_SGUNFLASH1;
            break;
        case wp_bfg:
            player->ammo[am_cell]--;
            local_8 = S_SGUNFLASH1;
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
    P_SetPSprite(1, local_8);
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
    int local_2c;
    int in_stack_ffffff9c;
    int in_stack_ffffffb8;
    sector_t* local_28;
    line_t* local_24;
    int local_20;
    int iVar2;

    vertex_t vv1, vv2;

    if (sector->validcheck != validcheck)
    {
        sector->validcheck = validcheck;
        iVar2 = 0;
        local_28 = sector;
        while (iVar2 < local_28->linecount) 
        {
            local_24 = &lines[local_28->lines[iVar2]];
            if ((local_24->flags & ML_TWOSIDED) || (local_24->special != 0))
            {
                R_TransformVertex((vertex_t*)(points + local_24->p1), (vertex_t*)&vv2);
                R_TransformVertex((vertex_t*)(points + local_24->p2), (vertex_t*)&vv1);
                if (((int)vv2.tx < 0) && (0 < vv1.tx)) 
                {
                    local_2c = FixedDiv(-vv2.tx, vv1.tx - vv2.tx);
                    local_20 = 0;
                }
                else 
                {
                    if ((((int)vv2.tx < 1) || (-1 < vv1.tx)) || (!(local_24->flags & ML_TWOSIDED)))
                        goto LAB_00021ce5;

                    local_2c = FixedDiv(vv2.tx, vv2.tx - vv1.tx);
                    local_20 = 1;
                }

                local_2c = vv2.tz + FixedMul(vv1.tz - vv2.tz, local_2c);

                if ((local_2c < 0x280001) && (-1 < local_2c)) 
                {
                    if (local_24->special == 0) 
                    {
                        P_UseFrontLines(sectors + sides[local_24->side[local_20 ^ 1]].sector);
                    }
                    else 
                    {
                        P_PlayerUseSpecialLine(local_24, local_20);
                    }
                }
            }
        LAB_00021ce5:
            iVar2++;
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
    player_t* ppVar2;

    ppVar2 = player;
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
        player->momx = ppVar2->momy = 0;
    }
    return;
}

int P_CheckSolidLines(int sectornum)
{
    short sVar1;
    //longlong lVar2;
    int uVar3;
    int local_20;
    int iVar4;
    uint8_t bVar5;
    int local_30;
    int local_2c;
    sector_t* psVar6;

    fixed_t temp;

    point_t p1, p2;

    bVar5 = 0;
    psVar6 = &sectors[sectornum];
    if (psVar6->validcheck != validcheck)
    {
        psVar6->validcheck = validcheck;
        local_2c = 0;
        local_30 = sectornum;
        while (local_2c < psVar6->linecount)
        {
            clipline = &lines[psVar6->lines[local_2c]];
            if ((((clipline->bbox[1] <= clipbox[3]) && (clipbox[1] <= clipline->bbox[3])) &&
                (clipbox[0] <= clipline->bbox[2])) && (clipline->bbox[0] <= clipbox[2])) 
            {
                if (clipline->slopetype == ls_slope) 
                {
                    p1 = points[clipline->p1];
                    p2 = points[clipline->p2];

                    local_20 = clipline->yintercept;

                    iVar4 = local_20 + FixedMul(clipline->slope, clipbox[1]);
                    temp = local_20 + FixedMul(clipline->slope, clipbox[3]);

                    if ((iVar4 < clipbox[0] || clipbox[2] < iVar4) && ((temp < clipbox[0] || clipbox[2] < temp)))
                    {
                        uVar3 = clipbox[0] - local_20;

                        iVar4 = FixedDiv(uVar3, clipline->slope);

                        if ((iVar4 < clipbox[1]) || (clipbox[3] < iVar4))
                        {
                            uVar3 = clipbox[2] - local_20;
                            local_20 = FixedDiv(uVar3, clipline->slope);
                            if ((local_20 < clipbox[1]) || (clipbox[3] < local_20)) goto LAB_00022118;
                        }
                    }
                }
                if ((clipline->flags & ML_BLOCKING)) 
                {
                    return 0;
                }
                if ((clipline->flags & ML_TWOSIDED)) 
                {
                    if ((int)sides[clipline->side[0]].sector == local_30) 
                    {
                        sVar1 = sides[clipline->side[1]].sector;
                    }
                    else 
                    {
                        sVar1 = sides[clipline->side[0]].sector;
                    }
                    local_20 = (int)sVar1;
                    if (sectors[local_20].ceilingheight - psVar6->floorheight < clipgap) 
                    {
                        return 0;
                    }
                    if (clipstepup < sectors[local_20].floorheight - psVar6->floorheight) 
                    {
                        return 0;
                    }
                    local_20 = P_CheckSolidLines(local_20);
                    if (local_20 == 0) 
                    {
                        return 0;
                    }
                }
            }
        LAB_00022118:
            local_2c = local_2c + 1;
        }
    }
    return 1;
}

int P_CheckPosition(fixed_t x, fixed_t y, fixed_t size, int basesector)
{
    uint8_t* local_2c;
    int local_24;
    int local_30;
    int local_20;

    clipbox[1] = x - size;
    clipbox[3] = x + size;
    clipbox[2] = y + size;
    clipbox[0] = y - size;

    local_30 = basesector;
    local_2c = P_BlockOrg(x, y);

    if ((*local_2c & 4) != 0) 
    {
        P_GetThingAt(local_2c);
    }
    if ((local_2c[1] & 4) != 0) 
    {
        P_GetThingAt(local_2c + 1);
    }
    if ((local_2c[mapwidth] & 4) != 0)
    {
        P_GetThingAt(local_2c + mapwidth);
    }
    if ((local_2c[mapwidth + 1] & 4) != 0) 
    {
        P_GetThingAt(local_2c + mapwidth + 1);
    }

    if (((((*local_2c & 2) == 0) && ((local_2c[1] & 2) == 0)) && ((local_2c[mapwidth] & 2) == 0)) && ((local_2c[mapwidth + 1] & 2) == 0)) 
    {
        local_24 = clipbox[0] - maporiginy >> 0x14;
        while (local_20 = clipbox[1] - maporiginx >> 0x14, local_24 <= clipbox[2] - maporiginy >> 0x14)
        {
            while (local_20 <= clipbox[3] - maporiginx >> 0x14) 
            {
                if (((blockmap[local_24 * mapwidth + local_20]) & 1) != 0) 
                {
                    validcheck++;
                    local_30 = P_CheckSolidLines(local_30);
                    return local_30;
                }
                local_20++;
            }
            local_24++;
        }
        local_30 = 1;
    }
    else 
    {
        local_30 = 0;
    }
    return local_30;
}

void P_ClipPlayerMove(void)
{
    player_t* ppVar1;
    fixed_t x;
    fixed_t y;
    int iVar2;

    x = player->r->x + player->momx;
    y = player->r->y + player->momy;
    iVar2 = P_CheckPosition(x, y, 0x100000, player->r->sector);
    if (iVar2 == 0)
    {
        x = player->r->x + player->momx;
        y = player->r->y;
        iVar2 = P_CheckPosition(x, y, 0x100000, player->r->sector);
        if (iVar2 == 0) 
        {
            x = player->r->x;
            y = player->r->y + player->momy;
            iVar2 = P_CheckPosition(x, y, 0x100000, player->r->sector);
            ppVar1 = player;
            if (iVar2 == 0) 
            {
                player->momy = 0;
                player->momx = ppVar1->momy;
                return;
            }
        }
    }
    iVar2 = P_CrossSectorBounds(player->r->sector, player->r->x, player->r->y, x, y);
    player->r->sector = iVar2;
    player->r->x = x;
    player->r->y = y;
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
    int iVar1;

    P_MovePlayer();
    P_SlowPlayer();
    iVar1 = player->viewz - player->r->z;
    if (iVar1 < 0x140001) 
    {
        if (iVar1 < 0x80001)
        {
            if ((cmd->buttons & 2) != 0) 
            {
                gameaction = ga_died;
            }
        }
        else 
        {
            player->viewz = player->viewz + -0x4000;
        }
    }
    else 
    {
        player->viewz = player->viewz + -0x10000;
    }
    return;
}

void P_PlayerGameThink(framecmd_t* cmd)
{
    int* piVar1;
    int* puVar2;

    weapontype_t newweapon;

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
        newweapon = (cmd->buttons >> 2) & 0xf;
        if (newweapon != wp_nochange) 
        {
            P_ChangeWeapon(newweapon);
        }
        player->firedown = (cmd->buttons & 2);
        if ((cmd->buttons & 1) == 0) 
        {
            player->r->angle = (player->r->angle - FixedMul(cmd->xmove, 0x20000)) & 0x1fff;
        }
        else 
        {
            if (cmd->xmove != 0)
            {
                P_Thrust(player->r->angle + -0x800, (int)(short)(char)cmd->xmove * 640);
            }
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
