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
#include "w_wad.h"
#include "g_game.h"
#include "r_ref.h"

typedef struct
{
	int x, y;
	int north, east, south, west;
	char* mapname;
} mappt_t;

mappt_t mappts[16] =
{ 
/*
 X   Y      N   E   S    W Map name
------------------------------------*/
{70,  160, -2,  0,  0,   0,  "E1M1"},    //[1] Hangar 2
{90,  136, -3,  0,  1,   0,  "E1M2"},    //[2] Supply Depot 2
{104, 124,  0,  5,  2,   4,  NULL},      //[3] Fork
{64,  100,  0,  3,  0,   0,  "E1M7"},    //[4] Recreation and training center
{154, 128,  0,  7,  6,   3,  NULL},      //[5] Fork
{168, 168,  5,  0,  0,   0,  "E1M3"},    //[6] Waste processing facility
{180, 124, -8,  0,  0,   6,  "E1M4"},    //[7] Refinery
{172, 110, -10, 9,  7,   0,  "E1M5"},    //[8] Power plant
{222, 96,   0,  0,  0,   8,  "E1M6"},    //[9] Quarters
{160, 86,  -11, 0,  9,   0,  "*E1M9"},   //[10] Control Center
{148, 70,   13, 14, 10,  12, NULL},      //[11] Fork
{104, 104,  11, 0,  0,   0,  "E1M8"},    //[12] Communication Tower
{104, 44,  -15, 11, 11, -16, "S3E1M10"}, //[13] Lab
{184, 56,  -15, 0,  0,   11, "E1M11"},   //[14] Supply Depot 1
{142, 30,   0,  14, 0,   13, "E1M12"},   //[15] Anomaly
{32,  16,   0,  13, 13,  0,  "E1M13"}    //[16] Observatory
};

void G_WorldMap()
{
    char* mapname;
    V_DrawPic(0, 0, (pic_t*)W_GetName("WORLD1"));
    IO_UpdateScreen();
    D_FadeIn(W_GetName("PLAYPAL"));
    IO_Ack();
    if (gameaction == ga_completed) 
    {
        do 
        {
            if (gamestate.mappoint++ == 15) 
            {
                gameaction = ga_victory;
                return;
            }
            mapname = mappts[gamestate.mappoint].mapname;
        } while (mapname == NULL);
    }
    G_WarpToMap(mappts[gamestate.mappoint].mapname);
    gameaction = ga_runmap;
    return;
}
