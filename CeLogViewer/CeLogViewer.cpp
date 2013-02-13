/* File:	CeLogViewer.cpp
 * Created:	June 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"
#include "ViewerDialog.h"
#include "Logger.h"

//// The application object.
CWinApp theApp;

//// Definitions.
#define LIBRARY_CELOG _T("CeLog.dll")
#define CELZONE_CELOG_VIEWER 0x08000000
#define CELID_HWND 1000

//// Global Variables
Logger* pLog;

//// Entry Point.
int _tmain(int argc, _TCHAR* argv[])
{
	// Initialize Log
	//  Do not use pLog before this point!
	pLog = new Logger(true);
	pLog->WriteMessage(_T(" Start"), false, true);

	// Test Trust Level.
	switch (CeGetCurrentTrust())
	{
		case OEM_CERTIFY_TRUST:
			pLog->WriteMessage(_T("Trusted (Exe)"), true, false);
			//MessageBox(NULL, _T("Trusted."), _T("CeLogViewer"), MB_OK);
			break;

		case OEM_CERTIFY_RUN:
			pLog->WriteMessage(_T("Run Only (Exe)"), true, false);
			//MessageBox(NULL, _T("Run Only."), _T("CeLogViewer"), MB_OK);
			break;

		case OEM_CERTIFY_FALSE: // Fall Through
		default:
			pLog->WriteMessage(_T("Not Trusted (Exe)"), true, false);
			//MessageBox(NULL, _T("Not Trusted."), _T("CeLogViewer"), MB_OK);
			break;
	}

	// Load the Library.
    HANDLE hLib = LoadKernelLibrary(LIBRARY_CELOG);
    if (hLib == NULL)
	{
		pLog->WriteLastError(_T("LoadKernelLibrary "), true, false);
		//MessageBox(NULL, _T("Failed to Load Library."), _T("CeLogViewer"), MB_OK);
		return 1;
	}

	// Initialize MFC.
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		pLog->WriteLastError(_T("AfxWinInit "), true, false);
		//MessageBox(NULL, _T("Failed to Load MFC."), _T("CeLogViewer"), MB_OK);
		return 1;
	}

	ViewerDialog vd;
	vd.DoModal();
	//CeLogData(FALSE, CELID_HWND, NULL, 0, CELZONE_CELOG_VIEWER, 0, 0, FALSE);

	//CeLogData(FALSE, CELID_HWND, NULL, 0, CELZONE_CELOG_VIEWER, 0, 0, FALSE);

    // Could call KernelLibIoControl using hLib now...

    // Exit without closing hLib; the library can't be unloaded anyway.

	// Close Handles
	pLog->WriteNewLine();
	pLog->WriteMessage(_T(" End!"), true, true);
	pLog->Close(); // Not needed, but good practice.
	delete pLog;
	return 0;
}

