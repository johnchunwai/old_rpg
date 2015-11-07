#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BIT	  24

#define CELL_WIDTH	  40
#define BACKGROUNDWIDTH		320
#define BACKGROUNDHEIGHT	160
#define WARCHARWIDTH		40
#define WARCHARHEIGHT		80

#define WARMONSTERWIDTH		40
#define WARMONSTERHEIGHT	80


// bitmap defines
#define BITMAP_ID            0x4D42 // universal id for a bitmap

#define UP		1
#define DOWN	2
#define LEFT	3
#define RIGHT	4
#define STAY	5

#define DISPLAY_OFFAMOUNT	8

#define NUMCHARMAPS	2
#define MAINCHAR	0
#define DEADCHAR	1

#define NUMWARCHARMAPS 2
#define WARMAINCHAR	0
#define WARDEADCHAR	1

#define MONSTER1TURN	0
#define MONSTER2TURN	1
#define MONSTER3TURN	2
#define CHARTURN		3

#define NOONE		2	// used by WarMain's turn[]

#define NUM_OF_LANDSCAPES	5
#define GRASS		0
#define WATER		4
#define VILLAGE		3
#define POISONLAND	1
#define RESTORELAND	2

#define NUMOFBACKGROUNDS	3
#define GRASSBG			0
#define POISONLANDBG	1
#define RESTORELANDBG	2

#define ALIVE	1
#define DEAD	2

#define NUMWEAPON	20
#define BAREHAND	0
#define CLUB		1

#define NUMARMOR		20
#define NOARMOR			0
#define CLOTHCLOTHES	1

#define X	0
#define Y	1

#define ATTACK	0
#define MAGIC	1
#define DEFEND	2
#define	ITEM	3

#define	WORLDZONE	0
#define WARZONE		1

#define MAXMONSTERINWAR	3
#define NUMMONSTER	2
#define MECAR	0
#define BIRD	1
#define DEADMONSTER	5

#define NUMCHAR	1
#define	WAI		0

#define NUMMONSTERMAPS	1
#define NORMALMONSTER	0

#define NOTFINISHED	0
#define CHARWIN		1
#define	MONSTERWIN	2


// MACROS /////////////////////////////////////////////////

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define _RGB24BIT(r, g ,b)  ((b&255)+((g&255)<<8)+((r&255)<<16))

typedef struct WEAPON_TAG
{
	int attackPower;
	char name[80];
} WEAPON, *WEAPON_PTR;

typedef struct ARMOR_TAG
{
	int defPower;
	char name[80];
} ARMOR, *ARMOR_PTR;

typedef struct MAIN_CHAR_TAG
{
	char name[30];
//	int id; // who this guy is
			// for weapon and armor
	int state;	// ALIVE, DEAD

	int LV;
	DWORD exp;
	DWORD gold;
	DWORD nextLV;	// exp needed for next LV

	int HP, maxHP;
	int MP, maxMP;
	int STR;
	int DEX;
	int INT;
	int SPIRIT;
	int CONS;	// constitution of character
	
	int ATK;
	int DEF;	

	int charPos[2];	// world position of the character
		
	int charDirection;

	WEAPON_PTR weapon;

	ARMOR_PTR armor;

	LPDIRECTDRAWSURFACE	lpddscharacter;	// offscreen surface associated with character
	LPDIRECTDRAWSURFACE lpddsbattlechar;	// character in battle

	RECT worldCharArea;
	RECT warCharArea;

	int warAction[3]; //[0] what action; [1] whom to apply; [2] with what;

}	MAIN_CHAR, *MAIN_CHAR_PTR;

typedef struct MONSTER_TAG
{
	char name[30];
//	int id; // who this guy is
			// for weapon and armor
//	int LV;
	DWORD exp;
	DWORD gold;
//	DWORD nextLV;	// exp needed for next LV

	int maxHP;
	int maxMP;
//	int STR;
	int DEX;
	int INT;
	int SPIRIT;
//	int CONS;	// constitution of character
	
	int ATK;
	int DEF;	

//	int charPos[2];	// world position of the character
		
//	int charDirection;

//	WEAPON_PTR weapon;

//	ARMOR_PTR armor;

	LPDIRECTDRAWSURFACE	lpddswarmonster;	// offscreen surface associated with character
//	LPDIRECTDRAWSURFACE lpddsbattlechar;	// character in battle

	RECT srcArea;
//	RECT worldCharArea;
	RECT warMonsterArea;

	int HP;
	int MP;
	int state;	// ALIVE, DEAD
	int warAction[3];

}	MONSTER, *MONSTER_PTR;


// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance
char buffer[80];                // used to print text

LPDIRECTDRAW	lpdd;
DDSURFACEDESC	ddsd;
DDSCAPS			ddscaps;
LPDIRECTDRAWSURFACE lpddsprimary,
				lpddsback;
LPDIRECTDRAWSURFACE lpddsgrass;
LPDIRECTDRAWSURFACE lpddsbackground;
LPDIRECTDRAWSURFACE lpddswarmonster[NUMMONSTER];

LPDIRECTDRAWCLIPPER	lpddclip;
DDBLTFX			ddbltfx;

RECT fill_area;
RECT src_area;

RECT warSrcArea;

DWORD grassIndex = GRASS;
DWORD backgroundIndex = GRASSBG;
DWORD charIndex = MAINCHAR;
DWORD warCharIndex;
DWORD warMonsterIndex[3];

char szWorldMapFile[80] = "..\\Map\\worldMap.dat";


MONSTER **monster;
MAIN_CHAR_PTR mainChar;
WEAPON weaponList[20];	// weapon list
ARMOR armorList[20];	// armor list

const int repCount = CELL_WIDTH/DISPLAY_OFFAMOUNT;

int worldMapOffset[2];
int firstRun = 1;
int startBattle = 1;
int worldMapWidth, worldMapHeight;
int **worldMap;

char buf[500];

int zone = WORLDZONE;

int hitCount = 0;	// 0: stop fighting; 1: 1 hit left; 2: 2 hits left
int warTurn[4];		// whose turn on [i]th turnCount respectively
int turnCount = 0;	// which turn in the round
int warOver = 0;	// 0 not finished. 1 char war. 2 monster win.
RECT warMessageBox;
int numMonsterInWar;
int numMonsterDead;		// number of monster died in the war