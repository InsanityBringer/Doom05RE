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
#include <math.h>
#include <limits.h>

fixed_t sines[10240];
fixed_t* cosines;
fixed_t cos45;

int maploaded;

int numlines;
line_t* lines;

int numsectors;
sector_t* sectors;

int numsides;
side_t* sides;

fixed_t scalelight[768];

int numpoints;
point_t* points;

int flatstartlump;
int patchstartlump;

fixed_t maporiginx, maporiginy;
int mapwidth, mapheight;
uint8_t* blockmap;

uint8_t** flatlookup;

int numflats;
int numpatches;

procline_t proclines[256];
forwardseg_t forwardsegs[128];

int nummappatches;
mappatch_t* mappatches;
patch_t** patchlookup;

int basetextures;
int numtextures;

int texturelookupsize;
maptexture_t** texturelookup;

int nummapflats;
int* flattranslation;

uint8_t amapcolor[256];

void R_InitLumps(void)
{
	int iVar1;

	flatstartlump = W_GetNumForName("F_START");
	numflats = W_GetNumForName("F_END");
	numflats -= flatstartlump;
	flatlookup = (uint8_t**)malloc(numflats * sizeof(uint8_t*));
	//[ISB]
#ifdef ISB_LINT
	if (flatlookup == NULL)
	{
		IO_Error("R_InitLumps: flatlookup is null");
		return;
	}
#endif
	for (iVar1 = 0; iVar1 < numflats; iVar1++)
	{
		flatlookup[iVar1] = (uint8_t*)lumpinfo[flatstartlump + iVar1].position;
	}
	patchstartlump = W_GetNumForName("P_START");
	iVar1 = W_GetNumForName("P_END");
	numpatches = iVar1 - patchstartlump;
	return;
}

void R_LoadMapPlanes(int lump)
{
	int iVar1;
	int* local_20;
	int iVar2;
	char buf[8];

	local_20 = (int*)W_GetLump(lump + 1);
	nummapflats = *local_20;
	local_20 = local_20 + 1;
	flattranslation = Z_Malloc(playzone, nummapflats * sizeof(int*));
	iVar2 = 0;
	while (iVar2 < nummapflats) 
	{
		strncpy(&buf[0], (char*)(local_20 + iVar2 * 2), 8);
		iVar1 = W_GetNumForName(buf);
		flattranslation[iVar2] = iVar1 - flatstartlump;
		iVar2 = iVar2 + 1;
	}
	return;
}


void R_LoadMapThings(int lump)
{
	int* mapthings;
	int numthings;
	mapthing_t* mapthing;
	int i;

	mapthings = (int*)W_GetLump(lump + 5);
	numthings = *mapthings;
	mapthing = (mapthing_t*)(mapthings + 1);
	for (i = 0; i < numthings; i++)
	{
		P_InitThing(mapthing);
		//[ISB] TODO: Alignment safe some of this code. Or just disable alignment...
		mapthing++;
	}
	return;
}

void R_LoadMapPoints(int maplump)
{
	mapvertex_t* mp;
	point_t* pt;
	int i;

	mp = (int32_t*)W_GetLump(maplump + 2);
	numpoints = *(int*)mp;
	((int*)mp)++;
	points = (point_t*)Z_Malloc(playzone, numpoints * sizeof(point_t));
	pt = points;
	for (i = 0; i < numpoints; i++)
	{
		pt->x = (fixed_t)mp->x << FRACBITS;
		pt->y = (fixed_t)mp->y << FRACBITS;
		pt++;
		mp++;
	}
	return;
}

void R_LoadMapLines(int maplump)
{
	int* piVar1;
	side_t* psVar2;
	unsigned int local_3c;
	side_t* local_34;
	line_t* local_2c;
	mapline_t* local_24;
	int local_20;
	int iStack28;

	piVar1 = (int*)W_GetLump(maplump + 3);
	numlines = *piVar1;
	numsides = 0;
	iStack28 = 0;
	local_24 = (mapline_t*)(piVar1 + 1);
	while (iStack28 < numlines) 
	{
		if (!(local_24->flags & 4)) 
		{
			numsides += 1;
		}
		else 
		{
			numsides += 2;
		}
		iStack28 = iStack28 + 1;
		local_24 = local_24 + 1;
	}
	lines = (line_t*)Z_Malloc(playzone, numlines * sizeof(line_t));
	sides = (side_t*)Z_Malloc(playzone, numsides * sizeof(side_t));
	local_2c = lines;
	iStack28 = 0;
	local_34 = sides;
	local_24 = (mapline_t*)(piVar1 + 1);
	while (iStack28 < numlines) 
	{
		local_2c->p1 = local_24->p1;
		local_2c->p2 = local_24->p2;
		local_2c->special = local_24->special;
		local_2c->tag = local_24->tag;
		local_2c->flags = local_24->flags;
		local_2c->length = local_24->length;
		R_SetLineTypeAndBox(iStack28);
		local_20 = 0;
		while (local_3c = ((local_2c->flags & 4U) != 0), local_20 <= (int)local_3c) 
		{
			local_2c->side[local_20] = (int)(local_34 - sides);
			psVar2 = local_24->side + local_20;
			local_34->sector = psVar2->firstcollumn;
			local_34->firstcollumn = psVar2->texturetop;
			local_34->midtexture = psVar2->midtexture;
			local_34->toptexture = psVar2->toptexture;
			local_34->bottomtexture = psVar2->bottomtexture;
			local_20 = local_20 + 1;
			local_34 = local_34 + 1;
		}
		local_24 = local_24 + 1;
		local_2c = local_2c + 1;
		iStack28 = iStack28 + 1;
	}
	return;
}

void R_LoadMapSectors(int maplump)
{
	int* piVar1;
	sector_t* local_2c;
	short* local_28;
	int* local_24;
	int local_20;
	int iVar2;

	local_24 = (int*)W_GetLump(maplump + 4);
	numsectors = *local_24;
	sectors = (sector_t*)Z_Malloc(playzone, numsectors * sizeof(sector_t));
	iVar2 = 0;
	local_2c = sectors;
	//[ISB] I'll clean this hideous mess up later, since the variable data length structures make this a very Fun task.
	while (iVar2 < numsectors) 
	{
		local_28 = (short*)((int)local_24 + local_24[iVar2 + 1]);
		local_2c->floorheight = (int)* local_28 << FRACBITS;
		local_2c->ceilingheight = (int)local_28[1] << FRACBITS;
		local_2c->floortexture = (short)flattranslation[(int)local_28[2]];
		local_2c->ceilingtexture = (short)flattranslation[(int)local_28[3]];
		local_2c->lightlevel = local_28[4];
		local_2c->special = local_28[5];
		local_2c->tag = local_28[6];
		local_2c->linecount = (int)local_28[7];
		piVar1 = (int*)Z_Malloc(playzone, local_2c->linecount * sizeof(line_t*));
		local_2c->lines = piVar1;
		local_2c->things = (thing_t*)NULL;
		local_2c->specialdata = 0;
		local_20 = 0;
		while (local_20 < local_2c->linecount) 
		{
			local_2c->lines[local_20] = (int)local_28[local_20 + 8];
			local_20 = local_20 + 1;
		}
		local_2c++;
		iVar2++;
	}
	return;
}

void R_InitTables(void)
{
	int level;
	int startmap;
	float value;
	fixed_t intval;
	int i, j;

	for (i = 0; i < 2048; i++)
	{
		value = sin((((double)i + 0.5) * 3.14159265) / 4096.0);

		//[ISB] round is c99, needs replacement if backporting
		intval = (int)round(value * 65536.0);
		sines[i] = intval;
		sines[i + 8192] = intval;
		sines[4095 - i] = intval;
		sines[i + 4096] = -intval;
		sines[8191 - i] = -intval;
	}
	cosines = &sines[2048];
	cos45 = sines[3072];
	
	for (i = 0; i < 832; i++)
	{
		startspans[i] = &spanlists[i * 16];
	}
	
	for (i = 0; i < 1152; i++)
	{
		viewfrontscale[i] = 0x7fffffff;
		viewbackscale[i] = 0;
	}
	memset((char*)forwardsegs, 0, sizeof(forwardsegs));
	memset((char*)proclines, 0, sizeof(proclines));
	for (i = 0; i < 16; i++)
	{
		level = (15 - i) * 64;
		for (j = 0; j < 48; j++)
		{
			startmap = level / 16 - j / 2;
			if (startmap < 0)
			{
				startmap = 0;
			}
			if (startmap > 31)
			{
				startmap = 31;
			}
			scalelight[i * 48 + j] = startmap;
		}
	}
	return;
}


void R_InitBlockMap(void)
{
	int iVar1;
	int iVar2;
	int local_34;
	int local_30;
	int local_2c;
	int local_28;
	int linenum;
	unsigned int uVar3;

	local_28 = INT_MAX;
	local_2c = INT_MIN;
	local_30 = INT_MIN;
	local_34 = INT_MAX;
	linenum = 0;
	while (linenum < numpoints) 
	{
		iVar1 = points[linenum].x;
		iVar2 = points[linenum].y;
		if (iVar1 < local_28) 
		{
			local_28 = iVar1;
		}
		if (local_2c < iVar1) 
		{
			local_2c = iVar1;
		}
		if (iVar2 < local_34) 
		{
			local_34 = iVar2;
		}
		if (local_30 < iVar2) 
		{
			local_30 = iVar2;
		}
		linenum = linenum + 1;
	}
	maporiginx = (local_28 >> 0x14) * 0x100000;
	maporiginy = (local_34 >> 0x14) * 0x100000;
	mapwidth = ((local_2c >> 0x14) * 0x100000 + (local_28 >> 0x14) * -0x100000 >> 0x14) + 1;
	mapheight = ((local_30 >> 0x14) * 0x100000 + (local_34 >> 0x14) * -0x100000 >> 0x14) + 1;
	blockmap = Z_Malloc(playzone, mapwidth * mapheight);
	memset((char*)blockmap, 0, mapwidth * mapheight);
	uVar3 = 0;
	while ((int)uVar3 < 0xff) 
	{
		if ((uVar3 & 0x10) == 0) 
		{
			amapcolor[uVar3] = 0xcd;
		}
		else 
		{
			amapcolor[uVar3] = 200;
		}
		uVar3++;
	}
	linenum = 0;
	while (linenum < numlines)
	{
		R_DrawBlockLine(linenum, '\x01');
		linenum++;
	}
	return;
}

void R_AddTexture(maptexture_t* texture)
{
	maptexture_t** newtexturelookup;

	if (numtextures == texturelookupsize) 
	{
		texturelookupsize = texturelookupsize + 64;
		newtexturelookup = (maptexture_t**)realloc(texturelookup, texturelookupsize * sizeof(maptexture_t*));
#ifdef ISB_LINT
		if (newtexturelookup == NULL)
		{
			IO_Error("R_AddTexture: can't expand texture lookups");
			return;
		}
#endif
		texturelookup = newtexturelookup;
	}
	texturelookup[numtextures] = texture;
	numtextures++;
}

void R_InitWorldTextures(void)
{
	patch_t* patch;
	char* names;
	int i;
	char name[8];
	int* textures;

	names = (char*)W_GetName("PNAMES");
	nummappatches = *(int*)names;
	((int*)names)++;
	patchlookup = (mappatch_t * *)malloc(nummappatches * sizeof(mappatch_t*));
#ifdef ISB_LINT
	if (patchlookup == NULL)
	{
		IO_Error("R_InitWorldTextures: can't allocate patchlookup\n");
		return;
	}
#endif
	for (i = 0; i < nummappatches; i++)
	{
		strncpy(name, (char*)(names + i * 8), 8);
		patch = (patch_t*)W_GetName(name);
		patchlookup[i] = patch;
	}
	textures = (int*)W_GetName("TEXTURES");
	basetextures = *textures;
	numtextures = 0;
	texturelookupsize = basetextures + 64;
	texturelookup = (maptexture_t * *)malloc(texturelookupsize * sizeof(maptexture_t*));
#ifdef ISB_LINT
	if (texturelookup == NULL)
	{
		IO_Error("R_InitWorldTextures: can't allocate texturelookup\n");
		return;
	}
#endif
	for (i = 0; i < basetextures; i++) 
	{
		R_AddTexture((maptexture_t*)(((char*)textures) + textures[i + 1]));
	}
	return;
}

void R_UnloadMap()
{
	int iVar1;

	iVar1 = basetextures;
	while (iVar1 < numtextures) 
	{
		if (texturelookup[iVar1]->collumndirectory != (void**)0x0) 
		{
			Z_CacheFree(texturelookup[iVar1]);
		}
		iVar1 = iVar1 + 1;
	}
	maploaded = 0;
	numsectors = 0;
	numpoints = 0;
	numlines = 0;
	numtextures = basetextures;
	nummapflats = 0;
	numsides = 0;

	return;
}

void R_LoadMap(char* name)
{
	int maplump;
	char* mapname;

	mapname = name;
	if (maploaded != 0) 
	{
		R_UnloadMap();
	}
	maplump = W_GetNumForName(mapname);
	R_LoadMapPlanes(maplump);
	R_LoadMapPoints(maplump);
	R_LoadMapLines(maplump);
	R_LoadMapSectors(maplump);
	R_InitBlockMap();
	R_LoadMapThings(maplump);
	P_SpawnSpecialSectors();
	maploaded = 1;
	extralight = 0;
	return;
}

void R_InitWorld(void)
{
	R_InitWorldTextures();
}

