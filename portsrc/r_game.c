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
#include <math.h>
#include <limits.h>

int maploaded;

int numsectors;
sector_t* sectors;

int mapheight;

int numpoints;
point_t* points;

int numpatches;
int nummappatches;
mappatch_t* mappatches;

int flatstartlump;
int patchstartlump;

fixed_t cos45;
fixed_t* cosines;

int numlines;
line_t* lines;

int basetextures;
int numtextures;

int numflats;
int nummapflats;
int* flattranslation;

fixed_t maporiginy, maporiginx;

byte** flatlookup;
patch_t** patchlookup;
maptexture_t** texturelookup;

byte* blockmap;

int texturelookupsize;

int mapwidth;

int numsides;
side_t* sides;

fixed_t scalelight[768];
fixed_t sines[10240];
fixed_t yslope[832];

byte amapcolor[256];

double round_(double value)
{
	int x = (int)(value + 0.5);
	return (double)x;
}

int R_AddTexture(maptexture_t* texture)
{
	if (numtextures == texturelookupsize)
	{
		texturelookupsize = texturelookupsize + 64;
		texturelookup = (maptexture_t**)realloc(texturelookup, texturelookupsize * sizeof(maptexture_t*));
#ifdef ISB_LINT
		if (texturelookup == NULL)
		{
			IO_Error("R_AddTexture: can't expand texture lookups");
			return;
		}
#endif
	}
	texturelookup[numtextures] = texture;
	return numtextures++;
}

void R_InitWorldTextures(void)
{
	maptexture_t* texture;
	patch_t* patch;
	int i, j;
	int* maptex;
	char name[9];
	char* names;

	name[8] = '\0';
	names = (char*)W_GetName("PNAMES");
	nummappatches = *(int*)names;
	((int*)names)++;
	patchlookup = (patch_t**)malloc(nummappatches * sizeof(patch_t*));
#ifdef ISB_LINT
	if (patchlookup == NULL)
	{
		IO_Error("R_InitWorldTextures: can't allocate patchlookup\n");
		return;
	}
#endif
	for (i = 0; i < nummappatches; i++)
	{
		strncpy(name, names + i * 8, 8);
		patchlookup[i] = W_GetName(name);
	}
	maptex = (int*)W_GetName("TEXTURES");
	basetextures = *maptex;
	numtextures = 0;
	texturelookupsize = basetextures + 64;
	texturelookup = (maptexture_t**)malloc(texturelookupsize * sizeof(maptexture_t*));
#ifdef ISB_LINT
	if (texturelookup == NULL)
	{
		IO_Error("R_InitWorldTextures: can't allocate texturelookup\n");
		return;
	}
#endif
	for (i = 0; i < basetextures; i++)
	{
		texture = (maptexture_t*)(((char*)maptex) + maptex[i + 1]);
		R_AddTexture(texture);
	}
}

void R_InitWorld(void)
{
	R_InitWorldTextures();
}

void R_InitTables(void)
{
	int i;
	int intval;
	double tang, value;
	int j;
	int level, startmap;

	for (i = 0; i < 2048; i++)
	{
		tang = (((double)i + 0.5) * 3.14159265) / 4096.0;
		value = sin(tang);

		intval = (int)round_(value * (double)FRACUNIT);
		sines[i] = intval;
		sines[i + 8192] = intval;
		sines[4095 - i] = intval;
		sines[i + 4096] = -intval;
		sines[8191 - i] = -intval;
	}

	cosines = &sines[2048];
	cos45 = sines[3072];

	for (i = 0; i < MAXSPANS; i++)
	{
		startspans[i] = &spanlists[i * NUMSPANSPERY];
	}

	for (i = 0; i < 1152; i++)
	{
		viewfrontscale[i] = 0x7fffffff;
		viewbackscale[i] = 0;
	}
	memset(forwardsegs, 0, sizeof(forwardsegs));
	memset(proclines, 0, sizeof(proclines));

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
}

void R_InitLumps(void)
{
	int i;

	flatstartlump = W_GetNumForName("F_START");
	numflats = W_GetNumForName("F_END") - flatstartlump;
	flatlookup = (byte**)malloc(numflats * sizeof(byte*));
#ifdef ISB_LINT
	if (flatlookup == NULL)
	{
		IO_Error("R_InitLumps: flatlookup is null");
		return;
	}
#endif
	for (i = 0; i < numflats; i++)
	{
		flatlookup[i] = (byte*)lumpinfo[flatstartlump + i].position;
	}
	patchstartlump = W_GetNumForName("P_START");
	numpatches = W_GetNumForName("P_END") - patchstartlump;
}

void R_LoadMapPlanes(int maplump)
{
	int* names;
	char name[9];
	int i;

	name[8] = '\0';
	names = (int*)W_GetLump(maplump + 1);
	nummapflats = *names++;

	flattranslation = Z_Malloc(playzone, nummapflats * sizeof(int*));

	for (i = 0; i < nummapflats; i++)
	{
		strncpy(&name[0], (char*)(names + i * 2), 8);
		flattranslation[i] = W_GetNumForName(name) - flatstartlump;
	}
}

void R_LoadMapThings(int maplump)
{
	int i;
	int* mapthings;
	mapthing_t* mapthing;
	int numthings;

	mapthings = (int*)W_GetLump(maplump + 5);
	numthings = *mapthings++;

	for (mapthing = (mapthing_t*)mapthings, i = 0; i < numthings; i++, mapthing++)
	{
		P_InitThing(mapthing);
	}
}

void R_LoadMapPoints(int maplump)
{
	int i;
	mapvertex_t* mp;
	point_t* pt;

	mp = (mapvertex_t*)W_GetLump(maplump + 2);
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
}

void R_LoadMapLines(int maplump)
{
	int i, s;
	mapline_t* ml;
	mapline_t* maplines;
	line_t* li;
	mapside_t* ms;
	side_t* si;

	maplines = (mapline_t*)W_GetLump(maplump + 3);
	numlines = *(int*)maplines;
	((int*)maplines)++;
	numsides = 0;

	for (ml = maplines, i = 0; i < numlines; i++, ml++)
	{
		if (ml->flags & ML_TWOSIDED)
			numsides += 2;
		else
			numsides++;
	}

	lines = (line_t*)Z_Malloc(playzone, numlines * sizeof(line_t));
	sides = (side_t*)Z_Malloc(playzone, numsides * sizeof(side_t));

	ml = maplines;
	li = lines;
	si = sides;
	for (i = 0; i < numlines; i++)
	{
		li->p1 = ml->p1;
		li->p2 = ml->p2;
		li->special = ml->special;
		li->tag = ml->tag;
		li->flags = ml->flags;
		li->length = ml->length;
		R_SetLineTypeAndBox(i);
		for (s = 0; s <= ((ml->flags & ML_TWOSIDED) ? 1 : 0); s++, si++)
		{
			li->side[s] = (int)(si - sides);
			ms = &ml->side[s];
			si->sector = ms->firstcollumn;
			si->firstcollumn = ms->texturetop;
			si->midtexture = ms->midtexture;
			si->toptexture = ms->toptexture;
			si->bottomtexture = ms->bottomtexture;
		}
		ml++;
		li++;
	}
}

void R_LoadMapSectors(int maplump)
{
	int i, j;
	int* mapsectors;
	mapsector_t* me;
	sector_t* en;

	mapsectors = (int*)W_GetLump(maplump + 4);
	numsectors = *mapsectors;
	sectors = (sector_t*)Z_Malloc(playzone, numsectors * sizeof(sector_t));

	en = sectors;
	for (i = 0; i < numsectors; i++)
	{
		me = (mapsector_t*)(((char*)mapsectors) + mapsectors[i + 1]);
		en->floorheight = me->floorheight << FRACBITS;
		en->ceilingheight = me->ceilingheight << FRACBITS;
		en->floortexture = flattranslation[me->floortexture];
		en->ceilingtexture = flattranslation[me->ceilingtexture];
		en->lightlevel = me->lightlevel;
		en->special = me->special;
		en->tag = me->tag;
		en->linecount = me->linecount;
		en->lines = Z_Malloc(playzone, en->linecount * sizeof(int));
		en->things = NULL;
		en->specialdata = NULL;
		for (j = 0; j < en->linecount; j++)
			en->lines[j] = (int)me->lines[j];

		en++;
	}
}

void R_InitBlockMap(void)
{
	int i, x, y;
	int left, right, top, bottom;

	left = INT_MAX;
	right = INT_MIN;
	top = INT_MIN;
	bottom = INT_MAX;

	for (i = 0; i < numpoints; i++)
	{
		x = points[i].x;
		y = points[i].y;
		if (x < left)
			left = x;

		if (x > right)
			right = x;

		if (y < bottom)
			bottom = y;

		if (y > top)
			top = y;
	}

	maporiginx = (left >> MAPBLOCKSHIFT) * MAPBLOCKSIZE;
	maporiginy = (bottom >> MAPBLOCKSHIFT) * MAPBLOCKSIZE;
	mapwidth = ((right >> MAPBLOCKSHIFT) * MAPBLOCKSIZE + (left >> MAPBLOCKSHIFT) * -MAPBLOCKSIZE >> MAPBLOCKSHIFT) + 1;
	mapheight = ((top >> MAPBLOCKSHIFT) * MAPBLOCKSIZE + (bottom >> MAPBLOCKSHIFT) * -MAPBLOCKSIZE >> MAPBLOCKSHIFT) + 1;
	blockmap = Z_Malloc(playzone, mapwidth * mapheight);
	memset(blockmap, 0, mapwidth * mapheight);

	for (i = 0; i < 255; i++)
	{
		if ((i & BMF_MAPPED) == 0)
			amapcolor[i] = 205;
		else
			amapcolor[i] = 200;
	}

	for (i = 0; i < numlines; i++)
		R_DrawBlockLine(i, BMF_CHECKLINES);
}

void R_UnloadMap()
{
	int i;
	maptexture_t* tex;

	for (i = basetextures; i < numtextures; i++)
	{
		tex = texturelookup[i];
		if (tex->collumndirectory != NULL)
			Z_CacheFree((void**)tex);
	}
	maploaded = 0;
	numsectors = 0;
	numpoints = 0;
	numlines = 0;
	numtextures = basetextures;
	nummapflats = 0;
	numsides = 0;
}

void R_LoadMap(char* name)
{
	int maplump;

	if (maploaded != 0)
	{
		R_UnloadMap();
	}
	maplump = W_GetNumForName(name);
	R_LoadMapPlanes(maplump);
	R_LoadMapPoints(maplump);
	R_LoadMapLines(maplump);
	R_LoadMapSectors(maplump);
	R_InitBlockMap();
	R_LoadMapThings(maplump);
	P_SpawnSpecialSectors();
	maploaded = 1;
	extralight = 0;
}
