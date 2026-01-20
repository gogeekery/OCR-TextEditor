// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='' publicKeyToken='6595b64144ccf1df' language=''\"")

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS 1		// Stop nagging about CRT security

#include <Windows.h>

#include <commctrl.h>
#include <commdlg.h>

#include <winnt.h>
#include <Shlobj.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <shellapi.h>
