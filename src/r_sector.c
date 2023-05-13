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

byte subsectors[SUBSECTORBUFFERSIZE];
subsector_t* vissec;

int* floorpixel, *ceilingpixel;
int* newfloor, *newceiling;
int* passfloor, *passceiling;

int *esectorscalelight, *esectorscalelight2, *esectorscalelight3;

#define MAXXEVENTS 200
#define XEVENTPROCNUMBITS 12
#define XEVENTPROCNUMMASK ((1 << XEVENTPROCNUMBITS) - 1)
#define STARTSEGEVENT (1 << XEVENTPROCNUMBITS)
#define ENDEVENT 8192
#define XVALUESHIFT 13

int xevents[MAXXEVENTS * 2];
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
    procline_t* proc, * scan;
    fixed_t pscale;

    if (procnum >= newprocline)
        IO_Error("R_AddActiveseg: bad procline number");

    proc = &proclines[procnum];

    pscale = proc->scale + proc->scalestep;

    for (scan = activehead.next; (scan != &activehead
        && (pscale <= (scan->scale + proc->scalestep)));
        scan = scan->next) {
    }

    proc->prev = scan->prev;
    proc->prev->next = proc;
    scan->prev = proc;
    proclines[procnum].next = scan;
}

void R_ClearXEvents(void)
{
    newxevent = &xevents[0];
    activehead.prev = &activehead;
    activehead.next = &activehead;
    activehead.line = -1;
}

void R_AddLineXEvents(int linenum)
{
    int procnum;
    line_t* line;
    procline_t* proc;

    if (numlines <= linenum)
        IO_Error("R_TryAddLine: line >= numlines");

    line = &lines[linenum];
    if (line->validcheck != validcheck)
        R_MakeProcline(linenum);

    if (line->procline == -1)
        return;

    procnum = line->procline;
    if (procnum >= newprocline)
        IO_Error("R_TryAddLine: procnum >= newprocline");

    proc = &proclines[procnum];

    if (proc->ipx2 <= sectorxl)
        return;

    if (proc->ipx1 > sectorxh)
        return;

    if (proc->sector != -1)
    {
        if (proc->sector == sectornumber)
            return;

        if (proc->chained)
            return;
    }

    if (proc->ipx1 < sectorxl)
    {
        proc->scale =
            proc->scale1 +
            (sectorxl - proc->ipx1) *
            proc->scalestep;

        R_AddActiveSeg(line->procline);
    }
    else
        *newxevent++ = procnum | STARTSEGEVENT | proc->ipx1 << XVALUESHIFT;

    if (proc->ipx2 <= sectorxh)
        *newxevent++ = procnum | proc->ipx2 << XVALUESHIFT;

    if (newxevent > &xevents[MAXXEVENTS - 1])
        IO_Error("R_AddLineXEvents: xevent overflow");
}


void R_SortXEvents(void)
{
    int numxevents;

    numxevents = (newxevent - &xevents[0]);
    if (numxevents == 0)
        sortedxevents = &xevents[0];
    else 
        sortedxevents = MergeSort(xevents, xevents + MAXXEVENTS, numxevents);
    
    sortedxevents[numxevents] = (sectorxh + 1) * ENDEVENT;
}

void R_ProcessXEvents(void)
{
    unsigned int event;
    int procnum;
    procline_t* proc;
    int x;
    int line;

    event = *sortedxevents++;
    x = sectorxl;

    //Find the active seg at the leftmost bounds
    if ((event >> XVALUESHIFT) == x)
    {
        for (proc = activehead.next; proc != &activehead; proc = proc->next)
            proc->scale = proc->scale1 + (x - proc->ipx1) * proc->scalestep;

        do
        {
            if (event == 0 && false) //no idea
                IO_Error("R_ProcessXEvents: remove event at first pixel");

            if ((event & XEVENTPROCNUMMASK) >= newprocline)
                IO_Error("R_ProcessXEvents: bad procline number");

            proclines[event & XEVENTPROCNUMMASK].scale =
                proclines[event & XEVENTPROCNUMMASK].scale1;
            R_AddActiveSeg(event & XEVENTPROCNUMMASK);
            event = *sortedxevents++;
        } while (event >> XVALUESHIFT == x);
    }

    proc = activehead.next;
    if (proc == &activehead)
        IO_Error("R_ProcessXEvents: no activeseg at first collumn");

    line = vwalldrange.number = proc->line;
    vwalldrange.xl = sectorxl;
    vissec->sectornum = proc->sector;
    if (vissec->sectornum != -1)
    {
        vissec->xl = sectorxl;
        vissec->proclines[0] = proc;
        vissec->numproclines = 1;
    }

    while (1)
    {
        x = event >> XVALUESHIFT;
        //Reached the right clipping bound of the current sector window, so draw the last seg. 
        if (sectorxh + 1 == x)
        {
            vwalldrange.xh = x - 1;
            R_DrawLineDrange();

            if (vissec->sectornum != -1)
            {
                vissec->xh = x - 1;
                vissec = (subsector_t*)(((byte*)vissec) + (vissec->numproclines - 1) * sizeof(procline_t*) + sizeof(subsector_t));
                if (vissec >= &subsectors[sizeof(subsectors) - sizeof(subsector_t) - 32])
                    IO_Error("R_ProcessXEvents: vissec overflow");
            }

            return;
        }

        //Handle remove events for this X coordinate
        while ((event & STARTSEGEVENT) == 0 && (event >> XVALUESHIFT) == x)
        {
            procnum = event & XEVENTPROCNUMMASK;
            proc = &proclines[procnum];
            proc->next->prev = proc->prev;
            proc->prev->next = proc->next;
            event = *sortedxevents++;
        }

        //Now handle add events
        if ((event >> XVALUESHIFT) == x)
        {
            for (proc = activehead.next; proc != &activehead; proc = proc->next)
                proc->scale = proc->scale1 + (x - proc->ipx1) * proc->scalestep;

            do
            {
                if ((event & XEVENTPROCNUMMASK) >= newprocline)
                    IO_Error("R_ProcessXEvents: bad procline number");

                proc = &proclines[event & XEVENTPROCNUMMASK];
                proc->scale = proc->scale1;
                R_AddActiveSeg(event & XEVENTPROCNUMMASK);
                event = *sortedxevents++;
            } while ((event >> XVALUESHIFT) == x);
        }

        proc = activehead.next;
        if (proc == &activehead)
            IO_Error("R_ProcessXEvents: no visible collumn");

        //Draw any active segs that were added
        if (proc->line != vwalldrange.number)
        {
            vwalldrange.xh = x - 1;
            R_DrawLineDrange();
            vwalldrange.number = proc->line;
            vwalldrange.xl = x;

            if (proc->sector == -1)
            {
                if (vissec->sectornum != -1)
                {
                    vissec->xh = x - 1;
                    vissec = (subsector_t*)(((byte*)vissec) + (vissec->numproclines - 1) * sizeof(procline_t*) + sizeof(subsector_t));
                    if (vissec >= &subsectors[sizeof(subsectors) - sizeof(subsector_t) - 32])
                        IO_Error("R_ProcessXEvents: vissec overflow");

                    vissec->sectornum = -1;
                }
            }
            else
            {
                if (vissec->sectornum != -1)
                {
                    vissec->xh = x - 1;
                    vissec = (subsector_t*)(((byte*)vissec) + (vissec->numproclines - 1) * sizeof(procline_t*) + sizeof(subsector_t));
                    if (vissec >= &subsectors[sizeof(subsectors) - sizeof(subsector_t) - 32])
                        IO_Error("R_ProcessXEvents: vissec overflow");
                }

                vissec->sectornum = proc->sector;
                vissec->xl = x;
                vissec->numproclines = 0;
                vissec->proclines[vissec->numproclines] = proc;
                vissec->numproclines++;
            }
        }
    }
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
            local_2c->proclines[local_28]->chained = 1;
            local_28 = local_28 + 1;
        }
        R_DrawSector(local_2c->sectornum, local_2c->xl, local_2c->xh, floorclip, local_20, local_24);
        local_28 = 0;
        while (local_28 < local_2c->numproclines) 
        {
            local_2c->proclines[local_28]->chained = 0;
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