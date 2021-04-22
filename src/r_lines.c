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

int collumn;

int toppixel, bottompixel, highpixel, lowpixel;

fixed_t scalestep;
fixed_t collumnstep;

fixed_t highscreenstep, lowscreenstep, topscreenstep, bottomscreenstep;

fixed_t scale;

fixed_t highscreen, lowscreen, topscreen, bottomscreen;

int newfseg;
forwardseg_t* wallfseg;
int newprocline;
int extralight;

drange_t vwalldrange;

int* walllight;

procline_t proclines[256];
forwardseg_t forwardsegs[128];

int dodiag;

void R_SetLineTypeAndBox(int linenum)
{
	unsigned int uVar2;
	line_t* plVar3;
	point_t* ppVar4;
	point_t* ppVar5;

	plVar3 = &lines[linenum];
	ppVar4 = &points[plVar3->p1];
	ppVar5 = &points[plVar3->p2];
	if (ppVar4->x == ppVar5->x) 
	{
		plVar3->slopetype = ls_vertical;
	}
	else 
	{
		if (ppVar4->y == ppVar5->y) 
		{
			plVar3->slopetype = ls_horziontal;
		}
		else
		{
			plVar3->slopetype = ls_slope;
			uVar2 = ppVar5->y - ppVar4->y;
			plVar3->slope = FixedDiv(uVar2, ppVar5->x - ppVar4->x);
			plVar3->yintercept = ppVar4->y - FixedMul(ppVar4->x, plVar3->slope);
		}
	}
	if (ppVar4->x < ppVar5->x) 
	{
		plVar3->bbox[1] = ppVar4->x;
		plVar3->bbox[3] = ppVar5->x;
	}
	else 
	{
		plVar3->bbox[1] = ppVar5->x;
		plVar3->bbox[3] = ppVar4->x;
	}
	if (ppVar4->y < ppVar5->y) 
	{
		plVar3->bbox[0] = ppVar4->y;
		plVar3->bbox[2] = ppVar5->y;
	}
	else 
	{
		plVar3->bbox[0] = ppVar5->y;
		plVar3->bbox[2] = ppVar4->y;
	}
}

void R_ClipToRight(vertex_t* v1, vertex_t* v2)
{
    int iVar1;
    long long lVar2;
    int iVar3;
    int iVar4;
    unsigned int uVar5;
    fixed_t frac;

    iVar3 = v2->tx - v1->tx;
    iVar4 = v2->tz - v1->tz;
    iVar1 = v1->texture;
    uVar5 = v1->tz - v1->tx;
    frac = FixedDiv(uVar5, (iVar3 - iVar4));
    v2->tz = v1->tz + FixedMul(iVar4, frac);
    v2->tx = v1->tx + FixedMul(iVar3, frac);
    v2->texture = v1->texture + FixedMul(v2->texture - iVar1, frac);
    v2->ipx = viewwidth;

    return;
}

void R_ClipToLeft(vertex_t* v1, vertex_t* v2)
{
    int iVar1;
    int iVar3;
    int iVar4;
    unsigned int uVar5;
    fixed_t frac;

    iVar3 = v2->tx - v1->tx;
    iVar4 = v2->tz - v1->tz;
    iVar1 = v1->texture;
    uVar5 = v1->tz + v1->tx;
    frac = FixedDiv(uVar5, (-iVar3 - iVar4));
    v2->tz = v1->tz + FixedMul(iVar4, frac);
    v2->tx = v1->tx + FixedMul(iVar3, frac);
    v2->texture = v1->texture + FixedMul(v2->texture - iVar1, frac);
    v2->ipx = 0;
    return;
}

void R_ClipToNearest(vertex_t* v1, vertex_t* v2)
{
    int iVar1;
    int iVar3;
    int iVar4;
    int uVar5;
    fixed_t iVar6;
    fixed_t local_2c;

    iVar3 = v2->tx - v1->tx;
    iVar4 = v2->tz - v1->tz;
    iVar1 = v1->texture;
    uVar5 = v1->tz + v1->tx;
    iVar6 = FixedDiv(uVar5, -iVar3 - iVar4);
    uVar5 = v1->tz - v1->tx;
    local_2c = FixedDiv(uVar5, iVar3 - iVar4);
    if (iVar6 < local_2c)
    {
        v2->ipx = 0;
        local_2c = iVar6;
    }
    else 
    {
        v2->ipx = viewwidth;
    }

    v2->tz = v1->tz + FixedMul(iVar4, local_2c);
    v2->tx = v1->tx + FixedMul(iVar3, local_2c);
    v2->texture = v1->texture + FixedMul(v2->texture - iVar1, local_2c);

    return;
}

void R_SetPassRange(int top, int bottom)
{
    int local_1c;
    int iVar1;

    local_1c = top;
    if (top <= ceilingpixel[sp_x])
    {
        local_1c = ceilingpixel[sp_x] + 1;
    }
    iVar1 = bottom;
    if (floorpixel[sp_x] <= bottom)
    {
        iVar1 = floorpixel[sp_x] - 1;
    }
    if (iVar1 < local_1c)
    {
        passfloor[sp_x] = floorpixel[sp_x];
        passceiling[sp_x] = floorpixel[sp_x] - 1;
    }
    else
    {
        passfloor[sp_x] = iVar1 + 1;
        passceiling[sp_x] = local_1c - 1;
    }
    return;
}

void R_SetWallRange(int top, int bottom)
{
    int local_1c;
    int iVar1;

    local_1c = top;
    if (top <= ceilingpixel[sp_x])
    {
        local_1c = ceilingpixel[sp_x] + 1;
    }
    iVar1 = bottom;
    if (floorpixel[sp_x] <= bottom)
    {
        iVar1 = floorpixel[sp_x] + -1;
    }
    if (iVar1 < local_1c)
    {
        newfloor[sp_x] = floorpixel[sp_x];
        newceiling[sp_x] = floorpixel[sp_x];
    }
    else
    {
        newfloor[sp_x] = iVar1;
        newceiling[sp_x] = local_1c;
    }
    return;
}

unsigned int R_SetClipRange(int top, int bottom)
{
    sp_y1 = top;
    if (top <= ceilingpixel[sp_x])
    {
        sp_y1 = ceilingpixel[sp_x] + 1;
    }
    sp_y2 = bottom;
    if (floorpixel[sp_x] <= bottom)
    {
        sp_y2 = floorpixel[sp_x] + -1;
    }
    return (sp_y1 <= sp_y2);
}

void R_DrawEndSeg(void)
{
    unsigned int uVar2;
    int iVar3;

    do 
    {
        toppixel = topscreen + 0xffff >> FRACBITS;
        bottompixel = bottomscreen - 1 >> FRACBITS;
        R_SetWallRange(toppixel, bottompixel);
        uVar2 = R_SetClipRange(toppixel, bottompixel);
        if (uVar2) 
        {
            iVar3 = R_LightFromVScale(scale);
            sp_colormap = walllight[iVar3];
            sp_fracstep = FixedDiv(FRACUNIT, scale);
            iVar3 = R_CacheColumn(wallfseg->texture, FixedMul(collumn, sp_fracstep) >> FRACBITS);
            sp_source = (uint8_t*)(iVar3 + 2);
            sp_frac = (wallfseg->toptextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - topscreen, sp_fracstep);

            ((void(*)())R_RawScale)();
        }
        outscale[sp_x] = scale;
        scale += scalestep;
        collumn += collumnstep;
        topscreen += topscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
    return;
}

void R_DrawTopSeg(void)
{
    unsigned int uVar2;
    int iVar3;

    while (sp_x <= vwalldrange.xh)
    {
        toppixel = topscreen + 0xffff >> FRACBITS;
        highpixel = (highscreen - 1) >> FRACBITS;
        bottompixel = (bottomscreen - 1) >> FRACBITS;
        if (bottompixel< highpixel) 
        {
            highpixel = bottompixel;
        }
        R_SetPassRange(highpixel + 1, bottompixel);
        R_SetWallRange(toppixel, bottompixel);
        uVar2 = R_SetClipRange(toppixel, highpixel);
        if (uVar2) 
        {
            iVar3 = R_LightFromVScale(scale);
            sp_colormap = walllight[iVar3];
            sp_fracstep = FixedDiv(FRACUNIT, scale);

            iVar3 = R_CacheColumn(wallfseg->texture, FixedMul(collumn, sp_fracstep) >> FRACBITS);
            sp_source = (uint8_t*)(iVar3 + 2);

            sp_frac = (wallfseg->toptextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - topscreen, sp_fracstep);
            ((void(*)())R_RawScale)();
        }
        outscale[sp_x] = scale;
        scale += scalestep;
        collumn += collumnstep;
        topscreen += topscreenstep;
        highscreen += highscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } 
    return;
}

void R_DrawBottomSeg(void)
{
    unsigned int uVar2;
    int iVar3;

    do 
    {
        toppixel = topscreen + 0xffff >> FRACBITS;
        lowpixel = lowscreen + 0xffff >> FRACBITS;
        bottompixel = bottomscreen - 1 >> FRACBITS;
        if (sp_y1 < toppixel)
        {
            sp_y1 = toppixel;
        }
        R_SetPassRange(toppixel, lowpixel + -1);
        R_SetWallRange(toppixel, bottompixel);
        uVar2 = R_SetClipRange(lowpixel, bottompixel);
        if (uVar2) 
        {
            iVar3 = R_LightFromVScale(scale);
            sp_colormap = walllight[iVar3];
            sp_fracstep = FixedDiv(FRACUNIT, scale);
            iVar3 = R_CacheColumn(wallfseg->bottomtexture, FixedMul(collumn, sp_fracstep) >> FRACBITS);

            sp_source = (uint8_t*)(iVar3 + 2);

            sp_frac = (wallfseg->bottomtextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - lowscreen, sp_fracstep);
            ((void(*)())R_RawScale)();
        }
        outscale[sp_x] = scale;
        scale = scale + scalestep;
        collumn = collumn + collumnstep;
        topscreen = topscreen + topscreenstep;
        lowscreen = lowscreen + lowscreenstep;
        bottomscreen = bottomscreen + bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
    return;
}

void R_DrawTopBottomSeg(void)
{
    long long lVar1;
    int local_20;
    unsigned int uVar2;
    int iVar3;

    while (sp_x <= vwalldrange.xh)
    {
        local_20 = R_LightFromVScale(scale);
        sp_colormap = walllight[local_20];
        sp_fracstep = FixedDiv(FRACUNIT, scale);
        local_20 = FixedMul(collumn, sp_fracstep) >> FRACBITS;

        toppixel = (topscreen + 0xffff) >> FRACBITS;
        highpixel = (highscreen - 1) >> FRACBITS;
        lowpixel = (lowscreen + 0xffff) >> FRACBITS;
        bottompixel = (bottomscreen - 1) >> FRACBITS;

        R_SetPassRange(highpixel + 1, lowpixel - 1);
        R_SetWallRange(toppixel, bottompixel);
        uVar2 = R_SetClipRange(toppixel, highpixel);
        if (uVar2)
        {
            iVar3 = R_CacheColumn(wallfseg->texture, local_20);
            sp_source = (uint8_t*)(iVar3 + 2);

            sp_frac = (wallfseg->toptextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - topscreen, sp_fracstep);
            //sp_source = colormaps;
            ((void(*)())R_RawScale)();
        }
        uVar2 = R_SetClipRange(lowpixel, bottompixel);
        if (uVar2) 
        {
            local_20 = R_CacheColumn(wallfseg->bottomtexture, local_20);
            sp_source = (uint8_t*)(local_20 + 2);

            sp_frac = (wallfseg->bottomtextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - lowscreen, sp_fracstep);
            ((void(*)())R_RawScale)();
        }
        outscale[sp_x] = scale;
        scale += scalestep;
        collumn += collumnstep;
        topscreen += topscreenstep;
        highscreen += highscreenstep;
        lowscreen += lowscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } 
    return;
}

void R_DrawEmptySeg(void)
{
    do 
    {
        toppixel = topscreen + 0xffff >> FRACBITS;
        bottompixel = bottomscreen + -1 >> FRACBITS;
        R_SetPassRange(toppixel, bottompixel);
        R_SetWallRange(toppixel, bottompixel);
        outscale[sp_x] = scale;
        scale += scalestep;
        topscreen += topscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
    return;
}

void R_ClearProclines(void)
{
	newprocline = 0;
    memset(proclines, 0, sizeof(proclines));
}

void R_ClearFsegs(void)
{
	newfseg = 0;
}

int R_NewProcline(void)
{
    int iVar1;

    if (newprocline == 256) 
    {
        IO_Error("R_NewProcline: no free spots\n");
    }

    return newprocline++;
}

int R_NewFseg(void)
{
    int iVar1;

    if (newfseg == 128) 
    {
        IO_Error("R_NewFseg: no free spots\n");
    }
    iVar1 = newfseg;
    newfseg++;
    return iVar1;
}

extern int debugsec;

void R_MakeProcline(int line)
{
    long long lVar1;
    int iVar2;
    vertex_t* pvVar3;
    fixed_t* pfVar4;
    vertex_t* pvVar5;
    uint8_t bVar6;
    unsigned int local_88;
    fixed_t afStack128[7];
    vertex_t mv1, mv2, mvtemp;
    int local_2c;
    int local_28;
    int local_20;
    line_t* line_p;

    bVar6 = 0;

    line_p = &lines[line];
    line_p->validcheck = validcheck;
    line_p->procline = -1;
    R_TransformVertex((vertex_t*)&points[line_p->p1], &mv1);
    R_TransformVertex((vertex_t*)&points[line_p->p2], &mv2);

    mv1.texture = 0;
    local_20 = (int)line_p->length << FRACBITS;
    mv2.texture = local_20;
    switch (mv2.flags + mv1.flags * 4) 
    {
    case 0:
        break;
    case 1:
        R_ClipToRight(&mv1, &mv2);
        break;
    case 2:
        if (!(line_p->flags & ML_TWOSIDED))
        {
            return;
        }
        R_ClipToLeft(&mv1, &mv2);
        break;
    case 3:
        R_ClipToNearest(&mv1, &mv2);
        break;
    case 4:
        if (!(line_p->flags & ML_TWOSIDED)) 
        {
            return;
        }
        R_ClipToRight(&mv2, &mv1);
        break;
    case 5:
    case 7:
    case 10:
    case 0xb:
    case 0xd:
    case 0xe:
    case 0xf:
        return;
    case 6:
        if (!(line_p->flags & ML_TWOSIDED))
        {
            return;
        }
        if ((mv1.tz < 0) && (mv2.tz < 0)) 
        {
            return;
        }
        R_ClipToRight(&mv2, &mv1);
        R_ClipToLeft(&mv1, &mv2);
        break;
    case 8:
        R_ClipToLeft(&mv2, &mv1);
        break;
    case 9:
        if ((mv1.tz < 0) && (mv2.tz < 0)) 
        {
            return;
        }
        R_ClipToLeft(&mv2, &mv1);
        R_ClipToRight(&mv1, &mv2);
        break;
    case 0xc:
        R_ClipToNearest(&mv2, &mv1);
    }
    if ((-1 < mv1.tz) && (-1 < mv2.tz))
    {
        if (mv1.tz < 0x40000)
        {
            mv1.tz = 0x40000;
        }
        mv1.xscale = FixedDiv(xproject, mv1.tz);
        mv1.yscale = FixedDiv(yproject, mv1.tz);
        if (mv1.flags == 0) 
        {
            mv1.ipx = centerx + ((FixedMul(mv1.tx, mv1.xscale) + 0x8000) >> FRACBITS);
        }

        if (mv2.tz < 0x40000) 
        {
            mv2.tz = 0x40000;
        }

        mv2.xscale = FixedDiv(xproject, mv2.tz);
        mv2.yscale = FixedDiv(yproject, mv2.tz);
        if (mv2.flags == 0)
        {
            mv2.ipx = centerx + ((FixedMul(mv2.tx, mv2.xscale) + 0x8000) >> FRACBITS);
        }
        if (mv1.ipx != mv2.ipx) 
        {
            local_28 = line;
            if (mv2.ipx < mv1.ipx)
            {
                if (!(line_p->flags & ML_TWOSIDED)) 
                {
                    return;
                }
                local_2c = 1;
                
                mvtemp = mv1;
                mv1 = mv2;
                mv2 = mvtemp;

                mv1.texture = local_20 - mv1.texture;
                mv2.texture = local_20 - mv2.texture;
            }
            else
            {
                local_2c = 0;
            }
            if ((0 < mv2.ipx) && (mv1.ipx < viewwidth))
            {
                iVar2 = R_NewProcline();
                line_p->procline = iVar2;
                iVar2 = line_p->procline;
                proclines[iVar2].side = local_2c;
                proclines[iVar2].ipx1 = mv1.ipx;
                proclines[iVar2].ipx2 = mv2.ipx;
                proclines[iVar2].texture1 = mv1.texture;
                proclines[iVar2].texture2 = mv2.texture;
                proclines[iVar2].scale1 = mv1.yscale;
                proclines[iVar2].scale2 = mv2.yscale;
                proclines[iVar2].scalestep =
                    (mv2.yscale - mv1.yscale) / (mv2.ipx - mv1.ipx) + 1;
                proclines[iVar2].line = local_28;
                proclines[iVar2].chained = 0;
                proclines[iVar2].debug = 0;
                if (!(line_p->flags & ML_TWOSIDED)) 
                {
                    proclines[iVar2].sector = -1;
                }
                else 
                {
                    local_88 = (unsigned int)(local_2c == 0);
                    proclines[iVar2].sector = (int)sides[line_p->side[local_88]].sector;
                }
                proclines[iVar2].seg = -1;

                if (debugsec == 57)
                {
                    printf("\t\tpost clipping: (%d) to (%d) scaling: %f to %f (step: %f), newprocline %d\n", mv1.ipx, mv2.ipx, proclines[iVar2].scale1 / (float)FRACUNIT, proclines[iVar2].scale2 / (float)FRACUNIT, proclines[iVar2].scalestep / (float)FRACUNIT, newprocline);
                }
            }
        }
    }

    return;
}

void R_MakeFseg(int line)
{
    long long lVar1;
    int iVar2;
    fixed_t iVar3;
    int local_30;
    int uVar4;
    fixed_t iVar5;
    int local_50;
    sector_t* local_4c;
    side_t* local_2c;
    side_t* local_28;
    forwardseg_t* local_24;
    procline_t* local_20;
    line_t* plVar6;

    plVar6 = &lines[line];
    local_20 = &proclines[plVar6->procline];
    local_28 = &sides[plVar6->side[proclines[plVar6->procline].side]];
    local_50 = line;
    iVar2 = R_NewFseg();
    local_20->seg = iVar2;
    iVar2 = local_20->seg;
    local_24 = &forwardsegs[iVar2];
    iVar3 = (int)local_28->firstcollumn * FRACUNIT;
    local_30 = local_20->ipx2 - local_20->ipx1;

    uVar4 = FixedMul(local_20->texture1 + iVar3, local_20->scale1);
    
    forwardsegs[iVar2].startcollumn = uVar4;
    forwardsegs[iVar2].collumnstep = (FixedMul(local_20->texture2 + iVar3, local_20->scale2) - uVar4) / local_30;
    iVar3 = viewz - sector->ceilingheight;
    
    iVar5 = centeryfrac + FixedMul(local_20->scale1, iVar3);

    iVar3 = centeryfrac + FixedMul(local_20->scale2, iVar3);

    forwardsegs[iVar2].starttopscreen = iVar5;
    forwardsegs[iVar2].topscreenstep = (iVar3 - iVar5) / local_30;
    iVar3 = viewz - sector->floorheight;
    iVar5 = centeryfrac + FixedMul(local_20->scale1, iVar3);

    iVar3 = centeryfrac + FixedMul(local_20->scale2, iVar3);

    forwardsegs[iVar2].startbottomscreen = iVar5;
    forwardsegs[iVar2].bottomscreenstep = (iVar3 - iVar5) / local_30 + -1;

    if (!(plVar6->flags & ML_TWOSIDED)) 
    {
        if (local_28->midtexture < 0)
        {
            IO_Error("R_MakeFseg: No mid texture on single sided line\n");
        }
        if (numtextures <= local_28->midtexture) 
        {
            IO_Error("R_MakeFseg: Bad mid texture number\n");
        }
        local_24->type = 1;
        local_24->texture = texturelookup[local_28->midtexture];
        if (!(plVar6->flags & ML_DONTPEGBOTTOM)) 
        {
            local_24->toptextureskip = 0;
        }
        else 
        {
            local_24->toptextureskip = 0x80 - (sector->ceilingheight - sector->floorheight >> 0x10 & 0x7fU);
        }
        if (!(plVar6->flags & ML_MAPPED))
        {
            plVar6->flags |= ML_MAPPED;
            R_DrawBlockLine(local_50, '\x10');
        }
    }
    else
    {
        local_24->type = 0;
        local_2c = &sides[plVar6->side[local_20->side ^ 1]];
        if (numsectors <= (int)local_2c->sector) 
        {
            IO_Error("R_MakeFseg: bad backends number\n");
        }
        local_4c = sectors + local_2c->sector;
        if (!((plVar6->flags & ML_MAPPED)) &&
            ((sector->ceilingheight != local_4c->ceilingheight ||
                (sector->floorheight != local_4c->floorheight)))) 
        {
            plVar6->flags |= ML_MAPPED;
            R_DrawBlockLine(local_50, '\x10');
        }
        if (local_4c->ceilingheight < sector->ceilingheight)
        {
            if (local_28->toptexture < 0)
            {
                IO_Error("R_Try add line: Exposed undefined top texture\n");
            }
            if (numtextures <= local_28->toptexture)
            {
                IO_Error("R_MakeFseg: Bad top texture number\n");
            }
            local_24->type |= 2;
            iVar2 = viewz - local_4c->ceilingheight;
            local_50 = centeryfrac + FixedMul(local_20->scale1, iVar2);

            iVar2 = centeryfrac + FixedMul(local_20->scale2, iVar2);

            local_24->starthighscreen = local_50;
            local_24->highscreenstep = (iVar2 - local_50) / local_30;
            local_24->texture = texturelookup[local_28->toptexture];
            if (!(plVar6->flags & ML_DONTPEGTOP)) 
            {
                local_24->toptextureskip = (int)local_24->texture->height - (sector->ceilingheight - local_4c->ceilingheight >> 0x10);
            }
            else
            {
                local_24->toptextureskip = 0;
            }
        }
        if (sector->floorheight < local_4c->floorheight) 
        {
            if (local_28->bottomtexture < 0) 
            {
                IO_Error("R_Try add line: Exposed undefined bottom texture\n");
            }
            if (numtextures <= local_28->bottomtexture) 
            {
                IO_Error("R_MakeFseg: Bad bottom texture number");
            }
            local_24->type |= 4;
            iVar2 = viewz - local_4c->floorheight;
            local_50 = centeryfrac + FixedMul(local_20->scale1, iVar2);

            iVar2 = centeryfrac + FixedMul(local_20->scale2, iVar2);

            local_24->startlowscreen = local_50;
            local_24->lowscreenstep = (iVar2 - local_50) / local_30 - 1;
            local_24->bottomtexture = texturelookup[local_28->bottomtexture];
            if (!(plVar6->flags & ML_DONTPEGBOTTOM)) 
            {
                local_24->bottomtextureskip = 0;
            }
            else 
            {
                local_24->bottomtextureskip = sector->ceilingheight - local_4c->floorheight >> 0x10 ;
            }
        }
    }
    return;
}

void R_DrawLineDrange(void)
{
    int uVar1;
    int iVar2;
    int iVar3;
    procline_t* local_20;

    if (numlines <= vwalldrange.number) 
    {
        IO_Error("R_DrawLineDrange: bad line number\n");
    }
    if (vwalldrange.xh < vwalldrange.xl)
    {
        IO_Error("R_DrawLineDrange: inverted range\n");
    }
    uVar1 = lines[vwalldrange.number].slopetype;
    if (uVar1 == 0) 
    {
        walllight = esectorscalelight2;
    }
    else if (uVar1 < 2)
    {
        walllight = esectorscalelight3;
    }
    else if (uVar1 == 2) 
    {
        walllight = esectorscalelight;
    }
    iVar2 = lines[vwalldrange.number].procline;

    local_20 = &proclines[iVar2];
    if (proclines[iVar2].seg == -1) 
    {
        R_MakeFseg(proclines[iVar2].line);
    }
    iVar2 = local_20->seg;
    wallfseg = &forwardsegs[iVar2];
    scalestep = local_20->scalestep;
    topscreenstep = forwardsegs[iVar2].topscreenstep;
    bottomscreenstep = forwardsegs[iVar2].bottomscreenstep;
    collumnstep = forwardsegs[iVar2].collumnstep;
    highscreenstep = forwardsegs[iVar2].highscreenstep;
    lowscreenstep = forwardsegs[iVar2].lowscreenstep;
    iVar3 = vwalldrange.xl - local_20->ipx1;
    if (iVar3 == 0) 
    {
        scale = local_20->scale1;
        topscreen = forwardsegs[iVar2].starttopscreen;
        bottomscreen = forwardsegs[iVar2].startbottomscreen;
        collumn = forwardsegs[iVar2].startcollumn;
        highscreen = forwardsegs[iVar2].starthighscreen;
        lowscreen = forwardsegs[iVar2].startlowscreen;
    }
    else
    {
        scale = iVar3 * scalestep + local_20->scale1;
        topscreen = forwardsegs[iVar2].starttopscreen + iVar3 * topscreenstep;
        bottomscreen = forwardsegs[iVar2].startbottomscreen + iVar3 * bottomscreenstep;
        if (wallfseg->type != 0)
        {
            collumn = forwardsegs[iVar2].startcollumn + iVar3 * collumnstep;
        }
        if ((wallfseg->type & 2)) 
        {
            highscreen = forwardsegs[iVar2].starthighscreen + iVar3 * highscreenstep;
        }
        if ((wallfseg->type & 4)) 
        {
            lowscreen = forwardsegs[iVar2].startlowscreen + iVar3 * lowscreenstep;
        }
    }
    sp_x = vwalldrange.xl;

    switch (wallfseg->type) 
    {
    case 0:
        R_DrawEmptySeg();
        break;
    case 1:
        R_DrawEndSeg();
        break;
    case 2:
        R_DrawTopSeg();
        break;
    case 4:
        R_DrawBottomSeg();
        break;
    case 6:
        R_DrawTopBottomSeg();
        break;
    default:
        IO_Error("R_DrawLineDrange: unknown fseg type\n");
        break;
    }
    return;
}

void R_DrawBlockLine(int linenum, uint8_t bits)
{
    short sVar1;
    short sVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    uint8_t* pbVar7;
    uint8_t bVar8;
    fixed_t local_3c;
    int local_38;
    int local_34;
    unsigned int local_28;
    uint8_t bVar9;
    point_t* local_20;
    point_t* local_1c;
    line_t* plVar10;

    bVar8 = 0;
    local_28 = linenum;
    bVar9 = bits;
    if (numlines <= linenum)
    {
        IO_Error("R_DrawBlockLine: bad line number\n");
    }
    plVar10 = &lines[local_28];
    local_1c = &points[plVar10->p1];
    local_20 = &points[plVar10->p2];
    local_3c = local_1c->x;
    //TODO: There is almost certainly a reason why this exists. Figure it out.
    //*(fixed_t*)(&stack0xffffffc8 + (uint)bVar8 * -8) = local_1c[(uint)bVar8 * 0x1fffffff].y;
    if (plVar10->length != 0)
    {
        iVar3 = local_20->x;
        iVar4 = local_1c->x;
        sVar1 = plVar10->length;
        iVar5 = local_20->y;
        iVar6 = local_1c->y;
        sVar2 = plVar10->length;
        local_34 = 0;

        local_3c = iVar4;
        local_38 = iVar6;
        while (local_34 <= plVar10->length)
        {
            pbVar7 = (uint8_t*)((local_38 - maporiginy >> 0x14) * mapwidth +
                (local_3c - maporiginx >> 0x14) + blockmap);
            *pbVar7 = *pbVar7 | bVar9;
            local_3c = local_3c + (iVar3 - iVar4) / (int)sVar1;
            local_38 = local_38 + (iVar5 - iVar6) / (int)sVar2;
            local_34 = local_34 + 1;
        }
    }
    return;
}
