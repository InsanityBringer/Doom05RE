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
#include "p_actor.h"
#include "r_local.h"

int snumber0;
int wnumber0;
fixed_t amaporgy;
fixed_t amaporgx;
int cardnumber;
pic_t* healthbar;
pic_t* armorbar;
pic_t* timebar;
pic_t* blankbar;

void P_DrawWeapon(void)
{
    int num;
    weapontype_t wep;

    wep = playerobjs[viewplayer].readyweapon;
    num = W_GetNumForName("I_WBAYON");
    V_DrawPic(4, 172, (pic_t*)W_GetLump(num + wep));
}

void P_DrawAmmo(void)
{
    int num;
    int ammo = 0;

    if (player->readyweapon < 8)
    {
        switch (player->readyweapon)
        {
        default:
            ammo = 0;
            break;
        case wp_rifle:
        case wp_auto:
            ammo = player->ammo[am_clip];
            break;
        case wp_shotgun:
            ammo = player->ammo[am_shell];
            break;
        case wp_missile:
            ammo = player->ammo[am_misl];
            break;
        case wp_claw:
            ammo = player->ammo[am_soul];
            break;
        case wp_bfg:
            ammo = player->ammo[am_cell];
            break;
        }
    }
    num = ammo / 100;
    V_DrawPic(56, 185, (pic_t*)W_GetLump(wnumber0 + num));
    ammo -= num * 100;
    num = ammo / 10;
    V_DrawPic(68, 185, (pic_t*)W_GetLump(wnumber0 + num));
    ammo -= num * 10;
    V_DrawPic(80, 185, (pic_t*)W_GetLump(wnumber0 + ammo));
}

void P_DrawHealth(void)
{
    int i;

    for (i = 0; i < player->health; i++)
        V_DrawPic(i * 4 + 140, 171, healthbar);

    for (; i < MAXBODY; i++)
        V_DrawPic(i * 4 + 140, 171, blankbar);
}

void P_DrawArmor(void)
{
    int i;

    for (i = 0; i < player->armor; i++)
        V_DrawPic(i * 4 + 140, 181, armorbar);

    for (; i < MAXARMOR; i++)
        V_DrawPic(i * 4 + 140, 181, blankbar);
}

void P_DrawTime(void)
{
    int i;

    for (i = 0; i < player->itemtime; i++)
        V_DrawPic(i * 4 + 140, 191, timebar);

    for (; i < 15; i++)
        V_DrawPic(i * 4 + 140, 191, blankbar);
}

void P_DrawCards(void)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        if (player->items[i] != 0)
            V_DrawPic(204, i * 8 + 172, (pic_t*)W_GetLump(cardnumber + 1 + i));
    }
}

void P_DrawPlayScreen(void)
{
    V_DrawPic(0, 168, (pic_t*)W_GetName("INFOLEFT"));
    V_DrawPic(136, 168, (pic_t*)W_GetName("INFOR1"));

    player = &playerobjs[sd->consoleplayer];
    P_DrawWeapon();
    P_DrawAmmo();
    P_DrawTime();
    P_DrawHealth();
    P_DrawArmor();
    P_DrawCards();
}

void P_GiveWeapon(weapontype_t weapon)
{
    player->weaponowned[weapon] = 1;
}

void P_GiveAmmo(ammotype_t ammo, int num)
{
    player->ammo[ammo] += num;
    switch (ammo)
    {
    case am_clip:
        if (player->ammo[ammo] > MAXCLIP)
            player->ammo[ammo] = MAXCLIP;

        if (((player->readyweapon == wp_rifle) || (player->readyweapon == wp_auto)) && (&playerobjs[sd->consoleplayer] == player))
            P_DrawAmmo();

        break;
    case am_shell:
        if (player->ammo[ammo] > MAXSHELL)
            player->ammo[ammo] = MAXSHELL;

        if ((player->readyweapon == wp_shotgun) && (&playerobjs[sd->consoleplayer] == player))
            P_DrawAmmo();

        break;
    case am_cell:
        if (player->ammo[ammo] > MAXCELL)
            player->ammo[ammo] = MAXCELL;

        if ((player->readyweapon == wp_bfg) && (&playerobjs[sd->consoleplayer] == player))
            P_DrawAmmo();

        break;
    case am_soul:
        if (player->ammo[ammo] > MAXSOUL)
            player->ammo[ammo] = MAXSOUL;

        if ((player->readyweapon == wp_claw) && (&playerobjs[sd->consoleplayer] == player))
            P_DrawAmmo();

        break;
    case am_misl:
        if (player->ammo[ammo] > MAXMISL)
            player->ammo[ammo] = MAXMISL;

        if ((player->readyweapon == wp_missile) && (&playerobjs[sd->consoleplayer] == player))
            P_DrawAmmo();

        break;
    }
}

void P_GiveBody(int num)
{
    player->health += num;
    if (player->health > MAXBODY)
        player->health = MAXBODY;

    P_DrawHealth();
}

void P_GiveArmor(int num)
{
    player->armor = num;
    P_DrawArmor();
}

void P_GivePoints(int points)
{
#if 0 //[ISB]
    player->score += points;
#endif
}

void P_GiveCard(item_t item)
{
    if ((player->items[item] == 0))
    {
        player->items[item] = 1;
        if (playernum == sd->consoleplayer)
        {
            P_DrawCards();
            if (commpresent != 0)
                goldshift += 6;

        }
    }
}

void P_GetThingAt(byte* spot)
{
    byte* spot2 = spot - 1;
    byte* spot3 = spot - mapwidth;
    byte* spot4 = spot - mapwidth - 1;
    actor_t* check = actorcap.next;

    for (; check != &actorcap; check = check->next)
    {
        if (check->maporigin == spot || check->maporigin == spot2 || check->maporigin == spot3 || check->maporigin == spot4)
            break;
    }

    if (check == &actorcap)
        IO_Error("P_GetThingAt: actor not located\n");

    switch (check->r->sprite)
    {
    case SPR_SHOT:
        if ((player->weaponowned[wp_shotgun] != 0) && (player->ammo[am_shell] == MAXSHELL))
            return;

        P_GiveWeapon(wp_shotgun);
        P_GiveAmmo(am_shell, 4);
        break;
    case SPR_MGUN:
        if ((player->weaponowned[wp_auto] != 0) && (player->ammo[am_clip] == MAXCLIP))
            return;

        P_GiveWeapon(wp_auto);
        P_GiveAmmo(am_clip, 4);
        break;
    case SPR_LAUN:
        if ((player->weaponowned[wp_missile] != 0) && (player->ammo[am_misl] == MAXMISL))
            return;

        P_GiveWeapon(wp_missile);
        P_GiveAmmo(am_misl, 2);
        break;
    case SPR_CSAW:
        if (player->weaponowned[wp_chainsaw] != 0)
            return;

        P_GiveWeapon(wp_chainsaw);
        break;
    case SPR_CLIP:
        if (player->ammo[am_clip] == MAXCLIP)
            return;

        P_GiveAmmo(am_clip, 8);
        break;
    case SPR_MBUL:
        if (player->ammo[am_clip] == MAXCLIP)
            return;

        P_GiveAmmo(am_clip, 25);
        break;
    case SPR_SHEL:
        if (player->ammo[am_shell] == MAXSHELL)
            return;

        P_GiveAmmo(am_shell, 6);
        break;
    case SPR_MSHE:
        if (player->ammo[am_shell] == MAXSHELL)
            return;

        P_GiveAmmo(am_shell, 20);
        break;
    case SPR_MISL:
        if (player->ammo[am_misl] == MAXMISL)
            return;

        P_GiveAmmo(am_misl, 1);
        break;
    case SPR_MMIS:
        if (player->ammo[am_misl] == MAXMISL)
            return;

        P_GiveAmmo(am_misl, 4);
        break;
    case SPR_SOUL:
        if (player->ammo[am_soul] == MAXSOUL)
            return;

        P_GiveAmmo(am_soul, 8);
        break;
    case SPR_STIM:
        if (player->health == 100) //[ISB] heh
            return;

        P_GiveBody(10);
        break;
    case SPR_MEDI:
        if (player->health == 100)
            return;

        P_GiveBody(25);
        break;
    case SPR_GKEY:
        P_GiveCard(it_bluecard);
        if (commpresent != 0)
            return;
        break;
    case SPR_SKEY:
        P_GiveCard(it_yellowcard);
        if (commpresent != 0)
            return;
        break;
    case SPR_BKEY:
        P_GiveCard(it_redcard);
        if (commpresent != 0)
            return;
        break;
    case SPR_ARM1:
        if (player->armor > 6)
            return;

        P_GiveArmor(7);
        break;
    case SPR_ARM2:
        P_GiveArmor(15);
        break;
    case SPR_BON1:
        P_GivePoints(100);
        break;
    case SPR_BON2:
        P_GivePoints(500);
        break;
    case SPR_BON3:
        P_GivePoints(1000);
        break;
    case SPR_BON4:
        P_GivePoints(10000);
        break;
    case SPR_POW1:
        break;
    case SPR_POW2:
        break;
    case SPR_POW3:
        break;
    case SPR_POW4:
        break;
    case SPR_POW5:
        break;
    case SPR_IGOG:
        break;
    default:
        IO_Error("P_GetThing: Unknown gettable thing\n");
        break;
    }
    P_RemoveGetMarks(check->r);
    P_RemoveActor(check);

    if (playernum == sd->consoleplayer)
        goldshift += 6;
}

void P_PlayerDied(int playernum)
{
    player_t* player = &playerobjs[playernum];
    P_RemoveBlockMarks(player->r);
    player->health = 0;
}

void P_DamagePlayer(int playernum, int damage)
{
    player_t* player;
    if (playernum == sd->consoleplayer)
        redshift += damage * 4;

    player = &playerobjs[playernum];
    player->health -= damage;

    //[ISB] eh? No wonder you die when you get shot with 10 hp left. Heh. 
    if (player->health <= damage)
        P_PlayerDied(playernum);

    if (playernum == sd->consoleplayer)
        P_DrawHealth();
}

void P_AMapPlot(int x, int y, int color)
{
    if (x >= 0 && x < SCREENWIDTH && y >= 0 && y < SCREENHEIGHT - SBARHEIGHT)
        collumnpointer[x][planewidthlookup[y]] = color;
}

void P_DrawPlayerMarker(player_t* player)
{
    int x = player->r->x - amaporgx >> MAPBLOCKSHIFT;
    int y = amaporgy - player->r->y >> MAPBLOCKSHIFT;
    P_AMapPlot(x, y, 0xb4);
    switch ((player->r->angle + (NUMANGLES/16) & ANGLEMASK) >> 10)
    {
    case 0:
        P_AMapPlot(x + 1, y, 0xb0);
        P_AMapPlot(x + -1, y, 0xb8);
        break;
    case 1:
        P_AMapPlot(x + 1, y + -1, 0xb0);
        P_AMapPlot(x + -1, y + 1, 0xb8);
        break;
    case 2:
        P_AMapPlot(x, y + -1, 0xb0);
        P_AMapPlot(x, y + 1, 0xb8);
        break;
    case 3:
        P_AMapPlot(x + -1, y + -1, 0xb0);
        P_AMapPlot(x + 1, y + 1, 0xb8);
        break;
    case 4:
        P_AMapPlot(x + -1, y, 0xb0);
        P_AMapPlot(x + 1, y, 0xb8);
        break;
    case 5:
        P_AMapPlot(x + -1, y + 1, 0xb0);
        P_AMapPlot(x + 1, y + -1, 0xb8);
        break;
    case 6:
        P_AMapPlot(x, y + 1, 0xb0);
        P_AMapPlot(x, y + -1, 0xb8);
        break;
    case 7:
        P_AMapPlot(x + 1, y + 1, 0xb0);
        P_AMapPlot(x + -1, y + -1, 0xb8);
    }
}

void P_DrawAMap(int x1, int y1, int x2, int y2, int mapx, int mapy)
{
    byte pixel;
    int source;
    int x;
    int y;
    int mx;
    int my;
    int mx1;

    my = mapy - maporiginy >> MAPBLOCKSHIFT;
    mx1 = mapx - maporiginx >> MAPBLOCKSHIFT;

    for (y = y1; y <= y2; y++, my--)
    {
        mx = mx1;

        for (x = x1; x <= x2; x++, mx++)
        {
            if ((my < mapheight) && (mx < mapwidth) && my >= 0 && mx >= 0)
            {
                source = my * mapwidth + mx;
                pixel = amapcolor[blockmap[source]];
            }

            else
                pixel = 0xcd;

            collumnpointer[x][planewidthlookup[y]] = pixel;
        }
    }
    V_MarkUpdateBlock(x1, y1, x2, y2);
}


void P_EnterAutoMap()
{
    if (!automapup)
    {
        player_t* player = &playerobjs[sd->consoleplayer];
        amaporgx = player->r->x - ((SCREENWIDTH / 2 * FRACUNIT) << 4);
        amaporgy = player->r->y + (((SCREENHEIGHT - SBARHEIGHT) / 2 * FRACUNIT) << 4);
        P_DrawAMap(0, 0, SCREENWIDTH - 1, SCREENHEIGHT - SBARHEIGHT - 1, amaporgx, amaporgy);
        P_DrawPlayerMarker(player);
        automapup = true;
        ignorekeyboard++;
        P_DrawPlayScreen();
    }
}

void P_ExitAutoMap(void)
{
    automapup = false;
    ignorekeyboard--;
    R_SetViewSize(config.viewsize, config.hdetail, 1);
}

void P_RunAutoMap(byte ch)
{
    if (ch > 0 && ch < 127)
        P_ExitAutoMap();
}
