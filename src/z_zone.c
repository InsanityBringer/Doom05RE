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

#include <string.h>

memzone_t* Z_AllocateZone(int size)
{
	int local_24;
	memzone_t* zone;

	zone = (memzone_t*)malloc(size + sizeof(memzone_t));
	if (zone == NULL)
	{
		IO_Error("Z_InitZone: Couldn't malloc %i bytes\n", size);
		return;
	}
	//[ISB] make sure zone is completely clean
	memset(zone, 0, size + sizeof(memzone_t));
	zone->size = size;
	zone->blocklist.size = 0;
	Z_ClearZone(zone);
	return zone;
}

void Z_ClearZone(memzone_t* zone)
{
	memblock_t* local_EAX_30;

	local_EAX_30 = (memblock_t*)(zone + 1);
	(zone->blocklist).prev = local_EAX_30;
	(zone->blocklist).next = (zone->blocklist).prev;
	(zone->blocklist).owner = (void*)0x1;
	zone->rover = local_EAX_30;
	*(memblock_t * *)& zone[1].blocklist.owner = &zone->blocklist;
	zone[1].blocklist.next = (memblock_t*)zone[1].blocklist.owner;
	zone[1].blocklist.size = 0;
	local_EAX_30->size = zone->size - sizeof(memzone_t);
	return;
}

void Z_Free(void* ptr)
{
	memblock_t* block = (memblock_t*)ptr - 1;
	memblock_t* prev;

	if (block->owner == NULL)
	{
		IO_Error("Z_Free: freed a freed pointer\n");
	}

	block->owner = NULL;
	prev = block->prev;

	//merge compatible blocks
	if (prev->owner == NULL)
	{
		prev->size += block->size;
		prev->next = block->next;
		block = prev;
	}

	prev = block->next;
	//this is how it is in the disassembly, probably a bug.
	//block is merged into prev in the final game.
	if (prev->owner == NULL)
	{
		prev->size += block->size;
		prev->prev = block->prev;
	}
}

void* Z_Malloc(memzone_t* zone, int size)

{
	int iVar1;
	int iVar2;
	memblock_t* rover;
	memblock_t* local_20;

	iVar1 = size + sizeof(memblock_t);
	rover = zone->rover;
	local_20 = rover;
	while ((local_20->owner != (void*)0x0 || (local_20->size < iVar1))) 
	{
		local_20 = local_20->next;
		if (local_20 == rover)
		{
			IO_Error("Z_Malloc: failed on allocation of %i bytes", iVar1);
		}
	}
	iVar2 = local_20->size - iVar1;
	if (0x40 < iVar2) 
	{
		rover = (memblock_t*)((int)& local_20[1].size + size);
		rover->size = iVar2;
		rover->owner = (void*)0x0;
		rover->prev = local_20;
		rover->next = local_20->next;
		local_20->next = rover;
		local_20->size = iVar1;
	}
	local_20->owner = (void*)0x2;
	zone->rover = local_20->next;
	return (uint8_t*)(local_20 + 1);
}


void* Z_CacheMalloc(memzone_t* zone, int size, void** user)
{
	int iVar1;
	memzone_t* local_2c;
	unsigned int local_28;
	memblock_t* local_20;
	memblock_t* local_18;

	local_28 = size + sizeof(memblock_t);
	local_2c = zone;
	if (zone->size - sizeof(memzone_t) < local_28) 
	{
		IO_Error("Z_CacheMalloc: %i bytes is greater than zone size\n", local_28);
	}
	local_20 = local_2c->rover;
	local_18 = local_20;
	if (local_2c->size - (int)((int)local_20 - (int)local_2c) < (int)local_28) 
	{
		local_20 = (local_2c->blocklist).next;
		local_18 = local_20;
	}
	for(;;)
	{
		if (local_20->owner != NULL) 
		{
			*local_20->owner = NULL;
		}
		if ((int)local_28 <= local_18->size) break;
		local_20 = local_20->next;
		if (&local_2c->blocklist == local_20) 
		{
			IO_Error("Z_CacheMalloc: wrapped zone\n");
		}
		local_18->size = local_18->size + local_20->size;
		local_18->next = local_20->next;
	}
	iVar1 = local_18->size - local_28;
	if (0x40 < iVar1)
	{
		local_20 = (memblock_t*)((int)& local_18->size + local_28);
		local_20->size = iVar1;
		local_20->owner = (void*)0x0;
		local_20->prev = local_18;
		local_20->next = local_18->next;
		local_18->next = local_20;
		local_18->size = local_28;
	}
	local_18->owner = user;
	local_2c->rover = local_18->next;

	//*(memblock_t * *)& ((memblock_t*)user)->size = local_18 + 1;
	*user = (local_18 + 1);
	return local_18 + 1;
}

void Z_CacheFree(void** user)
{
	//[BUG?] In the original source, *user is nulled before being Z_Free'd, which obviously isn't okay on modern operating systems.
	//I need to double check that I'm not missing something. 
	void** ptr = user;
	Z_Free(*user);
	*ptr = (void*)NULL;
	return;
}
