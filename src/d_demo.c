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
#include "r_ref.h"
#include "g_game.h"
#include "m_menu.h"

extern  void R_TestScaleColumn();

demoaction_t demoaction;
int democycle;

config_t config;

typedef struct
{
    fixed_t x, y, z;
    fixed_t texx, texy;
    fixed_t tx, ty, tz;
    fixed_t px, py;
} modelvertex_t;

modelvertex_t vertex[4] =
{ {-160 * FRACUNIT, 100 * FRACUNIT, 0, 0x10000, 0x10000, 0, 0, 0, 0, 0},
{160 * FRACUNIT, 100 * FRACUNIT, 0, 0x13F0000, 0x10000, 0, 0, 0, 0, 0},
{160 * FRACUNIT, -100 * FRACUNIT, 0, 0x13F0000, 0xC70000, 0, 0, 0, 0, 0},
{-160 * FRACUNIT, -100 * FRACUNIT, 0, 0x10000, 0xC70000, 0, 0, 0, 0, 0} };

uint8_t* dm_dest;
int dm_x1, dm_x2;
fixed_t dm_xfrac, dm_yfrac;
fixed_t dm_xstep, dm_ystep;
uint8_t* dm_picture;

uint8_t* rawplane;

void D_PolygonRow()
{
    uint8_t* local_14;
    int local_10;
    int local_8;

    local_8 = dm_x2;
    local_14 = dm_dest + (dm_x2 >> 2);

    local_10 = ((dm_x2) & 3U) * 16000;

    do
    {
        local_14[local_10] = dm_picture[(((dm_yfrac >> 0x10)) << 9) + ((dm_xfrac >> 0x10))];
        dm_xfrac += dm_xstep;
        dm_yfrac += dm_ystep;
        if (local_10 == 48000)
        {
            local_14++;
            local_10 = 0;
        }
        else
        {
            local_10 += 16000;
        }
        local_8++;
    } while (local_8 != dm_x1);
    return;
}

void D_DrawSpinBackground(void)
{
    memset(screenbuffer, 185, 64000);
    return;
}

void D_RenderPolygon(void)
{
    int iVar1;
    int count;
    int dm_y;
    int rightvertex;
    int leftvertex;
    int lefttexystep;
    int lefttexy;
    int lefttexxstep;
    int lefttexx;
    int righttexystep;
    int righttexy;
    int righttexxstep;
    int righttexx;
    int leftstep;
    int rightstep;
    int leftfrac;
    int rightfrac;
    int stopy;

    leftvertex = 0;

    //find the vertex with the lowest y value to start
    dm_y = 1;
    while (dm_y < 4)
    {
        if (vertex[dm_y].py < vertex[leftvertex].py)
        {
            leftvertex = dm_y;
        }
        dm_y++;
    }
    dm_y = vertex[leftvertex].py;


    rightvertex = leftvertex;
    while (1)
    {
        //end clipped off the top of the screen
        if (199 < dm_y)
        {
            return;
        }

        //Reached the left vertex's y value
        if (dm_y == vertex[leftvertex].py)
        {
            while (1)
            {
                leftfrac = vertex[leftvertex].px;
                lefttexx = vertex[leftvertex].texx;
                lefttexy = vertex[leftvertex].texy;
                leftvertex = leftvertex + 1;
                if (leftvertex == 4)
                {
                    leftvertex = 0;
                }
                lefttexystep = vertex[leftvertex].py - dm_y;
                if (lefttexystep != 0) break;
                if (rightvertex == leftvertex)
                {
                    return;
                }
            }
            leftstep = (vertex[leftvertex].px - leftfrac) / lefttexystep;
            lefttexxstep = (vertex[leftvertex].texx - lefttexx) / lefttexystep;
            lefttexystep = (vertex[leftvertex].texy - lefttexy) / lefttexystep;
        }

        //Reached the right? vertex's y value
        if (dm_y == vertex[rightvertex].py)
        {
            do
            {
                rightfrac = vertex[rightvertex].px;
                righttexx = vertex[rightvertex].texx;
                righttexy = vertex[rightvertex].texy;
                rightvertex = rightvertex + -1;
                if (rightvertex == -1)
                {
                    rightvertex = 3;
                }
                righttexystep = vertex[rightvertex].py - dm_y;
            } while (righttexystep == 0);
            rightstep = (vertex[rightvertex].px - rightfrac) / righttexystep;
            righttexxstep = (vertex[rightvertex].texx - righttexx) / righttexystep;
            righttexystep = (vertex[rightvertex].texy - righttexy) / righttexystep;
        }


        if (vertex[leftvertex].py < vertex[rightvertex].py)
        {
            stopy = vertex[leftvertex].py;
        }
        else
        {
            stopy = vertex[rightvertex].py;
        }

        //end clipped off the top of the screen?
        if (stopy < 1)
        {
            dm_y = stopy - dm_y;
            rightfrac = rightfrac + rightstep * dm_y;
            leftfrac = leftfrac + leftstep * dm_y;
            righttexx = righttexx + righttexxstep * dm_y;
            righttexy = righttexy + righttexystep * dm_y;
            lefttexx = lefttexx + lefttexxstep * dm_y;
            lefttexy = lefttexy + lefttexystep * dm_y;
            dm_y = stopy;
        }
        else
        {
            //clip starting point off the top of the screen
            if (dm_y < 0)
            {
                rightfrac = rightfrac - rightstep * dm_y;
                leftfrac = leftfrac - leftstep * dm_y;
                righttexx = righttexx - righttexxstep * dm_y;
                righttexy = righttexy - righttexystep * dm_y;
                lefttexx = lefttexx - lefttexxstep * dm_y;
                lefttexy = lefttexy - lefttexystep * dm_y;
                dm_y = 0;
            }

            //end clipped off of the bottom of the screen
            if (200 < stopy)
            {
                stopy = 200;
            }

            while (dm_y < stopy)
            {
                dm_dest = ylookup[dm_y];
                dm_x2 = rightfrac >> FRACBITS;
                iVar1 = leftfrac >> FRACBITS;
                if (rightfrac < leftfrac)
                {
                    count = (iVar1 - dm_x2) + 1;
                    dm_x1 = iVar1;
                    if (0 < count)
                    {
                        dm_xstep = (lefttexx - righttexx) / count;
                        dm_ystep = (lefttexy - righttexy) / count;
                        dm_xfrac = righttexx;
                        dm_yfrac = righttexy;
                        //LAB_0001cb49:
                        if (dm_x2 < 0)
                        {
                            dm_xfrac = dm_xfrac - dm_x2 * dm_xstep;
                            dm_x2 = 0;
                        }
                        if (319 < dm_x1)
                        {
                            dm_x1 = 319;
                        }
                        if (dm_x2 < dm_x1)
                        {
                            D_PolygonRow();
                        }
                        rightfrac = rightfrac + rightstep;
                        leftfrac = leftfrac + leftstep;
                        righttexx = righttexx + righttexxstep;
                        righttexy = righttexy + righttexystep;
                        lefttexx = lefttexx + lefttexxstep;
                        lefttexy = lefttexy + lefttexystep;
                    }
                }
                else
                {
                    count = (dm_x2 - iVar1) + 1;
                    dm_x1 = dm_x2;
                    dm_x2 = iVar1;
                    if (0 < count)
                    {
                        dm_xstep = (righttexx - lefttexx) / count;
                        dm_ystep = (righttexy - lefttexy) / count;
                        dm_xfrac = lefttexx;
                        dm_yfrac = lefttexy;

                        if (dm_x2 < 0)
                        {
                            dm_xfrac = dm_xfrac - dm_x2 * dm_xstep;
                            dm_x2 = 0;
                        }
                        if (319 < dm_x1)
                        {
                            dm_x1 = 319;
                        }
                        if (dm_x2 < dm_x1)
                        {
                            D_PolygonRow();
                        }
                        rightfrac = rightfrac + rightstep;
                        leftfrac = leftfrac + leftstep;
                        righttexx = righttexx + righttexxstep;
                        righttexy = righttexy + righttexystep;
                        lefttexx = lefttexx + lefttexxstep;
                        lefttexy = lefttexy + lefttexystep;
                        //goto LAB_0001cb49;
                    }
                }
                dm_y++;
            }
        }
        if (leftvertex == rightvertex)
        {
            return;
        }
    }
}

void D_MapPlane(int xr, int yr, int zr, int scale)
{
    fixed_t matr[4]; //TODO: I really need to figure out how to extract type information. This may only be 8 byte sof stack
    fixed_t cx, cy, cz;
    fixed_t sx, sy, sz;
    int i;

    cx = cosines[xr];
    cy = cosines[yr];
    cz = cosines[zr];
    sx = sines[xr];
    sy = sines[yr];
    sz = sines[zr];

    matr[0] = FixedMul(cz, cy);
    matr[1] = FixedMul(FixedMul(sz, sx), sy);

    matr[2] = FixedMul(-sz, cy);
    matr[3] = FixedMul(FixedMul(cz, sx), sy);

    for (i = 0; i < 4; i++)
    {
        vertex[i].tx = FixedMul(vertex[i].x, matr[0] + matr[1]) 
            + FixedMul(vertex[i].y, matr[2] + matr[3]) 
            + FixedMul(vertex[i].z, FixedMul(cx, sy));

        vertex[i].ty = FixedMul(vertex[i].x, FixedMul(sz, cx))
            + FixedMul(vertex[i].y, FixedMul(cz, cx))
            + FixedMul(vertex[i].z, -sx);

        vertex[i].px = FixedMul(vertex[i].tx, scale) + (160 << FRACBITS);
        vertex[i].py = 100 - (FixedMul(vertex[i].ty, scale) >> FRACBITS);
    }
    D_DrawSpinBackground();
    D_RenderPolygon();
    V_MarkUpdateBlock(0, 0, SCREENWIDTH-1, SCREENHEIGHT-1);
    IO_UpdateScreen();
}

void D_PlaneToBuffer(uint8_t* source)
{
    uint8_t* dest;
    int x, y;

    dest = screenbuffer;
    for (y = 0; y < SCREENHEIGHT; y++)
    {
        for (x = 0; x < SCREENBWIDE; x++)
        {
            *dest = *source;
            dest[16000] = source[1];
            dest[32000] = source[2];
            dest[48000] = source[3];
            source += 4;
            dest++;
        }
        source += 0xc0;
    }
    V_MarkUpdateBlock(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - 1);
    IO_UpdateScreen();
    return;
}

void D_BufferToPlane(uint8_t* dest)
{
    uint8_t* source;
    int x, y;

    source = screenbuffer;
    for (y = 0; y < SCREENHEIGHT; y++)
    {
        for (x = 0; x < SCREENBWIDE; x++)
        {
            *dest = *source;
            dest[1] = source[16000];
            dest[2] = source[32000];
            dest[3] = source[48000];
            dest += 4;
            source++;
        }
        dest += 0xc0;
    }
}

int D_SpinScreenIn(void)
{
    int start;
    int f;
    fixed_t scale;
    int xa, ya, za; //for some reason the stack has space for 3 variables even though they're all the same..
    int c;

    dm_picture = (uint8_t*)malloc(0x19000);
    rawplane = dm_picture;
    D_BufferToPlane(dm_picture);
    IO_StartAck();
    lastscan = 0;
    start = IO_GetTime();
    do
    {
        f = IO_GetTime() - start;
        if (119 < f)
        {
            D_PlaneToBuffer(rawplane);
            free(rawplane);
            return 0;
        }
        xa = ya = za = (f * 10240) / 120 - 2048 & ANGLEMASK;
        scale = FixedDiv(10 << FRACBITS, (150 - (f * 140) / 120) << FRACBITS);
        D_MapPlane(xa, ya, za, scale);
        c = IO_CheckAck();
    } while (c == 0);
    return c;
}

int D_SpinScreenOut(void)
{
    int start;
    int f;
    fixed_t scale;
    int xa, ya, za; //for some reason the stack has space for 3 variables even though they're all the same..
    int c;

    dm_picture = (uint8_t*)malloc(0x19000);
    rawplane = dm_picture;
    D_BufferToPlane(dm_picture);
    IO_StartAck();
    lastscan = 0;
    start = IO_GetTime();
    do
    {
        f = IO_GetTime() - start;
        if (119 < f)
        {
            D_DrawSpinBackground();
            V_MarkUpdateBlock(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - 1);
            IO_UpdateScreen();
            free(rawplane);
            return 0;
        }
        xa = ya = za = (f * 10240) / 120 & ANGLEMASK;
        scale = FixedDiv(10 << FRACBITS, ((f * 140) / 120 + 10) << FRACBITS);
        D_MapPlane(xa, ya, za, scale);
        c = IO_CheckAck();
    } while (c == 0);
    return c;
}

void D_SpinScreen(void)
{
    int c;

    c = D_SpinScreenIn();
    if (c == 0)
    {
        c = D_AckWait(TICRATE * 4);
        if (c == 0)
        {
            c = D_SpinScreenOut();
            if (c != 0)
            {
                gamestart = gs_controlmap;
                demoaction = da_startgame;
            }
        }
        else
        {
            gamestart = gs_controlmap;
            demoaction = da_startgame;
        }
    }
    else
    {
        gamestart = gs_controlmap;
        demoaction = da_startgame;
    }
    return;
}

void D_Title()
{
    demoaction = da_demo1;
    IO_SetPalette(W_GetName("TITLEPAL"));
    V_DrawPic(0, 0, (pic_t*)W_GetName("TITLEPIC"));
    D_SpinScreen();
}

void D_Credits()
{
    demoaction = da_demo2;
    V_Bar(0, 0, 320, 200, 0);
    V_CenterString(80, "CREDITS", hudfont);
    D_SpinScreen();
    return;
}

void D_Story()
{
    demoaction = da_demo3;
    V_Bar(0, 0, 320, 200, 0);
    V_CenterString(80, "SPINNING PLANET / STORY", hudfont);
    D_SpinScreen();
    return;
}

void D_HighScores()
{
    demoaction = da_demo4;
    V_Bar(0, 0, 320, 200, 0);
    V_CenterString(80, "HIGH SCORES", hudfont);
    D_SpinScreen();
    return;
}

void D_SaveConfig()
{
    //TODO: portability, alignment safe, and so on.
    D_WriteFile("config.lmp", (uint8_t*)&config, sizeof(config));
    return;
}

void D_LoadConfig()
{
    int i, num;

    num = W_CheckNumForName("CONFIG");
    if (num == -1)
    {
        config.hdetail = 1;
        config.viewsize = 8;
        for (i = 0; i < NUMHISCORES; i++)
        {
            strncpy(config.scores[i].name, "Id Software", 16);
            config.scores[i].skill = sk_deadly;
            config.scores[i].score = 10000;
        }
    }
    else
    {
#if 0 //{ISB] "safe" copy here
        local_EAX_63 = (int)memcpy(&config, src, lumpinfo[local_EAX_63].size);
#else
        int* src = W_GetLump(num);
        config.hdetail = *src; src++;
        config.viewsize = *src; src++;

        for (i = 0; i < NUMHISCORES; i++)
        {
            memcpy(config.scores[i].name, (void*)src, 16); src += 4;
            config.scores[i].skill = *src; src++;
            config.scores[i].score = *src; src++;
        }
#endif
    }
}

void D_DemoLoop(void)
{
    int p;

    demoaction = da_filmwarp;
    p = D_CheckParm("run");
    if (p != 0)
    {
        p = atoi(my_argv[p + 1]);
        G_StartNewGame(p, sk_baby, 0);
        demoaction = da_startgame;
        gamestart = gs_warp;
    }

    p = D_CheckParm("play");
    if (p != 0)
    {
        G_PlayDemo(my_argv[p + 1]);
        G_GameLoop();
        IO_Quit();
    }

    p = D_CheckParm("record");
    if (p != 0)
    {
        if (my_argc <= p + 2)
        {
            IO_Error("doom RECORD <level> <demoname>\n");
        }
        G_RecordDemo(my_argv[p + 1], my_argv[p + 2]);
        IO_Quit();
    }
    if (commpresent != 0)
    {
        demoaction = da_startgame;
        gamestart = gs_netgame;
    }
    while (1)
    {
        switch (demoaction)
        {
        case da_filmwarp:
            democycle = da_demo1;
            D_Title();
            break;
        case da_story:
            democycle = da_demo3;
            D_Story();
            break;
        case da_credits:
            democycle = da_demo2;
            D_Credits();
            break;
        case da_scores:
            democycle = da_demo4;
            D_HighScores();
            break;
        case da_demo1:
            democycle = da_credits;
            G_PlayDemo("DEMO1");
            break;
        case da_demo2:
            democycle = da_story;
            G_PlayDemo("DEMO2");
            break;
        case da_demo3:
            democycle = da_scores;
            G_PlayDemo("DEMO3");
            break;
        case da_demo4:
            democycle = da_filmwarp;
            G_PlayDemo("DEMO4");
            break;
        case da_startgame:
            G_GameLoop();
            if (demoaction != da_startgame)
            {
                demoaction = democycle;
            }
            break;
        default:
            IO_Error("D_DemoLoop: unknown demoaction\n");
            return;
        }
        D_FadeOut();
    }
}
