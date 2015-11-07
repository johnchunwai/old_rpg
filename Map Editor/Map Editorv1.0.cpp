/* Message Box and Menus. */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"

#define DISPLAYOFFAMOUNT	10

#define UP		1
#define DOWN	2
#define LEFT	3
#define RIGHT	4
#define STAY	0

#define CELLWIDTH	40
#define LANDSCAPENUM 3
#define GRASS		0
#define WATER		1
#define VILLAGE		2

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
BOOL CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
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
HBITMAP hGrass, hWater, hVillage;
HWND	hWnd;

char    buf[80];
char	szFileName[80];
char	*sourceDir = "e:\\RPG V1.0\\Map\\";

int **editMap;
int mapWidth=0, mapHeight=0;
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
	if ((x<0)||(y<0)||(x>=mapWidth)||(y>=mapHeight)||(landscape>=LANDSCAPENUM)||(landscape<0))
		return(0);
	editMap[x][y] = landscape;
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
int ReadMapFromFile(int **&map, LPSTR szFileName, int &width, int &height)
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

int WriteMapToFile(int **map, LPSTR szFileName, int width, int height)
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
			fprintf(fp,"%d ", map[i][j]);
		fprintf(fp, "%d", map[width-1][j]);
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
			switch (editMap[i][j])
			{
			case GRASS:
				SelectObject(memDC, hGrass);
				BitBlt(hdc, ((i-left)*CELLWIDTH), ((j-top)*CELLWIDTH),
					CELLWIDTH, CELLWIDTH, memDC, 0, 0, SRCCOPY);
				break;
			case WATER:
				SelectObject(memDC, hWater);
				BitBlt(hdc, ((i-left)*CELLWIDTH), ((j-top)*CELLWIDTH),
					CELLWIDTH, CELLWIDTH, memDC, 0, 0, SRCCOPY);
				break;
			case VILLAGE:
				SelectObject(memDC, hVillage);
				BitBlt(hdc, ((i-left)*CELLWIDTH), ((j-top)*CELLWIDTH),
					CELLWIDTH, CELLWIDTH, memDC, 0, 0, SRCCOPY);
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
		initVertDisplay = verticalDisplayOffset-CELLWIDTH;
		break;
	case LEFT:
		horizontalDisplayOffset = DISPLAYOFFAMOUNT;
		initHoriDisplay = horizontalDisplayOffset-CELLWIDTH;
		break;
	case RIGHT:
		strangeTemp[0] = -1;
		horizontalDisplayOffset = -DISPLAYOFFAMOUNT;
		initHoriDisplay = -DISPLAYOFFAMOUNT;
		break;

	}
	
	for(int rep=0; rep<CELLWIDTH/DISPLAYOFFAMOUNT; rep++)
	{
		for (int j=initVertDisplay; j<480; j+=CELLWIDTH)
		{
			if((tempY=top+(j-initVertDisplay)/CELLWIDTH+strangeTemp[1])==mapHeight)
			{
				needRedraw = 1;
				break;
			}
			for (i=initHoriDisplay; i<640; i+=CELLWIDTH)
			{
				if((tempX=left+(i-initHoriDisplay)/CELLWIDTH+strangeTemp[0]) == mapWidth)
				{
					needRedraw = 1;
					break;
				}
				switch (editMap[tempX][tempY])
				{
				case GRASS:
					SelectObject(memDC, hGrass);
					BitBlt(hdc, i, j,
						CELLWIDTH, CELLWIDTH, memDC, 0, 0, SRCCOPY);
					break;
				case WATER:
					SelectObject(memDC, hWater);
					BitBlt(hdc, i, j,
						CELLWIDTH, CELLWIDTH, memDC, 0, 0, SRCCOPY);
					break;
				case VILLAGE:
					SelectObject(memDC, hVillage);
					BitBlt(hdc, i, j,
						CELLWIDTH, CELLWIDTH, memDC, 0, 0, SRCCOPY);
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
	int **tempMap;
	tempMap = new int*[mapWidth];
	for(i=0; i<mapWidth; i++)
		tempMap[i] = new int[mapHeight];

	for(i=0; i<mapWidth; i++)
	{
		for(j=0; j<mapHeight; j++)
			tempMap[i][j] = editMap[i][j];
	}
	if(x==-1)
		mapHeight--;
	else
		mapWidth--;

	editMap = new int*[mapWidth];
	for(i=0; i<mapWidth; i++)
		editMap[i] = new int[mapHeight];

	if(x == -1)
	{
		for(i=0; i<mapWidth; i++)
		{
			for(j=0; j<y; j++)
				editMap[i][j] = tempMap[i][j];
			for(j=y; j<mapHeight; j++)
				editMap[i][j] = tempMap[i][j+1];
		}
	}
	else
	{
		for(j=0; j<mapHeight; j++)
		{
			for(i=0; i<x; i++)
				editMap[i][j] = tempMap[i][j];
			for(i=x; i<mapWidth; i++)
				editMap[i][j] = tempMap[i+1][j];
		}
	}

	delete tempMap;
	return(1);
}

int InsertRowCol(int x, int y)
{
	if((curLandscape>=LANDSCAPENUM)||(curLandscape<0))
		MessageBox(hWnd, "Error! Landscape not probably chosen!","Insert Error", MB_OK);
/*	if (x==-1)
		sprintf(buf,"Insert Row %d, %d, %d,%d", x,y, insertType,curLandscape);
	else
		sprintf(buf,"Insert Col %d, %d, %d,%d", x,y,insertType,curLandscape);
	MessageBox(hWnd, buf,"",MB_OK);
*/	int i, j;
	int **tempMap;

	tempMap = new int*[mapWidth];
	for(i=0; i<mapWidth; i++)
		tempMap[i] = new int[mapHeight];
	
	// copy the original map to tempMap
	for(i=0; i<mapWidth; i++)
	{
		for(j=0; j<mapHeight; j++)
			tempMap[i][j] = editMap[i][j];
	}

	// increment the map height and width
	if(x == -1)
		mapHeight++;
	else
		mapWidth++;

	// initialize the new map
	editMap = new int*[mapWidth];
	for(i=0; i<mapWidth; i++)
		editMap[i] = new int[mapHeight];

	// copy the data back
	if(x == -1)
	{
		if (insertType == AFTER)
			y++;
		for(i=0; i<mapWidth; i++)
		{
			for(j=0; j<y; j++)
				editMap[i][j] = tempMap[i][j];
			editMap[i][y] = curLandscape;
			for(j=y+1; j<mapHeight; j++)
				editMap[i][j] = tempMap[i][j-1];
		}
	}
	else
	{
		if(insertType == AFTER)
			x++;

		for(j=0; j<mapHeight; j++)
		{
			for(i=0; i<x; i++)
				editMap[i][j] = tempMap[i][j];
			editMap[x][j] = curLandscape;
			for(i=x+1; i<mapWidth; i++)
				editMap[i][j] = tempMap[i-1][j];
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
		"Map Editor V1.0",	/* title */
		WS_OVERLAPPEDWINDOW ^ WS_SIZEBOX,	/* window style - normal */
		CW_USEDEFAULT,	/* X coordinate - let Windows decide */
		CW_USEDEFAULT,	/* Y coordinate - let Windows decide */
		640,	/* width - let Windows decide */
		480,	/* height - let Windows decide */
		HWND_DESKTOP,	/* no parent window */
		NULL,	/* no menu */
		hThisInst, /* handle of this instance of the program */
		NULL	/* no additional arguments */
	);

	hInst = hThisInst;
	hWnd = hwnd;

	hGrass = (HBITMAP)LoadImage(NULL, "e:\\RPG V1.0\\Bitmaps\\Landscapes\\grassLand.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	hWater = (HBITMAP)LoadImage(NULL, "e:\\RPG V1.0\\Bitmaps\\Landscapes\\water.bmp",
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	hVillage = (HBITMAP)LoadImage(NULL, "e:\\RPG V1.0\\Bitmaps\\Landscapes\\village.bmp",
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
				DialogBox(hInst, "FILEOPENDLG", hwnd, (FARPROC)FileOpenDlg);
				break;
			case IDM_SAVE:
				DialogBox(hInst, "FILESAVEDLG", hwnd, (FARPROC)FileSaveDlg);
				break;
			case IDM_CUSTOM:
				DialogBox(hInst, "CUSTOMDLG", hwnd, (FARPROC)CustomDlg);
				break;
			case IDM_LANDSCAPE:
				DialogBox(hInst, "LANDSCAPEDLG", hwnd, (FARPROC)LandscapeDlg);
				break;
			case IDM_NONEACTIVE:
				cursorMode = NONEACTIVE;
				break;
			case IDM_MODIFY:
				cursorMode = MODIFY;
				break;
			case IDM_DELETEROW:
				cursorMode = DELETEROW;
				break;
			case IDM_DELETECOL:
				cursorMode = DELETECOL;
				break;
			case IDM_INSERTROW:
				cursorMode = INSERTROW;
				break;
			case IDM_INSERTCOL:
				cursorMode = INSERTCOL;
				break;
			case IDM_GOTO:
				DialogBox(hInst, "GOTODLG", hwnd, (FARPROC)GotoDlg);
				break;
			case IDM_DIALOG1:
				DialogBox(hInst, "MYDB", hwnd, (FARPROC)DialogProc);
				break;
			case IDM_DIALOG2:	
				MessageBox(hwnd, "Not implemented yet.", "", MB_OK);
				break;
			case IDM_HELP:	
				MessageBox(hwnd, "Help", "Help", MB_OK);
				break;
			}
			break;
		case WM_LBUTTONDBLCLK:
			if (cursorMode == MODIFY)
			{
				mouseX = LOWORD(lParam);
				mouseY = HIWORD(lParam);
				ModifyMap(upperLeftCorner[0]+mouseX/CELLWIDTH,
					upperLeftCorner[1]+mouseY/CELLWIDTH, curLandscape);
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
					upperLeftCorner[1]+mouseY/CELLWIDTH);
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
					DeleteRowCol(upperLeftCorner[0]+mouseX/CELLWIDTH,
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
						InsertRowCol(-1, upperLeftCorner[1]+mouseY/CELLWIDTH);
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
						InsertRowCol(upperLeftCorner[0]+mouseX/CELLWIDTH, -1);
						InvalidateRect(hwnd,NULL,TRUE);
					}
				}
			}

			break;
		case WM_KEYDOWN:
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

BOOL CALLBACK DialogProc(HWND hdwnd, UINT message,
					 WPARAM wParam, LPARAM lParam)
{
	long i;
	char str[80];
	switch(message)
	{
		case WM_INITDIALOG:	// initialize list box
			SendDlgItemMessage(hdwnd, ID_LB1,
							   LB_ADDSTRING, 0, (LPARAM)"Apple");
			SendDlgItemMessage(hdwnd, ID_LB1,
							   LB_ADDSTRING, 0, (LPARAM)"Orange");
			SendDlgItemMessage(hdwnd, ID_LB1,
							   LB_ADDSTRING, 0, (LPARAM)"Pear");
			SendDlgItemMessage(hdwnd, ID_LB1,
							   LB_ADDSTRING, 0, (LPARAM)"Grape");

			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					GetDlgItemText(hdwnd, ID_EB1, str, 80);
					MessageBox(hdwnd, str, "Edit Box Contains", MB_OK);
					return 1;
				case ID_LB1:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						i = SendDlgItemMessage(hdwnd, ID_LB1,
											   LB_GETCURSEL, 0, 0L); // get index
						if (i > -1) sprintf(str, "Index in list is: %d", i);
						else sprintf(str, "No Fruit Selected");
						MessageBox(hdwnd, str, "Selection Made", MB_OK);
					}
					return 1;
				case IDD_SELFRUIT:	/* Select Fruit has been pressed */
					i = SendDlgItemMessage(hdwnd, ID_LB1,
										   LB_GETCURSEL, 0, 0); // get index
					if (i > -1) sprintf(str, "Index in list is: %d", i);
					else sprintf(str, "No Fruit Selected");
					MessageBox(hdwnd, str, "Selection Made", MB_OK);
					return 1;
				case IDD_RED:
					MessageBox(hdwnd, "You Picked Red", "RED", MB_OK);
					return 1;
				case IDD_GREEN:
					MessageBox(hdwnd, "You Picked Green", "GREEN", MB_OK);
					return 1;
				case IDCANCEL:
					EndDialog(hdwnd, 0);
					return 1;
			}
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
					WriteMapToFile(editMap, szFileName, mapWidth, mapHeight);
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
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_LIST2:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						land = SendDlgItemMessage(hdwnd, IDC_LIST2,
											   LB_GETCURSEL, 0, 0L); // get index
						sprintf(buf, "Index in list is: %d", land);
						MessageBox(hdwnd, buf, "Selection Made", MB_OK);
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
					if (land == 1)
						land = 2;
					else
					{
						if(land == 2)
							land = 1;
					}

					if((fromX<0)||(fromY<0)||(land<0)||(land>LANDSCAPENUM)
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
			return 1;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_LIST1:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						curLandscape = SendDlgItemMessage(hdwnd, IDC_LIST1,
											   LB_GETCURSEL, 0, 0L); // get index
						if (curLandscape == 1)
							curLandscape = VILLAGE;
						else
						{
							if(curLandscape == 2)
								curLandscape = WATER;
						}
						EndDialog(hdwnd, 0);
					}
					return 1;

			case IDOK:	/* edit box OK button selected */
					/* display contents of the edit box */
					curLandscape = SendDlgItemMessage(hdwnd, IDC_LIST1,
										   LB_GETCURSEL, 0, 0L); // get index
					if (curLandscape == 1)
						curLandscape = VILLAGE;
					else
					{
						if(curLandscape == 2)
							curLandscape = WATER;
					
						else
						{
							if(curLandscape == -1)
								curLandscape = GRASS;
						}
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
