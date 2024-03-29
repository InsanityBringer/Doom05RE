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
#ifndef V_VIDEO_H
#define V_VIDEO_H

typedef struct
{
	byte width;
	byte height;
	byte data;
} pic_t;

typedef struct
{
	short height;
	byte width[256];
	short charofs[256];
} font_t;

typedef struct
{
	byte topdelta;
	byte length;
	byte data;
} post_t;

typedef post_t collumn_t;

typedef struct
{
	byte width;
	byte height;
	signed char leftoffs;
	signed char topoffset;
	short coloffsets[8];
} patch_t;

#define SCREENWIDTH 320
#define SCREENHEIGHT 200
#define SCREENBWIDE (SCREENWIDTH>>2)
#define SCREENSIZE (SCREENBWIDE * SCREENHEIGHT)

#define	PORTTILESWIDE		20      // all drawing takes place inside a
#define	PORTTILESHIGH		10		// non displayed port of this size

#define UPDATEWIDE			PORTTILESWIDE
#define UPDATEHIGH			PORTTILESHIGH

extern byte screenbuffer[];
extern byte* ylookup[];
extern int planewidthlookup[];

extern byte* collumnpointer[];
extern unsigned int ublocksource[];

extern byte update[UPDATEWIDE*UPDATEHIGH];

extern int playscreenupdateneeded;
extern int blockupdateneeded;

void V_DrawPatch(int x, int y, patch_t* patch);
void V_FadeOut(int start, int end, int red, int green, int blue, int steps);
void V_FadeIn(int start, int end, int steps, byte* palette);
int V_DrawChar(int x, int y, int ch, font_t* font);
void V_DrawPic(int x, int y, pic_t* pic);
void V_Bar(int xl, int yl, int width, int height, int color);
int V_DrawString(int sx, int sy, char* string, font_t* font);
int V_StringWidth(char* string, font_t* font);
void V_CenterString(int sy, char* string, font_t* font);
void V_Window(int width, int height);
void V_Printf(char* fmt, ...);
void V_MarkUpdateBlock(int x1, int y1, int x2, int y2);

void V_Startup();

#endif
