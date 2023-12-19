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
#ifndef __DOOMDEF_H__
#define __DOOMDEF_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//uncomment to enable some checks added by salad, just usual "avoid lint yelling at you" things.
#ifndef __WATCOMC__
#define ISB_LINT
#endif

//rip c++ support
#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef unsigned char byte;
typedef enum { false, true } boolean;
#endif

typedef int fixed_t;

#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)

#define TICRATE 35

#define MAXPLAYERS 4

#define	KEY_RIGHTARROW		0x4D
#define	KEY_LEFTARROW		0x4B
#define	KEY_UPARROW			0x48
#define	KEY_DOWNARROW		0x50
#define	KEY_ESCAPE			0x01
#define	KEY_ENTER			0x1C
#define	KEY_F1				0x3b
#define	KEY_F2				0x3c
#define	KEY_F3				0x3d
#define	KEY_F4				0x3e
#define	KEY_F5				0x3f
#define	KEY_F6				0x40
#define	KEY_F7				0x41
#define	KEY_F8				0x42
#define	KEY_F9				0x43
#define	KEY_F10				0x44
#define	KEY_F11				0x57
#define	KEY_F12				0x58

#define	KEY_BACKSPACE		0x0E

#define KEY_EQUALS			0x0D
#define KEY_MINUS			0x0C

#define	KEY_RSHIFT			(0x80+0x36)
#define	KEY_RCTRL			(0x80+0x1d)
#define	KEY_RALT			(0x80+0x38)

#define	KEY_LALT			0x38
#define KEY_LCTRL			0x1d
#define KEY_LSHIFT			0x36

#define KEY_0           0x0B
#define KEY_1           0x02
#define KEY_2           0x03
#define KEY_3           0x04
#define KEY_4           0x05
#define KEY_5           0x06
#define KEY_6           0x07
#define KEY_7           0x08
#define KEY_8           0x09
#define KEY_9           0x0A

#define KEY_A           0x1E
#define KEY_B           0x30
#define KEY_C           0x2E
#define KEY_D           0x20
#define KEY_E           0x12
#define KEY_F           0x21
#define KEY_G           0x22
#define KEY_H           0x23
#define KEY_I           0x17
#define KEY_J           0x24
#define KEY_K           0x25
#define KEY_L           0x26
#define KEY_M           0x32
#define KEY_N           0x31
#define KEY_O           0x18
#define KEY_P           0x19
#define KEY_Q           0x10
#define KEY_R           0x13
#define KEY_S           0x1F
#define KEY_T           0x14
#define KEY_U           0x16
#define KEY_V           0x2F
#define KEY_W           0x11
#define KEY_X           0x2D
#define KEY_Y           0x15
#define KEY_Z           0x2C

#define KEY_LBRACKET    0x1A
#define KEY_RBRACKET    0x1B

#define KEY_DEBUG    0x29

//Angles are different from release Doom, only existing as "fine" angles
#define NUMANGLES 8192
#define ANGLEMASK (NUMANGLES-1)
#define ANG45 (NUMANGLES/4)

#ifdef __WATCOMC__
fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);

#pragma aux FixedMul =	\
	"imul ebx",			\
	"shrd eax,edx,16"	\
	parm	[eax] [ebx] \
	value	[eax]		\
	modify exact [eax edx]

#pragma aux FixedDiv =	\
	"cdq",				\
	"shld edx,eax,16",	\
	"sal eax,16",		\
	"idiv ebx"			\
	parm	[eax] [ebx] \
	value	[eax]		\
	modify exact [eax edx]
#else
inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
	return ((long long)a * b) >> FRACBITS;
}

inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	return ((long long)a << FRACBITS) / b;
}
#endif

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
void Z_CacheMalloc(memzone_t* zone, int size, void** user);
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
#ifndef _MSC_VER
extern volatile int keydown[];
#else
extern int keydown[];
#endif
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
void IO_GetPalette(byte* pal);
void IO_SetPalette(byte* pal);
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
int D_Synchronize();
void D_FadeIn(byte* palette);
void D_FadeOut();
int D_WriteFile(char* name, byte* source, int length);
void D_ScreenShot(int savepcx);

byte D_Rnd(void);

#endif
