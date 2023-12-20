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
#include "p_local.h"
#include "m_menu.h"
#include "g_game.h"

// 
// MENU TYPEDEFS 
// 
typedef struct
{
	short	status;					// 0 = no cursor here, 1 = ok,2 = arrows ok 
	char	name[10];
	void	(*routine)(int choice);	// choice = menu item #. if status = 2, 
									// ...choice=0:leftarrow,1:rightarrow 
} menuitem_t;

typedef struct menu_s
{
	short			numitems;			// # of menu items 
	struct menu_s*	prevMenu;			// previous menu 
	menuitem_t*		items;				// menu items 
	void			(*routine)(void);	// draw routine 
	short			x, y;				// x,y of menu 
	short			lastOn;				// last item user was on in menu 
	short			lastScrollIndex;
	short			scrollsize;
} menu_t;

//prototypes
void M_DrawMainMenu();
void M_NewGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_QuitDOOM(int choice);

void M_DrawNewGame();
void M_StartGame(int choice);

void M_DrawOptions();
void M_Controls(int choice);
void M_Display(int choice);
void M_ChangeVolume(int choice);

void M_DrawControls();
void M_ChangeJoyType(int choice);
void M_ChangeSensitivity(int choice);

void M_DrawDisplay(void);
void M_ChangeDisplay(int choice);
void M_SizeDisplay(int choice);

void M_DrawThermo(int x, int y, int thermWidth, int thermDot);
void M_DrawEmptyCell(menu_t* menu, int item);
void M_DrawSelCell(menu_t* menu, int item);

void M_SetupNextMenu(menu_t* menudef);
void M_ClearMenus(void);

void M_StartControlPanel(void);

char inputstring[80];
int skullAnimCounter;
int menuDisplayed = 0;

#define         SKULLXOFF       -32 
#define LINEHEIGHT      16 

int scrollIndex;
int whichSkull;
int itemOn;
int validwidth;
int maxwidth;

int joystickType;
char joyTypeNames[2][10] = { "M_DIGIT", "M_ANALOG" };

int mouseSensitivity = 5;
int soundVolume = 8;

//============================================= 
// 
// DOOM MENU 
// 
//============================================= 
typedef enum
{
	newgame = 0,
	options,
	endgame,
	readthis,
	quitdoom,
	main_end
} main_e;

menuitem_t MainMenu[5] =
{
	{1, "M_NGAME", &M_NewGame},
	{1, "M_OPTION", &M_Options},
	{1, "M_ENDGAM", &M_EndGame},
	{1, "M_RDTHIS", &M_ReadThis},
	{1, "M_QUITG", &M_QuitDOOM}
};

menu_t MainDef =
{
	5,
	NULL,
	&MainMenu,
	&M_DrawMainMenu,
	97,
	72,
	0,
	0,
	0
};

//============================================= 
// 
// NEW GAME 
// 
//============================================= 
typedef enum
{
	killthings = 0,
	toorough,
	hurtme,
	violence,
	newg_end
} newgame_e;

menuitem_t NewGameMenu[4] =
{
	{1, "M_JKILL", &M_StartGame},
	{1, "M_ROUGH", &M_StartGame},
	{1, "M_HURT", &M_StartGame},
	{1, "M_ULTRA", &M_StartGame}
};

menu_t NewDef =
{
	4,
	&MainDef,
	&NewGameMenu,
	&M_DrawNewGame,
	48,
	63,
	2,
	0,
	0
};

//============================================= 
// 
// OPTIONS MENU 
// 
//============================================= 
typedef enum
{
	loadgame = 0,
	savegame,
	controls,
	display,
	soundvol,
	opt_end
} options_e;

menuitem_t OptionsMenu[5] =
{
	{1, "M_LOADG", NULL},
	{0, "M_SAVEG", NULL},
	{1, "M_CONTR", &M_Controls},
	{1, "M_DISP", &M_Display},
	{2, "M_SVOL", &M_ChangeVolume}
};

menu_t OptionsDef =
{
	5,
	&MainDef,
	&OptionsMenu,
	&M_DrawOptions,
	80,
	40,
	0,
	0,
	0
};

//============================================= 
// 
// CONTROLS MENU 
// 
//============================================= 
typedef enum
{
	mouse = 0,
	joy1,
	joy2,
	gamepad,
	customize,
	joytype,
	mousesens,
	control_end
} controls_e;

menuitem_t ControlsMenu[7] =
{
	{1, "M_MOUSE", NULL},
	{1, "M_JOY1", NULL},
	{1, "M_JOY2", NULL},
	{1, "M_GAMEPD", NULL},
	{1, "M_CUSCON", NULL},
	{1, "M_JTYPE", &M_ChangeJoyType},
	{2, "M_MSENS", &M_ChangeSensitivity}
};

menu_t ControlsDef =
{
	7,
	&OptionsDef,
	&ControlsMenu,
	&M_DrawControls,
	50,
	34,
	4,
	0,
	0
};

//============================================= 
// 
// DISPLAY MENU 
// 
//============================================= 
typedef enum
{
	high = 0,
	normal,
	low,
	hires,
	hicolor,
	emptyl,
	scrnsize,
	disp_end
} display_e;

menuitem_t DisplayMenu[8] =
{
	{0, "M_DETAIL", NULL},
	{1, "M_GDHIGH", &M_ChangeDisplay},
	{1, "M_GDNORM", &M_ChangeDisplay},
	{1, "M_GDLOW", &M_ChangeDisplay},
	{1, "M_HCDAC", &M_ChangeDisplay},
	{1, "M_HIRES", &M_ChangeDisplay},
	{0, "", NULL},
	{2, "M_SCRNSZ", &M_SizeDisplay}
};

menu_t DisplayDef =
{
	8,
	&OptionsDef,
	&DisplayMenu,
	&M_DrawDisplay,
	84,
	22,
	1,
	0,
	0
};

char skullName[2][8] = { "M_SKULL1", "M_SKULL2" };
void* stringcallback;
void (*keycallback)(int ch);

int huinput = 0;

font_t* hudfont;

int stringx, stringy;
menu_t* currentMenu;

//============================================= 
// 
// M_DrawMainMenu 
// 
//============================================= 
void M_DrawMainMenu(void)
{
	V_DrawPatch(94, 2, (patch_t*)W_GetName("M_DOOM"));
}

//============================================= 
// 
// M_NewGame 
// 
//============================================= 
void M_DrawNewGame(void)
{
	V_DrawPatch(96, 14, (patch_t*)W_GetName("M_NEWG"));
	V_DrawPatch(54, 38, (patch_t*)W_GetName("M_SKILL"));
}

void M_NewGame(int choice)
{
	M_SetupNextMenu(&NewDef);
}

void M_StartGame(int choice)
{
	G_StartNewGame(1, choice, 0);
}

//============================================= 
// 
// M_Options 
// 
//============================================= 
void M_DrawOptions(void)
{
	V_DrawPatch(108, 5, (patch_t*)W_GetName("M_OPTTTL"));
	M_DrawThermo(OptionsDef.x, OptionsDef.y + 80, 16, soundVolume);
}

void M_ChangeVolume(int choice)
{
	switch (choice)
	{
	case 0:
		if (soundVolume != 0)
			soundVolume--;
		break;
	case 1:
		if (soundVolume < 15)
			soundVolume++;
		break;
	}
}

void M_Options(int choice)
{
	M_SetupNextMenu(&OptionsDef);
}

//============================================= 
// 
// M_EndGame
// 
//============================================= 
void M_EndGame(int choice)
{
}

//============================================= 
// 
// M_ReadThis 
// 
//============================================= 
void M_ReadThis(int choice)
{
	//D_Alert("this");
}

//============================================= 
// 
// M_QuitDOOM 
// 
//============================================= 
void M_QuitDOOM(int choice)
{
	IO_Quit();
}

void M_DrawControls(void)
{
	V_DrawPatch(100, 5, (patch_t*)W_GetName("M_CTRLS"));
	M_DrawEmptyCell(&ControlsDef, 0);
	M_DrawEmptyCell(&ControlsDef, 1);
	M_DrawEmptyCell(&ControlsDef, 2);
	M_DrawEmptyCell(&ControlsDef, 3);
	V_DrawPatch(ControlsDef.x + 170, ControlsDef.y + 83, (patch_t*)W_GetName(joyTypeNames[joystickType]));
	M_DrawThermo(ControlsDef.x, ControlsDef.y + 112, 10, mouseSensitivity);
}

void M_ChangeJoyType(int choice)
{
	joystickType = joystickType ^ 1;
}

void M_ChangeSensitivity(int choice)
{
	switch (choice)
	{
	case 0:
		if (mouseSensitivity != 0)
			mouseSensitivity--;
		break;
	case 1:
		if (mouseSensitivity < 9)
			mouseSensitivity++;
		break;
	}
}

void M_Controls(int choice)
{
	M_SetupNextMenu(&ControlsDef);
}

void M_DrawDisplay(void)
{
	V_DrawPatch(52, 2, (patch_t*)W_GetName("M_DISOPT"));
	M_DrawEmptyCell(&DisplayDef, 1);
	M_DrawEmptyCell(&DisplayDef, 2);
	M_DrawEmptyCell(&DisplayDef, 3);
	M_DrawEmptyCell(&DisplayDef, 4);
	M_DrawEmptyCell(&DisplayDef, 5);
	M_DrawSelCell(&DisplayDef, currentdetail + 1);
	M_DrawThermo(DisplayDef.x, DisplayDef.y + 128, 11, screenblocks - 1);
}

void M_ChangeDisplay(int choice)
{
	R_SetDetail(choice - 1);
}

void M_SizeDisplay(int choice)
{
	switch (choice)
	{
	case 0:
		if (screenblocks > 0)
			R_SizeDown();
		break;
	case 1:
		if (screenblocks < 11)
			R_SizeUp();
		break;
	}
}

void M_Display(int choice)
{
	M_SetupNextMenu(&DisplayDef);
}

//============================================= 
//============================================= 
// 
//      Menu Functions 
// 
//============================================= 
//============================================= 
void M_DrawThermo(int x, int y, int thermWidth, int thermDot)
{
	int xx, i;

	xx = x;
	V_DrawPatch(xx, y, (patch_t*)W_GetName("M_THERML"));

	xx += 8;
	for (i = 0; i < thermWidth; i++)
	{
		V_DrawPatch(xx, y, (patch_t*)W_GetName("M_THERMM"));
		xx += 8;
	}

	V_DrawPatch(xx, y, (patch_t*)W_GetName("M_THERMR"));
	V_DrawPatch(thermDot * 8 + x + 8, y, (patch_t*)W_GetName("M_THERMO"));
}

void M_DrawEmptyCell(menu_t* menu, int item)
{
	patch_t* patch = (patch_t*)W_GetName("M_CELL1");
	V_DrawPatch(menu->x - 10, item * LINEHEIGHT + menu->y + -1, patch);
}

void M_DrawSelCell(menu_t* menu, int item)
{
	patch_t* patch = (patch_t*)W_GetName("M_CELL2");
	V_DrawPatch(menu->x - 10, item * LINEHEIGHT + menu->y + -1, patch);
}

void M_InputKey(void* callback)
{
	huinput = 0;
	keycallback = callback;
}

void M_InputString(int sx, int sy, int maxchars, void* callback)
{
	huinput = 1;
	stringy = sy;
	stringx = sx;
	maxwidth = maxchars;
	memset(inputstring, 0, sizeof(inputstring));
	stringcallback = callback;
	validwidth = 0;
}

void M_DrawInput(void)
{
	int max = V_DrawString(stringx, stringy, inputstring, hudfont);
	if ((processedframe & 16) != 0)
	{
		V_DrawChar(max, stringy, 95, hudfont);
	}
}

/*
===============================================================================

							CONTROL PANEL

===============================================================================
*/

/*
=====================
=
= M_ControlCallback
=
======================
*/

void M_ControlCallback(int ch)
{
	static int mapnum = 1;

	if (ch == KEY_F10)
		IO_Quit();
	
	if (menuDisplayed == 0)
	{
		if (ch == KEY_ESCAPE)
			M_StartControlPanel();
	}
	else //[ISB] is this a switch in the original like M_Responder in the release source?
	{
		if (ch == KEY_DOWNARROW)
		{
			do
			{
				if (currentMenu->numitems - 1 < itemOn + 1)
					itemOn = 0;
				else
					itemOn++;
			} while (currentMenu->items[itemOn].status == 0);
		}
		else if (ch == KEY_UPARROW)
		{
			do
			{
				if (!itemOn)
					itemOn = currentMenu->numitems;
				itemOn--;
			} while (currentMenu->items[itemOn].status == 0);
		}
		else if (ch == KEY_LEFTARROW)
		{
			if (currentMenu->items[itemOn].routine && currentMenu->items[itemOn].status == 2)
				currentMenu->items[itemOn].routine(0);
		}
		else if (ch == KEY_RIGHTARROW)
		{
			if (currentMenu->items[itemOn].routine && currentMenu->items[itemOn].status == 2)
				currentMenu->items[itemOn].routine(1);
		}
		else if (ch == KEY_ENTER)
		{
			if (currentMenu->items[itemOn].routine)
			{
				currentMenu->lastOn = itemOn;
				currentMenu->lastScrollIndex = scrollIndex;
				if (currentMenu->items[itemOn].status == 2)
					currentMenu->items[itemOn].routine(1);
				else
					currentMenu->items[itemOn].routine(itemOn);
			}
		}
		else if (ch == KEY_ESCAPE)
		{
			currentMenu->lastOn = itemOn;
			currentMenu->lastScrollIndex = scrollIndex;
			if (currentMenu->prevMenu)
			{
				currentMenu = currentMenu->prevMenu;
				itemOn = currentMenu->lastOn;
				scrollIndex = currentMenu->lastScrollIndex;
			}
			else
				M_ClearMenus();
		}
		else if ((ch == KEY_S) && (keydown[KEY_DEBUG] != 0))
		{
			M_DrawSelf();
			D_ScreenShot(0);
		}
	}

	if (ch == KEY_MINUS)
	{
		mapnum--;
		if (mapnum < 1)
			mapnum = 1;
		
		G_StartNewGame(mapnum, sk_hard, 0);
	}
	if (ch == KEY_EQUALS)
	{
		mapnum++;
		if (mapnum >= 13)
			mapnum = 13;
		
		G_StartNewGame(mapnum, sk_hard, 0);
	}
	if ((ch >= KEY_1) && (ch < KEY_0))
	{
		mapnum = ch - KEY_1 + 1;
		G_StartNewGame(mapnum, 3, 0);
	}
	M_InputKey(&M_ControlCallback);
}
/*
=====================
=
= M_StartControlPanel
=
======================
*/

void M_StartControlPanel(void)
{
	menuDisplayed = 1;
	currentMenu = &MainDef;
	itemOn = currentMenu->lastOn;
	ignorekeyboard++;
	M_InputKey(&M_ControlCallback);
}

/*
==============================================================================
=
= M_DrawSelf
=
= Called after the view has been rendered, but before it has been blitted.
=
==============================================================================
*/

void M_DrawSelf(void)
{
	static short	x, y;
	short			i, max;

	if (!menuDisplayed)
		return;

	R_StartInstanceDrawing();
	if (currentMenu->routine)
		currentMenu->routine();         // call Draw routine 
	
	// 
	// DRAW MENU 
	// 
	x = currentMenu->x;
	y = currentMenu->y;
	max = currentMenu->numitems;
	if (currentMenu->scrollsize != 0)
		max = currentMenu->scrollsize;
	
	for (i = 0; i < max; i++)
	{
		if (currentMenu->items[scrollIndex + i].name[0] != '\0')
			V_DrawPatch(x, y, 
				(patch_t*)W_GetName(currentMenu->items[scrollIndex + i].name));
			
		y += LINEHEIGHT;
	}

	// 
	// DRAW SKULL 
	// 
	if (--skullAnimCounter == 0)
	{
		whichSkull ^= 1;
		skullAnimCounter = 20;
	}
	V_DrawPatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn * LINEHEIGHT, 
		(patch_t*)W_GetName(skullName[whichSkull]));
}

void M_CheckInput(framecmd_t* cmd)
{
	if (controlmenumap)
	{
		cmd->xmove = 10;
		cmd->ymove = 0;
		cmd->buttons = wp_nochange << BT_WEAPONSHIFT;
	}

	if (cmd->keyscan)
	{
		if (menuDisplayed == 0)
		{
			if (cmd->keyscan == KEY_DEBUG)
			{
				if (keydown[KEY_C] != 0)
				{
					IO_ClearKeys();
					gameaction = ga_completed;
				}
				if (keydown[KEY_D] != 0)
				{
					IO_ClearKeys();
					gameaction = ga_died;
				}
				if (keydown[KEY_L] != 0)
				{
					keydown[KEY_L] = 0;
					D_ScreenShot(0);
				}
				if (keydown[KEY_P] != 0)
				{
					keydown[KEY_P] = 0;
					D_ScreenShot(1);
				}
				if (keydown[KEY_S] != 0)
				{
					R_TestSprites();
				}
			}
			if ((cmd->keyscan == KEY_ESCAPE) || (demo == dm_playback))
			{
				M_StartControlPanel();
				cmd->keyscan = 0;
			}
			if (cmd->keyscan == KEY_A)
			{
				P_EnterAutoMap();
				cmd->keyscan = 0;
			}
		}
		else
		{
			if (keycallback == NULL)
				IO_Error("M_CheckInput: no keycallback\n");
			
			keycallback(cmd->keyscan);
		}
	}
}

/*
=====================
=
= M_Startup
=
======================
*/

void M_Startup(void)
{
	hudfont = (font_t*)W_GetName("HUFONT");
	currentMenu = &MainDef;
	menuDisplayed = 1;
	itemOn = currentMenu->lastOn;
	whichSkull = 0;
	scrollIndex = 0;
	skullAnimCounter = 10;
}

/*
=====================
=
= M_ClearMenus
=
======================
*/

void M_ClearMenus(void)
{
	if (menuDisplayed != 0)
	{
		menuDisplayed = 0;
		ignorekeyboard = 0;
	}
}

//============================================= 
// 
// M_SetupNextMenu 
// 
//============================================= 
void M_SetupNextMenu(menu_t* menudef)
{
	currentMenu = menudef;
	itemOn = menudef->lastOn;
	scrollIndex = menudef->lastScrollIndex;
}

