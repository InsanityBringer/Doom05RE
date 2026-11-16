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
#ifndef __W_WAD_H__
#define __W_WAD_H__

//==============================================

typedef struct
{
	void* position;
	int size;
	char name[8];
} lumpinfo_t;

//==============================================

extern int numlumps;
extern lumpinfo_t* lumpinfo;

//==============================================

void W_AddFile(char* filename);
void W_InitMultipleFiles(char** filenames);
void W_InitFile(char* filename);
int W_CheckNumForName(char* name);
int W_GetNumForName(char* name);
void* W_GetLump(int lump);
void* W_GetName(char* name);

#endif
