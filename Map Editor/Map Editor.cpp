/* Message Box and Menus. */

//////////////////////////////////////////////////////////////
//	Known Bugs
//	
//////////////////////////////////////////////////////////////

#include <windows.h>
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
#include "resource.h"
#include "..\source\RPG.h"

#define DISPLAYOFFAMOUNT	10
#define NUMWATERFRAME		4

#define NONEACTIVE	0
#define MODIFY		1
#define DELETEROW	2
#define DELETECOL	3
#define INSERTROW	4
#define INSERTCOL	5

#define BEFORE	1
#define AFTER	2


LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);

/* the DialogBox callback function */
BOOL CALLBACK FileOpenDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK CustomDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK LandscapeDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK GotoDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK InsertDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK FileSaveDlg(HWND, UINT, WPARAM, LPARAM);

char szWinName[] = "MyWin";	/* name of window class */

HINSTANCE hInst;

// GLOBAL VARIABLES
HDC hdc, memDC;
HBITMAP hGrass, hWater, hVillage, hPoisonLand, hRestoreLand;
HWND	hWnd;

char	szFileName[80];
char	*sourceDir = "..\\Map\\";

MAPTILE **editMap;
int mapWidth=0; 
int mapHeight=0;
int upperLeftCorner[2] = {0,0};
int widthTile = 16;
int heightTile = 12;
int curLandscape = GRASS;
int cursorMode = NONEACTIVE;
int mouseX, mouseY;
int moveState = STAY;

int insertType = 0;

int ModifyMap(int x, int y, int landscape)
{
	if ((x<0)||(y<0)||(x>=mapWidth)||(y>=mapHeight)||(landscape>=NUM_OF_LANDSCAPES)||(landscape<0))
		return(0);
	editMap[x][y].landtype = landscape;
	if (landscape == WATER)
		editMap[x][y].numFrame = NUMWATERFRAME;
	else
		editMap[x][y].numFrame = 1;
	return(1);
}


int ModifyRangeMap(int fromX, int toX, int fromY, int toY, int landscape)
{
	if((fromX>toX)||(fromY>toY))
		return(0);
	for(int i=fromX; i<=toX; i++)
	{
		for(int j=fromY;j<=toY;j++)
			ModifyMap(i,j,landscape);
	}
	return(1);
}
int ReadMapFromFile(MAPTILE **&map, LPSTR szFileName, int &width, int &height)
{
	int i;
	FILE *fp;
	if ((fp = fopen(szFileName, "r"))==NULL)
	{
		MessageBox(hWnd, "No such file in the directory!", "File Open Error", MB_OK);
		return(0);
	}
	fscanf(fp, "%d %d", &width, &height);
	
	// allocating memory for the map
	map = new MAPTILE*[width];
	for(i=0; i<width; i++)
		map[i] = new MAPTILE[height];

	for (int j=0; j<height; j++)
	{
		fscanf(fp,"\n");
		for (i=0; i<width-1; i++)
		{
			fscanf(fp,"%d,%d,%d,%d", &(map[i][j].landtype),&(map[i][j].objtype),&(map[i][j].index),&(map[i][j].numFrame));
		}
		fscanf(fp, "%d,%d,%d,%d", &(map[width-1][j].landtype),&(map[width-1][j].objtype),&(map[width-1][j].index),&(map[width-1][j].numFrame));
	}

	fclose(fp);

	return(1);
}

int WriteMapToFile(MAPTILE **map, LPSTR szFileName, int width, int height)
{
	int i;
	FILE *fp;
	if((fp = fopen(szFileName, "w"))==NULL)
		return(0);
	fprintf(fp,"%d %d", width, height);

	for (int j=0; j<height; j++)
	{
		fprintf(fp,"\n");
		for (i=0; i<width-1; i++)
		{
			fprintf(fp, "%d,%d,%d,%d ", map[i][j].landtype, map[i][j].objtype, map[i][j].index, map[i][j].numFrame);
			
		}
		fprintf(fp, "%d,%d,%d,%d", map[width-1][j].landtype, map[width-1][j].objtype, map[width-1][j].index, map[width-1][j].numFrame);

	}

	fclose(fp);
	
	return(1);
}


int DrawMap(int left, int top) 
{

	int i;
	PAINTSTRUCT paintstruct;

	hdc = BeginPaint(hWnd, &paintstruct);
	memDC = CreateCompatibleDC(hdc);

	for (int j=top; j<top+heightTile; j++)
	{
		if(j == mapHeight)
			break;
		for (i=left; i<left+widthTile; i++)
		{
			if(i == mapWidth)
				break;
			switch (editMap[i][j].landtype)
			{
			case GRASS:
				SelectObject(memDC, hGrass);
				BitBlt(hdc, ((i-left)*CELL_WIDTH), ((j-top)*CELL_WIDTH),
					CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
				break;
			case WATER:
				SelectObject(memDC, hWater);
				BitBlt(hdc, ((i-left)*CELL_WIDTH), ((j-top)*CELL_WIDTH),
					CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
				break;
			case VILLAGE:
				SelectObject(memDC, hVillage);
				BitBlt(hdc, ((i-left)*CELL_WIDTH), ((j-top)*CELL_WIDTH),
					CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
				break;
			case POISONLAND:
				SelectObject(memDC, hPoisonLand);
				BitBlt(hdc, ((i-left)*CELL_WIDTH), ((j-top)*CELL_WIDTH),
					CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
				break;
			case RESTORELAND:
				SelectObject(memDC, hRestoreLand);
				BitBlt(hdc, ((i-left)*CELL_WIDTH), ((j-top)*CELL_WIDTH),
					CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
				break;
			}
		}
	}
	DeleteDC(memDC);
	EndPaint(hWnd, &paintstruct);
	
	return(1);
}

int AnimateMap()
{
	int i;
	int left = upperLeftCorner[0];
	int top = upperLeftCorner[1];
	int needRedraw = 0;
	int tempX;
	int tempY;

	int verticalDisplayOffset = 0;
	int horizontalDisplayOffset = 0;
	int initVertDisplay = 0;
	int initHoriDisplay = 0;
	int strangeTemp[2] = {0,0};

	if(moveState == STAY)
	{
		DrawMap(left, top);
		return(1);
	}
	hdc = GetDC(hWnd);
	memDC = CreateCompatibleDC(hdc);

	switch(moveState)
	{
	case DOWN:
		strangeTemp[1] = -1;
		verticalDisplayOffset = -DISPLAYOFFAMOUNT;
		initVertDisplay = -DISPLAYOFFAMOUNT;
		break;
	case UP:
		verticalDisplayOffset = DISPLAYOFFAMOUNT;
		initVertDisplay = verticalDisplayOffset-CELL_WIDTH;
		break;
	case LEFT:
		horizontalDisplayOffset = DISPLAYOFFAMOUNT;
		initHoriDisplay = horizontalDisplayOffset-CELL_WIDTH;
		break;
	case RIGHT:
		strangeTemp[0] = -1;
		horizontalDisplayOffset = -DISPLAYOFFAMOUNT;
		initHoriDisplay = -DISPLAYOFFAMOUNT;
		break;

	}
	
	for(int rep=0; rep<CELL_WIDTH/DISPLAYOFFAMOUNT; rep++)
	{
		for (int j=initVertDisplay; j<480; j+=CELL_WIDTH)
		{
			if((tempY=top+(j-initVertDisplay)/CELL_WIDTH+strangeTemp[1])==mapHeight)
			{
				needRedraw = 1;
				break;
			}
			for (i=initHoriDisplay; i<640; i+=CELL_WIDTH)
			{
				if((tempX=left+(i-initHoriDisplay)/CELL_WIDTH+strangeTemp[0]) == mapWidth)
				{
					needRedraw = 1;
					break;
				}
				switch (editMap[tempX][tempY].landtype)
				{
				case GRASS:
					SelectObject(memDC, hGrass);
					BitBlt(hdc, i, j,
						CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
					break;
				case WATER:
					SelectObject(memDC, hWater);
					BitBlt(hdc, i, j,
						CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
					break;
				case VILLAGE:
					SelectObject(memDC, hVillage);
					BitBlt(hdc, i, j,
						CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
					break;
				case POISONLAND:
					SelectObject(memDC, hPoisonLand);
					BitBlt(hdc, i, j,
						CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
					break;
				case RESTORELAND:
					SelectObject(memDC, hRestoreLand);
					BitBlt(hdc, i, j,
						CELL_WIDTH, CELL_WIDTH, memDC, 0, 0, SRCCOPY);
					break;

				}
			}
		}
		initHoriDisplay+=horizontalDisplayOffset;
		initVertDisplay+=verticalDisplayOffset;
	}
	DeleteDC(memDC);
	ReleaseDC(hWnd, hdc);

	moveState = STAY;
	if(needRedraw)
		InvalidateRect(hWnd, NULL, TRUE);
	return(1);
}

int DeleteRowCol(int x, int y)
{
	int i, j;
	MAPTILE **tempMap;
	tempMap = new MAPTILE*[mapWidth];
	for(i=0; i<mapWidth; i++)
		tempMap[i] = new MAPTILE[mapHeight];

	for(i=0; i<mapWidth; i++)
	{
		for(j=0; j<mapHeight; j++)
		{
			tempMap[i][j].landtype = editMap[i][j].landtype;
			tempMap[i][j].objtype = editMap[i][j].objtype;
			tempMap[i][j].index = editMap[i][j].index;
			tempMap[i][j].numFrame = editMap[i][j].numFrame;
		}
	}
	if(x==-1)
		mapHeight--;
	else
		mapWidth--;

	editMap = new MAPTILE*[mapWidth];
	for(i=0; i<mapWidth; i++)
		editMap[i] = new MAPTILE[mapHeight];

	if(x == -1)
	{
		for(i=0; i<mapWidth; i++)
		{
			for(j=0; j<y; j++)
			{
				editMap[i][j].landtype = tempMap[i][j].landtype;
				editMap[i][j].objtype = tempMap[i][j].objtype;
				editMap[i][j].index = tempMap[i][j].index;
				editMap[i][j].numFrame = tempMap[i][j].numFrame;
			}
			for(j=y; j<mapHeight; j++)
			{
				editMap[i][j].landtype = tempMap[i][j+1].landtype;
				editMap[i][j].objtype = tempMap[i][j+1].objtype;
				editMap[i][j].index = tempMap[i][j+1].index;
				editMap[i][j].numFrame = tempMap[i][j+1].numFrame;
			}
		}
	}
	else
	{
		for(j=0; j<mapHeight; j++)
		{
			for(i=0; i<x; i++)
			{
				editMap[i][j].landtype = tempMap[i][j].landtype;
				editMap[i][j].objtype = tempMap[i][j].objtype;
				editMap[i][j].index = tempMap[i][j].index;
				editMap[i][j].numFrame = tempMap[i][j].numFrame;
			}
			for(i=x; i<mapWidth; i++)
			{
				editMap[i][j].landtype = tempMap[i+1][j].landtype;
				editMap[i][j].objtype = tempMap[i+1][j].objtype;
				editMap[i][j].index = tempMap[i+1][j].index;
				editMap[i][j].numFrame = tempMap[i+1][j].numFrame;
			}
		}
	}

	delete tempMap;
	return(1);
}

int InsertRowCol(int x, int y)
{
	if((curLandscape>=NUM_OF_LANDSCAPES)||(curLandscape<0))
		MessageBox(hWnd, "Error! Landscape not probably chosen!","Insert Error", MB_OK);
	int i, j;
	MAPTILE **tempMap;

	tempMap = new MAPTILE*[mapWidth];
	for(i=0; i<mapWidth; i++)
		tempMap[i] = new MAPTILE[mapHeight];
	
	// copy the original map to tempMap
	for(i=0; i<mapWidth; i++)
	{
		for(j=0; j<mapHeight; j++)
		{
			tempMap[i][j].landtype = editMap[i][j].landtype;
			tempMap[i][j].objtype = editMap[i][j].objtype;
			tempMap[i][j].index = editMap[i][j].index;
			tempMap[i][j].numFrame = editMap[i][j].numFrame;
		}
	}

	// increment the map height and width
	if(x == -1)
		mapHeight++;
	else
		mapWidth++;

	// initialize the new map
	editMap = new MAPTILE*[mapWidth];
	for(i=0; i<mapWidth; i++)
		editMap[i] = new MAPTILE[mapHeight];

	// copy the data back
	if(x == -1)
	{
		if (insertType == AFTER)
			y++;
		for(i=0; i<mapWidth; i++)
		{
			for(j=0; j<y; j++)
			{
				editMap[i][j].landtype = tempMap[i][j].landtype;
				editMap[i][j].objtype = tempMap[i][j].objtype;
				editMap[i][j].index = tempMap[i][j].index;
				editMap[i][j].numFrame = tempMap[i][j].numFrame;
			}
			if (curLandscape == WATER)
				editMap[i][y].numFrame = NUMWATERFRAME;
			else
				editMap[i][y].numFrame = 1;
			editMap[i][y].landtype = curLandscape;
			editMap[i][y].objtype = 0;
			editMap[i][y].index = 0;
			for(j=y+1; j<mapHeight; j++)
			{
				editMap[i][j].landtype = tempMap[i][j-1].landtype;
				editMap[i][j].objtype = tempMap[i][j-1].objtype;
				editMap[i][j].index = tempMap[i][j-1].index;
				editMap[i][j].numFrame = tempMap[i][j-1].numFrame;
			}
		}
	}
	else
	{
		if(insertType == AFTER)
			x++;

		for(j=0; j<mapHeight; j++)
		{
			for(i=0; i<x; i++)
			{
				editMap[i][j].landtype = tempMap[i][j].landtype;
				editMap[i][j].objtype = tempMap[i][j].objtype;
				editMap[i][j].index = tempMap[i][j].index;
				editMap[i][j].numFrame = tempMap[i][j].numFrame;
			}
			if (curLandscape == WATER)
				editMap[x][j].numFrame = NUMWATERFRAME;
			else
				editMap[x][j].numFrame = 1;
			editMap[x][j].landtype = curLandscape;
			editMap[x][j].objtype = 0;
			editMap[x][j].index = 0;
			for(i=x+1; i<mapWidth; i++)
			{
				editMap[i][j].landtype = tempMap[i-1][j].landtype;
				editMap[i][j].objtype = tempMap[i-1][j].objtype;
				editMap[i][j].index = tempMap[i-1][j].index;
				editMap[i][j].numFrame = tempMap[i-1][j].numFrame;
			}
		}
	}

	delete tempMap;

	insertType = 0;
	return(1);
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst,
				   LPSTR lpszArgs, int nWinMode)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wcl;

	/* Define a window class */
	wcl.hInstance = hThisInst;	/* handle to this instance */
	wcl.lpszClassName = szWinName;	/* window class name */
	wcl.lpfnWndProc = WindowFunc;	/* window function */
	wcl.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;	/* default style */

	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);	/* icon style */
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);	/* cursor style */
	wcl.lpszMenuName = "MYMENU";	/* main menu */

	wcl.cbClsExtra = 0;	/* no extra */
	wcl.cbWndExtra = 0; /* information needed */

	/* Make the window background white. */
	wcl.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);

	/* Register the window class. */
	if(!RegisterClass(&wcl)) return 0;

	/* After registering, a window can be created. */
	hwnd = CreateWindow(
		szWinName,	/* name of window class */
		"Map Editor V2.0",	/* title */
		WS_OVERLAPPEDWINDOW ^ WS_SIZEBOX,	/* window style - normal */
		CW_USEDEFAULT,	/* X coordinate - let Windows decide */
		CW_USEDEFAULT,	/* Y coordinate - let Windows decide */
		640,	/* width - let Windows decide */
		480+CELL_WIDTH,	/* height - let Windows decide */
		HWND_DESKTOP,	/* no parent window */
		NULL,	/* no menu */
		hThisInst, /* handle of this instance of the program */
		NULL	/* no additional arguments */
	);

	hInst = hThisInst;
	hWnd = hwnd;

	hGrass = (HBITMAP)LoadImage(NULL, "..\\Bitmaps\\Landscapes\\grassLand.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	hWater = (HBITMAP)LoadImage(NULL, "..\\Bitmaps\\Landscapes\\water.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	hVillage = (HBITMAP)LoadImage(NULL, "..\\Bitmaps\\Landscapes\\village.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	hPoisonLand = (HBITMAP)LoadImage(NULL, "..\\Bitmaps\\Landscapes\\poisonLand.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	hRestoreLand = (HBITMAP)LoadImage(NULL, "..\\Bitmaps\\Landscapes\\restoreLand.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	/* Display the window */
	ShowWindow(hwnd, nWinMode);
	UpdateWindow(hwnd);

	/* Create the message loop */
	while(GetMessage(&msg, NULL, 0, 0))
	{
		/* check if keyboard accelerators are pressed */
		TranslateMessage(&msg);	/* allow use of keyboard */
		DispatchMessage(&msg);	/* return control to Windows */
	}
	return msg.wParam;
}


/* This function is called by Windows and is passed
	messages from the message queue. */
LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int response;

	switch(message) {
		case WM_COMMAND:	/* process menu options */
			switch (LOWORD(wParam)) {
			case IDM_EXIT:
				response = MessageBox(hwnd, "Really wanna Exit?", "Exit Confirmation", MB_OKCANCEL);
				switch(response) {
				case IDOK:
					PostQuitMessage(0);
					break;
				}
				break;
			case IDM_OPEN:
				if((!(mapWidth<=0))&&(!(mapHeight<=0)))
				{
					response = MessageBox(hWnd, "Map not yet saved. Save Map?",
						"Map Not Saved", MB_OKCANCEL);
					if (response == IDOK)
						DialogBox(hInst, "FILESAVEDLG", hWnd, (FARPROC)FileSaveDlg);
				}

				DialogBox(hInst, "FILEOPENDLG", hwnd, (FARPROC)FileOpenDlg);
				break;
			case IDM_SAVE:
				DialogBox(hInst, "FILESAVEDLG", hwnd, (FARPROC)FileSaveDlg);
				break;
			case IDM_CUSTOM:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				DialogBox(hInst, "CUSTOMDLG", hwnd, (FARPROC)CustomDlg);
				break;
			case IDM_LANDSCAPE:
				DialogBox(hInst, "LANDSCAPEDLG", hwnd, (FARPROC)LandscapeDlg);
				break;
			case IDM_NONEACTIVE:
				cursorMode = NONEACTIVE;
				break;
			case IDM_MODIFY:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				cursorMode = MODIFY;
				break;
			case IDM_DELETEROW:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				cursorMode = DELETEROW;
				break;
			case IDM_DELETECOL:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				cursorMode = DELETECOL;
				break;
			case IDM_INSERTROW:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				cursorMode = INSERTROW;
				break;
			case IDM_INSERTCOL:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				cursorMode = INSERTCOL;
				break;
			case IDM_GOTO:
				if ((mapWidth<=0)||(mapHeight<=0))
				{
					MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
					break;
				}
				DialogBox(hInst, "GOTODLG", hwnd, (FARPROC)GotoDlg);
				break;
			case IDM_HELP:
				sprintf(buf, "Map Width = %d; Map Height = %d\nUpperleft Corner = {%d, %d}",
					mapWidth, mapHeight, upperLeftCorner[0], upperLeftCorner[1]);
				MessageBox(hWnd, buf, "Help", MB_OK);
				break;
			}
			break;
		case WM_LBUTTONDBLCLK:
			if (cursorMode == MODIFY)
			{
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				ModifyMap(upperLeftCorner[0]+mouseX/CELL_WIDTH,
					upperLeftCorner[1]+mouseY/CELL_WIDTH, curLandscape);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			if (cursorMode == DELETEROW)
			{
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				response = MessageBox(hWnd, "Really delete this row?",
					"Confirm Delete Row", MB_OKCANCEL);
				if(response == IDOK)
				{
					DeleteRowCol(-1,
					upperLeftCorner[1]+mouseY/CELL_WIDTH);
					InvalidateRect(hwnd,NULL,TRUE);
				}
			}
			if (cursorMode == DELETECOL)
			{
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				response = MessageBox(hWnd, "Really delete this column?",
					"Confirm Delete Column", MB_OKCANCEL);
				if(response == IDOK)
				{
					DeleteRowCol(upperLeftCorner[0]+mouseX/CELL_WIDTH,
					-1);
					InvalidateRect(hwnd,NULL,TRUE);
				}
			}
			if (cursorMode == INSERTROW)
			{
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				DialogBox(hInst, "INSERTDLG", hwnd, (FARPROC)InsertDlg);
				if(insertType == 0)
					MessageBox(hWnd, "Invalid entry!","Insert Error",MB_OK);
				else
				{
					response = MessageBox(hWnd, "Really insert this row?",
						"Confirm Insert Row", MB_OKCANCEL);
					if(response == IDOK)
					{
						InsertRowCol(-1, upperLeftCorner[1]+mouseY/CELL_WIDTH);
						InvalidateRect(hwnd,NULL,TRUE);
					}
				}
			}
			if (cursorMode == INSERTCOL)
			{
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				DialogBox(hInst, "INSERTDLG", hwnd, (FARPROC)InsertDlg);
				if(insertType == 0)
					MessageBox(hWnd, "Invalid entry!","Insert Error",MB_OK);
				else
				{
					response = MessageBox(hWnd, "Really insert this column?",
						"Confirm Insert Row", MB_OKCANCEL);
					if(response == IDOK)
					{
						InsertRowCol(upperLeftCorner[0]+mouseX/CELL_WIDTH, -1);
						InvalidateRect(hwnd,NULL,TRUE);
					}
				}
			}

			break;
		case WM_KEYDOWN:
			if ((mapWidth<=0)||(mapHeight<=0))
			{
				MessageBox(hWnd, "Map not opened yet!","Map Error", MB_OK);
				break;
			}

			switch((int)wParam)
			{
			case VK_DOWN:
				if(upperLeftCorner[1]==(mapHeight-1))
					break;
				upperLeftCorner[1]++;
				moveState = DOWN;
				AnimateMap();
				break;
			case VK_UP:
				if(upperLeftCorner[1]==0)
					break;
				upperLeftCorner[1]--;
				moveState = UP;
				AnimateMap();
				break;
			case VK_LEFT:
				if(upperLeftCorner[0]==0)
					break;
				upperLeftCorner[0]--;
				moveState = LEFT;
				AnimateMap();
				break;
			case VK_RIGHT:
				if(upperLeftCorner[0]==(mapWidth-1))
					break;
				upperLeftCorner[0]++;
				moveState = RIGHT;
				AnimateMap();
				break;
			case VK_PRIOR:
/*				if((upperLeftCorner[0]-=16)<0)
					upperLeftCorner[0] = 0;
*/				if((upperLeftCorner[1]-=12)<0)
					upperLeftCorner[1] = 0;
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			case VK_NEXT:
/*				if((upperLeftCorner[0]+=16)>=mapWidth)
					upperLeftCorner[0] = mapWidth-1;
*/				if((upperLeftCorner[1]+=12)>=mapHeight)
					upperLeftCorner[1] = mapHeight-1;
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			}
			break;
		case WM_PAINT:
			DrawMap(upperLeftCorner[0], upperLeftCorner[1]);
			break;
		case WM_DESTROY:	/* terminate the program */
			PostQuitMessage(0);
			break;
		default:
			/* Let Windows process any messages not specified in
			the preceding switch statement. */
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}


BOOL CALLBACK FileOpenDlg(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					GetDlgItemText(hdwnd, IDC_EDIT1, buf, 80);
					strcpy(szFileName, sourceDir);
					strcat(szFileName, buf);
					ReadMapFromFile(editMap, szFileName, mapWidth, mapHeight);
					InvalidateRect(hWnd, NULL, TRUE);
					EndDialog(hdwnd, 0);
					return 1;
				case IDCANCEL:
					EndDialog(hdwnd, 0);
					return 1;
			}
	}
	return 0;
}

BOOL CALLBACK FileSaveDlg(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	int response;
	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					response = MessageBox(hWnd, "Are u sure u wanna save?",
						"File Save Confirmation", MB_OKCANCEL);
					if(response==IDOK)
					{
						GetDlgItemText(hdwnd, IDC_EDIT1, buf, 80);
						strcpy(szFileName, sourceDir);
						strcat(szFileName, buf);
						WriteMapToFile(editMap, szFileName, mapWidth, mapHeight);
						InvalidateRect(hWnd, NULL, TRUE);
					}
					EndDialog(hdwnd, 0);
					return 1;
				case IDCANCEL:
					EndDialog(hdwnd, 0);
					return 1;
			}
	}
	return 0;
}

BOOL CALLBACK CustomDlg(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	int land;
	int fromX, fromY, toX, toY;

	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			SendDlgItemMessage(hdwnd, IDC_LIST2,
							   LB_ADDSTRING, 0, (LPARAM)"Grass");
			SendDlgItemMessage(hdwnd, IDC_LIST2,
							   LB_ADDSTRING, 0, (LPARAM)"Water");
			SendDlgItemMessage(hdwnd, IDC_LIST2,
							   LB_ADDSTRING, 0, (LPARAM)"Village");
			SendDlgItemMessage(hdwnd, IDC_LIST2,
							   LB_ADDSTRING, 0, (LPARAM)"Poison land");
			SendDlgItemMessage(hdwnd, IDC_LIST2,
							   LB_ADDSTRING, 0, (LPARAM)"Restore land");

			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_LIST2:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						land = SendDlgItemMessage(hdwnd, IDC_LIST2,
											   LB_GETCURSEL, 0, 0L); // get index
					}
					return 1;

			case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					GetDlgItemText(hdwnd, IDC_EDIT1, buf, 80);
					sscanf(buf, "%d", &fromX);
					GetDlgItemText(hdwnd, IDC_EDIT2, buf, 80);
					sscanf(buf, "%d", &fromY);
					GetDlgItemText(hdwnd, IDC_EDIT3, buf, 80);
					sscanf(buf, "%d", &toX);
					GetDlgItemText(hdwnd, IDC_EDIT4, buf, 80);
					sscanf(buf, "%d", &toY);

					land = SendDlgItemMessage(hdwnd, IDC_LIST2,
										   LB_GETCURSEL, 0, 0L); // get index

					if((fromX<0)||(fromY<0)||(land<0)||(land>NUM_OF_LANDSCAPES)
						||(toX>=(mapWidth+1000))||(toY>=(mapHeight+1000)))
					{
						MessageBox(hWnd, "Invalid Entry!", "Entry Error", MB_OK);
						EndDialog(hdwnd, 0);
						return 1;
					}
					ModifyRangeMap(fromX, toX, fromY, toY, land);
					InvalidateRect(hWnd, NULL, TRUE);
					EndDialog(hdwnd, 0);
					return 1;
			case IDCANCEL:
					EndDialog(hdwnd, 0);
					return 1;
			}
	}
	return 0;
}

BOOL CALLBACK LandscapeDlg(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"Grass");
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"Water");
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"Village");
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"Poison land");
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"Restore land");
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_LIST1:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						curLandscape = SendDlgItemMessage(hdwnd, IDC_LIST1,
											   LB_GETCURSEL, 0, 0L); // get index
						EndDialog(hdwnd, 0);
					}
					return 1;

			case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					curLandscape = SendDlgItemMessage(hdwnd, IDC_LIST1,
										   LB_GETCURSEL, 0, 0L); // get index
					EndDialog(hdwnd, 0);
					return 1;
			case IDCANCEL:
					EndDialog(hdwnd, 0);
					return 1;
			}
	}
	return 0;
}

BOOL CALLBACK GotoDlg(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	int x, y;
	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					GetDlgItemText(hdwnd, IDC_EDIT1, buf, 80);
					sscanf(buf, "%d", &x);
					GetDlgItemText(hdwnd, IDC_EDIT2, buf, 80);
					sscanf(buf, "%d", &y);
					if((x<0)||(y<0)||(x>=(mapWidth+1000))||(y>=(mapHeight+1000)))
					{
						MessageBox(hWnd, "Invalid Entry!", "Entry Error", MB_OK);
						EndDialog(hdwnd, 0);
						return 1;
					}
					if(x>=mapWidth)
						x = mapWidth-1;
					if(y>=mapHeight)
						y = mapHeight-1;
					upperLeftCorner[0] = x;
					upperLeftCorner[1] = y;
					InvalidateRect(hWnd, NULL, TRUE);
					EndDialog(hdwnd, 0);
					return 1;
				case IDCANCEL:
					EndDialog(hdwnd, 0);
					return 1;
			}
	}
	return 0;
}

BOOL CALLBACK InsertDlg(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	int tempType;
	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"Before");
			SendDlgItemMessage(hdwnd, IDC_LIST1,
							   LB_ADDSTRING, 0, (LPARAM)"After");
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_LIST1:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						tempType = SendDlgItemMessage(hdwnd, IDC_LIST1,
											   LB_GETCURSEL, 0, 0L); // get index
						if (tempType == 0)
							insertType = AFTER;
						else
						{
							if(tempType == 1)
								insertType = BEFORE;
						}
						EndDialog(hdwnd, 0);
					}
					return 1;

			case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					tempType = SendDlgItemMessage(hdwnd, IDC_LIST1,
										   LB_GETCURSEL, 0, 0L); // get index
					if (tempType == 0)
						insertType = AFTER;
					else
					{
						if(tempType == 1)
							insertType = BEFORE;
						else
						{
							if(tempType == -1)
								insertType = 0;
						}
					}
					EndDialog(hdwnd, 0);
					return 1;
			}
	}
	return 0;
}
