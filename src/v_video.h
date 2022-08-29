#pragma once

typedef struct
{
	uint8_t width;
	uint8_t height;
	uint8_t data;
} pic_t;

typedef struct
{
	short height;
	uint8_t width[256];
	short charofs[256];
} font_t;

typedef struct
{
	uint8_t topdelta;
	uint8_t length;
	uint8_t data;
} post_t;

typedef post_t collumn_t;

typedef struct
{
	uint8_t width;
	uint8_t height;
	int8_t leftoffs;
	int8_t topoffset;
	short coloffsets[8];
} patch_t;

extern uint8_t screenbuffer[];
extern uint8_t* ylookup[];
extern int planewidthlookup[];

extern uint8_t* collumnpointer[];
extern uint32_t ublocksource[];

extern uint8_t update[];

extern int playscreenupdateneeded;
extern int blockupdateneeded;

void V_DrawPatch(int x, int y, patch_t* patch);
void V_FadeOut(int start, int end, int red, int green, int blue, int steps);
void V_FadeIn(int start, int end, int steps, uint8_t* palette);
int V_DrawChar(int x, int y, int ch, font_t* font);
void V_DrawPic(int x, int y, pic_t* pic);
void V_Bar(int xl, int yl, int width, int height, int color);
int V_DrawString(int sx, int sy, char* string, font_t* font);
int V_StringWidth(char* string, font_t* font);
void V_CenterString(int sy, char* string, font_t* font);
void V_Window(int width, int height);
void V_Printf(char* fmt, ...);
void V_MarkUpdateBlock(int x1, int y1, int x2, int y2);

void V_Startup();