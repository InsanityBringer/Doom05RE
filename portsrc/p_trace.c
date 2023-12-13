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
#include "r_local.h"

#define MAXCROSSINGS 32

typedef struct
{
    int z;
    thing_t* thing;
    line_t* line;
    int side;
} crossing_t;

crossing_t crossings[MAXCROSSINGS];
crossing_t* crossing_p;

int P_PointOnLineSide(int x, int y, line_t* line)
{
    int calcy;

    switch (line->slopetype)
    {
    case ls_horizontal:

        if (y == line->bbox[2])
        {
            return -1;
        }
        else
        {
            return (points[line->p1].x < points[line->p2].x != y < line->bbox[2]);
        }

    case ls_vertical:
        if (x == line->bbox[3])
        {
            return -1;
        }
        else
        {
            return (points[line->p1].y < points[line->p2].y != line->bbox[3] < x);
        }

    case ls_slope:
        calcy = line->yintercept + FixedMul(line->slope, x);
        if (calcy == y)
        {
            return -1;
        }
        else
        {
            return (points[line->p1].x < points[line->p2].x != y < calcy);
        }
        break;

    default:
        return 0;
    }
}

void P_TransformVertex(point_t* source, vertex_t* dest)
{
    fixed_t gxt;
    fixed_t gyt;

    gxt = source->x - viewx;
    gyt = source->y - viewy;
    dest->tx = -FixedMul(gyt, viewcos) + FixedMul(gxt, viewsin);
    dest->tz = (FixedMul(gxt, viewcos) + FixedMul(gyt, viewsin));
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
}

void P_CrossSectorLines(sector_t* sector, boolean checkall)
{
    fixed_t midz;
    line_t* line;
    int side;
    int i;

    vertex_t mv1, mv2;

    i = validcheck;
    if (sector->validcheck != validcheck)
    {
        sector->validcheck = validcheck;
        if (checkall != 0)
        {
            P_CrossSectorThings(sector);
        }

        for (i = 0; i < sector->linecount; i++)
        {
            line = &lines[sector->lines[i]];
            if (((checkall != 0) || (line->flags & ML_TWOSIDED)) && (line->validcheck != validcheck))
            {
                line->validcheck = validcheck;
                P_TransformVertex(&points[line->p1], &mv1);
                P_TransformVertex(&points[line->p2], &mv2);
                if (((int)mv1.tx < 1) && (0 < mv2.tx))
                {
                    midz = FixedDiv(-mv1.tx, mv2.tx - mv1.tx);
                    side = 0;
                }
                else
                {
                    if (((int)mv1.tx < 1) || ((0 < mv2.tx || (!(line->flags & ML_TWOSIDED)))))
                        continue;

                    midz = FixedDiv(mv1.tx, mv1.tx - mv2.tx);
                    side = 1;
                }
                midz = mv1.tz + FixedMul(mv2.tz - mv1.tz, midz);

                if (-1 < midz)
                {
                    crossing_p->z = midz;
                    crossing_p->thing = (thing_t*)NULL;
                    crossing_p->line = line;
                    crossing_p->side = side;
                    crossing_p++;
                }
            }
        }
    }
}

int P_CrossSectorBounds(int sector, fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    fixed_t deltax, deltay;
    int i, j, num;
    crossing_t temp, *test, *crossingstart;
    fixed_t distance;
    point_t p1;
    vertex_t mv1;

    if ((x1 == x2) && (y1 == y2))
        return sector;

    viewx = x1;
    viewy = y1;
    deltax = x2 - x1;
    deltay = y2 - y1;
    distance = abs(deltax) + abs(deltay);
    viewsin = FixedDiv(deltay, distance);
    viewcos = FixedDiv(deltax, distance);

    p1.x = x2;
    p1.y = y2;
    P_TransformVertex(&p1, &mv1);
    distance = mv1.tz;

    validcheck++;
    crossing_p = crossings;
    test = crossing_p;

    again:
    P_CrossSectorLines(&sectors[sector], false);
    num = crossing_p - test;

    for (i = 0; i < num - 1; i++)
    {
        for (j = 0; j < (num - 1) - i; j++)
        {
            if (test[j + 1].z < test[j].z)
            {
                temp = test[j];
                test[j] = test[j + 1];
                test[j + 1] = temp;
            }
        }
    }

    //That weird code structure from P_PlayerShoot strikes again.
    for (; test < crossing_p; )
    {
        if (test->z > distance)
        {
            return sector;
        }
        else
        {
            sector = sides[test->line->side[test->side ^ 1]].sector;
            test++;
            if (sectors[sector].validcheck != validcheck)
            {
                if (test[-1].line->special) 
                    P_PlayerCrossSpecialLine(test[-1].line);
                goto again;
            }
        }
    }

    return sector;
}

void P_PlayerShootWall(player_t* player, line_t* line)
{
    //printf("hit wall %d\n", line - &lines[0]);
}

boolean P_PlayerShootThing(player_t* player, thing_t* ithing)
{
    int damage;

    if (ithing->flags & TF_PLAYER)
    {
        P_DamagePlayer((player_t*)ithing->specialdata - playerobjs, 5);
        return true;
    }
    else
    {
        switch (player->readyweapon)
        {
        case wp_knife:
            damage = (D_Rnd() & 3) + 1;
            break;
        case wp_rifle:
        case wp_auto:
        case wp_chainsaw:
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
            break;
        default:
            IO_Error("P_PlayerShootThing: bad weapon number");
            break;
        }
        P_DamageEnemy((actor_t*)ithing->specialdata, damage);

        return true;
    }
}

void P_PlayerShoot(void)
{
    int i, j, num;
    crossing_t temp, *test, *crossingstart;
    int sector;

    viewx = player->r->x;
    viewy = player->r->y;
    viewcos = cosines[player->r->angle];
    viewsin = sines[player->r->angle];
    validcheck++;
    crossing_p = crossings;
    test = crossing_p;
    sector = player->r->sector;

    do
    {
        again:
        crossingstart = crossing_p;
        P_CrossSectorLines(&sectors[sector], true);
        num = crossing_p - test;

        for (i = 0; i < num - 1; i++)
        {
            for (j = 0; j < (num - 1) - i; j++)
            {
                if (test[j + 1].z < test[j].z)
                {
                    temp = test[j];
                    test[j] = test[j + 1];
                    test[j + 1] = temp;
                }
            }
        }

        //Handle all crossings detected.
        // I'm not 100% sure on the goto and for loop here, but this matches the unoptimized ASM most closely. 
        // I need to see how this function gets compiled. 
        // The for loop is here because the code is structured the way Watcom C compiled unoptimized for loops.
        // But why is the increment not part of the loop?
        for ( ; test < crossing_p; ) 
        {
            //BUG: since test isn't incremented if P_PlayerShootThing returns false,
            // this will infinite loop if P_PlayerShootThing returns false, but 
            // P_PlayerShootThing always returns true so this cannot be observed. 
            if (test->thing) 
            {
                if (P_PlayerShootThing(player, test->thing))
                    return;
            }
            else if (!(test->line->flags & ML_TWOSIDED))
            {
                P_PlayerShootWall(player, test->line);
                return;
            }
            else
            {
                sector = sides[test->line->side[test->side ^ 1]].sector;
                test++;
                if (sectors[sector].validcheck != validcheck)
                    goto again; 
            }
        }

        IO_Error("P_PlayerShoot: ran out of crossings"); //if IO_Error were to return, it would jump to the start of the outer do loop. 
    } while (1);
}

int numaudareas;

void P_RecursiveAudConnect(sector_t* sector)
{
    sector_t* newsector;
    int i;

    sector->audarea = numaudareas;
    sector->validcheck = validcheck;
    //Note: This doesn't actually work, since the sector adjacency information is never computed. 
    for (i = 0; i < sector->numadjacentsectors; i++)
    {
        newsector = sector->adjacentsectors[i];
        if (((newsector->validcheck != validcheck) &&
            (newsector->floorheight <= sector->ceilingheight)) &&
            (sector->floorheight <= newsector->ceilingheight))
        {
            P_RecursiveAudConnect(newsector);
        }
    }
}

void P_FixAudareas(void)
{
    sector_t* sector;
    int i;
    numaudareas = 0;

    validcheck++;

    sector = sectors;
    for (i = 0; i < numsectors; i++)
    {
        if (sector->validcheck != validcheck)
        {
            if (sector->floorheight == sector->ceilingheight)
            {
                sector->audarea = -1;
            }
            else
            {
                P_RecursiveAudConnect(sector);
                numaudareas++;
            }
        }
        sector++;
    }
}
