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

typedef int fixed_t;

#define FRACUNIT 0x10000
#define FRACBITS 16

inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
	int64_t t = (int64_t)a * (int64_t)b;
	return (fixed_t)(t >> 16);
}

inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	if (b == 0) return 0xFFFFFFFF;
	int64_t t = ((int64_t)a << FRACBITS) / (int64_t)b;
	return (fixed_t)(t);
}

#include "states.h"
#include "doomdata.h"
#include "sdl_key.h"

//z_*

typedef struct memblock_s
{ // Zone Allocator memory block
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

#define STUB(a) fprintf(stderr, a":STUB\n")

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

typedef enum
{
	wp_knife,
	wp_rifle,
	wp_shotgun,
	wp_auto,
	wp_missile,
	wp_chainsaw,
	wp_claw,
	wp_bfg,

	wp_nochange = 0xF
} weapontype_t;

#define MAXCLIP 199
#define MAXSHELL 99
#define MAXCELL 9
#define MAXSOUL 66
#define MAXMISL 9

typedef enum
{
	am_clip,
	am_shell,
	am_cell,
	am_soul,
	am_misl,
	NUMAMMO
} ammotype_t;

typedef enum
{
	dm_user,
	dm_recording,
	dm_playback
} demostate_t;

typedef enum
{
	it_bluecard,
	it_yellowcard,
	it_redcard
} item_t;

#define MAXBODY 15
#define MAXARMOR 15

typedef struct
{
	int viewz;
	struct thing_s* r;
	int momx;
	int momy;
	int momz;
	int bob;
	int lives;
	int score;
	int nextextra;
	int health;
	int armor;
	int itemtime;
	int items[3];
	weapontype_t readyweapon;
	weapontype_t pendingweapon;
	int weaponowned[8];
	int ammo[NUMAMMO];
	int firedown;
	int strafedown;
} player_t;


//i_*

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
void IO_Error(char* fmt, ...);
void TimeSpin();

void R_SetViewSize(int blocks, detail_t detail, int redrawall);
void R_SetDetail(detail_t detail);
void R_SizeUp();
void R_SizeDown();
void R_StartInstanceDrawing(); 
int R_ClearBuffer();

//video
typedef struct
{
	short height;
	uint8_t width[256];
	short charofs[256];
} font_t;


extern uint8_t screenbuffer[];

extern int playscreenupdateneeded;
extern int blockupdateneeded;

extern uint8_t* ylookup[];
extern int planewidthlookup[];
extern uint8_t* collumnpointer[];

void V_DrawPatch(int x, int y, patch_t* patch);
void V_FadeOut(int start, int end, int red, int green, int blue, int steps);
void V_FadeIn(int start, int end, int steps, uint8_t* palette);
int V_DrawChar(int x, int y, int ch, font_t* font);
void V_DrawPic(int x, int y, pic_t* pic);
void V_Bar(int xl, int yl, int width, int height, int color);
int V_DrawString(int sx, int sy, char* string, font_t* font);
int V_StringWidth(char* string, font_t* font);
void V_CenterString(int sy, char* string, font_t* font);
void V_Window(int width, int height);
void V_Printf(char* fmt, ...);
void V_MarkUpdateBlock(int x1, int y1, int x2, int y2);

void V_Startup();

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
void D_FadeIn(uint8_t* palette);
void D_FadeOut();
int D_WriteFile(char* name, uint8_t* source, int length);
void D_ScreenShot(int savepcx);

//game loop

typedef enum
{
	ga_runmap,
	ga_controlpanel,
	ga_completed,
	ga_died,
	ga_playing,
	ga_victory
} gameaction_t;

typedef enum
{
	gs_controlmap,
	gs_newgame,
	gs_demo,
	gs_netgame,
	gs_loadgame,
	gs_warp
} gamestart_t;

extern gamestart_t gamestart;
extern gameaction_t gameaction;
extern int gamestate;
extern int controlmenumap;

void G_PlayDemo(char* demoname);
void G_RecordDemo(char* map, char* demo);
void G_StartNewGame(int episode, int skill, int player);
void G_GameLoop();

void G_WarpToMap(char* mapname);

void G_WorldMap();


#define NUMPSPRITES 11

typedef struct
{
	int x1;
	int x2;
	fixed_t scale;
	int iscale;
	int topscreen;
	patch_t* patch;
	int colormap;
	fixed_t fracstep;
} vissprite_t;

typedef struct thing_s
{ // Displayable thing
	struct thing_s* prev;
	struct thing_s* next;
	int sector;
	vissprite_t* vissprite;
	int x;
	int y;
	int z;
	int angle;
	spritenum_t sprite;
	int frame;
	int validcheck;
	int flags;
	void* specialdata;
} thing_t;

typedef struct
{ // Player's commands for a frame
	int8_t xmove;
	int8_t ymove;
	uint8_t buttons;
	uint8_t keyscan;
} framecmd_t;

typedef struct
{
	int sector;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	int angle;
} framepos_t;

typedef struct
{
	int ident;
	int startgame;
	int commint;
	int consoleplayer;
	uint8_t playeringame[4];
	int playercmdframe[4];
	framecmd_t playercmd[128];
	int mousex;
	int mousey;
	int mousebuttons;
	int mousepresent;
	int oldmousex;
	int oldmousey;
	int lastframe;
	framepos_t framepos[2];
	int viewpos;
	char mapname[9];
} shared_t;

//menu

extern font_t* hudfont;
void M_Startup();
void M_DrawSelf();
void M_CheckInput(framecmd_t* cmd);
void M_ClearMenus();
void M_StartControlPanel();

//playsim

typedef struct thinker_s
{ // Any generic thinker
	struct thinker_s* prev;
	struct thinker_s* next;
	void* function;
	int tag;
} thinker_t;

typedef struct actor_s
{ // A generic object that thinks.
	thinker_t thinker;
	thing_t* r;
	struct actor_s* prev;
	struct actor_s* next;
	state_t* state;
	int tics;
	uint8_t* maporigin;
	int health;
} actor_t;

typedef struct
{ // Player view sprite
	state_t* state;
	fixed_t sx;
	fixed_t sy;
	fixed_t speedx;
	fixed_t speedy;
	int tics;
} pspdef_t;

typedef struct
{
	char startname[9];
	char endname[9];
	int numflats;
	int speed;
} flatanim_t;

extern shared_t* sd;

extern int processedframe;

extern memzone_t* playzone;

player_t* player;
player_t playerobjs[];
extern int playerthingfound[];
extern int playernum;
extern int viewplayer;
extern int viewplayerangle;
extern int debugmove;

extern int automapup;
extern int goldshift;
extern int redshift;

extern uint8_t* demoend;
extern int demo;
extern uint8_t* demo_p;

extern actor_t actor;
extern actor_t* actorcap;

extern int wnumber0;
extern int snumber0;
extern fixed_t amaporgy;
extern fixed_t amaporgx;
extern int cardnumber;
extern pic_t* healthbar;
extern pic_t* armorbar;
extern pic_t* timebar;
extern pic_t* blankbar;

//thinkers
void P_AddThinker(thinker_t* thinker);
void P_RemoveThinker(thinker_t* thinker);

uint8_t* P_BlockOrg(int x, int y);
int P_PlaceGetMarks(thing_t* rthing);
void P_PlaceBlockMarks(thing_t* rthing);
void P_RemoveGetMarks(thing_t* rthing);
void P_RemoveBlockMarks(thing_t* rthing);

//playsim
void P_Startup();
void P_InitThinkers();
void P_PlayLoop();

void P_SpawnSpecialSectors();

void P_EnterAutoMap();

//trace
void P_PlayerShoot();
int P_CrossSectorBounds(int sector, fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

//r_*

typedef enum
{
	ls_horziontal,
	ls_vertical,
	ls_slope
} lineslope_t;

typedef struct
{
	fixed_t x;
	fixed_t y;
} point_t;

typedef struct procline_s
{
	struct procline_s* prev;
	struct procline_s* next;
	int side;
	int ipx1;
	int ipx2;
	fixed_t texture1;
	fixed_t texture2;
	fixed_t scale1;
	fixed_t scale2;
	fixed_t scalestep;
	fixed_t scale;
	int line;
	int chained;
	int seg;
	int sector;
	int debug; //[ISB] pls fix
} procline_t;

typedef struct
{
	int type;
	int startcollumn;
	fixed_t collumnstep;
	int starttopscreen;
	fixed_t topscreenstep;
	int starthighscreen;
	fixed_t highscreenstep;
	int startlowscreen;
	fixed_t lowscreenstep;
	int startbottomscreen;
	fixed_t bottomscreenstep;
	int toptextureskip;
	int bottomtextureskip;
	maptexture_t* texture;
	int bottomtexture;
} forwardseg_t;

typedef struct
{
	short p1; // Symbol table implies they're shorts, not ptrs like final. Verify
	short p2;
	short flags;
	short length;
	short special;
	short tag;
	fixed_t bbox[4];
	lineslope_t slopetype;
	fixed_t slope;
	fixed_t yintercept;
	int validcheck;
	int procline;
	int side[2];
	int specialdata;
} line_t;

typedef struct sector_s
{
	fixed_t floorheight;
	fixed_t ceilingheight;
	short floortexture;
	short ceilingtexture;
	short lightlevel;
	short special;
	short tag;
	int audarea;
	int linecount;
	int* lines;
	int numadjacentsectors;
	struct sector_s** adjacentsectors;
	int validcheck;
	thing_t* things;
	void* specialdata;
} sector_t;

typedef struct 
{
	int rotate;
	short lump[8];
	uint8_t flip[8];
} spriteframe_t;

typedef struct  
{
	int numframes;
	spriteframe_t* spriteframes;
} spritedef_t;

typedef struct
{
	fixed_t tx;
	fixed_t tz;
	int texture;
	fixed_t xscale;
	fixed_t yscale;
	int flags;
	int ipx;
} vertex_t;

typedef struct  
{
	int sectornum;
	int xl;
	int xh;
	int numproclines;
	procline_t* proclines;
} subsector_t;

extern int* floorpixel, * ceilingpixel;
extern int* newfloor, * newceiling;
extern int* passfloor, * passceiling;

extern int *esectorscalelight, *esectorscalelight2, *esectorscalelight3;
extern sector_t* sector;

extern int newfseg;
extern int newprocline;
extern int extralight;
extern int vwalldrange;

extern procline_t proclines[];
extern forwardseg_t fsegs[];

extern subsector_t subsectors[];
extern subsector_t* vissec;

extern uint8_t* colormaps;
extern uint16_t* wordcolormaps;

extern int numlines;
extern line_t* lines;

extern int numsides;
extern side_t* sides;

extern int numsectors;
extern sector_t* sectors;

extern int numpoints;
extern point_t* points;

extern int nummappatches;
extern mappatch_t* mappatches;
extern patch_t** patchlookup;

extern int basetextures;
extern int numtextures;

extern int texturelookupsize;
extern maptexture_t** texturelookup;

extern fixed_t maporiginx, maporiginy;
extern int mapwidth, mapheight;
extern uint8_t* blockmap;
extern uint8_t amapcolor[];

extern int viewwidth, viewheight;

extern fixed_t xscale, yscale;
extern fixed_t *inscale, *outscale;

extern int* spans[];
extern int* startspans[];
extern int spanlists[];

extern fixed_t sines[];
extern fixed_t* cosines;

extern uint8_t** flatlookup;

extern fixed_t scalelight[];

extern int viewangle;
extern fixed_t viewx, viewy, viewz;
extern int viewsector;
extern fixed_t viewsin, viewcos;

extern fixed_t xproject, yproject;
extern int sp_x, sp_y1, sp_y2;

extern int centerx, centery;
extern fixed_t centerxfrac, centeryfrac;

extern int sectorxl, sectorxh;

extern int validcheck;

extern fixed_t yslope[];
extern fixed_t cos45;

extern fixed_t viewfrontscale[];
extern fixed_t viewbackscale[];

//textures
void* R_CacheColumn(maptexture_t* tex, int col);

//things
void R_TestSprites(void);
thing_t* R_GetNewThing(int sector);
void R_RemoveThing(thing_t* rthing);
void R_ClearVisSprites(void);

void R_InitSprites(char** namelist);

//sectors
void R_DrawSector(int sectornum, int xl, int xh, int* floorclip, int* ceilingclip, int* scaleclip);

//lines
void R_SetLineTypeAndBox(int linenum);
void R_ClearProclines(void);
void R_ClearFsegs(void);
void R_MakeProcline(int line);
void R_DrawLineDrange();
void R_DrawBlockLine(int linenum, uint8_t bits);

//planes
void R_GenerateSpans(void);
void R_PrepPlanes(void);
void R_DrawPlanes(void);

//game
void R_LoadMap(char* name);

//init
void R_Startup();
void R_TransformVertex(point_t* source, vertex_t* destination);
void R_ChangeWindow(int width, int height, fixed_t vertscale);
int R_LightFromVScale(fixed_t scale);
int R_LightFromZ(fixed_t z);
void R_RenderView(int sectornum, fixed_t x, fixed_t y, fixed_t z, int angle);
void R_DrawPlayerShape(int sprite, int frame, int sx, int sy);

//w_*

typedef struct
{ // Information defining a WAD file
	int identifaction;
	int numlumps;
	int infotableofs;
} wadinfo_t;

typedef struct
{ // Individual WAD lump
	void* position;
	int size;
	char name[8];
} lumpinfo_t;

typedef struct
{
	int filepos;
	int size;
	char name[8];
} filelump_t;

extern int numlumps;
extern lumpinfo_t* lumpinfo;

void W_AddFile(char* filename);
void W_InitMultipleFiles(char** filenames);
void W_InitFile(char* filename);
int W_CheckNumForName(char* name);
int W_GetNumForName(char* name);
void* W_GetLump(int lump);
void* W_GetName(char* name);
