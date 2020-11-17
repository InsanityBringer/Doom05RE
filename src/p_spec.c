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

char* alphSwitchList[16] =
{
	"COMWALL1", "COMWALL2",
	"TH_STSW1", "TH_STSW2",
	"BRN96SW1", "BRN96SW2",
	"JR_B1SW1", "JR_B1SW2",
	"EXITBUTT", "EXITBUT2",
	"JR_SW2", "JR_SW2A",
	"TH_192GS", "TH_192GT"
};

flatanim_t flatAnims[4] =
{
	{"NUKAGE3", "NUKAGE1", 3, 16},
	{"SEWAGE3", "SEWAGE1", 3, 32},
	{"CONVNA3", "CONVNA1", 3, 16}
};

int numswitches;
int switchlist[100];

anim_t anims[32];
anim_t* lastAnim;

int P_CheckSingleSectorHeights(int sector, int force, int recurse)
{
	return 1;
}

int P_CheckSectorHeights(int sector, int force)
{
	return 1;
}

int P_FindLowestFloorSurrounding(sector_t* sector, int secnum)
{
    short other;
    line_t* local_EDX_90;
    fixed_t check;
    int i;

    check = 0x7fffffff;
    i = 0;
    while (i < sector->linecount) 
    {
        local_EDX_90 = &lines[sector->lines[i]];
        if ((local_EDX_90->flags & ML_TWOSIDED) != 0) 
        {
            if ((int)sides[local_EDX_90->side[0]].sector == secnum) 
            {
                other = sides[local_EDX_90->side[1]].sector;
            }
            else 
            {
                other = sides[local_EDX_90->side[0]].sector;
            }
            if (sectors[other].floorheight < check) 
            {
                check = sectors[other].floorheight;
            }
        }
        i++;
    }
    return check;
}

int P_FindHighestFloorSurrounding(sector_t* sector, int secnum)
{
    short other;
    line_t* plVar2;
    fixed_t floor;
    int i;

    floor = -0x1f40000;
    i = 0;
    while (i < sector->linecount) 
    {
        plVar2 = &lines[sector->lines[i]];
        if ((plVar2->flags & ML_TWOSIDED) != 0) 
        {
            if ((int)sides[plVar2->side[0]].sector == secnum) 
            {
                other = sides[plVar2->side[1]].sector;
            }
            else
            {
                other = sides[plVar2->side[0]].sector;
            }
            if (floor < sectors[other].floorheight) 
            {
                floor = sectors[other].floorheight;
            }
        }
        i++;
    }
    return floor;
}

int P_FindNextHighestFloor(sector_t* sec, int secnum, int currentheight)
{
    short other;
    line_t* plVar2;
    int i;

    i = 0;
    do 
    {
        if (sector->linecount <= i) 
        {
            return currentheight;
        }
        plVar2 = &lines[sec->lines[i]];
        if ((plVar2->flags & ML_TWOSIDED) != 0) 
        {
            if ((int)sides[plVar2->side[0]].sector == secnum)
            {
                other = sides[plVar2->side[1]].sector;
            }
            else 
            {
                other = sides[plVar2->side[0]].sector;
            }
            if (currentheight < sectors[other].floorheight) 
            {
                return sectors[other].floorheight;
            }
        }
        i = i + 1;
    } while (1);
}

int P_FindLowestCeilingSurrounding(sector_t* sec, int secnum)
{
    short sVar1;
    line_t* local_EDX_90;
    fixed_t local_24;
    int iVar2;

    local_24 = 0x7fffffff;
    iVar2 = 0;
    while (iVar2 < sec->linecount)
    {
        local_EDX_90 = &lines[sec->lines[iVar2]];
        if ((local_EDX_90->flags & ML_TWOSIDED) != 0) 
        {
            if ((int)sides[local_EDX_90->side[0]].sector == secnum) 
            {
                sVar1 = sides[local_EDX_90->side[1]].sector;
            }
            else
            {
                sVar1 = sides[local_EDX_90->side[0]].sector;
            }
            if (sectors[sVar1].ceilingheight < local_24) 
            {
                local_24 = sectors[sVar1].ceilingheight;
            }
        }
        iVar2 = iVar2 + 1;
    }
    return local_24;
}

void P_MoveThings(int sector, int ymove)
{
    thing_t* local_18;

    local_18 = sectors[sector].things;
    while (local_18 != NULL) 
    {
        local_18->z += ymove;
        local_18 = local_18->next;
    }
    return;
}

int P_FindSectorFromLineTag(line_t* line, int start)
{
    int secnum;

    secnum = start;
    do 
    {
        secnum = secnum + 1;
        if (numsectors <= secnum) 
        {
            return -1;
        }
    } while (sectors[secnum].tag != line->tag);
    return secnum;
}

int P_FindMinSurroundingLight(sector_t* sector, int check)
{
    int iVar1;
    int side;

    iVar1 = 0;
    while (1) 
    {
        if (sector->linecount <= iVar1) 
        {
            return check;
        }
        if ((lines[sector->lines[iVar1]].flags & ML_TWOSIDED) != 0) break;
        iVar1++;
    }

    side = lines[sector->lines[iVar1]].side[0];

    if (sides[side].sector == (sector - sectors))
    {
        side = lines[sector->lines[iVar1]].side[1];
    }

    if (check <= sectors[sides[side].sector].lightlevel)
    {
        return check;
    }
    return sectors[sides[side].sector].lightlevel;
}

extern int flatstartlump;
void EV_GoopChange(line_t* line, int side)
{
    int iVar1;
    short local_c;
    int local_8;

    if (side == 0) 
    {
        iVar1 = W_GetNumForName("SEWAGE1");
        local_c = (short)iVar1;
    }
    else 
    {
        iVar1 = W_GetNumForName("NUKAGE1");
        local_c = (short)iVar1;
    }
    local_c = local_c - (short)flatstartlump;
    local_8 = 0;
    while (local_8 < numsectors)
    {
        if (sectors[local_8].tag == line->tag) 
        {
            sectors[local_8].floortexture = local_c;
        }
        local_8++;
    }
    return;
}

void T_MoveFloor(floormove_t* floor)
{
    int uVar1;

    uVar1 = floor->direction;

    if (uVar1 == 1) 
    {
        floor->sector->floorheight = floor->sector->floorheight + floor->speed;
        P_MoveThings(floor->sector - sectors, floor->speed);
        if (floor->floordestheight < floor->sector->floorheight) 
        {
            P_MoveThings(floor->sector - sectors, floor->floordestheight - floor->sector->floorheight);
            floor->sector->floorheight = floor->floordestheight;
            floor->sector->specialdata = 0;
            P_RemoveThinker((thinker_t*)floor);
        }
    }
    else if (uVar1 == -1)
    {
        floor->sector->floorheight = floor->sector->floorheight - floor->speed;
        P_MoveThings(floor->sector - sectors, -floor->speed);
        if (floor->sector->floorheight < floor->floordestheight) 
        {
            P_MoveThings(floor->sector - sectors, floor->floordestheight - floor->sector->floorheight);
            floor->sector->floorheight = floor->floordestheight;
            floor->sector->specialdata = 0;
            P_RemoveThinker((thinker_t*)floor);
        }
    }
    return;
}

int EV_BuildStairs(line_t* line)
{
    short sVar1;
    short sVar2;
    int ok;
    floormove_t* floor;
    floormove_t* local_3c;
    sector_t* sec;
    int i;
    int height;
    int secnum;
    int rtn;

    rtn = 0;
    line->special = 0;
    secnum = -1;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata != NULL)
            continue;

        rtn = 1;
        height = (sec->floorheight + (8 << FRACBITS));
        floor = (floormove_t*)Z_Malloc(playzone, sizeof(floormove_t));
        P_AddThinker((thinker_t*)floor);
        sec->specialdata = (void*)floor;
        floor->thinker.function = &T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED >> 2;
        floor->floordestheight = height;
        sVar1 = sec->floortexture;

        do
        {
            ok = 0;
            i = 0;

            while (i < sec->linecount)
            {
                if (((lines[sec->lines[i]].flags & ML_TWOSIDED) != 0) && (secnum == (int)sides[lines[sec->lines[i]].side[0]].sector))
                {
                    sVar2 = sides[lines[sec->lines[i]].side[1]].sector;
                    sec = sectors + (int)sVar2;
                    if (sec->floortexture != sVar1) break;
                    height = (void*)((int)height + (8 << FRACBITS));
                    if (sec->specialdata == NULL)
                    {
                        floor = (floormove_t*)Z_Malloc(playzone, sizeof(floormove_t));
                        P_AddThinker((thinker_t*)floor);
                        sec->specialdata = (void*)floor;
                        floor->thinker.function = &T_MoveFloor;
                        floor->direction = 1;
                        floor->sector = sec;
                        floor->speed = FLOORSPEED >> 2;
                        floor->floordestheight = height;
                        ok = 1;
                        secnum = (int)sVar2;
                    }
                }
                i++;
            }
        } while (ok);
    }

    return rtn;
}

int EV_RaiseFloorUpToNearest(line_t* line)
{
    floormove_t* floor;
    int height;
    sector_t* sec;
    int secnum;
    int ret = 0;

    line->special = 0;
    secnum = -1;
    while (secnum = P_FindSectorFromLineTag(line, secnum), -1 < secnum) 
    {
        sec = &sectors[secnum];
        if (sec->specialdata == NULL)
        {
            floor = (floormove_t*)Z_Malloc(playzone, sizeof(floormove_t));
            P_AddThinker((thinker_t*)floor);
            sec->specialdata = (void*)floor;
            floor->thinker.function = &T_MoveFloor;
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            height = P_FindNextHighestFloor(sec, secnum, sec->floorheight);
            floor->floordestheight = height;
            ret = 1;
        }
    }
    return ret;
}

int EV_RaiseFloor(line_t* line)
{
    floormove_t* floor;
    int height;
    sector_t* sec;
    int secnum;
    int ret = 0;

    line->special = 0;
    secnum = -1;
    while (secnum = P_FindSectorFromLineTag(line, secnum), -1 < secnum) 
    {
        sec = &sectors[secnum];
        if (sec->specialdata == NULL) 
        {
            floor = (floormove_t*)Z_Malloc(playzone, sizeof(floormove_t));
            P_AddThinker((thinker_t*)floor);
            *(floormove_t**)&sec->specialdata = floor;
            floor->thinker.function = &T_MoveFloor;
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            height = P_FindLowestCeilingSurrounding(sec, secnum);
            floor->floordestheight = height;
            if (sec->ceilingheight < floor->floordestheight) 
            {
                floor->floordestheight = sec->ceilingheight;
            }
            ret = 1;
        }
    }
    return ret;
}

int EV_LowerFloor(line_t* line)
{
    int height;
    sector_t* sector;
    floormove_t* floor;
    int secnum;
    int ret = 0;

    line->special = 0;
    secnum = -1;
    while (secnum = P_FindSectorFromLineTag(line, secnum), -1 < secnum) 
    {
        sector = &sectors[secnum];
        if (sector->specialdata == 0) 
        {
            floor = (floormove_t*)Z_Malloc(playzone, sizeof(floormove_t));
            P_AddThinker((thinker_t*)floor);
            sector->specialdata = floor;
            floor->thinker.function = &T_MoveFloor;
            floor->direction = -1;
            floor->sector = sector;
            floor->speed = FLOORSPEED;
            height = P_FindHighestFloorSurrounding(sector, secnum);
            floor->floordestheight = height;
            ret = 1;
        }
    }
    return ret;
}

void T_VerticalDoor(vldoor_t* door)
{
    int iVar2;
    int uVar3;

    iVar2 = door->direction;
    if (iVar2 == -1)
    {
        if ((door->sector - sectors) == player->r->sector) 
        {
            door->direction = 0;
            door->topcountdown = 10;
        }
        else 
        {
            door->sector->ceilingheight -= door->speed;
            uVar3 = door->type;
            if (uVar3 == normal)
            {
                if (door->sector->ceilingheight <= door->sector->floorheight) 
                {
                    door->sector->ceilingheight = door->sector->floorheight;
                    door->sector->specialdata = 0;
                    P_RemoveThinker((thinker_t*)door);
                }
            }
            else if (uVar3 == close30ThenOpen) 
            {
                if (door->sector->ceilingheight <= door->sector->floorheight) 
                {
                    door->sector->ceilingheight = door->sector->floorheight;
                    door->direction = 0;
                    door->topcountdown = 2100;
                }
            }
            else if (uVar3 == close)
            {
                if ((door->sector->ceilingheight <= door->sector->floorheight))
                {
                    door->sector->ceilingheight = door->sector->floorheight;
                    door->sector->specialdata = 0;
                    P_RemoveThinker((thinker_t*)door);
                }
            }
        }
    }
    else if (iVar2 == 0) 
    {
        door->topcountdown--;
        if (door->topcountdown == 0)
        {
            if (door->type == 0)
            {
                door->direction = -1;
            }
            else 
            {
                if (door->type == close30ThenOpen) 
                {
                    door->direction = 1;
                }
            }
        }
    }
    else 
    {
        if (iVar2 == 1)
        {
            door->sector->ceilingheight += door->speed;
            uVar3 = door->type;
            if (uVar3 == 0)
            {
                if (door->topheight <= door->sector->ceilingheight) 
                {
                    door->sector->ceilingheight = door->topheight;
                    door->direction = 0;
                    door->topcountdown = door->topwait;
                }
            }
            else
            {
                if (uVar3 < 2)
                {
                    if (door->topheight <= door->sector->ceilingheight) 
                    {
                        door->sector->ceilingheight = door->topheight;
                        door->sector->specialdata = 0;
                        P_RemoveThinker((thinker_t*)door);
                    }
                }
                else
                {
                    if ((uVar3 == 3) && (door->topheight <= door->sector->ceilingheight)) 
                    {
                        door->sector->ceilingheight = door->topheight;
                        door->sector->specialdata = 0;
                        P_RemoveThinker((thinker_t*)door);
                    }
                }
            }
        }
    }
    return;
}

int EV_DoDoor(line_t* line, vldoor_e type)
{
    vldoor_t* door;
    int height;
    sector_t* sec;
    int secnum;
    int ret = 0;

    secnum = -1;
    do 
    {
        do 
        {
            secnum = P_FindSectorFromLineTag(line, secnum);
            if (secnum < 0) 
            {
                return ret;
            }
            sec = &sectors[secnum];
        } while (sec->specialdata != 0);
        door = (vldoor_t*)Z_Malloc(playzone, sizeof(vldoor_t));
        P_AddThinker((thinker_t*)door);
        sec->specialdata = (void*)door;
        door->thinker.function = &T_VerticalDoor;
        door->sector = sec;
        if (type == normal || type == open)
        {
            door->direction = 1;
        }
        else
        {
            door->direction = -1;
        }

        door->type = type;
        door->speed = VDOORSPEED;
        door->topwait = VDOORWAIT;
        height = P_FindLowestCeilingSurrounding(sec, secnum);
        door->topheight = height;
        door->topheight = door->topheight + -(4 << FRACBITS);
        ret = 1;
    } while (1);
    return ret;
}

void EV_VerticalDoor(line_t* line, int side)
{
    short uVar1;
    short sVar2;
    vldoor_t* door;
    int iVar3;
    sector_t* sec;
    vldoor_t* local_20;

    uVar1 = line->special;

    iVar3 = 1;
    if (uVar1 == 26) 
    {
        iVar3 = player->items[0];
    }
    else if (uVar1 == 27) 
    {
        iVar3 = player->items[1];
    }
    else if (uVar1 == 28)
    {
        iVar3 = player->items[2];
    }

    if (iVar3 == 0)
    {
        return;
    }

    sVar2 = sides[line->side[side ^ 1]].sector;
    sec = sectors + (int)sVar2;
    if (sec->specialdata == NULL) 
    {
        door = (vldoor_t*)Z_Malloc(playzone, sizeof(vldoor_t));
        P_AddThinker((thinker_t*)door);
        sec->specialdata = door;
        door->thinker.function = &T_VerticalDoor;
        door->sector = sec;
        door->direction = 1;
        door->type = 0;
        door->speed = VDOORSPEED;
        door->topwait = VDOORWAIT;
        iVar3 = P_FindLowestCeilingSurrounding(sec, (int)sVar2);
        door->topheight = iVar3;
        door->topheight = door->topheight + -(4 << FRACBITS);
    }
    else
    {
        door = (vldoor_t*)sec->specialdata;

        if (door->direction == -1)
            door->direction = 1;
        else
            door->direction = -1;
    }
    return;
}

void T_PlatRaise(plat_t* plat)
{
    plat_e pVar1;
    plattype_e pVar2;

    pVar1 = plat->status;
    if (pVar1 == up) 
    {
        plat->sector->floorheight = plat->sector->floorheight + plat->speed;
        P_MoveThings(plat->sector - sectors, plat->speed);
        if (plat->high < plat->sector->floorheight) 
        {
            P_MoveThings(plat->sector - sectors, plat->high - plat->sector->floorheight);
            plat->sector->floorheight = plat->high;
            plat->count = plat->wait;
            plat->status = waiting;
            pVar2 = plat->type;
            if (pVar2 != raise) 
            {
                if (pVar2 < raiseAndChange) 
                {
                    plat->sector->specialdata = 0;
                    P_RemoveThinker((thinker_t*)plat);
                }
                else
                {
                    if (pVar2 == raiseAndChange) 
                    {
                        plat->sector->specialdata = 0;
                        P_RemoveThinker((thinker_t*)plat);
                    }
                }
            }
        }
    }
    else 
    {
        if (pVar1 < waiting) 
        {
            plat->sector->floorheight = plat->sector->floorheight - plat->speed;
            P_MoveThings(plat->sector - sectors, -plat->speed);
            if (plat->sector->floorheight < plat->low) 
            {
                P_MoveThings(plat->sector - sectors, plat->low - plat->sector->floorheight);
                plat->sector->floorheight = plat->low;
                plat->count = plat->wait;
                plat->status = waiting;
            }
        }
        else 
        {
            if ((pVar1 == waiting) && (plat->count = plat->count + -1, plat->count == 0)) 
            {
                if (plat->sector->floorheight == plat->low)
                {
                    plat->status = up;
                }
                else 
                {
                    plat->status = down;
                }
            }
        }
    }
    return;
}

void P_SpawnPlatRaise(sector_t* sector, int secnum)
{
    plat_t* plat;

    plat = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
    P_AddThinker((thinker_t*)plat);
    plat->speed = PLATSPEED;
    plat->low = P_FindLowestFloorSurrounding(sector, secnum);
    plat->high = P_FindHighestFloorSurrounding(sector, secnum);
    plat->wait = PLATWAIT;
    plat->sector = sector;

    plat->status = D_Rnd() & 1;
    plat->thinker.function = &T_PlatRaise;
    plat->type = raise;
    plat->sector->specialdata = (void*)plat;
    sector->special = 0;
    return;
}

int EV_RaisePlatUpToNearestFloorAndChange(line_t* line, int side)
{
    plat_t* plat;
    int iVar1;
    sector_t* sec;
    int local_1c;
    plat_t* local_18;
    int ret = 0;

    local_1c = -1;
    while (local_1c = P_FindSectorFromLineTag(line, local_1c), -1 < local_1c)
    {
        sec = &sectors[local_1c];
        if (sec->specialdata == 0) 
        {
            plat = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
            P_AddThinker((thinker_t*)plat);
            plat->speed = PLATSPEED/2;
            sec->floortexture = sectors[sides[line->side[side]].sector].floortexture;
            iVar1 = P_FindNextHighestFloor(sec, local_1c, sec->floorheight);
            plat->high = iVar1;
            plat->wait = 0;
            plat->sector = sec;
            plat->status = up;
            plat->thinker.function = &T_PlatRaise;
            plat->type = raiseAndChange;
            plat->sector->specialdata = (void*)plat;
            ret = 1;
        }
    }
    return ret;
}

int EV_PlatRaiseAndChange(line_t* line, int amount)
{
    plat_t* local_EAX_112;
    sector_t* psVar1;
    int local_20;
    int local_1c;
    plat_t* local_18;

    local_1c = -1;
    local_20 = 0;
    while (local_1c = P_FindSectorFromLineTag(line, local_1c), -1 < local_1c) 
    {
        psVar1 = sectors + local_1c;
        if (psVar1->specialdata == 0)
        {
            local_20 = 1;
            local_EAX_112 = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
            P_AddThinker((thinker_t*)local_EAX_112);
            local_EAX_112->speed = PLATSPEED / 2;
            psVar1->floortexture = sectors[sides[line->side[0]].sector].floortexture;
            local_EAX_112->high = psVar1->floorheight + amount * FRACUNIT;
            local_EAX_112->wait = 0;
            local_EAX_112->sector = psVar1;
            local_EAX_112->status = up;
            local_EAX_112->thinker.function = &T_PlatRaise;
            local_EAX_112->type = raiseAndChange;
            local_EAX_112->sector->specialdata = (void*)local_EAX_112;
        }
    }
    return local_20;
}

int EV_PlatLowerWaitRaiseStay(line_t* line)
{
    plat_t* plat;
    int iVar1;
    sector_t* sector;
    int local_20;
    plat_t* local_1c;
    int ret = 0;

    local_20 = -1;
    while (local_20 = P_FindSectorFromLineTag(line, local_20), -1 < local_20)
    {
        sector = &sectors[local_20];
        if (sector->specialdata == 0) 
        {
            plat = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
            P_AddThinker((thinker_t*)plat);
            plat->speed = PLATSPEED;
            iVar1 = P_FindLowestFloorSurrounding(sector, local_20);
            plat->low = iVar1;
            plat->high = sector->floorheight;
            plat->wait = PLATWAIT;
            plat->sector = sector;
            plat->status = down;
            plat->thinker.function = &T_PlatRaise;
            plat->type = downWaitUpStay;
            plat->sector->specialdata = (void*)plat;
            ret = 1;
        }
    }
    return ret;
}

void EV_TurnTagLIghtsOff(line_t* line)
{
    short sVar1;
    short sVar2;
    int local_20;
    int iVar3;

    local_20 = 0;
    while (local_20 < numsectors) 
    {
        if (sectors[local_20].tag == line->tag) 
        {
            sVar2 = sectors[local_20].lightlevel;
            iVar3 = 0;
            while (iVar3 < sector->linecount)
            {
                if ((lines[sector->lines[iVar3]].flags & ML_TWOSIDED) != 0) 
                {
                    if ((int)sides[lines[sector->lines[iVar3]].side[1]].sector == local_20)
                    {
                        sVar1 = sectors[sides[lines[sector->lines[iVar3]].side[0]].sector].lightlevel;
                    }
                    else 
                    {
                        sVar1 = sectors[sides[lines[sector->lines[iVar3]].side[1]].sector].lightlevel;
                    }
                    if (sVar1 < sVar2) 
                    {
                        sVar2 = sVar1;
                    }
                }
                iVar3++;
            }
            sectors[local_20].lightlevel = sVar2;
        }
        local_20++;
    }
    return;
}

void EV_LightTurnOn(line_t* line, int bright)
{
    short sVar1;
    int local_24;
    int local_20;
    int iVar2;

    iVar2 = 0;
    local_24 = bright;
    while (iVar2 < numsectors) 
    {
        if (sectors[iVar2].tag == line->tag) 
        {
            if (local_24 == 0)
            {
                local_20 = 0;
                while (local_20 < sectors[iVar2].linecount) 
                {
                    if ((lines[sectors[iVar2].lines[local_20]].flags & 4) == 0)
                    {
                        if ((int)sides[lines[sectors[iVar2].lines[local_20]].side[0]].sector == iVar2)
                        {
                            sVar1 = sectors[sides[lines[sectors[iVar2].lines[local_20]].side[1]].sector].lightlevel;
                        }
                        else 
                        {
                            sVar1 = sectors[sides[lines[sectors[iVar2].lines[local_20]].side[0]].sector].lightlevel;
                        }
                        if (local_24 < (int)sVar1)
                        {
                            local_24 = (int)sVar1;
                        }
                    }
                    local_20++;
                }
            }
            sectors[iVar2].lightlevel = (short)local_24;
        }
        iVar2 = iVar2 + 1;
    }
    return;
}

void EV_StartLightStrobing(line_t* line);

void P_PlayerCrossSpecialLine(line_t* line)
{
    short uVar1;

    uVar1 = line->special;
    if (uVar1 < 0xc) 
    {
        if (uVar1 < 3) 
        {
            if ((uVar1 != 0) && (1 < uVar1))
            {
                EV_DoDoor(line, open);
            }
        }
        else 
        {
            if (uVar1 < 4) 
            {
                EV_DoDoor(line, close);
            }
            else 
            {
                if (uVar1 < 5) 
                {
                    EV_DoDoor(line, normal);
                }
                else 
                {
                    if (uVar1 < 6) 
                    {
                        EV_RaiseFloor(line);
                    }
                    else
                    {
                        if (uVar1 == 10) 
                        {
                            EV_PlatLowerWaitRaiseStay(line);
                        }
                    }
                }
            }
        }
    }
    else 
    {
        if (uVar1 < 0xd) 
        {
            EV_LightTurnOn(line, 0);
        }
        else {
            if (uVar1 < 0x13) 
            {
                if (uVar1 < 0x10) 
                {
                    if (uVar1 == 0xd) 
                    {
                        EV_LightTurnOn(line, 0xff);
                    }
                }
                else 
                {
                    if (uVar1 < 0x11) 
                    {
                        EV_DoDoor(line, close30ThenOpen);
                    }
                    else 
                    {
                        if (uVar1 == 0x11) 
                        {
                            EV_StartLightStrobing(line);
                        }
                    }
                }
            }
            else 
            {
                if (uVar1 < 0x14) 
                {
                    EV_LowerFloor(line);
                }
                else 
                {
                    if (uVar1 < 0x68) 
                    {
                        if (uVar1 == 0x16)
                        {
                            EV_RaisePlatUpToNearestFloorAndChange(line, ((int)sides[line->side[0]].sector != player->r->sector));
                        }
                    }
                    else 
                    {
                        if (uVar1 < 0x69) 
                        {
                            EV_TurnTagLIghtsOff(line);
                        }
                        else 
                        {
                            if (uVar1 == 1000) 
                            {
                                gameaction = ga_completed;
                            }
                        }
                    }
                }
            }
        }
    }
    return;
}

int P_FindTexture(char* string)
{
    int uVar1;
    int i;

    i = 0;
    while (1) 
    {
        if (numtextures <= i)
        {
            IO_Error("P_InitSwitchList: Couldn't find %s!",string);
               return;
        }
        uVar1 = strcmp(texturelookup[i], string);
        if (uVar1 == 0) break;
        i++;
    }
    return i;
}

void P_InitSwitchList(void)
{
    int iVar1;
    int iVar2;

    iVar2 = 0;
    while (1) 
    {
        if (99 < iVar2) 
        {
            return;
        }
        if (alphSwitchList[iVar2] == NULL) break;
        iVar1 = P_FindTexture(alphSwitchList[iVar2]);
        switchlist[iVar2] = iVar1;
        iVar2++;
    }
    numswitches = iVar2;
    switchlist[iVar2] = -1;
    return;
}

void P_ChangeSwitchTexture(line_t* line, int useAgain)
{
    int local_24;

    if (useAgain == 0) 
    {
        line->special = 0;
    }
    local_24 = 0;
    while (1) 
    {
        if (numswitches <= (int)local_24) 
        {
            IO_Error("P_ChangeSwitchTexture: Couldn't find a matching switch!");
                return;
        }
        if (switchlist[local_24] == (int)sides[line->side[0]].toptexture)
        {
            sides[line->side[0]].toptexture = switchlist[local_24 ^ 1];
            return;
        }
        if (switchlist[local_24] == (int)sides[line->side[0]].midtexture) 
        {
            sides[line->side[0]].midtexture = switchlist[local_24 ^ 1];
            return;
        }
        if (switchlist[local_24] == (int)sides[line->side[0]].bottomtexture) break;
        local_24++;
    }
    sides[line->side[0]].bottomtexture = switchlist[local_24 ^ 1];
    return;
}

void P_PlayerUseSpecialLine(line_t* line, int side)
{
    short uVar1;
    int iVar2;
    line_t* local_1c;

    if (side != 0)
    {
        return;
    }
    uVar1 = line->special;
    if (uVar1 < 0x14) 
    {
        if (10 < uVar1) 
        {
            if (uVar1 < 0xc) 
            {
                IO_ClearKeys();
                gameaction = ga_completed;
                return;
            }
            if (uVar1 < 0xf) 
            {
                if (uVar1 != 0xe)
                {
                    return;
                }
                local_1c = line;
                iVar2 = EV_PlatRaiseAndChange(line, 0x20);
                if (iVar2 == 0) 
                {
                    return;
                }
                P_ChangeSwitchTexture(local_1c, 0);
                return;
            }
            if (uVar1 < 0x10) 
            {
                local_1c = line;
                iVar2 = EV_PlatRaiseAndChange(line, 0x18);
                if (iVar2 == 0) 
                {
                    return;
                }
                P_ChangeSwitchTexture(local_1c, 0);
                return;
            }
            if (uVar1 != 0x12) 
            {
                return;
            }
            local_1c = line;
            iVar2 = EV_RaiseFloorUpToNearest(line);
            if (iVar2 == 0) 
            {
                return;
            }
            P_ChangeSwitchTexture(local_1c, 0);
            return;
        }
        if (uVar1 == 0) 
        {
            return;
        }
        if (1 < uVar1) 
        {
            if (uVar1 != 7) 
            {
                return;
            }
            local_1c = line;
            iVar2 = EV_BuildStairs(line);
            if (iVar2 == 0) {
                return;
            }
            P_ChangeSwitchTexture(local_1c, 0);
            return;
        }
    }
    else 
    {
        if (uVar1 < 0x15) 
        {
            local_1c = line;
            iVar2 = EV_RaisePlatUpToNearestFloorAndChange(line, 0);
            if (iVar2 == 0) 
            {
                return;
            }
            P_ChangeSwitchTexture(local_1c, 0);
            return;
        }
        if (0x1c < uVar1) 
        {
            if (uVar1 < 0x1e) 
            {
                local_1c = line;
                iVar2 = EV_DoDoor(line, normal);
                if (iVar2 == 0) 
                {
                    return;
                }
                P_ChangeSwitchTexture(local_1c, 1);
                return;
            }
            if (uVar1 < 0x66) 
            {
                if (uVar1 != 0x65) 
                {
                    return;
                }
                local_1c = line;
                iVar2 = EV_RaiseFloor(line);
                if (iVar2 == 0) 
                {
                    return;
                }
                P_ChangeSwitchTexture(local_1c, 0);
                return;
            }
            if (uVar1 < 0x67) 
            {
                local_1c = line;
                iVar2 = EV_LowerFloor(line);
                if (iVar2 == 0) 
                {
                    return;
                }
                P_ChangeSwitchTexture(local_1c, 0);
                return;
            }
            if (uVar1 != 0x67) 
            {
                return;
            }
            local_1c = line;
            iVar2 = EV_DoDoor(line, open);
            if (iVar2 == 0) 
            {
                return;
            }
            P_ChangeSwitchTexture(local_1c, 0);
            return;
        }
        if (uVar1 < 0x16)
        {
            local_1c = line;
            iVar2 = EV_PlatLowerWaitRaiseStay(line);
            if (iVar2 == 0) 
            {
                return;
            }
            P_ChangeSwitchTexture(local_1c, 1);
            return;
        }
        if (uVar1 < 0x1a) 
        {
            return;
        }
    }
    EV_VerticalDoor(line, 0);
    return;
}

void P_PlayerShootSpecialLine(line_t* line)
{
    //it is once again, but a dream
    return;
}

void P_PlayerInSpecialSector(void)
{
    if (sectors[player->r->sector].special == 7) 
    {
        if (D_Rnd() < 8) 
        {
            P_DamagePlayer(playernum, 1);
        }
    }
    else 
    {
        IO_Error("P_PlayerInSpecialSector: unknown special %i", (int)sectors[player->r->sector].special);
    }
    return;
}

void P_AnimatePlanePics()
{
    anim_t* paVar2;
    int time;

    time = IO_GetTime();
    paVar2 = anims;
    while (paVar2 < lastAnim)
    {
        flatlookup[paVar2->picnum] = lumpinfo[flatstartlump + paVar2->basepic + ((time / paVar2->speed) % paVar2->numpics)].position;
        paVar2++;
    }
    return;
}

void P_StartupPicAnims(void)
{
    int iVar1;
    int iVar2;

    iVar2 = 0;
    lastAnim = anims;
    while (flatAnims[iVar2].numflats != 0) 
    {
        iVar1 = W_GetNumForName((char*)(flatAnims + iVar2));
        lastAnim->picnum = iVar1 - flatstartlump;
        iVar1 = W_GetNumForName(flatAnims[iVar2].endname);
        lastAnim->basepic = iVar1 - flatstartlump;
        lastAnim->numpics = flatAnims[iVar2].numflats;
        lastAnim->speed = flatAnims[iVar2].speed;
        lastAnim++;
        iVar2++;
    }
    return;
}

void T_LightFlash(lightflash_t* flash)
{
    flash->count--;
    if (flash->count == 0)
    {
        if ((int)flash->sector->lightlevel == flash->maxlight) 
        {
            flash->sector->lightlevel = flash->minlight;
            flash->count = (flash->mintime & D_Rnd()) + 1;
        }
        else 
        {
            flash->sector->lightlevel = flash->maxlight;
            flash->count = (flash->maxtime & D_Rnd()) + 1;
        }
    }
    return;
}

void P_SpawnLightFlash(sector_t* sector)
{
    lightflash_t* strobe;
    int iVar2;
    lightflash_t* local_1c;

    sector->special = 0;
    strobe = (lightflash_t*)Z_Malloc(playzone, sizeof(lightflash_t));
    P_AddThinker((thinker_t*)strobe);
    strobe->thinker.function = &T_LightFlash;
    strobe->sector = sector;
    strobe->maxlight = sector->lightlevel;
    iVar2 = P_FindMinSurroundingLight(sector, (int)sector->lightlevel);
    strobe->minlight = iVar2;
    strobe->maxtime = 64;
    strobe->mintime = 7;
    strobe->count = (strobe->maxtime & D_Rnd()) + 1;
    return;
}

void T_StrobeFlash(lightstrobe_t* flash)
{
    flash->count = flash->count + -1;
    if (flash->count == 0)
    {
        if ((int)flash->sector->lightlevel == flash->maxlight)
        {
            flash->sector->lightlevel = flash->minlight;
            flash->count = flash->brighttime;
        }
        else
        {
            flash->sector->lightlevel = flash->maxlight;
            flash->count = flash->darktime;
        }
    }
    return;
}

void P_SpawnStrobeFlash(sector_t* sector, int seccnum, int fastOrSlow)
{
    lightstrobe_t* strobe;
    int iVar2;

    strobe = (lightstrobe_t*)Z_Malloc(playzone, sizeof(lightstrobe_t));
    P_AddThinker((thinker_t*)strobe);
    strobe->sector = sector;
    strobe->darktime = fastOrSlow;
    strobe->brighttime = STROBEBRIGHT;
    strobe->thinker.function = &T_StrobeFlash;
    strobe->minlight = (int)sector->lightlevel;
    iVar2 = P_FindMinSurroundingLight(sector, (int)sector->lightlevel);
    strobe->maxlight = iVar2;
    if (strobe->maxlight == strobe->minlight)
    {
        strobe->maxlight = 0;
    }
    sector->special = 0;
    strobe->count = (D_Rnd() & 7) + 1;
    return;
}

void EV_StartLightStrobing(line_t* line)
{
    line_t* local_24;
    int start;

    start = -1;
    local_24 = line;
    while (start = P_FindSectorFromLineTag(local_24, start), -1 < start) 
    {
        if (sectors[start].specialdata == 0) 
        {
            P_SpawnStrobeFlash(sectors + start, start, SLOWDARK);
        }
    }
    return;
}

void P_SpawnSpecialSectors(void)
{
    int local_20;
    sector_t* sector;

    local_20 = 0;
    sector = sectors;
    while (local_20 < numsectors)
    {
        if (sector->special != 0)
        {
            switch (sector->special) 
            {
            case 1:
                P_SpawnLightFlash(sector);
                break;
            case 2:
                P_SpawnStrobeFlash(sector, local_20, FASTDARK);
                break;
            case 3:
                P_SpawnStrobeFlash(sector, local_20, SLOWDARK);
                break;
            case 4:
                P_SpawnPlatRaise(sector, local_20);
            }
        }
        local_20++;
        sector++;
    }
    return;
}


