
// P_spec.h

/*
===============================================================================

							P_SPEC

===============================================================================
*/

//[ISB] the ordering of this file doesn't seem to match Heretic's. oh well.
//-----------------------------------------------------------------------------
// Floors
//-----------------------------------------------------------------------------

typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int direction;
    int floordestheight;
    int speed;
} floormove_t;

#define FLOORSPEED FRACUNIT

//-----------------------------------------------------------------------------
// Doors
//-----------------------------------------------------------------------------

typedef enum
{
    normal,
    close30ThenOpen,
    close,
    open
} vldoor_e;

typedef struct
{
    thinker_t thinker;
    vldoor_e type;
    sector_t* sector;
    fixed_t topheight;
    fixed_t speed;
    int direction;
    int topwait;
    int topcountdown;
} vldoor_t;

#define VDOORSPEED (FRACUNIT*2)
#define VDOORWAIT 150

//-----------------------------------------------------------------------------
// Platforms
//-----------------------------------------------------------------------------

typedef enum
{
    down = 1,
    up = 0,
    waiting = 2
} plat_e;

typedef enum
{
    downWaitUpStay = 1,
    raise = 0,
    raiseAndChange = 2
} plattype_e;

typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plattype_e type;
} plat_t;

#define PLATSPEED FRACUNIT
#define PLATWAIT 105

typedef struct
{
    int picnum;
    int basepic;
    int numpics;
    int speed;
} anim_t;

typedef struct
{
	char startname[9];
	char endname[9];
	int numflats;
	int speed;
} flatanim_t;

#define	MAXANIMS		32

#define GLOWSPEED 8
#define	STROBEBRIGHT 5
#define	FASTDARK 15
#define SLOWDARK 35

typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;
} lightflash_t;

typedef struct
{
    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int darktime;
    int brighttime;
} lightstrobe_t;
