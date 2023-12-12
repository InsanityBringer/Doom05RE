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
#ifndef __I_LOCAL_H__
#define __I_LOCAL_H__

extern int ticcount;
#ifdef __WATCOMC__
extern byte* buffercrtc;
#else
extern int buffercrtc;
#endif

#ifndef __WATCOMC__
void IO_SetMapMask(byte mask);
void IO_SetStartAddress(int address);
void IO_WriteMunge(byte pixel, int offset);
void IO_WriteMungeDWord(unsigned int pixel, int offset);
#endif

void IO_SendFrame();

#endif
