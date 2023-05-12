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
#include "r_ref.h"
#include "r_local.h"

//TODO: Portability
#include <malloc.h>

int sectornumber;
sector_t* sector;
int sectorxl, sectorxh;

subsector_t subsectors[204];
subsector_t* vissec;

int* floorpixel, *ceilingpixel;
int* newfloor, *newceiling;
int* passfloor, *passceiling;

int *esectorscalelight, *esectorscalelight2, *esectorscalelight3;

int xevents[400];
int* sortedxevents;
int* newxevent;

int *esrc1, *esrc2, *edest;
int size1, size2;

procline_t activehead;

void Merge(void)
{
    //In retrospect the goto may be intentional (carmack used plenty of gotos)
    //but this function could probably still be cleaned up further. 
    if (*esrc2 < *esrc1) 
        goto LAB_00028de7;
    do {
        do {
            *edest++ = *esrc1++;
            size1--;
            if (size1 == 0)
            {
                while (size2-- != 0)
                    *edest++ = *esrc2++;
                
                return;
            }
        } while (*esrc1 < *esrc2);
    LAB_00028de7:
        do {
            *edest++ = *esrc2++;
            size2--;
            if (size2 == 0) 
            {
                while (size1-- != 0)
                    *edest++ = *esrc1++;
                
                return;
            }
        } while (*esrc2 <= *esrc1);
    } while (1);
}

int* MergeSort(int* source, int* spare, int count)
{
    int size, start, sort;
    int* sorted, * unsorted, * temp;
    int* arrays[2];

    if (count < 2)
        return source;

    size = 1;
    sort = 0;

    arrays[sort] = source;
    arrays[sort == 0 ? 1 : 0] = spare;

    sorted = arrays[sort];
    unsorted = arrays[sort == 0 ? 1 : 0];

    do
    {
        start = 0;
        edest = unsorted;

        do
        {
            esrc1 = sorted + start;
            size1 = size;
            esrc2 = esrc1 + size;
            start += size;
            size2 = count - start;
            if (size2 > size)
                size2 = size;

            start += size2;
            Merge();
        } while ((count - start) > size);

        while (start != count)
        {
            *edest++ = arrays[sort][start];
            start++;
        }

        sort ^= 1;
        temp = sorted;
        sorted = unsorted;
        unsorted = temp;
        size <<= 1;
    } while (size < count);

    return sorted;
}

void R_AddActiveSeg(int procnum)
{
    procline_t* ppVar1;
    procline_t* local_20;

    if (newprocline <= procnum) 
    {
        IO_Error("R_AddActiveseg: bad procline num\n");
    }
    ppVar1 = &proclines[procnum];

    local_20 = activehead.next;
    while ((local_20 != &activehead && (proclines[procnum].scale + proclines[procnum].scalestep <= local_20->scale + local_20->scalestep))) 
    {
        local_20 = local_20->next;
    }
    ppVar1->prev = local_20->prev;
    ppVar1->prev->next = ppVar1;
    local_20->prev = ppVar1;
    proclines[procnum].next = local_20;
    return;
}

void R_ClearXEvents(void)
{
    newxevent = &xevents[0];
    activehead.prev = &activehead;
    activehead.next = &activehead;
    activehead.line = -1;
    return;
}

void R_AddLineXEvents(int linenum)
{
    int* piVar1;
    unsigned int local_28;
    procline_t* local_24;
    line_t* local_20;

    if (numlines <= linenum) 
    {
        IO_Error("R_TryAddLine: line >= numlines\n");
    }
    local_20 = &lines[linenum];
    if (local_20->validcheck != validcheck) 
    {
        R_MakeProcline(linenum);
    }
    if (local_20->procline != -1) 
    {
        local_28 = local_20->procline;
        if (newprocline <= local_28)
        {
            IO_Error("R_TryAddLine: procnum >= newprocline\n");
        }
        piVar1 = newxevent;
        local_24 = proclines + local_28;
        if (((sectorxl < proclines[local_28].ipx2) &&
            (proclines[local_28].ipx1 <= sectorxh)) &&
            ((proclines[local_28].sector == -1 ||
                ((proclines[local_28].sector != sectornumber &&
                    (proclines[local_28].chained == 0)))))) 
        {
            if (proclines[local_28].ipx1 < sectorxl) 
            {
                proclines[local_28].scale =
                    proclines[local_28].scale1 +
                    (sectorxl - proclines[local_28].ipx1) *
                    proclines[local_28].scalestep;
                R_AddActiveSeg(local_20->procline);
            }
            else 
            {
                newxevent++;
                *piVar1 = local_28 | 0x1000 | proclines[local_28].ipx1 << 0xd;
            }
            piVar1 = newxevent;
            if (local_24->ipx2 <= sectorxh) 
            {
                newxevent++;
                *piVar1 = local_24->ipx2 << 0xd | local_28;
            }
            if (&xevents[199] < newxevent)
            {
                IO_Error("R_AddLineXEvents: xevent overflow");
            }
        }
    }
    return;
}

void R_SortXEvents(void)
{
    int count;

    count = (newxevent - &xevents[0]);
    if (count == 0)
    {
        sortedxevents = &xevents[0];
    }
    else 
    {
        sortedxevents = MergeSort(xevents, xevents + 200, count);
    }
    sortedxevents[count] = (sectorxh + 1) * 0x2000;

    return;
}

void R_ProcessXEvents(void)
{
    int* piVar1;
    subsector_t* psVar2;
    int iVar3;
    unsigned int uVar4;
    int local_28;
    procline_t* local_24;

    local_28 = sectorxl;
    piVar1 = sortedxevents;
    sortedxevents++;
    uVar4 = *piVar1;
    local_24 = activehead.next;
    if ((int)uVar4 >> 0xd == sectorxl)
    {
        while (local_24 != &activehead) 
        {
            local_24->scale = local_24->scale1 + (local_28 - local_24->ipx1) * local_24->scalestep;
            local_24 = local_24->next;
        }
        do 
        {
            if (newprocline <= (int)(uVar4 & 0xfff))
            {
                IO_Error("R_ProcessXEvents: bad procline number\n");
            }
            proclines[uVar4 & 0xfff].scale = proclines[uVar4 & 0xfff].scale1;
            R_AddActiveSeg(uVar4 & 0xfff);
            piVar1 = sortedxevents;
            sortedxevents++;
            uVar4 = *piVar1;
        } while ((int)uVar4 >> 0xd == local_28);
    }
    local_24 = activehead.next;
    if (activehead.next == &activehead) 
    {
        IO_Error("R_ProcessXEvents: no activeseg at first collumn\n");
    }
    psVar2 = vissec;
    local_28 = local_24->line;
    vwalldrange.xl = sectorxl;
    vwalldrange.number = local_28;
    vissec->sectornum = local_24->sector;
    if (psVar2->sectornum != -1) 
    {
        vissec->xl = sectorxl;
        vissec->proclines = local_24;
        vissec->numproclines = 1;
    }
    while (iVar3 = (int)uVar4 >> 0xd, sectorxh + 1 != iVar3) 
    {
        while (((uVar4 & 0x1000) == 0 && ((int)uVar4 >> 0xd == iVar3))) 
        {
            uVar4 = uVar4 & 0xfff;
            (proclines[uVar4].next)->prev = proclines[uVar4].prev;
            (proclines[uVar4].prev)->next = proclines[uVar4].next;
            piVar1 = sortedxevents;
            sortedxevents++;
            uVar4 = *piVar1;
        }
        local_24 = activehead.next;
        if ((int)uVar4 >> 0xd == iVar3) 
        {
            while (local_24 != &activehead) 
            {
                local_24->scale = local_24->scale1 + (iVar3 - local_24->ipx1) * local_24->scalestep;
                local_24 = local_24->next;
            }
            do 
            {
                if (newprocline <= (int)(uVar4 & 0xfff)) 
                {
                    IO_Error("R_ProcessXEvents: bad procline number\n");
                }
                proclines[uVar4 & 0xfff].scale = proclines[uVar4 & 0xfff].scale1;
                R_AddActiveSeg(uVar4 & 0xfff);
                piVar1 = sortedxevents;
                sortedxevents++;
                uVar4 = *piVar1;
            } while ((int)uVar4 >> 0xd == iVar3);
        }
        local_24 = activehead.next;
        if (activehead.next == &activehead)
        {
            IO_Error("R_ProcessXEvents: no visible collumn\n");
        }
        if (local_24->line != vwalldrange.number) 
        {
            vwalldrange.xh = iVar3 + -1;
            R_DrawLineDrange();
            vwalldrange.number = local_24->line;
            vwalldrange.xl = iVar3;
            if (local_24->sector == -1) 
            {
                if (vissec->sectornum != -1) 
                {
                    vissec->xh = iVar3 + -1;
                    vissec = (subsector_t*)(&vissec[1].sectornum + vissec->numproclines - 1);

                    if ((subsector_t*)((int)&subsectors[0xcb].sectornum + 3U) < vissec)
                    {
                        IO_Error("R_ProcessXEvents: vissec overflow");
                    }
                    vissec->sectornum = -1;
                }
            }
            else 
            {
                if (vissec->sectornum != -1) 
                {
                    vissec->xh = iVar3 + -1;
                    vissec = (subsector_t*)(&vissec[1].sectornum + vissec->numproclines + -1);
                    if ((subsector_t*)((int)&subsectors[0xcb].sectornum + 3U) < vissec) 
                    {
                        IO_Error("R_ProcessXEvents: vissec overflow");
                    }
                }
                vissec->sectornum = local_24->sector;
                vissec->xl = iVar3;
                vissec->numproclines = 0;
                (&vissec->proclines)[vissec->numproclines] = local_24;
                vissec->numproclines = vissec->numproclines + 1;
            }
        }
    }
    vwalldrange.xh = iVar3 + -1;
    R_DrawLineDrange();
    if (vissec->sectornum != -1) 
    {
        vissec->xh = iVar3 + -1;
        vissec =
            (subsector_t*)(&vissec[1].sectornum + vissec->numproclines + -1);
        if ((subsector_t*)((int)&subsectors[0xcb].sectornum + 3U) < vissec) 
        {
            IO_Error("R_ProcessXEvents: vissec overflow");
        }
    }
    return;
}

void R_DrawAdjacentSectors(subsector_t* start)
{
    subsector_t* local_2c;
    int local_28;
    fixed_t* local_24;
    int* local_20;
    int* floorclip;

    local_2c = start;
    local_24 = outscale;
    local_20 = passceiling;
    floorclip = passfloor;
    while (local_2c < vissec) 
    {
        local_28 = 0;
        while (local_28 < local_2c->numproclines)
        {
            (&local_2c->proclines)[local_28]->chained = 1;
            local_28 = local_28 + 1;
        }
        R_DrawSector(local_2c->sectornum, local_2c->xl, local_2c->xh, floorclip, local_20, local_24);
        local_28 = 0;
        while (local_28 < local_2c->numproclines) 
        {
            (&local_2c->proclines)[local_28]->chained = 0;
            local_28 = local_28 + 1;
        }
        local_2c = (subsector_t*)(&local_2c[1].sectornum + local_2c->numproclines + -1);
    }
    return;
}

void R_DrawSector(int sectornum, int xl, int xh, int* floorclip, int* ceilingclip, int* scaleclip)
{
    int local_20;
    int* uVar1;
    int local_14;
    int local_2c;
    int local_24;
    int local_1c;
    int iVar3;

    subsector_t* oldvissec;

    if (numsectors <= sectornum) 
    {
        IO_Error("R_DrawSector: sectornum >= numsectors\n");
    }
    if (((xl < 0) || (viewwidth <= xh)) || (xh < xl))
    {
        IO_Error("R_DrawSector: bad bounds\n");
    }
    sectorxh = xh;
    sectorxl = xl;
    sectornumber = sectornum;
    sector = &sectors[sectornumber];
    local_14 = extralight + ((int)sector->lightlevel >> 4);
    local_1c = local_14 + -1;
    local_20 = local_14 + 1;
    if (0xf < local_14) 
    {
        local_14 = 0xf;
    }
    if (local_14 < 0)
    {
        local_14 = 0;
    }
    if (0xf < local_1c) 
    {
        local_1c = 0xf;
    }
    if (local_1c < 0) 
    {
        local_1c = 0;
    }
    if (0xf < local_20) 
    {
        local_20 = 0xf;
    }
    if (local_20 < 0) 
    {
        local_20 = 0;
    }
    esectorscalelight = &scalelight[local_14 * 0x30];
    esectorscalelight2 = &scalelight[local_1c * 0x30];
    esectorscalelight3 = &scalelight[local_20 * 0x30];
    local_14 = (sectorxh - sectorxl) + 1;
    local_1c = sectorxl;
    local_20 = sectorxh;
    uVar1 = alloca(local_14 * 5 * sizeof(int));

    floorpixel = floorclip;
    ceilingpixel = ceilingclip;
    inscale = scaleclip;

    newfloor = &uVar1[-local_1c];
    newceiling = newfloor + local_14;
    passfloor = newceiling + local_14;
    passceiling = passfloor + local_14;
    outscale = passceiling + local_14;

    R_ClearXEvents();
    iVar3 = 0;
    //floorpixel = local_2c;
    while (iVar3 < sector->linecount)
    {
        local_24 = sector->lines[iVar3];
        R_AddLineXEvents(local_24);
        iVar3++;
    }

    R_SortXEvents();
    oldvissec = vissec;
    R_ProcessXEvents();
    R_GenerateSpans();
    R_DrawPlanes();

    R_DrawAdjacentSectors(oldvissec);
    vissec = oldvissec;

    ceilingpixel = ceilingclip;
    inscale = scaleclip;
    floorpixel = floorclip;

    outscale = &uVar1[-local_1c] + local_14 * 4;

    R_DrawSectorThings(&sectors[sectornum], local_1c, local_20);
    return;
}