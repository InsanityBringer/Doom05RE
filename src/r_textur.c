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
	return;
}

void P_RemovePatch(int wall, patch_t* patch, int x, int y)
{
	return;
}

void R_DrawCollumnInPost(post_t* post, uint8_t* cache, int originy)
{
	int local_20;
	unsigned int count;
	int local_18;
	int local_28;

	local_20 = cache + 2;
	local_28 = cache;
	while (post->topdelta != 0xff) 
	{
		count = (unsigned int)post->length;
		local_18 = originy + (unsigned int)post->topdelta;
		if (local_18 < 0) 
		{
			count = count + local_18;
			local_18 = 0;
		}
		if ((int)(unsigned int) * (uint8_t*)(local_28 + 1) < (int)(local_18 + count)) 
		{
			count = (unsigned int) * (uint8_t*)(local_28 + 1) - local_18;
		}
		if (0 < (int)count) 
		{
			memcpy((void*)(local_20 + local_18), &post->data, count);
		}
		post = (patch_t*)(&post->data + post->length);
	}
	return;
}

void R_GenerateTexture(maptexture_t* texture)
{
	short sVar1;
	void* cache;
	uint8_t* mem;
	unsigned int uVar2;
	unsigned int extraout_EDX;
	int size;
	void* patch;
	void*** user;

	//this may be the function the decompiler had the most issues with, nice.
	uint8_t* local_54_1;
	maptexture_t* local_54_2;
	int local_4c;
	void* local_48;
	int local_44;
	void** local_40;
	void** local_3c;
	int local_38;
	int local_34;
	int local_30;
	int local_2c;
	int local_28;
	patch_t* local_24;
	mappatch_t* local_20;
	uint8_t* puStack28;

	local_54_2 = texture;
	//uVar2 = watcom_stack386((int)local_EAX_5->width + 3U & 0xfffffffc);

	//This apppears to be a stack allocation. Replace with some alloca nonsense if possible
	/*if (extraout_EDX < uVar2) 
	{
		size = -((int)local_54_2->width + 3U & 0xfffffffc);
		puVar3 = (uint8_t*)(local_54 + size);
		local_54[0] = (uint8_t*)(local_54 + size);
	}
	else {
		local_54[0] = (undefined*)0x0;
	}*/

	//Amount of posts in each column of the texture. Must be >0, no windows allowed.
	local_54_1 = alloca((local_54_2->width + 3) * sizeof(uint8_t));

	mem = local_54_1;
	puStack28 = local_54_1;
	sVar1 = local_54_2->width;
	//*(int*)(puVar3 + -4) = 0x289c1;
	memset(mem, 0, (int)sVar1);

	local_44 = 0;
	local_20 = (mappatch_t*)& local_54_2->patches;
	while (local_44 < local_54_2->patchcount) 
	{
		if (nummappatches <= local_20->patch) 
		{
			IO_Error("R_GenerateTexture: bad patch number\n");
		}
		local_24 = patchlookup[local_20->patch];
		local_2c = (int)local_20->originx;
		if ((int)local_20->originx < 0) 
		{
			local_2c = 0;
		}
		local_30 = local_2c + patchlookup[local_20->patch]->width;
		if (local_54_2->width < local_30) 
		{
			local_30 = (int)local_54_2->width;
		}
		while (local_2c < local_30) 
		{
			puStack28[local_2c]++;
			local_2c = local_2c + 1;
		}
		local_44 = local_44 + 1;
		local_20 = local_20 + 1;
	}
	local_28 = 0;
	local_2c = 0;
	while (local_2c < local_54_2->width) 
	{
		if (puStack28[local_2c] == 0) 
		{
			IO_Error("R_GenerateTexture: collumn without a patch\n");
		}
		if (1 < (uint8_t)puStack28[local_2c])
		{
			local_28 = local_28 + 1;
		}
		local_2c = local_2c + 1;
	}
	local_38 = (int)local_54_2->height + 3;
	//This monstrosity is:
	//The width of the texture, times the amount of columns with more than one patch, times the texture height + 3 (to store the starts of posts)
	//This stores a directory of all columns, with excess data to allow for holding the multipatch data.
	//I can understand why it was abandoned in the final game.
	size = (int)local_54_2->width * sizeof(uint8_t*) + local_28 * local_38;
	local_4c = size;
	user = &local_54_2->collumndirectory;
	Z_CacheMalloc(texturezone, size, user);
	local_3c = local_54_2->collumndirectory;

	if (local_28 != 0) 
	{
		local_40 = local_3c + local_54_2->width;
		local_2c = 0;

		//Find all the columns with more than one patch, and set their pointers in the column directory correctly
		while (local_2c < local_54_2->width) 
		{
			if (1 < (uint8_t)puStack28[local_2c]) 
			{
				//TODO: For now I'm just gonna trust this code works
				*(void***)(local_3c + local_2c) = local_40;
				*(uint8_t*)local_40 = 0;
				*(uint8_t*)((int)local_40 + 1) = *(uint8_t*)& local_54_2->height;
				*(uint8_t*)((int)local_40 + local_38 + -1) = 0xff;
				local_40 = (void**)((int)local_40 + local_38);
			}
			local_2c = local_2c + 1;
		}
	}
	local_44 = 0;
	local_20 = (mappatch_t*)& local_54_2->patches;
	while (local_44 < local_54_2->patchcount) 
	{
		local_24 = patchlookup[local_20->patch];
		local_2c = (int)local_20->originx;
		if (local_2c < 0)
		{
			local_34 = local_34 - local_2c;
			local_2c = 0;
		}
		else 
		{
			local_34 = 0;
		}
		local_30 = local_2c + patchlookup[local_20->patch]->width;
		if (local_54_2->width < local_30) 
		{
			local_30 = (int)local_54_2->width;
		}
		while (local_2c < local_30) 
		{
			patch = (void*)((int)&local_24->width + local_24->coloffsets[local_34]);
			local_48 = patch;

			//Column caching. This is super reliant on the lack of WAD lump caching, so no surprise it's discarded.
			if (puStack28[local_2c] == 1) //only one post on this column, set the pointer for this column to the post memory
			{
				local_3c[local_2c] = patch;
			}
			else //more patches in this column, need to draw all the patches into this column
			{
				sVar1 = local_20->originy;
				cache = local_3c[local_2c];
				R_DrawCollumnInPost(patch, cache, (int)sVar1);
			}
			local_2c = local_2c + 1;
			local_34 = local_34 + 1;
		}
		local_44 = local_44 + 1;
		local_20 = local_20 + 1;
	}
	return;
}

void* R_CacheColumn(maptexture_t* tex, int col)
{
	//[ISB] let's preserve my favorite if pyramid from ghidra...
	if (tex->width < 0x100)
	{
		if (tex->width < 0x80)
		{
			if (tex->width < 0x40)
			{
				if (tex->width < 0x20)
				{
					if (tex->width < 0x10)
					{
						if (tex->width < 8)
						{
							if (tex->width < 4)
							{
								if (tex->width < 2)
								{
									col = 0;
								}
								else
								{
									col = col & 1;
								}
							}
							else
							{
								col = col & 3;
							}
						}
						else
						{
							col = col & 7;
						}
					}
					else
					{
						col = col & 0xf;
					}
				}
				else
				{
					col = col & 0x1f;
				}
			}
			else
			{
				col = col & 0x3f;
			}
		}
		else
		{
			col = col & 0x7f;
		}
	}
	else
	{
		col = col & 0xff;
	}

	if (tex->collumndirectory == NULL)
	{
		R_GenerateTexture(tex);
	}

#ifdef ISB_LINT
	if (tex->collumndirectory == NULL)
	{
		IO_Error("R_CacheColumn: null column directory. This will have crashed earlier, but this makes Visual Studio's linter happy\n");
		return;
	}
#endif
	return tex->collumndirectory[col];
}
