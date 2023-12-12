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
#ifndef __R_LOCAL_H__
#define __R_LOCAL_H__

//When the map structures are present, v_video structures are present before the map structures always.
#include "v_video.h"

//On-disk map data structures
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
	short lines[1];
} mapsector_t;

typedef struct
{
	short sector;
	short firstcollumn;
	short texturetop;
	short midtexture;
	short toptexture;
	short bottomtexture;
} mapside_t;

typedef struct
{
	short p1;
	short p2;
	short flags;
	short length;
	short special;
	short tag;
	mapside_t side[2];
} mapline_t;

#define ML_BLOCKING 1
#define ML_UNKNOWN2 2 //[ISB] different from release. 
#define ML_TWOSIDED 4
#define ML_DONTPEGTOP 8
#define ML_DONTPEGBOTTOM 16
#define ML_MAPPED 128 //[ISB] different from release

typedef struct
{
	fixed_t x;
	fixed_t y;
} point_t;

typedef struct
{
	int x1;
	int x2;
	fixed_t scale;
	int iscale;
	int topscreen;
	patch_t* patch;
	int colormap;
	fixed_t fracstep;
} vissprite_t;

typedef struct thing_s
{
	struct thing_s* prev;
	struct thing_s* next;
	int sector;
	vissprite_t* vissprite;
	int x;
	int y;
	int z;
	int angle;
	int sprite;
	int frame;
	int validcheck;
	int flags;
	void* specialdata;
} thing_t;

#define TF_GETTABLE				1	//thing has placed get marks
#define TF_UNUSED2				2
#define TF_PLAYER				4	//thing->specialdata points to a player_t, not an actor_t
#define TF_SOLID				8	//thing has placed block marks

typedef struct
{
	short sector;
	short firstcollumn;
	short texturetop;
	short midtexture;
	short toptexture;
	short bottomtexture;
} side_t;

typedef enum
{
	bottom,
	left,
	top,
	right
} bbox_t;

typedef enum
{
	ls_horizontal,
	ls_vertical,
	ls_slope
} lineslope_t;

typedef struct
{
	short p1;
	short p2;
	short flags;
	short length;
	short special;
	short tag;
	fixed_t bbox[4];
	lineslope_t slopetype;
	fixed_t slope;
	fixed_t yintercept;
	int validcheck;
	int procline;
	int side[2];
	int specialdata;
} line_t;

typedef struct sector_s
{
	fixed_t floorheight;
	fixed_t ceilingheight;
	short floortexture;
	short ceilingtexture;
	short lightlevel;
	short special;
	short tag;
	int audarea;
	int linecount;
	int* lines;
	int numadjacentsectors;
	struct sector_s** adjacentsectors;
	int validcheck;
	thing_t* things;
	void* specialdata;
} sector_t;

typedef struct
{
	fixed_t tx;
	fixed_t tz;
	int texture;
	fixed_t xscale;
	fixed_t yscale;
	int flags;
	int ipx;
} vertex_t;

extern int* floorpixel, * ceilingpixel;
extern int* newfloor, * newceiling;
extern int* passfloor, * passceiling;

extern int* esectorscalelight, * esectorscalelight2, * esectorscalelight3;
extern sector_t* sector;

extern byte* colormaps;
extern unsigned short* wordcolormaps;

extern int numlines;
extern line_t* lines;

extern int numsides;
extern side_t* sides;

extern int numsectors;
extern sector_t* sectors;

extern int numpoints;
extern point_t* points;

extern int nummappatches;
extern mappatch_t* mappatches;
extern patch_t** patchlookup;

extern int basetextures;
extern int numtextures;

extern int texturelookupsize;
extern maptexture_t** texturelookup;

extern fixed_t maporiginx, maporiginy;
extern int mapwidth, mapheight;
extern byte* blockmap;

#define MAPBLOCKUNITS 16
#define MAPBLOCKSIZE (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT 20

#define BMF_CHECKLINES				1 //Check lines of the thing's sector for collisions in this cell
#define BMF_SOLID					2 //This cell is solid
#define BMF_GETTABLE				4 //This cell contains a gettable item
#define BMF_MAPPED					16 //This cell will be shown on the automap

extern byte amapcolor[];

extern int viewwidth, viewheight;

extern fixed_t xscale, yscale;
extern fixed_t* inscale, * outscale;

#define MAXSPANS 832 //Total number of rows with spans. I don't know why it's higher than SCREENHEIGHT, maybe for the hires mode. 
#define NUMSPANSPERY 16 //Number of spans (individual x values) per row.

extern int* spans[];
extern int* startspans[];
extern int spanlists[];

extern fixed_t sines[];
extern fixed_t* cosines;

extern byte** flatlookup;

extern fixed_t scalelight[];

extern int viewangle;
extern fixed_t viewx, viewy, viewz;
extern int viewsector;
extern fixed_t viewsin, viewcos;

extern fixed_t xproject, yproject;
extern int sp_x, sp_y1, sp_y2;

extern int centerx, centery;
extern fixed_t centerxfrac, centeryfrac;

extern int sectorxl, sectorxh;

extern int validcheck;

extern fixed_t yslope[];
extern fixed_t cos45;

extern fixed_t viewfrontscale[];
extern fixed_t viewbackscale[];

extern int extralight;

//textures
void* R_CacheColumn(maptexture_t* tex, int col);

//things
#define	MAXVISSPRITES 128

void R_TestSprites(void);
thing_t* R_GetNewThing(int sector);
void R_RemoveThing(thing_t* rthing);
void R_ClearVisSprites(void);

void R_InitSprites(char** namelist);

//game
void R_LoadMap(char* name);

//init
void R_Startup();
void R_TransformVertex(point_t* source, vertex_t* destination);
void R_ChangeWindow(int width, int height, fixed_t vertscale);
int R_LightFromVScale(fixed_t scale);
int R_LightFromZ(fixed_t z);
void R_RenderView(int sectornum, fixed_t x, fixed_t y, fixed_t z, int angle);
void R_DrawPlayerShape(int sprite, int frame, int sx, int sy);

void R_SetViewSize(int blocks, detail_t detail, int redrawall);
void R_SetDetail(detail_t detail);
void R_SizeUp();
void R_SizeDown();
void R_StartInstanceDrawing();
int R_ClearBuffer();

#endif
