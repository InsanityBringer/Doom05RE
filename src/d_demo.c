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

byte* dm_dest;
int dm_x1, dm_x2;
fixed_t dm_xfrac, dm_yfrac;
fixed_t dm_xstep, dm_ystep;
byte* dm_picture;

byte* rawplane;

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
void D_RenderPolygon()
{
    int stopy;
    fixed_t leftfrac, rightfrac;
    fixed_t leftstep, rightstep;
    fixed_t lefttexx, lefttexxstep, lefttexy, lefttexystep;
    fixed_t righttexx, righttexxstep, righttexy, righttexystep;
    int topvertex, leftvertex, rightvertex;
    int deltay, count;
    int dm_y;

    topvertex = 0;
    for (leftvertex = 1; leftvertex < 4; leftvertex++)
    {
        if (vertex[leftvertex].py < vertex[topvertex].py)
        {
            topvertex = leftvertex;
        }
    }
    rightvertex = topvertex;
    leftvertex = topvertex;
    dm_y = vertex[topvertex].py;
    if (dm_y >= SCREENHEIGHT)
        return;

    if (dm_y != vertex[rightvertex].py)
        return;

    do
    {
        if (dm_y == vertex[rightvertex].py)
        {
            while (1)
            {
                rightfrac = vertex[rightvertex].px;
                righttexx = vertex[rightvertex].texx;
                righttexy = vertex[rightvertex].texy;
                rightvertex++;
                if (rightvertex == 4)
                    rightvertex = 0;

                deltay = vertex[rightvertex].py - dm_y;
                if (deltay != 0)
                    break;
                else if (leftvertex == rightvertex)
                    return;
            }

            rightstep = (vertex[rightvertex].px - rightfrac) / deltay;
            righttexxstep = (vertex[rightvertex].texx - righttexx) / deltay;
            righttexystep = (vertex[rightvertex].texy - righttexy) / deltay;
        }

        if (dm_y == vertex[leftvertex].py)
        {
            do
            {
                leftfrac = vertex[leftvertex].px;
                lefttexx = vertex[leftvertex].texx;
                lefttexy = vertex[leftvertex].texy;
                leftvertex--;
                if (leftvertex < 0)
                    leftvertex = 3;

                deltay = vertex[leftvertex].py - dm_y;
            } while (deltay == 0);

            leftstep = (vertex[leftvertex].px - leftfrac) / deltay;
            lefttexxstep = (vertex[leftvertex].texx - lefttexx) / deltay;
            lefttexystep = (vertex[leftvertex].texy - lefttexy) / deltay;
        }

        if (vertex[rightvertex].py < vertex[leftvertex].py)
            stopy = vertex[rightvertex].py;
        else
            stopy = vertex[leftvertex].py;

        if (stopy <= 0)
        {
            count = stopy - dm_y;
            leftfrac += leftstep * count;
            rightfrac += rightstep * count;
            lefttexx += lefttexxstep * count;
            lefttexy += lefttexystep * count;
            righttexx += righttexxstep * count;
            righttexy += righttexystep * count;
            dm_y = stopy;
        }
        else
        {
            if (dm_y < 0)
            {
                leftfrac -= leftstep * dm_y;
                rightfrac -= rightstep * dm_y;
                lefttexx -= lefttexxstep * dm_y;
                lefttexy -= lefttexystep * dm_y;
                righttexx -= righttexxstep * dm_y;
                righttexy -= righttexystep * dm_y;
                dm_y = 0;
            }

            if (stopy > 200)
                stopy = 200;

            for (; dm_y < stopy; dm_y++)
            {
                dm_dest = ylookup[dm_y];
                if (leftfrac < rightfrac)
                {
                    dm_x2 = leftfrac >> FRACBITS;
                    dm_x1 = rightfrac >> FRACBITS;
                    count = dm_x1 - dm_x2 + 1;
                    if (count < 1)
                        continue;

                    dm_xfrac = lefttexx;
                    dm_yfrac = lefttexy;
                    dm_xstep = (righttexx - lefttexx) / count;
                    dm_ystep = (righttexy - lefttexy) / count;
                }
                else
                {
                    dm_x1 = leftfrac >> FRACBITS;
                    dm_x2 = rightfrac >> FRACBITS;
                    count = dm_x1 - dm_x2 + 1;
                    if (count < 1)
                        continue;
                    dm_xfrac = righttexx;
                    dm_yfrac = righttexy;
                    dm_xstep = (lefttexx - righttexx) / count;
                    dm_ystep = (lefttexy - righttexy) / count;
                }

                if (dm_x2 < 0)
                {
                    count = -dm_x2;
                    dm_xfrac += count * dm_xstep;
                    dm_x2 = 0;
                }

                if (dm_x1 > SCREENWIDTH - 1)
                    dm_x1 = SCREENWIDTH - 1;

                if (dm_x1 > dm_x2)
                    D_PolygonRow();

                leftfrac += leftstep;
                rightfrac += rightstep;
                lefttexx += lefttexxstep;
                lefttexy += lefttexystep;
                righttexx += righttexxstep;
                righttexy += righttexystep;
            }
        }

        if (rightvertex == leftvertex)
            return;

    } while (dm_y < SCREENHEIGHT - 1);
}

void D_MapPlane(int xr, int yr, int zr, int scale)
{
    fixed_t matr[9];
    fixed_t cx, cy, cz;
    fixed_t sx, sy, sz;
    modelvertex_t* pt;
    int i;

    cx = cosines[xr];
    cy = cosines[yr];
    cz = cosines[zr];
    sx = sines[xr];
    sy = sines[yr];
    sz = sines[zr];

    matr[0] = FixedMul(cz, cy) + FixedMul(FixedMul(sz, sx), sy);
    matr[1] = FixedMul(sz, cx);
    matr[2] = FixedMul(cz, -sy) + FixedMul(FixedMul(sx, sz), cy);
    matr[3] = FixedMul(-sz, cy) + FixedMul(FixedMul(cz, sx), sy);
    matr[4] = FixedMul(cx, cz);
    matr[5] = FixedMul(-sy, -sz) + FixedMul(FixedMul(cx, sx), cy);
    matr[6] = FixedMul(sy, cx);
    matr[7] = -sx;
    matr[8] = FixedMul(cx, cy);

    for (i = 0; i < 4; i++)
    {
        pt = &vertex[i];
        pt->tx = FixedMul(pt->x, matr[0])
            + FixedMul(pt->y, matr[3])
            + FixedMul(pt->z, matr[6]);

        pt->ty = FixedMul(pt->x, matr[1])
            + FixedMul(pt->y, matr[4])
            + FixedMul(pt->z, matr[7]);

        pt->px = FixedMul(pt->tx, scale) + ((SCREENWIDTH / 2) << FRACBITS);
        pt->py = (SCREENHEIGHT / 2) - (FixedMul(pt->ty, scale) >> FRACBITS);
    }
    D_DrawSpinBackground();
    D_RenderPolygon();
    V_MarkUpdateBlock(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - 1);
    IO_UpdateScreen();
}

void D_PlaneToBuffer(byte* source)
{
    int x, y;
    byte* dest;

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
        source += 192;
    }
    V_MarkUpdateBlock(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - 1);
    IO_UpdateScreen();
    return;
}

void D_BufferToPlane(byte* dest)
{
    int x, y;
    byte* source;

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
        dest += 192;
    }
}

int D_SpinScreenIn(void)
{
    int c;
    int xa, ya, za;
    fixed_t scale;
    int z, f;
    int start, newt;

    dm_picture = (byte*)malloc(0x19000);
    rawplane = dm_picture;
    D_BufferToPlane(dm_picture);
    IO_StartAck();
    lastscan = 0;
    start = IO_GetTime();
    while(1)
    {
        newt = IO_GetTime();
        f = newt - start;
        if (f >= 120)
            break;

        xa = ya = za = (f * 10240) / 120 - 2048 & ANGLEMASK;
        z = (150 - (f * 140) / 120) << FRACBITS;
        scale = FixedDiv(10 << FRACBITS, z);
        D_MapPlane(xa, ya, za, scale);
        c = IO_CheckAck();
        if (c)
            return c;
    }

    D_PlaneToBuffer(rawplane);
    free(rawplane);
    return 0;
}

int D_SpinScreenOut(void)
{
    int c;
    int xa, ya, za;
    fixed_t scale;
    int z, f;
    int start, newt;

    dm_picture = (byte*)malloc(0x19000);
    rawplane = dm_picture;
    D_BufferToPlane(dm_picture);
    IO_StartAck();
    lastscan = 0;
    start = IO_GetTime();
    while (1)
    {
        newt = IO_GetTime();
        f = newt - start;
        if (f >= 120)
            break;

        xa = ya = za = (f * 10240) / 120 & ANGLEMASK;
        z = ((f * 140) / 120 + 10) << FRACBITS;
        scale = FixedDiv(10 << FRACBITS, z);
        D_MapPlane(xa, ya, za, scale);
        c = IO_CheckAck();
        if (c)
            return c;
    }

    D_DrawSpinBackground();
    V_MarkUpdateBlock(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - 1);
    IO_UpdateScreen();
    free(rawplane);
    return 0;
}

void D_SpinScreen(void)
{
    int c;

    c = D_SpinScreenIn();
    if (c)
    {
        gamestart = gs_controlmap;
        demoaction = da_startgame;
        return;
    }

    c = D_AckWait(TICRATE * 4);
    if (c)
    {
        gamestart = gs_controlmap;
        demoaction = da_startgame;
        return;
    }

    c = D_SpinScreenOut();
    if (c)
    {
        gamestart = gs_controlmap;
        demoaction = da_startgame;
    }
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
}

void D_Story()
{
    demoaction = da_demo3;
    V_Bar(0, 0, 320, 200, 0);
    V_CenterString(80, "SPINNING PLANET / STORY", hudfont);
    D_SpinScreen();
}

void D_HighScores()
{
    demoaction = da_demo4;
    V_Bar(0, 0, 320, 200, 0);
    V_CenterString(80, "HIGH SCORES", hudfont);
    D_SpinScreen();
}

void D_SaveConfig()
{
    //TODO: portability, alignment safe, and so on.
    D_WriteFile("config.lmp", (uint8_t*)&config, sizeof(config));
}

void D_LoadConfig()
{
    int num, i;

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
#if 0
        memcpy(&config, lumpinfo[num].position, lumpinfo[num].size);
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
            IO_Error("doom RECORD <level> <demoname>");
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
            IO_Error("D_DemoLoop: unknown demoaction");
            return;
        }
        D_FadeOut();
    }
}
