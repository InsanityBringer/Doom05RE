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

#include <limits.h>

#include "doomdef.h"
#include "w_wad.h"
#include "p_actor.h"
#include "g_game.h"
#include "p_spec.h"

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

#define MAXSWITCHES     100              // max # of wall switches in a level

flatanim_t flatAnims[4] =
{
    {"NUKAGE3", "NUKAGE1", 3, 16},
    {"SEWAGE3", "SEWAGE1", 3, 32},
    {"CONVNA3", "CONVNA1", 3, 16}
};

#define MAXANIMS                32

int numswitches;
int switchlist[MAXSWITCHES];

anim_t anims[MAXANIMS];
anim_t* lastAnim;

int P_CheckSingleSectorHeights(sector_t* sector, boolean force, boolean recurse)
{
    return 1;
}

int P_CheckSectorHeights(sector_t* sector, boolean force)
{
    return 1;
}

//==================================================================
//
//	FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindLowestFloorSurrounding(sector_t* sec, int secnum)
{
    int i;
    line_t* check;
    sector_t* other;
    fixed_t floor = INT_MAX;

    for (i = 0; i < sec->linecount; i++)
    {
        check = &lines[sec->lines[i]];
        if (!(check->flags & ML_TWOSIDED))
            continue;

        if (sides[check->side[0]].sector == secnum)
            other = &sectors[sides[check->side[1]].sector];
        else
            other = &sectors[sides[check->side[0]].sector];

        if (other->floorheight < floor)
            floor = other->floorheight;
    }
    return floor;
}

//==================================================================
//
//	FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindHighestFloorSurrounding(sector_t* sec, int secnum)
{
    int i;
    line_t* check;
    sector_t* other;
    fixed_t floor = -500 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
    {
        check = &lines[sec->lines[i]];
        if (!(check->flags & ML_TWOSIDED))
            continue;

        if (sides[check->side[0]].sector == secnum)
            other = &sectors[sides[check->side[1]].sector];
        else
            other = &sectors[sides[check->side[0]].sector];

        if (other->floorheight > floor)
            floor = other->floorheight;
    }
    return floor;
}

//==================================================================
//
//	FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindNextHighestFloor(sector_t* sec, int secnum, int currentheight)
{
    int i;
    line_t* check;
    sector_t* other;
    fixed_t height = currentheight;

    for (i = 0; i < sec->linecount; i++)
    {
        check = &lines[sec->lines[i]];
        if (!(check->flags & ML_TWOSIDED))
            continue;

        if (sides[check->side[0]].sector == secnum)
            other = &sectors[sides[check->side[1]].sector];
        else
            other = &sectors[sides[check->side[0]].sector];

        if (other->floorheight > height)
            return other->floorheight;
    }

    return height;
}

//==================================================================
//
//	FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
//==================================================================
fixed_t P_FindLowestCeilingSurrounding(sector_t* sec, int secnum)
{
    int i;
    line_t* check;
    sector_t* other;
    fixed_t height = INT_MAX;

    for (i = 0; i < sec->linecount; i++)
    {
        check = &lines[sec->lines[i]];
        if (!(check->flags & ML_TWOSIDED))
            continue;

        if (sides[check->side[0]].sector == secnum)
            other = &sectors[sides[check->side[1]].sector];
        else
            other = &sectors[sides[check->side[0]].sector];

        if (other->ceilingheight < height)
            height = other->ceilingheight;
    }
    return height;
}

void P_MoveThings(int sector, int ymove)
{
    thing_t* t = sectors[sector].things;

    while (t)
    {
        t->z += ymove;
        t = t->next;
    }
}

//==================================================================
//
//	RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
//==================================================================
int P_FindSectorFromLineTag(line_t* line, int start)
{
    int i;
    int secnum = -1;

    for (i=start+1;i<numsectors;i++)
        if (sectors[i].tag == line->tag)
            return i;
    
    return -1;
}

//==================================================================
//
//	Find minimum light from an adjacent sector
//
//==================================================================
int P_FindMinSurroundingLight(sector_t* sector, int max)
{
    int i, min, side;
    fixed_t height;
    sector_t* check;

    min = max;
    for (i = 0; i < sector->linecount; i++)
    {
        if (lines[sector->lines[i]].flags & ML_TWOSIDED)
        {
            if (sides[lines[sector->lines[i]].side[0]].sector == (sector - sectors))
                side = 1;
            else
                side = 0;

            check = &sectors[sides[lines[sector->lines[i]].side[side]].sector];
            if (check->lightlevel < min)
                min = check->lightlevel;

            break; //No wonder ghidra was getting so confused by this function. Why the early return?
        }
    }

    return min;
}

extern int flatstartlump;
void EV_GoopChange(line_t* line, int side)
{
    int i;
    short floor;

    if (side == 0)
    {
        floor = (short)W_GetNumForName("SEWAGE1");
    }
    else
    {
        floor = (short)W_GetNumForName("NUKAGE1");
    }
    floor = floor - (short)flatstartlump;

    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == line->tag)
        {
            sectors[i].floortexture = floor;
        }
    }
    return;
}

void T_MoveFloor(floormove_t* floor)
{
    if (floor->direction == 1)
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
    else if (floor->direction == -1)
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
    short texture;
    short newsecnum;
    int ok;
    floormove_t* floor;
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
        texture = sec->floortexture;

        do
        {
            ok = 0;
            i = 0;

            while (i < sec->linecount)
            {
                if (((lines[sec->lines[i]].flags & ML_TWOSIDED) != 0) && (secnum == (int)sides[lines[sec->lines[i]].side[0]].sector))
                {
                    newsecnum = sides[lines[sec->lines[i]].side[1]].sector;
                    sec = &sectors[newsecnum];
                    if (sec->floortexture != texture) break;
                    height += (8 << FRACBITS);

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
                        secnum = (int)newsecnum;
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
            sec->specialdata = (void*)floor;
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
            sector->specialdata = (void*)floor;
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
    if (door->direction == -1)
    {
        if ((door->sector - sectors) == player->r->sector)
        {
            door->direction = 0;
            door->topcountdown = 10;
        }
        else
        {
            door->sector->ceilingheight -= door->speed;
            if (door->type == normal)
            {
                if (door->sector->ceilingheight <= door->sector->floorheight)
                {
                    door->sector->ceilingheight = door->sector->floorheight;
                    door->sector->specialdata = 0;
                    P_RemoveThinker((thinker_t*)door);
                }
            }
            else if (door->type == close30ThenOpen)
            {
                if (door->sector->ceilingheight <= door->sector->floorheight)
                {
                    door->sector->ceilingheight = door->sector->floorheight;
                    door->direction = 0;
                    door->topcountdown = 2100;
                }
            }
            else if (door->type == close)
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
    else if (door->direction == 0)
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
    else if (door->direction == 1)
    {
        door->sector->ceilingheight += door->speed;
        if (door->type == 0)
        {
            if (door->topheight <= door->sector->ceilingheight)
            {
                door->sector->ceilingheight = door->topheight;
                door->direction = 0;
                door->topcountdown = door->topwait;
            }
        }
        else if (door->type < 2)
        {
            if (door->topheight <= door->sector->ceilingheight)
            {
                door->sector->ceilingheight = door->topheight;
                door->sector->specialdata = 0;
                P_RemoveThinker((thinker_t*)door);
            }
        }
        else if ((door->type == 3))
        {
            if ((door->topheight <= door->sector->ceilingheight))
            {
                door->sector->ceilingheight = door->topheight;
                door->sector->specialdata = 0;
                P_RemoveThinker((thinker_t*)door);
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
    short secnum;
    vldoor_t* door;
    sector_t* sec;

    if (line->special == 26)
    {
        if (!player->items[it_bluecard])
            return;
    }
    else if (line->special == 27)
    {
        if (!player->items[it_yellowcard])
            return;
    }
    else if (line->special == 28)
    {
        if (!player->items[it_redcard])
            return;
    }

    secnum = sides[line->side[side ^ 1]].sector;
    sec = sectors + (int)secnum;
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
        door->topheight = P_FindLowestCeilingSurrounding(sec, secnum);
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
    if (plat->status == up)
    {
        plat->sector->floorheight = plat->sector->floorheight + plat->speed;
        P_MoveThings(plat->sector - sectors, plat->speed);
        if (plat->high < plat->sector->floorheight)
        {
            P_MoveThings(plat->sector - sectors, plat->high - plat->sector->floorheight);
            plat->sector->floorheight = plat->high;
            plat->count = plat->wait;
            plat->status = waiting;

            if (plat->type == downWaitUpStay)
            {
                plat->sector->specialdata = 0;
                P_RemoveThinker((thinker_t*)plat);
            }
            else if (plat->type == raiseAndChange)
            {
                plat->sector->specialdata = 0;
                P_RemoveThinker((thinker_t*)plat);
            }
        }
    }
    else if (plat->status == down)
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
    else if ((plat->status == waiting))
    {
        plat->count--;
        if (plat->count == 0)
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
}

int EV_RaisePlatUpToNearestFloorAndChange(line_t* line, int side)
{
    plat_t* plat;
    sector_t* sec;
    int secnum;
    int rtn = 0;

    secnum = -1;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata == 0)
        {
            plat = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
            P_AddThinker((thinker_t*)plat);
            plat->speed = PLATSPEED / 2;
            sec->floortexture = sectors[sides[line->side[side]].sector].floortexture;
            plat->high = P_FindNextHighestFloor(sec, secnum, sec->floorheight);
            plat->wait = 0;
            plat->sector = sec;
            plat->status = up;
            plat->thinker.function = &T_PlatRaise;
            plat->type = raiseAndChange;
            plat->sector->specialdata = (void*)plat;
            rtn = 1;
        }
    }
    return rtn;
}

int EV_PlatRaiseAndChange(line_t* line, int amount)
{
    plat_t* plat;
    sector_t* sec;
    int rtn;
    int secnum;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata == 0)
        {
            rtn = 1;
            plat = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
            P_AddThinker((thinker_t*)plat);
            plat->speed = PLATSPEED / 2;
            sec->floortexture = sectors[sides[line->side[0]].sector].floortexture;
            plat->high = sec->floorheight + amount * FRACUNIT;
            plat->wait = 0;
            plat->sector = sec;
            plat->status = up;
            plat->thinker.function = &T_PlatRaise;
            plat->type = raiseAndChange;
            plat->sector->specialdata = (void*)plat;
        }
    }
    return rtn;
}

int EV_PlatLowerWaitRaiseStay(line_t* line)
{
    plat_t* plat;
    sector_t* sector;
    int secnum;
    int ret = 0;

    secnum = -1;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sector = &sectors[secnum];
        if (sector->specialdata == 0)
        {
            plat = (plat_t*)Z_Malloc(playzone, sizeof(plat_t));
            P_AddThinker((thinker_t*)plat);
            plat->speed = PLATSPEED;
            plat->low = P_FindLowestFloorSurrounding(sector, secnum);
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
    short val;
    short min;
    int i, j;

    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == line->tag)
        {
            min = sectors[i].lightlevel;

            for (j = 0; j < sector->linecount; j++)
            {
                if ((lines[sector->lines[j]].flags & ML_TWOSIDED) != 0)
                {
                    if ((int)sides[lines[sector->lines[j]].side[1]].sector == i)
                    {
                        val = sectors[sides[lines[sector->lines[j]].side[0]].sector].lightlevel;
                    }
                    else
                    {
                        val = sectors[sides[lines[sector->lines[j]].side[1]].sector].lightlevel;
                    }
                    if (val < min)
                    {
                        min = val;
                    }
                }
            }
            sectors[i].lightlevel = min;
        }
    }
}

void EV_LightTurnOn(line_t* line, int bright)
{
    short val;
    int j;
    int i;

    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == line->tag)
        {
            if (bright == 0)
            {
                for (j = 0; j < sectors[i].linecount; j++)
                {
                    if ((lines[sectors[i].lines[j]].flags & 4) == 0)
                    {
                        if (sides[lines[sectors[i].lines[j]].side[0]].sector == i)
                        {
                            val = sectors[sides[lines[sectors[i].lines[j]].side[1]].sector].lightlevel;
                        }
                        else
                        {
                            val = sectors[sides[lines[sectors[i].lines[j]].side[0]].sector].lightlevel;
                        }
                        if (bright < val)
                        {
                            bright = val;
                        }
                    }
                }
            }
            sectors[i].lightlevel = (short)bright;
        }
    }
}

void EV_StartLightStrobing(line_t* line);

/*
==============================================================================

                            EVENTS

Events are operations triggered by using, crossing, or shooting special lines, or by timed thinkers

==============================================================================
*/



/*
===============================================================================
=
= P_PlayerCrossSpecialLine - TRIGGER
=
= Called every ticcount a thing origin is about to cross
= a line with a non 0 special
=
===============================================================================
*/

void P_PlayerCrossSpecialLine(line_t* line)
{
    switch (line->special)
    {
    case 2: // Open Door
        EV_DoDoor(line, open);
        break;
    case 3: // Close Door
        EV_DoDoor(line, close);
        break;
    case 4: // Raise Door
        EV_DoDoor(line, normal);
        break;
    case 5: // Raise Floor
        EV_RaiseFloor(line);
        break;
    case 10: // PlatDownWaitUp
        EV_PlatLowerWaitRaiseStay(line);
        break;
    case 12: // Light Turn On - brightest near
        EV_LightTurnOn(line, 0);
        break;
    case 13: // Light Turn On 255
        EV_LightTurnOn(line, 255);
        break;
    case 16: // Close Door 30
        EV_DoDoor(line, close30ThenOpen);
        break;
    case 17: // Start Light Strobing
        EV_StartLightStrobing(line);
        break;
    case 19: // Lower Floor
        EV_LowerFloor(line);
        break;
    case 22: // Raise floor to nearest height and change texture
        EV_RaisePlatUpToNearestFloorAndChange(line, sides[line->side[0]].sector != player->r->sector);
        break;
    case 104: // Turn lights off in sector(tag)
        EV_TurnTagLIghtsOff(line);
        break;
    case 1000: // EXIT!
        gameaction = ga_completed;
        break;
    }
}

int P_FindTexture(char* string)
{
    int i;

    for (i = 0; i < numtextures; i++)
    {
        if (!strcmp(texturelookup[i]->name, string))
            return i;
    }

    IO_Error("P_InitSwitchList: Couldn't find %s!", string);
}

void P_InitSwitchList(void)
{
    int i;

    for (i = 0; i < MAXSWITCHES; i++)
    {
        if (alphSwitchList[i] == NULL) 
        {
            numswitches = i;
            switchlist[i] = -1;
            return;
        }

        switchlist[i] = P_FindTexture(alphSwitchList[i]);
    }
}

void P_ChangeSwitchTexture(line_t* line, int useAgain)
{
    int i;

    if (useAgain == 0)
    {
        line->special = 0;
    }
    i = 0;
    while (1)
    {
        if (i >= numswitches)
        {
            IO_Error("P_ChangeSwitchTexture: Couldn't find a matching switch!");
            return;
        }
        if (switchlist[i] == (int)sides[line->side[0]].toptexture)
        {
            sides[line->side[0]].toptexture = switchlist[i ^ 1];
            return;
        }
        if (switchlist[i] == (int)sides[line->side[0]].midtexture)
        {
            sides[line->side[0]].midtexture = switchlist[i ^ 1];
            return;
        }
        if (switchlist[i] == (int)sides[line->side[0]].bottomtexture) break;
        i++;
    }
    sides[line->side[0]].bottomtexture = switchlist[i ^ 1];
    return;
}

/*
==============================================================================
=
= P_PlayerUseSpecialLine
=
= Called when a thing uses a special line
= Only the front sides of lines are usable
===============================================================================
*/

void P_PlayerUseSpecialLine(line_t* line, int side)
{
    short uVar1;
    int iVar2;

    if (side != 0)
    {
        return;
    }

    switch (line->special)
    {
        //===============================================
        //	MANUALS
        //===============================================
    case 1:			// Vertical Door
    case 26:		// Blue Door/Locked
    case 27:		// Yellow Door /Locked
    case 28:		// Red Door /Locked
        EV_VerticalDoor(line, 0);
        break;

        //===============================================
        //	SWITCHES
        //===============================================
    case 7: // Switch_Build_Stairs (8 pixel steps)
        if (EV_BuildStairs(line))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 11:		// Exit level
        IO_ClearKeys();
        gameaction = ga_completed; 
        break;
    case 14:		// Raise Floor 32 and change texture
        if (EV_PlatRaiseAndChange(line, 32))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 15:		// Raise Floor 24 and change texture
        if (EV_PlatRaiseAndChange(line, 24))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 18:		// Raise Floor to next highest floor
        if (EV_RaiseFloorUpToNearest(line))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 20:		// Raise Plat next highest floor and change texture
        if (EV_RaisePlatUpToNearestFloorAndChange(line, 0))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 21:		// PlatDownWaitUpStay
        if (EV_PlatLowerWaitRaiseStay(line))
        {
            P_ChangeSwitchTexture(line, 1);
        }
        break;
    case 29:		// Raise Door
        if (EV_DoDoor(line, normal))
        {
            P_ChangeSwitchTexture(line, 1);
        }
        break;
    case 101:		// Raise Floor
        if (EV_RaiseFloor(line))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 102:		// Lower Floor to Surrounding floor height
        if (EV_LowerFloor(line))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    case 103:		// Open Door
        if (EV_DoDoor(line, open))
        {
            P_ChangeSwitchTexture(line, 0);
        }
        break;
    }

    return;
}

//----------------------------------------------------------------------------
//
// P_PlayerShootSpecialLine
//
// Called when a thing shoots a special line.
//
//----------------------------------------------------------------------------

void P_PlayerShootSpecialLine(line_t* line)
{
    //it is once again, but a dream
}

//----------------------------------------------------------------------------
//
// P_PlayerInSpecialSector
//
// Called every tic frame that the player origin is in a special sector.
//
//----------------------------------------------------------------------------

void P_PlayerInSpecialSector(void)
{
    switch (sectors[player->r->sector].special)
    {
    case 7: // Damage_Sludge
        if (D_Rnd() < 8)
        {
            P_DamagePlayer(playernum, 1);
        }
        break;
    default:
        IO_Error("P_PlayerInSpecialSector: unknown special %i", (int)sectors[player->r->sector].special);
    }
}

void P_AnimatePlanePics()
{
    anim_t* anim;
    int ticcount;

    ticcount = IO_GetTime();
    anim = anims;
    while (anim < lastAnim)
    {
        flatlookup[anim->picnum] = lumpinfo[flatstartlump + anim->basepic + ((ticcount / anim->speed) % anim->numpics)].position;
        anim++;
    }
}

void P_StartupPicAnims(void)
{
    int i;

    lastAnim = anims;
    for (i = 0; flatAnims[i].numflats != 0; i++)
    {
        lastAnim->picnum = W_GetNumForName((char*)(flatAnims + i)) - flatstartlump;
        lastAnim->basepic = W_GetNumForName(flatAnims[i].endname) - flatstartlump;
        lastAnim->numpics = flatAnims[i].numflats;
        lastAnim->speed = flatAnims[i].speed;
        lastAnim++;

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
    lightflash_t* flash;

    sector->special = 0;
    flash = (lightflash_t*)Z_Malloc(playzone, sizeof(lightflash_t));
    P_AddThinker((thinker_t*)flash);
    flash->thinker.function = &T_LightFlash;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, (int)sector->lightlevel);
    flash->maxtime = 64;
    flash->mintime = 7;
    flash->count = (flash->maxtime & D_Rnd()) + 1;
    return;
}

void T_StrobeFlash(lightstrobe_t* flash)
{
    flash->count--;
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

    strobe = (lightstrobe_t*)Z_Malloc(playzone, sizeof(lightstrobe_t));
    P_AddThinker((thinker_t*)strobe);
    strobe->sector = sector;
    strobe->darktime = fastOrSlow;
    strobe->brighttime = STROBEBRIGHT;
    strobe->thinker.function = &T_StrobeFlash;
    strobe->minlight = (int)sector->lightlevel;
    strobe->maxlight = P_FindMinSurroundingLight(sector, (int)sector->lightlevel);
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
    int secnum;

    secnum = -1;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        if (sectors[secnum].specialdata == 0)
        {
            P_SpawnStrobeFlash(&sectors[secnum], secnum, SLOWDARK);
        }
    }
    return;
}

/*
==============================================================================

                            SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecialSectors
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/

void P_SpawnSpecialSectors(void)
{
    int i;
    sector_t* sector;

    //
    //	Init special SECTORs
    //
    sector = sectors;
    for (i = 0; i < numsectors; i++)
    {
        if (sector->special != 0)
        {
            switch (sector->special)
            {
            case 1:		// FLICKERING LIGHTS
                P_SpawnLightFlash(sector);
                break;
            case 2:		// STROBE FAST
                P_SpawnStrobeFlash(sector, i, FASTDARK);
                break;
            case 3:		// STROBE SLOW
                P_SpawnStrobeFlash(sector, i, SLOWDARK);
                break;
            case 4:     // PLAT RAISE AND LOWER
                P_SpawnPlatRaise(sector, i);
            }
        }
        sector++;
    }
    return;
}
