#pragma once

//The play loop is dependent on the refresh
#include "r_ref.h"

typedef struct
{
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

	wp_nochange = 15
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

#define SBARHEIGHT 32

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

extern byte* demoend;
extern demostate_t demo;
extern byte* demo_p;

extern int wnumber0;
extern int snumber0;
extern fixed_t amaporgy;
extern fixed_t amaporgx;
extern int cardnumber;
extern pic_t* healthbar;
extern pic_t* armorbar;
extern pic_t* timebar;
extern pic_t* blankbar;

//playsim
void P_Startup();
void P_InitThinkers();
void P_PlayLoop();

void P_SpawnSpecialSectors();

void P_EnterAutoMap();

void P_InitThing(mapthing_t* mthing);

//trace
void P_PlayerShoot();
int P_CrossSectorBounds(int sector, fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

//statues
void P_DrawPlayScreen(); //Needed by platform-specific video code. 
