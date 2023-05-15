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
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//uncomment to enable some checks added by salad, just usual "avoid lint yelling at you" things.
#define ISB_LINT

//rip c++ support
//you know, maybe I should make this dbool with dtrue and dfalse before it floods the entire source
#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef unsigned char byte;
typedef enum { false, true } boolean;
#endif

typedef int fixed_t;

#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)

#define TICRATE 35

//Angles are different from release Doom, only existing as "fine" angles
#define NUMANGLES 8192
#define ANGLEMASK (NUMANGLES-1)

inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
	int64_t t = (int64_t)a * (int64_t)b;
	return (fixed_t)(t >> FRACBITS);
}

inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	if (b == 0) return 0xFFFFFFFF;
	int64_t t = ((int64_t)a << FRACBITS) / (int64_t)b;
	return (fixed_t)(t);
}

#include "sdl_key.h"

//[ISB] note: Need to look closer into this, but zone things always seem to come before the demo loop stuff.
//-----------
//MEMORY ZONE
//-----------

typedef struct memblock_s
{
	int size;
	void** owner;
	struct memblock_s* next;
	struct memblock_s* prev;
} memblock_t;

typedef struct
{
	int size;
	memblock_t blocklist;
	memblock_t* rover;
} memzone_t;

memzone_t* Z_AllocateZone(int size);
void Z_ClearZone(memzone_t* zone);

void Z_Free(void* ptr);
void* Z_Malloc(memzone_t* zone, int size);
void* Z_CacheMalloc(memzone_t* zone, int size, void** user);
void Z_CacheFree(void** user);

typedef enum
{
	da_filmwarp,
	da_story,
	da_credits,
	da_scores,
	da_demo1,
	da_demo2,
	da_demo3,
	da_demo4,
	da_startgame,
	da_gameloop
} demoaction_t;

typedef enum
{
	dt_high,
	dt_medium,
	dt_low,
	dt_highcolor,
	dt_hires
} detail_t;

typedef enum
{
	sk_baby,
	sk_easy,
	sk_normal,
	sk_hard,
	sk_deadly,
	sk_deadlybaby
} skill_t;

#define NUMHISCORES 6

typedef struct
{
	char name[16];
	int skill;
	int score;
} hscore_t;

typedef struct
{
	int hdetail;
	int viewsize;
	hscore_t scores[NUMHISCORES];
} config_t;

//global variables
extern int commpresent;
extern int novideo;

//keyboard access
extern int keydown[];
extern int lastscan;
extern int lastpress;
extern int ignorekeyboard;

extern detail_t currentdetail;
extern int screenblocks;

void IO_Startup();
void IO_NewFrame();

void IO_ClearKeys();
void IO_StartAck();
void IO_Ack();
int IO_GetTime();
int IO_CheckAck();
void IO_WaitVBL(int vbls);
void IO_GetPalette(uint8_t* pal);
void IO_SetPalette(uint8_t* pal);
void IO_BlitBlocks();
void IO_UpdateScreen();
void IO_DoEvents();
void IO_Quit();
#ifdef _MSC_VER
__declspec(noreturn) void IO_Error(char* fmt, ...);
#else
void IO_Error(char* fmt, ...);
#endif
void TimeSpin();

//demo loop
#define MAXPLAYERS 4

extern config_t config;
extern demoaction_t demoaction;

extern int my_argc;
extern char** my_argv;

void D_DemoLoop();

//misc functions:
void D_Alert(char* text);
void D_LoadConfig();
void D_SaveConfig();
int D_CheckParm(char* check);
int D_AckWait(int tics);
void D_Synchronize();
void D_FadeIn(byte* palette);
void D_FadeOut();
int D_WriteFile(char* name, byte* source, int length);
void D_ScreenShot(int savepcx);
