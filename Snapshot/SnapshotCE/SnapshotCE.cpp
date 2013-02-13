/* File:	Snapshot.cpp
 * Created: Dec 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

/* Icon Source "Gear"
 * License: See Link
 * http://dribbble.com/shots/319986-Gear-Domain-effects
 */

//// External Includes (see stdafx.h)

//// Local Includes
#include "stdafx.h"
#include "Logger.h"
// Added support for PPC (Imaging.h, GdiplusImaging.h, GdiplusPixelFormats.h)
#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
	#include "Imaging.h"
#else
	// Will not compile if this include is located in stdafx.h.
	#include <Imaging.h>
	// Use #include <WinGDI.h> // for desktop PCs.
#endif
#include "Missing.h" // Microsoft's Missing Definitnions.

//// The application object.
CWinApp theApp; // MFC

//// Definitions
// (http://msdn.microsoft.com/en-us/library/aa911386.aspx)
#define TH32CS_SNAPNOHEAPS   0x40000000

//// Structures
typedef struct _CAPTURE
{
	HBITMAP     hBitmap;
	PBITMAPINFO pBitmapInfo;
	LPVOID      pBytes;
	LPCTSTR     sFilename;
}CAPTURE, *PCAPTURE;

//// Enumerations
enum EncoderType
{
	EncoderTypeBMP,
	EncoderTypeGIF,
	EncoderTypeJPEG,
	EncoderTypePNG,
	EncoderTypeTIFF
};

//// Function Prototypes
//bool GetProcessWindows    (DWORD nTargetPID, CArray<HWND, HWND>* pahWnd, HWND hWnd = NULL);
bool CaptureDCToFile      (HWND hWnd, LPTSTR sFilename, EncoderType nEncoder = EncoderTypePNG);
bool LogProcessEntry      (PROCESSENTRY32* pProcessEntry);
bool ProcessSnapshot      (PROCESSENTRY32* pProcessEntry);
bool SaveCapture          (PCAPTURE pCapture, bool bCompress = false);
bool SaveEncodedCapture   (PCAPTURE pCapture, EncoderType nEncoder = EncoderTypePNG);
bool SnapshotHeap         (DWORD nPID);
bool SnapshotModules      (DWORD nPID);
bool SnapshotProcesses    (TCHAR* psTarget = NULL);
bool SnapshotThreads      (DWORD nPID);
bool SnapshotWindows      (DWORD nTargetPID, DWORD nLevel = (DWORD)0, HWND hWnd = NULL); // Requires MFC.
//DWORD StringToDWORD       (LPCTSTR sValue);
//LPCTSTR DWORDToString     (DWORD nValue);
//LPCTSTR LongToString      (long nValue);

//// Global Variables
Logger* pLog;

//// Program entry point.
int _tmain(int argc, _TCHAR* argv[])
{
	// Set High Thread Priority.
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// Initialize MFC.
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		return 1;
	}

	#ifdef SANDBOX
		// Stopwatch Start.
		int  nEnd;
		int  nStart = GetTickCount();
	#endif

	Sleep(3000);

	// Initialize Log
	//  Do not use pLog before this point!
	pLog = new Logger(true);
	pLog->WriteMessage(_T(" Start"), false, true);

	// TODO Parse Command Line Arguments.
	// Snapshot.exe -l"path\Logfile" -t"targetproc.exe"(or all)

	// Take a snapshot of the running system.
	bool nResult = SnapshotProcesses();

	// Close Handles
	pLog->WriteNewLine();
	pLog->WriteMessage(_T(" End Success!"), true, true);
	pLog->Close(); // Not needed, but good practice.
	delete pLog;

	#ifdef SANDBOX
		// Stopwatch Stop.
		nEnd = GetTickCount();

		// Display Results.
		TCHAR* sTemp = new TCHAR[48];
		_stprintf(sTemp, _T("Result     = %s\nTime(ms) = %d"), nResult ? _T("Success") : _T("Failed"), nEnd - nStart);
		MessageBox(NULL, sTemp, _T("Snapshot"), 0);
		delete[] sTemp;
	#endif

	// End of program.
	return 0;
}


////// Retrieve a list of window handles for a process.
//bool GetProcessWindows(DWORD nTargetPID, CArray<HWND, HWND>* pahWnd, HWND hWnd /*= NULL*/)
//{
//	// Validate parameters.
//	if (pahWnd == NULL)
//	{
//		SetLastError(ERROR_INVALID_PARAMETER);
//		return false;
//	}
//
//	// Get the topmost Window handle.
//	// If hWnd is NULL, then the handle returned is the topmost z-order window.
//	// Else, the handle is the topmost z-order child window of the parent window.
//	hWnd = GetTopWindow(hWnd);
//
//	// Find the process's Window handle.
//	DWORD nCurrentPID = 0;
//	while (hWnd != NULL)
//	{
//		// Check if the Window is part of the target.
//		GetWindowThreadProcessId(hWnd, &nCurrentPID);
//		if (nCurrentPID == nTargetPID)
//		{
//			// Add the Window's handle to the list.
//			pahWnd->Add(hWnd);
//
//			// Check each of the Window's children.
//			GetTargetHWNDs(nTargetPID, pahWnd, hWnd);
//		}
//
//		// Get the Next Window handle below the current.
//		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
//	}
//
//	// No handles found.
//	if (pahWnd->GetCount() == 0)
//	{
//		SetLastError(ERROR_NOT_FOUND);
//		return false;
//	}
//
//	// Done.
//	SetLastError(ERROR_SUCCESS);
//	return true;
//}


//// Write the process entry information to the log.
bool LogProcessEntry(PROCESSENTRY32* pProcessEntry)
{
	if (pProcessEntry == NULL)
	{
		return false;
	}

	pLog->WriteMessage(_T("------------------------------------------------------------\n"));

	TCHAR* psMessage = new TCHAR[MAX_PATH];
	_stprintf(psMessage, _T("Process:    %s\0"), pProcessEntry->szExeFile);
	pLog->WriteMessage(psMessage);
	_stprintf(psMessage, _T("ID:         0x%X\0"), pProcessEntry->th32ProcessID);
	pLog->WriteMessage(psMessage);
	_stprintf(psMessage, _T("Threads(#): %u\0"), pProcessEntry->cntThreads);
	pLog->WriteMessage(psMessage);
	_stprintf(psMessage, _T("Parent ID:  0x%X\0"), pProcessEntry->th32ParentProcessID);
	pLog->WriteMessage(psMessage);
	_stprintf(psMessage, _T("Heap ID:    0x%X\0"), pProcessEntry->th32DefaultHeapID);
	pLog->WriteMessage(psMessage);
	_stprintf(psMessage, _T("Load Addr:  0x%X\0"), pProcessEntry->th32MemoryBase);
	pLog->WriteMessage(psMessage);
	_stprintf(psMessage, _T("Access Key: 0x%X\0"), pProcessEntry->th32AccessKey);
	pLog->WriteMessage(psMessage);
	delete[] psMessage;

	return true;
}


//// Take snapshot of a single process.
bool ProcessSnapshot(PROCESSENTRY32* pProcessEntry)
{
	// Validate pointer.
	if (pProcessEntry == NULL)
	{
		return false;
	}

	// Target process found, open process for access.
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
								   PROCESS_VM_READ |
								   PROCESS_TERMINATE,
								   FALSE,
								   pProcessEntry->th32ProcessID);
	if (hProcess == NULL)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("OpenProcess wQUERY,VM_READ,TERMINATE"));
		return false;
	}

	// Log the process.
	LogProcessEntry(pProcessEntry);

	// Take snapshot of the heap.
	SnapshotHeap(pProcessEntry->th32ProcessID);

	// Take snapshot of the modules.
	SnapshotModules(pProcessEntry->th32ProcessID);

	// Take snapshot of the threads.
	SnapshotThreads(pProcessEntry->th32ProcessID);

	// Take snapshot of the windows.
	SnapshotWindows(pProcessEntry->th32ProcessID);

	// Cleanup.
	if (CloseHandle(hProcess) == 0)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("CloseHandle(hProcess)"));
	}

	return true;
}


//// Take snapshot of process's heap.
bool SnapshotHeap(DWORD nPID)
{
	// Function variables.
	TCHAR* psMessage = new TCHAR[48];

	//// Heap List Begin.
	pLog->WriteMessage(_T("\n\n -- Heap --"));
	pLog->WriteTimestamp();
	pLog->WriteMessage(_T("-- Address --  -- Size --"));

	// Get snapshot of the heap.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, nPID);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("SNAPHEAPLIST"));
		return false;
	}

	// Initialize structures.
	HEAPLIST32  HeapList  = {0};
	HEAPENTRY32 HeapEntry = {0};
	HeapList.dwSize  = sizeof(HEAPLIST32);
	HeapEntry.dwSize = sizeof(HEAPENTRY32);

	// Loop variables.
	//HANDLE hHeapBlock = NULL;
	DWORD  nTotalSize = 0;

	// Get the target process's heap details.
	if (Heap32ListFirst(hSnapshot, &HeapList) == FALSE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("Heap32ListFirst"));
		if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
		{
			pLog->WriteMessage(_T("Unable to close(SNAPHEAPLIST)!"), true, true);
		}
		return false;
	}
	do
	{
		//// Heap Entry Begin.
		// Get the heap's entry details.
		if ( Heap32First( hSnapshot,
						  &HeapEntry,
						  HeapList.th32ProcessID,
						  HeapList.th32HeapID ) == FALSE )
		{
			// See GetLastError().
			pLog->WriteLastError(_T("Heap32First"));
			continue;
		}
		do
		{
			// Keep track of total size.
			nTotalSize += HeapEntry.dwBlockSize;
			
			// Log the heap.
			// TODO enable this in release.
			#ifdef SANDBOX
				_stprintf(psMessage, _T("   0x%8X  %10u"), HeapEntry.dwAddress,
														   HeapEntry.dwBlockSize);
				pLog->WriteMessage(psMessage);
			#endif

			//hHeapBlock = HeapEntry.hHandle;
			//if (hHeapBlock != NULL)
			//{
			//	// TODO Determine if heap access if possible.
			//}

			// Get the next heap entry in the list.
		} while (Heap32Next(hSnapshot, &HeapEntry) == TRUE);
		//// Heap Entry End.


		// Get the next heap list in the list.
	} while (Heap32ListNext(hSnapshot, &HeapList) == TRUE);
	
	// Write heap tally.
	_stprintf(psMessage, _T("Heap Total = %u bytes\0"), nTotalSize);
	pLog->WriteMessage(psMessage);

	// Close list handle since target found.
	if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
	{
		pLog->WriteMessage(_T("Unable to close(SNAPHEAPLIST)!"), true, true);
	}

	// Cleanup.
	delete[] psMessage;

	//// Heap List End.
	return true;
}


//// Take snapshot of process's modules.
bool SnapshotModules(DWORD nPID)
{
	//// Module Entry Begin.
	pLog->WriteMessage(_T("\n\n -- Modules --"));
	pLog->WriteTimestamp();
	pLog->WriteMessage(_T("-- 0xID --  -- gUse --  -- pUse --  -- Path & Name --"));

	// Get a snapshot of the threads.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, nPID);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("SNAPMODULE"), true, true);
		return false;
	}

	// Setup module structure.
	MODULEENTRY32 ModuleEntry = {0};
	ModuleEntry.dwSize = sizeof(MODULEENTRY32);

	// Loop varibles.
	TCHAR* psMessage = new TCHAR[MAX_PATH+MAX_PATH+40]; // 560

	// Get the process's module details.
	if (Module32First(hSnapshot, &ModuleEntry) == FALSE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("Module32First"));
		if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
		{
			pLog->WriteMessage(_T("Unable to close(SNAPMODULE)!"), true, true);
		}
		return false;
	}
	do
	{
		// Log the module.
		_stprintf(psMessage, _T("0x%8X  %10u  %10u  %s\\%s\0"), ModuleEntry.th32ModuleID,
																ModuleEntry.GlblcntUsage,
																ModuleEntry.ProccntUsage,
																ModuleEntry.szExePath,
																ModuleEntry.szModule);
		pLog->WriteMessage(psMessage);

		// Get the next module entry in the list.
	} while (Module32Next(hSnapshot, &ModuleEntry) == TRUE);

	// Close list handle since target found.
	if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
	{
		pLog->WriteMessage(_T("Unable to close(SNAPMODULE)!"), true, true);
	}

	// Cleanup.
	delete[] psMessage;

	//// Module Entry End.
	return true;
}


//// Take snapshot of the running processes.
bool SnapshotProcesses(TCHAR* psTarget /*= NULL*/)
{
	//// Process Entry Begin.
	pLog->WriteMessage(_T("\n -- Processes --"));
	pLog->WriteTimestamp();

	// Obtain initial list of running processes, no heap.
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS |
												 TH32CS_SNAPNOHEAPS,
												 NULL );
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("SNAPPROCESS wNOHEAPS"));
		return false;
	}

	// Setup process structure.
	PROCESSENTRY32 ProcessEntry = {0};
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32); // Set size before use.

	// Get the first process in the list.
	if (Process32First(hSnapshot, &ProcessEntry) == FALSE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("Process32First"));
		if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
		{
			pLog->WriteMessage(_T("Unable to close(SNAPPROCESS wNOHEAPS)!"), true, true);
		}
		return false;
	}
	// Note that the first entry can and probably should be skipped since it is
	//  a system process and not usually relevant.  However, for consistency,
	//  it shall remain to provide a complete snapshot.
	do
	{
		// Look at only target or everything.
		if (psTarget == NULL ||
			(psTarget != NULL && _tcsicmp(ProcessEntry.szExeFile, psTarget) == 0))
		{
			// Log snapshot of process.
			ProcessSnapshot(&ProcessEntry);
		}

		// Get the next process in the list.
	} while(Process32Next(hSnapshot, &ProcessEntry) == TRUE);

	// Close handle.
	if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
	{
		pLog->WriteMessage(_T("Unable to close(SNAPPROCESS wNOHEAPS)!"), true, true);
	}

	//// Process Entry End.
	return true;
}


//// Take snapshot of process's threads.
bool SnapshotThreads(DWORD nPID)
{
	//// Thread Entry Begin.
	pLog->WriteMessage(_T("\n\n -- Threads --"));
	pLog->WriteTimestamp();
	pLog->WriteMessage(_T("-- 0xID --  -- Owner --  -- User --  -- nUse --"));

	// Get a snapshot of the threads.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, nPID /*Ignored*/);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("SNAPTHREAD"));
		return false;
	}

	// Setup thread structure.
	THREADENTRY32 ThreadEntry = {0};
	ThreadEntry.dwSize = sizeof(THREADENTRY32);

	// Loop varibles.
	TCHAR* psMessage = new TCHAR[64];

	// Get the process's thread details.
	if (Thread32First(hSnapshot, &ThreadEntry) == FALSE)
	{
		// See GetLastError().
		pLog->WriteLastError(_T("Thread32First"));
		if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
		{
			pLog->WriteMessage(_T("Unable to close(SNAPTHREAD)!"), true, true);
		}
		return false;
	}
	do
	{
		// Skip threads not specifically owned by the target process.
		if (nPID != ThreadEntry.th32OwnerProcessID)
		{
			continue;
		}

		// Log the thread.
		_stprintf(psMessage, _T("0x%8X   0x%8X  0x%8X  %10u  \0"), ThreadEntry.th32ThreadID,
																   ThreadEntry.th32OwnerProcessID,
																   ThreadEntry.th32CurrentProcessID,
																   ThreadEntry.cntUsage);
		pLog->WriteMessage(psMessage);

		// Get the next thread entry in the list.
	} while (Thread32Next(hSnapshot, &ThreadEntry) == TRUE);


	// Close list handle since target found.
	if (CloseToolhelp32Snapshot(hSnapshot) == FALSE)
	{
		pLog->WriteMessage(_T("Unable to close(SNAPTHREAD)!"), true, true);
	}

	// Cleanup.
	delete[] psMessage;

	//// Thread Entry End.
	return true;
}


bool SnapshotWindows(DWORD nTargetPID, DWORD nLevel /*= 0*/, HWND hWnd /*= NULL*/)
{
	//bool bContinue(true);
	//CArray<HWND, HWND> ahWnd;
	//while (bContinue)
	//{

	//}

	//if (!GetProcessWindows(nPID, &ahWnd))
	//{
	//	// See GetLastError().
	//	pLog->WriteLastError(_T("NO HWNDs"), true, true);
	//	return false;
	//}

	// Setup Header.
	if (nLevel == 0)
	{
		//// Window Entry Begin.
		pLog->WriteMessage(_T("\n\n -- Windows --"));
		pLog->WriteTimestamp();
		//pLog->WriteMessage(_T("-- hWnd --  -- Class Name --                  -- Atom --  -- ExMem Sz --  -- Style --"));
		pLog->WriteMessage(_T("-- hWnd --  -- Class Name --                                  -- Title --                                       -- Icon --  -- Cursor --  -- Style --  -- Rectangle Coordinates --  -- Height --  -- Width  --  -- Enabled --  -- Active --  -- Visible --  -- Unicode --  -- State --"));
	}
	/*pLog->WriteNewLine();
	for (DWORD x = 0; x < nLevel; ++x)
	{
		pLog->WriteCharacter(_T('\t'));
	}
	pLog->WriteMessage(_T("-- hWnd --  -- Class Name --                  -- Atom --  -- ExMem Sz --  -- Style --"), false);*/

	// Loop varibles.
	DWORD nCount(0);
	TCHAR* psMessage = new TCHAR[512];
	TCHAR* psClassName = new TCHAR[48];
	TCHAR* psTitle = new TCHAR[48];
	RECT Rectangle = {0};
	HDC hDC = NULL;
	HWND hActive = GetActiveWindow();

	SYSTEMTIME Time;
	GetLocalTime(&Time);
	TCHAR* psFilename = new TCHAR[256];

	// Get the topmost Window handle.
	// If hWnd is NULL, then the handle returned is the topmost z-order window.
	// Else, the handle is the topmost z-order child window of the parent window.
	hWnd = GetTopWindow(hWnd);

	// Find the process's Window handle.
	DWORD nCurrentPID = 0;
	while (hWnd != NULL)
	{
		// Check if the Window is part of the target.
		GetWindowThreadProcessId(hWnd, &nCurrentPID);
		if (nCurrentPID == nTargetPID)
		{
			nCount++;

			// References
			// http://msdn.microsoft.com/en-us/library/aa931797
			// http://msdn.microsoft.com/en-us/library/aa932895
			// http://msdn.microsoft.com/en-us/library/ms885631
			// http://msdn.microsoft.com/en-us/library/aa923118
			// http://msdn.microsoft.com/en-us/library/aa909175.aspx
			// http://msdn.microsoft.com/en-us/library/aa915081
			// http://msdn.microsoft.com/en-us/library/ms905085
			// http://msdn.microsoft.com/en-us/library/ms894920
			// http://msdn.microsoft.com/en-us/library/ms894477
			// http://msdn.microsoft.com/en-us/library/ms886730
			// http://msdn.microsoft.com/en-us/library/aa932480.aspx

			// Retrieve the Window Info.
			memset(psClassName, '\0', sizeof(TCHAR) * 48);
			GetClassName(hWnd, psClassName, 48);
			memset(psTitle, '\0', sizeof(TCHAR) * 48);
			GetWindowText(hWnd, psTitle, 48);
			GetWindowRect(hWnd, &Rectangle);
			//hDC = GetWindowDC(hWnd);

			_stprintf(psFilename,
				  _T("%s\\%u-%02u-%02u-%02u%02u-%02u-%08X_%s\0"), _T("\\Temp"),
																 Time.wYear,
																 Time.wMonth,
																 Time.wDay,
																 Time.wHour,
																 Time.wMinute,
																 Time.wSecond,
																 hWnd,
																 psTitle == NULL ? psClassName == NULL ? _T("Item") : psClassName : psTitle);
			CaptureDCToFile(hWnd, psFilename); // Works, but takes time to process all the windows.

			// Log the Window Info.
			_stprintf(psMessage, _T("0x%8X  %-48s  %-48s  0x%08X    0X%08X   0x%08X  l:%-4ld t:%-4ld r:%-4ld b:%-4ld  %12d  %12d  %13s  %12s  %13s  %13s  %11s\0"),
																		   hWnd,
																		   psClassName,
																		   psTitle == NULL ? _T("") : psTitle,
																		   GetClassLong(hWnd, GCL_HICON),
																		   GetClassLong(hWnd, GCL_HCURSOR),
																		   GetClassLong(hWnd, GCL_STYLE),
																		   Rectangle.left,
																		   Rectangle.top,
																		   Rectangle.right,
																		   Rectangle.bottom,
																		   Rectangle.bottom - Rectangle.top,
																		   Rectangle.right - Rectangle.left,
																		   IsWindowEnabled(hWnd) ? _T("Enabled") : _T("Disabled"),
																		   hActive == hWnd ? _T("!!!Active!!!") : _T(""),
																		   IsWindowVisible(hWnd) ? _T("Visible") : _T("Invisible"),
																		   IsWindowUnicode(hWnd) ? _T("Unicode") : _T("ASCII"),
																		   IsIconic(hWnd) ? _T("Minimized") : _T("Normal"));
			pLog->WriteNewLine();
			for (DWORD x = 0; x < nLevel; ++x)
			{
				pLog->WriteCharacter(_T('\t'));
			}
			pLog->WriteMessage(psMessage, false);

			// Check each of the Window's children.
			if (SnapshotWindows(nTargetPID, nLevel+1, hWnd))
			{
				pLog->WriteNewLine();
			}
		}

		// Get the Next Window handle below the current.
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	// Cleanup.
	delete[] psFilename;
	delete[] psClassName;
	delete[] psMessage;

	// Done.
	if (nCount > 0)
	{
		SetLastError(ERROR_SUCCESS);
		return true;
	}
	SetLastError(ERROR_NO_DATA_DETECTED);
	return false;
}


//// Convert String Numeric Value to Unsigned Long.
////  Only analyzes string til first space character or end line found,
////   then attempts to convert the characters up to that break.
////  Returns 0 if conversion failed.
//DWORD StringToDWORD(LPCTSTR sValue)
//{
//	if (sValue == NULL) { return 0; }
//	DWORD d;
//	LPTSTR pEnd;
//	d = (DWORD)_tcstoul(sValue, &pEnd, 10);
//	return d;
//}
//
//
//// Convert Unsigned Long to String Numeric Value.
////  Returns NULL if conversion failed.
//LPCTSTR DWORDToString(DWORD nValue)
//{
//	TCHAR* s = new TCHAR[12]; // 10 digits + sign + null
//	memset(s, '\0', sizeof(TCHAR) * 12);
//	if (_tcslen(_ultot(nValue, s, 10)) > 0)
//	{
//		return s;
//	}
//	return NULL;
//}
//
//
//// Convert Signed Long to String Numeric Value.
////  Returns NULL if conversion failed.
//LPCTSTR LongToString(long nValue)
//{
//	TCHAR* s = new TCHAR[12]; // 10 digits + sign + null
//	memset(s, '\0', sizeof(TCHAR) * 12);
//	if (_tcslen(_ltot(nValue, s, 10)) > 0)
//	{
//		return s;
//	}
//	return NULL;
//}


typedef struct tagWINDOWENTRY32 {
	HWND hWnd;
	TCHAR saClassName[MAX_PATH];
	TCHAR saTitle[MAX_PATH];
	HICON hIcon;
	HCURSOR hCursor;
	LONG nStyle;
	LONG nStyleEx;
	LONG nStyleBits;
	RECT rCoordinates;
	bool bEnabled;
	bool bActive;
	bool bVisible;
	bool bUnicode;
	bool bMinimized;
	WNDPROC hWndProc;
	DLGPROC hDlgProc;
	LONG nID;
	LONG nUserData;
	LONG nMsgResult;
	LONG nDlgUserData;
} WINDOWENTRY32, *PWINDOWENTRY32, *LPWINDOWENTRY32;


//// Save a screenshot of the primary display to an image file.
// hWnd      - (IN) Handle to the Window for the DC.
// sFilename - (IN) Path and filename (excluding extension) of the image file.
// nEncoder  - (IN) Image encoder to use if available, else manual bitmap is generated.
// Returns true on success and false on failure. See GetLastError for details.
bool CaptureDCToFile(HWND hWnd, LPTSTR sFilename, EncoderType nEncoder /* = EncoderTypePNG */)
{
	// Validate parameters.
	if (sFilename == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Retrieve the device context handle to the entire screen.
	HDC hDC = (hWnd == NULL ? GetDC( NULL ) : GetWindowDC( hWnd ));
	//HDC hDC = GetDC(hWnd);
	if (hDC == NULL)
	{
		// See GetLastError().
		return false;
	}

	// Create a compatible device context handle for the bitmap.
	HDC cDC = CreateCompatibleDC(hDC);
	if (cDC == NULL)
	{
		// See GetLastError().
		ReleaseDC( NULL, hDC );
		return false;
	}

	// Get the system metrics.
	//  Note: Define HI_RES_AWARE   CEUX   {1} in the resource for
	//   compatibility with screen resolutions higher than 240x320.
	RECT r = {0};
	if (hWnd != NULL) { GetWindowRect( hWnd, &r ); }
	int w = (hWnd == NULL ? GetDeviceCaps( hDC, HORZRES ) : r.right - r.left);  // Width (in pixels).
	int h = (hWnd == NULL ? GetDeviceCaps( hDC, VERTRES ) : r.bottom - r.top);  // Height (in pixels).
	int b = GetDeviceCaps( hDC, COLORRES ); // Bit Depth

	// Validate bit depth.
	// Bit depths below 16 are not supported (for now). Need to add
	//  support for colorTables to use bit depths lower than 16.
	/*if      (b <  4) { b =  1; }
	else if (b <  8) { b =  4; }
	else if (b < 16) { b =  8; }*/
	if      (b < 16) { return false; }
	else if (b < 24) { b = 16; }
	else if (b < 32) { b = 24; }
	else             { b = 32; }

	// Bitmap Information Header "DIB" Section.
	//  Initilize to zero, then only set what is needed.
	BITMAPINFOHEADER bih = {0};
	bih.biSize        = sizeof(BITMAPINFOHEADER);
	bih.biWidth       = abs(w);
	bih.biHeight      = abs(h);
	bih.biPlanes      = 1;
	bih.biBitCount    = b;
	bih.biCompression = BI_RGB;
	
	// Bitmap Info Structure.
	//  No color table is needed for bit depths above 8.
	//  Color Table only provides optimizations for bit depths above 8.
	BITMAPINFO bi;
	bi.bmiHeader = bih;
	bi.bmiColors[0].rgbBlue     = 0;
	bi.bmiColors[0].rgbGreen    = 0;
	bi.bmiColors[0].rgbRed      = 0;
	bi.bmiColors[0].rgbReserved = 0;

	// Define Screen Capture structure.
	CAPTURE Capture;
	Capture.pBitmapInfo = &bi;
	Capture.sFilename   = sFilename;

	// Create the bitmap specified in the header and assign the pointers.
	Capture.hBitmap = CreateDIBSection(hDC, Capture.pBitmapInfo, DIB_RGB_COLORS,
									   &Capture.pBytes, NULL, 0);
	if (Capture.hBitmap == NULL || Capture.pBytes == NULL)
	{
		// See GetLastError()
		ReleaseDC( NULL, hDC );
		DeleteDC(cDC);
		return false;
	}

	// Associate the bitmap with the compatible device context handle.
	HGDIOBJ hOld = SelectObject(cDC, Capture.hBitmap);
	if (hOld == NULL || hOld == (HGDIOBJ)HGDI_ERROR)
	{
		// See GetLastError()
		ReleaseDC( NULL, hDC );
		DeleteDC(cDC);
		DeleteObject(Capture.hBitmap);
		return false;
	}

	// Copy the bit data.
	//  Use StretchBlt to flip the data vertically.
	//if (BitBlt(cDC,
	//		   0, 0,
	//		   Capture.pBitmapInfo->bmiHeader.biWidth,
	//		   Capture.pBitmapInfo->bmiHeader.biHeight,
	//		   hDC,
	//		   0, 0,
	//		   SRCCOPY) == FALSE)
	if (StretchBlt(cDC,
				   0, 0,
				   Capture.pBitmapInfo->bmiHeader.biWidth,
				   Capture.pBitmapInfo->bmiHeader.biHeight,
				   hDC,
				   0, Capture.pBitmapInfo->bmiHeader.biHeight,
				   Capture.pBitmapInfo->bmiHeader.biWidth,
				   -1 * Capture.pBitmapInfo->bmiHeader.biHeight,
				   SRCCOPY) == FALSE)
	{
		// See GetLastError()
		ReleaseDC( NULL, hDC );
		DeleteDC(cDC);
		DeleteObject(Capture.hBitmap);
		return false;
	}

	// Cleanup
	SelectObject(cDC, hOld); // Restore DC object.
	ReleaseDC( NULL, hDC );
	DeleteDC(cDC);

	// Save the screen capture.
	bool bSuccess = true;
	if (!SaveEncodedCapture(&Capture, nEncoder))
	{
		// Save as manual bitmap and compress.
		if (!SaveCapture(&Capture, true))
		{
			bSuccess = false;
		}
	}

	// Cleanup.
	DeleteObject(Capture.hBitmap); // (Note: This also deletes the pBytes array!)

	return bSuccess;
}


//// Manually generate a bitmap file using the screenshot data.
// pCapture  - (IN) Pointer to a filled CAPTURE structure.
// bCompress - (IN) Flag to compress the bitmap to a zip file.
// Returns true on success and false on failure. See GetLastError for details.
bool SaveCapture(PCAPTURE pCapture, bool bCompress /* = false */)
{
	// Validate parameters.
	if (pCapture == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Calculate the size of the bitmap data. (in bytes).
	int nBytes = pCapture->pBitmapInfo->bmiHeader.biWidth  *
				 pCapture->pBitmapInfo->bmiHeader.biHeight *
				 (pCapture->pBitmapInfo->bmiHeader.biBitCount / 8);
	int nFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + nBytes;

	// Bitmap File Header.
	BITMAPFILEHEADER bfh = {0};
	bfh.bfType    = 0x4D42; //BM
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	bfh.bfSize    = bfh.bfOffBits + nBytes;

	// Adjust for vertically flipped image.
	//  (Be sure not to use negative value for calculations.)
	pCapture->pBitmapInfo->bmiHeader.biHeight *= -1;

	// Create the bitmap file in memory.
	// (stack and heap take about the same time to process).
	// Pick which ever method you prefer.
	//byte* pBitmapFile = (byte*)malloc(nFileSize); // Do not forget to free.
	byte* pBitmapFile = new byte[nFileSize]; // Do not forget to delete.
	byte* pIndex = pBitmapFile;

	// Copy Bitmap File Header Structure.
	memcpy(pIndex, &bfh, sizeof(BITMAPFILEHEADER));
	pIndex += sizeof(BITMAPFILEHEADER);

	// Copy Bitmap Info structure.
	memcpy(pIndex, pCapture->pBitmapInfo, sizeof(BITMAPINFO));
	pIndex += sizeof(BITMAPINFO);

	// Copy Bitmap data.
	memcpy(pIndex, pCapture->pBytes, nBytes);

	// Append format to string setup.
	int nLength = _tcslen(pCapture->sFilename);
	TCHAR* psFilename = new TCHAR[nLength + 5];
	_tcsncpy(psFilename, pCapture->sFilename, nLength);

	// Check for compression.
	if (bCompress)
	{
		// Append format to string.
		_tcsncpy(psFilename + nLength, _T(".zip\0"), 5);

		// Zip the Bitmap and Save.
		HZIP zFile = CreateZip(psFilename, NULL);
		if (zFile != NULL)
		{
			if (ZipAdd(zFile, _T("Screenshot.bmp"), pBitmapFile, nFileSize) != ZR_OK)
			{
				delete[] pBitmapFile;
				return false;
			}
			CloseZip(zFile);
		}

		// Cleanup.
		delete[] psFilename;
		delete[] pBitmapFile;

		// Capture saved and compressed.
		return true;
	}

	// Append format to string.
	_tcsncpy(psFilename + nLength, _T(".bmp\0"), 5);

	// Create bitmap file.
	HANDLE hFile = CreateFile(psFilename, GENERIC_WRITE, NULL, NULL,
							  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		delete[] pBitmapFile;
		return false;
	}

	DWORD nWritten;
	if (WriteFile(hFile, pBitmapFile, nFileSize, &nWritten, NULL) != TRUE)
	{
		// See GetLastError().
		delete[] pBitmapFile;
		CloseHandle(hFile);
		return false;
	}

	// Close the file.
	CloseHandle(hFile);

	// Cleanup.
	delete[] psFilename;
	delete[] pBitmapFile;

	// Capture saved.
	return true;
}


//// Saves the screen capture using the selected image encoder.
// pCapture - (IN) Pointer to a filled CAPTURE structure.
// nEncoder - (IN) Image encoder to use if available.
// Returns true on success and false on failure. See GetLastError for details.
bool SaveEncodedCapture(PCAPTURE pCapture, EncoderType nEncoder /* = EncoderTypePNG */)
{
	// Validate parameters.
	if (pCapture == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Results variable.
	HRESULT hResult;

	// Initialize COM library.
	hResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hResult != S_OK && hResult != S_FALSE)
	{
		return false;
	}

	// Create Image Factory object.
	IImagingFactory* pImageFactory = NULL;
	hResult = CoCreateInstance(CLSID_ImagingFactory,
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   IID_IImagingFactory,
							   (void**)&pImageFactory);
	if (hResult != S_OK || pImageFactory == NULL)
	{
		CoUninitialize();
		return false;
	}

	// Get the installed encoders.
	UINT nCount = 0;
	ImageCodecInfo* pImageCodecInfo = NULL;
	hResult = pImageFactory->GetInstalledEncoders(&nCount, &pImageCodecInfo);
	if (hResult != S_OK || pImageCodecInfo == NULL)
	{
		CoUninitialize();
		return false;
	}

	// Append format to string setup.
	int nLength = _tcslen(pCapture->sFilename);
	TCHAR* psFilename = new TCHAR[nLength + 5];
	_tcsncpy(psFilename, pCapture->sFilename, nLength);

	// Set desired format. (default: png).
	//  In case of encoder failure, fallback to bitmap.
	WCHAR* sFormat;
	switch(nEncoder)
	{
	case EncoderTypeBMP:
		sFormat = L"image/bmp";
		_tcsncpy(psFilename + nLength, _T(".bmp\0"), 5);
		break;
	//// GIF and JPEG format options are not wanted due to licensing.
	//case EncoderTypeGIF:
	//	sFormat = L"image/gif";
	//	_tcsncpy(psFilename + nLength, _T(".gif\0"), 5);
	//	break;
	//// JPEG encoder seems to require 32 bit images only.
	//case EncoderTypeJPEG:
	//	sFormat = L"image/jpg";
	//	_tcsncpy(psFilename + nLength, _T(".jpg\0"), 5);
	//	break;
	case EncoderTypeTIFF:
		sFormat = L"image/tiff";
		_tcsncpy(psFilename + nLength, _T(".tif\0"), 5);
		break;
	case EncoderTypePNG:
	default:	
		sFormat = L"image/png";
		_tcsncpy(psFilename + nLength, _T(".png\0"), 5);
		break;
	}

	// Loop Setup.
	bool bFound = false;
	CLSID encoderID = {0};

	// Search for desired encoder.
	for(UINT k = 0; k < nCount && !bFound; ++k)
	{
		if (wcscmp(pImageCodecInfo[k].MimeType, sFormat) == 0)
		{
			encoderID = pImageCodecInfo[k].Clsid;
			bFound = true;
		}
	}

	// Cleanup.
	CoTaskMemFree(pImageCodecInfo);

	// Desired encoder not found.
	if (!bFound)
	{
		CoUninitialize();
		return false;
	}

	// Link encoder to an image file.
	IImageEncoder* pImageEncoder = NULL;
	hResult = pImageFactory->CreateImageEncoderToFile(&encoderID,
													  T2CW(psFilename),
													  &pImageEncoder);
	if (hResult != S_OK || pImageEncoder == NULL)
	{
		delete[] psFilename;
		CoUninitialize();
		return false;
	}

	// Cleanup.
	delete[] psFilename;

	// Obtain image sink from encoder.
	IImageSink* pImageSink = NULL;
	hResult = pImageEncoder->GetEncodeSink(&pImageSink);
	if (hResult != S_OK || pImageSink == NULL)
	{
		CoUninitialize();
		return false;
	}

	// Build the Image Info structure.
	ImageInfo ImgInfo;
	ImgInfo.RawDataFormat = ImageFormatMemoryBMP;
	ImgInfo.Width         = pCapture->pBitmapInfo->bmiHeader.biWidth;
	ImgInfo.Height        = pCapture->pBitmapInfo->bmiHeader.biHeight;
	ImgInfo.Flags         = SinkFlagsBottomUp | SinkFlagsFullWidth;
	INT nPixelFormat;
	int b = pCapture->pBitmapInfo->bmiHeader.biBitCount;
	if      (b <  4) { nPixelFormat = PixelFormat1bppIndexed; }
	else if (b <  8) { nPixelFormat = PixelFormat4bppIndexed; }
	else if (b < 16) { nPixelFormat = PixelFormat8bppIndexed; }
	else if (b < 24) { nPixelFormat = PixelFormat16bppRGB555; }
	else if (b < 32) { nPixelFormat = PixelFormat24bppRGB;    }
	else             { nPixelFormat = PixelFormat32bppARGB;   
	                   ImgInfo.Flags |= SinkFlagsHasAlpha;    }
	ImgInfo.PixelFormat   = nPixelFormat;

	// Initialize the sink.
	hResult = pImageSink->BeginSink(&ImgInfo, NULL);
	if (hResult != S_OK)
	{
		CoUninitialize();
		return false;
	}

	// Create the color palette.
	//  Only a blank palette (for now) since current implementation
	//   only supports bit depths 16 and above.
	ColorPalette* pPalette = NULL;
	pPalette = (ColorPalette*)malloc(sizeof(ColorPalette));
	pPalette->Count = 0;
	if (pCapture->pBitmapInfo->bmiHeader.biBitCount > 24)
	{
		pPalette->Flags = PALFLAG_HASALPHA;
	}

	// Send the color palette to the sink.
	hResult = pImageSink->SetPalette(pPalette);
	if (hResult != S_OK)
	{
		CoUninitialize();
		free(pPalette);
		return false;
	}

	// Cleanup.
	free(pPalette);

	// Calculate the stride (Bytes per Scanline).
	INT nBitsPerLine = ImgInfo.Width * pCapture->pBitmapInfo->bmiHeader.biBitCount;
	INT nBitsPerLong = sizeof(LONG) * 8; // 256
	INT nStride      = nBitsPerLong * (nBitsPerLine / nBitsPerLong);
	if ((nBitsPerLine % nBitsPerLong) != 0)
	{
		// Adjust for inbalance.
		nStride += nBitsPerLong;
	}
	// Convert back to bytes from bits.
	nStride = nStride / 8;

	// Create Bitmap Data structure.
	BitmapData Data;
	Data.Height      = ImgInfo.Height;
	Data.Width       = ImgInfo.Width;
	Data.Stride      = nStride;
	Data.PixelFormat = nPixelFormat;
	Data.Scan0       = pCapture->pBytes;
	Data.Reserved    = 0;

	// Create a bitmap from the buffer.
	bool bSuccess = false;
	IBitmapImage* pBitmap;
	hResult = pImageFactory->CreateBitmapFromBuffer(&Data, &pBitmap);
	if (hResult == S_OK)
	{
		// Create an image interface for the bitmap.
		IImage* pImage;
		hResult = pBitmap->QueryInterface(IID_IImage, (void**)&pImage);
		if (hResult == S_OK)
		{
			// Send the image through the encoder and to the file.
			hResult = pImage->PushIntoSink(pImageSink);
			if (hResult == S_OK)
			{
				bSuccess = true;
			}
		}
	}

	// Close the sink.
	pImageSink->EndSink(S_OK);

	// Cleanup.
	pImageSink->Release();
	
	// Close the encoder.
	pImageEncoder->TerminateEncoder();

	// Cleanup.
	CoUninitialize();

	// Capture saved.
	return bSuccess;
}
