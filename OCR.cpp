// OCR.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "OCR.h"


const char g_szClassName[]		= "OCRTEXTED";
const char g_szChildClassName[]	= "OCRTEXTEDC";

#define g_szWindowName		"Image to Text Editor"




#define WM_OPEN_NEW_FILE 0x0FA0		// Custom window message

HWND g_hMDIClient = NULL;
HWND g_hMainWindow = NULL;


tesseract::TessBaseAPI* gTessApi = new tesseract::TessBaseAPI();

// rename: LoadImageFileToText
BOOL LoadTextFileToEdit(HWND hEdit, LPCTSTR pszFileName)
{
	HANDLE hFile;
	BOOL bSuccess = FALSE;

// --

	// Open input image with leptonica library
	Pix* lImage = pixRead(pszFileName);

	if (lImage != NULL) {

		char* lImgText = NULL;

		gTessApi->SetImage(lImage);
		lImgText = gTessApi->GetUTF8Text();
		pixDestroy(&lImage);

		if (lImgText == NULL) {
			// Error Message
			 return false;
		}

		// Convert the Unix new lines to windows (\r\n);
		int lOldTextLength = strlen(lImgText);
		char* lImgTextWindows = new char[lOldTextLength*2+3];
		int lOPos = 0;
		int lNPos = 0;

		while (lImgText[lOPos] != '\0') {
			if (lImgText[lOPos] == '\n') {
				lImgTextWindows[lNPos] = '\r';
				++lNPos;
				lImgTextWindows[lNPos] = '\n';
			} else {
				lImgTextWindows[lNPos] = lImgText[lOPos];
			}
			++lNPos;
			++lOPos;
		}

		lImgTextWindows[lNPos] = '\0';

		delete[] lImgText;

		if (SetWindowText(hEdit, lImgTextWindows))
			bSuccess = TRUE;

		delete[] lImgTextWindows;

	}

	return bSuccess;

// --

/*
	hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize;

		dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != 0xFFFFFFFF)
		{
			LPSTR pszFileText;

			pszFileText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
			if (pszFileText != NULL)
			{
				DWORD dwRead;

				if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL))
				{
					pszFileText[dwFileSize] = 0; // Add null terminator
					if (SetWindowText(hEdit, pszFileText))
						bSuccess = TRUE;
				}
				GlobalFree(pszFileText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
*/
}

BOOL SaveTextFileFromEdit(HWND hEdit, LPCTSTR pszFileName)
{
	HANDLE hFile;
	BOOL bSuccess = FALSE;

	hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwTextLength;

		dwTextLength = GetWindowTextLength(hEdit);
		// No need to bother if there's no text.
		if (dwTextLength > 0)
		{
			LPSTR pszText;
			DWORD dwBufferSize = dwTextLength + 1;

			pszText = (LPSTR)GlobalAlloc(GPTR, dwBufferSize);
			if (pszText != NULL)
			{
				if (GetWindowText(hEdit, pszText, dwBufferSize))
				{
					DWORD dwWritten;

					if (WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL))
						bSuccess = TRUE;
				}
				GlobalFree(pszText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

void DoFileOpen(HWND hwnd)
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));
// jpg, png, tiff, webp, jp2, bmp, pnm, gif, ps, pdf
// Ancillary supported file formats
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0Images (*.bmp, *.jpg, *.png, *.gif)\0*.png|*.bmp|*.jpg|*.gif\0";//"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";

	if (GetOpenFileName(&ofn))
	{
		HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
		if (LoadTextFileToEdit(hEdit, szFileName))
		{
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Opened...");
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)szFileName);

			SetWindowText(hwnd, szFileName);
		}
	}
}

void DoFileSave(HWND hwnd)
{
	OPENFILENAME ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "txt";
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
		if (SaveTextFileFromEdit(hEdit, szFileName))
		{
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Saved...");
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)szFileName);

			SetWindowText(hwnd, szFileName);
		}
	}
}

HWND CreateNewMDIChild(HWND hMDIClient)
{
	MDICREATESTRUCT mcs;
	HWND hChild;

	mcs.szTitle = "[Untitled]";
	mcs.szClass = g_szChildClassName;
	mcs.hOwner = GetModuleHandle(NULL);
	mcs.x = mcs.cx = CW_USEDEFAULT;
	mcs.y = mcs.cy = CW_USEDEFAULT;
	mcs.style = MDIS_ALLCHILDSTYLES;// | WS_EX_DLGMODALFRAME; // I would like the icon gone!

	hChild = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM) & mcs);
	if (!hChild)
	{
		MessageBox(hMDIClient, "MDI Child creation failed.", "Oh Oh...",
			MB_ICONEXCLAMATION | MB_OK);
	}
	return hChild;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HWND hTool;
		TBBUTTON tbb[3];
		TBADDBITMAP tbab;

		HWND hStatus;
		int statwidths[] = { 100, -1 };

		CLIENTCREATESTRUCT ccs;

		// Create MDI Client

		// Find window menu where children will be listed
		ccs.hWindowMenu = GetSubMenu(GetMenu(hwnd), 2);
		ccs.idFirstChild = ID_MDI_FIRSTCHILD;

		g_hMDIClient = CreateWindowEx(WS_EX_CLIENTEDGE, "mdiclient", NULL,
			WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwnd, (HMENU)IDC_MAIN_MDI, GetModuleHandle(NULL), (LPVOID)&ccs);

		if (g_hMDIClient == NULL)
			MessageBox(hwnd, "Could not create MDI client.", "Error", MB_OK | MB_ICONERROR);

		// Create Toolbar

		hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
			hwnd, (HMENU)IDC_MAIN_TOOL, GetModuleHandle(NULL), NULL);
		if (hTool == NULL)
			MessageBox(hwnd, "Could not create tool bar.", "Error", MB_OK | MB_ICONERROR);

		// Send the TB_BUTTONSTRUCTSIZE message, which is required for
		// backward compatibility.
		SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

		tbab.hInst = HINST_COMMCTRL;
		tbab.nID = IDB_STD_SMALL_COLOR;
		SendMessage(hTool, TB_ADDBITMAP, 0, (LPARAM)&tbab);

		ZeroMemory(tbb, sizeof(tbb));
		tbb[0].iBitmap = STD_FILENEW;
		tbb[0].fsState = TBSTATE_ENABLED;
		tbb[0].fsStyle = TBSTYLE_BUTTON;
		tbb[0].idCommand = ID_FILE_NEW;

		tbb[1].iBitmap = STD_FILEOPEN;
		tbb[1].fsState = TBSTATE_ENABLED;
		tbb[1].fsStyle = TBSTYLE_BUTTON;
		tbb[1].idCommand = ID_FILE_OPEN;

		tbb[2].iBitmap = STD_FILESAVE;
		tbb[2].fsState = TBSTATE_ENABLED;
		tbb[2].fsStyle = TBSTYLE_BUTTON;
		tbb[2].idCommand = ID_FILE_SAVEAS;

		SendMessage(hTool, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON), (LPARAM)&tbb);

		// Create Status bar

		hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
			WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
			hwnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);

		SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);
		SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Started...");
	}
	break;
	case WM_SIZE:
	{
		HWND hTool;
		RECT rcTool;
		int iToolHeight;

		HWND hStatus;
		RECT rcStatus;
		int iStatusHeight;

		HWND hMDI;
		int iMDIHeight;
		RECT rcClient;

		// Size toolbar and get height

		hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
		SendMessage(hTool, TB_AUTOSIZE, 0, 0);

		GetWindowRect(hTool, &rcTool);
		iToolHeight = rcTool.bottom - rcTool.top;

		// Size status bar and get height

		hStatus = GetDlgItem(hwnd, IDC_MAIN_STATUS);
		SendMessage(hStatus, WM_SIZE, 0, 0);

		GetWindowRect(hStatus, &rcStatus);
		iStatusHeight = rcStatus.bottom - rcStatus.top;

		// Calculate remaining height and size edit

		GetClientRect(hwnd, &rcClient);

		iMDIHeight = rcClient.bottom - iToolHeight - iStatusHeight;

		hMDI = GetDlgItem(hwnd, IDC_MAIN_MDI);
		SetWindowPos(hMDI, NULL, 0, iToolHeight, rcClient.right, iMDIHeight, SWP_NOZORDER);
	}
	break;
	case WM_COPYDATA:
	{

		PCOPYDATASTRUCT pcds = (PCOPYDATASTRUCT)lParam;

		if (pcds->dwData == 1) {

			LPCTSTR FName = (LPSTR)(pcds->lpData);
			HWND hChild = CreateNewMDIChild(g_hMDIClient);

			//MessageBox(hwnd, FName, "File Pointer:", MB_OK); // Debug only.

			if (!hChild)
				MessageBox(hwnd, "MDI Child creation failed.", "Error", MB_ICONEXCLAMATION | MB_OK);

			if (!LoadTextFileToEdit(GetDlgItem(hChild, IDC_CHILD_EDIT), FName))
				MessageBox(hwnd, FName, "Couldn't Open File:", MB_ICONEXCLAMATION | MB_OK);

			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Opened...");
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)FName);

			SetWindowText(hChild, FName); // Give it the correct title...

		}

	}
	break;
	case WM_DROPFILES:
	{

		char Num = DragQueryFile((HDROP)wParam, -1, 0, 0);
		TCHAR* FName = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH);

		if (Num) for (char Pos = 0; Pos < Num; ++Pos) {

			DragQueryFile((HDROP)wParam, (UINT)Pos, FName, (UINT)MAX_PATH);

			HWND hChild = CreateNewMDIChild(g_hMDIClient);

			if (!LoadTextFileToEdit(GetDlgItem(hChild, IDC_CHILD_EDIT), FName))
				MessageBox(hwnd, FName, "Couldn't Open File:", MB_OK);

			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Opened...");
			SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)FName);

			SetWindowText(hChild, FName); // Give it the correct title...

		}

		DragFinish((HDROP)wParam);

	}
	break;
	case WM_CLOSE:
		//			MessageBox(0, "Ending", "", 0);
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0); // End the entire process, when the main window is destroyed!
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case ID_FILE_NEW:
			CreateNewMDIChild(g_hMDIClient);
			break;
		case ID_FILE_OPEN:
		{
			HWND hChild = CreateNewMDIChild(g_hMDIClient);
			if (hChild)
			{
				DoFileOpen(hChild);
			}
		}
		break;
		case ID_FILE_CLOSE:
		{
			HWND hChild = (HWND)SendMessage(g_hMDIClient, WM_MDIGETACTIVE, 0, 0);
			if (hChild)
			{
				SendMessage(hChild, WM_CLOSE, 0, 0);
			}
		}
		break;
		case ID_WINDOW_TILE:
			SendMessage(g_hMDIClient, WM_MDITILE, 0, 0);
			break;
		case ID_WINDOW_CASCADE:
			SendMessage(g_hMDIClient, WM_MDICASCADE, 0, 0);
			break;
		default:
		{
			if (LOWORD(wParam) >= ID_MDI_FIRSTCHILD)
			{
				DefFrameProc(hwnd, g_hMDIClient, WM_COMMAND, wParam, lParam);
			}
			else
			{
				HWND hChild = (HWND)SendMessage(g_hMDIClient, WM_MDIGETACTIVE, 0, 0);
				if (hChild)
				{
					SendMessage(hChild, WM_COMMAND, wParam, lParam);
				}
			}
		}
		}
		break;
	default:
		return DefFrameProc(hwnd, g_hMDIClient, msg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HFONT hfDefault;
		HWND hEdit;

		// Create Edit Control

		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
			0, 0, 100, 100, hwnd, (HMENU)IDC_CHILD_EDIT, GetModuleHandle(NULL), NULL);
		if (hEdit == NULL)
			MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

		hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
	}
	break;
	case WM_MDIACTIVATE:
	{
		HMENU hMenu, hFileMenu;
		UINT EnableFlag;

		hMenu = GetMenu(g_hMainWindow);
		if (hwnd == (HWND)lParam)
		{	   //being activated, enable the menus
			EnableFlag = MF_ENABLED;
		}
		else
		{						   //being de-activated, gray the menus
			EnableFlag = MF_GRAYED;
		}

		EnableMenuItem(hMenu, 1, MF_BYPOSITION | EnableFlag);
		EnableMenuItem(hMenu, 2, MF_BYPOSITION | EnableFlag);

		hFileMenu = GetSubMenu(hMenu, 0);
		EnableMenuItem(hFileMenu, ID_FILE_SAVEAS, MF_BYCOMMAND | EnableFlag);

		EnableMenuItem(hFileMenu, ID_FILE_CLOSE, MF_BYCOMMAND | EnableFlag);
		EnableMenuItem(hFileMenu, ID_FILE_CLOSEALL, MF_BYCOMMAND | EnableFlag);

		DrawMenuBar(g_hMainWindow);
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_OPEN:
			DoFileOpen(hwnd);
			break;
		case ID_FILE_SAVEAS:
			DoFileSave(hwnd);
			break;
		case ID_EDIT_CUT:
			SendDlgItemMessage(hwnd, IDC_CHILD_EDIT, WM_CUT, 0, 0);
			break;
		case ID_EDIT_COPY:
			SendDlgItemMessage(hwnd, IDC_CHILD_EDIT, WM_COPY, 0, 0);
			break;
		case ID_EDIT_PASTE:
			SendDlgItemMessage(hwnd, IDC_CHILD_EDIT, WM_PASTE, 0, 0);
			break;
		}
		break;
	case WM_SIZE:
	{
		HWND hEdit;
		RECT rcClient;

		// Calculate remaining height and size edit

		GetClientRect(hwnd, &rcClient);

		hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
		SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
	}
	return DefMDIChildProc(hwnd, msg, wParam, lParam);
	default:
		return DefMDIChildProc(hwnd, msg, wParam, lParam);

	}
	return 0;
}

BOOL SetUpMDIChildWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MDIChildWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;//LoadIcon(GetModuleHandle(NULL), NULL);//MAKEINTRESOURCE(APPICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szChildClassName;
	wc.hIconSm = NULL;//LoadIcon(GetModuleHandle(NULL), NULL);//MAKEINTRESOURCE(APPICON));

	if (!RegisterClassEx(&wc))
	{
		MessageBox(0, "Could Not Register Child Window", "Ohh No...",
			MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	else
		return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	/*LPSHELLSTATE LPST; // LOL Windows 95!
	LPST->fWin95Classic = true;
	SHGetSetSettings(LPST, NULL, true);*/

	LPCWSTR MyMutex = (LPCWSTR)"MG_MuteExKey"; // Mutex handles, very important!
	HANDLE MuteHandle = CreateMutexW(NULL, FALSE, MyMutex);
	bool AnotherIsActive = false;

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		AnotherIsActive = true;

	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	InitCommonControls();

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(APPICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(APPICON));

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	if (!SetUpMDIChildWindowClass(hInstance))
		return 0;

	if (!AnotherIsActive) {

		hwnd = CreateWindowEx(
			0,
			g_szClassName, g_szWindowName,
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CW_USEDEFAULT, CW_USEDEFAULT, 480, 320,
			NULL, NULL, hInstance, NULL);

		ShowWindow(hwnd, nCmdShow);
		UpdateWindow(hwnd); // Quickly display the window!

		DragAcceptFiles(hwnd, true);

	}
	else {

		hwnd = CreateWindowEx(
			0, g_szClassName, "",
			0, 0, 0, 0, 0,
			NULL, NULL, hInstance, NULL);

		ShowWindow(hwnd, SW_HIDE); // Hide the temp window...

	}

	if (hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	g_hMainWindow = hwnd;



	//--------


	HRSRC lRes = FindResourceA(NULL, MAKEINTRESOURCE(IDD_DATA), RT_RCDATA);
	if (!lRes)
	{
		fprintf(stderr, "Could not find resource.\n");
		return 1;
	}

	HGLOBAL lResHandle = LoadResource(NULL, lRes);
	if (!lResHandle)
	{
		fprintf(stderr, "Could not load resource\n");
		return 1;
	}

	const char* lData = (const char*)LockResource(lResHandle);
	int lDataSize = SizeofResource(NULL, lRes);

	// TODO: Set Tesseract image processing configuration?
	// Or will manual image preprocessing need to occur on an image level?
	// https://github.com/tesseract-ocr/tessdoc/blob/main/ImproveQuality.md

	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (gTessApi->Init(lData, lDataSize, "eng", tesseract::OEM_DEFAULT, NULL, 0, NULL, NULL, false, NULL)) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}



	/*
	LPWSTR			*ArgVal;
	unsigned short	ArgCount;
	*/

	//ArgVal = CommandLineToArgvW(GetCommandLineW(), &ArgCount);

	/*
	for (ArgCount = 0; ArgCount < nCmdShow; ++ArgCount)
		ArgVal[ArgCount] =
	*/

	/*
		while (__argc > 1) { // Two command line arguments (the program's path, and the file to be opened).

			--__argc;						// Lower the counter, and the number of files to open...
			LPSTR FName = __argv[__argc];	// The command line argument (string) of the file to be opened.

			// WM_OPEN_NEW_FILE || WM_COPYDATA
			if (AnotherIsActive) { // Another instance of the program is running.

				HWND HFind = FindWindow(g_szClassName, g_szWindowName); // Get the other window

				if (HFind == NULL)
					return 0;//MessageBox(hwnd, "Could not get a handle on the window!", "ERROR", MB_OK);

				COPYDATASTRUCT cds;
				cds.dwData = 1;
				cds.cbData = sizeof(TCHAR) * (MAX_PATH + 1); //_tcslen(FName)
				cds.lpData = FName;

				SendMessage(HFind, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cds);

				return 0;

			} else { // No other instance is present.

				HWND hChild = CreateNewMDIChild(g_hMDIClient);

				if (!LoadTextFileToEdit(GetDlgItem(hChild, IDC_CHILD_EDIT), FName))
					MessageBox(hwnd, FName, "Couldn't Open File:", MB_OK);

				SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"Opened...");
				SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)FName);

				SetWindowText(hChild, FName); // Give it the correct title...

			}

		}
	*/

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		if (!TranslateMDISysAccel(g_hMDIClient, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	gTessApi ->End();
	return Msg.wParam;

}



/*
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OCR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    char* outText;

    HRSRC lRes = FindResourceA(NULL, MAKEINTRESOURCE(IDD_DATA), RT_RCDATA);
    if (!lRes)
    {
        fprintf(stderr, "Could not find resource.\n");
        return 1;
    }

    HGLOBAL lResHandle = LoadResource(NULL, lRes);
    if (!lResHandle)
    {
        fprintf(stderr, "Could not load resource\n");
        return 1;
    }

    const char* lData = (const char*)LockResource(lResHandle);
    int lDataSize = SizeofResource(NULL, lRes);

    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(lData, lDataSize, "eng", tesseract::OEM_DEFAULT, NULL, 0, NULL, NULL, false, NULL)) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }

    // Open input image with leptonica library
    Pix* image = pixRead("images/test.jpg");
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    FILE* lFile = fopen("output.txt", "w");
    fprintf(lFile, "OCR output:\n%s", outText);
    fclose(lFile);

    // Destroy used object and release memory
    api->End();
    delete[] outText;
    pixDestroy(&image);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OCR));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OCR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_OCR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_FILE_OPEN:
                break;
            case ID_FILE_SAVE:
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
*/
