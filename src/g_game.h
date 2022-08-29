#pragma once

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

//TODO: I didn't notice this structure
//Need to determine the proper data types still, and if it's actually used anywhere.  
typedef struct
{
	int mapcomplete;
	int mappoint;
} gamestate_t;

extern gamestart_t gamestart;
extern gameaction_t gameaction;
extern gamestate_t gamestate;
extern int controlmenumap;

void G_PlayDemo(char* demoname);
void G_RecordDemo(char* map, char* demo);
void G_StartNewGame(int episode, int skill, int player);
void G_GameLoop();

void G_WarpToMap(char* mapname);

void G_WorldMap();
