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

extern  void R_TestScaleColumn();

demoaction_t demoaction;
int democycle;

config_t config;

typedef struct
{
    fixed_t f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
} planepoint_t;

planepoint_t vertex[4] =
{ {0xFF600000, 0x640000, 0, 0x10000, 0x10000, 0, 0, 0, 0, 0},
{0xA00000, 0x640000, 0, 0x13F0000, 0x10000, 0, 0, 0, 0, 0},
{0xA00000, 0xFF9C0000, 0, 0x13F0000, 0xC70000, 0, 0, 0, 0, 0},
{0xFF600000, 0xFF9C0000, 0, 0x10000, 0xC70000, 0, 0, 0, 0, 0} };

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

    /*if ((((dm_x1 < 0) || (viewwidth + 1 < dm_x2)) || (dm_x2 <= dm_x1)))
    {
        IO_Error("MapRow: mr_x1 = %i  mr_x2 = %i", dm_x1, dm_x2);
        return;
    }*/
    /*if (dm_picture == NULL)
    {
        IO_Error("MapRow: NULL picture");
        return;
    }*/
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
    memset(screenbuffer, 0xb9, 64000);
    return;
}

void D_RenderPolygon(void)
{
    int iVar1;
    int iVar2;
    int local_54;
    int iVar3;
    int local_50;
    int local_4c;
    int local_48;
    int local_44;
    int local_40;
    int local_3c;
    int local_38;
    int local_34;
    int local_30;
    int local_2c;
    int local_28;
    int local_24;
    int local_20;
    int iVar4;

    local_50 = 0;
    local_54 = 1;
    while (local_54 < 4)
    {
        if (vertex[local_54].f10 < vertex[local_50].f10)
        {
            local_50 = local_54;
        }
        local_54 = local_54 + 1;
    }
    local_54 = vertex[local_50].f10;
    iVar3 = local_50;
    while (1)
    {
        if (199 < local_54)
        {
            return;
        }
        if (local_54 == vertex[local_50].f10)
        {
            while (1)
            {
                local_24 = vertex[local_50].f9;
                local_40 = vertex[local_50].f4;
                local_48 = vertex[local_50].f5;
                local_50 = local_50 + 1;
                if (local_50 == 4)
                {
                    local_50 = 0;
                }
                local_4c = vertex[local_50].f10 - local_54;
                if (local_4c != 0) break;
                if (iVar3 == local_50)
                {
                    return;
                }
            }
            local_2c = (vertex[local_50].f9 - local_24) / local_4c;
            local_44 = (vertex[local_50].f4 - local_40) / local_4c;
            local_4c = (vertex[local_50].f5 - local_48) / local_4c;
        }
        if (local_54 == vertex[iVar3].f10)
        {
            do
            {
                local_20 = vertex[iVar3].f9;
                local_30 = vertex[iVar3].f4;
                local_38 = vertex[iVar3].f5;
                iVar3 = iVar3 + -1;
                if (iVar3 == -1)
                {
                    iVar3 = 3;
                }
                local_3c = vertex[iVar3].f10 - local_54;
            } while (local_3c == 0);
            local_28 = (vertex[iVar3].f9 - local_20) / local_3c;
            local_34 = (vertex[iVar3].f4 - local_30) / local_3c;
            local_3c = (vertex[iVar3].f5 - local_38) / local_3c;
        }
        if (vertex[local_50].f10 < vertex[iVar3].f10)
        {
            iVar4 = vertex[local_50].f10;
        }
        else
        {
            iVar4 = vertex[iVar3].f10;
        }
        if (iVar4 < 1)
        {
            local_54 = iVar4 - local_54;
            local_20 = local_20 + local_28 * local_54;
            local_24 = local_24 + local_2c * local_54;
            local_30 = local_30 + local_34 * local_54;
            local_38 = local_38 + local_3c * local_54;
            local_40 = local_40 + local_44 * local_54;
            local_48 = local_48 + local_4c * local_54;
            local_54 = iVar4;
        }
        else
        {
            if (local_54 < 0)
            {
                local_20 = local_20 - local_28 * local_54;
                local_24 = local_24 - local_2c * local_54;
                local_30 = local_30 - local_34 * local_54;
                local_38 = local_38 - local_3c * local_54;
                local_40 = local_40 - local_44 * local_54;
                local_48 = local_48 - local_4c * local_54;
                local_54 = 0;
            }
            if (200 < iVar4)
            {
                iVar4 = 200;
            }
            while (local_54 < iVar4)
            {
                dm_dest = ylookup[local_54];
                dm_x2 = local_20 >> 0x10;
                iVar1 = local_24 >> 0x10;
                if (local_20 < local_24)
                {
                    iVar2 = (iVar1 - dm_x2) + 1;
                    dm_x1 = iVar1;
                    if (0 < iVar2)
                    {
                        dm_xstep = (local_40 - local_30) / iVar2;
                        dm_ystep = (local_48 - local_38) / iVar2;
                        dm_xfrac = local_30;
                        dm_yfrac = local_38;
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
                        local_20 = local_20 + local_28;
                        local_24 = local_24 + local_2c;
                        local_30 = local_30 + local_34;
                        local_38 = local_38 + local_3c;
                        local_40 = local_40 + local_44;
                        local_48 = local_48 + local_4c;
                    }
                }
                else
                {
                    iVar2 = (dm_x2 - iVar1) + 1;
                    dm_x1 = dm_x2;
                    dm_x2 = iVar1;
                    if (0 < iVar2)
                    {
                        dm_xstep = (local_30 - local_40) / iVar2;
                        dm_ystep = (local_38 - local_48) / iVar2;
                        dm_xfrac = local_40;
                        dm_yfrac = local_48;

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
                        local_20 = local_20 + local_28;
                        local_24 = local_24 + local_2c;
                        local_30 = local_30 + local_34;
                        local_38 = local_38 + local_3c;
                        local_40 = local_40 + local_44;
                        local_48 = local_48 + local_4c;
                        //goto LAB_0001cb49;
                    }
                }
                local_54++;
            }
        }
        if (local_50 == iVar3)
        {
            return;
        }
    }
}

void D_MapPlane(int xr, int yr, int zr, int scale)
{
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    int lVar6;
    int lVar7;
    int lVar8;
    int lVar9;
    int lVar10;
    int lVar11;
    int lVar12;
    int local_30;

    iVar1 = cosines[xr];
    iVar2 = cosines[zr];
    iVar3 = sines[xr];
    iVar4 = sines[yr];
    iVar5 = sines[zr];

    lVar6 = FixedMul(iVar2, cosines[yr]);
    lVar7 = FixedMul(FixedMul(iVar5, iVar3), iVar4);

    lVar8 = FixedMul(-iVar5, cosines[yr]);
    lVar9 = FixedMul(FixedMul(iVar2, iVar3), iVar4);

    local_30 = 0;
    while (local_30 < 4)
    {
        lVar10 = FixedMul(vertex[local_30].f1, lVar6 + lVar7);
        lVar11 = FixedMul(vertex[local_30].f2, lVar8 + lVar9);
        lVar12 = FixedMul(vertex[local_30].f3, FixedMul(iVar1, iVar4));

        vertex[local_30].f6 = lVar10 + lVar11 + lVar12;

        lVar10 = FixedMul(vertex[local_30].f1, FixedMul(iVar5, iVar1));
        lVar11 = FixedMul(vertex[local_30].f2, FixedMul(iVar2, iVar1));
        lVar12 = FixedMul(vertex[local_30].f3, -iVar3);

        vertex[local_30].f7 = lVar10 + lVar11 + lVar12;

        vertex[local_30].f9 = FixedMul(vertex[local_30].f6, scale) + 0xA00000;
        vertex[local_30].f10 = 100 - (FixedMul(vertex[local_30].f7, scale) >> FRACBITS);

        local_30++;
    }
    D_DrawSpinBackground();
    D_RenderPolygon();
    V_MarkUpdateBlock(0, 0, 0x13f, 199);
    IO_UpdateScreen();
}

void D_PlaneToBuffer(uint8_t* source)
{
    uint8_t* pbVar1;
    uint8_t* local_28;
    uint8_t* local_24;
    int local_20;
    int iVar2;

    local_24 = screenbuffer;
    local_20 = 0;
    local_28 = source;
    while (local_20 < 200)
    {
        iVar2 = 0;
        while (iVar2 < 0x50)
        {
            *local_24 = *local_28;
            local_24[16000] = local_28[1];
            pbVar1 = local_28 + 3;
            local_24[32000] = local_28[2];
            local_28 = local_28 + 4;
            local_24[48000] = *pbVar1;
            local_24 = local_24 + 1;
            iVar2 = iVar2 + 1;
        }
        local_28 = local_28 + 0xc0;
        local_20 = local_20 + 1;
    }
    V_MarkUpdateBlock(0, 0, 319, 199);
    IO_UpdateScreen();
    return;
}

int D_BufferToPlane(uint8_t* dest)
{
    uint8_t* pbVar1;
    uint8_t* local_28;
    uint8_t* local_24;
    int pbVar2;
    int iVar3;

    local_24 = screenbuffer;
    pbVar1 = dest;
    local_28 = dest;
    pbVar2 = 0;
    while (pbVar2 < 200)
    {
        iVar3 = 0;
        while (iVar3 < 0x50)
        {
            *local_28 = *local_24;
            local_28[1] = local_24[16000];
            pbVar1 = local_28 + 3;
            local_28[2] = local_24[32000];
            local_28 = local_28 + 4;
            *pbVar1 = local_24[48000];
            local_24 = local_24 + 1;
            iVar3 = iVar3 + 1;
        }
        local_28 = local_28 + 0xc0;
        pbVar1 = pbVar2;
        pbVar2++;
    }
    return (int)pbVar1;
}

int D_SpinScreenIn(void)
{
    int local_38;
    int iVar1;
    int xr;

    dm_picture = (uint8_t*)malloc(0x19000);
    rawplane = dm_picture;
    D_BufferToPlane(dm_picture);
    IO_StartAck();
    lastscan = 0;
    local_38 = IO_GetTime();
    do
    {
        iVar1 = IO_GetTime();
        iVar1 = iVar1 - local_38;
        if (119 < iVar1)
        {
            D_PlaneToBuffer(rawplane);
            free(rawplane);
            return 0;
        }
        xr = (iVar1 * 0x2800) / 0x78 - 0x800U & 0x1fff;
        D_MapPlane(xr, xr, xr, FixedDiv(0xA0000, (0x96 - (iVar1 * 0x8c) / 0x78) << FRACBITS));
        iVar1 = IO_CheckAck();
    } while (iVar1 == 0);
    return iVar1;
}

int D_SpinScreenOut(void)
{
    int local_38;
    int iVar1;
    int xr;

    dm_picture = (uint8_t*)malloc(0x19000);
    rawplane = dm_picture;
    D_BufferToPlane(dm_picture);
    IO_StartAck();
    lastscan = 0;
    local_38 = IO_GetTime();
    do
    {
        iVar1 = IO_GetTime();
        iVar1 = iVar1 - local_38;
        if (119 < iVar1)
        {
            D_DrawSpinBackground();
            V_MarkUpdateBlock(0, 0, 319, 199);
            IO_UpdateScreen();
            free(rawplane);
            return 0;
        }
        xr = (iVar1 * 0x2800) / 0x78 & 0x1fff;
        D_MapPlane(xr, xr, xr, FixedDiv(0xA0000, ((iVar1 * 0x8c) / 0x78 + 10) << FRACBITS));
        iVar1 = IO_CheckAck();
    } while (iVar1 == 0);
    return iVar1;
}

void D_SpinScreen(void)
{
    int iVar1;

    iVar1 = D_SpinScreenIn();
    if (iVar1 == 0)
    {
        iVar1 = D_AckWait(0x8c);
        if (iVar1 == 0)
        {
            iVar1 = D_SpinScreenOut();
            if (iVar1 != 0)
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
    uint8_t* pal;
    pic_t* pic;

    demoaction = da_demo1;
    pal = W_GetName("TITLEPAL");
    IO_SetPalette(pal);
    pic = (pic_t*)W_GetName("TITLEPIC");
    V_DrawPic(0, 0, pic);
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

void D_DemoLoop(void)
{
    int episode;
    int skill;

    demoaction = da_filmwarp;
    episode = D_CheckParm("run");
    if (episode != 0)
    {
        episode = atoi(my_argv[episode + 1]);
        skill = sk_baby;
        G_StartNewGame(episode, skill, 0);
        demoaction = da_startgame;
        gamestart = gs_warp;
    }
    episode = D_CheckParm("play");
    if (episode != 0)
    {
        G_PlayDemo(my_argv[episode + 1]);
        G_GameLoop();
        IO_Quit();
    }
    episode = D_CheckParm("record");
    if (episode != 0)
    {
        if (my_argc <= episode + 2)
        {
            IO_Error("doom RECORD <level> <demoname>\n");
        }
        G_RecordDemo(my_argv[episode + 1], my_argv[episode + 2]);
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
        case 0:
            democycle = da_demo1;
            D_Title();
            break;
        case 1:
            democycle = da_demo3;
            D_Story();
            break;
        case 2:
            democycle = da_demo2;
            D_Credits();
            break;
        case 3:
            democycle = da_demo4;
            D_HighScores();
            break;
        case 4:
            democycle = da_credits;
            G_PlayDemo("DEMO1");
            break;
        case 5:
            democycle = da_story;
            G_PlayDemo("DEMO2");
            break;
        case 6:
            democycle = da_scores;
            G_PlayDemo("DEMO3");
            break;
        case 7:
            democycle = da_filmwarp;
            G_PlayDemo("DEMO4");
            break;
        case 8:
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
