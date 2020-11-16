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

//ir_
extern uint8_t* lowcollumntable[320];
extern uint8_t* highcollumntable[320];

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

//r_init
extern int viewheight, viewwidth;
extern int sp_x, sp_y1, sp_y2;
extern uint8_t* sp_source;
extern int sp_colormap;
extern fixed_t sp_frac, sp_fracstep;

extern int mr_x1, mr_x2, mr_y;
extern uint8_t* mr_picture;
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

//r_init
extern fixed_t viewfrontscale[];
extern fixed_t viewbackscale[];

extern int viewfloorpixels[];
extern int viewceilingpixels[];
