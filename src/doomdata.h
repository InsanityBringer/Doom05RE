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
#pragma once

typedef struct  
{
	char name[16];
	int skill;
	int score;
} hscore_t;

typedef struct  
{
	int hdetail;
	int viewsize;
	hscore_t scores[6];
} config_t;

typedef struct
{
	short x;
	short y;
} mapvertex_t;

typedef struct
{
	mapvertex_t origin;
	short ang;
	short type;
	short options;
	short sector;
} mapthing_t;

typedef struct
{
	short sector;
	short firstcollumn;
	short texturetop;
	short midtexture;
	short toptexture;
	short bottomtexture;
} side_t;

typedef struct
{
	short p1;
	short p2;
	short flags;
	short length;
	short special;
	short tag;
	side_t side[2];
} mapline_t;

#define ML_BLOCKING 1
#define ML_UNKNOWN2 2 //[ISB] different from release. 
#define ML_TWOSIDED 4
#define ML_DONTPEGTOP 8
#define ML_DONTPEGBOTTOM 16
#define ML_MAPPED 128 //[ISB] different from release

typedef struct
{
	uint8_t width;
	uint8_t height;
	int8_t leftoffs;
	int8_t topoffset;
	short coloffsets[8];
} patch_t;

typedef struct
{
	uint8_t topdelta;
	uint8_t length;
	uint8_t data;
} post_t;

typedef post_t collumn_t;

typedef struct
{
	uint8_t width;
	uint8_t height;
	uint8_t data;
} pic_t;

typedef struct 
{
	short originx;
	short originy;
	short patch;
	short stepdir;
	short colormap;
}  mappatch_t;

typedef struct 
{
	char name[8];
	int masked;
	short width;
	short height;
	void** collumndirectory;
	short patchcount;
	//[ISB] I love these hacks
	mappatch_t patches[1];
} maptexture_t;

typedef struct
{
	short floorheight;
	short ceilingheight;
	short floortexture;
	short ceilingtexture;
	short lightlevel;
	short special;
	short tag;
	short linecount;
} mapsector_t;
