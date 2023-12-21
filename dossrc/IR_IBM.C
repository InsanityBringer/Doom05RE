#include <stdarg.h>
#include <stdio.h>
#include <i86.h>
#include <conio.h>
#include <dos.h>
#include <graph.h> //no idea why

#include "doomdef.h"
#include "w_wad.h"
#include "i_local.h"
#include "p_play.h"
#include "r_local.h"

int screenblocks;
detail_t currentdetail;

int windowx, windowy;
int windoworg;

int novideo;

byte* colormaps;
unsigned short* wordcolormaps;

byte* lowcollumntable[320];
byte* highcollumntable[320];

byte* hiresbuffer;
int noplayblit;
int updatedone;

byte* bordertile;

/*
=============================================================================

							CONSTANTS

=============================================================================
*/

#define SC_INDEX                0x3C4
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_H_TOTAL    0
#define CRTC_H_DISPEND  1
#define CRTC_H_BLANK    2
#define CRTC_H_ENDBLANK 3
#define CRTC_H_RETRACE  4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL    6
#define CRTC_OVERFLOW   7
#define CRTC_ROWSCAN    8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND  11
#define CRTC_STARTHIGH  12
#define CRTC_STARTLOW   13
#define CRTC_CURSORHIGH 14
#define CRTC_CURSORLOW  15
#define CRTC_V_RETRACE  16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND  18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE  20
#define CRTC_V_BLANK    21
#define CRTC_V_ENDBLANK 22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE 24


#define GC_INDEX                0x3CE
#define GC_SETRESET             0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE 2
#define GC_DATAROTATE   3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN    17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN              19
#define ATR_COLORSELECT 20

#define STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA                0x3c9
#define PEL_MASK                0x3c6


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


void nullfunction()
{
}

//--------------------------------------------------------------------------
//
// IO_WaitVBL
//
//--------------------------------------------------------------------------

void IO_WaitVBL(int vbls)
{
	int i;
	int old, stat;

	if(novideo)
		return;
	
	//this entire function is fuckin weird, man
	while(vbls--)
	{
		outer:
		_disable();
		while(inp(STATUS_REGISTER_1) & 1) {}
		
		start:
		_enable();
		i--; //weird...
		_disable();
		for (i = 0; i < 10; i++)
		{
			stat = inp(STATUS_REGISTER_1);
			if (stat & 8)
				goto outer;
			if (!(stat&1))
				goto start;
		}
		_enable();
	}
}

//--------------------------------------------------------------------------
//
// IO_SetPalette
//
// Palette source must use 8 bit RGB elements.
//
//--------------------------------------------------------------------------

void IO_SetPalette(byte *pal)
{
	int i;

	if(novideo)
		return;
	
	IO_WaitVBL(1);
	outp(PEL_WRITE_ADR, 0);
	for(i = 0; i < 768; i++)
		outp(PEL_DATA, (*pal++)>>2);
}

//--------------------------------------------------------------------------
//
// IO_GetPalette
//
//--------------------------------------------------------------------------

void IO_GetPalette(byte *pal)
{
	int i;

	if(novideo)
		return;
	
	IO_WaitVBL(1);
	outp(PEL_READ_ADR, 0);
	for(i = 0; i < 768; i++)
		*pal++ = inp(PEL_DATA) << 2;
}

void IO_Color (int r, int g, int b)
{
	outp(PEL_WRITE_ADR,0);
	outp(PEL_DATA,r >> 2);
	outp(PEL_DATA,g >> 2);
	outp(PEL_DATA,b >> 2);
}

void IO_SetHighColor()
{
  	if(novideo)
		return;
	
	_disable();
	inp(0x3C6); //Enter control mode
	inp(0x3C6);
	inp(0x3C6);
	inp(0x3C6);
	outp(0x3C6,160);
	_enable();
}

void IO_ClearHighColor()
{
  	if(novideo)
		return;
	
	_disable();
	inp(0x3C6); //Enter control mode
	inp(0x3C6);
	inp(0x3C6);
	inp(0x3C6);
	outp(0x3C6,0);
	outp(0x3C6,255);
	_enable();
}

void IO_SetHighRes()
{
	if (!hiresbuffer)
	{
		hiresbuffer = (byte*)malloc(SCREENWIDTH * SCREENHEIGHT * 2);
		if (!hiresbuffer)
			IO_Error("IO_SetHighRes: Couldn't malloc buffer");
	}
	
	if(novideo)
		return;
	
	_disable();
	outp(CRTC_INDEX, CRTC_MAXSCANLINE);
	outp(CRTC_INDEX+1, inp(CRTC_INDEX+1) & 224);
	_enable();	
}

void IO_ClearHighRes()
{
	if(novideo)
		return;
	
	_disable();
	outp(CRTC_INDEX, CRTC_MAXSCANLINE);
	outp(CRTC_INDEX+1, inp(CRTC_INDEX+1) & 224); //BUG: This does the same thing as SetHighRes. It should restore the upper 3 bits to their original values
	_enable();	
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
	pic_t* p1, * p2;

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

	outp(SC_INDEX, SC_MAPMASK);
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
			outp(SC_INDEX+1, 1 << p);
			source = screenbuffer + p * 16000 + windoworg;
			dest = windoworg + buffercrtc;
			
			for (y = 0; y < viewheight; y++)
			{
				memcpy(dest, source, len);
				source += SCREENBWIDE;
				dest += SCREENBWIDE;
			}
		}
		break;
	case dt_medium:
		len = viewwidth / 2;
		for (p = 0; p < 4; p += 2)
		{
			outp(SC_INDEX+1, 3 << p);
			source = screenbuffer + p * 16000 + windoworg;
			dest = windoworg + buffercrtc;
			
			for (y = 0; y < viewheight; y++)
			{
				memcpy(dest, source, len);
				source += SCREENBWIDE;
				dest += SCREENBWIDE;
			}
		}
		break;
	case dt_low:
		len = viewwidth / 2;
		
		for (p = 0; p < 4; p += 2)
		{
			outp(SC_INDEX+1, 3 << p);
			source = screenbuffer + p * 16000 + windoworg;
			dest = windoworg + buffercrtc;
			
			for (y = 0; y < viewheight; y++)
			{
				memcpy(dest, source, len);
				memcpy(dest + SCREENBWIDE, source, len);
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
	byte* dest;
	int i;

	if (novideo)
		return;
	
	tics = ticcount - oldtics;
	oldtics = ticcount;
	if (tics > 20)
		tics = 20;
	
	if (tics < 0)
		IO_Error("IO_DrawTics: tics = %i", tics);
	
	outp(SC_INDEX+1, 3);
	dest = buffercrtc + SCREENBWIDE * (SCREENHEIGHT - 1);
	
	for (i = 0; i < tics; i++)
	{
		*dest++ = 0xff;
	}
	
	for (; i < 20; i++)
	{
		*dest++ = 0;
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
		return;

	updatedone = 1;
	if (noplayblit == 2)
		IO_WaitVBL(1);
	if (noplayblit)
		R_FixDetailPlanes();
	if (blockupdateneeded)
		IO_BlitBlocks();
	if ((layscreenupdateneeded && !noplayblit)
		IO_BlitPlayScreen();
	if (blockupdateneeded)
		blockupdateneeded--;
	if (playscreenupdateneeded)
		playscreenupdateneeded--;
	if (noplayblit)
		noplayblit--;
}

void IO_UpdateScreen(void)
{
	if (novideo)
		return;

	if (updatedone == 0)
		IO_UpdateOnly();
	
	updatedone = 0;
	IO_DrawTics();
	outp(CRTC_INDEX, CRTC_STARTHIGH);
	outp(CRTC_INDEX+1, (size_t)buffercrtc >> 8);
	buffercrtc = (byte*)((size_t)buffercrtc ^ 0x8000);
}

void V_DrawPatch(int x, int y, patch_t* patch)
{
	int col;
	byte* collumn;
	int top, bottom;
	byte* source, * dest;

	y = y - patch->topoffset;
	x = x - patch->leftoffs;
	V_MarkUpdateBlock(x, y, x + patch->width - 1, y + patch->height - 1);

	col = 0;
	outp(SC_INDEX, SC_MAPMASK);
	for (; col < patch->width; x++, col++)
	{
		collumn = (byte*)patch + patch->coloffsets[col];

		outp(SC_INDEX + 1, 1 << (x & 3));
		while (*collumn != 0xff)
		{
			top = y + collumn[0];
			bottom = top + collumn[1];
			source = collumn + 2;
			dest = planewidthlookup[top] + buffercrtc + (x / 4);

			while (top++ < bottom)
			{
				*dest = *source++;
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

void R_ClearBuffer(void)
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

			for (y = 0; y < viewheight; y ++)
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
	buffercrtc = (byte*)0xA8000;

	lump = W_GetNumForName("COLORMAP");
	colormaps = (byte*)((size_t)malloc(8448) + 255 & ~255);
	memcpy(colormaps, (void*)lumpinfo[lump].position, lumpinfo[lump].size);
	
	lump = W_GetNumForName("COLORS15");
	wordcolormaps = (unsigned short*)((size_t)malloc(16896) + 511 & ~511);
	memcpy(wordcolormaps, (void*)lumpinfo[lump].position, lumpinfo[lump].size);
	
	for (i = 0; i < SCREENWIDTH; i++)
		highcollumntable[i] = screenbuffer + (i >> 2) + (i & 3) * (SCREENBWIDE * SCREENHEIGHT);

	for (i = 0; i < SCREENWIDTH; i++)
		lowcollumntable[i] = screenbuffer + (i >> 1) + (i & 1) * (SCREENBWIDE * SCREENHEIGHT * 2);
}
