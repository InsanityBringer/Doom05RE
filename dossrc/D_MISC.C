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

#include <io.h> //TODO: portability
#include <fcntl.h>
#include <ctype.h>
#include "doomdef.h"
#include "w_wad.h"
#include "r_local.h"
#include "m_menu.h"
#include "i_local.h"

unsigned char rndtable[256] =
{
	0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249
};
int rndindex;

void D_Alert(char* text)
{
	int w;

	w = V_StringWidth(text, hudfont);
	V_Window(w + 32, 40);
	V_CenterString(90, text, hudfont);
	IO_UpdateScreen();
	IO_Ack();
	IO_ClearKeys();
}

int D_CheckParm(char* check)
{
	int i;
	char* parm;

	for (i = 1; i < my_argc; i++)
	{
		parm = my_argv[i];

		//strange. Finds the first letter in the parameter string, this isn't in the final version at all.
		//Wolfenstein 3D's US_CheckParm has similar code. 
		while (!isalpha(*parm) && *parm++);

		if (!_stricmp(check, parm))
			return i;
	}
	return 0;
}

int D_WriteFile(char* name, byte* source, int length)
{
	int handle, count;

	handle = _open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 438);
	if (handle == -1)
		return 0;
	count = _write(handle, source, length);
	_close(handle);

	if (count < length)
		return 0;
		
	return 1;
}

int D_AckWait(int tics)
{
	int start;
	int c;

	start = IO_GetTime();
	do
	{
		if (IO_GetTime() - start >= tics)
			return 0;
		
		c = IO_CheckAck();
	} while (c == 0);
	return c;
}

int D_Synchronize()
{
	int i;

	IO_ClearKeys();
	memset(sd->playercmd, 0, sizeof(sd->playercmd));
	if (commpresent)
	{
		V_CenterString(90, "Syncronizing", hudfont);
		IO_UpdateScreen();
	}

	sd->playercmdframe[sd->consoleplayer] = -2;
	IO_SendFrame();

	do
	{
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (sd->playeringame[i] && sd->playercmdframe[i] != -2)
			{
				if (keydown[KEY_ESCAPE])
				{
					IO_Error("Communication syncronization aborted");
				}
				break;
			}
		}

		if (i == MAXPLAYERS)
		{
			sd->playercmdframe[sd->consoleplayer] = -1;
			processedframe = sd->playercmdframe[sd->consoleplayer];

			for (i = 0; i < MAXPLAYERS; i++)
			{
				sd->playercmdframe[i] = -1;
			}

			return i;
		}
	} while (true);
}

byte D_Rnd(void)
{
	rndindex = rndindex + 1 & 0xff;
	return rndtable[rndindex];
}

void D_FadeIn(byte* palette)
{
	V_FadeIn(0, 255, 16, palette);
}

void D_FadeOut()
{
	V_FadeOut(0, 255, 128, 128, 128, 32);
}

//Screenshot nonsense
typedef struct
{
	short w;
	short h;
	short x;
	short y;
	byte nPlanes;
	byte masking;
	byte compression;
	byte padding;
	short transparentColor;
	byte xAspect;
	byte yAspect;
	short pageWidth;
	short pageHeight;
} iffbmhd_t;

int ShortBigEndian(int l)
{
	return (l & 0xff) * 0x100 + (l >> 8 & 0xff);
}

int LongBigEndian(int l)
{
	return l * 0x1000000 + (l >> 8 & 0xff) * 0x10000 +
		(l >> 0x10 & 0xff) * 0x100 + (l >> 0x18);
}

void WriteLBMFile(char* filename, byte* data, int width, int height, byte* palette)
{
	//Adapted from the ROTT source, which seems to be derived directly from this routine.
	byte* lbm, * lbmptr;
	long* formlength, * bmhdlength, * cmaplength, * bodylength;
	long    length;
	iffbmhd_t  basebmhd;
	int     i;

	lbm = lbmptr = (byte*)malloc(SCREENWIDTH * SCREENHEIGHT + 1000);

	//
	// start FORM
	//
	*lbmptr++ = 'F';
	*lbmptr++ = 'O';
	*lbmptr++ = 'R';
	*lbmptr++ = 'M';

	formlength = (long*)lbmptr;
	lbmptr += 4;                      // leave space for length

	*lbmptr++ = 'P';
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = ' ';

	//
	// write BMHD
	//
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = 'H';
	*lbmptr++ = 'D';

	bmhdlength = (long*)lbmptr;
	lbmptr += 4;                      // leave space for length

	memset(&basebmhd, 0, sizeof(basebmhd));
	basebmhd.w = ShortBigEndian(width);
	basebmhd.h = ShortBigEndian(height);
	//[ISB] The swaps are supposed to be there, despite being byte-sized. This causes only 0s to be written, but apparently dpaint is fine with it. This bug was maintained in ROTT. 
	basebmhd.nPlanes = ShortBigEndian(8);
	basebmhd.xAspect = ShortBigEndian(5);
	basebmhd.yAspect = ShortBigEndian(6);
	basebmhd.pageWidth = ShortBigEndian(width);
	basebmhd.pageHeight = ShortBigEndian(height);

	memcpy(lbmptr, &basebmhd, sizeof(basebmhd));
	lbmptr += sizeof(basebmhd);

	length = lbmptr - (byte*)bmhdlength - 4;
	*bmhdlength = LongBigEndian(length);
	if (length & 1)
		*lbmptr++ = 0;          // pad chunk to even offset

	//
	// write CMAP
	//
	*lbmptr++ = 'C';
	*lbmptr++ = 'M';
	*lbmptr++ = 'A';
	*lbmptr++ = 'P';

	cmaplength = (long*)lbmptr;
	lbmptr += 4;                      // leave space for length

	memcpy(lbmptr, &palette[0], 768);
	lbmptr += 768;

	length = lbmptr - (byte*)cmaplength - 4;
	*cmaplength = LongBigEndian(length);
	if (length & 1)
		*lbmptr++ = 0;          // pad chunk to even offset

	//
	// write BODY
	//
	*lbmptr++ = 'B';
	*lbmptr++ = 'O';
	*lbmptr++ = 'D';
	*lbmptr++ = 'Y';

	bodylength = (long*)lbmptr;
	lbmptr += 4;                      // leave space for length

	memcpy(lbmptr, data, width * height);
	lbmptr += width * height;

	length = lbmptr - (byte*)bodylength - 4;
	*bodylength = LongBigEndian(length);
	if (length & 1)
		*lbmptr++ = 0;          // pad chunk to even offset

	//
	// done
	//
	length = lbmptr - (byte*)formlength - 4;
	*formlength = LongBigEndian(length);
	if (length & 1)
		*lbmptr++ = 0;          // pad chunk to even offset

	//
	// write output file
	//
	D_WriteFile(filename, lbm, lbmptr - lbm);
	free(lbm);
}

typedef struct
{
	char    manufacturer;
	char    version;
	char    encoding;
	char    bits_per_pixel;
	unsigned short  xmin, ymin, xmax, ymax;
	unsigned short  hres, vres;
	unsigned char   palette[48];
	char    reserved;
	char    color_planes;
	unsigned short  bytes_per_line;
	unsigned short  palette_type;
	char    filler[58];
	unsigned char   data;           // unbounded
} pcx_t;

void WritePCXFile(char* filename, byte* data, int width, int height, byte* palette)
{
	int     i, length;
	pcx_t* pcx;
	byte* pack;

	pcx = malloc(width * height * 2 + 1000);

	pcx->manufacturer = 0x0a;   // PCX id
	pcx->version = 5;           // 256 color
	pcx->encoding = 1;      // uncompressed
	pcx->bits_per_pixel = 8;        // 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = width - 1;
	pcx->ymax = height - 1;
	pcx->hres = width;
	pcx->vres = height;
	memset(pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;      // chunky image
	pcx->bytes_per_line = width;
	pcx->palette_type = 2;       // not a grey scale
	memset(pcx->filler, 0, sizeof(pcx->filler));

	//
	// pack the image
	//
	pack = &pcx->data;

	for (i = 0; i < width * height; i++)
		if ((*data & 0xc0) != 0xc0)
			*pack++ = *data++;
		else
		{
			*pack++ = 0xc1;
			*pack++ = *data++;
		}

	//
	// write the palette
	//
	*pack++ = 0x0c; // palette ID byte
	for (i = 0; i < 768; i++)
		*pack++ = *palette++;

	//
	// write output file
	//
	length = pack - (byte*)pcx;
	D_WriteFile(filename, (byte*)pcx, length);

	free(pcx);
}

void D_ScreenShot(int savepcx)
{
	int i, x, y;
	char lbmname[12];
	byte* linear, * source, * dest;
	byte palette[768];

	linear = (byte*)malloc(SCREENHEIGHT * SCREENWIDTH);
	source = screenbuffer;
	dest = linear;
	for (y = 0; y < SCREENHEIGHT; y++)
	{
		for (x = 0; x < SCREENBWIDE; x++)
		{
			*dest = *source;
			dest[1] = source[SCREENSIZE];
			dest[2] = source[SCREENSIZE*2];
			dest[3] = source[SCREENSIZE*3];
			dest += 4;
			source++;
		}
	}
	V_Window(140, 40);
	V_CenterString(90, "saving screen", hudfont);
	IO_UpdateScreen();

	strcpy(lbmname, "DOOM00.");

	if (savepcx)
		strcat(lbmname, "pcx");
	else
		strcat(lbmname, "lbm");

	for (i = 0; i < 100; i++)
	{
		lbmname[4] = (char)(i / 10) + '0';
		lbmname[5] = (char)(i % 10) + '0';
		if (_access(lbmname, 0) == -1) break;
	}

	if (i == 100)
		IO_Error("IO_ScreenShot: Couldn't create an LBM");

	IO_GetPalette(palette);
	if (savepcx)
		WritePCXFile(lbmname, linear, SCREENWIDTH, SCREENHEIGHT, palette);
	else
		WriteLBMFile(lbmname, linear, SCREENWIDTH, SCREENHEIGHT, palette);

	free(linear);
	V_Window(140, 40);
	V_CenterString(90, "Screen saved", hudfont);
	IO_UpdateScreen();
	IO_Ack();
}
