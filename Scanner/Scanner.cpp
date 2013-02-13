/* File:	Scanner.cpp
 * Created:	Jan 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

/* Icon Source "Barcode Scanner"
 * License: Unknown, Icon from Android Barcode Scanner Application
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"
#include "ScannerDialog.h"
// Millennium Mobile's Message Definitions. (For Conflict Reference Only)
//#include "WindowsMessages.h"

//// The application object.
CWinApp theApp;

//// Definitions.
// (http://msdn.microsoft.com/en-us/library/aa911386.aspx)
#define TH32CS_SNAPNOHEAPS   0x40000000
#define APPLICATION_TARGET   _T("mmobile.exe")
// Prompt Type Flags
#define PT_DEFAULT           MB_TASKMODAL | MB_TOPMOST | MB_OK
#define PT_ERROR             PT_DEFAULT | MB_ICONERROR
#define PT_INFO              PT_DEFAULT | MB_ICONINFORMATION
#define PT_QUESTION          MB_TASKMODAL | MB_TOPMOST | MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
#define PT_WARNING           PT_DEFAULT | MB_ICONWARNING

//// Function Prototypes.
bool GetProcessEntry(TCHAR* psTarget, PROCESSENTRY32* pProcess);
HWND GetProcessHwnd (DWORD nPID, HWND hWnd = NULL);
bool GetTargetHWNDs (DWORD nTargetPID, CArray<HWND, HWND>* pahWnd, HWND hWnd = NULL);
int Prompt          (UINT nType, UINT nTitleID, UINT nMessageID, ...);
int Prompt          (UINT nType, LPCTSTR psTitle, LPCTSTR psMessage, ...);
int Prompt          (UINT nType, CString sTitle, CString sMessage, va_list pArgList);
int PromptForCommand(CString& sBarcode);
bool SendMessageTo  (CArray<HWND, HWND>* pahWnd, CString sMessage);

//// Program entry point.
// argc - (IN) Number of command line arguments.
// argv - (IN) Array of string pointers to the arguments.
// envp - (IN) Array of string pointers to environment parameters.
// Returns a numeric exit code. 0 for success, else n for status of exit.
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	// Set High Thread Priority.
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// Initialize MFC.
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		return 1;
	}

	// Find the process.
	PROCESSENTRY32 ProcessEntry;
	if(!GetProcessEntry(APPLICATION_TARGET, &ProcessEntry))
	{
		// Failed to find the process.
		Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_NOT_FOUND, APPLICATION_TARGET);
		return 1;
	}

	// Get the target process's window handles.
	CArray<HWND, HWND> ahWnd;
	GetTargetHWNDs(ProcessEntry.th32ProcessID, &ahWnd);
	if (ahWnd.GetCount() < 1)
	{
		// No Windows found.
		Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_NO_WINDOW, APPLICATION_TARGET);
		return 1;
	}

	// Get the message to send.
	CString sMessage = _T("Snowflake");
	int nReturn = PromptForCommand(sMessage);
	if (nReturn != 0)
	{
		return nReturn;
	}

	// Send the message.
	SendMessageTo(&ahWnd, sMessage);

	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Scanner"), _T("Done"));
	#endif

	// Done.
	return 0;
}


bool SendMessageTo(CArray<HWND, HWND>* pahWnd, CString sMessage)
{
	// Validate Parameters.
	if (pahWnd == NULL || sMessage.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	// Setup the Copy Data Structure.
	COPYDATASTRUCT cds = {0};
	cds.cbData = sMessage.GetLength() * sizeof(TCHAR);
	cds.dwData = 0x42435345; // BCSE
	cds.lpData = sMessage.GetBuffer(cds.cbData);

	// Loop Variables.
	DWORD nReturn;
	LRESULT nResult;

	// Loop through each Target Window.
	for (int x = 0; x < pahWnd->GetCount(); ++x)
	{
		// Send the Message.
		//SendMessage(ahWnd.GetAt(x),   // Target's Window handle.
		//			WM_COPYDATA,      // Message.
		//			(WPARAM)NULL,     // Sender's Window handle.
		//			(LPARAM)&cds);    // Pointer to the Data.
		nResult = SendMessageTimeout(pahWnd->GetAt(x), // Target's Window handle.
									 WM_COPYDATA,      // Message.
									 (WPARAM)NULL,     // Sender's Window handle.
									 (LPARAM)&cds,     // Pointer to the Data.
									 SMTO_NORMAL,      // Only CE spported flag.
									 2000,             // Timeout in milliseconds.
									 &nReturn);        // Result from processing.
		//CallWindowProc(

		/*CString sTemp;
		GetWindowText(pahWnd->GetAt(x), sTemp.GetBuffer(100), 100);
		sTemp.ReleaseBuffer();
		MessageBox(NULL, sTemp, _T("Window Text"), MB_OK);*/
	}

	// Cleanup.
	sMessage.ReleaseBuffer();

	// Analyze Results.

	return true;
}


//// Take snapshot of the running processes.
bool GetProcessEntry(TCHAR* psTarget, PROCESSENTRY32* pProcess)
{
	// Validate parameters.
	if (psTarget == NULL || pProcess == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	// Obtain initial list of running processes, no heap.
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS |
												 TH32CS_SNAPNOHEAPS,
												 NULL );
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		// See GetLastError().
		return false;
	}

	// Setup process structure.
	pProcess->dwSize = sizeof(PROCESSENTRY32); // Set size before use.

	// Get the first process in the list.
	if (Process32First(hSnapshot, pProcess) == FALSE)
	{
		// See GetLastError().
		CloseToolhelp32Snapshot(hSnapshot);
		return false;
	}
	// Note that the first entry can and probably should be skipped since it is
	//  a system process and not usually relevant.  However, for consistency,
	//  it shall remain to provide a complete snapshot.
	do
	{
		// Look for the target.
		if (_tcsicmp(pProcess->szExeFile, psTarget) == 0)
		{
			// Close handle.
			CloseToolhelp32Snapshot(hSnapshot);

			//// Process Entry End.
			return true;
		}

		// Get the next process in the list.
	} while(Process32Next(hSnapshot, pProcess) == TRUE);

	// Close handle.
	CloseToolhelp32Snapshot(hSnapshot);

	//// Process Entry End.
	SetLastError(ERROR_NOT_FOUND);
	return false;
}


bool GetTargetHWNDs(DWORD nTargetPID, CArray<HWND, HWND>* pahWnd, HWND hWnd /*= NULL*/)
{
	// Validate parameters.
	if (pahWnd == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

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
			// Add the Window's handle to the list.
			pahWnd->Add(hWnd);

			// Check each of the Window's children.
			GetTargetHWNDs(nTargetPID, pahWnd, hWnd);
		}

		// Get the Next Window handle below the current.
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	// No handles found.
	if (pahWnd->GetCount() == 0)
	{
		SetLastError(ERROR_NOT_FOUND);
		return false;
	}

	// Done.
	SetLastError(ERROR_SUCCESS);
	return true;
}


//// Format, Build, and Display a message to the user.
// nType      - (IN) Prompt dialog numeric type.
// nTitleID   - (IN) ID of the title/caption String.
// nMessageID - (IN) Format string resource id. Uses %n as argument placeholders.
// ...        - (IN) Arguments to replace %n(s) in message.
// Returns the result of the dialog else -1 if in silent mode, -2 if invalid
//  message, -3 if the message formatting failed.
// See GetLastError for more detailed failure information.
int Prompt(UINT nType, UINT nTitleID, UINT nMessageID, ...)
{
	// Setup.
	SetLastError(ERROR_SUCCESS);

	// Load the Message.
	CString sMessage;
	if (sMessage.LoadString(nMessageID) == 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -2;
	}

	// Load the Title.
	CString sTitle;
	if (sTitle.LoadString(nTitleID) == 0)
	{
		// Use Application Title on Failure.
		//sTitle.LoadString(IDS_APP_TITLE);
	}

	// Setup Argument List.
	va_list pArgList;
	va_start(pArgList, nMessageID);

	// Call Real Function.
	int nRet = Prompt(nType, sTitle, sMessage, pArgList);

	// Cleanup and Return Results.
	va_end(pArgList);
	return nRet;
}


//// Format, Build, and Display a message to the user.
// nType     - (IN) Prompt dialog numeric type.
// psTitle   - (IN) Title string for the prompt.
// psMessage - (IN) Format string using %n as argument placeholders.
// ...       - (IN) Arguments to replace %n(s) in the message. (OR)
// Returns the result of the dialog else -1 if in silent mode, -2 if invalid
//  message, -3 if the message formatting failed.
// See GetLastError for more detailed failure information.
int Prompt(UINT nType, LPCTSTR psTitle, LPCTSTR psMessage, ...)
{
	// Setup.
	SetLastError(ERROR_SUCCESS);

	// Validate Parameters.
	if (psMessage == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -2;
	}
	
	// Load the Message.
	CString sMessage(psMessage);

	// Load the Title.
	CString sTitle = _T("");
	if (psTitle != NULL)
	{
		sTitle = CString(psTitle);
	}

	// Setup Argument List.
	va_list pArgList = NULL;
	va_start(pArgList, psMessage);

	// Call Real Function.
	int nRet = Prompt(nType, sTitle, sMessage, pArgList);

	// Cleanup and Return Results.
	va_end(pArgList);
	return nRet;
}


//// Format, Build, and Display a message to the user.
// nType    - (IN) Prompt dialog numeric type.
// sTitle   - (IN) Title string for the prompt.
// sMessage - (IN) Format string using %n as argument placeholders.
// pArgList - (IN) Arguments to replace %n(s) in the message.
// Returns the result of the dialog else -1 if in silent mode, -2 if invalid
//  message, -3 if the message formatting failed.
// See GetLastError for more detailed failure information.
int Prompt(UINT nType, CString sTitle, CString sMessage, va_list pArgList)
{
	// Setup.
	SetLastError(ERROR_SUCCESS);

	// Validate parameter.
	if (sMessage.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -2;
	}
	//if (sTitle.IsEmpty())
	//{
	//	sTitle.LoadString(IDS_APP_TITLE);
	//}

	// Format the message.
	LPTSTR psTemp;
	if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | 
					  FORMAT_MESSAGE_ALLOCATE_BUFFER,
					  (LPCVOID)sMessage,
					  0, /* Error ID */
					  0, /* Default language */
					  (LPWSTR)&psTemp,
					  0, /* Minimum allocation size */
					  &pArgList) == 0 || psTemp == NULL)
	{
		// See GetLastError().
		return -3;
	}

	// Display the message.
	int nReturn = MessageBox(NULL, psTemp, sTitle, nType);

	// Cleanup.
	LocalFree(psTemp);

	return nReturn;
}


//// Display a dialog prompt to the user for the barcode string.
// sBarcode - (IN)(OUT) The barcode string entered by the user.
// Returns 0 for success, else the do modal return value.
int PromptForCommand(CString& sBarcode)
{
	// Create Dialog.
	ScannerDialog ScannerDlg;

	// Set the string.
	ScannerDlg.SetBarcode(sBarcode);

	// Not fullscreen.
	//ScannerDialog.m_bFullScreen = FALSE;

	// Show Dialog.
	INT_PTR nReturn = ScannerDlg.DoModal();

	// Parse return value.
	switch (nReturn)
	{
	case IDOK:
		// Retrieve string.
		sBarcode = ScannerDlg.GetBarcode();
		break;

	case IDABORT:  // Fallthrough.
	case IDCANCEL: // Fallthrough.
	default:
		// End.
		return (int)nReturn;
		break;
	}
	return 0;
}
