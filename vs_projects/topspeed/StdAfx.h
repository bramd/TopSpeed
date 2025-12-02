/**
* Top Speed 3
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8DC9DBBE_49A8_49D1_A21E_BA2D3D303DE5__INCLUDED_)
#define AFX_STDAFX_H__8DC9DBBE_49A8_49D1_A21E_BA2D3D303DE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#ifdef TOPSPEED_USE_SDL2
// SDL2 mode: No MFC dependency - use Windows SDK directly
#ifdef __EMSCRIPTEN__
// Emscripten: Include SDL2Compat.h for Windows type stubs
#include <DxCommon/If/SDL2Compat.h>
#else
#include <windows.h>
#include <tchar.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#else
// Original DirectX mode: MFC for dialog UI
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#endif // TOPSPEED_USE_SDL2


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8DC9DBBE_49A8_49D1_A21E_BA2D3D303DE5__INCLUDED_)
