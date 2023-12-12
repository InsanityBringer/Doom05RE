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

#ifndef __M_MENU_H__
#define __M_MENU_H__

//Dependent on the playsim just for framecmd_t
#include "p_local.h"


//menu
//[ISB] Does the menu code really own the hudfont, or is it owned by video and set by menu?
extern font_t* hudfont;
void M_Startup();
void M_DrawSelf();
void M_CheckInput(framecmd_t* cmd);
void M_ClearMenus();
void M_StartControlPanel();

#endif
