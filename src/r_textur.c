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
//TODO: Portability
#include <malloc.h>
#include "r_ref.h"
#include "r_local.h"

memzone_t* texturezone;

void R_InitTextureCache()
{
	texturezone = Z_AllocateZone(0x80000);
}

//[ISB] these are interesting, were they considering dynamic patches at this time?
//no development on this front though...
void R_PlacePatch(int wall, patch_t* patch, int x, int y)
{
}

void P_RemovePatch(int wall, patch_t* patch, int x, int y)
{
}

void R_DrawCollumnInPost(post_t* patch, post_t* cache, int originy)
{
	int count, position;
	byte* source, * dest;

	dest = (byte*)cache + 2;

	while (patch->topdelta != 255)
	{
		source = ((byte*)patch) + 2;
		count = patch->length;

		position = patch->topdelta + originy;

		if (position < 0)
		{
			count += position;
			position = 0;
		}

		if (position + count > cache->length)
			count = cache->length - position;

		if (count > 0)
		{
			memcpy(dest + position, source, count);
		}

		patch = ((byte*)patch) + patch->length + 2;
	}
}

void R_GenerateTexture(maptexture_t* texture)
{
	byte* patchcount;
	mappatch_t* patch;
	patch_t* realpatch;
	int custom;
	int x, x2;
	int pcol;
	size_t compositesize;
	void** directory;
	byte* custom_p;
	int i;
	byte* patchcol;
	size_t size;

	patchcount = alloca(texture->width + 3 & ~3);

	memset(patchcount, 0, texture->width + 3 & ~3);

	for (patch = texture->patches, i = 0; i < texture->patchcount; i++, patch++)
	{
		if (patch->patch > nummappatches)
			IO_Error("R_GenerateTexture: bad patch number");

		realpatch = patchlookup[patch->patch];
		x = patch->originx;
		if (x < 0)
			x = 0;

		x2 = x + realpatch->width;
		if (x2 > texture->width)
			x2 = texture->width;

		for (; x < x2; x++)
			patchcount[x]++;
	}

	custom = 0;
	for (x = 0; x < texture->width; x++)
	{
		if (patchcount[x] == 0)
			IO_Error("R_GenerateTexture: collumn without a patch");
		else if (patchcount[x] > 1)
			custom++;
	}

	compositesize = texture->height + 3;
	size = texture->width * sizeof(void*) + (custom * compositesize);
	Z_CacheMalloc(texturezone, size, &texture->collumndirectory);
	directory = texture->collumndirectory;
	texture->collumndirectory = directory;

	if (custom != 0)
	{
		custom_p = directory + texture->width;
		for (x = 0; x < texture->width; x++)
		{
			if (patchcount[x] > 1)
			{
				directory[x] = custom_p;
				custom_p[0] = 0;
				custom_p[1] = texture->height;
				custom_p[compositesize - 1] = 255;
				custom_p += compositesize;
			}
		}
	}

	for (patch = texture->patches, i = 0; i < texture->patchcount; i++, patch++)
	{
		realpatch = patchlookup[patch->patch];

		x = patch->originx;
		if (x < 0)
		{
			pcol -= x;
			x = 0;
		}
		else
			pcol = 0;

		x2 = x + realpatch->width;
		if (x2 > texture->width)
			x2 = texture->width;

		for (; x < x2; x++, pcol++)
		{
			patchcol = ((byte*)realpatch) + realpatch->coloffsets[pcol];
			if (patchcount[x] == 1)
				directory[x] = patchcol;
			else
				R_DrawCollumnInPost(patchcol, directory[x], patch->originy);
		}
	}
}

void* R_CacheColumn(maptexture_t* tex, int viscollumn)
{
	int col;

	if (tex->width >= 256)
		col = viscollumn & 255;
	else if (tex->width >= 128)
		col = viscollumn & 127;
	else if (tex->width >= 64)
		col = viscollumn & 63;
	else if (tex->width >= 32)
		col = viscollumn & 31;
	else if (tex->width >= 16)
		col = viscollumn & 15;
	else if (tex->width >= 8)
		col = viscollumn & 7;
	else if (tex->width >= 4)
		col = viscollumn & 3;
	else if (tex->width >= 2)
		col = viscollumn & 1;
	else
		col = 0;

	if (tex->collumndirectory == NULL)
		R_GenerateTexture(tex);

#ifdef ISB_LINT
	if (tex->collumndirectory == NULL)
	{
		IO_Error("R_CacheColumn: null column directory. This will have crashed earlier, but this makes Visual Studio's linter happy\n");
		return;
	}
#endif
	return tex->collumndirectory[col];
}
