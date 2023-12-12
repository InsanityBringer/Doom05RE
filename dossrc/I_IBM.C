#include <stdarg.h>
#include <stdio.h>
#include <i86.h>
#include <conio.h>
#include <dos.h>
#include <graph.h> //no idea why

#include "doomdef.h"
#include "w_wad.h"
#include "v_video.h"
#include "r_ref.h"
#include "g_game.h"
#include "p_play.h"
#include "i_local.h"
#include "s_sound.h"

/*
=============================================================================

							CONSTANTS

=============================================================================
*/

#define SC_INDEX                0x3C4
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_H_TOTAL    0
#define CRTC_H_DISPEND  1
#define CRTC_H_BLANK    2
#define CRTC_H_ENDBLANK 3
#define CRTC_H_RETRACE  4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL    6
#define CRTC_OVERFLOW   7
#define CRTC_ROWSCAN    8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND  11
#define CRTC_STARTHIGH  12
#define CRTC_STARTLOW   13
#define CRTC_CURSORHIGH 14
#define CRTC_CURSORLOW  15
#define CRTC_V_RETRACE  16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND  18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE  20
#define CRTC_V_BLANK    21
#define CRTC_V_ENDBLANK 22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE 24


#define GC_INDEX                0x3CE
#define GC_SETRESET             0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE 2
#define GC_DATAROTATE   3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN    17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN              19
#define ATR_COLORSELECT 20

#define STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA                0x3c9
#define PEL_MASK                0x3c6


//TODO: Determine how watcom chose variable packing, may be based on usage order?
int commpresent;
int capslock;
void* profile_p;
int paused;

byte* buffercrtc;

int lastpress;
void* profilebuffer;
int lastscan;

byte* screen;

int timerspeed;
int specialkey;

int lastascii;



volatile int oldisrcounter;

void (__interrupt __far *oldtimerisr)();
void (__interrupt __far *oldkeyboardisr)();

void __far *sourceplane; //todo: two padding bytes, is it a far pointer? or is it useless because it isn't actually referenced

union REGS regs;
struct SREGS segregs;

volatile int keydown[256];

//keyboard nonsense
//key bindings
int key_right = KEY_RIGHTARROW;
int key_left = KEY_LEFTARROW;
int key_up = KEY_UPARROW;
int key_down = KEY_DOWNARROW;
int key_raise = KEY_EQUALS; //unused
int key_lower = KEY_MINUS; //unused
int key_fire = KEY_LCTRL;
int key_strafe = KEY_LALT;
int key_speed = KEY_LSHIFT;
int key_weapon[8] = { 2, 3, 4, 5, 6, 7, 8, 0 };

int ignorekeyboard;

volatile int ticcount = 0;
int hookspeed = 0x7D00;
volatile int hookcounter;


char        ASCIINames[] =		// Unshifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					},
					ShiftNames[] =		// Shifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,	// 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',	// 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',	// 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					SpecialNames[] =	// ASCII for 0xe0 prefixed codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
	0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					};

#define VBLCOUNTER              16000           // hardware tics to a frame


#define TIMERINT 8
#define KEYBOARDINT 9


void dpmi_lockregion(void *addr, size_t size)
{
	union REGS regs;
	
	regs.w.ax = 0x600;
	regs.w.bx = (unsigned short)((size_t)addr >> 16);
	regs.w.cx = (unsigned short)((size_t)addr & 0xFFFF);
	regs.w.si = (unsigned short)(size >> 16);
	regs.w.di = (unsigned short)(size & 0xFFFF);
	
	int386(0x31, &regs, &regs);
}

void dpmi_unlockregion(void *addr, size_t size)
{
	union REGS regs;
	
	regs.w.ax = 0x601;
	regs.w.bx = (unsigned short)((size_t)addr >> 16);
	regs.w.cx = (unsigned short)((size_t)addr & 0xFFFF);
	regs.w.si = (unsigned short)(size >> 16);
	regs.w.di = (unsigned short)(size & 0xFFFF);
	
	int386(0x31, &regs, &regs);
}

void IO_ClearKeys()
{
	lastscan = lastpress = 0;
	memset((void*)keydown, 0, sizeof(keydown));
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
	int c = 0;

	c = lastscan;
	if (lastscan != 0) 
	{
		keydown[lastscan] = 0;
		lastscan = 0;
	}
	return c;
}

void IO_Ack(void)
{
	int c;

	IO_StartAck();
	do 
	{
		c = IO_CheckAck();
	} while (c == 0);
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
		if (keydown[key_speed] != 0)
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
		IO_Error("IO_NewFrame: playerframe < processedframe");
	
	do {}
		while (sd->playercmdframe[sd->consoleplayer] == processedframe);
}

void IO_BeginUserInput()
{
}

void IO_EndUserInput()
{
}

void IO_SuspendUserInput()
{
}

void IO_ResumeUserInput()
{
}

void IO_StartupSound()
{
	S_Startup();
}

void IO_ShutdownSound()
{
	S_Shutdown();
}

void IO_FindShared()
{
	int v, seg, off;
	
	for (v = 0x60; v < 0x68; v++)
	{
		off = ((unsigned short*)0)[v * 2];
		seg = ((unsigned short*)2)[v * 2];
		
		sd = (shared_t*)((seg << 4) + off); 
		
		if (sd->ident == 0x444f4f4d)
		{
			printf("Found shared memory at 0x%x\n", sd);
			return;
		}
	}
	
	printf("No shared memory found\n");
	sd = (shared_t*)malloc(sizeof(*sd));
	memset(sd, 0, sizeof(*sd));
}

void IO_StartupComm()
{
	int i;
	for (i = 0; i < MAXPLAYERS; i++)
	{
		sd->playercmdframe[i] = -3;
	}
	commpresent = sd->commint != 0;
}

void IO_SendFrame()
{
	if (commpresent)
		int386(0x61, &regs, &regs);
}

void IO_ShutdownComm()
{
}

void IO_ColorBlack (int r, int g, int b)
{
	outp(PEL_WRITE_ADR,0);
	outp(PEL_DATA,r);
	outp(PEL_DATA,g);
	outp(PEL_DATA,b);
}

void __interrupt __far IO_TimerISR()
{
#pragma aux IO_TimerISR parm loadds
	hookcounter += timerspeed;
	
	if (hookspeed <= hookcounter) 
	{
		hookcounter -= hookspeed;
		IO_PlayerInput();
	}
	oldisrcounter += timerspeed;
	if (oldisrcounter < 65536) 
		outp(32,32);
	else 
	{
		oldisrcounter -= 65536;
		_chain_intr(oldtimerisr);
	}
}

//[ISB] from the heretic source, this is all the same
/*
=====================
=
= IO_SetTimer0
=
= Sets system timer 0 to the specified speed
=
=====================
*/

void IO_SetTimer0(int speed)
{
	if (speed > 0 && speed < 150)
		IO_Error ("INT_SetTimer0: %i is a bad value",speed);

	timerspeed = speed;
	outp(0x43,0x36);                            // Change timer 0
	outp(0x40,speed);
	outp(0x40,speed >> 8);
}


/*
===============
=
= IO_StartupTimer
=
===============
*/

void IO_StartupTimer (void)
{
	oldtimerisr = _dos_getvect(TIMERINT);
	oldisrcounter = 0;
	dpmi_lockregion((void*)IO_TimerISR, 4096);
	dpmi_lockregion((void*)IO_PlayerInput, 4096);
	dpmi_lockregion((void*)ticcount, sizeof(ticcount));
	dpmi_lockregion((void*)hookspeed, sizeof(hookspeed));
	dpmi_lockregion((void*)hookcounter, sizeof(hookcounter));
	dpmi_lockregion((void*)timerspeed, sizeof(timerspeed));
	dpmi_lockregion((void*)oldisrcounter, sizeof(oldisrcounter));
	dpmi_lockregion((void*)ignorekeyboard, sizeof(ignorekeyboard));
	
	_dos_setvect (0x8000 | TIMERINT, IO_TimerISR);
	IO_SetTimer0 (VBLCOUNTER);
}

void IO_ShutdownTimer (void)
{
	if (oldtimerisr)
	{
		IO_SetTimer0 (0);              // back to 18.4 ips
		_dos_setvect (TIMERINT, oldtimerisr);
		dpmi_unlockregion((void*)IO_TimerISR, 4096);
		dpmi_unlockregion((void*)IO_PlayerInput, 4096);
		dpmi_unlockregion((void*)ticcount, sizeof(ticcount));
		dpmi_unlockregion((void*)hookspeed, sizeof(hookspeed));
		dpmi_unlockregion((void*)hookcounter, sizeof(hookcounter));
		dpmi_unlockregion((void*)timerspeed, sizeof(timerspeed));
		dpmi_unlockregion((void*)oldisrcounter, sizeof(oldisrcounter));
		dpmi_unlockregion((void*)ignorekeyboard, sizeof(ignorekeyboard));
	}
}

void __interrupt __loadds IO_KeyboardISR(void)
{
	byte k;
	
	k = inp(0x60);
	outp(0x20,0x20);
	if (k == 224) 
	{
		specialkey = 1;
	}
	else if (k == 225) 
	{
		paused = 1;
	}
	else 
	{
		if (!(k & 128)) 
		{
			lastscan = k;
			lastpress = k;
			keydown[k] = 1;
		}
		else 
		{
			keydown[k & 127] = 0;
		}
		specialkey = 0;
	}
}

void IO_StartupKeyboard (void)
{
	oldkeyboardisr = _dos_getvect(KEYBOARDINT);
	_dos_setvect (0x8000 | KEYBOARDINT, IO_KeyboardISR);
	dpmi_lockregion((void*)IO_KeyboardISR, 4096);
	dpmi_lockregion((void*)keydown, sizeof(keydown));
	dpmi_lockregion((void*)paused, sizeof(paused));
	dpmi_lockregion((void*)lastscan, sizeof(lastscan));
	dpmi_lockregion((void*)lastpress, sizeof(lastpress));
	dpmi_lockregion((void*)specialkey, sizeof(specialkey));
}

void IO_ShutdownKeyboard (void)
{
	if (oldkeyboardisr)
	{
		_dos_setvect (KEYBOARDINT, oldkeyboardisr);
		dpmi_unlockregion((void*)IO_KeyboardISR, 4096);
		dpmi_unlockregion((void*)keydown, sizeof(keydown));
		dpmi_unlockregion((void*)paused, sizeof(paused));
		dpmi_unlockregion((void*)lastscan, sizeof(lastscan));
		dpmi_unlockregion((void*)lastpress, sizeof(lastpress));
		dpmi_unlockregion((void*)specialkey, sizeof(specialkey));
	}
	*(short *)0x41c = *(short *)0x41a;      // clear bios key buffer
}

void __interrupt __far IO_MouseService(int mousex, int mousey, int mousebuttons)
{
#pragma aux IO_MouseService parm loadds [ESI] [EDI] [EBX]
	sd->mousex = mousex;
	sd->mousey = mousey;
	sd->mousebuttons = mousebuttons;
}

int IO_ResetMouse (void)
{
	regs.w.ax = 0;                  // reset
	int386 (0x33, &regs, &regs);
	return regs.w.ax;
}

void IO_StartupMouse()
{
	void* far function;
	sd->mousepresent = false;
	if (IO_ResetMouse() == 65535)
	{
		printf("Mouse detected\n");
		sd->mousepresent = 1;
		dpmi_lockregion((void*)IO_MouseService, 4096);
		regs.w.ax = 12;
		regs.w.cx = 255;
		regs.x.edx = FP_OFF(IO_MouseService);
		segread(&segregs);
		segregs.es = FP_SEG(IO_MouseService);
		int386x(0x33, &regs, &regs, &segregs);
	}
	else
	{
		printf("Mouse not present\n");
	}
}

void IO_ShutdownMouse()
{
	if (sd->mousepresent != 0) 
	{
		dpmi_unlockregion((void*)IO_MouseService,4096);
		IO_ResetMouse();
	}
}

void IO_StartupGraphics()
{
	if (novideo)
	{
		return;
	}
	
	screen = (byte*)0xa0000;
	
	regs.w.ax = 0x13;
	int386(0x10, &regs, &regs);
	
	//
	// turn off chain 4 and odd/even
	//
	outp (SC_INDEX,SC_MEMMODE);
	outp (SC_INDEX+1,(inp(SC_INDEX+1)&~8)|4);

	//
	// turn off odd/even and set write mode 0
	//
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)&~0x13);

	//
	// turn off chain
	//
	outp (GC_INDEX,GC_MISCELLANEOUS);
	outp (GC_INDEX+1,inp(GC_INDEX+1)&~2);
	
	outpw(SC_INDEX,3842);
	memset(screen, 0, 65536);
	
	outp(CRTC_INDEX,CRTC_UNDERLINE);
    outp(CRTC_INDEX+1,inp(981) & 191);
    outp(CRTC_INDEX,CRTC_MODE);
    outp(CRTC_INDEX+1,inp(981) | 64);
}

void IO_ShutdownGraphics()
{
	if (novideo)
		return;
	
	regs.w.ax = 3;
	int386(0x10, &regs, &regs); // back to text mode
}

extern void cdecl _GETDS();

void IO_Startup()
{
	novideo = D_CheckParm("novideo");
	dpmi_lockregion((void*)_GETDS,4096);
	dpmi_lockregion((void*)_chain_intr,4096);
	dpmi_lockregion((void*)inp,4096);
	dpmi_lockregion((void*)outp,4096);
	
	IO_FindShared();
	printf("IO_StartupMouse\n");
	IO_StartupMouse();
	printf("IO_StartupComm\n");
	IO_StartupComm();
	printf("IO_StartupKeyboard\n");
	IO_StartupKeyboard();
	printf("IO_StartupTimer\n");
	IO_StartupTimer();
	printf("IO_StartupGraphics\n");
	IO_StartupGraphics();
}

void IO_Shutdown()
{
	IO_ShutdownSound();
	IO_ShutdownMouse();
	IO_ShutdownComm();
	IO_ShutdownKeyboard();
	IO_ShutdownTimer();
	IO_ShutdownGraphics();
	dpmi_unlockregion(_GETDS,4096);
	dpmi_unlockregion(_chain_intr,4096);
	dpmi_unlockregion(outp,4096);
	dpmi_unlockregion(inp,4096);
}

void IO_Error(char *error, ...)
{
	va_list argptr;
	
	IO_Shutdown();
	S_Shutdown();
	if (demo == dm_recording) 
	{
		printf("Writing CRASHDEM.LMP\n");
		D_WriteFile("CRASHDEM.LMP", demobuffer, (int)demo_p - (int)demobuffer);
	}
	printf("(%i, 0x%x, 0x%x, 0x%x, %i)\n", viewsector, viewx, viewy, viewz, viewangle);
	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}

void IO_Quit(void)
{
	byte *scr;

	D_SaveConfig();
	scr = W_GetName("ENDOOM");
	IO_Shutdown();
	S_Shutdown();
	memcpy((void *)0xB8000,scr,4000);
	
	regs.w.ax = 512;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int386(16,&regs, &regs);
					/* WARNING: Subroutine does not return */
	exit(0);
}
