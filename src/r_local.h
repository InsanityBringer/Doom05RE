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

#include "r_ref.h"

#define MAXPROCLINES 256

typedef struct procline_s
{
	struct procline_s* prev;
	struct procline_s* next;
	int side;
	int ipx1;
	int ipx2;
	fixed_t texture1;
	fixed_t texture2;
	fixed_t scale1;
	fixed_t scale2;
	fixed_t scalestep;
	fixed_t scale;
	int line;
	int chained;
	int seg;
	int sector;
} procline_t;

extern procline_t proclines[MAXPROCLINES];

#define MAXFSEGS 128

typedef struct
{
	int type;
	int startcollumn;
	fixed_t collumnstep;
	int starttopscreen;
	fixed_t topscreenstep;
	int starthighscreen;
	fixed_t highscreenstep;
	int startlowscreen;
	fixed_t lowscreenstep;
	int startbottomscreen;
	fixed_t bottomscreenstep;
	int toptextureskip;
	int bottomtextureskip;
	maptexture_t* texture;
	int bottomtexture;
} forwardseg_t;

extern forwardseg_t forwardsegs[MAXFSEGS];

typedef struct
{
	int sectornum;
	int xl;
	int xh;
	int numproclines;
	procline_t* proclines[1];
} subsector_t;

typedef struct
{
	int number;
	fixed_t xl, xh;
} drange_t;

typedef struct
{
	boolean rotate;
	short lump[8];
	byte flip[8];
} spriteframe_t;

typedef struct
{
	int numframes;
	spriteframe_t* spriteframes;
} spritedef_t;

#define MINZ (FRACUNIT*4)

//ir_
extern byte* lowcollumntable[320];
extern byte* highcollumntable[320];

extern int windowx, windowy;
extern int windoworg;

void R_RawScaleHigh();
void R_RawScaleHighC();

void R_RawScaleMed();
void R_RawScaleMedC();

void R_RawScaleLow();
void R_RawScaleLowC();

void R_MapRowHigh();
void R_MapRowHighC();

void R_MapRowMed();
void R_MapRowMedC();

void R_MapRowLow();
void R_MapRowLowC();

void R_RawScaleHighColor();
void R_MapRowHighColor();

extern int numsprites;
extern spritedef_t* sprites;
void R_DrawSprite(int xclipl, int xcliph, vissprite_t* spr);

extern int newfseg;
extern int newprocline;
extern int extralight;
extern drange_t vwalldrange;

extern procline_t proclines[];
extern forwardseg_t fsegs[];

//in bytes
#define SUBSECTORBUFFERSIZE 4096
extern byte subsectors[SUBSECTORBUFFERSIZE];
extern subsector_t* vissec;

//r_init
extern int viewheight, viewwidth;
extern int sp_x, sp_y1, sp_y2;
extern byte* sp_source;
extern int sp_colormap;
extern fixed_t sp_frac, sp_fracstep;

extern int mr_x1, mr_x2, mr_y;
extern byte* mr_picture;
extern int mr_colormap;
extern fixed_t mr_xfrac, mr_yfrac, mr_xstep, mr_ystep;

extern int naturalwidth, naturalheight;

extern void(*R_MapRow)();
extern void(*R_RawScale)();

void R_InitTables();
void R_InitLumps();
void R_InitWorld();
void R_InitTextureCache();
void R_InitVideoDevice(); //provided in ir_*.c

void R_DrawSectorThings(sector_t* sector, int xl, int xh);

//r_game

//lines
void R_SetLineTypeAndBox(int linenum);
void R_ClearProclines(void);
void R_ClearFsegs(void);
void R_MakeProcline(int line);
void R_DrawLineDrange();
void R_DrawBlockLine(int linenum, byte bits);

//planes
void R_GenerateSpans(void);
void R_PrepPlanes(void);
void R_DrawPlanes(void);

//sectors
void R_DrawSector(int sectornum, int xl, int xh, int* floorclip, int* ceilingclip, int* scaleclip);

//r_init
extern fixed_t viewfrontscale[];
extern fixed_t viewbackscale[];

extern int viewfloorpixels[];
extern int viewceilingpixels[];
