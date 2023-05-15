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
#include "i_local.h"
#include "p_play.h"
#include "r_local.h"
#include "sdl_gl.h"

int screenblocks;
detail_t currentdetail;

int windowx, windowy;
int windoworg;
int buffercrtc;

int novideo;

byte* colormaps;
unsigned short* wordcolormaps;

byte* lowcollumntable[320];
byte* highcollumntable[320];

byte* hiresbuffer;
int noplayblit;
int updatedone;

byte* bordertile;

void R_MapRowHighC(void)
{
	int x, spot, plane;
	byte* dest;
	byte* colormap;

	if ((((mr_x1 < 0) || (viewwidth + 1 < mr_x2)) || (mr_x2 <= mr_x1)) || (viewheight <= mr_y)) 
		IO_Error("MapRow: mr_x1 = %i  mr_x2 = %i mr_y = %i", mr_x1, mr_x2, mr_y);
	
	if (mr_picture == NULL) 
		IO_Error("MapRow: NULL picture");
	
	x = mr_x1;
	colormaps = &colormaps[mr_colormap * 256];
	dest = screenbuffer + (int)(planewidthlookup[mr_y + windowy] + (mr_x1 + windowx >> 2));
	plane = ((mr_x1 + windowx) & 3U) * 16000;
	do 
	{
		spot = ((mr_yfrac >> 0x14) & 0xfc0U) + ((mr_xfrac >> 0x1a) & 0x3fU);
		dest[plane] = dest[mr_picture[spot]];
		
		mr_xfrac += mr_xstep;
		mr_yfrac += mr_ystep;
		if (plane == 48000)
		{
			dest++;
			plane = 0;
		}
		else 
			plane += 16000;
		
		x++;
	} while (x != mr_x2);
}

void R_MapRowMedC(void)
{
	int x, spot, plane;
	byte* dest;
	byte* colormap;

	if ((((mr_x1 < 0) || (viewwidth + 1 < mr_x2)) || (mr_x2 <= mr_x1)) || (viewheight <= mr_y))
		IO_Error("MapRow: mr_x1 = %i  mr_x2 = %i mr_y = %i", mr_x1, mr_x2, mr_y);

	if (mr_picture == NULL)
		IO_Error("MapRow: NULL picture");
	
	x = mr_x1;
	colormap = &colormaps[mr_colormap * 256];
	dest = screenbuffer + (int)(planewidthlookup[mr_y + windowy] + (mr_x1 * 2 + windowx >> 2));
	plane = ((mr_x1 * 2 + windowx) & 3U) * 16000;
	do
	{
		spot = ((mr_yfrac >> 0x14) & 0xfc0U) + ((mr_xfrac >> 0x1a) & 0x3fU);
		dest[plane] = colormap[mr_picture[spot]];
		mr_xfrac += mr_xstep;
		mr_yfrac += mr_ystep;
		if (plane == 32000)
		{
			dest++;
			plane = 0;
		}
		else
			plane = 32000;
		
		x++;
	} while (x != mr_x2);
}

void R_MapRowLowC(void)
{
	int x, spot, plane;
	byte* dest;
	byte* colormap;

	if ((((mr_x1 < 0) || (viewwidth + 1 < mr_x2)) || (mr_x2 <= mr_x1)) || (viewheight <= mr_y))
		IO_Error("MapRow: mr_x1 = %i  mr_x2 = %i mr_y = %i", mr_x1, mr_x2, mr_y);

	if (mr_picture == NULL)
		IO_Error("MapRow: NULL picture");
	
	x = mr_x1;
	colormap = &colormaps[mr_colormap * 256];
	dest = screenbuffer + (int)(planewidthlookup[mr_y * 2 + windowy] + (mr_x1 * 2 + windowx >> 2));
	plane = ((mr_x1 * 2 + windowx) & 3U) * 16000;
	do
	{
		spot = ((mr_yfrac >> 0x14) & 0xfc0U) + ((mr_xfrac >> 0x1a) & 0x3fU);
		dest[plane] = colormap[mr_picture[spot]];
		mr_xfrac += mr_xstep;
		mr_yfrac += mr_ystep;
		if (plane == 32000)
		{
			dest++;
			plane = 0;
		}
		else
			plane = 32000;

		x++;
	} while (x != mr_x2);
}

void R_RawScaleHighC(void)
{
	int count;
	byte* dest;
	byte* colormap;

	if ((((sp_y1 < 0) || (viewheight <= sp_y2)) || (sp_y2 < sp_y1)) || (viewwidth <= sp_x))
		IO_Error("R_ScalePost: sp_y1: %i  sp_y2: %i  sp_x: %i", sp_y1, sp_y2, sp_x);

	if (sp_source == NULL)
		IO_Error("R_ScalePost: NULL pointer\n");

	colormap = &colormaps[sp_colormap * 256];
	dest = planewidthlookup[sp_y1 + windowy] + highcollumntable[sp_x + windowx];
	count = (sp_y2 - sp_y1) + 1;
	while (count-- != 0)
	{
		*dest = colormap[sp_source[sp_frac >> FRACBITS]];
		dest += SCREENBWIDE;
		sp_frac += sp_fracstep;
#ifdef HIRES_FIXES
		//[ISB] wrap code
		//todo: optimize
		sp_frac %= (128 << FRACBITS);
#endif
	}
}

void R_RawScaleMedC(void)
{
	int count;
	byte* dest;
	byte* colormap;

	if ((((sp_y1 < 0) || (viewheight <= sp_y2)) || (sp_y2 < sp_y1)) || (viewwidth <= sp_x))
		IO_Error("R_ScalePost: sp_y1: %i  sp_y2: %i  sp_x: %i", sp_y1, sp_y2, sp_x);

	if (sp_source == NULL)
		IO_Error("R_ScalePost: NULL pointer");

	colormap = &colormaps[sp_colormap * 256];
	dest = planewidthlookup[sp_y1 + windowy] + highcollumntable[sp_x * 2 + windowx];
	count = (sp_y2 - sp_y1) + 1;
	while (count-- != 0)
	{
		*dest = colormap[sp_source[sp_frac >> FRACBITS]];
		dest += SCREENBWIDE;
		sp_frac += sp_fracstep;
	}
}

void R_RawScaleLowC(void)
{
	int count;
	byte* dest;
	byte* colormap;

	if ((((sp_y1 < 0) || (viewheight <= sp_y2)) || (sp_y2 < sp_y1)) || (viewwidth <= sp_x))
		IO_Error("R_ScalePost: sp_y1: %i  sp_y2: %i  sp_x: %i", sp_y1, sp_y2, sp_x);

	if (sp_source == NULL)
		IO_Error("R_ScalePost: NULL pointer\n");

	colormap = &colormaps[sp_colormap * 256];
	dest = planewidthlookup[sp_y1 * 2 + windowy] + highcollumntable[sp_x * 2 + windowx];
	count = (sp_y2 - sp_y1) + 1;
	while (count-- != 0)
	{
		*dest = colormap[sp_source[sp_frac >> FRACBITS]];
		dest += SCREENBWIDE * 2;
		sp_frac += sp_fracstep;
	}
}

void IO_WaitVBL(int vbls)
{
	//SDL_Delay((1000 / 70) * vbls);
}

void IO_SetPalette(uint8_t* pal)
{
	if (novideo)
		return;

	IO_WaitVBL(1);
	IO_GL_SetPalette(pal);
}

void IO_GetPalette(uint8_t* pal)
{
	if (novideo)
		return;

	IO_GL_GetPalette(pal);
}

void IO_SetHighColor(void)
{
	if (novideo == 0)
		return;
	
	SDL_GL_SetHighColor(SCREENWIDTH, SCREENHEIGHT);
}

void IO_ClearHighColor(void)
{
	if (novideo)
		return;
	
	IO_GL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, NULL);
}

void IO_SetHighRes(void)
{
	if (hiresbuffer == NULL)
	{
		hiresbuffer = (uint8_t*)malloc(0x1f400);
		if (hiresbuffer == NULL)
		{
			IO_Error("IO_SetHighRes: Couldn't malloc buffer\n");
		}
	}
	
	if (novideo)
		return;
}

void IO_ClearHighRes(void)
{
	if (novideo)
		return;
}

void R_SetViewSize(int blocks, detail_t detail, int redrawall)
{
	int yscale;
	detail_t olddetail;
	int oldblocks;
	int width, height;

	olddetail = currentdetail;
	oldblocks = screenblocks;
	currentdetail = detail;
	screenblocks = blocks;
	config.hdetail = detail;
	config.viewsize = blocks;

	if (blocks == 11)
	{
		naturalwidth = 320;
		naturalheight = 200;
		windowx = 0;
		windowy = 0;
		windoworg = 0;
	}
	else
	{
		naturalwidth = blocks * 32;
		naturalheight = (blocks * (SCREENHEIGHT - SBARHEIGHT) * 32) / SCREENWIDTH & 0xfffffffc;
		windowx = (blocks * -32 + SCREENWIDTH) / 2;
		windowy = ((SCREENHEIGHT - SBARHEIGHT) - naturalheight) / 2;
		windoworg = (windowy * SCREENWIDTH) / 4 + windowx / 4;
	}
	width = naturalwidth;
	height = naturalheight;

	switch (detail)
	{
	case dt_high:
		yscale = FRACUNIT;
		R_MapRow = &R_MapRowHigh;
		R_RawScale = &R_RawScaleHigh;
		break;
	case dt_medium:
		width /= 2;
		yscale = FRACUNIT * 2;
		R_MapRow = &R_MapRowMed;
		R_RawScale = &R_RawScaleMed;
		break;
	case dt_low:
		width /= 2;
		height /= 2;
		yscale = FRACUNIT;
		R_MapRow = &R_MapRowLow;
		R_RawScale = &R_RawScaleLow;
		break;
	case dt_highcolor:
		width /= 2;
		yscale = FRACUNIT * 2;
		R_MapRow = &R_MapRowHighColor;
		R_RawScale = &R_RawScaleHighColor;
		break;
	case dt_hires:
		height *= 2;
		yscale = FRACUNIT * 2;
		R_MapRow = &R_MapRowHighC;
		R_RawScale = &R_RawScaleHighC;
		break;
#ifdef ISB_LINT
	default:
		IO_Error("R_SetViewSize: Bad detail level\n");
#endif
	}
	R_ChangeWindow(width, height, yscale);

	if (olddetail == dt_highcolor && currentdetail != dt_highcolor)
		IO_ClearHighColor();
	else if (olddetail == dt_hires && currentdetail != dt_hires)
		IO_ClearHighRes();

	if (redrawall)
	{
		P_DrawPlayScreen();
		R_DrawViewBorder();
		R_DrawViewEdge();
	}
	else
	{
		if (oldblocks == 11 && screenblocks != 11)
			P_DrawPlayScreen();

		if (screenblocks < oldblocks)
			R_DrawViewBorder();

		if (oldblocks != screenblocks && screenblocks < 10)
			R_DrawViewEdge();

		if (currentdetail == dt_highcolor && olddetail != dt_highcolor)
		{
			IO_SetHighColor();
			P_DrawPlayScreen();
		}
		else if (currentdetail == dt_hires && olddetail != dt_hires)
		{
			IO_SetHighRes();
			P_DrawPlayScreen();
		}
	}

	V_MarkUpdateBlock(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - SBARHEIGHT - 1);
	IO_NoPlayBlit();
}

void IO_BlitBlocks(void)
{
	int iVar1;
	int iVar2;
	int puVar3;

	//out(0x3c4, 2);
	iVar2 = 199;
	do 
	{
		while (update[iVar2] == 0)
		{
			iVar2 = iVar2 - 1;
			if (iVar2 < 0) 
			{
				return;
			}
		}
		update[iVar2]--;
		iVar1 = ublocksource[iVar2];
		puVar3 = iVar1 + buffercrtc;
		//out(0x3c5, 1);
		int i;
		//TODO: unroll
		//TODO: investigate if transfer horizontally (despite the penalty to setting the mapmask) will make this faster.
		IO_SetMapMask(1);
		for (i = 0; i < 1600; i+=80)
		{
			IO_WriteMungeDWord(*(uint32_t*)(&screenbuffer[iVar1] + (i)), puVar3 + (i));
		}

		IO_SetMapMask(2);
		for (i = 0; i < 1600; i += 80)
		{
			IO_WriteMungeDWord(*(uint32_t*)(&screenbuffer[iVar1] + 16000 + (i)), puVar3 + (i));
		}
		
		IO_SetMapMask(4);
		for (i = 0; i < 1600; i += 80)
		{
			IO_WriteMungeDWord(*(uint32_t*)(&screenbuffer[iVar1] + 32000 + (i)), puVar3 + (i));
		}

		IO_SetMapMask(8);
		for (i = 0; i < 1600; i += 80)
		{
			IO_WriteMungeDWord(*(uint32_t*)(&screenbuffer[iVar1] + 48000 + (i)), puVar3 + (i));
		}
		iVar2 = iVar2 + -1;
	} while (-1 < iVar2);
	return;
}

void R_BorderFill(int xl, int yl, int xh, int yh)
{
	uint8_t* pbVar1;
	int local_20;
	int local_1c;
	int local_18;
	uint8_t* local_14;

	local_20 = 0;
	while (local_1c = yl, local_20 < 4) 
	{
		while (pbVar1 = bordertile, (int)local_1c <= yh)
		{
			local_18 = xl >> 2;
			local_14 = screenbuffer + (int)(planewidthlookup[local_1c] + (xl >> 2) + local_20 * 16000);
			while ((int)local_18 <= xh >> 2)
			{
				*local_14 = pbVar1[(local_18 & 0xf) * 4 + local_20 + (local_1c & 0x3f) * 0x40];
				local_18 = local_18 + 1;
				local_14 = local_14 + 1;
			}
			local_1c++;
		}
		local_20++;
	}
	return;
}

void R_DrawViewBorder()
{
	if (screenblocks < 10) 
	{
		R_BorderFill(0, 0, 0x13f, windowy + -5);
		R_BorderFill(0, windowy + naturalheight + 4, 0x13f, 0xa7);
		R_BorderFill(0, windowy + -4, windowx + -5, windowy + naturalheight + 4);
		R_BorderFill(windowx + naturalwidth + 4, windowy + -4, 0x13f, windowy + naturalheight + 4);
	}
}

void R_DrawViewEdge()
{
	int local_24;
	int local_28;
	pic_t* local_30;
	pic_t* local_2c;
	int local_20;

	if (screenblocks < 10)
	{
		local_24 = naturalwidth / 4;
		local_28 = naturalheight / 4;

		local_2c = (pic_t*)W_GetName("WCORNUL");
		V_DrawPic(windowx + -4, windowy + -4, local_2c);

		local_2c = (pic_t*)W_GetName("WCORNUR");
		V_DrawPic(windowx + -4, windowy + naturalheight, local_2c);

		local_2c = (pic_t*)W_GetName("WCORNLL");
		V_DrawPic(windowx + naturalwidth, windowy + -4, local_2c);
		local_2c = (pic_t*)W_GetName("WCORNLR");
		V_DrawPic(windowx + naturalwidth, windowy + naturalheight, local_2c);
		local_2c = (pic_t*)W_GetName("WEDGET");
		local_30 = (pic_t*)W_GetName("WEDGEB");

		local_20 = 0;
		while (local_20 < local_24) 
		{
			V_DrawPic(local_20 * 4 + windowx, windowy + -4, local_2c);
			V_DrawPic(local_20 * 4 + windowx, windowy + naturalheight, local_30);
			local_20 = local_20 + 1;
		}

		local_2c = (pic_t*)W_GetName("WEDGEL");
		local_30 = (pic_t*)W_GetName("WEDGER");
		local_20 = 0;
		while (local_20 < local_28)
		{
			V_DrawPic(windowx + -4, local_20 * 4 + windowy, local_2c);
			V_DrawPic(windowx + naturalwidth, local_20 * 4 + windowy, local_30);
			local_20 = local_20 + 1;
		}
	}
	return;
}

void R_SetDetail(detail_t detail)
{
	R_SetViewSize(screenblocks, detail, 0);
}

void R_SizeUp(void)
{
	if (screenblocks < 11) 
	{
		R_SetViewSize(screenblocks + 1, currentdetail, 0);
	}
	return;
}

void R_SizeDown(void)
{
	if (1 < screenblocks) 
	{
		R_SetViewSize(screenblocks + -1, currentdetail, 0);
	}
	return;
}

void R_SetViewBorder(char* border)
{
	bordertile = W_GetName(border);
}

void R_InitVideoDevice(void)
{
	int iVar1;
	void* pvVar2;
	int* puVar3;
	int puVar4;

	R_SetViewBorder("FLOOR5_1");
	screenblocks = 10;
	currentdetail = 0;
	buffercrtc = 0;
	iVar1 = W_GetNumForName("COLORMAP");
	colormaps = (uint8_t*)malloc(8448);
#ifdef ISB_LINT
	if (colormaps == NULL)
	{
		IO_Error("R_InitVideoDevice: cannot allocate colormaps");
		return;
	}
	if (lumpinfo[iVar1].size > 8448)
	{
		IO_Error("R_InitVideoDevice: bad colormap size");
		return;
	}
#endif
	memcpy(colormaps, (void*)lumpinfo[iVar1].position, lumpinfo[iVar1].size);
	iVar1 = W_GetNumForName("COLORS15");
	wordcolormaps = (uint16_t*)malloc(16896);
#ifdef ISB_LINT
	if (wordcolormaps == NULL)
	{
		IO_Error("R_InitVideoDevice: cannot allocate wordcolormaps");
		return;
	}
	if (lumpinfo[iVar1].size > 16896)
	{
		IO_Error("R_InitVideoDevice: bad wordcolormap size");
		return;
	}
#endif
	puVar3 = memcpy(wordcolormaps, (void*)lumpinfo[iVar1].position, lumpinfo[iVar1].size);
	puVar4 = 0;
	while ((int)puVar4 < 320)
	{
		highcollumntable[puVar4] = &screenbuffer[0] + ((int)puVar4 >> 2) + ((uint32_t)puVar4 & 3) * 16000;
		puVar3 = puVar4;
		puVar4++;
	}
	puVar4 = 0;
	while ((int)puVar4 < 160)
	{
		lowcollumntable[puVar4] = screenbuffer + ((int)puVar4 >> 1) + ((uint32_t)puVar4 & 1) * 32000;
		puVar3 = puVar4;
		puVar4++;
	}
	return;
}

void IO_BlitPlayScreen(void)
{
	size_t local_24;
	uint8_t* local_28;
	int local_2c;
	int local_20;
	int iVar1;

	int c;

	if (novideo == 0) 
	{
		//watcom_outp(0x3c4, 2);
		//IO_SetMapMask(2);
		if (currentdetail < 5)
		{
			local_24 = viewwidth >> 1;
			switch (currentdetail) 
			{
			default:
				local_24 = viewwidth >> 2;
				if (currentdetail == 3) 
				{
					local_24 = local_24 << 1;
				}
				local_20 = 0;
				while (local_20 < 4)
				{
					//watcom_outp(0x3c5, (byte)(1 << ((byte)start & 0x1f)));
					IO_SetMapMask(1 << (local_20 & 0x1f));
					local_28 = screenbuffer + windoworg + local_20 * 16000;
					local_2c = windoworg + buffercrtc;
					iVar1 = 0;
					while (iVar1 < viewheight) 
					{
						//memcpy(local_2c, local_28, local_24);
						for (c = 0; c < local_24 >> 2; c++)
						{
							IO_WriteMungeDWord(((int*)local_28)[c], local_2c + (c << 2));
						}
						local_2c = local_2c + 80;
						local_28 = local_28 + 80;
						iVar1++;
					}
					local_20++;
				}
				break;
			case 1:
				local_20 = 0;
				while (local_20 < 3)
				{
					//watcom_outp(0x3c5, (byte)(3 << ((byte)start & 0x1f)));
					IO_SetMapMask(3 << (local_20 & 0x1f));
					local_28 = screenbuffer + windoworg + local_20 * 16000;
					local_2c = windoworg + buffercrtc;
					iVar1 = 0;
					while (iVar1 < viewheight)
					{
						//watcom_memcpy(local_2c, local_28, local_24);
						for (c = 0; c < local_24 >> 2; c++)
						{
							IO_WriteMungeDWord(((int*)local_28)[c], local_2c + (c << 2));
						}
						local_2c = local_2c + 80;
						local_28 = local_28 + 80;
						iVar1++;
					}
					local_20 += 2;
				}
				break;
				
			case 2:
				local_20 = 0;
				while (local_20 < 3) 
				{
					//watcom_outp(0x3c5, (byte)(3 << ((byte)start & 0x1f)));
					IO_SetMapMask(3 << (local_20 & 0x1f));
					local_28 = screenbuffer + windoworg + local_20 * 16000;
					local_2c = windoworg + buffercrtc;
					iVar1 = 0;
					while (iVar1 < viewheight) 
					{
						//watcom_memcpy(local_2c, local_28, local_24);
						//watcom_memcpy((void*)((int)local_2c + 0x50), local_28, local_24);

						for (c = 0; c < local_24 >> 2; c++)
						{
							IO_WriteMungeDWord(((int*)local_28)[c], local_2c + (c << 2));
						}

						for (c = 0; c < local_24 >> 2; c++)
						{
							IO_WriteMungeDWord(((int*)local_28)[c], (local_2c + 80) + (c << 2));
						}

						local_2c = local_2c + 160;
						local_28 = local_28 + 160;
						iVar1++;
					}
					local_20 += 2;
				}
			}
		}
		else 
		{
			IO_Error("IO_BlitPlayScreen: unknown detail level\n");
		}
	}
	return;
}
void IO_DrawTics(void)
{
	int tics;
	uint32_t dest;
	int i;
	static int oldtics;

	if (novideo == 0) 
	{
		tics = ticcount - oldtics;
		oldtics = ticcount;
		if (20 < tics)
		{
			tics = 20;
		}
		if (tics < 0)
		{
			IO_Error("IO_DrawTics: tics = %i", tics);
		}
		IO_SetMapMask(3);
		i = 0;
		dest = buffercrtc + 15920;
		while (i < tics)
		{
			//*dest = 0xff;
			IO_WriteMunge(0xff, dest);
			i = i + 1;
			dest = dest + 1;
		}
		while (i < 20) 
		{
			//*dest = 0;
			IO_WriteMunge(0, dest);
			i = i + 1;
			dest = dest + 1;
		}
	}
	return;
}

void R_FixDetailPlanes(void)
{
	uint8_t* src;
	int local_28;
	int local_24;
	size_t local_20;

	if (currentdetail < 5) 
	{
		local_20 = viewwidth >> 1;
		switch (currentdetail)
		{
		default:
			break;
		case 1:
			local_28 = 0;
			while (local_28 < 3)
			{
				src = &screenbuffer[windoworg + local_28 * 16000];
				local_24 = 0;
				while (local_24 < viewheight) 
				{
					memcpy(src + 16000, src, local_20);
					src = src + 0x50;
					local_24 = local_24 + 1;
				}
				local_28 = local_28 + 2;
			}
			break;
		case 2:
			local_28 = 0;
			while (local_28 < 3)
			{
				src = &screenbuffer[windoworg + local_28 * 16000];
				local_24 = 0;
				while (local_24 < viewheight)
				{
					memcpy(src + 0x50, src, local_20);
					memcpy(src + 16000, src, local_20);
					memcpy(src + 0x3ed0, src, local_20);
					src = src + 0xa0;
					local_24 = local_24 + 1;
				}
				local_28 = local_28 + 2;
			}
		}
	}
	return;
}

void IO_UpdateOnly(void)
{
	if (novideo == 0) 
	{
		updatedone = 1;
		if (noplayblit == 2)
		{
			IO_WaitVBL(1);
		}
		if (noplayblit != 0) 
		{
			R_FixDetailPlanes();
		}
		if (blockupdateneeded != 0) 
		{
			IO_BlitBlocks();
		}
		if ((playscreenupdateneeded != 0) && (noplayblit == 0)) 
		{
			IO_BlitPlayScreen();
		}
		if (blockupdateneeded != 0) 
		{
			blockupdateneeded--;
		}
		if (playscreenupdateneeded != 0) 
		{
			playscreenupdateneeded--;
		}
		if (noplayblit != 0)
		{
			noplayblit--;
		}
	}
	return;
}

void IO_UpdateScreen(void)
{
	if (novideo == 0)
	{
		if (updatedone == 0)
		{
			IO_UpdateOnly();
		}
		updatedone = 0;
		IO_DrawTics();
		IO_SetStartAddress(buffercrtc);
		buffercrtc ^= 0x8000;
	}
	return;
}

void V_DrawPatch(int x, int y, patch_t* patch)
{
	uint8_t bVar1;
	int extraout_EAX = x;
	uint32_t local_24;
	uint8_t* local_18;
	int extraout_EDX = y;
	int iVar2;
	int local_34;
	int local_30;
	uint8_t* local_28;
	int local_1c;
	int iVar3;

	local_30 = extraout_EDX - (short)patch->topoffset;
	local_34 = extraout_EAX - (short)patch->leftoffs;
	V_MarkUpdateBlock(local_34, local_30, patch->width + local_34 - 1, patch->height + local_30 - 1);
	iVar3 = 0;
	//watcom_outp(0x3c4, 2);
	while (iVar3 < patch->width)
	{
		local_18 = &patch->width + *(short*)((int)patch->coloffsets + iVar3 * 2);
		//watcom_outp(0x3c5, (byte)(1 << ((byte)local_34 & 3)));
		IO_SetMapMask((uint8_t)(1 << (local_34 & 3)));
		while (*local_18 != 0xff) 
		{
			iVar2 = local_30 + *local_18;
			bVar1 = local_18[1];
			local_24 = planewidthlookup[iVar2] + buffercrtc + (local_34 >> 2);
			local_28 = local_18 + 2;
			local_1c = iVar2;
			while (local_1c < (iVar2 + bVar1)) 
			{
				//*dest = *local_28;
				IO_WriteMunge(*local_28, local_24);
				local_24 = local_24 + 0x50;
				local_28 = local_28 + 1;
				local_1c = local_1c + 1;
			}
			local_18 = local_18 + local_18[1] + 2;
		}
		local_34 = local_34 + 1;
		iVar3 = iVar3 + 1;
	}
	return;
}

void IO_NoPlayBlit(void)
{
	V_MarkUpdateBlock(windowx, windowy, windowx + naturalwidth - 1, windowy + naturalheight - 1);
	noplayblit = 2;
	return;
}

void R_StartInstanceDrawing(void)
{
	IO_UpdateOnly();
	IO_NoPlayBlit();
	return;
}

int R_ClearBuffer(void)
{
	int iVar1;
	uint8_t* local_28;
	int local_20;
	int count;
	int local_24;

	iVar1 = viewwidth >> 2;
	if (currentdetail < 5) 
	{
		local_24 = viewwidth >> 1;
		switch (currentdetail) 
		{
		default:
			local_24 = iVar1;
			if (currentdetail == 3) 
			{
				local_24 = iVar1 << 1;
			}
			local_20 = 0;
			while (local_20 < 4) 
			{
				local_28 = screenbuffer + windoworg + local_20 * 16000;
				count = 0;
				iVar1 = local_20;
				while (count < viewheight) 
				{
					memset((char*)local_28, validcheck, local_24);
					local_28 = local_28 + 0x50;
					count = count + 1;
				}
				local_20 = iVar1 + 1;
			}
			break;
		case 1:
			local_20 = 0;
			count = local_24;
			while (iVar1 = local_24, local_20 < 3) 
			{
				local_28 = screenbuffer + windoworg + local_20 * 16000;
				local_24 = 0;
				while (local_24 < viewheight) 
				{
					memset((char*)local_28, validcheck, count);
					local_28 = local_28 + 0x50;
					local_24 = local_24 + 1;
				}
				local_20 = local_20 + 2;
			}
			break;
		case 2:
			local_20 = 0;
			count = local_24;
			while (iVar1 = local_24, local_20 < 3)
			{
				local_28 = screenbuffer + windoworg + local_20 * 16000;
				local_24 = 0;
				while (local_24 < viewheight) 
				{
					memset((char*)local_28, validcheck, count);
					local_28 = local_28 + 0xa0;
					local_24 = local_24 + 1;
				}
				local_20 = local_20 + 2;
			}
		}
	}
	else 
	{
		IO_Error("R_ClearBuffer: unknown detail level");
	}
	return;
}

extern void P_DrawPlayerShapes(int pnum);

void TimeSpin(void)
{
	int start;
	int i;
	int tics;

	start = IO_GetTime();
	do 
	{
		IO_DoEvents(); //[ISB] I should use a timer thread....
		i = IO_GetTime();
	} while (i == start);
	start = start + 1;
	
	for (i = 0; i < 8192; i += 64)
	{
		R_RenderView(viewsector, viewx, viewy, viewz, i);
		P_DrawPlayerShapes(viewplayer);
		IO_UpdateScreen();
		IO_DoEvents();
	}
	i = IO_GetTime();
	tics = i - start;
	IO_Error("Time:%i  (%f ips)", tics, (double)(8960.0f / (float)tics));
	return;
}


extern void R_GenerateTexture(maptexture_t* texture);
extern uint8_t* videoMemory;
void R_TestScaleColumn()
{
	viewwidth = 320;
	viewheight = 200;

	static int texnum = 0;
	static int colormapnum = 0;

	R_ChangeWindow(320, 168, FRACUNIT);

	/*for (int i = 0; i < 32; i++)
	{
		sp_source = colormaps;
		sp_colormap = i;
		sp_frac = 0;
		sp_fracstep = (fixed_t)(1 * FRACUNIT);
		sp_x = i;
		sp_y1 = 0;
		sp_y2 = 167;
		R_RawScaleHigh();
	}*/

	/*int x, y;
	uint8_t color;
	uint8_t* dest;
	amapcolor[1] = 255;
	amapcolor[2] = 32;
	amapcolor[4] = 160;
	for (x = 0; x < 320; x++)
	{
		for (y = 0; y < 168; y++)
		{
			if (x >= mapwidth || y >= mapheight)
				color = amapcolor[0];
			else
				color = amapcolor[blockmap[y * mapwidth + x]];

			dest = planewidthlookup[y] + highcollumntable[x];
			*dest = color;
		}
	}*/

	/*if (keydown[SDL_SCANCODE_UP])
	{
		keydown[SDL_SCANCODE_UP] = 0;
		texnum++;
		if (texnum >= numtextures)
			texnum = numtextures - 1;
	}
	else if (keydown[SDL_SCANCODE_DOWN])
	{
		keydown[SDL_SCANCODE_DOWN] = 0;
		texnum--;
		if (texnum < 0)
			texnum = 0;
	}
	else if (keydown[SDL_SCANCODE_RIGHT])
	{
		keydown[SDL_SCANCODE_RIGHT] = 0;
		colormapnum++;
		if (colormapnum > 31)
			colormapnum = 31;
	}
	else if (keydown[SDL_SCANCODE_LEFT])
	{
		keydown[SDL_SCANCODE_LEFT] = 0;
		colormapnum--;
		if (colormapnum < 0)
			colormapnum = 0;
	}
	maptexture_t* texture = texturelookup[texnum];

	if (texture->collumndirectory == NULL)
		R_GenerateTexture(texture);

	for (int i = 0; i < 320; i++)
	{
		sp_source = (uint8_t*)R_CacheColumn(texture, i) + 2;
		sp_colormap = colormapnum;
		sp_frac = 0;
		sp_fracstep = (fixed_t)(1 * FRACUNIT);
		sp_x = i;
		sp_y1 = 0;
		sp_y2 = min(texture->height-1, 167);
		R_RawScaleHigh();
	}*/
	R_RawScale = &R_RawScaleHigh;
	R_MapRow = &R_MapRowHighC;

	//memset(screenbuffer, 0, 320 * 200);
	//sectors[0].lightlevel = 192;
	R_RenderView(playerobjs[0].r->sector, playerobjs[0].r->x, playerobjs[0].r->y, playerobjs[0].r->z + (41 << FRACBITS), playerobjs[0].r->angle);
	playerobjs[0].r->angle = (playerobjs[0].r->angle + 10) & 8191;
	
	V_MarkUpdateBlock(0, 0, 319, 167);
}

