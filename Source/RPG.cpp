// WINX.CPP - Game Console Shell

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

#include <ddraw.h>


// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINXCLASS"  // class name

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BIT	  24

#define CELL_WIDTH	  40

// bitmap defines
#define BITMAP_ID            0x4D42 // universal id for a bitmap

#define UP		1
#define DOWN	2
#define LEFT	3
#define RIGHT	4
#define STAY	5

#define DISPLAY_OFFAMOUNT	8

#define NUM_OF_LANDSCAPES	3
#define GRASS	0
#define WATER	1
#define VILLAGE 2

// MACROS /////////////////////////////////////////////////

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define _RGB24BIT(r, g ,b)  ((b&255)+((g&255)<<8)+((r&255)<<16))

// container structure for bitmaps .BMP file
typedef struct BITMAP_FILE_TAG
        {
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info including the palette
//        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data

        } BITMAP_FILE, *BITMAP_FILE_PTR;


// PROTOTYPES /////////////////////////////////////////////

int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

int CreateOffScreen(LPDIRECTDRAWSURFACE &lpddshello, int pixel_width, int pixel_height);

LPDIRECTDRAWCLIPPER DDAttachClipper(LPDIRECTDRAWSURFACE lpdds, int num_rects, LPRECT clip_list);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance
char buffer[80];                // used to print text

LPDIRECTDRAW	lpdd;
DDSURFACEDESC	ddsd;
DDSCAPS			ddscaps;
LPDIRECTDRAWSURFACE lpddsprimary,
				lpddsback;
LPDIRECTDRAWSURFACE lpddscharacter,
				lpddsgrass;
LPDIRECTDRAWCLIPPER	lpddclip;
DDBLTFX			ddbltfx;

RECT fill_area;
RECT src_area;
RECT char_area;

DWORD backgroundIndex = GRASS;

int		charX = 23, charY = 17;	// init position of main character
int		charDirection = STAY;	// init character direction of moving
const int repCount = CELL_WIDTH/DISPLAY_OFFAMOUNT;
int worldMapOffset[2] = {(charX-7), (charY-5)};
int firstRun = 1;
int worldMapWidth, worldMapHeight;
int **worldMap;
// WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

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
/*
	map = new int*[width];
	for(i=0; i<width; i++)
		map[i] = new int[height];
		*/

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
int LoadImage(LPDIRECTDRAWSURFACE lpwork, LPSTR szImage, int landscapeIndex)
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
		IMAGE_BITMAP, 40,
		40, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
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

	if(BitBlt(hdcSurf, landscapeIndex*CELL_WIDTH, 0, (landscapeIndex+1)*CELL_WIDTH, CELL_WIDTH,
		hdcImage, 0, 0, SRCCOPY) == FALSE)
	{
		MessageBox(main_window_handle, "blt","hi",MB_OK);
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

int CheckWater()
{
	if(worldMap[charX][charY] == WATER)
		return(1);
	else
		return(0);
}

int Game_Main(void *parms)
{
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

// your code goes here
	int strangeTemp[2] = {0,0};
	if (KEY_DOWN(VK_ESCAPE))
		PostMessage(main_window_handle, WM_DESTROY,0,0);

	if (KEY_DOWN(VK_UP))
	{
		if(charY == 5)
			return(1);
		charY--;
		if(CheckWater())
		{
			charY++;
			return(1);
		}
		worldMapOffset[1]--;
		charDirection = UP;	
	}
	else if(KEY_DOWN(VK_DOWN))
	{
		if(charY == (worldMapHeight-7))
			return(1);
		charY++;
		if(CheckWater())
		{
			charY--;
			return(1);
		}
		worldMapOffset[1]++;
		charDirection = DOWN;	
	}
	else if(KEY_DOWN(VK_RIGHT))
	{
		if(charX == (worldMapWidth-9))
			return(1);
		charX++;
		if(CheckWater())
		{
			charX--;
			return(1);
		}
		worldMapOffset[0]++;
		charDirection = RIGHT;	
	}
	else if(KEY_DOWN(VK_LEFT))
	{
		if(charX == 7)
			return(0);
		charX--;
		if(CheckWater())
		{
			charX++;
			return(1);
		}
		worldMapOffset[0]--;
		charDirection = LEFT;	
	}
	else
	{
		charDirection = STAY;
		if (!firstRun)
			return(1);
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

	switch(charDirection) {
	case DOWN:
//		if(charDirection == DOWN) 
		strangeTemp[1] = -1;
		verticalDisplayOffset = -DISPLAY_OFFAMOUNT;
		initVertDisplay = -DISPLAY_OFFAMOUNT;
		break;
	case UP:
//			if(charDirection == UP)
//		strangeTemp = 0;
		verticalDisplayOffset = DISPLAY_OFFAMOUNT;
		initVertDisplay = verticalDisplayOffset-CELL_WIDTH;
		break;
	case LEFT:
//		if(charDirection == LEFT)
//		strangeTemp = 0;
		horizontalDisplayOffset = DISPLAY_OFFAMOUNT;
		initHoriDisplay = horizontalDisplayOffset-CELL_WIDTH;
		break;
	case RIGHT:
//		if(charDirection == RIGHT
		strangeTemp[0] = -1;
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
		for (int i=initVertDisplay; i<SCREEN_HEIGHT; i+=CELL_WIDTH)
		{
			for (int j=initHoriDisplay; j<SCREEN_WIDTH; j+=CELL_WIDTH)
			{
				fill_area.top = i;
				fill_area.left = j;
				fill_area.bottom = i+CELL_WIDTH;
				fill_area.right = j+CELL_WIDTH;
				
				backgroundIndex = worldMap[worldMapOffset[0]+(j-initHoriDisplay)/CELL_WIDTH+strangeTemp[0]][worldMapOffset[1]+(i-initVertDisplay)/CELL_WIDTH+strangeTemp[1]];
				src_area.left = (backgroundIndex<<3)+(backgroundIndex<<5);
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
		lpddsback->Blt(&char_area,//280,200,
		lpddscharacter,
		NULL,
		DDBLT_WAIT|DDBLT_KEYSRC,
		//DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
		NULL);

		initHoriDisplay+=horizontalDisplayOffset;
		initVertDisplay+=verticalDisplayOffset;
		//////////////////////////////////////
		lpdd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);

		while(lpddsprimary->Flip(NULL, DDFLIP_WAIT)!=DD_OK);

//		Sleep(500);
	}




/*	lpddsback->Blt(&fill_area,
		lpddscharacter,
		NULL,
		DDBLT_WAIT | DDBLT_KEYSRC,
		NULL);
*/


	Sleep(30);
	// return success
	return(1);
}
 // end Game_Main


int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game
	int i;
	int j;
	
	// try outputing the map to a file
	if(!ReadMapFromFile(worldMap, "e:\\RPG V1.0\\Map\\worldMap.dat", worldMapWidth, worldMapHeight))
		return(0);

//	worldMapWidth = 48;
//	worldMapHeight = 36;

/*	worldMap = new int*[48];
	for(i=0; i<48; i++)
		worldMap[i] = new int[36];
*/
/*	for (i = 0; i<48; i++)
	{
		for (j=0; j<36; j++)
		{
			worldMap[i][j] = GRASS;
		}
	}
	for(i=16; i<19; i++)
	{
		for(j=17; j<21; j++)
			worldMap[i][j] = WATER;
	}
	for(i=23;i<28;i++)
	{
		for(j = 10; j<14; j++)
			worldMap[i][j] = WATER;
	}
	for(i=0; i<3; i++)
	{
		for(j=0;j<3;j++)
			worldMap[i][j] = WATER;
		for(j=35; j>32; j--)
			worldMap[i][j] = WATER;
	}
	for(i=47; i>44; i--)
	{
		for(j=0;j<3;j++)
			worldMap[i][j] = WATER;
		for(j=35; j>32; j--)
			worldMap[i][j] = WATER;
	}
	worldMap[32][24] = VILLAGE;
*/

/*	if(!WriteMapToFile(worldMap, "e:\\RPG V1.0\\Map\\worldMap.dat", worldMapWidth, worldMapHeight))
		return(0);
*/

	char_area.top = 200;
	char_area.left = 280;
	char_area.bottom = 240;
	char_area.right = 320;



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

	//////////////////////////////// OFFSCREEN SURFACE///////////////////
 /*   ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | 
		           DDSD_WIDTH | DDSD_CKSRCBLT; 
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN; 
    ddsd.dwHeight = 40; 
    ddsd.dwWidth = 40;
    // Set the source color key to green.
    ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = _RGB24BIT(255,255,255);
    ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = _RGB24BIT(255,255,255);

    if ( FAILED( lpdd->CreateSurface( &ddsd, &lpddscharacter, NULL ) ) )
		return(0);

*/  
	if(!CreateOffScreen(lpddsgrass, NUM_OF_LANDSCAPES*CELL_WIDTH, CELL_WIDTH))
		return(0);


	if(!CreateOffScreen(lpddscharacter, CELL_WIDTH, CELL_WIDTH))
		return(0);

	DDCOLORKEY	key;
	key.dwColorSpaceLowValue = _RGB24BIT(1,1,1);//1;
//	key.dwColorSpaceHighValue = 1;//_RGB24BIT(255,255,255);//(1,1,1);
	lpddscharacter->SetColorKey(DDCKEY_SRCBLT, &key);
//	lpddsgrass->SetColorKey(DDCKEY_SRCBLT, &key);

	/////////////// LOADING BITMAPS //////////////////////////
	if(!LoadImage(lpddscharacter, "e:\\RPG V1.0\\Bitmaps\\Characters\\RPGmainChar.bmp",0))
		return(0);
	if(!LoadImage(lpddsgrass, "e:\\RPG V1.0\\Bitmaps\\Landscapes\\grassLand.bmp",GRASS))
		return(0);
	if(!LoadImage(lpddsgrass, "e:\\RPG V1.0\\Bitmaps\\Landscapes\\water.bmp",WATER))
		return(0);
	if(!LoadImage(lpddsgrass, "e:\\RPG V1.0\\Bitmaps\\Landscapes\\village.bmp",VILLAGE))
		return(0);

	RECT cliplist[1] = {{0,0,SCREEN_WIDTH, SCREEN_HEIGHT}};
	lpddclip = DDAttachClipper(lpddsback, 1, cliplist);
	if(lpddclip==NULL)
		return(0);

	src_area.top = 0;
	src_area.bottom = CELL_WIDTH;
	// return success
	return(1);
} // end Game_Init

///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated

// your code goes here
	if (lpddscharacter)
		lpddscharacter->Release();
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
						  "WinX Game Console",	 // title
						  WS_POPUP | WS_VISIBLE,
					 	  0,0,	   // x,y
						  WINDOW_WIDTH,  // width
                          WINDOW_HEIGHT, // height
						  NULL,	   // handle to parent 
						  NULL,	   // handle to menu
						  hinstance,// instance
						  NULL)))	// creation parms
return(0);

ShowCursor(FALSE);
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
    Game_Main();

	} // end while

// shutdown game and release all resources
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain


