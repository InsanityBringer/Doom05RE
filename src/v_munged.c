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
#include "v_local.h"

uint8_t screenbuffer[64000*4];
uint8_t* ylookup[800];
int planelookup[320];
int planewidthlookup[200];

uint8_t* collumnpointer[320];
uint32_t ublocksource[200];

uint8_t update[200];

int playscreenupdateneeded;
int blockupdateneeded;

int winxl, winxh;
int winyl, winyh;
int cursorx, cursory;

void V_MarkUpdateBlock(int x1, int y1, int x2, int y2)
{
	int iVar1;
	uint8_t* local_2c;
	int iVar2;

	int newx, newy;

	if ((((x1 < 0) || (x2 > 0x4ff)) || (y1 < 0)) || (y2 > 199))
	{
		IO_Error("bad V_MarkUpdateBlock(%i,%i,%i,%i)", x1, y1, x2, y2);
	}
	newx = x1 / 16;
	newy = y1 / 0x14;
	iVar1 = x2 / 16;
	local_2c = &update[newx + newy * 0x14];
	while (iVar2 = newx, newy <= y2 / 0x14)
	{
		while (iVar2 <= iVar1) 
		{
			*local_2c = 2;
			local_2c = local_2c + 1;
			iVar2 = iVar2 + 1;
		}
		local_2c = local_2c + (0x13 - (iVar1 - newx));
		newy++;
	}
	blockupdateneeded = 2;
	return;
}

void V_FadeOut(int start, int end, int red, int green, int blue, int steps)
{
	int local_20;
	int local_1c;
	int local_14;
	int iVar1;
	uint8_t pal[768];
	uint8_t newpal[768];

	local_20 = end;
	local_1c = start;
	IO_GetPalette(&pal[0]);
	iVar1 = 0;
	while (local_14 = local_1c, iVar1 <= steps) 
	{
		while (local_14 <= local_20) 
		{
			newpal[local_14 * 3 + 0] = (((red - pal[local_14 * 3 + 0]) * iVar1) / steps) + pal[local_14 * 3 + 0];
			newpal[local_14 * 3 + 1] = (((green - pal[local_14 * 3 + 1]) * iVar1) / steps) + pal[local_14 * 3 + 1];
			newpal[local_14 * 3 + 2] = (((blue - pal[local_14 * 3 + 2]) * iVar1) / steps) + pal[local_14 * 3 + 2];
			local_14++;
		}
		IO_SetPalette(&newpal[0]);
		IO_DoEvents(); //[ISB]
		iVar1++;
	}
	return;
}

void V_FadeIn(int start, int end, int steps, uint8_t* palette)
{
	int local_28;
	int local_20;
	int local_1c;
	int local_18;
	int local_14;

	uint8_t pal[768];
	uint8_t newpal[768];

	local_28 = end;
	local_20 = start;
	local_14 = steps;
	IO_GetPalette(&pal[0]);
	local_20 = local_20 * 3;
	local_28 = local_28 * 3;
	local_18 = 0;
	while (local_1c = local_20, local_18 < local_14)
	{
		while (local_1c <= local_28) 
		{
			newpal[local_1c] = pal[local_1c] + (((palette[local_1c] - pal[local_1c]) * local_18) / local_14);
			local_1c++;
		}
		IO_SetPalette(&newpal[0]);
		IO_DoEvents(); //[ISB]
		local_18++;
	}
	IO_SetPalette(&palette[0]);
	return;
}

void V_Bar(int xl, int yl, int width, int height, int color)
{
	int local_24;
	int local_28;
	uint8_t* local_2c;
	int local_3c;
	int local_38;
	int local_14;
	int iVar1;

	width = xl + width;
	local_24 = yl + height;
	iVar1 = 0;
	local_3c = xl;
	local_38 = yl;
	while (iVar1 < 4)
	{
		local_28 = (3 - iVar1) + local_3c >> 2;
		local_2c = ylookup[iVar1 * 200 + local_38] + local_28;
		local_28 = ((3 - iVar1) + width >> 2) - local_28;
		local_14 = local_38;
		while (local_14 < local_24) 
		{
			memset(local_2c, color, local_28);
			local_2c += 0x50;
			local_14++;
		}
		iVar1++;
	}
	V_MarkUpdateBlock(local_3c, local_38, width + -1, local_24 + -1);
	return;
}

void V_DrawPic(int x, int y, pic_t* pic)
{
	int local_2c;
	int local_30;
	int iVar1;
	uint8_t* local_18;
	int local_3c;
	int iVar2;
	int local_38;
	int local_28;
	int local_24;
	uint8_t* local_20;
	int iVar3;

	local_2c = pic->width;
	local_30 = pic->height;
	local_20 = &pic->data;
	local_3c = x;
	local_38 = y;
	V_MarkUpdateBlock(x, y, local_2c * 4 + x + -1, y + local_30 + -1);
	iVar1 = (local_3c & 3) * 200;
	iVar2 = (int)local_3c >> 2;
	local_24 = 0;
	local_28 = iVar1;
	while (local_24 < 4) 
	{
		local_18 = ylookup[local_28 + local_38] + iVar2;
		iVar3 = 0;
		iVar1 = local_24;
		while (iVar3 < (int)local_30) 
		{
			memcpy(local_18, local_20, local_2c);
			local_18 = local_18 + 0x50;
			local_20 = (uint8_t*)((int)local_20 + local_2c);
			iVar3 = iVar3 + 1;
		}
		local_28 = local_28 + 200;
		if (local_28 == 800) 
		{
			local_28 = 0;
			iVar2 = iVar2 + 1;
		}
		local_24 = iVar1 + 1;
	}
	return;
}

int V_DrawChar(int x, int y, int ch, font_t* font)
{
	uint8_t bVar1;
	short sVar2;
	uint8_t* local_24;
	int local_28;
	uint8_t* pix;
	int local_1c;
	int local_14;
	int iVar3;

	pix = (uint8_t*)((uintptr_t)&font->height + (uintptr_t)font->charofs[ch]);
	bVar1 = font->width[ch];
	local_24 = ylookup[y] + (x / 4);
	local_1c = planelookup[x];
	sVar2 = font->height;
	local_28 = bVar1;
	while (local_28 != 0)
	{
		local_14 = (int)font->height;
		while (local_14 != 0) 
		{
			if (*pix != 0)
			{
				local_24[local_1c] = *pix;
			}
			local_24 += 0x50;
			pix++;
			local_14--;
		}
		local_24 += (int)sVar2 * -0x50;
		if (local_1c == 48000)
		{
			local_24++;
			local_1c = 0;
			local_28--;
		}
		else 
		{
			local_1c += 16000;
			local_28--;
		}
	}
	return bVar1;
}

int V_DrawString(int sx, int sy, char* string, font_t* font)
{
	int iVar1;
	int local_24;
	int local_20;
	font_t* local_18;
	int x2;

	local_24 = sx;
	local_20 = sy;
	local_18 = font;
	x2 = sx;
	while (*string != '\0') 
	{
		iVar1 = V_DrawChar(x2, local_20, (int)(short)*string, local_18);
		x2 += iVar1 + 1;
		string++;
	}
	V_MarkUpdateBlock(local_24, local_20, x2, local_18->height + local_20 + -1);
	return x2;
}

int V_StringWidth(char* string, font_t* font)
{
	char* local_24;
	int iVar1;

	iVar1 = 0;
	local_24 = string;
	while (*local_24 != '\0') 
	{
		iVar1 = iVar1 + font->width[(short)*local_24] + 1;
		local_24++;
	}
	return iVar1;
}

void V_CenterString(int sy, char* string, font_t* font)
{
	int iVar1;
	int local_28;
	char* local_24;

	local_28 = sy;
	local_24 = string;
	iVar1 = V_StringWidth(string, font);
	V_DrawString(160 - iVar1 / 2, local_28, local_24, font);
}

void V_Window(int width, int height)
{
	int local_1c;
	int iVar1;

	winxl = 0xa0 - width / 2;
	winxh = winxl + width;
	winyl = 100 - height / 2;
	winyh = winyl + height;
	local_1c = width;
	iVar1 = height;
	V_Bar(winxl, winyl, width, height, 0xc6);
	V_Bar(winxl + 6, winyl + 6, local_1c + -12, iVar1 + -12, 3);
	winxl = winxl + 8;
	winxh = winxh + -8;
	winyl = winyl + 8;
	winyh = winyh + -8;
	cursory = winyl;
	cursorx = winxl;
	return;
}

void V_Printf(char* fmt, ...)
{
	//TODO: this is very broken in the original
}

void V_Startup(void)
{
	int iVar1;
	int iVar2;
	uint32_t* local_28;
	int iVar4;
	int uVar5;
	int i, j;

	for (i = 0; i < 800; i++)
	{
		iVar4 = i * 0x140 >> 0x1f;
		//ylookup[i] = &screenbuffer + ((int)((i * 0x140 + iVar4 * -4) - (uint)(iVar4 << 1 < 0)) >> 2);
		//[ISB] pointer arithemetic confuses me.
		ylookup[i] = (uint8_t*)((uintptr_t)&screenbuffer + (uintptr_t)(i * 80));
	}
	for (i = 0; i < 200; i++)
	{
		planewidthlookup[i] = (i * 0x50);
	}
	for (i = 0; i < 320; i++) 
	{
		collumnpointer[i] = &screenbuffer[0] + (i >> 2) + ((i & 3) * 16000);
	}
	local_28 = ublocksource;
	for (i = 0; i < 10; i++)
	{
		for (j = 0; j < 0x14; j++)
		{
			iVar1 = i * 0x140 >> 0x1f;
			iVar2 = j * 0x10 >> 0x1f;
			*local_28 = (((i * 0x140 + iVar1 * -4) - (uint32_t)(iVar1 << 1 < 0)) >> 2) * 0x14 + (((j * 0x10 + iVar2 * -4) - (uint32_t)(iVar2 << 1 < 0)) >> 2);
			local_28++;
		}
	}
	memset(update, 0, 200);
	playscreenupdateneeded = 0;
	blockupdateneeded = 0;
	for (i = 0; i < 320; i++)
	{
		planelookup[i] = (uint8_t*)((i & 3) * 16000);
	}
}
