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
#include "s_sound.h"
#include "i_local.h"
#include "p_play.h"
#include "sdl_gl.h"

#include <stdarg.h>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_surface.h>

void IO_Shutdown();

//comm nonsense
int commpresent;

//timer nonsense
int ticcount;
SDL_TimerID gametimer;
Uint32 basetime;

//keyboard nonsense
//key bindings
int key_right = KEY_RIGHT;
int key_left = KEY_LEFT;
int key_up = KEY_UP;
int key_down = KEY_DOWN;
int key_raise = KEY_EQUALS; //unused
int key_lower = KEY_MINUS; //unused
int key_fire = SDL_SCANCODE_LCTRL;
int key_strafe = SDL_SCANCODE_LALT;
int key_speed = SDL_SCANCODE_LSHIFT;
int key_weapon[8] = { KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8 };

//key status
int lastscan;
int lastpress;
int keydown[256];
int ignorekeyboard;

//key emulation
int paused;
int specialkey;

int autorun = 1; //[ISB]

//mouse nonsense
int novert; //[ISB]

//[ISB] sdl nonsense
SDL_Window* window;

void IO_SendFrame();

void IO_ClearKeys()
{
	lastscan = lastpress = 0;
	memset(keydown, 0, sizeof(keydown));
}

int IO_GetTime()
{
	return ticcount;
}

void IO_StartAck()
{
	IO_ClearKeys();
}

int IO_CheckAck()
{
	int iVar1 = 0;

	IO_DoEvents(); //[ISB] make sure events are still pumped
	iVar1 = lastscan;
	if (lastscan != 0) 
	{
		keydown[lastscan] = 0;
		lastscan = 0;
	}
	return iVar1;
}

void IO_Ack(void)
{
	int iVar1;

	IO_StartAck();
	do 
	{
		iVar1 = IO_CheckAck();
	} while (iVar1 == 0);
	return;
}

void IO_PlayerInput()
{
	framecmd_t* cmd;
	int frame, i;
	weapontype_t newweapon;
	int xmove, ymove;
	int speed;
	int deltax, deltay;

	ticcount++;

	frame = sd->playercmdframe[sd->consoleplayer];
	if (frame == -3)
		return;

	if (frame == -2)
	{
		IO_SendFrame();
		return;
	}

	frame++;
	sd->playercmdframe[sd->consoleplayer] = frame;


	cmd = &sd->playercmd[sd->consoleplayer * NUMPLAYERFRAMES + (frame & PLAYERFRAMEMASK)];
	cmd->keyscan = lastpress;
	lastpress = 0;
	cmd->buttons = 0x3c;
	xmove = ymove = 0;
	speed = 40;

	if (ignorekeyboard == 0) 
	{
		if (keydown[key_speed] == autorun) //[ISB] I had to, sorry.
			speed = 80;
		
		if (keydown[key_right] != 0) 
			xmove = speed;
		
		if (keydown[key_left] != 0) 
			xmove -= speed;
		
		if (keydown[key_up] != 0) 
			ymove = speed;
		
		if (keydown[key_down] != 0)
			ymove -= speed;
		
		cmd->buttons = 0;
		if (keydown[key_fire] != 0) 
			cmd->buttons |= 2;
		
		if (keydown[key_strafe] != 0) 
			cmd->buttons |= 1;
		
		newweapon = wp_nochange;
		
		for (i = 0; i < 8; i++)
		{
			if (keydown[key_weapon[i]] != 0) 
			{
				newweapon = i;
			}
		}
		cmd->buttons |= newweapon << 2;
	}

	if (sd->mousepresent != 0) 
	{
		if ((sd->mousebuttons & 1) != 0) 
			cmd->buttons |= 2;
		
		if ((sd->mousebuttons & 2) != 0) 
			cmd->buttons |= 1;
		
		if ((sd->mousebuttons & 4) != 0)
			ymove += speed;

		if (sd->mousex < 0x5555 && sd->oldmousex > 0xAAAA)
		{
			deltax = sd->mousex + 0x10000 - sd->oldmousex;
		}
		else if (sd->mousex > 0xAAAA && sd->oldmousex < 0x5555)
		{
			deltax = -(0x1000 - sd->mousex + sd->oldmousex);
		}
		else
		{
			deltax = sd->mousex - sd->oldmousex;
		}

		if (sd->mousey < 0x5555 && sd->oldmousey > 0xAAAA)
		{
			deltay = sd->mousey + 0x10000 - sd->oldmousey;
		}
		else if (sd->mousey > 0xAAAA && sd->oldmousey < 0x5555)
		{
			deltay = -(0x1000 - sd->mousey + sd->oldmousey);
		}
		else
		{
			deltay = sd->mousey - sd->oldmousey;
		}

		sd->oldmousex = sd->mousex;
		sd->oldmousey = sd->mousey;

		if ((cmd->buttons & 1) == 0) 
			xmove += (deltax << 8) / 300;
		else
			xmove += (deltax << 8) / 150;
		
		ymove -= (deltay << 8) / 150;
	}

	if (xmove > 127)
		xmove = 127;
	else if (xmove < -128)
		xmove = -128;
	
	if (ymove > 127)
		ymove = 127;
	else if (ymove < -128)
		ymove = -128;

	cmd->xmove = xmove;
	cmd->ymove = ymove;
	IO_SendFrame();
}

void IO_NewFrame()
{
	if (sd->playercmdframe[sd->consoleplayer] < processedframe) 
	{
		IO_Error("IO_NewFrame: playerframe < processedframe");
	}
	while (sd->playercmdframe[sd->consoleplayer] == processedframe)
	{
		IO_DoEvents();
	}
	return;
}

void IO_FindShared()
{
	sd = malloc(sizeof(shared_t));
	//[ISB]
	if (sd == NULL)
	{
		IO_Error("IO_FindShared: Cannot initalize shared memory\n");
		return;
	}
	memset(sd, 0, sizeof(shared_t));
}

void IO_SendFrame()
{
	//rip
}

void IO_Quit()
{
	IO_Shutdown();
	S_Shutdown();
	D_SaveConfig();
	exit(0);
}

#ifdef _MSC_VER
__declspec(noreturn) void IO_Error(char* fmt, ...)
#else
void IO_Error(char* fmt, ...)
#endif
{
	char heh[1024];
	snprintf(heh, 1024, "(%i, 0x%x, 0x%x, 0x%x, %i)\n", viewsector, viewx, viewy, viewz, viewangle);
	heh[1023] = '\0';
	va_list arglist;
	char buf[1024];
	va_start(arglist, fmt);
	vsnprintf(buf, 1024, fmt, arglist);
	va_end(arglist);

	fprintf(stderr, buf);

#ifndef _CONSOLE
	SDL_ShowSimpleMessageBox(0, heh, buf, window);
#endif

	//[ISB] this is originally done earlier but this causes SDL crashes
	IO_Shutdown();
	S_Shutdown();

	exit(1);
}

void SDL_Startup()
{
	int width = 1024;
	int height = 768;
	int arg;
	int flags = SDL_WINDOW_OPENGL;
	int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	if (res)
	{
		IO_Error("Error initalizing SDL: %s\n", SDL_GetError());
	}

	arg = D_CheckParm("width");
	if (arg && (arg+1) < my_argc)
	{
		width = atoi(my_argv[arg + 1]);
	}

	arg = D_CheckParm("height");
	if (arg && (arg + 1) < my_argc)
	{
		height = atoi(my_argv[arg + 1]);
	}

	if (D_CheckParm("fullscreen"))
		flags |= SDL_WINDOW_FULLSCREEN;

	IO_InitGLContextAttribs();
	window = SDL_CreateWindow("Doom 0.5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	if (!window)
	{
		IO_Error("Error creating game window: %s\n", SDL_GetError());
		return;
	}

	if (D_CheckParm("autorun"))
		autorun = 0;
}

void SDL_Shutdown()
{
}

Uint32 IO_TimerCallback(Uint32 interval, void* param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	/* In this example, our callback pushes an SDL_USEREVENT event
	into the queue, and causes our callback to be called again at the
	same interval: */

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = &IO_PlayerInput;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return(interval);
}

//Try to smooth over timing issues with floating point nonsense. Ugh. 
int IO_InternalTime()
{
	Uint32 SDLTics = SDL_GetTicks() - basetime;
	double secs = SDLTics / 1000.0;

	return (int)(secs * 35);
}

void IO_StartupTimer()
{
	basetime = SDL_GetTicks();
	//SDL only using MS for timing is still one of the worst things ever, btw.
	/*gametimer = SDL_AddTimer(58, &IO_TimerCallback, NULL);
	if (!gametimer)
		IO_Error("IO_StartupTimer: Cannot initalize timer: %s\n", SDL_GetError());*/
}

void IO_ShutdownTimer()
{
	//SDL_RemoveTimer(gametimer);
}

void IO_StartupMouse()
{
	sd->mousepresent = 1;
	if (D_CheckParm("novert"))
		novert = 1;
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void IO_ShutdownMouse()
{
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void IO_StartupGraphics()
{
	IO_StartupGL(window);
	SDL_Rect rectangle;
	rectangle.x = 0; rectangle.y = 0;
	rectangle.w = 320; rectangle.h = 200;
	IO_GL_SetVideoMode(320, 200, &rectangle);

	//debug
	/*
	for (int i = 0; i < 80 * 200; i+=4)
	{
		IO_SetMapMask(rand() & 15);
		IO_WriteMungeDWord(rand() | (rand() << 16), i);
	}*/
}

void IO_ShutdownGraphics()
{
	if (window)
	{
		SDL_DestroyWindow(window);
		IO_ShutdownGL();
	}
}

void IO_Startup()
{
	IO_FindShared();
	novideo = D_CheckParm("novideo");

	printf("SDL_Startup\n");
	SDL_Startup();

	printf("IO_StartupMouse\n");
	IO_StartupMouse();

	printf("IO_StartupTimer\n");
	IO_StartupTimer();

	printf("IO_StartupGraphics\n");
	IO_StartupGraphics();
}

void IO_Shutdown()
{
	IO_ShutdownMouse();
	IO_ShutdownTimer();
	IO_ShutdownGraphics();
	SDL_Shutdown();
}

void IO_MouseHandler()
{
	int x, y;
	sd->mousebuttons = SDL_GetRelativeMouseState(&x, &y);
	sd->mousex += x;
	if (!novert)
		sd->mousey += y;
}

void IO_KeyHandler(SDL_Scancode incode, int state)
{
	paused = 0;
	specialkey = 0;

	//Emulate handling of special keys. 
	if (incode == SDL_SCANCODE_PAUSE)
	{
		paused = 1;
		incode = SDL_SCANCODE_LCTRL;
	}
	if (incode == SDL_SCANCODE_RCTRL || incode == SDL_SCANCODE_RSHIFT || incode == SDL_SCANCODE_RALT)
	{
		specialkey = 1;
		incode -= 4;
	}

	if (incode < 256)
	{
		keydown[incode] = state;
		if (state == SDL_PRESSED)
		{
			lastscan = incode;
			lastpress = incode;
		}
	}
}

//sdl nonsense
void IO_DoEvents()
{
	//Timing
	int idealTics = IO_InternalTime();

	while (ticcount < idealTics)
	{
		IO_PlayerInput();
	}

	IO_MouseHandler();

	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
			//Flush input if you click the window, so that you don't abort your game when clicking back in at the ESC menu. heh...
		case SDL_WINDOWEVENT:
		{
			SDL_WindowEvent winEv = ev.window;
			switch (winEv.event)
			{
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				SDL_FlushEvents(SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				IO_Quit();
				return; //should never get here
			}
		}
		/*case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			IO_MouseHandler(ev.button.button, ev.button.state);
			break;*/
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			IO_KeyHandler(ev.key.keysym.scancode, ev.key.state);
			break;
		case SDL_USEREVENT:
			break;
		}
	}
	//TODO: Do elsewhere
	IO_GL_DrawFramebuffer();
	SDL_GL_SwapWindow(window);
}

