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
#include "r_local.h"

fixed_t xscale, yscale;
fixed_t* inscale, *outscale;

fixed_t zerocosine, zerosine;

int* spans[832];
int* startspans[832];

int spanlists[13312];

void R_PostSpans(int oldtop, int oldbottom, int newtop, int newbottom, int x)
{
    int* piVar1;
    int local_1c;
    int local_18;
    int iVar2;

    local_1c = oldtop;
    local_18 = oldbottom;
    iVar2 = newbottom;
    if ((((oldtop < -1) || (newtop < -1)) || (viewheight < oldbottom)) || (viewheight < newtop))
    {
        IO_Error("R_PostSpans: bad range\n");
    }
    if (local_18 - local_1c < 2) 
    {
        while (newtop = newtop + 1, newtop < iVar2) 
        {
            piVar1 = spans[newtop];
            spans[newtop] = spans[newtop] + 1;
            *piVar1 = x;
        }
    }
    else 
    {
        if (iVar2 - newtop < 2) 
        {
            while (local_1c = local_1c + 1, local_1c < local_18) 
            {
                piVar1 = spans[local_1c];
                spans[local_1c] = spans[local_1c] + 1;
                *piVar1 = x;
            }
        }
        else 
        {
            if (local_1c == newtop) 
            {
                if (local_18 < iVar2) 
                {
                    do 
                    {
                        iVar2 = iVar2 + -1;
                        piVar1 = spans[iVar2];
                        spans[iVar2] = spans[iVar2] + 1;
                        *piVar1 = x;
                    } while (local_18 < iVar2);
                }
                else 
                {
                    while (iVar2 < local_18)
                    {
                        local_18 = local_18 + -1;
                        piVar1 = spans[local_18];
                        spans[local_18] = spans[local_18] + 1;
                        *piVar1 = x;
                    }
                }
            }
            else 
            {
                if (local_18 == iVar2) 
                {
                    if (newtop < local_1c) 
                    {
                        do 
                        {
                            local_18 = local_1c + -1;
                            piVar1 = spans[local_1c];
                            spans[local_1c] = spans[local_1c] + 1;
                            *piVar1 = x;
                            local_1c = local_18;
                        } while (newtop < local_18);
                    }
                    else
                    {
                        do 
                        {
                            local_18 = newtop + -1;
                            piVar1 = spans[newtop];
                            spans[newtop] = spans[newtop] + 1;
                            *piVar1 = x;
                            newtop = local_18;
                        } while (local_1c < local_18);
                    }
                }
                else 
                {
                    if (newtop < local_1c) 
                    {
                        if (local_1c < iVar2) 
                        {
                            if (iVar2 < local_18) 
                            {
                                while (newtop = newtop + 1, newtop <= local_1c) 
                                {
                                    piVar1 = spans[newtop];
                                    spans[newtop] = spans[newtop] + 1;
                                    *piVar1 = x;
                                }
                                while (local_18 = local_18 + -1, iVar2 <= local_18) 
                                {
                                    piVar1 = spans[local_18];
                                    spans[local_18] = spans[local_18] + 1;
                                    *piVar1 = x;
                                }
                            }
                            else 
                            {
                                while (newtop = newtop + 1, newtop <= local_1c)
                                {
                                    piVar1 = spans[newtop];
                                    spans[newtop] = spans[newtop] + 1;
                                    *piVar1 = x;
                                }
                                while (iVar2 = iVar2 + -1, local_18 <= iVar2) 
                                {
                                    piVar1 = spans[iVar2];
                                    spans[iVar2] = spans[iVar2] + 1;
                                    *piVar1 = x;
                                }
                            }
                        }
                        else
                        {
                            while (newtop = newtop + 1, newtop < iVar2) 
                            {
                                piVar1 = spans[newtop];
                                spans[newtop] = spans[newtop] + 1;
                                *piVar1 = x;
                            }
                            while (local_1c = local_1c + 1, local_1c < local_18) 
                            {
                                piVar1 = spans[local_1c];
                                spans[local_1c] = spans[local_1c] + 1;
                                *piVar1 = x;
                            }
                        }
                    }
                    else 
                    {
                        if (newtop < local_18)
                        {
                            if (local_18 < iVar2)
                            {
                                while (local_1c = local_1c + 1, local_1c <= newtop) 
                                {
                                    piVar1 = spans[local_1c];
                                    spans[local_1c] = spans[local_1c] + 1;
                                    *piVar1 = x;
                                }
                                while (iVar2 = iVar2 + -1, local_18 <= iVar2) 
                                {
                                    piVar1 = spans[iVar2];
                                    spans[iVar2] = spans[iVar2] + 1;
                                    *piVar1 = x;
                                }
                            }
                            else 
                            {
                                while (local_1c = local_1c + 1, local_1c <= newtop) 
                                {
                                    piVar1 = spans[local_1c];
                                    spans[local_1c] = spans[local_1c] + 1;
                                    *piVar1 = x;
                                }
                                while (local_18 = local_18 + -1, iVar2 <= local_18)
                                {
                                    piVar1 = spans[local_18];
                                    spans[local_18] = spans[local_18] + 1;
                                    *piVar1 = x;
                                }
                            }
                        }
                        else 
                        {
                            while (local_1c = local_1c + 1, local_1c < local_18)
                            {
                                piVar1 = spans[local_1c];
                                spans[local_1c] = spans[local_1c] + 1;
                                *piVar1 = x;
                            }
                            while (newtop = newtop + 1, newtop < iVar2)
                            {
                                piVar1 = spans[newtop];
                                spans[newtop] = spans[newtop] + 1;
                                *piVar1 = x;
                            }
                        }
                    }
                }
            }
        }
    }
    return;
}

void R_GenerateSpans(void)
{
    int x;
    int x_00;

    memcpy(spans, startspans, sizeof(int) * 832);
    x = sectorxl;
    R_PostSpans(-1, 0, ceilingpixel[sectorxl], newceiling[sectorxl], sectorxl);
    R_PostSpans(-1, 0, newfloor[x], floorpixel[x], x);
    while (x_00 = x + 1, x_00 <= sectorxh)
    {
        if (x_00 == 0x7c) 
        {
            x_00 = 0x7c;
        }
        R_PostSpans(ceilingpixel[x_00 + -1], newceiling[x_00 + -1], ceilingpixel[x_00], newceiling[x_00], x_00);
        R_PostSpans(newfloor[x_00 + -1], floorpixel[x_00 + -1], newfloor[x_00], floorpixel[x_00], x_00);
        x = x_00;
    }
    R_PostSpans(ceilingpixel[x], newceiling[x], -1, 0, x_00);
    R_PostSpans(newfloor[x_00 + -1], floorpixel[x_00 + -1], -1, 0, x_00);
    return;
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
                ((void(*)())R_MapRow)();
            } while (local_30 < local_34);
        }
        mr_y++;
    }
    return;
}
