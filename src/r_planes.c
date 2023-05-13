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
    unsigned int uVar1;

    uVar1 = viewangle + 0x400U & 0x1fff;
    xscale = FixedDiv(viewsin, xproject) << 10;
    yscale = FixedDiv(viewcos, xproject) << 10;
    zerocosine = cosines[uVar1];
    zerosine = sines[uVar1];
    return;
}

void R_DrawPlanes(void)
{
    int* piVar1;
    int uVar2;
    fixed_t local_28;
    fixed_t local_24;
    fixed_t local_20;
    int* local_34;
    int* local_30;

    mr_picture = flatlookup[sector->ceilingtexture];
    uVar2 = viewz - sector->ceilingheight;
    if ((-0x10000 < (int)uVar2) && ((int)uVar2 < 0x10000))
    {
        uVar2 = 0x10000;
    }
    mr_y = 0;
    while (mr_y < viewheight) 
    {
        if (mr_y == centery) 
        {
            mr_picture = flatlookup[sector->floortexture];
            uVar2 = viewz - sector->floorheight;
            if ((-0x10000 < (int)uVar2) && ((int)uVar2 < 0x10000))
            {
                uVar2 = 0x10000;
            }
        }
        local_30 = startspans[mr_y];
        local_34 = spans[mr_y];

        if (local_30 != local_34)
        {
            if (((int)((int)local_34 - (int)local_30) >> 2 & 1U) != 0) 
            {
                IO_Error("R_DrawSectorPlanes: odd number of spans");
            }

            local_28 = FixedDiv(uVar2, yslope[mr_y]);
            local_24 = FixedDiv(local_28, cos45);
            local_20 = R_LightFromZ(local_28);
            //[ISB] clamp, since this can error if your head goes in the ceiling. Which happens a lot in the alpha versions.
            if (local_20 < 0) local_20 = 0;
            mr_colormap = esectorscalelight[local_20];

            mr_xstep = FixedMul(local_28, xscale);
            mr_ystep = FixedMul(local_28, yscale);
            local_20 = (FixedMul(local_24, zerocosine) + viewx) << 10;
            local_24 = -(FixedMul(local_24, zerosine) + viewy) << 10;

            do
            {
                piVar1 = local_30 + 1;
                mr_x1 = *local_30;
                local_30 = local_30 + 2;
                mr_x2 = *piVar1;
                mr_xfrac = local_20 + mr_xstep * mr_x1;
                mr_yfrac = local_24 + mr_ystep * mr_x1;
                R_MapRow();
            } while (local_30 < local_34);
        }
        mr_y++;
    }
    return;
}
