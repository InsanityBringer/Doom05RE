#pragma once

extern uint8_t screenbuffer[];
extern uint8_t* ylookup[];
extern int planewidthlookup[];

extern uint8_t* collumnpointer[];
extern uint32_t ublocksource[];

extern uint8_t update[];

extern int playscreenupdateneeded;
extern int blockupdateneeded;

void V_Startup();
void V_MarkUpdateBlock(int x1, int y1, int x2, int y2);
void V_DrawPatch(int x, int y, patch_t* patch);
