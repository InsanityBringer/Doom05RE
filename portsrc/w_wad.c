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
// W_wad.c

#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

#include "doomdef.h"
#include "w_wad.h"

//===============
//   TYPES
//===============

typedef struct
{
	char		identification[4];		// should be IWAD
	int			numlumps;
	int			infotableofs;
} wadinfo_t;


typedef struct
{
	int			filepos;
	int			size;
	char		name[8];
} filelump_t;

//=============
// GLOBALS
//=============

lumpinfo_t* lumpinfo;		// location of each lump on disk
int			numlumps;

void ExtractFileBase(char* path, char* dest)
{
	char* src;
	int		length;

	src = path + strlen(path) - 1;

	//
	// back up until a \ or the start
	//
	while (src != path && *(src - 1) != '\\' && *(src - 1) != '/')
		src--;

	//
	// copy up to eight characters
	//
	memset(dest, 0, 8);
	length = 0;
	while (*src && *src != '.')
	{
		if (++length == 9)
			IO_Error("Filename base of %s >8 chars", path);
		*dest++ = toupper((int)*src++);
	}
}

/*
============================================================================

						LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_AddFile
=
= All files are optional, but at least one file must be found
= Files with a .wad extension are wadlink files with multiple lumps
= Other files are single lumps with the base filename for the lump name
=
====================
*/

void W_AddFile(char* filename)
{
	wadinfo_t* header;
	lumpinfo_t* lump_p;
	unsigned		i;
	int				handle, length;
	int				startlump;
	filelump_t* fileinfo, singleinfo;
	byte* data;

	//
	// open the file and add to directory
	//	
	if ((handle = open(filename, O_RDONLY | O_BINARY)) == -1)
		return;

	length = filelength(handle);
	data = (byte*)malloc(length);

	if (!data)
		IO_Error("W_InitMultipleFiles: couldn't malloc %i", length);

	if (read(handle, data, length) != length)
		IO_Error("W_InitMultipleFiles: couldn't read all of %s", filename);

	close(handle);

	startlump = numlumps;
	header = (wadinfo_t*)data;

	if (strcmpi(filename + strlen(filename) - 3, "wad"))
	{
		// single lump file
		fileinfo = &singleinfo;
		singleinfo.filepos = 0;
		singleinfo.size = length;
		ExtractFileBase(filename, singleinfo.name);
		numlumps++;
	}
	else
	{
		// WAD file
		if (strncmp(header->identification, "IWAD", 4))
			IO_Error("Wad file %s doesn't have IWAD id\n", filename);

		fileinfo = (filelump_t*)(data + header->infotableofs);
		numlumps += header->numlumps;
	}

	//
	// Fill in lumpinfo
	//
	lumpinfo = realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));
#ifdef ISB_LINT
	if (!lumpinfo)
		IO_Error("Couldn't realloc lumpinfo");
#endif
	lump_p = &lumpinfo[startlump];

	for (i = startlump; i < numlumps; i++, lump_p++, fileinfo++)
	{
		lump_p->position = (void*)(data + fileinfo->filepos);
		lump_p->size = fileinfo->size;
		strncpy(lump_p->name, fileinfo->name, 8);
	}
}

/*
====================
=
= W_InitMultipleFiles
=
= Pass a null terminated list of files to use.
=
= All files are optional, but at least one file must be found
=
= Files with a .wad extension are idlink files with multiple lumps
=
= Other files are single lumps with the base filename for the lump name
=
= Lump names can appear multiple times. The name searcher looks backwards,
= so a later file can override an earlier one.
=
====================
*/

void W_InitMultipleFiles(char** filenames)
{
	int		size;

	//
	// open all the files, load headers, and count lumps
	//
	numlumps = 0;
	lumpinfo = malloc(1);	// will be realloced as lumps are added

	for (; *filenames; filenames++)
		W_AddFile(*filenames);

	if (!numlumps)
		IO_Error("W_InitFiles: no files found");
}

/*
====================
=
= W_InitFile
=
= Just initialize from a single file
=
====================
*/

void W_InitFile(char* filename)
{
	char* names[2];

	names[0] = filename;
	names[1] = NULL;
	W_InitMultipleFiles(names);
}


/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/

int	W_CheckNumForName(char* name)
{
	char	name8[9];
	int		v1, v2;
	lumpinfo_t* lump_p;

	// make the name into two integers for easy compares

	strncpy(name8, name, 8);
	name8[8] = 0;			// in case the name was a fill 8 chars
	strupr(name8);			// case insensitive

	v1 = *(int*)name8;
	v2 = *(int*)&name8[4];


	// scan backwards so patch lump files take precedence

	lump_p = lumpinfo + numlumps;

	while (lump_p-- != lumpinfo)
		if (*(int*)lump_p->name == v1 && *(int*)&lump_p->name[4] == v2)
			return lump_p - lumpinfo;


	return -1;
}

/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int	W_GetNumForName(char* name)
{
	int	i;

	i = W_CheckNumForName(name);
	if (i != -1)
		return i;

	IO_Error("W_GetNumForName: %s not found!", name);
	return -1;
}


/*
====================
=
= W_GetLump
=
====================
*/

void* W_GetLump(int lump)
{
	if (lump >= numlumps)
		IO_Error("W_CacheLump: %i >= numlumps!", lump);

	return (void*)lumpinfo[lump].position;
}

/*
====================
=
= W_GetName
=
====================
*/

void* W_GetName(char* name)
{
	return W_GetLump(W_GetNumForName(name));
}
