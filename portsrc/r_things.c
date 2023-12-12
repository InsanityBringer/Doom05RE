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
#include "r_data.h"
#include "p_local.h"
#include "r_local.h"

char* spritename;
spritedef_t* sprites;
spriteframe_t sprtemp[26];
int numsprites;
int maxframe;

vissprite_t vissprites[MAXVISSPRITES];
int newvissprite;

void R_DrawSprite(int xclipl, int xcliph, vissprite_t* spr)
{
	byte* collumn;
	int texturecollumn;
	fixed_t frac;
	fixed_t topscreen, bottomscreen;

	if (spr->x1 < xclipl)
	{
		frac = spr->fracstep * (xclipl - spr->x1);
		if (spr->fracstep < 0)
			frac = frac + spr->patch->width * FRACUNIT - 1;
	}
	else
	{
		xclipl = spr->x1;
		if (spr->fracstep < 0)
			frac = spr->patch->width * FRACUNIT - 1;
		else
			frac = 0;
	}

	if (xcliph > spr->x2)
		xcliph = spr->x2;

	sp_colormap = spr->colormap;
	sp_fracstep = spr->iscale;
	sp_x = xclipl;

	while (sp_x <= xcliph)
	{
		if ((spr->scale < inscale[sp_x]) && (outscale[sp_x] <= spr->scale))
		{
			texturecollumn = frac >> FRACBITS;
			if ((texturecollumn < 0) || (texturecollumn >= spr->patch->width))
			{
				IO_Error("R_DrawSprite: bad texturecollumn");
#ifdef ISB_LINT
				return;
#endif
			}

			collumn = &spr->patch->width + spr->patch->coloffsets[texturecollumn];
			while (*collumn != 0xff)
			{
				topscreen = spr->topscreen + *collumn * spr->scale;
				bottomscreen = topscreen + collumn[1] * spr->scale;
				sp_y1 = topscreen + (FRACUNIT - 1) >> FRACBITS;
				sp_y2 = (bottomscreen - 1) >> FRACBITS;

				if (sp_y2 >= floorpixel[sp_x])
					sp_y2 = floorpixel[sp_x] - 1;

				if (sp_y1 <= ceilingpixel[sp_x])
					sp_y1 = ceilingpixel[sp_x] + 1;

				if (sp_y1 <= sp_y2)
				{
					sp_source = collumn + 2;

					sp_frac = FixedMul(sp_y1 * FRACUNIT - topscreen, sp_fracstep);
					((void(*)())R_RawScale)();
				}
				collumn = collumn + collumn[1] + 2;
			}
		}
		sp_x++;
		frac += spr->fracstep;
	}
}

void R_TestDrawSprite(int x, int y, int sprite, int frame, int rotation)
{
	int tx;
	fixed_t xscale, yscale, iscale;
	patch_t* patch;
	int x1, x2;
	spritedef_t* sprdef;
	spriteframe_t* sprframe;
	int lump;
	int flip;
	vissprite_t* vis;
	vissprite_t visspr;
	fixed_t centerxfrac, centeryfrac;
	int xclipl, xcliph;

	sprdef = &sprites[sprite];
	sprframe = &sprdef->spriteframes[frame];
	lump = sprframe->lump[rotation];
	flip = sprframe->flip[rotation];
	patch = (patch_t*)lumpinfo[lump].position;

	centerxfrac = x << FRACBITS;
	centeryfrac = y << FRACBITS;

	xscale = yscale = FRACUNIT;
	tx = -patch->leftoffs * FRACUNIT;
	x1 = (FixedMul(tx, xscale) + centerxfrac) >> FRACBITS;

	if (x1 > viewwidth)
		return;

	tx += (patch->width << FRACBITS);
	x2 = (FixedMul(tx, xscale) + centerxfrac) >> FRACBITS;

	if (x2 <= 0)
		return;

	vis = &visspr;
	iscale = FixedDiv(FRACUNIT, yscale);

	vis->x1 = x1;
	vis->x2 = x2 - 1;
	vis->fracstep = FixedDiv(FRACUNIT, xscale);
	if (flip)
		vis->fracstep = -vis->fracstep;

	vis->scale = yscale;
	vis->iscale = iscale;
	vis->patch = patch;
	vis->topscreen = centeryfrac - FixedMul(patch->topoffset << FRACBITS, yscale);
	vis->colormap = 0;

	xclipl = x1 < 0 ? 0 : x1;
	xcliph = x2 >= viewwidth ? viewwidth - 1 : x2;

	R_DrawSprite(xclipl, xcliph, vis);
}

void R_TestSprites(void)
{
	int sprite;
	int frame;
	fixed_t emptyout[1120];
	spritedef_t* sprdef;

	sprite = 0;
	frame = 0;
	memset(&emptyout, 0, sizeof(emptyout));
	floorpixel = viewfloorpixels;
	ceilingpixel = viewceilingpixels;
	inscale = viewfrontscale;

	outscale = &emptyout[0];
	do
	{
		sprdef = &sprites[sprite];
		R_ClearBuffer();
		R_TestDrawSprite(40, 90, sprite, frame, 0);
		R_TestDrawSprite(120, 90, sprite, frame, 1);
		R_TestDrawSprite(200, 90, sprite, frame, 2);
		R_TestDrawSprite(280, 90, sprite, frame, 3);
		R_TestDrawSprite(40, 190, sprite, frame, 4);
		R_TestDrawSprite(120, 190, sprite, frame, 5);
		R_TestDrawSprite(200, 190, sprite, frame, 6);
		R_TestDrawSprite(280, 190, sprite, frame, 7);
		playscreenupdateneeded = 2;
		IO_UpdateScreen();
		lastscan = 0;
		while (lastscan == 0)
		{
			IO_NewFrame();
#ifndef __WATCOMC__
			IO_DoEvents(); //[ISB] ugh
#endif
		}
		if ((lastscan == KEY_UPARROW) && (sprite < numsprites - 1))
		{
			sprite++;
			frame = 0;
		}
		if ((lastscan == KEY_DOWNARROW) && (0 < sprite))
		{
			sprite--;
			frame = 0;
		}
		if ((lastscan == KEY_RIGHTARROW) && (frame < sprdef->numframes - 1))
		{
			frame++;
		}
		if ((lastscan == KEY_LEFTARROW) && (0 < frame))
		{
			frame--;
		}
	} while (lastscan != KEY_ESCAPE);
	lastscan = 0;
	D_Synchronize();
}

/*
=================
=
= R_InstallSpriteLump
=
= Local function for R_InitSprites
=================
*/

void R_InstallSpriteLump(int lump, int frame, int rotation, int flipped)
{
	int		r;
	char	buf[8];

	if (frame >= 26 || rotation > 8)
	{
		strncpy(buf, lumpinfo[lump].name, 8);
		IO_Error("R_InstallSpriteLump: Bad frame characters in %s", buf);
		return;
	}

	if ((int)frame > maxframe)
		maxframe = frame;

	if (rotation == 0)
	{
		// the lump should be used for all rotations
		if (!sprtemp[frame].rotate == true)
			IO_Error("R_InitSprites: Sprite %s frame %c has multip rot=0 lump", spritename, 'A' + frame);
		if (sprtemp[frame].rotate == false)
			IO_Error("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump", spritename, 'A' + frame);

		sprtemp[frame].rotate = 0;
		for (r = 0; r < 8; r++)
		{
			sprtemp[frame].lump[r] = lump;
			sprtemp[frame].flip[r] = flipped;
		}
		return;
	}

	// the lump is only used for one rotation
	if (sprtemp[frame].rotate == false)
		IO_Error("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump", spritename, 'A' + frame);

	sprtemp[frame].rotate = 1;

	rotation--;		// make 0 based
	if (sprtemp[frame].lump[rotation] != -1)
		IO_Error("R_InitSprites: Sprite %s : %c : %c has two lumps mapped to it", spritename, 'A' + frame, '1' + rotation);

	sprtemp[frame].lump[rotation] = lump;
	sprtemp[frame].flip[rotation] = flipped;
}

/*
=================
=
= R_InitSpriteDefs
=
= Pass a null terminated list of sprite names (4 chars exactly) to be used
= Builds the sprite rotation matrixes to account for horizontally flipped
= sprites.  Will report an error if the lumps are inconsistant
=
Only called at startup
=
= Sprite lump names are 4 characters for the actor, a letter for the frame,
= and a number for the rotation, A sprite that is flippable will have an
= additional letter/number appended.  The rotation character can be 0 to
= signify no rotations
=================
*/

void R_InitSprites(char** namelist)
{
	char** check;
	int		i, l, intname, frame, rotation;
	int		start, end;

	// count the number of sprite names
	check = namelist;
	while (*check != NULL)
		check++;
	numsprites = check - namelist;

	if (!numsprites)
		return;

	sprites = (spritedef_t*)malloc(numsprites * sizeof(spritedef_t));

	start = W_GetNumForName("S_START");
	end = W_GetNumForName("S_END");

	// scan all the lump names for each of the names, noting the highest
	// frame letter
	// Just compare 4 characters as ints
	for (i = 0; i < numsprites; i++)
	{
		spritename = namelist[i];
		memset(sprtemp, -1, sizeof(sprtemp));

		maxframe = -1;
		intname = *(int*)namelist[i];

		//
		// scan the lumps, filling in the frames for whatever is found
		//
		for (l = start + 1; l < end; l++)
			if (*(int*)lumpinfo[l].name == intname)
			{
				frame = lumpinfo[l].name[4] - 'A';
				rotation = lumpinfo[l].name[5] - '0';
				R_InstallSpriteLump(l, frame, rotation, false);
				if (lumpinfo[l].name[6])
				{
					frame = lumpinfo[l].name[6] - 'A';
					rotation = lumpinfo[l].name[7] - '0';
					R_InstallSpriteLump(l, frame, rotation, true);
				}
			}

		//
		// check the frames that were found for completeness
		//
		if (maxframe == -1)
		{
			IO_Error("R_InitSprites: No lumps found for sprite %s", namelist[i]);
		}

		maxframe++;
		for (frame = 0; frame < maxframe; frame++)
		{
			switch ((int)sprtemp[frame].rotate)
			{
			case -1:	// no rotations were found for that frame at all
				IO_Error("R_InitSprites: No patches found for %s frame %c"
					, namelist[i], frame + 'A');
			case 0:	// only the first rotation is needed
				break;

			case 1:	// must have all 8 frames
				for (rotation = 0; rotation < 8; rotation++)
					if (sprtemp[frame].lump[rotation] == -1)
						IO_Error("R_InitSprites: Sprite %s frame %c is missing rotations"
							, namelist[i], frame + 'A');
			}
		}

		//
		// allocate space for the frames present and copy sprtemp to it
		//
		sprites[i].numframes = maxframe;
		sprites[i].spriteframes = (spriteframe_t*)malloc(maxframe * sizeof(spriteframe_t));
		memcpy(sprites[i].spriteframes, sprtemp, maxframe * sizeof(spriteframe_t));
	}

}

thing_t* R_GetNewThing(int sector)
{
	thing_t* rthing;
	thing_t* rnext;

	rthing = (thing_t*)Z_Malloc(playzone, sizeof(thing_t));
	rthing->prev = (thing_t*)NULL;
	rnext = sectors[sector].things;
	rthing->next = rnext;
	if (rnext != NULL)
	{
		rnext->prev = rthing;
	}
	sectors[sector].things = rthing;
	rthing->sector = sector;
	return rthing;
}

void R_RemoveThing(thing_t* rthing)
{
	//removing the sector's head
	if (rthing->prev == NULL)
	{
		sectors[rthing->sector].things = rthing->next;
	}
	else
	{
		rthing->prev->next = rthing->next;
	}

	if (rthing->next != NULL)
	{
		rthing->next->prev = rthing->prev;
	}

	Z_Free(rthing);
	return;
}

void R_ChangeThingSector(thing_t* rthing, int newsector)
{
	thing_t* rnext;

	if (rthing->prev == NULL)
	{
		sectors[rthing->sector].things = rthing->next;
	}
	else
	{
		rthing->prev->next = rthing->next;
	}
	if (rthing->next != NULL)
	{
		rthing->next->prev = rthing->prev;
	}
	rnext = sectors[newsector].things;
	rthing->prev = NULL;
	rthing->next = rnext;
	if (rnext != NULL)
	{
		rnext->prev = rthing;
	}
	sectors[newsector].things = rthing;
	rthing->sector = newsector;
	return;
}

void R_ClearVisSprites()
{
	newvissprite = 0;
}

vissprite_t* R_NewVisSprite(void)
{
	if (newvissprite == MAXVISSPRITES)
	{
		IO_Error("R_NewVisSprite: no free spots\n");
	}

	return &vissprites[newvissprite++];
}

/*
===================
=
= R_ProjectSprite
=
= Generates a vissprite for a thing if it might be visible
=
===================
*/

void R_ProjectThing(thing_t* thing)
{
	fixed_t		trx, try;
	fixed_t		gxt, gyt;
	fixed_t		tx, tz;
	fixed_t		xscale, yscale;
	patch_t* patch;
	int			x1, x2;
	spritedef_t* sprdef;
	spriteframe_t* sprframe;
	int			lump;
	unsigned	rot;
	boolean		flip;
	int			collumntop;
	vissprite_t* vis;
	int			light;

	thing->vissprite = NULL;

	//
	// transform the origin point
	//
	trx = thing->x - viewx;
	try = thing->y - viewy;

	gxt = FixedMul(trx, viewcos);
	gyt = -FixedMul(try, viewsin);
	tz = gxt - gyt;

	if (tz < MINZ)
		return;		// thing is behind view plane

	xscale = FixedDiv(xproject, tz);
	yscale = FixedDiv(yproject, tz);

	gxt = -FixedMul(trx, viewsin);
	gyt = FixedMul(try, viewcos);
	tx = -(gyt + gxt);

	//#ifdef RANGECHECK
	if (thing->sprite >= numsprites)
		IO_Error("R_TryAddThing: invalid sprite number %i\n", thing->sprite);
	//#endif
	sprdef = &sprites[thing->sprite];

	//#ifdef RANGECHECK
	if (thing->frame >= sprdef->numframes)
		IO_Error("R_TryAddThing: invalid sprite frame %i : %i\n", thing->sprite, thing->frame);
	//endif
	sprframe = &sprdef->spriteframes[thing->frame];

	if (sprframe->rotate)
	{	// choose a different rotation based on player view
		rot = (viewangle - thing->angle) + 0x1200 >> 10 & 7;
		lump = sprframe->lump[rot];
		flip = (boolean)sprframe->flip[rot];
	}
	else
	{	// use single rotation for all views
		lump = sprframe->lump[0];
		flip = (boolean)sprframe->flip[0];
	}

	patch = (patch_t*)lumpinfo[lump].position;

	//
	// calculate edges of the shape
	//
	tx -= (patch->leftoffs * FRACUNIT);
	x1 = (FixedMul(tx, xscale) + centerxfrac) >> FRACBITS;
	if (x1 > viewwidth)
		return;		// off the right side

	tx += (patch->width * FRACUNIT);
	x2 = (FixedMul(tx, xscale) + centerxfrac) >> FRACBITS;
	if (x2 <= 0)
		return;		// off the left side

	vis = R_NewVisSprite();
	thing->vissprite = vis;
	vis->x1 = x1;
	vis->x2 = x2 - 1;
	vis->fracstep = FixedDiv(FRACUNIT, xscale);
	if (flip)
		vis->fracstep = -vis->fracstep;

	vis->scale = yscale;
	collumntop = viewz - (thing->z + patch->topoffset * FRACUNIT);
	vis->iscale = FixedDiv(FRACUNIT, yscale);
	vis->patch = patch;
	vis->topscreen = FixedMul(collumntop, yscale) + centeryfrac;

	light = R_LightFromVScale(yscale) + (sectors[thing->sector].lightlevel >> 4) * 48;
	vis->colormap = scalelight[light];
}

void R_DrawSectorThings(sector_t* sector, int xl, int xh)
{
	thing_t* rthing, * rnext, * rprev;

	if (sector->validcheck != validcheck)
	{
		sector->validcheck = validcheck;
		rnext = sector->things;
		while (rthing = rnext, rthing != NULL)
		{
			rnext = rthing->next;
			R_ProjectThing(rthing);
			while (((rprev = rthing->prev, rprev != NULL &&
				(rprev->vissprite != NULL)) &&
				((rthing->vissprite == NULL ||
					(rthing->vissprite->scale <= rprev->vissprite->scale)))))
			{
				if (rthing->next != NULL)
				{
					rthing->next->prev = rprev;
				}
				rprev->next = rthing->next;
				rthing->next = rprev;
				rthing->prev = rprev->prev;
				if (rthing->prev == NULL)
				{
					sector->things = rthing;
				}
				else
				{
					rthing->prev->next = rthing;
				}
				rprev->prev = rthing;
			}
		}
	}
	rthing = sector->things;
	while (rthing != NULL)
	{
		if (rthing->vissprite != NULL)
		{
			R_DrawSprite(xl, xh, rthing->vissprite);
		}
		rthing = rthing->next;
	}
	return;
}
