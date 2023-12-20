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

#include <stdarg.h>
#include "doomdef.h"
#include "v_video.h"

int blockupdateneeded;
int playscreenupdateneeded;

font_t* printfont; //[ISB] Never initialized, so V_Printf won't work.

int winyl, winyh;
int winxl, winxh;

int cursorx, cursory;

byte update[UPDATEWIDE*UPDATEHIGH];
byte screenbuffer[SCREENWIDTH*SCREENHEIGHT];
unsigned int ublocksource[UPDATEWIDE*UPDATEHIGH];
byte* ylookup[SCREENHEIGHT*4];
int planelookup[SCREENWIDTH];
int planewidthlookup[SCREENHEIGHT];
byte* collumnpointer[SCREENWIDTH];

void V_FillPalette(int r, int g, int b)
{
	int i;
	byte pal[768];

	for (i = 0; i < 768; i += 3)
	{
		pal[i + 0] = r;
		pal[i + 1] = g;
		pal[i + 2] = b;
	}

	IO_SetPalette(pal);
}

void V_FadeOut(int start, int end, int red, int green, int blue, int steps)
{
	byte basep[768];
	byte work[768];
	int i, j;
	int delta;

	IO_GetPalette(&basep[0]);
	for (i = 0; i <= steps; i++)
	{
		for (j = start; j <= end; j++)
		{
			delta = red - basep[j * 3 + 0];
			work[j * 3 + 0] = basep[j * 3 + 0] + delta * i / steps;
			delta = green - basep[j * 3 + 1];
			work[j * 3 + 1] = basep[j * 3 + 1] + delta * i / steps;
			delta = blue - basep[j * 3 + 2];
			work[j * 3 + 2] = basep[j * 3 + 2] + delta * i / steps;
		}
		IO_SetPalette(&work[0]);
#ifndef __WATCOMC__
		IO_DoEvents(); //[ISB]
#endif
	}
}

void V_FadeIn(int start, int end, int steps, byte* palette)
{
	byte basep[768];
	byte work[768];
	int i, j;
	int delta;

	IO_GetPalette(&basep[0]);
	start *= 3;
	end *= 3;
	for (i = 0; i < steps; i++)
	{
		for (j = start; j <= end; j++)
		{
			delta = palette[j] - basep[j];
			work[j] = basep[j] + delta * i / steps;
		}
		IO_SetPalette(&work[0]);
#ifndef __WATCOMC__
		IO_DoEvents(); //[ISB]
#endif
	}
	IO_SetPalette(&palette[0]);
}

void V_Bar(int x1, int y1, int width, int height, int color)
{
	int p;
	int y;
	int px1, px2;
	int x2, y2;
	int count;
	byte* dest;

	x2 = x1 + width;
	y2 = y1 + height;
	for (p = 0; p < 4; p++)
	{
		px1 = (3 - p) + x1 >> 2;
		px2 = (3 - p) + x2 >> 2;
		dest = ylookup[p * SCREENHEIGHT + y1] + px1;
		count = px2 - px1;
		for (y = y1; y < y2; y++)
		{
			memset(dest, color, count);
			dest += SCREENBWIDE;
		}
	}
	V_MarkUpdateBlock(x1, y1, x2 + -1, y2 + -1);
}

void V_DrawPic(int x, int y, pic_t* pic)
{
	int yy;
	byte* screen, * source, * dest;
	int plane, planey;
	int width, height;

	width = pic->width;
	height = pic->height;
	source = &pic->data;

	V_MarkUpdateBlock(x, y, width * 4 + x - 1, y + height - 1);

	planey = (x & 3) * SCREENHEIGHT;
	x >>= 2;
	for (plane = 0; plane < 4; plane++)
	{
		dest = ylookup[planey + y] + x;
		screen = dest; //[ISB] I don't know why this is done, but the stack usage implies this.
		for (yy = 0; yy < height; yy++)
		{
			memcpy(screen, source, width);
			screen += SCREENBWIDE;
			source += width;
		}

		planey += SCREENHEIGHT;

		if (planey == SCREENHEIGHT*4)
		{
			planey = 0;
			x++;
		}
	}
}

int V_DrawChar(int sx, int sy, int ch, font_t* font)
{
	byte pix;
	int x, y, width, plane;
	byte* source, * dest;
	int backup;

	source = ((byte*)&font->height) + font->charofs[ch];
	width = font->width[ch];
	x = width;
	dest = ylookup[sy] + (sx / 4);
	plane = planelookup[sx];
	backup = font->height * SCREENBWIDE;
	while (x-- != 0)
	{
		y = font->height;
		while (y-- != 0) 
		{
			pix = *source;
			if (pix != 0)
				dest[plane] = pix;
			
			dest += SCREENBWIDE;
			source++;
		}
		dest -= backup;

		if (plane == SCREENSIZE*3)
		{
			dest++;
			plane = 0;
		}
		else 
		{
			plane += SCREENSIZE;
		}
	}
	return width;
}

int V_DrawString(int sx, int sy, char* string, font_t* font)
{
	int tx = sx;
	while (*string != '\0') 
	{
		tx += V_DrawChar(tx, sy, *string, font) + 1;
		string++;
	}
	V_MarkUpdateBlock(sx, sy, tx, font->height + sy - 1);
	return tx;
}

int V_StringWidth(char* string, font_t* font)
{
	int width = 0;
	while (*string != '\0') 
		width += font->width[*string++] + 1;

	return width;
}

void V_CenterString(int y, char* string, font_t* font)
{
	int w, x;

	w = V_StringWidth(string, font);
	x = SCREENWIDTH / 2 - w / 2;
	V_DrawString(x, y, string, font);
}

void V_Window(int width, int height)
{
	winxl = SCREENWIDTH / 2 - width / 2;
	winxh = winxl + width;
	winyl = SCREENHEIGHT / 2 - height / 2;
	winyh = winyl + height;

	V_Bar(winxl, winyl, width, height, 198);
	V_Bar(winxl + 6, winyl + 6, width - 12, height - 12, 3);

	winxl = winxl + 8;
	winxh = winxh + -8;
	winyl = winyl + 8;
	winyh = winyh + -8;
	cursory = winyl;
	cursorx = winxl;
}

void V_Printf(char* fmt, ...)
{
	va_list argptr;
	char string[1024];

	va_start(argptr, fmt);
	vsprintf(string, fmt, argptr);
	va_end(argptr);

	cursorx += V_DrawString(cursorx, cursory, string, printfont);
}

void V_MarkUpdateBlock(int x1, int y1, int x2, int y2)
{
	int	x, y, xt1, yt1, xt2, yt2, nextline;
	byte* mark;

	if (x1 < 0 || x2 >= SCREENWIDTH * 4 || y1 < 0 || y2 >= SCREENHEIGHT)
		IO_Error("bad V_MarkUpdateBlock(%i,%i,%i,%i)", x1, y1, x2, y2);

	xt1 = x1 / (SCREENWIDTH/UPDATEWIDE);
	yt1 = y1 / (SCREENHEIGHT/UPDATEHIGH);

	xt2 = x2 / (SCREENWIDTH/UPDATEWIDE);
	yt2 = y2 / (SCREENHEIGHT/UPDATEHIGH);

	mark = &update[xt1 + yt1 * UPDATEWIDE];
	nextline = UPDATEWIDE - (xt2 - xt1) - 1;

	for (y=yt1;y<=yt2;y++)
	{
		for (x=xt1;x<=xt2;x++)
			*mark++ = 2;
		
		mark += nextline;
	}

	blockupdateneeded = 2;
}

void V_Startup(void)
{
	int x, y, i;
	unsigned int* usrc;

	for (y = 0; y < SCREENHEIGHT * 4; y++)
	{
		ylookup[y] = screenbuffer + y * SCREENBWIDE;
	}
	for (i = 0; i < SCREENHEIGHT; i++)
	{
		planewidthlookup[i] = i * SCREENBWIDE;
	}
	for (i = 0; i < SCREENWIDTH; i++) 
	{
		collumnpointer[i] = &screenbuffer[0] + (i >> 2) + ((i & 3) * SCREENSIZE);
	}

	usrc = ublocksource;
	for (y = 0; y < UPDATEHIGH; y++)
	{
		for (x = 0; x < UPDATEWIDE; x++)
		{
			*usrc++ = y * SCREENBWIDE * UPDATEWIDE + x * 4;
		}
	}
	memset(update, 0, sizeof(update));
	playscreenupdateneeded = 0;
	blockupdateneeded = 0;
	for (x = 0; x < SCREENWIDTH; x++)
	{
		planelookup[x] = (x & 3) * SCREENSIZE;
	}
}
