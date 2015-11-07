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
#include <iostream> // include important C/C++ stuff
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


// PROTOTYPES /////////////////////////////////////////////

int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);
int War_Main(void *parms=NULL);

int CreateOffScreen(LPDIRECTDRAWSURFACE &lpddshello, int pixel_width, int pixel_height);

LPDIRECTDRAWCLIPPER DDAttachClipper(LPDIRECTDRAWSURFACE lpdds, int num_rects, LPRECT clip_list);



// WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int ReadMapFromFile(MAPTILE **&map, LPSTR szFileName, int &width, int &height)
{
	int i;
	int j;
	FILE *fp;
	if ((fp = fopen(szFileName, "r"))==NULL)
		return(0);

	fscanf(fp, "%d %d", &width, &height);
	
	// allocating memory for the map
	map = new MAPTILE*[width];
	for(i=0; i<width; i++)
		map[i] = new MAPTILE[height];

	for (j=0; j<height; j++)
	{
		fscanf(fp,"\n");
		for (i=0; i<width-1; i++)
		{
			fscanf(fp,"%d,%d,%d,%d", &(map[i][j].landtype),&(map[i][j].objtype),&(map[i][j].index),&(map[i][j].numFrame));
			// initialize the curFrame variable
			map[i][j].curFrame = 0;
			if (map[i][j].landtype == WATER)
				map[i][j].frameSequence = frameSequenceWater;
			else
				map[i][j].frameSequence = frameSequenceNormal;
		}
		fscanf(fp, "%d,%d,%d,%d", &(map[width-1][j].landtype),&(map[width-1][j].objtype),&(map[width-1][j].index),&(map[width-1][j].numFrame));
		// initialize the curFrame variable
		map[i][j].curFrame = 0;
		if (map[i][j].landtype == WATER)
			map[i][j].frameSequence = frameSequenceWater;
		else
			map[i][j].frameSequence = frameSequenceNormal;
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

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	if(FAILED(lpdd->CreateSurface(&ddsd,&lpddshello, NULL)))
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
		if(FAILED(lpdd->CreateSurface(&ddsd,&lpddshello, NULL)))
			return(0);
	}
	return(1);
}

int CreateOffScreenSystemMemory(LPDIRECTDRAWSURFACE &lpddshello, int pixel_width, int pixel_height)
{
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = pixel_width;
	ddsd.dwHeight = pixel_height;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;

	if(FAILED(lpdd->CreateSurface(&ddsd,&lpddshello, NULL)))
		return(0);

	return(1);
}

int CheckLandType(int x, int y, int type)
{
	if(worldMap[x][y].landtype == type)
		return(1);
	else
		return(0);
}

int PrintStatus(int x, int y)
{

	MAIN_CHAR_PTR character;
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		if (hdcSurf)
			lpddsprimary->ReleaseDC(hdcSurf);

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
	for (int m=0; m<curNumChar; m++)
	{
		character = &(mainChar[m]);
		TextOut(hdcSurf,x,y+j*h,character->name,strlen(character->name));
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
		x+=250;
		j =0;
	}
	lpddsprimary->ReleaseDC(hdcSurf);

	return(1);
}

int OutMessage(int x, int y)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		if (hdcSurf)
			lpddsprimary->ReleaseDC(hdcSurf);

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

	PrintStatus(/*mainChar,*/ x, y);

	return(1);
}

int OutText(char *str, int x, int y)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		if (hdcSurf)
			lpddsprimary->ReleaseDC(hdcSurf);

		return(0);
	}

	TEXTMETRIC tm;
	int h;
	int j = 0;

	SetTextColor(hdcSurf, RGB(255,255,255));
	SetBkMode(hdcSurf, TRANSPARENT);
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	TextOut(hdcSurf,x,y,str,strlen(str));


	lpddsprimary->ReleaseDC(hdcSurf);

	return(1);

}

// this prints numToPrint Monsters' statuses on screen
int WarPrintStatus(int x, int y)
{
	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		if (hdcSurf)
			lpddsprimary->ReleaseDC(hdcSurf);
		return(0);
	}

	TEXTMETRIC tm;
	int h;
	int j = 0;

	SetTextColor(hdcSurf, RGB(255,255,255));
	SetBkMode(hdcSurf, TRANSPARENT);
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	for (int i=0; i<numMonsterInWar; i++)
	{
		if (warMonsterIndex[i]!=DEADMONSTER)
		{
		TextOut(hdcSurf,x,y+j*h,monster[warMonsterIndex[i]][i].name,strlen(monster[warMonsterIndex[i]][i].name));
		j++;
		sprintf(buf,"EXP: %d   Gold: %d",monster[warMonsterIndex[i]][i].exp,monster[warMonsterIndex[i]][i].gold);
		TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
		j++;
		sprintf(buf, "HP: %d/%d   MP: %d/%d",monster[warMonsterIndex[i]][i].HP,monster[warMonsterIndex[i]][i].maxHP,monster[warMonsterIndex[i]][i].MP,monster[warMonsterIndex[i]][i].maxMP);
		TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
		j++;
		sprintf(buf, "DEX: %d   INT: %d   SPIRIT: %d",monster[warMonsterIndex[i]][i].DEX,monster[warMonsterIndex[i]][i].INT,monster[warMonsterIndex[i]][i].SPIRIT);
		TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
		j++;
		sprintf(buf, "ATK: %d   DEF: %d",monster[warMonsterIndex[i]][i].ATK,monster[warMonsterIndex[i]][i].DEF);
		TextOut(hdcSurf,x,y+j*h,buf,strlen(buf));
		x += 150;
		j = 0;
		}
	}
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
	int i;

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

	if (xOrY==X)
	{
		if(CheckLandType(mainChar[WAI].charPos[X]+plusOrMinus,mainChar[WAI].charPos[Y], WATER))
		{
			return(0);
		}
	}
	else
	{
		if(CheckLandType(mainChar[WAI].charPos[X],mainChar[WAI].charPos[Y]+plusOrMinus, WATER))
		{
			return(0);
		}
	}

	for (i=1; i<curNumChar; i++)
	{
		mainChar[i].charPos[X] = mainChar[i-1].charPos[X];
		mainChar[i].charPos[Y] = mainChar[i-1].charPos[Y];
	}
	mainChar[WAI].charPos[xOrY]+=plusOrMinus;
	for (i=1; i<curNumChar; i++)
	{
		(mainChar[i].worldCharArea).bottom = (mainChar[WAI].worldCharArea).bottom + (mainChar[i].charPos[Y]-mainChar[WAI].charPos[Y])*CELL_WIDTH;
		(mainChar[i].worldCharArea).left = (mainChar[WAI].worldCharArea).left + (mainChar[i].charPos[X]-mainChar[WAI].charPos[X])*WORLDCHARWIDTH;
		(mainChar[i].worldCharArea).top = (mainChar[i].worldCharArea).bottom - CELL_WIDTH;
		(mainChar[i].worldCharArea).right = (mainChar[i].worldCharArea).left + WORLDCHARWIDTH;
		(mainChar[i].worldCharUpperArea).bottom = (mainChar[i].worldCharArea).bottom - CELL_WIDTH;
		(mainChar[i].worldCharUpperArea).left = (mainChar[i].worldCharArea).left;
		(mainChar[i].worldCharUpperArea).top = (mainChar[i].worldCharArea).top - CELL_WIDTH;
		(mainChar[i].worldCharUpperArea).right = (mainChar[i].worldCharArea).right;
	}

	if (numCharDead != curNumChar)
	{
		numSteps++;
		if (numSteps >= MINSTEPSBEFOREENCOUNTER)
		{
			encounter = ((rand()%(MAXSTEPSBEFOREENCOUNTER - numSteps)) == 0);
		}
		else
			encounter = 0;
		if (encounter)
		{
			numMonsterInWar = rand()%3+1;
			int i;
			for (i=0; i<numMonsterInWar; i++)
				warMonsterIndex[i] = rand()%2;
			for (i=numMonsterInWar; i<MAXMONSTERINWAR; i++)
				warMonsterIndex[i] = DEADMONSTER;
		}
		for (i=curNumChar-1; i>=WAI; i--)
		{
			if(mainChar[i].state!=DEAD)
			{
				if(CheckLandType(mainChar[i].charPos[X],mainChar[i].charPos[Y],RESTORELAND))
				{
					mainChar[i].HP = mainChar[i].maxHP;
					mainChar[i].MP = mainChar[i].maxMP;
					displayEffect = RESTORELAND;
				}
				else if(CheckLandType(mainChar[i].charPos[X],mainChar[i].charPos[Y],POISONLAND))
				{
					mainChar[i].HP--;
					if(mainChar[i].HP == 0)
					{
						mainChar[i].state = DEAD;
						charIndex[i] = DEADCHAR;
						numCharDead++;
					}
					displayEffect = POISONLAND;
				}
			}
			mainChar[i].charDirection = mainChar[i-1].charDirection;
		}
	}

	worldMapOffset[xOrY]+=plusOrMinus;
	mainChar[WAI].charDirection = direction;	

	return(1);
}

int FromWarZoneToWorldZone()
{
	int i;
	startBattle = 1;
	warOver = NOTFINISHED;
	hitCount = 0;
	numMonsterDead = 0;

	firstRun = 1;

	zone = WORLDZONE;
	for (i=0; i<curNumChar; i++)
	{
		if (mainChar[i].state == ALIVE)
			charIndex[i] = MAINCHAR;
		if (mainChar[i].state == DEAD)
			charIndex[i] = DEADCHAR;
	}

	return(1);
}

// helper function for WhoseTurn()
int WarSortTurn()
{
	int i;
	int j;
	int key;
	int key1;

	for (j=1; j<hitCount; j++)
	{
		key = warTurnFactor[j];
		key1 = warTurn[j];
		i = j-1;
		while((i>=0) && (warTurnFactor[i]<key))
		{
			warTurnFactor[i+1] = warTurnFactor[i];
			warTurn[i+1] = warTurn[i];
			i = i--;
		}
		warTurnFactor[i+1] = key;
		warTurn[i+1] = key1;

	}

	return(1);
}

// determine the characters' and monsters' fighting sequence
int WhoseTurn()
{
	int i;
	int j = 0;
	for (i=0; i<numMonsterInWar; i++)
	{
		if(warMonsterIndex[i]!=DEADMONSTER)
		{
			warTurnFactor[j] = int(monster[warMonsterIndex[i]][i].DEX*(0.5 + 0.05*(rand()%16)));
			warTurn[j] = i;
			j++;
		}
	}
	for (i=0; i<curNumChar; i++)
	{
		if (mainChar[i].state != DEAD)
		{
			warTurnFactor[j] = int(mainChar[i].DEX*(0.5 + 0.05*(rand()%16)));
			warTurn[j] = WAITURN + i;
			j++;
		}
	}
	
	WarSortTurn();

	return(1);
}

// HARD CODE FOR NOW
int MonsterAction()
{
	int i,j;
	int action;
	for (i=0; i<numMonsterInWar; i++)
	{
		if (warMonsterIndex[i] != DEADMONSTER)
		{
			monster[warMonsterIndex[i]][i].warAction[0] = ATTACK;
			for (j=0; j<curNumChar; j++)
			{
				if (mainChar[j].state != DEAD)
				{
					monster[warMonsterIndex[i]][i].warAction[1] = j;
					break;
				}
			}
			hitCount++;
		}
	}
/*	action = rand()%3;
	if(action!=CAST)
		monster[index][which].warAction[0] = action;
	else if(!monster[index][which].MP)
		monster[index][which].warAction[0] = ATTACK;
	if (monster[index][which].warAction == ATTACK)
		// HARD CODE FOR NOW
		monster[index][which].warAction[1] = WAI;
*/	return(1);
}

int CharLevelUP(int who)
{
	lpddsprimary->Blt(&warInstantMessageBox,
		NULL,
		NULL,
		DDBLT_COLORFILL | DDBLT_WAIT,
		&ddbltfx);

	OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);

	keyTemp = 0;
	Sleep(200);	// wait until the key state changes back to up.
	while(!keyTemp)
	{
		if(KEY_DOWN('A') || KEY_DOWN('M')||KEY_DOWN('D')||KEY_DOWN('I')||KEY_DOWN(VK_SPACE)||KEY_DOWN(VK_RETURN))
			keyTemp = 1;
	}

	mainChar[who].nextLV = (mainChar[who].nextLV + mainChar[who].nextLV/2);
	mainChar[who].LV++;

	sprintf(buf,"%s LEVEL UP!", mainChar[who].name);

	if (mainChar[who].exp >= mainChar[who].nextLV)
		CharLevelUP(who);
	
	return(1);
}

int CharKillMonster(int whichChar, int whichMonster)
{
	int expIncrease;
	int i;
	monster[warMonsterIndex[whichMonster]][whichMonster].HP = monster[warMonsterIndex[whichMonster]][whichMonster].maxHP;
	monster[warMonsterIndex[whichMonster]][whichMonster].MP = monster[warMonsterIndex[whichMonster]][whichMonster].maxMP;
	monster[warMonsterIndex[whichMonster]][whichMonster].state = ALIVE;
	mainChar[WAI].gold += monster[warMonsterIndex[whichMonster]][whichMonster].gold;
	expIncrease = (monster[warMonsterIndex[whichMonster]][whichMonster].exp)/(curNumChar - numCharDead);
	if (expIncrease < 1)
		expIncrease = 1;
	for (i=0; i<curNumChar;i++)
	{
		if (mainChar[i].state != DEAD)
			mainChar[i].exp += expIncrease;
	}

	lpddsprimary->Blt(&warInstantMessageBox, 
		NULL, 
		NULL, 
		DDBLT_COLORFILL | DDBLT_WAIT, 
		&ddbltfx);

	OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);

	keyTemp = 0;
	Sleep(200);	// wait until the key state changes back to up.
	while(!keyTemp)
	{
		if(KEY_DOWN('A') || KEY_DOWN('M')||KEY_DOWN('D')||KEY_DOWN('I')||KEY_DOWN(VK_SPACE)||KEY_DOWN(VK_RETURN))
			keyTemp = 1;
	}

	sprintf(buf, "Monster %s is Dead!", monster[warMonsterIndex[whichMonster]][whichMonster].name);

	for (i=0; i<curNumChar; i++)
	{
		if (mainChar[i].exp >= mainChar[i].nextLV)
		{
			CharLevelUP(i);
		}
	}

	warMonsterIndex[whichMonster] = DEADMONSTER;

	numMonsterDead++;

	return(1);
}

int TargetMonsterDead(int &target)
{
	int j;
	if(warMonsterIndex[target] == DEADMONSTER)
	{
		for (j=0; j<numMonsterInWar; j++)
		{
			if (warMonsterIndex[j] != DEADMONSTER)
			{
				target = j;
				break;
			}
		}
	}	// end if(warMonsterIndex[target] == DEADMONSTER)
	return(1);
}

int TargetCharacterDead(int &target)
{
	int j;
	if(mainChar[target].state == DEAD)
	{
		for (j=WAI; j<curNumChar; j++)
		{
			if (mainChar[j].state!=DEAD)
			{
				target = j;
				break;
			}
		}
	}
	return(1);
}
int FightOneRound()
{
	int hurt;	// how many points to hurt
	int heal;	// how many points to heal

	int index;	// warAction[0] index of active character or monster
	int target;		// warAction[1] indicating target to perform the action
	int useWhat;	// warAction[2] indicating using what magic or item
	int dependWhat;	// the effect of the action depends on what factor of the character

	int i;
//	int j;

	int hasMessage = 1;
	sprintf(buf,"");

	if(warTurn[turnCount] <= MONSTER3TURN)
	{
		index = warTurn[turnCount];
		if (warMonsterIndex[index] != DEADMONSTER)
		{
			// for attack
			if (monster[warMonsterIndex[index]][index].warAction[0] == ATTACK)
			{
				target = monster[warMonsterIndex[index]][index].warAction[1];

				TargetCharacterDead(target);	
				
				if ((hurt = int(0.5+monster[warMonsterIndex[index]][index].ATK*(0.9 + 0.02*(rand()%11)) - mainChar[target].DEF*(0.5+0.02*(rand()%11)))) < 0)
					hurt = 0;
				mainChar[target].HP-=hurt;
				sprintf(buf, "%s hits %s by %d points", monster[warMonsterIndex[index]][index].name, mainChar[target].name, hurt);

				if(mainChar[target].HP <= 0)
				{
					mainChar[target].HP = 0;
					mainChar[target].state = DEAD;
					numCharDead++;
					warCharIndex[target] = WARDEADCHAR;
					lpddsprimary->Blt(&warInstantMessageBox, 
						NULL, 
						NULL, 
						DDBLT_COLORFILL | DDBLT_WAIT, 
						&ddbltfx);

					OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);
					keyTemp = 0;
					Sleep(200);	// wait until the key state changes back to up.
					while(!keyTemp)
					{
						if(KEY_DOWN('A') || KEY_DOWN('M')||KEY_DOWN('D')||KEY_DOWN('I')||KEY_DOWN(VK_SPACE)||KEY_DOWN(VK_RETURN))
							keyTemp = 1;
					}
					sprintf(buf, "%s is Dead!", mainChar[target].name);
					if (numCharDead == curNumChar)
						warOver = MONSTERWIN;
				}
			}
/*			else if (monster[warMonsterIndex[index]][index].warAction[0] == DEFEND)
			{
				monster[warMonsterIndex[index]][index].DEF = (monster[warMonsterIndex[index]][index].DEF)<<1;
				sprintf(buf, "%s's defense doubles!", monster[warMonsterIndex[index]][index].name);
*///			}
		}
		else
		{
			hasMessage = 0;
		}

/*		else
		{
			// in case the dead monster chooses defend
			monster[warMonsterIndex[index]][index].warAction[0] == ATTACK;
		}
*/	}
	else if (warTurn[turnCount] > MONSTER3TURN)
	{
		index = warTurn[turnCount] - WAITURN;
		if (mainChar[index].state != DEAD)
		{
			if (mainChar[index].warAction[0] == ATTACK)
			{
				target = mainChar[index].warAction[1];
				
				TargetMonsterDead(target);

				if ((hurt = int(0.5+mainChar[index].ATK*(0.9 + 0.02*(rand()%11)) - monster[warMonsterIndex[target]][target].DEF*(0.5+0.02*(rand()%11)))) < 0)
					hurt = 0;
				monster[warMonsterIndex[target]][target].HP-=hurt;
				sprintf(buf, "%s hits %s by %d points", mainChar[index].name, monster[warMonsterIndex[target]][target].name, hurt);
				if(monster[warMonsterIndex[target]][target].HP <= 0)
				{
					CharKillMonster(index, target);

					if (numMonsterInWar == numMonsterDead)
						warOver = CHARWIN;
				}
			}	// end if(mainChar[index].warAction[0] == ATTACK)

			else if (mainChar[index].warAction[0] == DEFEND)
			{
				mainChar[index].DEF = (mainChar[index].DEF)<<1;
				sprintf(buf, "%s's defense doubles!", mainChar[index].name);
			}	// end if(mainChar[index].warAction[0] == DEFEND)

			else if (mainChar[index].warAction[0] == CAST)
			{
				useWhat = mainChar[index].warAction[2];
				dependWhat = magicList[useWhat].dependOn;
				mainChar[index].MP -= magicList[useWhat].MPneeded;

				sprintf(buf, "%s casts %s", mainChar[index].name, magicList[useWhat].name);
				lpddsprimary->Blt(&warInstantMessageBox, 
					NULL, 
					NULL, 
					DDBLT_COLORFILL | DDBLT_WAIT, 
					&ddbltfx);

				OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);

				keyTemp = 0;
				Sleep(200);	// wait until the key state changes back to up.
				while(!keyTemp)
				{
					if(KEY_DOWN('A') || KEY_DOWN('M')||KEY_DOWN('D')||KEY_DOWN('I')||KEY_DOWN(VK_SPACE)||KEY_DOWN(VK_RETURN))
						keyTemp = 1;
				}
				if ((magicList[useWhat].type == ATTACKMAGIC) && (magicList[useWhat].who == TO1ENEMY))
				{
					target = mainChar[index].warAction[1];

					TargetMonsterDead(target);

					if (dependWhat == INTMAGIC)
						hurt = int(0.5+ magicList[useWhat].power*( 0.5 + ( (float(mainChar[index].INT)) / (float(monster[warMonsterIndex[target]][target].INT)) ) * ( 0.05*(rand()%11) ) ) );
					else if (dependWhat == SPIRITMAGIC)
						hurt = int(0.5+ magicList[useWhat].power*( 0.5 + ( (float(mainChar[index].SPIRIT)) / (float(monster[warMonsterIndex[target]][target].SPIRIT)) ) * ( 0.05*(rand()%11) ) ) );
					if (hurt < 0)
						hurt = 0;
					if(magicList[useWhat].status == TOHP)
					{
						monster[warMonsterIndex[target]][target].HP-=hurt;
						sprintf(buf, "%s was hurt %d points!", monster[warMonsterIndex[target]][target].name, hurt);
						if(monster[warMonsterIndex[target]][target].HP <= 0)
						{
							CharKillMonster(index, target);

							if (numMonsterInWar == numMonsterDead)
								warOver = CHARWIN;
						}
					}	// end if(magicList[useWhat].status == TOHP)
				}	// end if ((magicList[useWhat].type == ATTACKMAGIC) && (magicList[useWhat].who == TO1ENEMY))
 
				else if ((magicList[useWhat].type == REFRESHMAGIC) && (magicList[useWhat].who == TO1CHAR))
				{
					target = mainChar[index].warAction[1];

					TargetCharacterDead(target);

					if (dependWhat == INTMAGIC)
						heal = int(0.5 + magicList[useWhat].power*( 0.7 + (mainChar[index].INT*(0.001 *(rand()%9))) ));
					else if (dependWhat == SPIRITMAGIC)
						heal = int(0.5 + magicList[useWhat].power*( 0.7 + (mainChar[index].SPIRIT*(0.001 *(rand()%9))) ));
					if(magicList[useWhat].status == TOHP)
					{
						mainChar[target].HP+=heal;
						sprintf(buf, "%s regains %d HPs!", mainChar[target].name, heal); 
						if (mainChar[target].HP>mainChar[target].maxHP)
							mainChar[target].HP = mainChar[target].maxHP;
					}	// end if(magic[useWhat].status == TOHP)
				}	// end else if ((magicList[(mainChar[index].warAction[2])].type == REFRESHMAGIC) && (magicList[(mainChar[index].warAction[2])].who == TO1CHAR))
			}	// end if(mainChar[index].warAction[0] == CAST)
		}
		else
		{
			hasMessage = 0;
		}
	}

	if (hasMessage)
	{
		lpddsprimary->Blt(&warInstantMessageBox, 
			NULL, 
			NULL, 
			DDBLT_COLORFILL | DDBLT_WAIT, 
			&ddbltfx);

		OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);

		keyTemp = 0;
		Sleep(200);	// wait until the key state changes back to up.
		while(!keyTemp)
		{
			if(KEY_DOWN('A') || KEY_DOWN('M')||KEY_DOWN('D')||KEY_DOWN('I')||KEY_DOWN(VK_SPACE)||KEY_DOWN(VK_RETURN))
				keyTemp = 1;
		}
	}
	// reset the def of characters
	for (i=0; i<curNumChar; i++)
	{
		if ((hitCount == 1) && (mainChar[i].warAction[0] == DEFEND))
			mainChar[i].DEF = (mainChar[i].DEF)>>1;
	}

/*	for (i=0; i<NUMMONSTER; i++)
	{
		if ((hitCount == 1) && (monster[warMonsterIndex[i]][i].warAction[0] == DEFEND))
			monster[warMonsterIndex[i]][i].DEF = (monster[warMonsterIndex[i]][i].DEF)>>1;
	}
*/

	return(1);
}

int QueryMonster(int k)
{
	int i;
	lpddsprimary->Blt(&warInstantMessageBox, 
		NULL, 
		NULL, 
		DDBLT_COLORFILL | DDBLT_WAIT, 
		&ddbltfx);

	OutText("To which monster?(1-3)",warInstantMessageBox.left,warInstantMessageBox.top);
	Sleep(200);
	i = 0;
	keyTemp = 0;
	while(!i)
	{
		while(!keyTemp)
		{
			if(KEY_DOWN('1'))
			{
				warKey = 0;
				keyTemp=1;
			}
			else if(KEY_DOWN('2'))
			{
				warKey = 1;
				keyTemp = 1;
			}
			else if (KEY_DOWN('3'))
			{
				warKey = 2;
				keyTemp = 1;
			}
		}	// end while(!keyTemp)
		if (warMonsterIndex[warKey] != DEADMONSTER)
		{
			mainChar[k].warAction[1] = warKey;
			i = 1;
		}
		else
		{
			lpddsprimary->Blt(&warInstantMessageBox, 
				NULL, 
				NULL, 
				DDBLT_COLORFILL | DDBLT_WAIT, 
				&ddbltfx);
			OutText("Invalid Monster or Monster already Dead! Choose another one(1-3)",warInstantMessageBox.left,warInstantMessageBox.top);
			Sleep(200);
			keyTemp = 0;
		}
	}	// end while(!i)

	return(1);
}

int QueryCharacter(int k)
{
	int i;
	lpddsprimary->Blt(&warInstantMessageBox, 
		NULL, 
		NULL, 
		DDBLT_COLORFILL | DDBLT_WAIT, 
		&ddbltfx);

	OutText("To which character?(W=Wai  R=Ray)",warInstantMessageBox.left,warInstantMessageBox.top);
	Sleep(200);
	i = 0;
	keyTemp = 0;
	while(!i)
	{
		while(!keyTemp)
		{
			if(KEY_DOWN('W'))
			{
				warKey = WAI;
				keyTemp=1;
			}
			else if(KEY_DOWN('R'))
			{
				warKey = RAY;
				keyTemp = 1;
			}
		}	// end while(!keyTemp)

		if (mainChar[warKey].state != DEAD)
		{
			mainChar[k].warAction[1] = warKey;
			i = 1;
		}
		else
		{
			lpddsprimary->Blt(&warInstantMessageBox, 
				NULL, 
				NULL, 
				DDBLT_COLORFILL | DDBLT_WAIT, 
				&ddbltfx);
			OutText("Character is Dead! Choose another one(W=Wai R=Ray)",warInstantMessageBox.left,warInstantMessageBox.top);
			Sleep(200);
			keyTemp = 0;
		}
	}	// end while(!i)

	return(1);
}

int QueryMagic(int k)
{
	lpddsprimary->Blt(&warInstantMessageBox, 
		NULL, 
		NULL, 
		DDBLT_COLORFILL | DDBLT_WAIT, 
		&ddbltfx);

	int h = 0;
	h=sprintf(buf, "Which Magic?");
	for (int m=0; m<mainChar[k].numMagic; m++)
	{
		h += sprintf(buf+h, " %s", magicList[(mainChar[k].magic[m])].name);
	}

	OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);
	Sleep(200);

	keyTemp = 0;
	warKey = 0;
	while(!keyTemp)
	{
		if(KEY_DOWN('L'))
		{
			warKey--;
			if (warKey < 0)
				warKey = mainChar[k].numMagic-1;
			Sleep(200);
		}
		else if(KEY_DOWN('R'))
		{
			warKey++;
			if (warKey >= mainChar[k].numMagic)
				warKey = 0;
			Sleep(200);
		}
		else if (KEY_DOWN('A'))
		{
			keyTemp = 1;
		}
	}	// end while(!keyTemp)
	mainChar[k].warAction[2] = mainChar[k].magic[warKey];

	return(1);
}

int War_Main(void *parms)
{
	int i;
	int j;

	if(warOver)
	{
		FromWarZoneToWorldZone();
		return(1);
	}

	if (!startBattle)
	{
		while(!hitCount)
		{
			j=0;
			while(hitCount!=curNumChar-numCharDead)	// loop for player to enter command
			{
				// find an alive character
				while(mainChar[j].state == DEAD)
				{
					j++;
				}
				sprintf(buf, "%s's Turn", mainChar[j].name);
				lpddsprimary->Blt(&warInstantMessageBox, 
					NULL, 
					NULL, 
					DDBLT_COLORFILL | DDBLT_WAIT, 
					&ddbltfx);

				OutText(buf,warInstantMessageBox.left,warInstantMessageBox.top);
				keyTemp = 0;
				while(!keyTemp)
				{
					if(KEY_DOWN('A'))
					{
						mainChar[j].warAction[0] = ATTACK;
						QueryMonster(j);
						
						hitCount++;
						j++;
						Sleep(200);	// wait until the key state changes back to up.
					}	// end if(KEY_DOWN('A')
			
					else if (KEY_DOWN('M'))
					{
						mainChar[j].warAction[0] = CAST;
						QueryMagic(j);
						if (magicList[(mainChar[j].warAction[2])].MPneeded > mainChar[j].MP)
						{
							lpddsprimary->Blt(&warInstantMessageBox, 
								NULL, 
								NULL, 
								DDBLT_COLORFILL | DDBLT_WAIT, 
								&ddbltfx);
							OutText("Not Enough MP!",warInstantMessageBox.left,warInstantMessageBox.top);
							keyTemp = 0;
							Sleep(200);	// wait until the key state changes back to up.
							while(!keyTemp)
							{
								if(KEY_DOWN('A') || KEY_DOWN('M')||KEY_DOWN('D')||KEY_DOWN('I')||KEY_DOWN(VK_SPACE)||KEY_DOWN(VK_RETURN))
									keyTemp = 1;
							}
						}
						else
						{
							if ((magicList[(mainChar[j].warAction[2])].type == ATTACKMAGIC) && (magicList[(mainChar[j].warAction[2])].who == TO1ENEMY))
							{
								QueryMonster(j);
							}
							else if ((magicList[(mainChar[j].warAction[2])].type == REFRESHMAGIC) && (magicList[(mainChar[j].warAction[2])].who == TO1CHAR))
							{
								QueryCharacter(j);
							}

							hitCount++;
							j++;
						}
						Sleep(200);	// wait until the key state changes back to up.
					}
					else if (KEY_DOWN('D'))
					{
						mainChar[j].warAction[0] = DEFEND;
						hitCount++;
						j++;
						Sleep(200);	// wait until the key state changes back to up.
						keyTemp = 1;
					}	// end if(KEY_DOWN('D'))
				}	// end while(!keyTemp)
			}	// end while(hitCount!=curNumChar-numCharDead)
			MonsterAction();
			WhoseTurn();
			turnCount = 0;
			if (KEY_DOWN('Q'))
			{
				FromWarZoneToWorldZone();
				return(1);
			}
		}	// end while(!hitCount)
	}	// end while(!startBattle)
	else
	{
		startBattle = 0;	// do the init display
	}

	if(hitCount)
	{
		FightOneRound();
		hitCount--;
		turnCount++;
	}
	
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = _RGB24BIT(0, 0, 0);

	lpddsback->Blt(&warMessageBox, 
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

	for (i=0; i<curNumChar; i++)
	{
		(mainChar[i].warCharSrcArea).left = (warCharIndex[i]<<3)+(warCharIndex[i]<<5);
		(mainChar[i].warCharSrcArea).right = (mainChar[i].warCharSrcArea).left+WARCHARWIDTH;
		lpddsback->Blt(&(mainChar[i].warCharArea),
			mainChar[i].lpddsbattlechar,
			&(mainChar[i].warCharSrcArea),
			DDBLT_WAIT|DDBLT_KEYSRC,
			NULL);
	}

	for(i=0; i<numMonsterInWar; i++)
	{
		if (warMonsterIndex[i] != DEADMONSTER)
		{
			lpddsback->Blt(&(monster[warMonsterIndex[i]][i].warMonsterArea),
				monster[warMonsterIndex[i]][i].lpddswarmonster,
				&(monster[warMonsterIndex[i]][i].srcArea),
				DDBLT_WAIT|DDBLT_KEYSRC,
				NULL);
		}
	}

	while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);
	OutMessage(20,330);
	if(!WarPrintStatus(20,20))
		return (0);

	Sleep(30);
	return(1);
}

int FromWorldZoneToWarZone()
{
	int i;

	zone = WARZONE;
	numSteps = 0;
	for (i=0; i<curNumChar; i++)
	{
		if (mainChar[i].state == ALIVE)
			warCharIndex[i] = WARMAINCHAR;
		if (mainChar[i].state == DEAD)
			warCharIndex[i] = WARDEADCHAR;
		(mainChar[i].warCharSrcArea).left = (warCharIndex[i]<<3)+(warCharIndex[i]<<5);
		(mainChar[i].warCharSrcArea).right = (mainChar[i].warCharSrcArea).left+WARCHARWIDTH;
	}

	fill_area.left = 0;
	fill_area.top = 0;
	fill_area.right = 640;
	fill_area.bottom = 320;
	
	if (CheckLandType(mainChar[WAI].charPos[X],mainChar[WAI].charPos[Y],GRASS))
		backgroundIndex = GRASSBG;
	else if(CheckLandType(mainChar[WAI].charPos[X],mainChar[WAI].charPos[Y],VILLAGE))
		backgroundIndex = GRASSBG;
	else if(CheckLandType(mainChar[WAI].charPos[X],mainChar[WAI].charPos[Y],POISONLAND))
		backgroundIndex = POISONLANDBG;
	else if(CheckLandType(mainChar[WAI].charPos[X],mainChar[WAI].charPos[Y],RESTORELAND))
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
	int tempX, tempY;
	int i, j;

	landscapeFrameCount++;

	if (KEY_DOWN(VK_ESCAPE))
		PostMessage(main_window_handle, WM_DESTROY,0,0);

	if (KEY_DOWN('S'))
	{
		if(!PrintStatus(/*mainChar,*/ 40, 40))
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
		if ((!firstRun) && (landscapeFrameCount < 3000))
		{
			
/*			if (landscapeFrameCount == 3000)
			{
				landscapeFrameCount = 0;
				for (int i=0; i<SCREEN_HEIGHT; i+=CELL_WIDTH)
				{
					for (int j=0; j<SCREEN_WIDTH; j+=CELL_WIDTH)
					{
						fill_area.top = i;
						fill_area.left = j;
						fill_area.bottom = i+CELL_WIDTH;
						fill_area.right = j+CELL_WIDTH;
					
						tempX = worldMapOffset[0] + j/CELL_WIDTH;
						tempY = worldMapOffset[1] + i/CELL_WIDTH;
						
						grassIndex = worldMap[tempX][tempY].landtype;
						if (worldMap[tempX][tempY].numFrame != 1)
						{
							worldMap[tempX][tempY].curFrame++;
							if(worldMap[tempX][tempY].curFrame == worldMap[tempX][tempY].numFrame)
								worldMap[tempX][tempY].curFrame = 0;
							else
								grassIndex = worldMap[tempX][tempY].curFrame + NUM_OF_LANDSCAPES-1;
						}
					
			
						src_area.left = (grassIndex<<3)+(grassIndex<<5);
						src_area.right = src_area.left+CELL_WIDTH;
						lpddsback->Blt(&fill_area,
							lpddsgrass,
							&src_area,
							DDBLT_WAIT,
							NULL);
					}
				}
				src_area.left = (charIndex<<3)+(charIndex<<5);
				src_area.right = src_area.left+CELL_WIDTH;

				lpddsback->Blt(&(mainChar[WAI].worldCharArea),//280,200,
					mainChar[WAI].lpddscharacter,
					&src_area,
					DDBLT_WAIT|DDBLT_KEYSRC,
					NULL);
				
				while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);
			}
*/
			return(1);
		}
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
			if (landscapeFrameCount >= 5)
			{
				for (i=initVertDisplay; i<SCREEN_HEIGHT; i+=CELL_WIDTH)
				{
					for (j=initHoriDisplay; j<SCREEN_WIDTH; j+=CELL_WIDTH)
					{
						fill_area.top = i;
						fill_area.left = j;
						fill_area.bottom = i+CELL_WIDTH;
						fill_area.right = j+CELL_WIDTH;
						
						tempX = worldMapOffset[0]+(j-initHoriDisplay)/CELL_WIDTH+strangeTemp[0];
						tempY = worldMapOffset[1]+(i-initVertDisplay)/CELL_WIDTH+strangeTemp[1];
						
						grassIndex = worldMap[tempX][tempY].landtype;
			
						if (worldMap[tempX][tempY].numFrame != 1)
						{
							if((++(worldMap[tempX][tempY].curFrame)) == worldMap[tempX][tempY].numFrame)
							{
								worldMap[tempX][tempY].curFrame = 0;
							}
							grassIndex += worldMap[tempX][tempY].frameSequence[(worldMap[tempX][tempY].curFrame)];
						}
						src_area.left = (grassIndex<<3)+(grassIndex<<5);
						src_area.right = src_area.left+CELL_WIDTH;
						lpddsback->Blt(&fill_area,
							lpddsgrass,
							&src_area,
							DDBLT_WAIT,
							NULL);
					}
				}
			}
			else
			{
				for (i=initVertDisplay; i<SCREEN_HEIGHT; i+=CELL_WIDTH)
				{
					for (j=initHoriDisplay; j<SCREEN_WIDTH; j+=CELL_WIDTH)
					{
						fill_area.top = i;
						fill_area.left = j;
						fill_area.bottom = i+CELL_WIDTH;
						fill_area.right = j+CELL_WIDTH;
						
						tempX = worldMapOffset[0]+(j-initHoriDisplay)/CELL_WIDTH+strangeTemp[0];
						tempY = worldMapOffset[1]+(i-initVertDisplay)/CELL_WIDTH+strangeTemp[1];
						
						grassIndex = worldMap[tempX][tempY].landtype;
			
						grassIndex += worldMap[tempX][tempY].frameSequence[worldMap[tempX][tempY].curFrame];
						src_area.left = (grassIndex<<3)+(grassIndex<<5);
						src_area.right = src_area.left+CELL_WIDTH;
						lpddsback->Blt(&fill_area,
							lpddsgrass,
							&src_area,
							DDBLT_WAIT,
							NULL);
					}
				}
			}

	
			for (i=curNumChar-1; i>=0; i--)
			{
				(mainChar[i].worldCharSrcArea).left = (charIndex[i]<<3)+(charIndex[i]<<5);
				(mainChar[i].worldCharSrcArea).right = (mainChar[i].worldCharSrcArea).left+WORLDCHARWIDTH;
				(mainChar[i].worldCharSrcUpperArea).left = (mainChar[i].worldCharSrcArea).left;
				(mainChar[i].worldCharSrcUpperArea).right = (mainChar[i].worldCharSrcArea).right;

				lpddsback->Blt(&(mainChar[i].worldCharArea),//280,200,
				mainChar[i].lpddscharacter,
				&(mainChar[i].worldCharSrcArea),
				DDBLT_WAIT|DDBLT_KEYSRC,
				//DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
				NULL);
			}
			for (i=curNumChar-1; i>=0; i--)
			{
				lpddsback->Blt(&(mainChar[i].worldCharUpperArea),//280,200,
				mainChar[i].lpddscharacter,
				&(mainChar[i].worldCharSrcUpperArea),
				DDBLT_WAIT|DDBLT_KEYSRC,
				//DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
				NULL);
			}
		}

		initHoriDisplay+=horizontalDisplayOffset;
		initVertDisplay+=verticalDisplayOffset;
		//////////////////////////////////////
//		lpdd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0); // slows down so much with this

		while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);
		if (landscapeFrameCount>=5)
			landscapeFrameCount = 0;
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

int MagicInit()
{
	magicList[FIRE].name = "Fire";
	magicList[FIRE].MPneeded = 2;
	magicList[FIRE].who = TO1ENEMY;
	magicList[FIRE].status = TOHP;
	magicList[FIRE].type = ATTACKMAGIC;
	magicList[FIRE].power = 13;
	magicList[FIRE].where = WARMAGIC;
	magicList[FIRE].dependOn = INTMAGIC;

	magicList[CURE].name = "Cure";
	magicList[CURE].MPneeded = 3;
	magicList[CURE].who = TO1CHAR;
	magicList[CURE].status = TOHP;
	magicList[CURE].type = REFRESHMAGIC;
	magicList[CURE].power = 30;
	magicList[CURE].where = BOTHMAGIC;
	magicList[CURE].dependOn = SPIRITMAGIC;

	return(1);
}

int CharacterInit()
{
	///////////////////////////////////////////////////////
	// SET main character attributes
	///////////////////////////////////////////////////////
	int i;

	mainChar = new MAIN_CHAR[NUMCHAR];

	sprintf(mainChar[WAI].name, "Wai");
	mainChar[WAI].charPos[X] = 40;
	mainChar[WAI].charPos[Y] = 30;
	mainChar[WAI].charDirection = STAY;

	mainChar[WAI].state = ALIVE;
	mainChar[WAI].LV = 1;
	mainChar[WAI].exp = 4;
	mainChar[WAI].gold = 80;
	mainChar[WAI].nextLV = 8;

	mainChar[WAI].maxHP = 15;
	mainChar[WAI].maxMP = 3;
	mainChar[WAI].STR = 15;
	mainChar[WAI].DEX = 12;
	mainChar[WAI].INT = 8;
	mainChar[WAI].SPIRIT = 8;
	mainChar[WAI].CONS = 12;

	mainChar[WAI].weapon = &(weaponList[CLUB]);
	mainChar[WAI].armor = &(armorList[CLOTHCLOTHES]);
	mainChar[WAI].magic[0] = FIRE;
//	mainChar[WAI].magic[1] = CURE;
	mainChar[WAI].numMagic = 1;
	

	(mainChar[WAI].worldCharArea).top = 200;
	(mainChar[WAI].worldCharArea).left = 280;
	(mainChar[WAI].worldCharArea).bottom = 240;
	(mainChar[WAI].worldCharArea).right = 320;
	(mainChar[WAI].worldCharUpperArea).top = (mainChar[WAI].worldCharArea).top - CELL_WIDTH;
	(mainChar[WAI].worldCharUpperArea).left = (mainChar[WAI].worldCharArea).left;
	(mainChar[WAI].worldCharUpperArea).bottom = (mainChar[WAI].worldCharArea).bottom - CELL_WIDTH;
	(mainChar[WAI].worldCharUpperArea).right = (mainChar[WAI].worldCharArea).right;

	sprintf(mainChar[RAY].name, "Ray");
	mainChar[RAY].charPos[X] = mainChar[WAI].charPos[X];
	mainChar[RAY].charPos[Y] = mainChar[WAI].charPos[Y];
	mainChar[RAY].charDirection = STAY;

	mainChar[RAY].state = ALIVE;
	mainChar[RAY].LV = 1;
	mainChar[RAY].exp = 3;
	mainChar[RAY].gold = 0;
	mainChar[RAY].nextLV = 7;

	mainChar[RAY].maxHP = 13;
	mainChar[RAY].maxMP = 5;
	mainChar[RAY].STR = 12;
	mainChar[RAY].DEX = 10;
	mainChar[RAY].INT = 8;
	mainChar[RAY].SPIRIT = 13;
	mainChar[RAY].CONS = 10;

//	mainChar[RAY].weapon = new WEAPON;
	mainChar[RAY].weapon = &(weaponList[CLUB]);
	mainChar[RAY].armor = &(armorList[CLOTHCLOTHES]);
	mainChar[RAY].magic[0] = CURE;
	mainChar[RAY].numMagic = 1;

	for (i=0; i<NUMCHAR; i++)
	{
		mainChar[i].HP = mainChar[i].maxHP;
		mainChar[i].MP = mainChar[i].maxMP;
		mainChar[i].ATK = mainChar[i].STR/2 + (mainChar[i].weapon)->attackPower;
		mainChar[i].DEF = mainChar[i].DEX/2 + (mainChar[i].armor)->defPower;
		(mainChar[i].warCharSrcArea).top = 0;
		(mainChar[i].warCharSrcArea).left = 0;
		(mainChar[i].warCharSrcArea).right = WARCHARWIDTH;
		(mainChar[i].warCharSrcArea).bottom = WARCHARHEIGHT;
		(mainChar[i].worldCharArea).bottom = (mainChar[WAI].worldCharArea).bottom + (mainChar[i].charPos[Y]-mainChar[WAI].charPos[Y])*CELL_WIDTH;
		(mainChar[i].worldCharArea).left = (mainChar[WAI].worldCharArea).left + (mainChar[i].charPos[X]-mainChar[WAI].charPos[X])*WORLDCHARWIDTH;
		(mainChar[i].worldCharArea).top = (mainChar[i].worldCharArea).bottom - CELL_WIDTH;
		(mainChar[i].worldCharArea).right = (mainChar[i].worldCharArea).left + WORLDCHARWIDTH;
		(mainChar[i].worldCharUpperArea).bottom = (mainChar[i].worldCharArea).bottom - CELL_WIDTH;
		(mainChar[i].worldCharUpperArea).left = (mainChar[i].worldCharArea).left;
		(mainChar[i].worldCharUpperArea).top = (mainChar[i].worldCharArea).top - CELL_WIDTH;
		(mainChar[i].worldCharUpperArea).right = (mainChar[i].worldCharArea).right;
		(mainChar[i].worldCharSrcArea).top = CELL_WIDTH;
		(mainChar[i].worldCharSrcArea).bottom = WORLDCHARHEIGHT;
		(mainChar[i].worldCharSrcUpperArea).top = 0;
		(mainChar[i].worldCharSrcUpperArea).bottom = CELL_WIDTH;

		if (i<2)
		{
			(mainChar[i].warCharArea).top = 160+i*70;
			(mainChar[i].warCharArea).left = 420+i*70;
		}
		else
		{
			(mainChar[i].warCharArea).top = 160 -(i-1)*70;
			(mainChar[i].warCharArea).left = 420+(i-1)*70;
		}
		(mainChar[i].warCharArea).right = (mainChar[i].warCharArea).left+40;
		(mainChar[i].warCharArea).bottom = (mainChar[i].warCharArea).top+80;
	}
	return(1);
}

int MonsterInit()
{
	int i, j;
	///////////////////////////////////////////////////////
	// SET monsters attributes
	///////////////////////////////////////////////////////
	monster = new MONSTER*[NUMMONSTER];
	for(i=0; i<NUMMONSTER; i++)
		monster[i] = new MONSTER[MAXMONSTERINWAR];

	for (i=0; i<MAXMONSTERINWAR; i++)
	{
		sprintf(monster[MECAR][i].name, "Car of Me %d", i+1);

		monster[MECAR][i].exp = 1;
		monster[MECAR][i].gold = 5;

		monster[MECAR][i].maxHP = 7;
		monster[MECAR][i].maxMP = 0;
		monster[MECAR][i].DEX = 8;
		monster[MECAR][i].INT = 3;
		monster[MECAR][i].SPIRIT = 2;

		monster[MECAR][i].ATK = 7;
		monster[MECAR][i].DEF = 5;

		sprintf(monster[BIRD][i].name, "Killer Bird %d", i+1);

		monster[BIRD][i].exp = 2;
		monster[BIRD][i].gold = 7;

		monster[BIRD][i].maxHP = 8;
		monster[BIRD][i].maxMP = 0;
		monster[BIRD][i].DEX = 9;
		monster[BIRD][i].INT = 5;
		monster[BIRD][i].SPIRIT = 2;

		monster[BIRD][i].ATK = 8;
		monster[BIRD][i].DEF = 6;

	}

	for (j=0; j<NUMMONSTER; j++)
	{
		for (i=0; i<MAXMONSTERINWAR; i++)
		{
			monster[j][i].HP = monster[j][i].maxHP;
			monster[j][i].MP = monster[j][i].maxMP;
			monster[j][i].srcArea.left = 0;
			monster[j][i].srcArea.right = WARMONSTERWIDTH;
			monster[j][i].srcArea.top = 0;
			monster[j][i].srcArea.bottom = WARMONSTERHEIGHT;

			monster[j][i].state = ALIVE;

			(monster[j][i].warMonsterArea).right = 220 - (i*70);
			(monster[j][i].warMonsterArea).left = (monster[j][i].warMonsterArea).right-WARMONSTERWIDTH;

			if(i == 0)
				(monster[j][i].warMonsterArea).top = 150;
			else if(i == 1)
				(monster[j][i].warMonsterArea).top = 220;
			else if(i==2)
				(monster[j][i].warMonsterArea).top = 80;
			(monster[j][i].warMonsterArea).bottom = (monster[j][i].warMonsterArea).top+WARMONSTERHEIGHT;
		}
	}
	return(1);
}


int InitBitmap()
{
	if(!LoadImage(mainChar[WAI].lpddscharacter, "..\\Bitmaps\\Characters\\RPGmainChar.bmp",MAINCHAR,WORLDCHARWIDTH,WORLDCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[WAI].lpddscharacter, "..\\Bitmaps\\Characters\\deadChar.bmp",DEADCHAR,WORLDCHARWIDTH,WORLDCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[WAI].lpddsbattlechar, "..\\Bitmaps\\WarCharacters\\mainChar.bmp",WARMAINCHAR,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[WAI].lpddsbattlechar, "..\\Bitmaps\\WarCharacters\\deadChar.bmp",WARDEADCHAR,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[RAY].lpddscharacter, "..\\Bitmaps\\Characters\\mainRay.bmp",MAINCHAR,WORLDCHARWIDTH,WORLDCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[RAY].lpddscharacter, "..\\Bitmaps\\Characters\\rayDeadChar.bmp",DEADCHAR,WORLDCHARWIDTH,WORLDCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[RAY].lpddsbattlechar, "..\\Bitmaps\\WarCharacters\\rayMainChar.bmp",WARMAINCHAR,WARCHARWIDTH,WARCHARHEIGHT))
		return(0);
	if(!LoadImage(mainChar[RAY].lpddsbattlechar, "..\\Bitmaps\\WarCharacters\\rayDeadChar.bmp",WARDEADCHAR,WARCHARWIDTH,WARCHARHEIGHT))
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
	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\water2.bmp",WATER2,CELL_WIDTH,CELL_WIDTH))
		return(0);
	if(!LoadImage(lpddsgrass, "..\\Bitmaps\\Landscapes\\water3.bmp",WATER3,CELL_WIDTH,CELL_WIDTH))
		return(0);

	if(!LoadImage(lpddsbackground, "..\\Bitmaps\\Backgrounds\\grass.bmp",GRASSBG,BACKGROUNDWIDTH,BACKGROUNDHEIGHT))
		return(0);
	if(!LoadImage(lpddsbackground, "..\\Bitmaps\\Backgrounds\\try.bmp",POISONLANDBG,BACKGROUNDWIDTH,BACKGROUNDHEIGHT))
		return(0);
	if(!LoadImage(lpddsbackground, "..\\Bitmaps\\Backgrounds\\restore.bmp",RESTORELANDBG,BACKGROUNDWIDTH,BACKGROUNDHEIGHT))
		return(0);

	if(!LoadImage(lpddswarmonster[MECAR], "..\\Bitmaps\\Monsters\\meCar.bmp",NORMALMONSTER,WARMONSTERWIDTH,WARMONSTERHEIGHT))
		return(0);
	if(!LoadImage(lpddswarmonster[BIRD], "..\\Bitmaps\\Monsters\\bird.bmp",NORMALMONSTER,WARMONSTERWIDTH,WARMONSTERHEIGHT))
		return(0);
	
	return(1);
}

HRESULT CALLBACK EnumDisplayModeCallback(LPDDSURFACEDESC pDDSD, LPVOID Context)
{
	DWORD w = pDDSD->dwWidth;
	DWORD h = pDDSD->dwHeight;
	auto depth = pDDSD->ddpfPixelFormat.dwRGBBitCount;

/*	DisplayMode** DMList = (DisplayMode**)Context;

	DisplayMode* NewDM = new DisplayMode;
	if (NewDM)
	{
		NewDM->Width = pDDSD->dwWidth;
		NewDM->Height = pDDSD->dwHeight;
		NewDM->Depth = pDDSD->ddpfPixelFormat.dwRGBBitCount;

		NewDM->Next = NULL;
	}

	if (!(*DMList))
		*DMList = NewDM;
	else
	{
		for (DisplayMode* TheDM = *DMList; TheDM->Next; TheDM = TheDM->Next)
			;

		TheDM->Next = NewDM;
	}
	*/
	return DDENUMRET_OK;
}

///////////////////////////////////////////////////////////////////
//
//	Game_Init()
//
///////////////////////////////////////////////////////////////////
int Game_Init(void *parms)
{
	int i;
	// initialize variables
	WeaponInit();

	ArmorInit();

	MagicInit();

	for (i=0; i<NUMCHAR; i++)
		charIndex[i] = MAINCHAR;
	curNumChar = 2;
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
	{
		sprintf(buf, "Error reading map!");
		return(0);
	}


// your code goes here
	if(FAILED(DirectDrawCreate(NULL,&lpdd,NULL)))
	{
		sprintf(buf, "Error Creating DD Obj!");
		return(0);
	}
	if(FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
		DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
	{
		sprintf(buf, "Error Setting CoopLevel!");
		return(0);
	}

//	lpdd->EnumDisplayModes(0, NULL, NULL, (LPDDENUMMODESCALLBACK)EnumDisplayModeCallback);

	if(FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BIT)))
	{
		SCREEN_BIT = 16;
		if(FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BIT)))
		{
			sprintf(buf, "Error Setting Display Mode!");
			return(0);
		}
	}
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = 
		DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = NUMBUFFER;

	if(FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary,NULL)))
	{
		sprintf(buf, "Error creating Primary Surface!");
		return(0);
	}

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

	if(lpddsprimary->GetAttachedSurface(&ddscaps, &lpddsback)!=DD_OK)
	{
		sprintf(buf,"Error Creating BackBuffer!");
		return(0);
	}

	if(!CreateOffScreen(lpddsgrass, (NUM_OF_LANDSCAPES+NUMOFANIMLANDSCAPES)*CELL_WIDTH, CELL_WIDTH))
	{
		sprintf(buf,"Error Creating Offscreen for Landscapes!");
		return(0);
	}
	for(i=0; i<NUMCHAR; i++)
	{
		if(!CreateOffScreen(mainChar[i].lpddscharacter, NUMCHARMAPS*WORLDCHARWIDTH, WORLDCHARHEIGHT))
		{
			sprintf(buf, "Error Creating Offscreen for World Char!");
			return(0);
		}
		if(!CreateOffScreen(mainChar[i].lpddsbattlechar, NUMWARCHARMAPS*WARCHARWIDTH, WARCHARHEIGHT))
		{
			sprintf(buf, "Error Creating Offscreen for War Char!");
			return(0);
		}
	}
	for (i=0; i<NUMMONSTER; i++)
	{
		if(!CreateOffScreen(lpddswarmonster[i], NUMMONSTERMAPS*WARMONSTERWIDTH, WARMONSTERHEIGHT))
		{
			sprintf(buf, "Error Creating Offscreen for Monster!");
				return(0);
		}
	}
	if(!CreateOffScreen(lpddsbackground, NUMOFBACKGROUNDS*BACKGROUNDWIDTH, BACKGROUNDHEIGHT))
	{
		sprintf(buf, "Error Creating Offscreen for Background!");
		return(0);
	}
	for (i=0; i<NUMMONSTER; i++)
	{
		for (int j=0; j<MAXMONSTERINWAR; j++)
		{
			monster[i][j].lpddswarmonster = lpddswarmonster[i];
		}
	}

	DDCOLORKEY	key;
	key.dwColorSpaceLowValue = _RGB24BIT(8,8,8);
//	key.dwColorSpaceHighValue = _RGB24BIT(8,8,8);
	(mainChar[WAI].lpddscharacter)->SetColorKey(DDCKEY_SRCBLT, &key);
	(mainChar[WAI].lpddsbattlechar)->SetColorKey(DDCKEY_SRCBLT, &key);
	(mainChar[RAY].lpddscharacter)->SetColorKey(DDCKEY_SRCBLT, &key);
	(mainChar[RAY].lpddsbattlechar)->SetColorKey(DDCKEY_SRCBLT, &key);
	(lpddswarmonster[MECAR])->SetColorKey(DDCKEY_SRCBLT, &key);
	(lpddswarmonster[BIRD])->SetColorKey(DDCKEY_SRCBLT, &key);

	/////////////// LOADING BITMAPS //////////////////////////
	if(!InitBitmap())
	{
		sprintf(buf, "Error Init Bitmap!");
		return(0);
	}

	RECT cliplist[1] = {{0,0,SCREEN_WIDTH, SCREEN_HEIGHT}};
	lpddclip = DDAttachClipper(lpddsback, 1, cliplist);
	if(lpddclip==NULL)
	{
		sprintf(buf, "Error Attaching Clipper!");
		return(0);
	}

	src_area.top = 0;
	src_area.bottom = CELL_WIDTH;
	warSrcArea.top = 0;
	warSrcArea.bottom = BACKGROUNDHEIGHT;

	warMessageBox.top = 320;
	warMessageBox.bottom = SCREEN_HEIGHT;
	warMessageBox.left = 0;
	warMessageBox.right = SCREEN_WIDTH;

	HDC hdcSurf;
	if(FAILED(lpddsprimary->GetDC(&hdcSurf)))
	{
		if (hdcSurf)
			lpddsprimary->ReleaseDC(hdcSurf);
		sprintf(buf, "Error Getting DC!");
		return(0);
	}

	TEXTMETRIC tm;
	int h;
	GetTextMetrics(hdcSurf, &tm);
	h = tm.tmHeight + tm.tmExternalLeading;

	warInstantMessageBox.top = 0;
	warInstantMessageBox.bottom = h;
	warInstantMessageBox.left = 0;
	warInstantMessageBox.right = 640;

	lpddsprimary->ReleaseDC(hdcSurf);

	srand((unsigned)time(NULL));
	numSteps = 0;
	landscapeFrameCount = 0;
	numCharDead = 0;
	// return success
	return(1);
} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// your code goes here
	int i;
	for (i=0; i<curNumChar; i++)
	{
		if (mainChar[i].lpddscharacter)
			(mainChar[i].lpddscharacter)->Release();
		if (mainChar[i].lpddsbattlechar)
			(mainChar[i].lpddsbattlechar)->Release();
	}
	for (i=0; i<NUMMONSTER; i++)
	{
		if (lpddswarmonster[i])
			lpddswarmonster[i]->Release();
	}

	if (lpddsbackground)
		lpddsbackground->Release();
	if (lpddsgrass)
		lpddsgrass->Release();
	if (lpddsprimary)
		lpddsprimary->Release();
	if (lpdd!=NULL)
		lpdd->Release();
	delete [] monster;
	delete [] worldMap;
	delete [] mainChar;

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
winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= NULL; 
winclass.lpszClassName	= WINDOW_CLASS_NAME;

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
						  "RPG V3.0",	 // title
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
	MessageBox(main_window_handle, buf, "MB Test",MB_OK);
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


