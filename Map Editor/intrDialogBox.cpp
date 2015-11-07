/* Message Box and Menus. */

#include <windows.h>
#include <stdio.h>
#include "resource.h"


LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);
/* the DialogBox callback function */
BOOL CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);


char szWinName[] = "MyWin";	/* name of window class */

HINSTANCE hInst;

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst,
				   LPSTR lpszArgs, int nWinMode)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wcl;
	HACCEL hAccel;

	/* Define a window class */
	wcl.hInstance = hThisInst;	/* handle to this instance */
	wcl.lpszClassName = szWinName;	/* window class name */
	wcl.lpfnWndProc = WindowFunc;	/* window function */
	wcl.style = 0;	/* default style */

	wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);	/* icon style */
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);	/* cursor style */
	wcl.lpszMenuName = "MYMENU";	/* main menu */

	wcl.cbClsExtra = 0;	/* no extra */
	wcl.cbWndExtra = 0; /* information needed */

	/* Make the window background white. */
	wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);

	/* Register the window class. */
	if(!RegisterClass(&wcl)) return 0;

	/* After registering, a window can be created. */
	hwnd = CreateWindow(
		szWinName,	/* name of window class */
		"Modal Dialog Box",	/* title */
		WS_OVERLAPPEDWINDOW,	/* window style - normal */
		CW_USEDEFAULT,	/* X coordinate - let Windows decide */
		CW_USEDEFAULT,	/* Y coordinate - let Windows decide */
		CW_USEDEFAULT,	/* width - let Windows decide */
		CW_USEDEFAULT,	/* height - let Windows decide */
		HWND_DESKTOP,	/* no parent window */
		NULL,	/* no menu */
		hThisInst, /* handle of this instance of the program */
		NULL	/* no additional arguments */
	);

	hInst = hThisInst;

	/* load the keyboard accelerators */
	hAccel = LoadAccelerators(hThisInst, "MYMENU");

	/* Display the window */
	ShowWindow(hwnd, nWinMode);
	UpdateWindow(hwnd);

	/* Create the message loop */
	while(GetMessage(&msg, NULL, 0, 0))
	{
		/* check if keyboard accelerators are pressed */
		if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
			TranslateMessage(&msg);	/* allow use of keyboard */
			DispatchMessage(&msg);	/* return control to Windows */
		}
	}
	return msg.wParam;
}

/* This function is called by Windows and is passed
	messages from the message queue. */
LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:	/* process menu options */
			switch (LOWORD(wParam)) {
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
