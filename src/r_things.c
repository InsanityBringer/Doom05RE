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

char* spritename;
spritedef_t* sprites;
spriteframe_t tempsprite[26];
int numsprites;
int maxframe;

vissprite_t vissprites[128];
int newvissprite;

void R_DrawSprite(int xclipl, int xcliph, vissprite_t* spr)
{
	int local_1c;
	int local_18;
	uint8_t* pbVar2;
	int local_2c;

	if (spr->x1 < xclipl) 
	{
		local_1c = spr->fracstep * (xclipl - spr->x1);
		sp_x = xclipl;
		if (spr->fracstep < 0)
		{
			local_1c = local_1c + spr->patch->width * FRACUNIT + -1;
		}
	}
	else 
	{
		sp_x = spr->x1;
		if (spr->fracstep < 0) 
		{
			local_1c = spr->patch->width * FRACUNIT + -1;
		}
		else 
		{
			local_1c = 0;
		}
	}
	local_2c = xcliph;
	if (spr->x2 < xcliph)
	{
		local_2c = spr->x2;
	}
	sp_colormap = spr->colormap;
	sp_fracstep = spr->iscale;

	while (sp_x <= local_2c)
	{
		if ((spr->scale < inscale[sp_x]) && (outscale[sp_x] <= spr->scale))
		{
			local_18 = local_1c >> FRACBITS;
			if ((local_18 < 0) || ((int)spr->patch->width <= local_18)) 
			{
				IO_Error("R_DrawSprite: bad texturecollumn\n");
			}

			pbVar2 = &spr->patch->width + ((int)spr->patch->coloffsets[local_18]);
			while (*pbVar2 != 0xff)
			{
				local_18 = spr->topscreen + *pbVar2 * spr->scale;
				sp_y1 = local_18 + 0xffff >> FRACBITS;
				sp_y2 = (int)(local_18 + pbVar2[1] * spr->scale + -1) >> FRACBITS;
				if (floorpixel[sp_x] <= sp_y2) 
				{
					sp_y2 = floorpixel[sp_x] + -1;
				}
				if (sp_y1 <= ceilingpixel[sp_x]) 
				{
					sp_y1 = ceilingpixel[sp_x] + 1;
				}
				if (sp_y1 <= sp_y2)
				{
					sp_source = pbVar2 + 2;

					sp_frac = FixedMul(sp_y1 * FRACUNIT - local_18, sp_fracstep);
					((void(*)())R_RawScale)();
				}
				pbVar2 = pbVar2 + pbVar2[1] + 2;
			}
		}
		sp_x++;
		local_1c = local_1c + spr->fracstep;
	}
	return;
}

void R_TestDrawSprite(int x, int y, int sprite, int frame, int rotation)
{
	int iVar1;
	int local_84;
	int local_80;
	vissprite_t local_7c;

	local_7c.patch =
		(patch_t*)lumpinfo[sprites[sprite].spriteframes[frame].lump[rotation]].position;
	iVar1 = local_7c.patch->leftoffs * -FRACUNIT;

	local_7c.x1 = (FixedMul(iVar1, FRACUNIT) + (x * FRACUNIT)) >> FRACBITS;

	local_80 = (FixedMul(iVar1 + local_7c.patch->width * FRACUNIT, FRACUNIT) + (x * FRACUNIT)) >> FRACBITS;

	if (local_7c.x1 <= viewwidth && 0 < local_80)
	{
		local_7c.x2 = local_80 - 1;
		local_7c.fracstep = 0x10000;
		if (sprites[sprite].spriteframes[frame].flip[rotation] != 0)
		{
			local_7c.fracstep = -0x10000;
		}
		local_7c.scale = 0x10000;
		local_7c.iscale = 0x10000;
		local_7c.topscreen = (y * FRACUNIT) + FixedMul(local_7c.patch->topoffset << FRACBITS, FRACUNIT) * -1;
		local_7c.colormap = 0;
		local_84 = local_7c.x1;
		if (local_7c.x1 < 0)
		{
			local_84 = 0;
		}
		if (viewwidth <= local_80) 
		{
			local_80 = viewwidth - 1;
		}
		R_DrawSprite(local_84, local_80, &local_7c);
	}
	return;
}

void R_TestSprites(void)
{
	spritedef_t* local_24;
	int local_20;
	int sprite;

	uint8_t buf[4480];

	sprite = 0;
	local_20 = 0;
	memset(&buf, 0, 0x1180);
	floorpixel = viewfloorpixels;
	ceilingpixel = viewceilingpixels;
	inscale = viewfrontscale;

	outscale = (fixed_t*)&buf[0];
	do 
	{
		local_24 = &sprites[sprite];
		R_ClearBuffer();
		R_TestDrawSprite(0x28, 0x5a, sprite, local_20, 0);
		R_TestDrawSprite(0x78, 0x5a, sprite, local_20, 1);
		R_TestDrawSprite(200, 0x5a, sprite, local_20, 2);
		R_TestDrawSprite(0x118, 0x5a, sprite, local_20, 3);
		R_TestDrawSprite(0x28, 0xbe, sprite, local_20, 4);
		R_TestDrawSprite(0x78, 0xbe, sprite, local_20, 5);
		R_TestDrawSprite(200, 0xbe, sprite, local_20, 6);
		R_TestDrawSprite(0x118, 0xbe, sprite, local_20, 7);
		playscreenupdateneeded = 2;
		IO_UpdateScreen();
		lastscan = 0;
		while (lastscan == 0) 
		{
			IO_NewFrame();
			IO_DoEvents(); //[ISB] ugh
		}
		if ((lastscan == KEY_UP) && (sprite < numsprites - 1)) 
		{
			sprite++;
			local_20 = 0;
		}
		if ((lastscan == KEY_DOWN) && (0 < sprite))
		{
			sprite--;
			local_20 = 0;
		}
		if ((lastscan == KEY_RIGHT) && (local_20 < local_24->numframes - 1)) 
		{
			local_20++;
		}
		if ((lastscan == KEY_LEFT) && (0 < local_20))
		{
			local_20--;
		}
	} while (lastscan != KEY_ESC);
	lastscan = 0;
	D_Synchronize();
	return;
}

thing_t* R_GetNewThing(int sector)
{
	thing_t* ptVar1;
	thing_t* ptVar2;
	int local_28;

	local_28 = sector;
	ptVar2 = (thing_t*)Z_Malloc(playzone, sizeof(thing_t));
	ptVar2->prev = (thing_t*)NULL;
	ptVar1 = sectors[local_28].things;
	ptVar2->next = ptVar1;
	if (ptVar1 != NULL) 
	{
		ptVar1->prev = ptVar2;
	}
	sectors[local_28].things = ptVar2;
	ptVar2->sector = local_28;
	return ptVar2;
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
	thing_t* ptVar1;

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
	ptVar1 = sectors[newsector].things;
	rthing->prev = NULL;
	rthing->next = ptVar1;
	if (ptVar1 != NULL)
	{
		ptVar1->prev = rthing;
	}
	sectors[newsector].things = rthing;
	rthing->sector = newsector;
	return;
}

uint32_t R_InstallSpriteLump(int lump, int param_2, int param_3, int param_4)
{
	uint32_t local_1c;
	uint32_t local_20;
	short sVar1;
	uint32_t bVar2;
	uint32_t local_18;
	int iVar3;
	char buf[8];

	local_20 = param_2;
	local_1c = lump;
	local_18 = param_4;
	if ((0x19 < param_2) || (8 < (uint32_t)param_3)) 
	{
		strncpy(buf, lumpinfo[lump].name, 8);
		IO_Error("R_InitSprites: Bad frame characters in %s\n", buf);
	}
	bVar2 = (uint8_t)local_18;
	sVar1 = (short)local_1c;
	if (maxframe < (int)local_20)
	{
		maxframe = local_20;
	}
	if (param_3 == 0) 
	{
		if (tempsprite[local_20].rotate == 0) 
		{
			IO_Error("R_InitSprites: Sprite %s frame %c has multip rot=0 lump\n", spritename, local_20 + 0x41);
		}
		if (tempsprite[local_20].rotate == 1)
		{
			IO_Error("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump\n", spritename, local_20 + 0x41);
		}
		tempsprite[local_20].rotate = 0;
		local_1c = local_20 * 0x1c;
		local_18 = 0;
		while ((int)local_18 < 8) 
		{
			tempsprite[local_20].lump[local_18] = sVar1;
			tempsprite[local_20].flip[local_18] = bVar2;
			local_1c = local_18;
			local_18 = local_18 + 1;
		}
	}
	else 
	{
		if (tempsprite[local_20].rotate == 0)
		{
			IO_Error("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump\n", spritename, local_20 + 0x41);
		}
		tempsprite[local_20].rotate = 1;
		iVar3 = param_3 + -1;
		if (tempsprite[local_20].lump[iVar3] != -1)
		{
			IO_Error("R_InitSprites: Sprite %s : %c : %c has two lumps mapped to it\n", spritename, local_20 + 0x41, param_3 + 0x2f);
		}
		tempsprite[local_20].lump[iVar3] = (short)local_1c;
		local_1c = iVar3 * 2 & 0xffff0000U | local_1c & 0xff00 | local_18 & 0xff;
		tempsprite[local_20].flip[iVar3] = (uint8_t)(local_18 & 0xff);
	}
	return local_1c;
}

void R_InitSprites(char** namelist)
{
	int pcVar1;
	int iVar2;
	int local_EAX_100;
	int iVar3;
	spriteframe_t* psVar4;
	int local_34;
	int local_30;
	int local_2c;
	int local_24;
	int local_20;
	char** ppcStack28;

	ppcStack28 = namelist;
	while (*ppcStack28 != NULL) 
	{
		ppcStack28 = ppcStack28 + 1;
	}
	numsprites = (int)((int)ppcStack28 - (int)namelist) / sizeof(char*);
	if (numsprites != 0) 
	{
		sprites = (spritedef_t*)malloc(numsprites * sizeof(spritedef_t));
#ifdef ISB_LINT
		if (sprites == NULL)
		{
			IO_Error("R_InitSprites: can't allocate sprites\n");
			return;
		}
#endif
		local_EAX_100 = W_GetNumForName("S_START");
		iVar3 = W_GetNumForName("S_END");
		local_20 = 0;
		while (local_20 < numsprites)
		{
			spritename = namelist[local_20];
			memset((char*)tempsprite, -1, sizeof(tempsprite));
			maxframe = -1;
			pcVar1 = *(int*)namelist[local_20];
			local_24 = local_EAX_100;
			while (local_24 = local_24 + 1, local_24 < iVar3)
			{
				if (*(int*)lumpinfo[local_24].name == pcVar1)
				{
					R_InstallSpriteLump(local_24, lumpinfo[local_24].name[4] - 0x41, lumpinfo[local_24].name[5] - 0x30, 0);
					if ((lumpinfo[local_24].name[6]) != '\0')
					{
						R_InstallSpriteLump(local_24, lumpinfo[local_24].name[6] - 0x41, lumpinfo[local_24].name[7] - 0x30, 1);
					}
				}
			}
			if (maxframe == -1) 
			{
				IO_Error("R_InitSprites: No lumps found for sprite %s\n", namelist[local_20]);
			}
			maxframe++;
			local_2c = 0;
			while (local_2c < maxframe) 
			{
				iVar2 = tempsprite[local_2c].rotate;
				if (iVar2 < 0)
				{
					if (iVar2 == -1) 
					{
						IO_Error("R_InitSprites: No patches found for %s frame %c\n", namelist[local_20], local_2c + 0x41);
					}
				}
				else 
				{
					if ((0 < iVar2) && (iVar2 == 1)) 
					{
						local_30 = 0;
						while (local_30 < 8)
						{
							if (tempsprite[local_2c].lump[local_30] == -1) 
							{
								IO_Error("R_InitSprites: Sprite %s frame %c is missing rotations\n", namelist[local_20],
									local_2c + 0x41);
							}
							local_30 = local_30 + 1;
						}
					}
				}
				local_2c = local_2c + 1;
			}
			sprites[local_20].numframes = maxframe;
			psVar4 = (spriteframe_t*)malloc(maxframe * sizeof(spriteframe_t));
#ifdef ISB_LINT
			if (psVar4 == NULL)
			{
				IO_Error("R_InitSprites: can't allocate sprite frame list\n");
				return;
			}
#endif
			sprites[local_20].spriteframes = psVar4;
			memcpy(sprites[local_20].spriteframes, tempsprite, maxframe * sizeof(spriteframe_t));
			local_20++;
		}
	}
	return;
}

void R_ClearVisSprites()
{
	newvissprite = 0;
}

vissprite_t* R_NewVisSprite(void)
{
	int iVar1;

	if (newvissprite == 128) 
	{
		IO_Error("R_NewVisSprite: no free spots\n");
	}
	iVar1 = newvissprite;
	newvissprite = newvissprite + 1;
	return &vissprites[iVar1];
}


void R_ProjectThing(thing_t* thing)
{
	uint8_t bVar1;
	short sVar2;
	int local_2c;
	int local_44;
	int local_38;
	int local_34;
	spriteframe_t* psVar5;
	int local_5c;
	vissprite_t* vissprite;
	int extraout_EDX;
	thing_t* local_6c;
	int* local_64;
	spritedef_t* local_4c;
	patch_t* patch;

	thing->vissprite = (void*)NULL;
	local_2c = thing->x - viewx;
	local_44 = thing->y - viewy;

	local_38 = FixedMul(local_2c, viewcos) + FixedMul(local_44, viewsin);

	if (0x3ffff < local_38)
	{
		local_34 = FixedDiv(xproject, local_38);
		local_38 = FixedDiv(yproject, local_38);

		local_2c = -(FixedMul(local_44, viewcos) - FixedMul(local_2c, viewsin));

		local_6c = thing;
		if (numsprites <= thing->sprite)
		{
			IO_Error("R_TryAddThing: invalid sprite number %i\n", thing->sprite);
		}
		local_4c = &sprites[local_6c->sprite];
		if (local_4c->numframes <= local_6c->frame) 
		{
			IO_Error("R_TryAddThing: invalid sprite frame %i : %i\n", local_6c->sprite, local_6c->frame);
		}
		psVar5 = &local_4c->spriteframes[local_6c->frame];
		if (psVar5->rotate == 0)
		{
			sVar2 = psVar5->lump[0];
			bVar1 = psVar5->flip[0];
		}
		else 
		{
			local_5c = (viewangle - local_6c->angle) + 0x1200 >> 10 & 7;
			sVar2 = psVar5->lump[local_5c];
			bVar1 = psVar5->flip[local_5c];
		}
		local_5c = bVar1;
		patch = (patch_t*)lumpinfo[sVar2].position;
		local_2c = local_2c + patch->leftoffs * -FRACUNIT;

		local_44 = (FixedMul(local_2c, local_34) + centerxfrac) >> FRACBITS;

		if (local_44 <= viewwidth)
		{
			local_2c = (FixedMul(local_2c + patch->width * FRACUNIT, local_34) + centerxfrac) >> FRACBITS;

			if (0 < local_2c)
			{
				vissprite = R_NewVisSprite();
				local_6c->vissprite = vissprite;
				vissprite->x1 = local_44;
				vissprite->x2 = local_2c - 1;
				vissprite->fracstep = FixedDiv(FRACUNIT, local_34);

				if (local_5c != 0)
				{
					vissprite->fracstep = -vissprite->fracstep;
				}

				vissprite->scale = local_38;
				local_2c = viewz - (local_6c->z + patch->topoffset * FRACUNIT);
				vissprite->iscale = FixedDiv(FRACUNIT, local_38);
				vissprite->patch = patch;
				vissprite->topscreen = FixedMul(local_2c, local_38) + centeryfrac;
				vissprite->colormap = scalelight[R_LightFromVScale(local_38) + (sectors[thing->sector].lightlevel >> 4) * 48];
			}
		}
	}
	return;
}

void R_DrawSectorThings(sector_t* sector, int xl, int xh)
{
	thing_t* ptVar1;
	thing_t* ptVar2;
	thing_t* local_14;

	if (sector->validcheck != validcheck) 
	{
		sector->validcheck = validcheck;
		ptVar1 = sector->things;
		while (local_14 = ptVar1, local_14 != NULL) 
		{
			ptVar1 = local_14->next;
			R_ProjectThing(local_14);
			while (((ptVar2 = local_14->prev, ptVar2 != NULL &&
				(ptVar2->vissprite != NULL)) &&
				((local_14->vissprite == NULL ||
					(local_14->vissprite->scale <= ptVar2->vissprite->scale))))) 
			{
				if (local_14->next != NULL) 
				{
					local_14->next->prev = ptVar2;
				}
				ptVar2->next = local_14->next;
				local_14->next = ptVar2;
				local_14->prev = ptVar2->prev;
				if (local_14->prev == NULL) 
				{
					sector->things = local_14;
				}
				else 
				{
					local_14->prev->next = local_14;
				}
				ptVar2->prev = local_14;
			}
		}
	}
	local_14 = sector->things;
	while (local_14 != NULL)
	{
		if (local_14->vissprite != NULL) 
		{
			R_DrawSprite(xl, xh, local_14->vissprite);
		}
		local_14 = local_14->next;
	}
	return;
}
