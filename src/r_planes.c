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
#include "r_ref.h"
#include "r_local.h"

fixed_t xscale, yscale;
fixed_t* inscale, *outscale;

fixed_t zerocosine, zerosine;

int* spans[832];
int* startspans[832];

int spanlists[13312];

void R_PostSpans(int oldtop, int oldbottom, int newtop, int newbottom, int x)
{
    if (oldtop < -1 || newtop < -1 || oldbottom > viewheight || newbottom > viewheight)
        IO_Error("R_PostSpans: bad range");

    if (oldbottom - oldtop <= 1)
    {
        while (1)
        {
            newtop++;
            if (newtop >= newbottom)
                break;

            *spans[newtop]++ = x;
        }
    }
    else
    {
        if (newbottom - newtop <= 1)
        {
            while (1)
            {
                oldtop++;
                if (oldtop >= oldbottom)
                    break;

                *spans[oldtop]++ = x;
            }
        }
        else
        {
            if (oldtop == newtop)
            {
                if (oldbottom < newbottom)
                {
                    do
                    {
                        newbottom--;
                        *spans[newbottom]++ = x;
                    } while (newbottom > oldbottom);
                }
                else
                {
                    while (oldbottom > newbottom)
                    {
                        oldbottom--;
                        *spans[oldbottom]++ = x;
                    }
                }
            }
            else
            {
                if (oldbottom == newbottom)
                {
                    if (oldtop > newtop)
                    {
                        do
                        {
                            *spans[oldtop--]++ = x;
                        } while (oldtop > newtop);
                    }
                    else
                    {
                        do
                        {
                            *spans[newtop--]++ = x;
                        } while (newtop > oldtop);
                    }
                }
                else
                {
                    if (oldtop > newtop)
                    {
                        if (newbottom <= oldtop)
                        {
                            while (1)
                            {
                                newtop++;
                                if (newtop >= newbottom)
                                    break;
                                *spans[newtop]++ = x;
                            }

                            while (1)
                            {
                                oldtop++;
                                if (oldtop >= oldbottom)
                                    break;
                                *spans[oldtop]++ = x;
                            }
                        }
                        else
                        {
                            if (newbottom < oldbottom)
                            {
                                while (1)
                                {
                                    newtop++;
                                    if (newtop > oldtop)
                                        break;
                                    *spans[newtop]++ = x;
                                }

                                while (1)
                                {
                                    oldbottom--;
                                    if (oldbottom < newbottom)
                                        break;
                                    *spans[oldbottom]++ = x;
                                }
                            }
                            else
                            {
                                while (1)
                                {
                                    newtop++;
                                    if (newtop > oldtop)
                                        break;
                                    *spans[newtop]++ = x;
                                }

                                while (1)
                                {
                                    newbottom--;
                                    if (newbottom < oldbottom)
                                        break;
                                    *spans[newbottom]++ = x;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (newtop >= oldbottom)
                        {
                            while (1)
                            {
                                oldtop++;
                                if (oldtop >= oldbottom)
                                    break;
                                *spans[oldtop]++ = x;
                            }

                            while (1)
                            {
                                newtop++;
                                if (newtop >= newbottom)
                                    break;
                                *spans[newtop]++ = x;
                            }
                        }
                        else
                        {
                            if (newbottom > oldbottom)
                            {
                                while (1)
                                {
                                    oldtop++;
                                    if (oldtop > newtop)
                                        break;
                                    *spans[oldtop]++ = x;
                                }

                                while (1)
                                {
                                    newbottom--;
                                    if (newbottom < oldbottom)
                                        break;
                                    *spans[newbottom]++ = x;
                                }
                            }
                            else
                            {
                                while (1)
                                {
                                    oldtop++;
                                    if (oldtop > newtop)
                                        break;
                                    *spans[oldtop]++ = x;
                                }

                                while (1)
                                {
                                    oldbottom--;
                                    if (oldbottom < newbottom)
                                        break;
                                    *spans[oldbottom]++ = x;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void R_GenerateSpans(void)
{
    int x;

    memcpy(spans, startspans, sizeof(int) * 832);
    x = sectorxl;
    R_PostSpans(-1, 0, ceilingpixel[x], newceiling[x], x);
    R_PostSpans(-1, 0, newfloor[x], floorpixel[x], x);
    while (++x <= sectorxh)
    {
        if (x == 124) 
            x = 124;
        
        R_PostSpans(ceilingpixel[x - 1], newceiling[x - 1], ceilingpixel[x], newceiling[x], x);
        R_PostSpans(newfloor[x - 1], floorpixel[x - 1], newfloor[x], floorpixel[x], x);
    }
    R_PostSpans(ceilingpixel[x - 1], newceiling[x - 1], -1, 0, x);
    R_PostSpans(newfloor[x - 1], floorpixel[x - 1], -1, 0, x);
}


void R_PrepPlanes(void)
{
    int angle = viewangle + 1024 & 8191;
    zerocosine = cosines[angle];
    zerosine = sines[angle];
    xscale = FixedDiv(viewsin, xproject) << 10;
    yscale = FixedDiv(viewcos, xproject) << 10;
}

void R_DrawPlanes(void)
{
    fixed_t deltaheight;
    fixed_t zerox, zeroy;
    fixed_t pointz, length;
    int* span_p, * stop;

    mr_picture = flatlookup[sector->ceilingtexture];
    deltaheight = viewz - sector->ceilingheight;
    if (deltaheight > -FRACUNIT && deltaheight < FRACUNIT)
        deltaheight = FRACUNIT;

    for (mr_y = 0; mr_y < viewheight; mr_y++)
    {
        if (mr_y == centery)
        {
            mr_picture = flatlookup[sector->floortexture];
            deltaheight = viewz - sector->floorheight;
            if (deltaheight > -FRACUNIT && deltaheight < FRACUNIT)
                deltaheight = FRACUNIT;

        }
        span_p = startspans[mr_y];
        stop = spans[mr_y];

        if (span_p != stop)
        {
            if (((int)(stop - span_p) & 1) != 0)
                IO_Error("R_DrawSectorPlanes: odd number of spans");

            pointz = FixedDiv(deltaheight, yslope[mr_y]);
            length = FixedDiv(pointz, cos45);

            mr_colormap = esectorscalelight[R_LightFromZ(pointz)];

            mr_xstep = FixedMul(pointz, xscale);
            mr_ystep = FixedMul(pointz, yscale);
            zerox = (FixedMul(length, zerocosine) + viewx) << 10;
            zeroy = -(FixedMul(length, zerosine) + viewy) << 10;

            do
            {
                mr_x1 = *span_p++;
                mr_x2 = *span_p++;
                mr_xfrac = zerox + mr_xstep * mr_x1;
                mr_yfrac = zeroy + mr_ystep * mr_x1;
                R_MapRow();
            } while (span_p < stop);
        }
    }
}

