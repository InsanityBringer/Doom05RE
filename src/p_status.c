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
#include "p_local.h"

int wnumber0;
int snumber0;
fixed_t amaporgy;
fixed_t amaporgx;
int cardnumber;
pic_t* healthbar;
pic_t* armorbar;
pic_t* timebar;
pic_t* blankbar;

void P_DrawWeapon(void)
{
    int iVar1;
    pic_t* pic;
    weapontype_t local_20;

    local_20 = playerobjs[viewplayer].readyweapon;
    iVar1 = W_GetNumForName("I_WBAYON");
    pic = (pic_t*)W_GetLump(iVar1 + local_20);
    V_DrawPic(4, 172, pic);
    return;
}

void P_DrawAmmo(void)
{
    int local_20;
    pic_t* pic;
    int num = 0;

    if (player->readyweapon < 8) 
    {
        switch (player->readyweapon) 
        {
        default:
            num = 0;
            break;
        case wp_rifle:
        case wp_auto:
            num = player->ammo[am_clip];
            break;
        case wp_shotgun:
            num = player->ammo[am_shell];
            break;
        case wp_missile:
            num = player->ammo[am_misl];
            break;
        case wp_claw:
            num = player->ammo[am_soul];
            break;
        case wp_bfg:
            num = player->ammo[am_cell];
            break;
        }
    }
    local_20 = num / 100;
    pic = (pic_t*)W_GetLump(snumber0 + local_20);
    V_DrawPic(56, 185, pic);
    num = num + local_20 * -100;
    local_20 = num / 10;
    pic = (pic_t*)W_GetLump(snumber0 + local_20);
    V_DrawPic(68, 185, pic);
    pic = (pic_t*)W_GetLump(snumber0 + num + local_20 * -10);
    V_DrawPic(80, 185, pic);
    return;
}

void P_DrawHealth(void)
{
    int iVar1;

    iVar1 = 0;
    while (iVar1 < player->health) 
    {
        V_DrawPic(iVar1 * 4 + 140, 171, healthbar);
        iVar1++;
    }
    while (iVar1 < 0xf) 
    {
        V_DrawPic(iVar1 * 4 + 140, 171, blankbar);
        iVar1++;
    }
    return;
}

void P_DrawArmor(void)
{
    int iVar1;

    iVar1 = 0;
    while (iVar1 < player->armor) 
    {
        V_DrawPic(iVar1 * 4 + 140, 181, armorbar);
        iVar1 = iVar1 + 1;
    }
    while (iVar1 < 0xf) 
    {
        V_DrawPic(iVar1 * 4 + 140, 181, blankbar);
        iVar1 = iVar1 + 1;
    }
    return;
}

void P_DrawTime(void)
{
    int iVar1;

    iVar1 = 0;
    while (iVar1 < player->itemtime)
    {
        V_DrawPic(iVar1 * 4 + 140, 191, timebar);
        iVar1 = iVar1 + 1;
    }
    while (iVar1 < 0xf) 
    {
        V_DrawPic(iVar1 * 4 + 140, 191, blankbar);
        iVar1 = iVar1 + 1;
    }
    return;
}

void P_DrawCards(void)
{
    pic_t* pic;
    int iVar1;

    iVar1 = 0;
    while (iVar1 < 3)
    {
        if (player->items[iVar1] != 0) 
        {
            pic = (pic_t*)W_GetLump(cardnumber + 1 + iVar1);
            V_DrawPic(204, iVar1 * 8 + 172, pic);
        }
        iVar1 = iVar1 + 1;
    }
    return;
}

void P_DrawPlayScreen(void)
{
    pic_t* pic;

    pic = (pic_t*)W_GetName("INFOLEFT");
    V_DrawPic(0, 168, pic);
    pic = (pic_t*)W_GetName("INFOR1");
    V_DrawPic(136, 168, pic);
    player = &playerobjs[sd->consoleplayer];
    P_DrawWeapon();
    P_DrawAmmo();
    P_DrawTime();
    P_DrawHealth();
    P_DrawArmor();
    P_DrawCards();
    return;
}

void P_GiveWeapon(weapontype_t weapon)
{
    player->weaponowned[weapon] = 1;
    return;
}

void P_GiveAmmo(ammotype_t ammo, int num)
{
    player->ammo[ammo] = player->ammo[ammo] + num;
    switch (ammo) 
    {
    case am_clip:
        if (MAXCLIP < player->ammo[ammo]) 
        {
            player->ammo[ammo] = MAXCLIP;
        }
        if (((player->readyweapon == wp_rifle) || (player->readyweapon == wp_auto)) && (&playerobjs[sd->consoleplayer] == player)) 
        {
            P_DrawAmmo();
        }
        break;
    case am_shell:
        if (MAXSHELL < player->ammo[ammo]) 
        {
            player->ammo[ammo] = MAXSHELL;
        }
        if ((player->readyweapon == wp_shotgun) && (&playerobjs[sd->consoleplayer] == player)) 
        {
            P_DrawAmmo();
        }
        break;
    case am_cell:
        if (MAXCELL < player->ammo[ammo]) 
        {
            player->ammo[ammo] = MAXCELL;
        }
        if ((player->readyweapon == wp_bfg) && (&playerobjs[sd->consoleplayer] == player)) 
        {
            P_DrawAmmo();
        }
        break;
    case am_soul:
        if (MAXSOUL < player->ammo[ammo])
        {
            player->ammo[ammo] = MAXSOUL;
        }
        if ((player->readyweapon == wp_claw) && (&playerobjs[sd->consoleplayer] == player)) 
        {
            P_DrawAmmo();
        }
        break;
    case am_misl:
        if (MAXMISL < player->ammo[ammo])
        {
            player->ammo[ammo] = MAXMISL;
        }
        if ((player->readyweapon == wp_missile) && (&playerobjs[sd->consoleplayer] == player))
        {
            P_DrawAmmo();
        }
    }
    return;
}

void P_GiveBody(int num)
{
    player->health += num;
    if (MAXBODY < player->health)
    {
        player->health = MAXBODY;
    }
    P_DrawHealth();
    return;
}

void P_GiveArmor(int num)
{
    player->armor = num;
    P_DrawArmor();
    return;
}

void P_GivePoints(int points)
{
#if 0 //[ISB]
    player->score += points;
#endif
    return;
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
            {
                goldshift = goldshift + 6;
            }
        }
    }
    return;
}

void P_GetThingAt(uint8_t* spot)
{
    actor_t* local_28;

    local_28 = actor.next;
    while ((((local_28 != &actor && (local_28->maporigin != spot)) &&
        (local_28->maporigin != spot - 1)) &&
        ((local_28->maporigin != spot - mapwidth &&
            (local_28->maporigin != (spot - mapwidth) - 1)))))
    {
        local_28 = local_28->next;
    }
    if (local_28 == &actor) 
    {
        IO_Error("P_GetThingAt: actor not located\n");
    }
    switch (local_28->r->sprite)
    {
    case SPR_SHOT:
        if ((player->weaponowned[wp_shotgun] != 0) && (player->ammo[am_shell] == MAXSHELL)) 
        {
            return;
        }
        P_GiveWeapon(wp_shotgun);
        P_GiveAmmo(am_shell, 4);
        break;
    case SPR_MGUN:
        if ((player->weaponowned[wp_auto] != 0) && (player->ammo[am_clip] == MAXCLIP)) 
        {
            return;
        }
        P_GiveWeapon(wp_auto);
        P_GiveAmmo(am_clip, 4);
        break;
    case SPR_LAUN:
        if ((player->weaponowned[wp_missile] != 0) && (player->ammo[am_misl] == MAXMISL)) 
        {
            return;
        }
        P_GiveWeapon(wp_missile);
        P_GiveAmmo(am_misl, 2);
        break;
    case SPR_CSAW:
        if (player->weaponowned[wp_chainsaw] != 0) 
        {
            return;
        }
        P_GiveWeapon(wp_chainsaw);
        break;
    case SPR_CLIP:
        if (player->ammo[am_clip] == MAXCLIP) 
        {
            return;
        }
        P_GiveAmmo(am_clip, 8);
        break;
    case SPR_SHEL:
        if (player->ammo[am_shell] == MAXSHELL)
        {
            return;
        }
        P_GiveAmmo(am_shell, 6);
        break;
    case SPR_MISL:
        if (player->ammo[am_misl] == MAXMISL)
        {
            return;
        }
        P_GiveAmmo(am_misl, 1);
        break;
    case SPR_STIM:
        if (player->health == 100) //[ISB] heh
        {
            return;
        }
        P_GiveBody(10);
        break;
    case SPR_MEDI:
        if (player->health == 100) 
        {
            return;
        }
        P_GiveBody(25);
        break;
    case SPR_SOUL:
        if (player->ammo[am_soul] == MAXSOUL) 
        {
            return;
        }
        P_GiveAmmo(am_soul, 8);
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
    case SPR_ARM1:
        if (6 < player->armor)
        {
            return;
        }
        P_GiveArmor(7);
        break;
    case SPR_ARM2:
        P_GiveArmor(15);
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
    case SPR_MMIS:
        if (player->ammo[am_misl] == MAXMISL) 
        {
            return;
        }
        P_GiveAmmo(am_misl, 4);
        break;
    case SPR_MBUL:
        if (player->ammo[am_clip] == MAXCLIP) 
        {
            return;
        }
        P_GiveAmmo(am_clip, 25);
        break;
    case SPR_MSHE:
        if (player->ammo[am_shell] == MAXSHELL) 
        {
            return;
        }
        P_GiveAmmo(am_shell, 20);
        break;
    case SPR_GKEY:
        P_GiveCard(it_bluecard);
        if (commpresent != 0)
        {
            return;
        }
        break;
    case SPR_SKEY:
        P_GiveCard(it_yellowcard);
        if (commpresent != 0)
        {
            return;
        }
        break;
    case SPR_BKEY:
        P_GiveCard(it_redcard);
        if (commpresent != 0)
        {
            return;
        }
        break;
    default:
        IO_Error("P_GetThing: Unknown gettable thing\n");
        break;
    }
    P_RemoveGetMarks(local_28->r);
    P_RemoveActor(local_28);
    if (playernum == sd->consoleplayer) 
    {
        goldshift += 6;
    }
    return;
}

void P_PlayerDied(int playernum)
{
    player_t* ppVar1;

    ppVar1 = &playerobjs[playernum];
    P_RemoveBlockMarks(playerobjs[playernum].r);
    ppVar1->health = 0;
    return;
}

void P_DamagePlayer(int player, int damage)
{
    if (player == sd->consoleplayer) 
    {
        redshift += damage * 4;
    }
    playerobjs[player].health -= damage;

    //[ISB] eh? No wonder you die when you get shot with 10 hp left. Heh. 
    if (playerobjs[player].health <= damage) 
    {
        P_PlayerDied(player);
    }
    if (player == sd->consoleplayer) 
    {
        P_DrawHealth();
    }
    return;
}

void P_AMapPlot(int x, int y, int color)
{
    if ((((-1 < x) && (x < 320)) && (-1 < y)) && (y < 200))
    {
        *(collumnpointer[x] + planewidthlookup[y]) = (uint8_t)color;
    }
    return;
}

void P_DrawPlayerMarker(player_t* player)
{
    int x;
    int local_20;

    x = player->r->x - amaporgx >> 0x14;
    local_20 = amaporgy - player->r->y >> 0x14;
    P_AMapPlot(x, local_20, 0xb4);
    switch ((int)(player->r->angle + 0x200U & 0x1fff) >> 10)
    {
    case 0:
        P_AMapPlot(x + 1, local_20, 0xb0);
        P_AMapPlot(x + -1, local_20, 0xb8);
        break;
    case 1:
        P_AMapPlot(x + 1, local_20 + -1, 0xb0);
        P_AMapPlot(x + -1, local_20 + 1, 0xb8);
        break;
    case 2:
        P_AMapPlot(x, local_20 + -1, 0xb0);
        P_AMapPlot(x, local_20 + 1, 0xb8);
        break;
    case 3:
        P_AMapPlot(x + -1, local_20 + -1, 0xb0);
        P_AMapPlot(x + 1, local_20 + 1, 0xb8);
        break;
    case 4:
        P_AMapPlot(x + -1, local_20, 0xb0);
        P_AMapPlot(x + 1, local_20, 0xb8);
        break;
    case 5:
        P_AMapPlot(x + -1, local_20 + 1, 0xb0);
        P_AMapPlot(x + 1, local_20 + -1, 0xb8);
        break;
    case 6:
        P_AMapPlot(x, local_20 + 1, 0xb0);
        P_AMapPlot(x, local_20 + -1, 0xb8);
        break;
    case 7:
        P_AMapPlot(x + 1, local_20 + 1, 0xb0);
        P_AMapPlot(x + -1, local_20 + -1, 0xb8);
    }
    return;
}

void P_DrawAMap(int x1, int y1, int x2, int y2, int mapx, int mapy)
{
    int local_24;
    int local_20;
    int local_1c;
    int local_18;
    uint8_t bVar1;

    local_24 = mapy - maporiginy >> 0x14;
    mapx = mapx - maporiginx;
    local_1c = y1;
    while (local_20 = mapx >> 0x14, local_18 = x1, local_1c <= y2) 
    {
        while (local_18 <= x2) 
        {
            if ((local_24 < mapheight) && (local_20 < mapwidth) && local_24 >= 0 && local_20 >= 0) 
            {
                bVar1 = amapcolor[blockmap[local_20 + local_24 * mapwidth]];
            }
            else
            {
                bVar1 = 0xcd;
            }
            *(collumnpointer[local_18] + planewidthlookup[local_1c]) = bVar1;
            local_18 = local_18 + 1;
            local_20 = local_20 + 1;
        }
        local_1c = local_1c + 1;
        local_24 = local_24 - 1;
    }
    V_MarkUpdateBlock(x1, y1, x2, y2);
    return;
}


void P_EnterAutoMap()
{
    int iVar1;
    player_t* player_00;

    if (automapup == 0) 
    {
        iVar1 = sd->consoleplayer;
        player_00 = &playerobjs[iVar1];
        amaporgx = (playerobjs[iVar1].r)->x + -0xa000000;
        amaporgy = (playerobjs[iVar1].r)->y + 0x5400000;
        P_DrawAMap(0, 0, 319, 167, amaporgx, amaporgy);
        P_DrawPlayerMarker(player_00);
        automapup = 1;
        ignorekeyboard++;
        P_DrawPlayScreen();
    }
    return;
}

void P_ExitAutoMap(void)
{
    automapup = 0;
    ignorekeyboard--;
    R_SetViewSize(config.viewsize, config.hdetail, 1);
    return;
}

void P_RunAutoMap(char ch)
{
    if (('\0' < ch) && (ch < '\x7f'))
    {
        P_ExitAutoMap();
    }
    return;
}
