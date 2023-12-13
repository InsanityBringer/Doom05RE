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
	memzone_t* header;

	header = (memzone_t*)malloc(size + sizeof(memzone_t));
	if (header == NULL)
		IO_Error("Z_InitZone: Couldn't malloc %i bytes\n", size);

	//[ISB] make sure zone is completely clean
	memset(header, 0, size + sizeof(memzone_t));
	header->size = size;
	Z_ClearZone(header);

	return header;
}

void Z_ClearZone(memzone_t* zone)
{
	memblock_t* block;

	block = (memblock_t*)(zone + 1);
	(zone->blocklist).prev = block;
	(zone->blocklist).next = (zone->blocklist).prev;
	(zone->blocklist).owner = (void*)0x1;

	zone->rover = block;
	*(memblock_t**)&zone[1].blocklist.owner = &zone->blocklist;
	zone[1].blocklist.next = (memblock_t*)zone[1].blocklist.owner;
	zone[1].blocklist.size = 0;
	block->size = zone->size - sizeof(memzone_t);
}

void Z_Free(void* ptr)
{
	memblock_t* block = (memblock_t*)ptr - 1;
	memblock_t* other;

	if (block->owner == NULL)
		IO_Error("Z_Free: freed a freed pointer\n");
	
	block->owner = NULL;

	other = block->prev;
	if (other->owner == NULL)
	{	// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		block = other;
	}

	other = block->next;
	if (other->owner == NULL)
	{	// merge the next free block onto the end
		other->size += block->size;
		other->prev = block->prev;
	}
}

#define MINFRAGMENT 64

void* Z_Malloc(memzone_t* zone, int size)
{
	int extra;
	memblock_t* start, *rover, *new;

	size += sizeof(memblock_t);
	start = zone->rover;
	rover = start;
	while ((rover->owner != NULL || (rover->size < size)))
	{
		rover = rover->next;
		if (rover == start)
			IO_Error("Z_Malloc: failed on allocation of %i bytes", size);
	}

	extra = rover->size - size;
	if (extra > MINFRAGMENT)
	{
		new = (memblock_t*)((byte*)rover + size);
		new->size = extra;
		new->owner = NULL;
		new->prev = rover;
		new->next = rover->next;
		rover->next = new;
		rover->size = size;
	}
	rover->owner = (void*)2;
	zone->rover = rover->next;
	return (byte*)(rover + 1);
}

void Z_CacheMalloc(memzone_t* zone, int size, void** user)
{
	int extra;
	memblock_t* rover, * new, * kill;

	size += sizeof(memblock_t);
	if (zone->size - sizeof(memzone_t) < size)
		IO_Error("Z_CacheMalloc: %i bytes is greater than zone size", size);

	rover = zone->rover;
	extra = zone->size - ((byte*)rover - (byte*)zone);

	if (extra < size)
		rover = zone->blocklist.next;
	
	kill = rover;
	while (1)
	{
		if (kill->owner)
			*kill->owner = NULL;
		
		if (rover->size >= size)
			break;

		kill = kill->next;
		if (&zone->blocklist == kill)
			IO_Error("Z_CacheMalloc: wrapped zone");

		rover->size += kill->size;
		rover->next = kill->next;
	}

	extra = rover->size - size;
	if (extra > MINFRAGMENT)
	{
		new = (memblock_t*)((byte*)rover + size);
		new->size += extra;
		new->owner = NULL;
		new->prev = rover;
		new->next = rover->next;
		rover->next = new;
		rover->size = size;
	}

	rover->owner = user;
	zone->rover = rover->next;
	*user = (rover + 1);
}

void Z_CacheFree(void** user)
{
	//[BUG?] In the original source, *user is nulled before being Z_Free'd, which obviously isn't right. This seems to be correct to the alpha. 
#ifdef __WATCOMC__
	*user = (void*)NULL;
	Z_Free(*user);
#else
	void** ptr = user;
	Z_Free(*user);
	*ptr = (void*)NULL;
#endif
}
