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
#include "w_wad.h"
#include "r_ref.h"
#include "p_play.h"
#include "r_local.h"
#include "r_data.h"

int validcheck;

int viewheight, viewwidth;
int naturalwidth, naturalheight;

fixed_t viewfrontscale[1152];
fixed_t viewbackscale[1152];

int viewfloorpixels[1152];
int viewceilingpixels[1152];

int viewangle;
fixed_t viewx, viewy, viewz;
fixed_t viewsin, viewcos;
int viewsector;

int viewheight;
fixed_t vscalelight;

//ScalePost details
int sp_x, sp_y1, sp_y2;
byte* sp_source;
int sp_colormap;
fixed_t sp_frac, sp_fracstep;

//MapRow details
int mr_x1, mr_x2, mr_y;
byte* mr_picture;
int mr_colormap;
fixed_t mr_xfrac, mr_yfrac, mr_xstep, mr_ystep;

int centerx, centery;
fixed_t centerxfrac, centeryfrac;

fixed_t xproject, yproject;

vissprite_t pshape8;
fixed_t pshapexscale;

void(*R_MapRow)();
void(*R_RawScale)();

void R_Startup()
{
	R_InitLumps();
	R_InitTables();
	R_InitWorld();
	R_InitTextureCache();
	R_InitVideoDevice();
}

void R_ChangeWindow(int width, int height, fixed_t vertscale)
{
	int i;

	centerx = width / 2;
	centery = (height * 2) / 5;
	centerxfrac = centerx << FRACBITS;
	centeryfrac = centery << FRACBITS;

	yproject = FixedMul(centerxfrac, vertscale);
	vscalelight = FixedDiv((SCREENWIDTH / 2) * FRACUNIT, yproject);
	pshape8.iscale = FixedDiv(SCREENWIDTH * FRACUNIT, width * vertscale);
	pshape8.scale = FixedDiv(width * vertscale, SCREENWIDTH * FRACUNIT);
	pshape8.fracstep = FixedDiv(SCREENWIDTH * FRACUNIT, width << FRACBITS);
	pshapexscale = FixedDiv(width << FRACBITS, SCREENWIDTH * FRACUNIT);

	viewheight = height;
	viewwidth = width;
	xproject = centerxfrac;
	for (i = 0; i < viewwidth; i++)
	{
		viewfloorpixels[i] = viewheight;
		viewceilingpixels[i] = -1;
	}

	for (i = 0; i < viewheight; i++)
		yslope[i] = FixedDiv(((i - centery) << FRACBITS) + 0x8000, yproject);
}

void R_TransformVertex(point_t* source, vertex_t* dest)
{
	fixed_t trx, try; //[ISB] oops, C++ reserved word...
	fixed_t gxt, gyt;

	trx = source->x - viewx;
	try = source->y - viewy;

	gxt = -FixedMul(trx, viewsin);
	gyt = FixedMul(try, viewcos);

	dest->tx = -(gyt + gxt);

	gxt = FixedMul(trx, viewcos);
	gyt = -FixedMul(try, viewsin);
	dest->tz = gxt - gyt;

	dest->flags = 0;
	if (dest->tz < dest->tx)
		dest->flags = 1;
	
	if (-dest->tz != dest->tx && dest->tz <= -dest->tx)
		dest->flags |= 2;
}

int R_LightFromVScale(fixed_t scale)
{
	int index;

	index = FixedMul(scale, vscalelight) >> 12;
	if (index > 47) 
		index = 47;

	return index;
}

int R_LightFromZ(fixed_t z)
{
	//This occasionally (if your head pokes in the ceiling) produces an insane value from a negative input.
	//This seems to be accurate behavior. 
	int index;

	index = FixedMul(FixedDiv(yproject, z), vscalelight) >> 12;
	if (index > 47) 
		index = 47;
#ifdef _MSC_VER
	if (index < 0)
		index = 0;
#endif
	
	return index;
}

void R_DrawPlayerShape(int sprite, int frame, int sx, int sy)
{
	int x1, x2;
	patch_t* patch;
	int lump;
	spritedef_t* sprdef;

	if (sprite >= numsprites)
		IO_Error("R_DrawPlayerShape: invalid sprite number %i ", sprite);
	
	sprdef = &sprites[sprite];
	if (sprdef->numframes <= frame)
		IO_Error("R_DrawPlayerShape: invalid sprite frame %i : %i", sprite, frame);
	
	lump = sprdef->spriteframes[frame].lump[0];
	patch = (patch_t*)lumpinfo[lump].position;
	pshape8.patch = patch;

	sx -= patch->leftoffs + (SCREENWIDTH / 2);
	sy -= patch->topoffset + (SCREENHEIGHT / 2);

	x1 = pshape8.x1 = FixedMul(sx, pshapexscale) + centerx;
	x2 = pshape8.x2 = x1 + FixedMul(pshape8.patch->width, pshapexscale);

	if (x1 < 0)
		x1 = 0;
	
	x2--;
	if (x2 >= viewwidth)
		x2 = viewwidth - 1;
	
	pshape8.topscreen = centeryfrac + sy * pshape8.scale;
	R_DrawSprite(x1, x2, &pshape8);
}

void R_RenderView(int sectornum, fixed_t x, fixed_t y, fixed_t z, int angle)
{
	framepos_t* fp;

	validcheck++;
	viewangle = angle;
	viewcos = cosines[angle];
	viewsin = sines[angle];
	viewsector = sectornum;
	viewz = z;
	viewy = y;
	viewx = x;
	sd->lastframe ^= 1;
	fp = &sd->framepos[sd->lastframe];
	fp->sector = viewsector;
	fp->x = viewx;
	fp->y = viewy;
	fp->z = viewz;
	fp->angle = viewangle;

	R_PrepPlanes();
	R_ClearFsegs();
	R_ClearProclines();
	R_ClearVisSprites();

	vissec = (subsector_t*)&subsectors[0];
	R_DrawSector(sectornum, 0, viewwidth - 1, viewfloorpixels, viewceilingpixels, viewfrontscale);
	
	playscreenupdateneeded = 2;
	inscale = &viewfrontscale[0];
	outscale = &viewbackscale[0];
	pshape8.colormap = scalelight[((int)sectors[sectornum].lightlevel >> 4) * 48 + 47];
}
