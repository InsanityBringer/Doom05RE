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

#include <string.h>
//TODO: portability
#include <io.h>
#include <fcntl.h>

int numlumps = 0;
lumpinfo_t* lumpinfo;


void ExtractFileBase(char* path, char* dest)
{
	char* pcVar1;
	int local_1c;
	int iVar2;
	char* local_24;
	char* local_20;
	char* pcVar3;
	char* pcVar4;

	local_24 = path;
	local_20 = dest;
	local_1c = strlen(path);
	pcVar4 = local_24 + local_1c;
	do 
	{
		pcVar3 = pcVar4 + -1;
		if ((pcVar3 == local_24) || (pcVar4[-2] == '\\')) break;
		pcVar1 = pcVar4 + -2;
		pcVar4 = pcVar3;
	} while (*pcVar1 != '/');
	memset(local_20, 0, 8);
	local_1c = 0;
	while ((*pcVar3 != '\0' && (*pcVar3 != '.'))) 
	{
		local_1c = local_1c + 1;
		if (local_1c == 9)
		{
			IO_Error("Filename base of %s >8 chars\n", local_24);
		}
		pcVar4 = pcVar3 + 1;
		iVar2 = toupper((int)(short)* pcVar3);
		*local_20 = (char)iVar2;
		local_20 = local_20 + 1;
		pcVar3 = pcVar4;
	}
	return;
}

void W_AddFile(char* filename)
{
	wadinfo_t* pwVar1;
	int uVar2;
	unsigned int size;
	int iVar3;
	char* extraout_EDX;
	int local_4c;
	unsigned int local_48;
	char acStack68[8];
	char* local_3c;
	lumpinfo_t* local_38;
	unsigned int local_34;
	wadinfo_t* local_30;
	unsigned int local_24;
	lumpinfo_t* local_20;
	filelump_t singleinfo;

	local_3c = filename;
	//uVar2 = watcom_open(extraout_EAX, 0x200, local_4c);
	uVar2 = _open(filename, O_BINARY | O_RDONLY);
	if (uVar2 != -1) 
	{
		size = _filelength(uVar2);
		local_30 = (wadinfo_t*)malloc(size);
		if (local_30 == NULL)
		{
			IO_Error("W_InitMultipleFiles: couldn't malloc %i", size);
			return;
		}
		//TODO:
		int length = _read(uVar2, (char*)local_30, size);
		if (length != size)
		{
			IO_Error("W_InitMultipleFiles: couldn't read all of %s", local_3c);
		}
		_close(uVar2);

		local_34 = numlumps;
		iVar3 = strlen(local_3c);
		iVar3 = _stricmp(local_3c + iVar3 + -3, "WAD");
		pwVar1 = local_30;
		if (iVar3 == 0)
		{
			iVar3 = strncmp((char*)local_30, "IWAD", 4);
			if (iVar3 != 0)
			{
				IO_Error("Wad file %s doesn't have IWAD id", local_3c);
			}
			local_38 = (int*)((int)&local_30->identifaction + pwVar1->infotableofs);
			numlumps = numlumps + pwVar1->numlumps;
		}
		else 
		{
			//local_38 = (int*)& stack0xffffffb4;
			//local_48 = size;
			singleinfo.size = size;
			singleinfo.filepos = 0;
			ExtractFileBase(local_3c, singleinfo.name);

			local_38 = &singleinfo;
			numlumps++;
		}
		lumpinfo_t* newlumpinfo;
		newlumpinfo = (lumpinfo_t*)realloc(lumpinfo, numlumps << 4);
		//[ISB]
		if (newlumpinfo == NULL)
		{
			IO_Error("W_InitMultipleFiles: couldn't realloc lumpinfo\n");
			return;
		}
		lumpinfo = newlumpinfo;
		local_20 = lumpinfo + local_34;
		local_24 = local_34;
		while (local_24 < numlumps) 
		{
			//TODO: I hate uintptr_t but
			local_20->position = (uintptr_t)&local_30->identifaction + (uintptr_t)local_38->position;
			local_20->size = local_38->size;
			strncpy((char*)& local_20->name, local_38->name, 8);
			local_24++;
			local_20++;
			local_38++;
		}
	}
	return;
}

void W_InitMultipleFiles(char** filenames)
{
	char** ppcVar1;

	numlumps = 0;
	ppcVar1 = filenames;
	lumpinfo = (lumpinfo_t*)malloc(sizeof(lumpinfo_t));
	while (*ppcVar1 != NULL) 
	{
		W_AddFile(*ppcVar1);
		ppcVar1++;
	}
	if (numlumps == 0) 
	{
		IO_Error("W_InitFiles: no files found\n");
	}
	return;
}

void W_InitFile(char* filename)
{
	char* filenames[2];

	filenames[0] = filename;
	filenames[1] = NULL;

	W_InitMultipleFiles(filenames);
	return;
}

int W_CheckNumForName(char* name)
{
	lumpinfo_t* lump_p;
	char buf[9];

	//int v1, v2;

	strncpy(buf, name, 8);
	buf[8] = '\0';
	_strupr(buf);

	//v1 = &buf[0];
	//v2 = &buf[4];

	lump_p = &lumpinfo[numlumps];

	while (lump_p-- != lumpinfo)
	{
		//this did the "compare two ints" technique of release doom but every attempt to implement this in msvc
		//just broke so have a strcmp. I blame strict aliasing
		if (!strncmp(lump_p->name, buf, 8))
		{
			return lump_p - lumpinfo;
		}
	} 

	return -1;
}

int W_GetNumForName(char* name)
{
	int local_20;

	local_20 = W_CheckNumForName(name);
	if (local_20 == -1) 
	{
		IO_Error("W_GetNumForName: %s not found!\n", name);
		return -1;
	}
	return local_20;
}

void* W_GetLump(int lump)
{
	if (lump >= numlumps) 
	{
		IO_Error("W_CacheLump: % i >= numlumps!\n", lump);
	}
	return (void*)lumpinfo[lump].position;
}

void* W_GetName(char* name)
{
	int iVar1;
	iVar1 = W_GetNumForName(name);
	return W_GetLump(iVar1);
}
