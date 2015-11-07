// RPG.CPP - Game Console Shell

//////////////////////////////////////////////////////////////////
//
//	Known Bugs
//	
//	1) Lost of surface due to 'Alt-Tab' will cause the system to
//		hang.
//
//////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN  

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream.h> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include <ddraw.h>

#include "RPG.h"

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINXCLASS"  // class name


// container structure for bitmaps .BMP file
/*typedef struct BITMAP_FILE_TAG
        {
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info including the palette
//        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data

        } BITMAP_FILE, *BITMAP_FILE_PTR;
*/

// PROTOTYPES /////////////////////////////////////////////

int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);
int War_Main(void *parms=NULL);

int CreateOffScreen(LPDIRECTDRAWSURFACE &lpddshello, int pixel_width, int pixel_height);

LPDIRECTDRAWCLIPPER DDAttachClipper(LPDIRECTDRAWSURFACE lpdds, int num_rects, LPRECT clip_list);



// WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int ReadMapFromFile(int **&map, LPSTR szFileName, int &width, int &height)
{
	int i;
	FILE *fp;
	if ((fp = fopen(szFileName, "r"))==NULL)
		return(0);

	fscanf(fp, "%d %d", &width, &height);
	
	// allocating memory for the map
	map = new int*[width];
	for(i=0; i<width; i++)
		map[i] = new int[height];

	for (int j=0; j<height; j++)
	{
		fscanf(fp,"\n");
		for (i=0; i<width-1; i++)
			fscanf(fp,"%d ", &map[i][j]);
		fscanf(fp, "%d", &map[width-1][j]);
	}

	fclose(fp);

	return(1);
}

LPDIRECTDRAWCLIPPER
DDAttachClipper(LPDIRECTDRAWSURFACE lpdds,
				int num_rects,
				LPRECT clip_list)
{
	int index;
	LPDIRECTDRAWCLIPPER lpddclipper;
	LPRGNDATA region_data;

	if(FAILED(lpdd->CreateClipper(0,&lpddclipper, NULL)))
		return(NULL);

	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER) +
			num_rects*sizeof(RECT));
	memcpy(region_data->Buffer, clip_list,
		sizeof(RECT)*num_rects);

	region_data->rdh.dwSize = sizeof(RGNDATAHEADER);
	region_data->rdh.iType = RDH_RECTANGLES;
	region_data->rdh.nCount = num_rects;
	region_data->rdh.nRgnSize = num_rects*sizeof(RECT);

	region_data->rdh.rcBound.left = -64000;
	region_data->rdh.rcBound.top = -64000;
	region_data->rdh.rcBound.right = 64000;
	region_data->rdh.rcBound.bottom = 64000;

	for(index=0; index<num_rects; index++)
	{
		if (clip_list[index].left<
			region_data->rdh.rcBound.left)
			region_data->rdh.rcBound.left =
			clip_list[index].left;
		if (clip_list[index].top<
			region_data->rdh.rcBound.top)
			region_data->rdh.rcBound.top =
			clip_list[index].top;
		if (clip_list[index].right>
			region_data->rdh.rcBound.right)
			region_data->rdh.rcBound.right =
			clip_list[index].right;
		if (clip_list[index].bottom>
			region_data->rdh.rcBound.bottom)
			region_data->rdh.rcBound.bottom =
			clip_list[index].bottom;
	}

	if (FAILED(lpddclipper->SetClipList(region_data,0)))
	{
		free(region_data);
		return(NULL);
	}
	if(FAILED(lpdds->SetClipper(lpddclipper)))
	{
		free(region_data);
		return(NULL);
	}
	free(region_data);
	return(lpddclipper);
}	
int LoadImage(LPDIRECTDRAWSURFACE lpwork, LPSTR szImage, int landscapeIndex, int width, int height)
{
	HBITMAP hbm;
	HDC		hdcImage = NULL;
	HDC		hdcSurf = NULL;

	int bReturn = 0;

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	if(FAILED(lpwork->GetSurfaceDesc(&ddsd)))
		goto Exit;

    if ( ( ddsd.ddpfPixelFormat.dwFlags != DDPF_RGB ) ||
         ( ddsd.ddpfPixelFormat.dwRGBBitCount < 16 ) )

    {
        MessageBox(main_window_handle, "Non-palettized RGB mode required.", "",MB_OK);
        goto Exit;        
    }

	hbm = (HBITMAP)LoadImage(NULL, szImage,
		IMAGE_BITMAP, width,
		height, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if(hbm==NULL)
	{
		MessageBox(main_window_handle, "load","hi",MB_OK);
		goto Exit;
	}

	hdcImage = CreateCompatibleDC(NULL);
	SelectObject(hdcImage, hbm);

	if(FAILED(lpwork->GetDC(&hdcSurf)))
	{
		MessageBox(main_window_handle, "hdcsurf","hi",MB_OK);
		goto Exit;
	}

	if(BitBlt(hdcSurf, landscapeIndex*width, 0, width, height,
		hdcImage, 0, 0, SRCCOPY) == FALSE)
	{
		MessageBox(main_window_handle, "blt error","hi",MB_OK);
		goto Exit;
	}
	bReturn = 1;
Exit:

	if(hdcSurf)
		lpwork->ReleaseDC(hdcSurf);
	if(hdcImage)
		DeleteDC(hdcImage);
	if(hbm)
		DeleteObject(hbm);
	return bReturn;
}

int CreateOffScreen(LPDIRECTDRAWSURFACE &lpddshello, int pixel_width, int pixel_height)
{
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = pixel_width;
	ddsd.dwHeight = pixel_height;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_SYSTEMMEMORY;

	if(FAILED(lpdd->CreateSurface(&ddsd,&lpddshello, NULL)))
		return(0);
	return(1);
}

int CheckLandType(int type)
{
	if(worldMap[mainChar[WAI].charPos[X]][mainChar[WAI].charPos[Y]] == type)
		return(1);
	else
		return(0);
}

int PrintStatus(MAIN_CHAR_PTR character, int x, int y)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		return(0);
	}

	TEXTMETRIC tm;
	int h;
	int j = 0;
	int i;
	SetTextColor(hdcSurf, RGB(255,255,255));
	SetBkMode(hdcSurf, TRANSPARENT);
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	TextOut(hdcSurf,x,y+j*h,mainChar[WAI].name,strlen(mainChar[WAI].name));
	j++;
	sprintf(buf,"LV: %d   EXP: %d   Gold: %d   Next LV: %d",character->LV,character->exp,character->gold,character->nextLV);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "HP: %d/%d   MP: %d/%d",character->HP,character->maxHP,character->MP,character->maxMP);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "STR: %d   DEX: %d   INT: %d   SPIRIT: %d   CONS: %d",character->STR,character->DEX,character->INT,character->SPIRIT,character->CONS);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "ATK: %d   DEF: %d",character->ATK,character->DEF);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	i = sprintf(buf, "Weapon: +%dAT  Name: ",(character->weapon)->attackPower);
	sprintf(buf + i, (character->weapon)->name);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	i = sprintf(buf, "Armor: +%dDEF  Name: ",(character->armor)->defPower);
	sprintf(buf + i, (character->armor)->name);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));

	lpddsprimary->ReleaseDC(hdcSurf);

	return(1);
}

int OutMessage(int x, int y)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		return(0);
	}

	TEXTMETRIC tm;
	int h;
	int j = 0;

	SetTextColor(hdcSurf, RGB(255,255,255));
	SetBkMode(hdcSurf, TRANSPARENT);
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	sprintf(buf,"A = Attack");
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "M = Magic");
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "D = Defend");
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "I = Item");
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j = 0;
	x += 200;

	lpddsprimary->ReleaseDC(hdcSurf);

	PrintStatus(mainChar, x, y);

	return(1);
}

int OutText(char *str)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		return(0);
	}

	TEXTMETRIC tm;
	int h;
	int j = 0;

	SetTextColor(hdcSurf, RGB(255,255,255));
	SetBkMode(hdcSurf, TRANSPARENT);
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	TextOut(hdcSurf,20,290,str,strlen(str));


	lpddsprimary->ReleaseDC(hdcSurf);

	return(1);

}
int WarPrintStatus(int x, int y)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		return(0);
	}

	TEXTMETRIC tm;
	int h;
	int j = 0;

	SetTextColor(hdcSurf, RGB(255,255,255));
	SetBkMode(hdcSurf, TRANSPARENT);
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	TextOut(hdcSurf,x,y+j*h,monster[warMonsterIndex].name,strlen(monster[warMonsterIndex].name));
	j++;
	sprintf(buf,"EXP: %d   Gold: %d",monster[warMonsterIndex].exp,monster[warMonsterIndex].gold);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "HP: %d/%d   MP: %d/%d",monster[warMonsterIndex].HP,monster[warMonsterIndex].maxHP,monster[warMonsterIndex].MP,monster[warMonsterIndex].maxMP);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "DEX: %d   INT: %d   SPIRIT: %d",monster[warMonsterIndex].DEX,monster[warMonsterIndex].INT,monster[warMonsterIndex].SPIRIT);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
	j++;
	sprintf(buf, "ATK: %d   DEF: %d",monster[warMonsterIndex].ATK,monster[warMonsterIndex].DEF);
	TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));

	lpddsprimary->ReleaseDC(hdcSurf);

	return(1);
}

int CharacterMove(int direction, int &displayEffect, int &encounter)
{
	if (firstRun)
	{
		mainChar[WAI].charDirection = STAY;
		firstRun = 0;
		return(1);
	}

	int xOrY;
	int plusOrMinus;
	int limit;

	displayEffect = GRASS;
	
	if (direction == UP)
	{
		xOrY = Y;
		plusOrMinus = -1;
		limit = 5;
	} 
	else if (direction == DOWN)
	{
		xOrY = Y;
		plusOrMinus = 1;
		limit = worldMapHeight-7;
	}
	else if (direction == RIGHT)
	{
		xOrY = X;
		plusOrMinus = 1;
		limit = worldMapWidth-9;
	}
	else if(direction == LEFT)
	{
		xOrY = X;
		plusOrMinus = -1;
		limit = 7;
	}

	if(mainChar[WAI].charPos[xOrY] == limit)
		return(0);

	mainChar[WAI].charPos[xOrY]+=plusOrMinus;

	if(CheckLandType(WATER))
	{
		mainChar[WAI].charPos[xOrY]-=plusOrMinus;
		return(0);
	}

	if (mainChar[WAI].state != DEAD)
	{
		encounter = ((rand()%9) == 1);
		if (encounter)
			warMonsterIndex = rand()%2;
		if(CheckLandType(RESTORELAND))
		{
			mainChar[WAI].HP = mainChar[WAI].maxHP;
			displayEffect = RESTORELAND;
		}
		if(CheckLandType(POISONLAND))
		{
			mainChar[WAI].HP--;
			if(mainChar[WAI].HP == 0)
			{
				mainChar[WAI].state = DEAD;
				charIndex = DEADCHAR;
			}
			displayEffect = POISONLAND;
		}
	}

	worldMapOffset[xOrY]+=plusOrMinus;
	mainChar[WAI].charDirection = direction;	

	return(1);
}

int FromWarZoneToWorldZone()
{
	firstRun = 1;

	zone = WORLDZONE;
	if (mainChar[WAI].state == ALIVE)
		charIndex = MAINCHAR;
	if (mainChar[WAI].state == DEAD)
		charIndex = DEADCHAR;
	src_area.bottom = CELL_WIDTH;

	return(1);
}

int CharBeforeMonster(int dex1, int dex2)
{
	int charDex, monsterDex;
	charDex = rand()%6+1*dex1;
	monsterDex = rand()%6+1*dex2;

	return (charDex > monsterDex);
}

int MonsterAction(int index)
{
	if(!monster[index].MP)
		return ATTACK;
	else
		return (rand()%2);
}

int FightOneRound()
{
	int hurt;
	if(warTurn[0] == MONSTERTURN)
	{
		warTurn[1] = warMonsterIndex;
		if (monster[warTurn[1]].warAction == ATTACK)
		{
			if ((hurt = (monster[warTurn[1]].ATK/5*(rand()%5+1) - (mainChar[WAI].DEF/5*(rand()%2+1)))) < 0)
				hurt = 0;
			mainChar[WAI].HP-=hurt;
			sprintf(buf, "%s hits %s by %d points", monster[warTurn[1]].name, mainChar[WAI].name, hurt);

			if(mainChar[WAI].HP <= 0)
			{
				mainChar[WAI].HP = 0;
				mainChar[WAI].state = DEAD;
				warOver = MONSTERWIN;
			}
		}
		warTurn[0] = CHARTURN;
	}
	else if (warTurn[0] == CHARTURN)
	{
		warTurn[2] = warMonsterIndex;
		if (mainChar[WAI].warAction == ATTACK)
		{
			if ((hurt = (mainChar[WAI].ATK/5*(rand()%5+1) - (monster[warTurn[2]].DEF/5*(rand()%2+1)))) < 0)
				hurt = 0;
			monster[warTurn[2]].HP-=hurt;
			sprintf(buf, "%s hits %s by %d points", mainChar[WAI].name, monster[warTurn[1]].name, hurt);
			if(monster[warTurn[2]].HP <= 0)
			{
				monster[warTurn[2]].HP = 0;
				monster[warTurn[2]].state = DEAD;
				warOver = CHARWIN;
			}
		}
		else if (mainChar[WAI].warAction == DEFEND)
		{
			mainChar[WAI].DEF = (mainChar[WAI].DEF)<<1;
			sprintf(buf, "%s's defense doubles!", mainChar[WAI].name);
		}
		warTurn[0] = MONSTERTURN;
	}
	if ((hitCount == 1) && (mainChar[WAI].warAction == DEFEND))
		mainChar[WAI].DEF = (mainChar[WAI].DEF)>>1;

	OutText(buf);
	Sleep(30);
	return(1);
}

int War_Main(void *parms)
{
	if(warOver)
	{
		if (warOver == CHARWIN)
		{
			monster[warMonsterIndex].HP = monster[warMonsterIndex].maxHP;
			mainChar[WAI].exp += monster[warMonsterIndex].exp;
			mainChar[WAI].nextLV -= monster[warMonsterIndex].exp;
			if (mainChar[WAI].nextLV <= 0)
			{
				mainChar[WAI].LV++;
				OutText("LEVEL UP!");
			}
			mainChar[WAI].gold += monster[warMonsterIndex].gold;
		}
		warOver = NOTFINISHED;
		hitCount = 0;
		FromWarZoneToWorldZone();
		return(1);
	}

	if (!startBattle)
	{
	while(!hitCount)
	{
		if (KEY_DOWN('A'))
		{
			hitCount = 2;
			mainChar[WAI].warAction = ATTACK;
			monster[warMonsterIndex].warAction = MonsterAction(warMonsterIndex);
			warTurn[0] = CharBeforeMonster(mainChar[WAI].DEX, monster[warMonsterIndex].DEX);
		}
/*		else if (KEY_DOWN('M'))
		{
			mainChar[WAI].warAction = MAGIC;
			monster[warMonsterIndex].warAction = MonsterAction(warMonsterIndex);
			warTurn[0] = CharBeforeMonster(mainChar[WAI].DEX, monster[warMonsterIndex].DEX);
		}
*/		else if (KEY_DOWN('D'))
		{
			hitCount = 2;
			mainChar[WAI].warAction = DEFEND;
			monster[warMonsterIndex].warAction = MonsterAction(warMonsterIndex);
			warTurn[0] = CharBeforeMonster(mainChar[WAI].DEX, monster[warMonsterIndex].DEX);
		}
/*		else if (KEY_DOWN('I'))
		{
			mainChar[WAI].warAction = ITEM;
			monster[warMonsterIndex].warAction = MonsterAction(warMonsterIndex);
			warTurn[0] = CharBeforeMonster(mainChar[WAI].DEX, monster[warMonsterIndex].DEX);
		}
*/	}
	}
	else
	{
		startBattle = 0;
	}

	if(hitCount)
	{
		FightOneRound();
		hitCount--;
	}
	
	if (KEY_DOWN(VK_DOWN))
	{
		FromWarZoneToWorldZone();
		return(1);
	}
/*	else
	{
		if (!startBattle)
			return(1);
		startBattle = 0;
	}
*/
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = _RGB24BIT(0, 0, 0);

	lpddsback->Blt(NULL, 
		NULL, 
		NULL, 
		DDBLT_COLORFILL | DDBLT_WAIT, 
		&ddbltfx);

	warSrcArea.left = BACKGROUNDWIDTH*backgroundIndex;
	warSrcArea.right = BACKGROUNDWIDTH+warSrcArea.left;

	lpddsback->Blt(&fill_area,
		lpddsbackground,
		&warSrcArea,
		DDBLT_WAIT,
		NULL);

	lpddsback->Blt(&(mainChar[WAI].warCharArea),
		mainChar[WAI].lpddsbattlechar,
		&src_area,
		DDBLT_WAIT|DDBLT_KEYSRC,
		NULL);

	lpddsback->Blt(&(monster[warMonsterIndex].warMonsterArea),
		monster[warMonsterIndex].lpddswarmonster,
		&(monster[warMonsterIndex].srcArea),
		DDBLT_WAIT|DDBLT_KEYSRC,
		NULL);

	while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);

	OutMessage(20,330);
	if(!WarPrintStatus(260, 20))
		return (0);

	Sleep(30);
	return(1);
}

int FromWorldZoneToWarZone()
{
	startBattle = 1;
	zone = WARZONE;
	if (mainChar[WAI].state == ALIVE)
		warCharIndex = WARMAINCHAR;
	if (mainChar[WAI].state == DEAD)
		warCharIndex = WARDEADCHAR;
	src_area.left = (warCharIndex<<3)+(warCharIndex<<5);
	src_area.right = src_area.left+WARCHARWIDTH;
	src_area.bottom = WARCHARHEIGHT;

	fill_area.left = 0;
	fill_area.top = 0;
	fill_area.right = 640;
	fill_area.bottom = 320;
	
	if (CheckLandType(GRASS))
		backgroundIndex = GRASSBG;
	else if(CheckLandType(VILLAGE))
		backgroundIndex = GRASSBG;
	else if(CheckLandType(POISONLAND))
		backgroundIndex = POISONLANDBG;
	else if(CheckLandType(RESTORELAND))
		backgroundIndex = RESTORELANDBG;
	else
		return(0);

	return(1);
}

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

// your code goes here
	int strangeTemp[2] = {0,0};
	int specialEffect = GRASS;
	int encounter = 0;

	if (KEY_DOWN(VK_ESCAPE))
		PostMessage(main_window_handle, WM_DESTROY,0,0);

	if (KEY_DOWN('S'))
	{
		if(!PrintStatus(mainChar, 40, 40))
			return (0);
	}

	if (KEY_DOWN(VK_UP))
	{
		if(!CharacterMove(UP, specialEffect, encounter))
			return(1);
	}
	
	else if(KEY_DOWN(VK_DOWN))
	{
		if(!CharacterMove(DOWN, specialEffect, encounter))
			return(1);
	}
	else if(KEY_DOWN(VK_RIGHT))
	{
		if(!CharacterMove(RIGHT, specialEffect, encounter))
			return(1);
	}
	else if(KEY_DOWN(VK_LEFT))
	{
		if(!CharacterMove(LEFT, specialEffect, encounter))
			return(1);
	}
	else
	{
		mainChar[WAI].charDirection = STAY;
		if (!firstRun)
			return(1);
		specialEffect = GRASS;
		firstRun = 0;
	}

	//////////////////////////////////
/*	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	lpddsback->Lock(NULL, &ddsd, 
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	UCHAR *video_buffer = NULL;
	video_buffer = (UCHAR*)ddsd.lpSurface;

	video_buffer[300*3 + 300*640*3] = _RGB24BIT(30,90,255);
	lpddsback->Unlock(ddsd.lpSurface);
*/
	///////////////////////////////////
/*	RECT fill_area;
	memset(&ddbltfx,0,sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = _RGB24BIT(0,255,0);

	fill_area.top = 100;
	fill_area.left = 400;
	fill_area.bottom = 200;
	fill_area.right = 500;

	lpddsback->Blt(&fill_area, 
		NULL, 
		NULL, 
		DDBLT_COLORFILL | DDBLT_WAIT, 
		&ddbltfx);
*/
	int verticalDisplayOffset = 0;
	int initVertDisplay = 0;
	int horizontalDisplayOffset = 0;
	int initHoriDisplay = 0;	

	switch(mainChar[WAI].charDirection) {
	case DOWN:
//		if(mainChar[WAI].charDirection == DOWN) 
		strangeTemp[Y] = -1;
		verticalDisplayOffset = -DISPLAY_OFFAMOUNT;
		initVertDisplay = -DISPLAY_OFFAMOUNT;
		break;
	case UP:
//			if(mainChar[WAI].charDirection == UP)
//		strangeTemp = 0;
		verticalDisplayOffset = DISPLAY_OFFAMOUNT;
		initVertDisplay = verticalDisplayOffset-CELL_WIDTH;
		break;
	case LEFT:
//		if(mainChar[WAI].charDirection == LEFT)
//		strangeTemp = 0;
		horizontalDisplayOffset = DISPLAY_OFFAMOUNT;
		initHoriDisplay = horizontalDisplayOffset-CELL_WIDTH;
		break;
	case RIGHT:
//		if(mainChar[WAI].charDirection == RIGHT
		strangeTemp[X] = -1;
		horizontalDisplayOffset = -DISPLAY_OFFAMOUNT;
		initHoriDisplay = -DISPLAY_OFFAMOUNT;
		break;
	case STAY:
//		strangeTemp = 0;
		break;
	default:
		break;
	}
//	RECT fill_area;

	for (int rep = 0; rep < repCount; rep++)
	{	
/*		if (rep == (repCount-1))
			strangeTemp[0] = strangeTemp[1] = 0;
*/
//		RECT fill_area;
/*		memset(&ddbltfx,0,sizeof(ddbltfx));
		ddbltfx.dwSize = sizeof(ddbltfx);
		ddbltfx.dwFillColor = _RGB24BIT(0,0,0);
*/
/*		fill_area.top = 100;
		fill_area.left = 400;
		fill_area.bottom = 200;
		fill_area.right = 500;
*/
		 
/*		lpddsback->Blt(NULL, 
			NULL, 
			NULL, 
			DDBLT_COLORFILL | DDBLT_WAIT, 
			&ddbltfx);
*/
		if(((rep == (repCount-2))||(rep == repCount-3)) && (specialEffect!=GRASS))
		{
			if(specialEffect == POISONLAND)
			{
				ZeroMemory(&ddbltfx, sizeof(ddbltfx));
				ddbltfx.dwSize = sizeof(ddbltfx);
				ddbltfx.dwFillColor = _RGB24BIT(215,34,38);

				lpddsback->Blt(NULL, 
				NULL, 
				NULL, 
				DDBLT_COLORFILL | DDBLT_WAIT, 
				&ddbltfx);
			}
			else if(specialEffect == RESTORELAND)
			{
				ZeroMemory(&ddbltfx, sizeof(ddbltfx));
				ddbltfx.dwSize = sizeof(ddbltfx);
				ddbltfx.dwFillColor = _RGB24BIT(35, 122, 228);

				lpddsback->Blt(NULL, 
				NULL, 
				NULL, 
				DDBLT_COLORFILL | DDBLT_WAIT, 
				&ddbltfx);
			}
		}
		else
		{
			for (int i=initVertDisplay; i<SCREEN_HEIGHT; i+=CELL_WIDTH)
			{
				for (int j=initHoriDisplay; j<SCREEN_WIDTH; j+=CELL_WIDTH)
				{
					fill_area.top = i;
					fill_area.left = j;
					fill_area.bottom = i+CELL_WIDTH;
					fill_area.right = j+CELL_WIDTH;
					
					grassIndex = worldMap[worldMapOffset[0]+(j-initHoriDisplay)/CELL_WIDTH+strangeTemp[0]][worldMapOffset[1]+(i-initVertDisplay)/CELL_WIDTH+strangeTemp[1]];
					src_area.left = (grassIndex<<3)+(grassIndex<<5);
					src_area.right = src_area.left+CELL_WIDTH;
		
					lpddsback->Blt(&fill_area,
						lpddsgrass,
						&src_area,
						DDBLT_WAIT,
						NULL);
				}
			}
/*		fill_area.top = 200;
		fill_area.left = 280;
		fill_area.bottom = 240;
		fill_area.right = 320;
*/
			src_area.left = (charIndex<<3)+(charIndex<<5);
			src_area.right = src_area.left+CELL_WIDTH;
	
			lpddsback->Blt(&(mainChar[WAI].worldCharArea),//280,200,
			mainChar[WAI].lpddscharacter,
			&src_area,
			DDBLT_WAIT|DDBLT_KEYSRC,
			//DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
			NULL);
		}

		initHoriDisplay+=horizontalDisplayOffset;
		initVertDisplay+=verticalDisplayOffset;
		//////////////////////////////////////
//		lpdd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0); // slows down so much with this

		while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);

//		Sleep(500);
	}

	if(encounter)
	{
		FromWorldZoneToWarZone();
	}

	Sleep(30);
	// return success
	return(1);
}
 // end Game_Main

int WeaponInit()
{
	///////////////////////////////////////////////////////
	// SET weapon attributes
	///////////////////////////////////////////////////////
	weaponList[BAREHAND].attackPower = 0;
	sprintf(weaponList[BAREHAND].name, "");
	weaponList[CLUB].attackPower = 2;
	sprintf(weaponList[CLUB].name, "Club");

	return(1);
}

int ArmorInit()
{
	///////////////////////////////////////////////////////
	// SET armor attributes
	///////////////////////////////////////////////////////
	armorList[NOARMOR].defPower = 0;
	sprintf(armorList[NOARMOR].name, "");
	armorList[CLOTHCLOTHES].defPower = 2;
	sprintf(armorList[CLOTHCLOTHES].name, "Cloth Clothes");

	return(1);
}

int CharacterInit()
{
	///////////////////////////////////////////////////////
	// SET main character attributes
	///////////////////////////////////////////////////////
	mainChar = new MAIN_CHAR[NUMCHAR];

	sprintf(mainChar[WAI].name, "Wai");
	mainChar[WAI].charPos[X] = 39;
	mainChar[WAI].charPos[Y] = 29;
	mainChar[WAI].charDirection = STAY;

	mainChar[WAI].state = ALIVE;
	mainChar[WAI].LV = 1;
	mainChar[WAI].exp = 0;
	mainChar[WAI].gold = 80;
	mainChar[WAI].nextLV = 6;

	mainChar[WAI].maxHP = 15;
	mainChar[WAI].HP = 15;
	mainChar[WAI].maxMP = 3;
	mainChar[WAI].MP = 3;
	mainChar[WAI].STR = 12;
	mainChar[WAI].DEX = 10;
	mainChar[WAI].INT = 8;
	mainChar[WAI].SPIRIT = 8;
	mainChar[WAI].CONS = 10;

//	mainChar[WAI].weapon = new WEAPON;
	mainChar[WAI].weapon = &(weaponList[CLUB]);
	mainChar[WAI].armor = &(armorList[CLOTHCLOTHES]);

	mainChar[WAI].ATK = mainChar[WAI].STR/2 + (mainChar[WAI].weapon)->attackPower;
	mainChar[WAI].DEF = mainChar[WAI].DEX/2 + (mainChar[WAI].armor)->defPower;

	(mainChar[WAI].worldCharArea).top = 200;
	(mainChar[WAI].worldCharArea).left = 280;
	(mainChar[WAI].worldCharArea).bottom = 240;
	(mainChar[WAI].worldCharArea).right = 320;

	(mainChar[WAI].warCharArea).top = 200;
	(mainChar[WAI].warCharArea).left = 560;
	(mainChar[WAI].warCharArea).bottom = 280;
	(mainChar[WAI].warCharArea).right = 600;

	return(1);
}

int MonsterInit()
{
	///////////////////////////////////////////////////////
	// SET monsters attributes
	///////////////////////////////////////////////////////
	monster = new MONSTER[NUMMONSTER];

	sprintf(monster[MECAR].name, "Car of Me");

	monster[MECAR].state = ALIVE;

	monster[MECAR].exp = 1;
	monster[MECAR].gold = 5;

	monster[MECAR].maxHP = 7;
	monster[MECAR].HP = 7;
	monster[MECAR].maxMP = 0;
	monster[MECAR].MP = 0;
	monster[MECAR].DEX = 8;
	monster[MECAR].INT = 2;
	monster[MECAR].SPIRIT = 1;

	monster[MECAR].ATK = 7;
	monster[MECAR].DEF = 3;

	(monster[MECAR].warMonsterArea).top = 200;
	(monster[MECAR].warMonsterArea).left = 80;
	(monster[MECAR].warMonsterArea).bottom = 280;
	(monster[MECAR].warMonsterArea).right = 120;

	sprintf(monster[BIRD].name, "Killer Bird");

	monster[BIRD].state = ALIVE;

	monster[BIRD].exp = 1;
	monster[BIRD].gold = 7;

	monster[BIRD].maxHP = 9;
	monster[BIRD].HP = 9;
	monster[BIRD].maxMP = 0;
	monster[BIRD].MP = 0;
	monster[BIRD].DEX = 9;
	monster[BIRD].INT = 2;
	monster[BIRD].SPIRIT = 1;

	monster[BIRD].ATK = 6;
	monster[BIRD].DEF = 3;

	(monster[BIRD].warMonsterArea).top = 100;
	(monster[BIRD].warMonsterArea).left = 80;
	(monster[BIRD].warMonsterArea).bottom = 180;
	(monster[BIRD].warMonsterArea).right = 120;

	for(int i=0; i<NUMMONSTER; i++)
	{
		monster[i].srcArea.left = 0;
		monster[i].srcArea.right = WARCHARWIDTH;
		monster[i].srcArea.top = 0;
		monster[i].srcArea.bottom = WARCHARHEIGHT;
	}
	return(1);
}


int InitBitmap()
{
	if(!LoadImage(mainChar[WAI].lpddscharacter, "..\\Bitmaps\\Characters\\RPGmainChar.bmp",MAINCHAR,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(mainChar[WAI].lpddscharacter, "..\\Bitmaps\\Characters\\deadChar.bmp",DEADCHAR,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(mainChar[WAI].lpddsbattlechar, "..\\Bitmaps\\WarCharacters\\mainChar.bmp",WARMAINCHAR,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[WAI].lpddsbattlechar, "..\\Bitmaps\\WarCharacters\\mainChar.bmp",WARDEADCHAR,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);

	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\grassLand.bmp",GRASS,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\water.bmp",WATER,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\village.bmp",VILLAGE,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\poisonLand.bmp",POISONLAND,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\restoreLand.bmp",RESTORELAND,CELL_WIDTH,CELL_WIDTH))
		return(0);

	if(!LoadImage(lpddsbackground, "..\\Bitmaps\\Backgrounds\\grass.bmp",GRASSBG,BACKGROUNDWIDTH,BACKGROUNDHEIGHT))
		return(0);
	if(!LoadImage(lpddsbackground, "..\\Bitmaps\\Backgrounds\\try.bmp",POISONLANDBG,BACKGROUNDWIDTH,BACKGROUNDHEIGHT))
		return(0);
	if(!LoadImage(lpddsbackground, "..\\Bitmaps\\Backgrounds\\restore.bmp",RESTORELANDBG,BACKGROUNDWIDTH,BACKGROUNDHEIGHT))
		return(0);

	if(!LoadImage(monster[MECAR].lpddswarmonster, "..\\Bitmaps\\Monsters\\meCar.bmp",NORMALMONSTER,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);
	if(!LoadImage(monster[BIRD].lpddswarmonster, "..\\Bitmaps\\Monsters\\bird.bmp",NORMALMONSTER,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);
	
	return(1);
}

///////////////////////////////////////////////////////////////////
//
//	Game_Init()
//
///////////////////////////////////////////////////////////////////
int Game_Init(void *parms)
{
	// initialize variables
	WeaponInit();

	ArmorInit();
	
	CharacterInit();

	MonsterInit();

	///////////////////////////////////////////////////////
	// Initialize map data
	///////////////////////////////////////////////////////
	worldMapOffset[0] = mainChar[WAI].charPos[X]-7;
	worldMapOffset[1] = mainChar[WAI].charPos[Y]-5;

// this function is where you do all the initialization 
// for your game
	
	// try outputing the map to a file
	if(!ReadMapFromFile(worldMap, szWorldMapFile, worldMapWidth, worldMapHeight))
		return(0);


// your code goes here
	if(FAILED(DirectDrawCreate(NULL,&lpdd,NULL)))
		return(0);
	if(FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
		DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
		return(0);
	if(FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BIT)))
		return(0);
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = 
		DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;

	if(FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary,NULL)))
		return(0);

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	if(lpddsprimary->GetAttachedSurface(&ddscaps, &lpddsback)!=DD_OK)
		return(0);

	if(!CreateOffScreen(lpddsgrass, NUM_OF_LANDSCAPES*CELL_WIDTH, CELL_WIDTH))
		return(0);
	if(!CreateOffScreen(lpddsbackground, NUMOFBACKGROUNDS*BACKGROUNDWIDTH, BACKGROUNDHEIGHT))
		return(0);

	if(!CreateOffScreen(mainChar[WAI].lpddscharacter, NUMCHARMAPS*CELL_WIDTH, CELL_WIDTH))
		return(0);
	if(!CreateOffScreen(mainChar[WAI].lpddsbattlechar, NUMWARCHARMAPS*WARCHARWIDTH, WARCHARHEIGHT))
		return(0);
	if(!CreateOffScreen(monster[MECAR].lpddswarmonster, NUMWARCHARMAPS*WARCHARWIDTH, WARCHARHEIGHT))
		return(0);
	if(!CreateOffScreen(monster[BIRD].lpddswarmonster, NUMWARCHARMAPS*WARCHARWIDTH, WARCHARHEIGHT))
		return(0);

	DDCOLORKEY	key;
	key.dwColorSpaceLowValue = _RGB24BIT(1,1,1);//1;
//	key.dwColorSpaceHighValue = 1;//_RGB24BIT(255,255,255);//(1,1,1);
	(mainChar[WAI].lpddscharacter)->SetColorKey(DDCKEY_SRCBLT, &key);
	(mainChar[WAI].lpddsbattlechar)->SetColorKey(DDCKEY_SRCBLT, &key);
	(monster[MECAR].lpddswarmonster)->SetColorKey(DDCKEY_SRCBLT, &key);
	(monster[BIRD].lpddswarmonster)->SetColorKey(DDCKEY_SRCBLT, &key);

	/////////////// LOADING BITMAPS //////////////////////////
	if(!InitBitmap())
		return(0);

	RECT cliplist[1] = {{0,0,SCREEN_WIDTH, SCREEN_HEIGHT}};
	lpddclip = DDAttachClipper(lpddsback, 1, cliplist);
	if(lpddclip==NULL)
		return(0);

	src_area.top = 0;
	src_area.bottom = CELL_WIDTH;
	warSrcArea.top = 0;
	warSrcArea.bottom = BACKGROUNDHEIGHT;
	// return success
	return(1);
} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// your code goes here
	if (mainChar[WAI].lpddscharacter)
		(mainChar[WAI].lpddscharacter)->Release();
	if (mainChar[WAI].lpddsbattlechar)
		(mainChar[WAI].lpddsbattlechar)->Release();
	for (int i=0; i<NUMMONSTER; i++)
	{
		if (monster[i].lpddswarmonster)
			(monster[i].lpddswarmonster)->Release();
	}
	if (lpddsbackground)
		lpddsbackground->Release();
	if (lpddsgrass)
		lpddsgrass->Release();
	if (lpddsprimary)
		lpddsprimary->Release();
	if (lpdd!=NULL)
		lpdd->Release();
// return success
	return(1);
} // end Game_Shutdown


// FUNCTIONS //////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT	ps;		   // used in WM_PAINT
HDC			hdc;	   // handle to a device context

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
		return(0);
		} break;

    case WM_PAINT:
         {
         // start painting
         hdc = BeginPaint(hwnd,&ps);

         // end painting
         EndPaint(hwnd,&ps);
         return(0);
        } break;

	case WM_DESTROY: 
		{
		// kill the application			
		PostQuitMessage(0);
		return(0);
		} break;

	default:break;

    } // end switch

// process any messages that we didn't take care of 
return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{

WNDCLASS winclass;	// this will hold the class we create
HWND	 hwnd;		// generic window handle
MSG		 msg;		// generic message
HDC      hdc;       // generic dc
PAINTSTRUCT ps;     // generic paintstruct

// first fill in the window class stucture
winclass.style			= CS_DBLCLKS | CS_OWNDC | 
                          CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hinstance;
winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
winclass.hbrBackground	= GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= NULL; 
winclass.lpszClassName	= WINDOW_CLASS_NAME;

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
						  "RPG V2.0",	 // title
						  WS_POPUP | WS_VISIBLE,
					 	  0,0,	   // x,y
						  SCREEN_WIDTH,  // width
                          SCREEN_HEIGHT, // height
						  NULL,	   // handle to parent 
						  NULL,	   // handle to menu
						  hinstance,// instance
						  NULL)))	// creation parms
return(0);

//ShowCursor(FALSE);
// save the window handle and instance in a global
main_window_handle = hwnd;
main_instance      = hinstance;

// perform all game console specific initialization
if(!Game_Init())
	MessageBox(main_window_handle, "Wrong!", "MB Test",MB_OK);
else
	MessageBox(main_window_handle, "Right!", "MB Test", MB_OK);

// enter main event loop
while(1)
	{
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{ 
		// test if this is a quit
        if (msg.message == WM_QUIT)
           break;
	
		// translate any accelerator keys
		TranslateMessage(&msg);

		// send the message to the window proc
		DispatchMessage(&msg);
		} // end if
    
    // main game processing goes here
	if(zone == WORLDZONE)
	    Game_Main();
	else if(zone == WARZONE)
		War_Main();

	} // end while

// shutdown game and release all resources
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain


