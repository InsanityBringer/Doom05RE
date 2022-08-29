#pragma once

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
