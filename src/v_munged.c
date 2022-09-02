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
#include "v_video.h"

byte screenbuffer[64000*4];
byte* ylookup[800];
int planelookup[320];
int planewidthlookup[200];

byte* collumnpointer[320];
unsigned int ublocksource[200];

byte update[200];

int playscreenupdateneeded;
int blockupdateneeded;

int winxl, winxh;
int winyl, winyh;
int cursorx, cursory;

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
		IO_DoEvents(); //[ISB]
	}
	return;
}

void V_FadeIn(int start, int end, int steps, byte* palette)
{
	int i, j;
	int delta;

	byte basep[768];
	byte work[768];

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
		IO_DoEvents(); //[ISB]
	}
	IO_SetPalette(&palette[0]);
	return;
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
		dest = ylookup[p * 200 + y1] + px1;
		px2 = (3 - p) + x2 >> 2;
		count = px2 - px1;
		for (y = y1; y < y2; y++)
		{
			memset(dest, color, count);
			dest += SCREENBWIDE;
		}
	}
	V_MarkUpdateBlock(x1, y1, x2 + -1, y2 + -1);
	return;
}

void V_DrawPic(int x, int y, pic_t* pic)
{
	int yy;
	byte* screen, * source, * dest;
	int planey, plane;
	int width, height;

	width = pic->width;
	height = pic->height;
	source = &pic->data;

	V_MarkUpdateBlock(x, y, width * 4 + x - 1, y + height - 1);

	planey = (x & 3) * 200;
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

		planey += 200;

		if (planey == 800) 
		{
			planey = 0;
			x++;
		}
	}
	return;
}

int V_DrawChar(int sx, int sy, int ch, font_t* font)
{
	byte pix;
	int x, y, width, plane;
	byte* source, * dest;
	int backup; //there's a dword-size hole between backup and font, but it seems unimportant. Used just for return. 

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

		if (plane == 48000)
		{
			dest++;
			plane = 0;
		}
		else 
		{
			plane += 16000;
		}
	}
	return width;
}

int V_DrawString(int sx, int sy, char* string, font_t* font)
{
	int tx;

	tx = sx;
	while (*string != '\0') 
	{
		tx += V_DrawChar(tx, sy, (int)(short)*string, font) + 1;
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
	V_DrawString(SCREENWIDTH / 2 - w / 2, y, string, font);
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
	//TODO: this is very broken in the original
}

void V_MarkUpdateBlock(int x1, int y1, int x2, int y2)
{
	int x, y;
	int xt1, yt1, xt2, yt2;
	byte* nextline;
	byte* mark;

	if ((((x1 < 0) || (x2 > 1279)) || (y1 < 0)) || (y2 > 199))
	{
		IO_Error("bad V_MarkUpdateBlock(%i,%i,%i,%i)", x1, y1, x2, y2);
	}

	xt1 = x1 / 16;
	yt1 = y1 / 20;
	xt2 = x2 / 16;
	yt2 = y2 / 20;
	mark = &update[xt1 + yt1 * 20];
	for (y = yt1; y <= yt2; y++)
	{
		nextline = mark + 20;
		for (x = xt1; x <= xt2; x++)
		{
			*mark++ = 2;
		}
		mark = nextline;
	}

	blockupdateneeded = 2;
	return;
}

void V_Startup(void)
{
	int x, y, i;
	unsigned int* usrc;

	for (i = 0; i < 800; i++)
	{
		ylookup[i] = screenbuffer + i * SCREENBWIDE;
	}
	for (i = 0; i < SCREENHEIGHT; i++)
	{
		planewidthlookup[i] = i * SCREENBWIDE;
	}
	for (i = 0; i < SCREENWIDTH; i++) 
	{
		collumnpointer[i] = &screenbuffer[0] + (i >> 2) + ((i & 3) * 16000);
	}

	usrc = ublocksource;
	for (y = 0; y < 10; y++)
	{
		for (x = 0; x < 20; x++)
		{
			*usrc++ = y * SCREENBWIDE * 20 + x * 4; //I think this is correct
		}
	}
	memset(update, 0, 200);
	playscreenupdateneeded = 0;
	blockupdateneeded = 0;
	for (i = 0; i < 320; i++)
	{
		planelookup[i] = (i & 3) * 16000;
	}
}
