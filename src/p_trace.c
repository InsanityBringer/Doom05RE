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

typedef struct 
{
    int z;
    thing_t* thing;
    line_t* line;
    int side;
} crossing_t;

crossing_t crossings[32];
crossing_t* crossing_p;

int P_PointOnLineSide(int x, int y, line_t* line)
{
    int uVar1;
    int iVar3;
    int local_c;

    uVar1 = line->slopetype;
    if (uVar1 == ls_horziontal) 
    {
        if (y == line->bbox[2]) 
        {
            local_c = -1;
        }
        else 
        {
            local_c = (points[line->p1].x < points[line->p2].x != y < line->bbox[2]);
        }
    }
    else if (uVar1 == ls_vertical) 
    {
        if (x == line->bbox[3])
        {
            local_c = -1;
        }
        else
        {
            local_c = (points[line->p1].y < points[line->p2].y != line->bbox[3] < x);
        }
    }
    else 
    {
        if (uVar1 == ls_slope) 
        {
            iVar3 = line->yintercept + FixedMul(line->slope, x);
            if (iVar3 == y) 
            {
                local_c = -1;
            }
            else 
            {
                local_c = (points[line->p1].x < points[line->p2].x != y < iVar3);
            }
        }
        else 
        {
            local_c = 0;
        }
    }
    return local_c;
}

void P_TransformVertex(point_t* source, vertex_t* dest)
{
    int iVar3;
    int iVar4;

    iVar3 = source->x - viewx;
    iVar4 = source->y - viewy;
    dest->tx = -FixedMul(iVar4, viewcos) + FixedMul(iVar3, viewsin);
    dest->tz = (FixedMul(iVar3, viewcos) + FixedMul(iVar4, viewsin));
    return;
}

void P_CrossSectorThings(sector_t* sector)
{
    thing_t* thing;
    point_t p1;
    vertex_t mv1;

    thing = sector->things;
    while (thing != (thing_t*)NULL) 
    {
        if (thing->validcheck != validcheck) 
        {
            thing->validcheck = validcheck;
            p1.x = thing->x;
            p1.y = thing->y;
            P_TransformVertex(&p1, &mv1);
            if (((0 < mv1.tz) && (-0x140001 < mv1.tx)) && (mv1.tx < 0x140001)) 
            {
                crossing_p->z = mv1.tz;
                crossing_p->thing = thing;
                crossing_p->line = (line_t*)NULL;
                crossing_p++;
            }
        }
        thing = thing->next;
    }
    return;
}

void P_CrossSectorLines(sector_t* sector, int checkall)
{
    int local_2c;
    sector_t* local_28;
    int local_24;
    line_t* local_20;
    int local_1c;
    int i;

    vertex_t mv1, mv2;

    i = validcheck;
    if (sector->validcheck != validcheck) 
    {
        sector->validcheck = validcheck;
        local_28 = sector;
        local_24 = checkall;
        if (checkall != 0) 
        {
            P_CrossSectorThings(sector, (thing_t*)i);
        }
        i = 0;
        while (i < local_28->linecount) 
        {
            local_20 = &lines[local_28->lines[i]];
            if (((local_24 != 0) || (local_20->flags & ML_TWOSIDED)) && (local_20->validcheck != validcheck)) 
            {
                local_20->validcheck = validcheck;
                P_TransformVertex(&points[local_20->p1], &mv1);
                P_TransformVertex(&points[local_20->p2], &mv2);
                if (((int)mv1.tx < 1) && (0 < mv2.tx)) 
                {
                    local_2c = FixedDiv(-mv1.tx, mv2.tx - mv1.tx);
                    local_1c = 0;
                }
                else 
                {
                    if (((int)mv1.tx < 1) || ((0 < mv2.tx || (!(local_20->flags & ML_TWOSIDED)))))
                        goto LAB_000235c5; //TODO: Untangle me

                    local_2c = FixedDiv(mv1.tx, mv1.tx - mv2.tx);
                    local_1c = 1;
                }
                local_2c = mv1.tz + FixedMul(mv2.tz - mv1.tz, local_2c);

                if (-1 < local_2c) 
                {
                    crossing_p->z = local_2c;
                    crossing_p->thing = (thing_t*)NULL;
                    crossing_p->line = local_20;
                    crossing_p->side = local_1c;
                    crossing_p++;
                }
            }
        LAB_000235c5:
            i++;
        }
    }
    return;
}

int P_CrossSectorBounds(int sector, fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    int deltax;
    int deltay;
    int notdeltay;
    int local_20;
    int extraout_EDX_00;
    int* puVar1;
    int* puVar2;
    int* puVar3;
    int* puVar4;
    uint8_t bVar5;
    int distance;
    int iVar6;
    crossing_t* crossingstart;
    crossing_t* pcVar7;
    crossing_t temp;

    point_t p1;
    vertex_t mv1;

    int j;
    int notdeltax;

    bVar5 = 0;

    if ((x1 == x2) && (y1 == y2))
    {
        return sector;
    }
    deltax = x2 - x1;
    deltay = y2 - y1;
    viewy = y1;
    viewx = x1;
    notdeltax = abs(deltax);
    notdeltay = abs(deltay);
    viewsin = FixedDiv(deltay, notdeltax + notdeltay);
    viewcos = FixedDiv(deltax, notdeltax + notdeltay);

    p1.x = x2;
    p1.y = y2;
    P_TransformVertex(&p1, &mv1);
    distance = mv1.tz;

    validcheck++;
    crossing_p = &crossings[0];
    crossingstart = crossings;
    do {
        P_CrossSectorLines(&sectors[sector], 0);
        local_20 = crossing_p - crossingstart;
        notdeltay = 0;
        while (notdeltay < local_20 + -1)
        {
            j = 0;
            while (j < (local_20 + -1) - notdeltay) 
            {
                if (crossingstart[j + 1].z < crossingstart[j].z)
                {
                    temp = crossingstart[j];
                    crossingstart[j] = crossingstart[j + 1];
                    crossingstart[j + 1] = temp;
                }
                j++;
            }
            notdeltay++;
        }
        do
        {
            pcVar7 = crossingstart;
            if (crossing_p <= pcVar7) 
            {
                return sector;
            }
            if (distance < pcVar7->z)
            {
                return sector;
            }
            sector = (sector_t*)(int)sides[pcVar7->line->side[pcVar7->side ^ 1]].sector;
            crossingstart = pcVar7 + 1;
        } while (sectors[(int)sector].validcheck == validcheck);
        if (pcVar7->line->special != 0) 
        {
            P_PlayerCrossSpecialLine(pcVar7->line);
        }
    } while (1);
}

void P_PlayerShootWall(player_t* player, line_t* line)
{
    //printf("hit wall %d\n", line - &lines[0]);
    return;
}

int P_PlayerShootThing(player_t* player, thing_t* ithing)
{
    thing_t* local_20;
    int damage;

    if (!(ithing->flags & 4)) 
    {
        local_20 = player;
        if (player->readyweapon < 8)
        {
            switch (player->readyweapon)
            {
            case wp_knife:
                damage = (D_Rnd() & 3) + 1;
                break;
            default:
                damage = (D_Rnd() & 3) * 2 + 2;
                break;
            case wp_shotgun:
            case wp_claw:
                damage = (D_Rnd() & 3) * 4 + 4;
                break;
            case wp_missile:
                damage = (D_Rnd() & 3) * 8 + 8;
                break;
            case wp_bfg:
                damage = (D_Rnd() & 3) * 32 + 32;
            }
        }
        else 
        {
            IO_Error("P_PlayerShootThing: bad weapon number");
        }
        P_DamageEnemy((actor_t*)ithing->specialdata, damage);
    }
    else 
    {
        P_DamagePlayer((player_t*)ithing->specialdata - playerobjs, 5);
    }
    return 1;
}

void P_PlayerShoot(void)
{
    int iVar1;
    crossing_t temp;
    crossing_t* crossingstart;
    int sector;
    int num;
    int j;
    int i;

    //bVar8 = 0;
    viewx = player->r->x;
    viewy = player->r->y;
    viewcos = cosines[player->r->angle];
    viewsin = sines[player->r->angle];
    validcheck++;
    crossing_p = crossings;
    crossingstart = &crossings[0];
    sector = player->r->sector;

    do 
    {
        //local_3c.side = (int)crossing_p; //TODO: decompiler mess, try to verify if needed
        P_CrossSectorLines(&sectors[sector], 1);
        num = crossing_p - crossingstart;
        i = 0;

        //sort crossings
        while (i < num - 1) 
        {
            j = 0;
            while (j < (num - 1) - i)
            {
                if (crossingstart[j + 1].z < crossingstart[j].z) 
                {
                    temp = crossingstart[j];
                    crossingstart[j] = crossingstart[j + 1];
                    crossingstart[j + 1] = temp;
                }
                j = j + 1;
            }
            i = i + 1;
        }
        do 
        {
            while (1) 
            {
                if (crossing_p <= crossingstart) 
                {
                    IO_Error("P_PlayerShoot: ran out of crossings\n");
                }
                if (crossingstart->thing == (thing_t*)NULL) break;
                iVar1 = P_PlayerShootThing(player, crossingstart->thing);
                if (iVar1 != 0) 
                {
                    return;
                }
            }
            if (!(crossingstart->line->flags & ML_TWOSIDED)) 
            {
                P_PlayerShootWall(player, crossingstart->line);
                return;
            }
            sector = (int)sides[crossingstart->line->side[crossingstart->side ^ 1]].sector;
            crossingstart++;
        } while (sectors[sector].validcheck == validcheck);
    } while (1);
}
