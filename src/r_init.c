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
uint8_t* sp_source;
int sp_colormap;
fixed_t sp_frac, sp_fracstep;

//MapRow details
int mr_x1, mr_x2, mr_y;
uint8_t* mr_picture;
int mr_colormap;
fixed_t mr_xfrac, mr_yfrac, mr_xstep, mr_ystep;

int centerx, centery;
fixed_t centerxfrac, centeryfrac;

fixed_t xproject, yproject;

vissprite_t pshape8;
fixed_t pshapexscale;

int DAT_00069b94[640];
int DAT_0006ad14[640];

fixed_t yslope[832];

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

void R_TransformVertex(vertex_t* source, vertex_t* destination)
{
	//longlong lVar1;
	//longlong lVar2;
	int iVar3;
	int iVar4;

	iVar3 = source->tx - viewx;
	iVar4 = source->tz - viewy;

	destination->tx = -FixedMul(iVar4, viewcos) + FixedMul(iVar3, viewsin);
	destination->tz = (FixedMul(iVar3, viewcos) + FixedMul(iVar4, viewsin));

	destination->flags = 0;
	if (destination->tz < destination->tx)
	{
		destination->flags = 1;
	}
	if (-destination->tz != destination->tx && destination->tz <= -destination->tx)
	{
		destination->flags |= 2;
	}
	return;
}

void R_ChangeWindow(int width, int height, fixed_t vertscale)
{
	int uVar1;
	int iVar2;

	centerx = width / 2;
	centery = (height * 2) / 5;
	centerxfrac = centerx << FRACBITS;
	centeryfrac = centery << FRACBITS;
	
	yproject = FixedMul(centerxfrac, vertscale);
	vscalelight = FixedDiv(0xa00000, yproject);
	pshape8.iscale = FixedDiv(0x1400000, width * vertscale);
	pshape8.scale = FixedDiv(width * vertscale, 0x1400000);
	pshape8.fracstep = FixedDiv(0x1400000, width << FRACBITS);
	pshapexscale = FixedDiv(width << FRACBITS, 0x1400000);

	iVar2 = 0;
	viewheight = height;
	viewwidth = width;
	xproject = centerxfrac;
	while (iVar2 < (int)viewwidth) 
	{
		viewfloorpixels[iVar2] = viewheight;
		viewceilingpixels[iVar2] = -1;//0xffffffff;
		iVar2++;
	}

	for (iVar2 = 0; iVar2 < viewheight; iVar2++)
	{
		uVar1 = ((iVar2 - centery) << FRACBITS) + 0x8000;
		yslope[iVar2] = FixedDiv(uVar1, yproject);
	}
}

int R_LightFromVScale(fixed_t scale)
{
	int uVar1;

	uVar1 = FixedMul(scale, vscalelight) >> 0xc;
	if (0x2f < uVar1) 
	{
		uVar1 = 0x2f;
	}
	return uVar1;
}

int R_LightFromZ(fixed_t z)
{
	//TODO: This occasionally (if your head pokes in the ceiling) produces an insane value from a negative input
	//determine if that's accurate.
	int local_20;

	local_20 = FixedMul(FixedDiv(yproject, z), vscalelight) >> 0xc;
	if (0x2f < local_20) 
	{
		local_20 = 0x2f;
	}
	return local_20;
}

void R_RenderView(int sectornum, fixed_t x, fixed_t y, fixed_t z, int angle)
{
	framepos_t* pfVar1;

	validcheck = validcheck + 1;
	viewangle = angle;
	viewcos = cosines[angle];
	viewsin = sines[angle];
	viewsector = sectornum;
	viewz = z;
	viewy = y;
	viewx = x;
	sd->lastframe ^= 1;
	pfVar1 = &sd->framepos[sd->lastframe];
	pfVar1->sector = viewsector;
	pfVar1->x = viewx;
	pfVar1->y = viewy;
	pfVar1->z = viewz;
	pfVar1->angle = viewangle;

	R_PrepPlanes();
	R_ClearFsegs();
	R_ClearProclines();
	R_ClearVisSprites();

	vissec = &subsectors[0];
	R_DrawSector(sectornum, 0, viewwidth - 1, viewfloorpixels, viewceilingpixels, viewfrontscale);
	
	playscreenupdateneeded = 2;
	inscale = &viewfrontscale[0];
	outscale = &viewbackscale[0];
	pshape8.colormap = scalelight[((int)sectors[sectornum].lightlevel >> 4) * 0x30 + 0x2f];
	return;
}

void R_DrawPlayerShape(int sprite, int frame, int sx, int sy)
{
	int local_14;
	int local_34;
	int local_30;
	int local_28;
	spritedef_t* local_20;
	int xclipl;

	local_34 = sprite;
	local_30 = frame;
	local_28 = sy;
	if (numsprites <= sprite) 
	{
		IO_Error("R_DrawPlayerShape: invalid sprite number %i\n", sprite);
	}
	local_20 = &sprites[local_34];
	if (local_20->numframes <= frame) 
	{
		IO_Error("R_DrawPlayerShape: invalid sprite frame %i : %i", local_34, local_30);
	}

	pshape8.patch = (patch_t*)lumpinfo[local_20->spriteframes[local_30].lump[0]].position;

	pshape8.x1 = FixedMul(sx - pshape8.patch->leftoffs - 0xa0, pshapexscale) + centerx;
	pshape8.x2 = pshape8.x1 + FixedMul(pshape8.patch->width, pshapexscale);
	xclipl = pshape8.x1;
	if (pshape8.x1 < 0) 
	{
		xclipl = 0;
	}
	local_14 = pshape8.x2 + -1;
	if (viewwidth <= local_14) 
	{
		local_14 = viewwidth + -1;
	}
	pshape8.topscreen = centeryfrac + (local_28 - pshape8.patch->topoffset + -100) * pshape8.scale;
	R_DrawSprite(xclipl, local_14, &pshape8);
	return;
}