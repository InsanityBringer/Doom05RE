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
#include "r_ref.h"
#include "m_menu.h"

int my_argc;
char** my_argv;

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
	int iVar1;

	iVar1 = V_StringWidth(text, hudfont);
	V_Window(iVar1 + 32, 40);
	V_CenterString(0x5a, text, hudfont);
	IO_UpdateScreen();
	IO_Ack();
	IO_ClearKeys();
	return;
}

void D_LoadConfig()
{
	uint32_t* src;
	int local_EAX_63;
	int local_20;

	local_EAX_63 = W_CheckNumForName("CONFIG");
	if (local_EAX_63 == -1) 
	{
		config.hdetail = 1;
		config.viewsize = 8;
		local_20 = 0;
		while (local_20 < 6) 
		{
			strncpy(config.scores[local_20].name, "Id Software", 16);
			config.scores[local_20].skill = 4;
			config.scores[local_20].score = 10000;
			local_20++;
		}
	}
	else 
	{
		src = W_GetLump(local_EAX_63);
#if 0 //{ISB] "safe" copy here
		local_EAX_63 = (int)memcpy(&config, src, lumpinfo[local_EAX_63].size);
#endif
		config.hdetail = *src; src++;
		config.viewsize = *src; src++;

		for (local_20 = 0; local_20 < 6; local_20++)
		{
			memcpy(config.scores[local_20].name, (void*)src, 16); src += 4;
			config.scores[local_20].skill = *src; src++;
			config.scores[local_20].score = *src; src++;
		}
	}
}

int D_CheckParm(char* check)
{
	char cVar1;
	unsigned int uVar2;
	int iVar3;
	char* local_28;
	char* local_20;
	char* pcVar4;
	int iVar5;

	iVar5 = 1;
	local_28 = check;
	do 
	{
		if (my_argc <= iVar5) 
		{
			return 0;
		}
		local_20 = my_argv[iVar5];
		do 
		{
			uVar2 = isalpha((int)(short)*local_20);
			pcVar4 = local_20;
			if (uVar2 != 0) break;
			pcVar4 = local_20 + 1;
			cVar1 = *local_20;
			local_20 = pcVar4;
		} while (cVar1 != '\0');

		iVar3 = _stricmp(local_28, pcVar4);
		if (iVar3 == 0)
		{
			return iVar5;
		}
		iVar5++;
	} while (1);
}

int D_WriteFile(char* name, uint8_t* source, int length)
{
	int handle;
	int count;

	handle = _open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 438);
	if (handle == -1)
	{
		return 0;
	}
	else 
	{
		count = _write(handle, source, length);
		_close(handle);
		if (count < length) 
		{
			return 0;
		}
	}
	return 1;
}

int D_AckWait(int tics)
{
	int iVar1;
	int iVar2;
	int local_28;

	local_28 = tics;
	iVar1 = IO_GetTime();
	do 
	{
		iVar2 = IO_GetTime();
		if (local_28 <= iVar2 - iVar1) 
		{
			return 0;
		}
		iVar2 = IO_CheckAck();
	} while (iVar2 == 0);
	return iVar2;
}

void D_Synchronize()
{
}

uint8_t D_Rnd(void)
{
	rndindex = rndindex + 1 & 0xff;
	return rndtable[rndindex];
}

void D_FadeIn(uint8_t* palette)
{
	V_FadeIn(0, 0xff, 0x10, palette);
}

void D_FadeOut()
{
	V_FadeOut(0, 0xFF, 0x80, 0x80, 0x80, 0x20);
}

//Screenshot nonsense
typedef struct
{
	short w;
	short h;
	short x;
	short y;
	int8_t nplanes;
	int8_t masking;
	int8_t compression;
	int8_t padding;
	short transparentColor;
	int8_t xAspect;
	int8_t yAspect;
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

void WriteLBMFile(char* filename, uint8_t* data, int width, int height, uint8_t* palette)
{
	uint8_t* pbVar1;
	int* plVar2;
	uint8_t* source;
	int iVar3;
	int lVar4;
	iffbmhd_t local_4c;
	uint8_t* local_14;

	int length;

	source = (uint8_t*)malloc(width * height + 1000);

	//FORM
	*source = 0x46;
	source[1] = 0x4f;
	source[2] = 0x52;
	source[3] = 0x4d;

	//PBM 
	source[8] = 0x50;
	source[9] = 0x42;
	source[10] = 0x4d;
	source[0xb] = 0x20;

	//BMHD
	source[0xc] = 0x42;
	source[0xd] = 0x4d;
	source[0xe] = 0x48;
	source[0xf] = 0x44;

	memset((char*)&local_4c, 0, sizeof(iffbmhd_t));
	local_4c.w = ShortBigEndian(width);
	local_4c.h = ShortBigEndian(height);
	//[ISB] The swaps are supposed to be there, despite being byte-sized. This causes only 0s to be written, but apparently dpaint is fine with it.
	local_4c.nplanes = ShortBigEndian(8);
	local_4c.xAspect = ShortBigEndian(5);
	local_4c.yAspect = ShortBigEndian(6);
	local_4c.pageWidth = ShortBigEndian(0x140);
	local_4c.pageHeight = ShortBigEndian(200);

	memcpy(source + 0x14, &local_4c, 0x14);
	pbVar1 = source + 0x28;

	//lVar4 = LongBigEndian((pbVar1 + (-4 - (int)(source + 0x10))));

	//write BMHD length
	length = (int)(pbVar1 - (source + 20));
	*(int*)(source + 0x10) = LongBigEndian(length);

	local_14 = pbVar1;

	if ((length & 1) != 0) 
	{
		local_14 = source + 0x29;
		*pbVar1 = 0;
	}

	//CMAP
	*local_14 = 0x43;
	local_14[1] = 0x4d;
	local_14[2] = 0x41;
	local_14[3] = 0x50;
	memcpy(local_14 + 8, palette, 0x300);


	pbVar1 = local_14 + 0x308;
	length = (int)(pbVar1 - (local_14 + 8));
	//lVar4 = LongBigEndian((long)(pbVar1 + (-4 - (int)(local_14 + 4))));
	*(int*)(local_14 + 4) = LongBigEndian(length);

	if ((length & 1) != 0) 
	{
		*pbVar1 = 0;
		pbVar1 = local_14 + 0x309;
	}

	local_14 = pbVar1;

	//BODY
	*local_14 = 0x42;
	local_14[1] = 0x4f;
	local_14[2] = 0x44;
	local_14[3] = 0x59;

	plVar2 = (int*)(local_14 + 4);

	memcpy(local_14 + 8, data, width * height);

	pbVar1 = local_14 + 8 + width * height;

	length = (int)(pbVar1 - (local_14 + 8));
	//iVar3 = LongBigEndian((int)(pbVar1 + (-4 - (int)piVar2)));
	*plVar2 = LongBigEndian(length);

	local_14 = pbVar1;
	if ((length & 1) != 0) 
	{
		local_14 = pbVar1 + 1;
		*pbVar1 = 0;
	}

	length = (int)(local_14 - (source + 8));
	//iVar3 = LongBigEndian((int)(local_14 + (-4 - (int)(source + 4))));
	*(int*)(source + 4) = LongBigEndian(length);

	if ((length & 1) != 0)
	{
		*local_14 = 0;
		local_14 = local_14 + 1;
	}
	D_WriteFile(filename, source, (int)(local_14 + -(int)source));
	free(source);
	return;
}

void WritePCXFile(char* filename, uint8_t* data, int width, int height, uint8_t* palette)
{
	//TODO: I'll clean this up and make it use the structure when I am not so tired.
	char* local_2c;
	uint8_t* local_28;
	int local_20;
	uint8_t* pbVar1;
	uint8_t* local_1c;
	uint8_t* local_18;
	int iVar2;

	local_2c = filename;
	local_28 = data;
	local_20 = height;
	local_18 = (uint8_t*)malloc(width * height * 2 + 1000);
	*local_18 = 10;
	local_18[1] = 5;
	local_18[2] = 1;
	local_18[3] = 8;
	*(short*)(local_18 + 4) = 0;
	*(short*)(local_18 + 6) = 0;
	*(short*)(local_18 + 8) = (short)width + -1;
	*(short*)(local_18 + 10) = (short)local_20 + -1;
	*(short*)(local_18 + 0xc) = (short)width;
	*(short*)(local_18 + 0xe) = (short)local_20;
	memset((char*)(local_18 + 0x10), 0, 0x30);
	local_18[0x41] = 1;
	*(short*)(local_18 + 0x42) = (short)width;
	*(short*)(local_18 + 0x44) = 2;
	memset((char*)(local_18 + 0x46), 0, 0x3a);
	iVar2 = 0;
	local_1c = local_18 + 0x80;
	while (width * local_20 - iVar2 != 0 && iVar2 <= width * local_20) 
	{
		if ((*local_28 & 0xc0) == 0xc0) 
		{
			*local_1c = 0xc1;
			pbVar1 = local_1c + 2;
			local_1c[1] = *local_28;
		}
		else 
		{
			pbVar1 = local_1c + 1;
			*local_1c = *local_28;
		}
		local_28 = local_28 + 1;
		iVar2 = iVar2 + 1;
		local_1c = pbVar1;
	}
	*local_1c = 0xc;
	iVar2 = 0;
	while (local_1c = local_1c + 1, iVar2 < 0x300) 
	{
		*local_1c = *palette;
		iVar2 = iVar2 + 1;
		palette = palette + 1;
	}
	D_WriteFile(local_2c, local_18, (int)(local_1c + -(int)local_18));
	free(local_18);
	return;
}

void D_ScreenShot(int savepcx)
{
	uint8_t* pbVar1;
	int iVar2;
	uint8_t bVar3;
	uint8_t local_344;
	uint8_t abStack832[768];
	char local_40[12];
	uint8_t* local_30;
	uint8_t* local_2c;
	uint8_t* local_28;
	int local_24;
	int local_20;
	int iStack28;

	bVar3 = 0;
	local_28 = (uint8_t*)malloc(64000);
	local_30 = screenbuffer;
	local_24 = 0;
	local_2c = local_28;
	while (local_24 < 200)
	{
		local_20 = 0;
		while (local_20 < 0x50) 
		{
			*local_2c = *local_30;
			local_2c[1] = local_30[16000];
			pbVar1 = local_2c + 3;
			local_2c[2] = local_30[32000];
			local_2c = local_2c + 4;
			*pbVar1 = local_30[48000];
			local_30 = local_30 + 1;
			local_20 = local_20 + 1;
		}
		local_24 = local_24 + 1;
	}
	V_Window(0x8c, 0x28);
	V_CenterString(0x5a, "saving screen", hudfont);
	IO_UpdateScreen();

	strncpy(local_40, "DOOM00.", 8);
	local_40[7] = 0;

	if (savepcx == 0) 
	{
		strcat(local_40, "lbm");
	}
	else 
	{
		strcat(local_40, "pcx");
	}
	iStack28 = 0;
	while (iStack28 < 100)
	{
		local_40[4] = (char)(iStack28 / 10) + '0';
		local_40[5] = (char)(iStack28 % 10) + '0';
		iVar2 = _access(local_40, 0);
		if (iVar2 == -1) break;

		iStack28 = iStack28 + 1;
	}
	if (iStack28 == 100) 
	{
		IO_Error("IO_ScreenShot: Couldn't create an LBM");
	}

	IO_GetPalette(abStack832);
	if (savepcx == 0) 
	{
		WriteLBMFile(local_40, local_28, 0x140, 200, abStack832);
	}
	else 
	{
		WritePCXFile(local_40, local_28, 0x140, 200, abStack832);
	}
	free(local_28);
	V_Window(0x8c, 0x28);
	V_CenterString(0x5a, "Screen saved", hudfont);
	IO_UpdateScreen();
	IO_Ack();
	return;
}


