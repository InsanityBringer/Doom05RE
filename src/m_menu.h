#pragma once

//Dependent on the playsim just for framecmd_t
#include "p_play.h"


//menu
//[ISB] Does the menu code really own the hudfont, or is it owned by video and set by menu?
extern font_t* hudfont;
void M_Startup();
void M_DrawSelf();
void M_CheckInput(framecmd_t* cmd);
void M_ClearMenus();
void M_StartControlPanel();
