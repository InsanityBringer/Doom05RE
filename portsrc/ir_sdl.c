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
#include "p_local.h"
#include "r_data.h"
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
		for (i = 0; i < 1600; i += 80)
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
}

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
	if (novideo)
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

void R_SetDetail(detail_t detail)
{
	R_SetViewSize(screenblocks, detail, 0);
}

void R_SizeUp(void)
{
	if (screenblocks < 11)
		R_SetViewSize(screenblocks + 1, currentdetail, 0);
}

void R_SizeDown(void)
{
	if (screenblocks > 1)
		R_SetViewSize(screenblocks - 1, currentdetail, 0);
}

void R_SetViewBorder(char* border)
{
	bordertile = W_GetName(border);
}

void R_BorderFill(int xl, int yl, int xh, int yh)
{
	byte* source, * dest;
	int x, y, p;

	xl /= 4; xh /= 4;

	for (p = 0; p < 4; p++)
	{
		for (y = yl; y <= yh; y++)
		{
			dest = screenbuffer + planewidthlookup[y] + p * 16000 + xl;
			source = bordertile + ((y & 63) << 6) + p;

			for (x = xl; x <= xh; x++)
			{
				*dest++ = source[((x & 15) << 2)];
			}
		}
	}
}

void R_DrawViewBorder()
{
	if (screenblocks < 10)
	{
		R_BorderFill(0, 0, SCREENWIDTH - 1, windowy - 5);
		R_BorderFill(0, windowy + naturalheight + 4, SCREENWIDTH - 1, SCREENHEIGHT - SBARHEIGHT - 1);
		R_BorderFill(0, windowy - 4, windowx - 5, windowy + naturalheight + 4);
		R_BorderFill(windowx + naturalwidth + 4, windowy - 4, SCREENWIDTH - 1, windowy + naturalheight + 4);
	}
}

void R_DrawViewEdge()
{
	int x, y, w, h;
	patch_t* p1, * p2;

	if (screenblocks < 10)
	{
		w = naturalwidth / 4;
		h = naturalheight / 4;

		V_DrawPic(windowx - 4, windowy - 4, (pic_t*)W_GetName("WCORNUL"));
		V_DrawPic(windowx - 4, windowy + naturalheight, (pic_t*)W_GetName("WCORNUR"));
		V_DrawPic(windowx + naturalwidth, windowy - 4, (pic_t*)W_GetName("WCORNLL"));
		V_DrawPic(windowx + naturalwidth, windowy + naturalheight, (pic_t*)W_GetName("WCORNLR"));

		p1 = (pic_t*)W_GetName("WEDGET");
		p2 = (pic_t*)W_GetName("WEDGEB");

		for (x = 0; x < w; x++)
		{
			V_DrawPic(x * 4 + windowx, windowy - 4, p1);
			V_DrawPic(x * 4 + windowx, windowy + naturalheight, p2);
		}

		p1 = (pic_t*)W_GetName("WEDGEL");
		p2 = (pic_t*)W_GetName("WEDGER");
		
		for (y = 0; y < h; y++)
		{
			V_DrawPic(windowx - 4, y * 4 + windowy, p1);
			V_DrawPic(windowx + naturalwidth, y * 4 + windowy, p2);
		}
	}
}

void IO_BlitPlayScreen(void)
{
	int y, p, len;
	byte* source, * dest;

	if (novideo)
		return;

	//outp(SC_INDEX, SC_MAPMASK);
	switch (currentdetail)
	{
	case dt_high:
	case dt_highcolor:
	case dt_hires:
		len = viewwidth / 4;
		if (currentdetail == dt_highcolor)
			len *= 2;
		
		for (p = 0; p < 4; p++)
		{
			//outp(SC_INDEX+1, 1 << p);
			IO_SetMapMask(1 << p);
			source = screenbuffer + p * 16000 + windoworg;
			dest = windoworg + buffercrtc;
			
			for (y = 0; y < viewheight; y++)
			{
				//memcpy(dest, source, length);
				for (int c = 0; c < len >> 2; c++)
					IO_WriteMungeDWord(((int*)source)[c], dest + (c << 2));

				source += SCREENBWIDE;
				dest += SCREENBWIDE;
			}
		}
		break;
	case dt_medium:
		len = viewwidth / 2;
		for (p = 0; p < 4; p += 2)
		{
			//outp(SC_INDEX+1, 3 << p);
			IO_SetMapMask(3 << p);
			source = screenbuffer + p * 16000 + windoworg;
			dest = windoworg + buffercrtc;
			
			for (y = 0; y < viewheight; y++)
			{
				//memcpy(dest, source, len);
				for (int c = 0; c < len >> 2; c++)
					IO_WriteMungeDWord(((int*)source)[c], dest + (c << 2));
				
				source += SCREENBWIDE;
				dest += SCREENBWIDE;
			}
		}
		break;
	case dt_low:
		len = viewwidth / 2;
		
		for (p = 0; p < 4; p += 2)
		{
			//outp(SC_INDEX+1, 3 << p);
			IO_SetMapMask(3 << p);
			source = screenbuffer + p * 16000 + windoworg;
			dest = windoworg + buffercrtc;
			
			for (y = 0; y < viewheight; y++)
			{
				//memcpy(dest, source, len);
				//memcpy(dest + SCREENBWIDE, source, len);

				for (int c = 0; c < len >> 2; c++)
					IO_WriteMungeDWord(((int*)source)[c], dest + (c << 2));

				for (int c = 0; c < len >> 2; c++)
					IO_WriteMungeDWord(((int*)source)[c], (dest + SCREENBWIDE) + (c << 2));
				

				source += SCREENBWIDE * 2;
				dest += SCREENBWIDE * 2;
			}
		}
		break;
	default:
		IO_Error("IO_BlitPlayScreen: unknown detail level");
	}
}

void IO_DrawTics(void)
{
	static int oldtics;
	int tics;
	uint32_t dest;
	int i;

	if (novideo)
	{
		return;
	}
	tics = ticcount - oldtics;
	oldtics = ticcount;
	if (tics > 20)
		tics = 20;
	
	if (tics < 0)
		IO_Error("IO_DrawTics: tics = %i", tics);
	
	//outp(SC_INDEX+1, 3);
	IO_SetMapMask(3);
	dest = buffercrtc + SCREENBWIDE * (SCREENHEIGHT - 1);
	
	for (i = 0; i < tics; i++)
	{
		//*dest++ = 0xff;
		IO_WriteMunge(0xff, dest++);
	}
	
	for (; i < 20; i++)
	{
		//*dest++ = 0;
		IO_WriteMunge(0, dest++);
	}
}

void R_FixDetailPlanes(void)
{
	byte* src;
	int len;
	int y, p;

	switch (currentdetail)
	{
	case dt_high:
	case dt_highcolor:
	case dt_hires:
		break;
	case 1:
		len = viewwidth / 2;
		
		for (p = 0; p < 3; p += 2)
		{
			src = &screenbuffer[windoworg + p * 16000];
			
			for (y = 0; y < viewheight; y++)
			{
				memcpy(src + 16000, src, len);
				src = src + SCREENBWIDE;
			}
		}
		break;
	case 2:
		len = viewwidth / 2;

		for (p = 0; p < 3; p += 2)
		{
			src = &screenbuffer[windoworg + p * 16000];

			for (y = 0; y < viewheight; y++)
			{
				memcpy(src + SCREENBWIDE, src, len);
				memcpy(src + 16000, src, len);
				memcpy(src + 16000 + SCREENBWIDE, src, len);
				src = src + SCREENBWIDE * 2;
			}
		}
		break;
	}
}

void IO_UpdateOnly(void)
{
	if (novideo)
	{
		return;
	}

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

void IO_UpdateScreen(void)
{
	if (novideo)
	{
		return;
	}

	if (updatedone == 0)
		IO_UpdateOnly();
	
	updatedone = 0;
	IO_DrawTics();
	//outp(CRTC_INDEX, CRTC_STARTHIGH);
	//outp(CRTC_INDEX+1, buffercrtc >> 8);
	IO_SetStartAddress(buffercrtc);
	buffercrtc ^= 0x8000;
}

void V_DrawPatch(int x, int y, patch_t* patch)
{
	int col;
	byte* collumn;
	int top, bottom;
	byte* source;
	int dest;

	y = y - patch->topoffset;
	x = x - patch->leftoffs;
	V_MarkUpdateBlock(x, y, x + patch->width - 1, y + patch->height - 1);

	col = 0;
	//outp(SC_INDEX, SC_MAPMASK);
	for (; col < patch->width; x++, col++)
	{
		collumn = (byte*)patch + patch->coloffsets[col];

		//outp(SC_INDEX + 1, 1 << (x & 3));
		IO_SetMapMask(1 << (x & 3));
		while (*collumn != 0xff)
		{
			top = y + collumn[0];
			bottom = top + collumn[1];
			source = collumn + 2;
			dest = planewidthlookup[top] + buffercrtc + (x / 4);

			while (top++ < bottom)
			{
				//*dest = *source++;
				IO_WriteMunge(*source++, dest);
				dest += SCREENBWIDE;
			}
			collumn += collumn[1] + 2;
		}
	}
}

void IO_NoPlayBlit(void)
{
	V_MarkUpdateBlock(windowx, windowy, windowx + naturalwidth - 1, windowy + naturalheight - 1);
	noplayblit = 2;
}

void R_StartInstanceDrawing(void)
{
	IO_UpdateOnly();
	IO_NoPlayBlit();
}

int R_ClearBuffer(void)
{
	int y, p, len;
	byte* dest;

	switch (currentdetail) 
	{
	case dt_high:
	case dt_highcolor:
	case dt_hires:
		len = viewwidth / 4;
		if (currentdetail == 3) 
			len *= 2;
		
		for (p = 0; p < 4; p++)
		{
			dest = screenbuffer + windoworg + p * 16000;

			for (y = 0; y < viewheight; y++)
			{
				memset((char*)dest, validcheck, len);
				dest += SCREENBWIDE;
			}
		}
		break;
	case dt_medium:
		len = viewwidth / 2;

		for (p = 0; p < 3; p += 2)
		{
			dest = screenbuffer + windoworg + p * 16000;
			
			for (y = 0; y < viewheight; y++)
			{
				memset(dest, validcheck, len);
				dest += SCREENBWIDE;
			}
		}
		break;
	case dt_low:
		len = viewwidth / 2;

		for (p = 0; p < 3; p += 2)
		{
			dest = screenbuffer + windoworg + p * 16000;

			for (y = 0; y < viewheight; y++)
			{
				memset(dest, validcheck, len);
				dest += SCREENBWIDE * 2;
			}
		}
		break;
	default:
		IO_Error("R_ClearBuffer: unknown detail level");
	}
	return;
}

void R_InitVideoDevice(void)
{
	int lump;
	int i;

	R_SetViewBorder("FLOOR5_1");
	screenblocks = 10;
	currentdetail = 0;
	buffercrtc = 0;

	lump = W_GetNumForName("COLORMAP");
	colormaps = (byte*)malloc(8448);
#ifdef ISB_LINT
	if (colormaps == NULL)
	{
		IO_Error("R_InitVideoDevice: cannot allocate colormaps");
		return;
	}
	if (lumpinfo[lump].size > 8448)
	{
		IO_Error("R_InitVideoDevice: bad colormap size");
		return;
	}
#endif
	memcpy(colormaps, (void*)lumpinfo[lump].position, lumpinfo[lump].size);

	lump = W_GetNumForName("COLORS15");
	wordcolormaps = (unsigned short*)malloc(16896);
#ifdef ISB_LINT
	if (wordcolormaps == NULL)
	{
		IO_Error("R_InitVideoDevice: cannot allocate wordcolormaps");
		return;
	}
	if (lumpinfo[lump].size > 16896)
	{
		IO_Error("R_InitVideoDevice: bad wordcolormap size");
		return;
	}
#endif
	memcpy(wordcolormaps, (void*)lumpinfo[lump].position, lumpinfo[lump].size);
	
	for (i = 0; i < SCREENWIDTH; i++)
		highcollumntable[i] = screenbuffer + (i >> 2) + (i & 3) * (SCREENBWIDE * SCREENHEIGHT);

	for (i = 0; i < SCREENWIDTH; i++)
		lowcollumntable[i] = screenbuffer + (i >> 1) + (i & 1) * (SCREENBWIDE * SCREENHEIGHT * 2);
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

