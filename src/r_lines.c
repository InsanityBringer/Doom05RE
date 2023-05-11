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

procline_t proclines[MAXPROCLINES];
forwardseg_t forwardsegs[MAXFSEGS];

void R_SetPassRange(int top, int bottom)
{
    if (top <= ceilingpixel[sp_x])
        top = ceilingpixel[sp_x] + 1;
    
    if (floorpixel[sp_x] <= bottom)
        bottom = floorpixel[sp_x] - 1;
    
    if (bottom < top)
    {
        passfloor[sp_x] = floorpixel[sp_x];
        passceiling[sp_x] = floorpixel[sp_x] - 1;
    }
    else
    {
        passfloor[sp_x] = bottom + 1;
        passceiling[sp_x] = top - 1;
    }
}

void R_SetWallRange(int top, int bottom)
{
    if (top <= ceilingpixel[sp_x])
        top = ceilingpixel[sp_x] + 1;
    
    if (bottom >= floorpixel[sp_x])
        bottom = floorpixel[sp_x] + -1;
    
    if (bottom < top)
    {
        newfloor[sp_x] = floorpixel[sp_x];
        newceiling[sp_x] = floorpixel[sp_x];
    }
    else
    {
        newfloor[sp_x] = bottom;
        newceiling[sp_x] = top;
    }
}

int R_SetClipRange(int top, int bottom)
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
    byte* texcollumn;
    int texturecollumn;

    do
    {
        toppixel = topscreen + 0xffff >> FRACBITS;
        bottompixel = bottomscreen - 1 >> FRACBITS;
        R_SetWallRange(toppixel, bottompixel);

        if (R_SetClipRange(toppixel, bottompixel))
        {
            sp_colormap = walllight[R_LightFromVScale(scale)];
            sp_fracstep = FixedDiv(FRACUNIT, scale);
            texturecollumn = FixedMul(collumn, sp_fracstep) >> FRACBITS;
            texcollumn = R_CacheColumn(wallfseg->texture, texturecollumn);
            sp_source = texcollumn + 2;
            sp_frac = (wallfseg->toptextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - topscreen, sp_fracstep);

            R_RawScale();
        }
        outscale[sp_x] = scale;
        scale += scalestep;
        collumn += collumnstep;
        topscreen += topscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
}

void R_DrawTopSeg(void)
{
    byte* texcollumn;
    int texturecollumn;

    do
    {
        toppixel = topscreen + 0xffff >> FRACBITS;
        highpixel = (highscreen - 1) >> FRACBITS;
        bottompixel = (bottomscreen - 1) >> FRACBITS;
        if (bottompixel < highpixel)
        {
            highpixel = bottompixel;
        }
        R_SetPassRange(highpixel + 1, bottompixel);
        R_SetWallRange(toppixel, bottompixel);
        if (R_SetClipRange(toppixel, highpixel))
        {
            sp_colormap = walllight[R_LightFromVScale(scale)];
            sp_fracstep = FixedDiv(FRACUNIT, scale);
            texturecollumn = FixedMul(collumn, sp_fracstep) >> FRACBITS;
            texcollumn = R_CacheColumn(wallfseg->texture, texturecollumn);
            sp_source = texcollumn + 2;

            sp_frac = (wallfseg->toptextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - topscreen, sp_fracstep);
            R_RawScale();
        }
        outscale[sp_x] = scale;
        scale += scalestep;
        collumn += collumnstep;
        topscreen += topscreenstep;
        highscreen += highscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
}

void R_DrawBottomSeg(void)
{
    byte* texcollumn;
    int texturecollumn;

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
        if (R_SetClipRange(lowpixel, bottompixel))
        {
            sp_colormap = walllight[R_LightFromVScale(scale)];
            sp_fracstep = FixedDiv(FRACUNIT, scale);
            texturecollumn = FixedMul(collumn, sp_fracstep) >> FRACBITS;
            texcollumn = R_CacheColumn(wallfseg->bottomtexture, texturecollumn);

            sp_source = texcollumn + 2;

            sp_frac = (wallfseg->bottomtextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - lowscreen, sp_fracstep);
            R_RawScale();
        }
        outscale[sp_x] = scale;
        scale = scale + scalestep;
        collumn = collumn + collumnstep;
        topscreen = topscreen + topscreenstep;
        lowscreen = lowscreen + lowscreenstep;
        bottomscreen = bottomscreen + bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
}

void R_DrawTopBottomSeg(void)
{
    byte* texcollumn;
    int texturecollumn;

    do
    {
        sp_colormap = walllight[R_LightFromVScale(scale)];
        sp_fracstep = FixedDiv(FRACUNIT, scale);
        texturecollumn = FixedMul(collumn, sp_fracstep) >> FRACBITS;

        toppixel = (topscreen + 0xffff) >> FRACBITS;
        highpixel = (highscreen - 1) >> FRACBITS;
        lowpixel = (lowscreen + 0xffff) >> FRACBITS;
        bottompixel = (bottomscreen - 1) >> FRACBITS;

        R_SetPassRange(highpixel + 1, lowpixel - 1);
        R_SetWallRange(toppixel, bottompixel);
        if (R_SetClipRange(toppixel, highpixel))
        {
            texcollumn = R_CacheColumn(wallfseg->texture, texturecollumn);
            sp_source = texcollumn + 2;

            sp_frac = (wallfseg->toptextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - topscreen, sp_fracstep);
            R_RawScale();
        }
        if (R_SetClipRange(lowpixel, bottompixel))
        {
            texcollumn = R_CacheColumn(wallfseg->bottomtexture, texturecollumn);
            sp_source = texcollumn + 2;

            sp_frac = (wallfseg->bottomtextureskip << FRACBITS) + FixedMul((sp_y1 << FRACBITS) - lowscreen, sp_fracstep);
            R_RawScale();
        }
        outscale[sp_x] = scale;
        scale += scalestep;
        collumn += collumnstep;
        topscreen += topscreenstep;
        highscreen += highscreenstep;
        lowscreen += lowscreenstep;
        bottomscreen += bottomscreenstep;
        sp_x++;
    } while (sp_x <= vwalldrange.xh);
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
}

void R_SetLineTypeAndBox(int linenum)
{
    point_t* p1, * p2;
    line_t* line;

    line = &lines[linenum];
    p1 = &points[line->p1];
    p2 = &points[line->p2];

    if (p1->x == p2->x)
        line->slopetype = ls_vertical;
    else if (p1->y == p2->y)
        line->slopetype = ls_horizontal;
    else
    {
        line->slopetype = ls_slope;
        line->slope = FixedDiv(p2->y - p1->y, p2->x - p1->x);
        line->yintercept = p1->y - FixedMul(p1->x, line->slope);
    }

    if (p1->x < p2->x)
    {
        line->bbox[left] = p1->x;
        line->bbox[right] = p2->x;
    }
    else
    {
        line->bbox[left] = p2->x;
        line->bbox[right] = p1->x;
    }

    if (p1->y < p2->y)
    {
        line->bbox[bottom] = p1->y;
        line->bbox[top] = p2->y;
    }
    else
    {
        line->bbox[bottom] = p2->y;
        line->bbox[top] = p1->y;
    }
}

void R_ClearProclines(void)
{
    newprocline = 0;
}

int R_NewProcline(void)
{
    if (newprocline == MAXPROCLINES)
        IO_Error("R_NewProcline: no free spots\n");

    return newprocline++;
}

void R_ClearFsegs(void)
{
    newfseg = 0;
}

int R_NewFseg(void)
{
    if (newfseg == MAXFSEGS)
        IO_Error("R_NewFseg: no free spots\n");

    return newfseg++;
}

void R_ClipToLeft(vertex_t* v1, vertex_t* v2)
{
    fixed_t dx, dz, dtex;
    fixed_t frac;

    dx = v2->tx - v1->tx;
    dz = v2->tz - v1->tz;
    dtex = v2->texture - v1->texture;
    frac = FixedDiv(v1->tz + v1->tx, -dz - dx);
    v2->tz = v1->tz + FixedMul(dz, frac);
    v2->tx = v1->tx + FixedMul(dx, frac);
    v2->texture = v1->texture + FixedMul(dtex, frac);
    v2->ipx = 0;
}

void R_ClipToRight(vertex_t* v1, vertex_t* v2)
{
    fixed_t dx, dz, dtex;
    fixed_t frac;

    dx = v2->tx - v1->tx;
    dz = v2->tz - v1->tz;
    dtex = v2->texture - v1->texture;
    frac = FixedDiv(v1->tz - v1->tx, dx - dz);
    v2->tz = v1->tz + FixedMul(dz, frac);
    v2->tx = v1->tx + FixedMul(dx, frac);
    v2->texture = v1->texture + FixedMul(dtex, frac);
    v2->ipx = viewwidth;
}

void R_ClipToNearest(vertex_t* v1, vertex_t* v2)
{
    fixed_t dx, dz, dtex;
    fixed_t leftfrac, rightfrac;
    fixed_t frac;

    dx = v2->tx - v1->tx;
    dz = v2->tz - v1->tz;
    dtex = v2->texture - v1->texture;

    leftfrac = FixedDiv(v1->tz + v1->tx, -dz - dx);
    rightfrac = FixedDiv(v1->tz - v1->tx, dx - dz); 

    if (leftfrac < rightfrac)
    {
        frac = leftfrac;
        v2->ipx = 0;
    }
    else
    {
        frac = rightfrac;
        v2->ipx = viewwidth;
    }

    v2->tz = v1->tz + FixedMul(dz, frac);
    v2->tx = v1->tx + FixedMul(dx, frac);
    v2->texture =  v1->texture + FixedMul(dtex, frac);
}

//Unused function for clipping against near plane.
//The game only clips against left and right, and instead clamps near to always be 4 * FRACUNIT or greater. 
void R_ClipToZ(vertex_t* v1, vertex_t* v2)
{
    fixed_t dx, dz, dtex;
    fixed_t frac;

    dx = v1->tx;
    dz = v2->tz - v1->tz;
    dtex = v1->texture;
    frac = FixedDiv(v1->tz - (4 * FRACUNIT), dx - dz);
    v2->tz = v1->tz + FixedMul(dz, frac);
    v2->tx = v1->tx + FixedMul(dx, frac);
    v2->texture = v1->texture + FixedMul(dtex, frac);
    v2->ipx = viewwidth;
}

void R_MakeProcline(int line)
{
    line_t* line_p;
    vertex_t mv1, mv2, mtemp;
    int side;
    fixed_t texlength;
    int proc;

    line_p = &lines[line];
    line_p->validcheck = validcheck;
    line_p->procline = -1;

    R_TransformVertex(&points[line_p->p1], &mv1);
    R_TransformVertex(&points[line_p->p2], &mv2);

    mv1.texture = 0;
    texlength = line_p->length << FRACBITS;
    mv2.texture = texlength;

    switch (mv2.flags + mv1.flags * 4)
    {
    case 0:
        break;
    case 1:
        R_ClipToRight(&mv1, &mv2);
        break;
    case 2:
        if (!(line_p->flags & ML_TWOSIDED))
            return;

        R_ClipToLeft(&mv1, &mv2);
        break;
    case 3:
        R_ClipToNearest(&mv1, &mv2);
        break;
    case 4:
        if (!(line_p->flags & ML_TWOSIDED))
            return;

        R_ClipToRight(&mv2, &mv1);
        break;
    case 6:
        if (!(line_p->flags & ML_TWOSIDED))
            return;

        if (mv1.tz < 0 && mv2.tz < 0)
            return;

        R_ClipToRight(&mv2, &mv1);
        R_ClipToLeft(&mv1, &mv2);
        break;
    case 8:
        R_ClipToLeft(&mv2, &mv1);
        break;
    case 9:
        if (mv1.tz < 0 && mv2.tz < 0)
            return;

        R_ClipToLeft(&mv2, &mv1);
        R_ClipToRight(&mv1, &mv2);
        break;
    case 12:
        R_ClipToNearest(&mv2, &mv1);
        break;
    case 5:
    case 7:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
        return;
    }

    if (mv1.tz >= 0 && mv2.tz >= 0)
    {
        if (mv1.tz < 4 * FRACUNIT)
            mv1.tz = 4 * FRACUNIT;

        mv1.xscale = FixedDiv(xproject, mv1.tz);
        mv1.yscale = FixedDiv(yproject, mv1.tz);
        if (mv1.flags == 0)
            mv1.ipx = centerx + ((FixedMul(mv1.tx, mv1.xscale) + (FRACUNIT / 2)) >> FRACBITS);

        if (mv2.tz < 4 * FRACUNIT)
            mv2.tz = 4 * FRACUNIT;

        mv2.xscale = FixedDiv(xproject, mv2.tz);
        mv2.yscale = FixedDiv(yproject, mv2.tz);
        if (mv2.flags == 0)
            mv2.ipx = centerx + ((FixedMul(mv2.tx, mv2.xscale) + (FRACUNIT / 2)) >> FRACBITS);

        if (mv1.ipx != mv2.ipx)
        {
            if (mv2.ipx < mv1.ipx)
            {
                if (!(line_p->flags & ML_TWOSIDED))
                    return;
                side = 1;

                mtemp = mv1;
                mv1 = mv2;
                mv2 = mtemp;
                mv1.texture = texlength - mv1.texture;
                mv2.texture = texlength - mv2.texture;
            }
            else
                side = 0;

            if (mv2.ipx > 0 && mv1.ipx < viewwidth)
            {
                proc = R_NewProcline();
                line_p->procline = proc;
                proclines[line_p->procline].side = side;
                proclines[line_p->procline].ipx1 = mv1.ipx;
                proclines[line_p->procline].ipx2 = mv2.ipx;
                proclines[line_p->procline].texture1 = mv1.texture;
                proclines[line_p->procline].texture2 = mv2.texture;
                proclines[line_p->procline].scale1 = mv1.yscale;
                proclines[line_p->procline].scale2 = mv2.yscale;
                proclines[line_p->procline].scalestep =
                    (mv2.yscale - mv1.yscale) / (mv2.ipx - mv1.ipx) + 1;
                proclines[line_p->procline].line = line;
                proclines[line_p->procline].chained = 0;

                if (line_p->flags & ML_TWOSIDED)
                    proclines[line_p->procline].sector = sides[line_p->side[side == 0]].sector;
                else
                    proclines[line_p->procline].sector = -1;

                proclines[line_p->procline].seg = -1;
            }
        }
    }
}

void R_MakeFseg(int linenum)
{
    line_t* line_p;
    procline_t* proc;
    forwardseg_t* newfseg_p;
    side_t* front, * back;
    int delta;
    int line;
    fixed_t collumn1, collumn2;
    fixed_t texoffs;
    int y1, y2;
    sector_t* backsector;

    line_p = &lines[linenum];
    proc = &proclines[line_p->procline];
    front = &sides[line_p->side[proc->side]];
    proc->seg = R_NewFseg();
    newfseg_p = &forwardsegs[proc->seg];

    texoffs = front->firstcollumn << FRACBITS;
    delta = proc->ipx2 - proc->ipx1;

    collumn1 = FixedMul(proc->texture1 + texoffs, proc->scale1);
    collumn2 = FixedMul(proc->texture2 + texoffs, proc->scale2);
    newfseg_p->startcollumn = collumn1;
    newfseg_p->collumnstep = (collumn2 - collumn1) / delta;

    line = viewz - sector->ceilingheight;
    y1 = centeryfrac + FixedMul(proc->scale1, line);
    y2 = centeryfrac + FixedMul(proc->scale2, line);
    newfseg_p->starttopscreen = y1;
    newfseg_p->topscreenstep = (y2 - y1) / delta;

    line = viewz - sector->floorheight;
    y1 = centeryfrac + FixedMul(proc->scale1, line);
    y2 = centeryfrac + FixedMul(proc->scale2, line);
    newfseg_p->startbottomscreen = y1;
    newfseg_p->bottomscreenstep = (y2 - y1) / delta - 1;

    if (!(line_p->flags & ML_TWOSIDED))
    {
        if (front->midtexture < 0)
            IO_Error("R_MakeFseg: No mid texture on single sided line");

        if (front->midtexture >= numtextures)
            IO_Error("R_MakeFseg: Bad mid texture number");

        newfseg_p->type = 1;
        newfseg_p->texture = texturelookup[front->midtexture];

        if (!(line_p->flags & ML_DONTPEGBOTTOM))
            newfseg_p->toptextureskip = 0;
        else
            newfseg_p->toptextureskip = 128 - (sector->ceilingheight - sector->floorheight >> FRACBITS & 127);

        if (!(line_p->flags & ML_MAPPED))
        {
            line_p->flags |= ML_MAPPED;
            R_DrawBlockLine(linenum, BMF_MAPPED);
        }
    }
    else
    {
        newfseg_p->type = 0;
        back = &sides[line_p->side[proc->side ^ 1]];
        if (back->sector >= numsectors)
            IO_Error("R_MakeFseg: bad backends number");
        backsector = &sectors[back->sector];

        if (!(line_p->flags & ML_MAPPED)
            && (sector->ceilingheight != backsector->ceilingheight) ||
            (sector->floorheight != backsector->floorheight))
        {
            line_p->flags |= ML_MAPPED;
            R_DrawBlockLine(linenum, BMF_MAPPED);
        }

        if (backsector->ceilingheight < sector->ceilingheight)
        {
            if (front->toptexture < 0)
                IO_Error("R_Try add line: Exposed undefined top texture");

            if (front->toptexture >= numtextures)
                IO_Error("R_MakeFseg: Bad top texture number");

            newfseg_p->type |= 2;
            line = viewz - backsector->ceilingheight;
            y1 = FixedMul(proc->scale1, line) + centeryfrac;
            y2 = FixedMul(proc->scale2, line) + centeryfrac;

            newfseg_p->starthighscreen = y1;
            newfseg_p->highscreenstep = (y2 - y1) / delta;
            newfseg_p->texture = texturelookup[front->toptexture];
            if (!(line_p->flags & ML_DONTPEGTOP))
                newfseg_p->toptextureskip = newfseg_p->texture->height - (sector->ceilingheight - backsector->ceilingheight >> FRACBITS);
            else
                newfseg_p->toptextureskip = 0;
        }
        if (sector->floorheight < backsector->floorheight)
        {
            if (front->bottomtexture < 0)
                IO_Error("R_Try add line: Exposed undefined bottom texture");

            if (front->bottomtexture >= numtextures)
                IO_Error("R_MakeFseg: Bad bottom texture number");

            newfseg_p->type |= 4;
            line = viewz - backsector->floorheight;
            y1 = centeryfrac + FixedMul(proc->scale1, line);
            y2 = centeryfrac + FixedMul(proc->scale2, line);

            newfseg_p->startlowscreen = y1;
            newfseg_p->lowscreenstep = (y2 - y1) / delta - 1;
            newfseg_p->bottomtexture = texturelookup[front->bottomtexture];
            if (!(line_p->flags & ML_DONTPEGBOTTOM))
                newfseg_p->bottomtextureskip = 0;
            else
                newfseg_p->bottomtextureskip = sector->ceilingheight - backsector->floorheight >> FRACBITS;
        }
    }
}

void R_DrawLineDrange(void)
{
    line_t* line;
    procline_t* proc;
    int delta;

    if (vwalldrange.number >= numlines)
        IO_Error("R_DrawLineDrange: bad line number");
    if (vwalldrange.xh < vwalldrange.xl)
        IO_Error("R_DrawLineDrange: inverted range");
    
    line = &lines[vwalldrange.number];
    if (line->slopetype == ls_horizontal)
        walllight = esectorscalelight2;
    else if (line->slopetype == ls_vertical)
        walllight = esectorscalelight3;
    else if (line->slopetype == ls_slope)
        walllight = esectorscalelight;
    
    proc = &proclines[lines[vwalldrange.number].procline];

    if (proc->seg == -1)
        R_MakeFseg(proc->line);

    wallfseg = &forwardsegs[proc->seg];
    scalestep = proc->scalestep;
    topscreenstep = forwardsegs[proc->seg].topscreenstep;
    bottomscreenstep = forwardsegs[proc->seg].bottomscreenstep;
    collumnstep = forwardsegs[proc->seg].collumnstep;
    highscreenstep = forwardsegs[proc->seg].highscreenstep;
    lowscreenstep = forwardsegs[proc->seg].lowscreenstep;
    delta = vwalldrange.xl - proc->ipx1;
    if (delta == 0)
    {
        scale = proc->scale1;
        topscreen = forwardsegs[proc->seg].starttopscreen;
        bottomscreen = forwardsegs[proc->seg].startbottomscreen;
        collumn = forwardsegs[proc->seg].startcollumn;
        highscreen = forwardsegs[proc->seg].starthighscreen;
        lowscreen = forwardsegs[proc->seg].startlowscreen;
    }
    else
    {
        scale = delta * scalestep + proc->scale1;
        topscreen = forwardsegs[proc->seg].starttopscreen + delta * topscreenstep;
        bottomscreen = forwardsegs[proc->seg].startbottomscreen + delta * bottomscreenstep;
        if (wallfseg->type != 0)
            collumn = forwardsegs[proc->seg].startcollumn + delta * collumnstep;
        if ((wallfseg->type & 2))
            highscreen = forwardsegs[proc->seg].starthighscreen + delta * highscreenstep;
        if ((wallfseg->type & 4))
            lowscreen = forwardsegs[proc->seg].startlowscreen + delta * lowscreenstep;
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
}

void R_DrawBlockLine(int linenum, byte bits)
{
    line_t* line;
    point_t* p1, * p2;
    point_t pt;
    int step[2];
    int x, y, i;

    if (linenum >= numlines)
        IO_Error("R_DrawBlockLine: bad line number");

    line = &lines[linenum];
    p1 = &points[line->p1];
    p2 = &points[line->p2];
    pt = *p1;

    if (line->length > 0)
    {
        step[0] = (p2->x - p1->x) / line->length;
        step[1] = (p2->y - p1->y) / line->length;

        for (i = 0; i < line->length; i++)
        {
            x = (pt.x - maporiginx) >> MAPBLOCKSHIFT;
            y = (pt.y - maporiginy) >> MAPBLOCKSHIFT;

            blockmap[y * mapwidth + x] |= bits;

            pt.x += step[0];
            pt.y += step[1];
        }
    }
}
