/* File:	Run.cpp
 * Created:	Dec 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"
#include "RunDialog.h"

//// The application object.
CWinApp theApp;

//// Definitions.
#define REG_PATH_SHELL_FOLDERS _T("System\\Explorer\\Shell Folders")
#define REG_PATH_DEVICE_IDENT  _T("Ident")
#define SYSTEM_PATH_SYMBOL     _T('%')
#define APP_WINDOWS_EXPLORER   _T("fexplore.exe")
// Prompt Type Flags
#define PT_DEFAULT             MB_TASKMODAL | MB_TOPMOST | MB_OK
#define PT_ERROR               PT_DEFAULT | MB_ICONERROR
#define PT_INFO                PT_DEFAULT | MB_ICONINFORMATION
#define PT_QUESTION            MB_TASKMODAL | MB_TOPMOST | MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
#define PT_WARNING             PT_DEFAULT | MB_ICONWARNING

//// Enumerations.
enum TFormat
{
	TF_ASCII,
	TF_UTF_1,
	TF_UTF_7,
	TF_UTF_8,
	TF_UTF_16BE,
	TF_UTF_16LE,
	TF_UTF_32BE,
	TF_UTF_32LE,
	TF_UTF_EBCDIC,
	TF_SCSU,
	TF_BOCU_1,
	TF_GB_18030,
	TF_UNKNOWN
};

//// Function Prototypes.
CString ConcatArray       (CArray<CString, CString>* paStrings, int nStart = 0, int nCount = -1, bool bQuote = true);
TFormat DetectTextEncoding(byte* pData);
bool FileExists           (CString& sFilepath, bool bIsDirectory = false, bool bMatch = false);
bool FileExists           (CString& sFilepath, bool& bIsDirectory, int nIgnore = 0);
bool FormatIntoString     (CString& sString, LPCTSTR psFormat, ...);
CString FormatParameters  (CArray<CString, CString>* paArgs);
bool GetAppDirectory      (CString& sDirectory, LPTSTR sArgument = NULL);
bool GetDirectoryAt       (CString sPath, int& nOffset, CString& sDirectory);
bool GetExtensionCommand  (CString sExt, CString& sCommand, CString sVerb = _T(""));
bool GetKeyValueString    (CString sKey, CString sValue, CString& sData, HKEY hRoot = HKEY_CLASSES_ROOT);
bool GetShellFolders      (CMapStringToString* pmShellFolders);
CString GetTableString    (UINT nResourceID);
bool GetVerbCommand       (CString sProgram, CString& sVerb, CString& sCommand, bool bOnlyVerb = false);
int GetWrapCount          (CString sString, TCHAR cSymbol = _T('\"'));
bool HasWhitespace        (CString sString);
bool IsSymbolOnly         (CString sString, TCHAR cSymbol);
//bool IsWrapped            (CString sString, TCHAR cSymbol = _T('\"'), UINT nTimes = 1);
bool IsWrapped            (CString sString, TCHAR cSymbol = _T('\"'), int nTimes = -1);
bool ParseCommandString   (CString sString, CArray<CString, CString>* paArgs);
bool ParseExtensionCommand(CString sString, CArray<CString, CString>* paArgs, CMapStringToString* paFolders = NULL);
bool ParseExternalKeywords(CMapStringToString* pmKeywords);
bool ParseInternalKeywords(CMapStringToString* pmKeywords);
bool ParseKeywords        (CMapStringToString* pmKeywords, CString sKeywords);
bool ParseShowOption      (CString sArg, int& nShow);
bool ParseVerbOption      (CString sArg, CString& sVerb);
int PromptForCommand      (CString& sCommand);
int Prompt                (UINT nType, UINT nTitleID, UINT nMessageID, ...);
int Prompt                (UINT nType, LPCTSTR psTitle, LPCTSTR psMessage, ...);
int Prompt                (UINT nType, CString sTitle, CString sMessage, va_list pArgList);
//  PromptInput
bool ReadEntireFile       (CString sFilename, byte* pData, DWORD& nSize);
bool ReadEntireFileAsText (CString sFilename, CString& sData);
bool ReadEntireFileString (CString sFilename, CString& sData);
bool ReadEntireTextFile   (CString sFilename, CString& sData);
CString RemoveWhitespace  (CString sString);
CString RemoveWrap        (CString sString, TCHAR cSymbol = _T('\"'), int nTimes = -1);
//CString SymbolWrap        (CString sString, TCHAR cSymbol = _T('\"'), bool bRemove = false, bool bDouble = false);
CString SymbolWrap        (CString sString, TCHAR cSymbol = _T('\"'), int nTimes = 1, bool bExact = false);
//CString TripleWrap        (CString sString, TCHAR cSymbol = _T('\"'), bool bRemove = false);
bool ReplaceRelativePaths (CArray<CString, CString>* paArgs);
bool ReplaceRelativePaths (CString& sPath);
bool ReplaceSystemKeywords(CMapStringToString* paKeywords, CString& sString);
bool SearchSystemPaths    (CMapStringToString* paPaths, CString& sFile, bool bMatch = false);
bool ValidateString       (CString sString, bool bBackslash = false);
int WhitespaceFind        (CString sString);
TCHAR* xAsciiToTChar      (byte* pData, UINT nCount);
CString xCharToCString    (char* pData, UINT nCount);
TCHAR* xCharToTChar       (char* pData, UINT nCount);
char* xCStringToChar      (CString sString);
char* xTCharToChar        (TCHAR* pString, UINT nCount);

//// Globals.
CString sHere; // The Application's Relative Directory.

// TODO Base the .\ relative directory against the caller if available.
// TODO Standardize prompts against Microsoft's Run.
// TODO Consider, Should System Paths be checked for System Paths? (Recursive Check)
// TODO Generate PATH and PATHEXT system variables for constant use.
// TODO User PATH instead of Shell folders to search in order of importance.
//  Create a Path.txt file in \Windows\ which contains the PATH and PATHEXT.

// References
// http://msdn.microsoft.com/en-us/library/aa453706
// http://msdn.microsoft.com/en-us/library/aa453707
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb762494(v=vs.85).aspx

//// Command String Format Structure.
// The command string has to follow in this order: Verb Show Command Parameters.
//
// Verb       - (Optional) The shell associated verb for the command. Verbs only
//               work with files and not all verbs are available for each file.
//              If the verb is not found, the default shell action is used.
// Show       - (Optional) For applications only. The show state to send to the
//               application upon execution.
// Command    - (Required) This is the actual command to execute. This may be a
//               file or a folder. If the full path is not given then the file
//               is searched for in the known system paths (including ./).
// Parameters - (Optional) These are the arguments to pass onto the command line
//               of the command item (Only works with applications).
//
// System Keywords in the command string are replaced with their true values.
//  For Example %ProgramFiles% will be replaced by the actual file path to the
//  Program Files directory on the device. Only keywords defined by the system
//  in the registry key "System\\Explorer\\Shell Folders" will be accepted.
//
// Custom Keywords are defined using the keywords.txt file in the same directory
//  as Run. If the command equals a keyword, that keyword command shall be
//  replaced by the value defined for that keyword.


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
		// Error!
		Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_MFC);
		SetLastError(ERROR_NOT_FOUND);
		return 1;
	}

	// Initialize Globals.
	sHere = _T("");

	// Retrieve Command Line Parameters.
	CString sCommand = _T("");
	if (argv != NULL)
	{
		// Skip self declaration, Start at 1.
		for (int x = 1; x < argc; ++x)
		{
			// Skip first loop.
			if (x != 1)
			{
				// Add space between items.
				sCommand += _T(" ");
			}

			// Validate Pointer.
			if (argv[x] != NULL)
			{
				// Add item.
				sCommand += argv[x];
			}
		}
	}

	Prompt(PT_INFO, _T("Debug"), sCommand);

	// Keywords Map.
	CMapStringToString mKeywords;

	// Parse Internal Keywords.
	ParseInternalKeywords(&mKeywords);

	// Parse External Keywords.
	// Note, external keywords will overwrite same internal keywords.
	ParseExternalKeywords(&mKeywords);

	// System Folders Map.
	CMapStringToString mShellFolders;

	// Retrieve a list of known shell folders.
	GetShellFolders(&mShellFolders);

	// Environment Variables Map.
	CMapStringToString mEnvironment;

	// Generate the Environment.


	// Command line arguments array.
	CArray<CString, CString> aArgs;

	// Options
	CString sVerb = _T("");
	int     nShow = -1;

	// Setup initial prompt.
	bool bPrompt = false;
	if (sCommand.IsEmpty())
	{
		bPrompt = true;
	}

	// First argument.
	CString sFirst;

	// Loop until valid command received.
	bool bGood = false;
	while(!bGood)
	{
		// Show Dialog if no command line received.
		if (bPrompt)
		{
			int nReturn = PromptForCommand(sCommand);
			if (nReturn != 0)
			{
				return nReturn;
			}
		}
		bPrompt = true;

		// Validate input string.
		if (sCommand.IsEmpty())
		{
			Prompt(PT_INFO, IDS_APP_TITLE, IDS_ERROR_EMPTY);
			continue;
		}

		// Replace all shell path keywords in the command string.
		// Shell path kewords are delimited with percent symbols (%keyword%).
		// Quotes added if required and not present in command.
		ReplaceSystemKeywords(&mShellFolders, sCommand);

		// Clean.
		aArgs.RemoveAll();

		// Parse the Command line arguments.
		if (!ParseCommandString(sCommand, &aArgs))
		{
			// Failed to parse the command string.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_PARSE, sCommand);
			continue;
		}

		// Parse Options.
		if (aArgs.GetCount() > 1)
		{
			// Parse Verb from command if any.
			if (ParseVerbOption(aArgs.GetAt(0), sVerb))
			{
				// Remove option from command.
				aArgs.RemoveAt(0);
			}

			// Parse Show option from command if any.
			if (ParseShowOption(aArgs.GetAt(0), nShow))
			{
				// Remove option from command.
				aArgs.RemoveAt(0);
			}

			// DEBUG Validation.
			#ifdef SANDBOX
			if (!sVerb.IsEmpty())
			{
				CString sDebug;
				sDebug.Format(_T("Main\n\nVerb: %s\nShow: %n"), sVerb, nShow);
				Prompt(PT_DEFAULT, _T("Debug"), sDebug);
			}
			#endif

		}

		// Replace all relative paths in the command arguments.
		// .  -> C:\Program Files\Company
		// .. -> C:\Program Files
		ReplaceRelativePaths(&aArgs);

		// Check ONLY first argument against keywords.
		CString sValue = _T("");
		if (mKeywords.Lookup(aArgs.GetAt(0).MakeLower(), sValue) == TRUE)
		{
			// Keyword found, replace keywords.
			ReplaceSystemKeywords(&mShellFolders, sValue);
			ReplaceRelativePaths(sValue);

			// Replace the keyword with its data value.
			aArgs.SetAt(0, sValue);
		}

		// Check first argument for reserved characters (exception backslash).
		sFirst = aArgs.GetAt(0);
		if (!ValidateString(sFirst))
		{
			// Argument one contains invalid characters.
			//DisplayMessageFormat(0, IDS_ERROR_INVALID_CHAR, sFirst);
			if (Prompt(PT_QUESTION, IDS_APP_TITLE, IDS_WARNING_INVALID_CHAR, sFirst) != IDYES)
			{
				continue;
			}
		}

		// TODO Verify that commands with multiple/duplicate backslashes are
		//  correctly handled. C:\\Temp = C:\Temp
		// Remove Duplicate backslashes.
		// UPDATE Their appears to be no problems with multiple/duplicate
		//  backslashes. However, to prevent any future issues, the extra
		//  backslashes will be removed. Yet, leave this note in the event
		//  that the removal has caused an issue.
		// UPDATE 2 Actually, do not replace/correct the strings since it
		//  would require an assumption that the user is wrong.

		// TODO Generate a PATH array with order of importance for the folders.
		// If the first argument does not exist inherently,
		//  check for its existance in the Shell Folders.
		if (!SearchSystemPaths(&mShellFolders, sFirst))
		{
			// TODO Only search the specified PATHEXT extensions.
			// Check for existance of command item missing its extension.
			// Search for item starting with sFirst and use first found match.
			if (!SearchSystemPaths(&mShellFolders, sFirst, true))
			{
				// Argument one, not found!
				Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_NOTFOUND, sFirst);
				continue;
			}

			// DEBUG Validation.
			#ifdef SANDBOX
				Prompt(PT_DEFAULT, _T("Run"), sFirst);
			#endif
		}
		aArgs.SetAt(0, sFirst); // Keep the path.

		// The command passed all tests!
		bGood = true;
	}

	// DEBUG Validation.
	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Debug"), _T("Main\n\nAll Tests Passed!"));
	#endif

	// TODO Determine file type (document, application, etc).
	// If directory, open the path with fexplore.exe
	if (FileExists(sFirst, true, 1))
	{
		// NOTE!
		// FExplore Command Line
		// fexplore.exe \My Documents
		// DO NOT QUOTE PATHS WITH SPACES!
		// Trailing backslash is optional.

		// Argument is a Directory.
		CString sExplore(APP_WINDOWS_EXPLORER);
		if (SearchSystemPaths(&mShellFolders, sExplore))
		{
			// Insert explorer as the caller.
			aArgs.InsertAt(0, sExplore);
			sFirst = sExplore;
		}
		else
		{
			// Error. Unable to find explorer.
			Prompt(PT_WARNING, IDS_APP_TITLE, IDS_ERROR_NO_EXPLORER, sExplore);
		}
	}

	// Setup Shell Execute Information Structure.
	SHELLEXECUTEINFO sei = {0};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask  = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd   = NULL;
	sei.lpVerb = NULL;
	if (!sVerb.IsEmpty())
	{
		sei.lpVerb = sVerb;
	}
	CString sFile = aArgs.GetAt(0);
	sei.lpFile = sFile;
	CString sParameters = FormatParameters(&aArgs);
	sei.lpParameters = sParameters;
	sei.lpDirectory = NULL;  // Not Supported in Mobile.
	sei.nShow = nShow; // Ignored except for applications.
	sei.hInstApp = NULL; // (Out)
	sei.hProcess = NULL; // (Out)

	// DEBUG Validation.
	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Debug"), sFile);
		Prompt(PT_DEFAULT, _T("Debug"), sParameters);
	#endif

	// Attempt Shell Execute.
	if (ShellExecuteEx(&sei) != 0)
	{
		//HANDLE hProcess = sei.hProcess;
		// DEBUG Validation.
		#ifdef SANDBOX
			Prompt(PT_DEFAULT, _T("Debug"), _T("Main\n\nShell Execute Success!"));
		#endif

		// Success.
		return 0;
	}

	// Failed.
	// DEBUG Validation.
	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Debug"), _T("Main\n\nShell Execute Failed!"));
	#endif

	// TODO Handle All ShellExecute errors.
	bool bContinue(true);
	switch ((int)sei.hInstApp) // Ignore hInstApp unless failure.
	{
		case SE_ERR_FNF: // File not found. (Fallthrough)
		case SE_ERR_PNF: // Path not found.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_NOTFOUND, aArgs.GetAt(0));
			bContinue = false;
			break;
		case SE_ERR_ACCESSDENIED: // Access denied.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_ACCESS_DENIED, aArgs.GetAt(0));
			bContinue = false;
			break;
		case SE_ERR_OOM: // Out of memory.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_OUT_OF_MEMORY, aArgs.GetAt(0));
			bContinue = false;
			break;
		case SE_ERR_DLLNOTFOUND: // Dynamic-link library not found.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_DLL_NOT_FOUND, aArgs.GetAt(0));
			bContinue = false;
			break;
		case SE_ERR_SHARE: // Cannot share an open file.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_FILE_LOCKED, aArgs.GetAt(0));
			bContinue = false;
			break;
		case SE_ERR_ASSOCINCOMPLETE: // File associate information not complete.
			#ifdef SANDBOX
				Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_INCOMPLETE_ASSOC, aArgs.GetAt(0));
			#endif
			break;
		case SE_ERR_NOASSOC: // File association not available.
			#ifdef SANDBOX
				Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_NO_ASSOCICATION, aArgs.GetAt(0));
			#endif
			break;

		// Dynamic Data Exchange
		case SE_ERR_DDETIMEOUT: // DDE operation timed out.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_DDE_TIME_OUT);
			bContinue = false;
			break;
		case SE_ERR_DDEFAIL: // DDE operation failed.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_DDE_FAILURE);
			bContinue = false;
			break;
		case SE_ERR_DDEBUSY: // DDE operation is busy.
			Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_DDE_BUSY);
			bContinue = false;
			break;

		default: // Unknown Error.
			#ifdef SANDBOX
				Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_UNKNOWN);
			#endif
			break;
	}

	if (!bContinue)
	{
		return 1;
	}

	// TODO Parse the file extension if any.
	//CString sExt =

	// TODO Upon certain failures of ShellExecute attempt Manual execution.
	// Parse type of file based upon extension and retrieve command by verb or
	// default command if no verb or verb not found.
	// (If no extension, assume directory) or open the file with fexplorer.
	//ParseExtensionCommand(sFirst, &aArgs);

	// TODO Open application files with command line arguments.
	//CreateProcess(

	// DO NOT use system(command). 
	// UnSafe! It grants my priveleges to the command.

	// Cleanup.
	//sFile.ReleaseBuffer();
	//sParameters.ReleaseBuffer();

	// Done.
	return 0;
}


//// Combine all the elements of a string array into one string.
// Note: Array elements are space delimited in the combined string.
// paStrings - (IN) Array of strings to combine.
// nStart    - (IN) Zero based index of the array where to start the concatination. (Default 0 = beginning)
// nCount    - (IN) The number of elements to concatinate. (Default 0 = end of array).
// bQuote    - (IN) Add quotes to elements containing spaces. (Default true)
// Returns a CString of all the array elements combined, null string if empty.
CString ConcatArray(CArray<CString, CString>* paStrings, int nStart /* = 0*/, int nCount /* = -1 */, bool bQuote /*= true*/)
{
	// Setup.
	SetLastError(ERROR_SUCCESS);

	// Validate parameters.
	if (paStrings == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return _T("");
	}
	if (nStart < 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		nStart = 0;
	}
	if (nCount < 0)
	{
		nCount = paStrings->GetCount();
	}
	if (nCount+nStart > paStrings->GetCount()-1)
	{
		// Do not allow overflowing of bounds.
		SetLastError(ERROR_STACK_OVERFLOW);
		nCount = paStrings->GetCount();
	}

	// Combine strings.
	CString sTemp = _T("");
	CString sCombined = _T("");
	for (int i = nStart; i < nCount; ++i)
	{
		// Add spacer.
		if (i != nStart) { sCombined += _T(" "); }

		sTemp = paStrings->GetAt(i);

		// Add quotes if needed.
		if (bQuote && HasWhitespace(sTemp) && !IsWrapped(sTemp))
		{
			sTemp = SymbolWrap(sTemp);
		}

		// Concatinate.
		sCombined += sTemp;
	}
	return sCombined;
}


//// Detect the Text Encoding for a byte array.
// pData   - (IN) The byte array to analyze.
// nOffset - (OUT) The offset in bits caused by the byte order mark (BOM).
// Returns the base number of bits per code point.
TFormat DetectTextEncoding(byte* pData)
{
	// Validate Parameter.
	if (pData == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return TF_UNKNOWN;
	}
	SetLastError(ERROR_SUCCESS);

	// Byte Order Mark      ASCIIEx  Type    Endian
	// -------------------  -------  ------  ------
	// None                          ASCII
	// 0xEF 0xBB 0xBF       ∩ ╕ ┐    UTF-8
	// 0xFE 0xFF            ■        UTF-16  Big
	// 0xFF 0xFE              ■      UTF-16  Little
	// 0x00 0x00 0xFE 0xFF      ■    UTF-32  Big
	// 0xFF 0xFE 0x00 0x00    ■      UTF-32  Little
	// 0x2B 0x2F 0x76 0x38  + / v 8  UTF-7 v8
	// 0x2B 0x2F 0x76 0x39  + / v 9  UTF-7 v9
	// 0x2B 0x2F 0x76 0x2B  + / v +  UTF-7 v+
	// 0x2B 0x2F 0x76 0x2F  + / v /  UTF-7 v/
	// 0x2B 0x2F 0x76 0x3B  + / v ;  UTF-7 v;
	// 0xF7 0x64 0x4C       ≈ d L    UTF-1
	// 0xDD 0x73 0x66 0x73    s f s  UTF-EBCDIC
	// 0x0E 0xFE 0xFF         ■      SCSU
	// 0xFB 0xEE 0x28       √ ε (    BOCU-1
	// 0x84 0x31 0x95 0x33  ä 1 ò 3  GB-18030

	// Detect Type.
	DWORD nBOM = 0;
	for (UINT x = 0; x < 4; ++x)
	{
		// Get Next BOM byte.
		if (pData[x] != NULL)
		{
			nBOM = nBOM << 8;
			nBOM += pData[x];
		}

		// Skip first loop.
		if (x == 0) { continue; }

		// Match Type.
		switch(nBOM)
		{
			case 0xEFBBBF: // UTF-8
				//nOffset = 24;
				//return 8; // 8, 16, 24, 32, 40, or 48
				return TF_UTF_8;
				break;
			case 0xFEFF: // Default Big Endian
				if (x == 1) // UTF-16 Big
				{
					//nOffset = 16;
					//return 16; // 16 or 32
					return TF_UTF_16BE;
				}
				else if (x == 3) // UTF-32 Big
				{
					//nOffset = 32;
					//return 32;
					return TF_UTF_32BE;
				}
				break;
			case 0xFFFE: // UTF-16 Little
				//nOffset = 16;
				//return -16; // -16 or -32
				return TF_UTF_16LE;
				break;
			case 0xFFFE0000: // UTF-32 Little
				//nOffset = 32;
				//return -32;
				return TF_UTF_32LE;
				break;
			//case 0x2B2F76: // UTF-7 without flex byte.
			case 0x2B2F7638: // UTF-7 v8
			case 0x2B2F7639: // UTF-7 v9
			case 0x2B2F762B: // UTF-7 v+
			case 0x2B2F762F: // UTF-7 v/
			case 0x2B2F763B: // UTF-7 v;
				//nOffset = 30;
				//return 7;
				return TF_UTF_7;
				break;
			case 0xF7644C: // UTF-1
				//nOffset = 24;
				//return 8; // 8, 16, 24, or 40
				return TF_UTF_1;
				break;
			case 0xDD736673: // UTF-EBCDIC
				//nOffset = 32;
				//return 8;
				return TF_UTF_EBCDIC;
				break;
			case 0x0EFEFF: // SCSU
				//nOffset = 24;
				//return 8;
				return TF_SCSU;
				break;
			case 0xFBEE28: // BOCU-1
				//nOffset = 24;
				//return 8;
				return TF_BOCU_1;
				break;
			case 0x84319533: // GB-18030
				//nOffset = 32;
				//return 8;
				return TF_GB_18030;
				break;
			default:
				// Do Nothing Until All Loops Complete.
				break;
		}
	}

	// No BOM Identified.
	//nOffset = 0;
	//return 8;
	//return TF_UTF_8;
	//return TF_ASCII;
	return TF_UNKNOWN;
}


//// Validate the existance of a file (file or directory).
// sFile        - (IN) String of the entire path for the file to validate.
// bIsDirectory - (IN) True, only validate directories; False, files and folders.
// bMatch       - (IN) Return first match to the file path.
// Returns true on success and false on failure. See GetLastError for details.
bool FileExists(CString& sFilepath, bool bIsDirectory /* = false*/, bool bMatch /* = false*/)
{
	// Validate parameters.
	if (sFilepath.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Remove wrap (if any).
	int nWraps = GetWrapCount(sFilepath);
	if (nWraps > 0)
	{
		//sFilepath = SymbolWrap(sFilepath, _T('\"'), true);
		sFilepath = RemoveWrap(sFilepath);
	}

	// Remove trailing backslash.
	if (sFilepath.ReverseFind(_T('\\'))+1 == sFilepath.GetLength())
	{
		sFilepath = sFilepath.Left(sFilepath.GetLength() -1);
	}

	// Set to Mobile root if empty.
	if (sFilepath.IsEmpty())
	{
		sFilepath = _T("\\"); // Windows CE/Mobile Root Path
	}

	// Copy Filepath for Use.
	CString sPath(sFilepath);

	// TODO Consider if it is valid or worthwhile to have a directory and files
	//  verified at the same time.  Not only one or the other at a time.

	// Find First Complete Match.
	if (bMatch)
	{
		// Append Wild Character for 'Any'.
		if (bIsDirectory)
		{
			sPath.AppendChar(_T('*'));
		}
		else
		{
			// Only Match complete filenames.
			// If the period is left out, then the first filename beginning with
			//  the string will be selected; instead of a complete name match.
			sPath.Append(_T(".*"));
		}
	}

	// Validate Path.
	WIN32_FIND_DATA Data;
	HANDLE hFile = FindFirstFile(sPath, &Data);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	do
	{
		//TCHAR* psTemp = new TCHAR[12];
		//_stprintf(psTemp, _T("%u"), Data.dwFileAttributes);
		//Prompt(PT_DEFAULT, _T("Debug"), psTemp);
		//delete[] psTemp;
		if (!bIsDirectory ||
			(bIsDirectory && Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// Found!

			// Rebuild Filepath.
			sFilepath = sPath.Left(sPath.ReverseFind(_T('\\'))+1) + Data.cFileName;
			if (nWraps > 0)
			{
				sFilepath = SymbolWrap(sFilepath, _T('\"'), nWraps);
			}

			// Cleanup.
			FindClose(hFile);
			return true;
		}
	} while (FindNextFile(hFile, &Data) == TRUE);

	// Did not find.
	FindClose(hFile);
	return false;
}


//// Validate the existance of a filepath (file and or directory).
// sFilepath    - (IN)(OUT) String of the filepath to validate. Exact filepath
//                 is set upon success.
// bIsDirectory - (OUT) Flag identifying the filepath type.
// nIgnore      - (IN) Ignore files = 1 or ignore directories = 2.
// Returns true on success and false on failure.
bool FileExists(CString& sFilepath, bool& bIsDirectory, int nIgnore /*= 0*/)
{
	// Validate parameters.
	if (sFilepath.IsEmpty())
	{
		return false;
	}

	// Remove wrap (if any).
	CString sPath = RemoveWrap(sFilepath);

	// Remove trailing backslash(es).
	while (sPath.GetAt(sPath.GetLength()-1) == _T('\\'))
	{
		sPath = sPath.Left(sPath.GetLength() -1);
	}

	// Validate Path.
	WIN32_FIND_DATA Data;
	HANDLE hFile = FindFirstFile(sPath, &Data);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	do // Found!
	{
		// Directory or File.
		bIsDirectory = false;
		if (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			bIsDirectory = true;
		}

		// Check Options.
		if ((!bIsDirectory && nIgnore == 1) ||
			 (bIsDirectory && nIgnore == 2))
		{
			// Wrong Type.
			continue;
		}

		// Rebuild Filepath (in case of Wildchars).
		sFilepath = sPath.Left(sPath.ReverseFind(_T('\\'))+1) + Data.cFileName;

		// Wrap Once (if needed).
		if (HasWhitespace(sFilepath))
		{
			sFilepath = SymbolWrap(sFilepath, _T('\"'), 1, true);
		}

		// Cleanup.
		FindClose(hFile);
		return true;
	} while (FindNextFile(hFile, &Data) == TRUE);

	// Did not find.
	FindClose(hFile);
	return false;
}


//// Not needed since CString has built in format method, but it is safer.
//// Format a string using the specified parameters.
// sString  - (OUT) The string to be formatted.
// psFormat - (IN) Format string using %n as argument placeholders.
// ...      - (IN) Arguments to replace %n(s) in message.
// Returns true upon successful format, false on failure. See GetLastError for details.
bool FormatIntoString(CString& sString, LPCTSTR psFormat, ...)
{
	// Validate parameter.
	if (psFormat == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Setup Argument List.
	va_list argList;
	va_start(argList, psFormat);

	// Format the message.
	LPTSTR psTemp;
	if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | 
					  FORMAT_MESSAGE_ALLOCATE_BUFFER,
					  psFormat,
					  0, /* Error ID */
					  0, /* Default language */
					  (LPWSTR)&psTemp,
					  0, /* Minimum allocation size */
					  &argList) == 0 || psTemp == NULL)
	{
		// See GetLastError().
		return false;
	}

	// Copy the message.
	sString = CString(psTemp);

	// Cleanup.
	LocalFree(psTemp);
	va_end(argList);

	// Success.
	return true;
}


//// Create a formatted string of the parameters.
// paArgs - (IN) String array of parameters. First element skipped.
// Returns the formatted parameters string.
CString FormatParameters(CArray<CString, CString>* paArgs)
{
	// Validate parameters.
	if (paArgs == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return _T("");
	}

	// Rules.
	bool bQuote = true; // Wrap the Parameter in quotes if it contains spaces.

	// Check for Special Command Line Rules.
	CString sProgram = paArgs->GetAt(0);
	sProgram = sProgram.Mid(sProgram.ReverseFind(_T('\\')) +1).MakeLower();
	if (sProgram.CompareNoCase(_T("fexplore.exe")) == 0)
	{
		bQuote = false;
	}

	// Apply Triple Quotation Wrap to parameters with whitespace and or quotations.
	CString sTemp = _T("");
	for (int i = 1; i < paArgs->GetCount(); ++i)
	{
		sTemp = paArgs->GetAt(i);
		if (bQuote && HasWhitespace(sTemp))
		{
			// Triple Quotes are required for ShellExecuteEx Parameters when that
			//  Parameter contains quotes (per MSDN documentation). See Remarks.
			// http://msdn.microsoft.com/en-us/library/windows/desktop/bb759784(v=vs.85).aspx
			paArgs->SetAt(i, SymbolWrap(sTemp, _T('\"'), 3, true));
			//paArgs->SetAt(i, TripleWrap(sTemp));
		}
		else
		{
			paArgs->SetAt(i, RemoveWrap(sTemp, _T('\"')));
		}
	}

	// Concatinate Parameters.
	return ConcatArray(paArgs, 1, -1, bQuote);
}


bool GenerateEnvironment(CMapStringToString* pmEnvironment)
{
	// Validate Parameters.
	if (pmEnvironment == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	CString sName;
	GetKeyValueString(REG_PATH_DEVICE_IDENT, _T("Name"), sName, HKEY_LOCAL_MACHINE);
	pmEnvironment->SetAt(_T("ComputerName"), sName);
	//pmEnvironment->SetAt(_T("HomeDrive"), );
	//pmEnvironment->SetAt(_T("HomePath"), );
	//pmEnvironment->SetAt(_T("OS"), );
	//pmEnvironment->SetAt(_T("Path"), );
	//pmEnvironment->SetAt(_T("PathExt"), );
	//pmEnvironment->SetAt(_T("SystemDrive"), );
	//pmEnvironment->SetAt(_T("SystemRoot"), );
	//pmEnvironment->SetAt(_T("Temp"), );
	//pmEnvironment->SetAt(_T("Tmp"), );
	//pmEnvironment->SetAt(_T("WinDir"), );
	return true;
}


//// Retrieve the current application's directory path.
// sDirectory - (OUT) CString to store the directory path (includes trailing backslash).
// sArgument  - (IN)  Optional, manual entry for first command line argument.
// Returns true on success and false on failure. See GetLastError for details.
bool GetAppDirectory(CString& sDirectory, LPTSTR sArgument /* = NULL */)
{
	// Check if already identified.
	if (!sHere.IsEmpty())
	{
		sDirectory = sHere;
		return true;
	}
	sDirectory.Empty();

	// Retrieve Application Path.
	int nSize = MAX_PATH;
	LPTSTR psAppPath = (LPTSTR)malloc(sizeof(TCHAR) * nSize);
	while (GetModuleFileName(NULL, psAppPath, sizeof(TCHAR) * nSize) == sizeof(TCHAR) * nSize &&
		   GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		// Allocate more memory since there was not enough.
		nSize += nSize;
		psAppPath = (LPTSTR)realloc((PVOID)psAppPath, sizeof(TCHAR) * nSize);
	}

	// Validate String.
	if (psAppPath != NULL && _tcslen(psAppPath) > 0)
	{
		// Copy and truncate application name.
		sDirectory = psAppPath;
		sDirectory = sDirectory.Left(sDirectory.ReverseFind(_T('\\')) +1);
	}

	// Cleanup.
	free(psAppPath);

	if (FileExists(sDirectory, true))
	{
		// DEBUG Validation.
		#ifdef SANDBOX
			Prompt(PT_DEFAULT, _T("Debug"), _T("GetAppDirectory\n\n") + sDirectory);
		#endif

		// Success.
		sHere = sDirectory;
		return true;
	}

	// Attempt to retrieve path from first command line argument.
	if (sArgument != NULL)
	{
		sDirectory = sArgument;
	}
	else
	{
		// Get the command line.
		sDirectory = GetCommandLine();
	}

	// Parse the command line.
	CArray<CString, CString> aArgs;
	if (ParseCommandString(sDirectory, &aArgs))
	{
		sDirectory = aArgs.GetAt(0);
	}

	// Studies:
	// 0123456789012345678901234567890123456789012345678901234
	// "C:\Program Files\Company\Application\app.exe" Argument
	// 012345678901234567890123456789012345
	// C:\Temp\Application\app.exe Argument

	// Remove quotes (if any).
	if (IsWrapped(sDirectory))
	{
		//sDirectory = SymbolWrap(sDirectory, _T('\"'), true);
		sDirectory = RemoveWrap(sDirectory);
	}

	// Remove app.exe, but leave the trailing backslash.
	sDirectory = sDirectory.Left(sDirectory.ReverseFind(_T('\\')) +1);
	
	if (FileExists(sDirectory, true))
	{
		// DEBUG Validation.
		#ifdef SANDBOX
			Prompt(PT_DEFAULT, _T("Debug"), _T("GetAppDirectory\n\n") + sDirectory);
		#endif

		// Success.
		sHere = sDirectory;
		return true;
	}

	// Done.
	sHere.Empty();
	return false;
}


//// Retrieve the directory name at the specified offset in a path string.
// sPath      - (IN) The path string.
// nOffset    - (IN)(OUT) Character offset where the directory is in the path.
// sDirectory - (OUT) String of the directory's name.
// Returns true upon success, false upon failure. See GetLastError for details.
bool GetDirectoryAt(CString sPath, int& nOffset, CString& sDirectory)
{
	// Clear string parameter.
	sDirectory.Empty();

	SetLastError(ERROR_SUCCESS);

	// Validate parameters.
	if (sPath.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	if (nOffset >= sPath.GetLength())
	{
		SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	if (nOffset < 0)
	{
		nOffset = 0;
		SetLastError(ERROR_INVALID_PARAMETER);
	}

	// Find start of the directory in the path.
	int nLeft = sPath.Left(nOffset+1).ReverseFind(_T('\\'));
	// Not found, -1 result is handled below with plus 1.
	nLeft += 1; // Exclude backslash.

	// Find the end of the directory in the path.
	int nRight = sPath.Find(_T('\\'), nOffset+1);
	// Set to length of string if -1, not found.
	// Out of bounds prevented by minus 1 below.
	if (nRight == -1) { nRight = sPath.GetLength(); }

	// Save the directory name.
	sDirectory = sPath.Mid(nLeft, nRight - nLeft);

	// Update the offset.
	nOffset = nRight;

	// Check if two backslashes next to each other.
	if (sDirectory.IsEmpty())
	{
		// Skip duplicate backslash and try again.
		return GetDirectoryAt(sPath, nOffset, sDirectory);
	}

	// Success.
	return true;
}


//// Retireve the associated command string for the extension.
// sExt     - (IN)  String of just the extension including the preceeding period.
// sCommand - (OUT) Command string associated with the extension.
// Returns true on success and false on failure. See GetLastError for details.
bool GetExtensionCommand(CString sExt, CString& sCommand, CString sVerb /*= _T("")*/)
{
	// Validate parameters.
	if (sExt.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Steps to retrieve command string from the registry by file extension.
	// 1. Open HKEY_CLASSES_ROOT\.ext\Default = <keyname>
	// 2. Open HKEY_CLASSES_ROOT\<keyname>\Shell\Open\Command = <format message>
	// 3. Use <fomat message> in FormatMessage() with the command string as the variable.
	// If Step 2 does not exist:
	// 2. Open HKEY_CLASSES_ROOT\<keyname>\CLSID = <CLSID#>
	// 3. Open HKEY_CLASSES_ROOT\CLSID\<CLSID#>\ProgID = <keyname2>
	// 4. Open HKEY_CLASSES_ROOT\<keyname2>\Shell\Open\Command = <format message>
	// 5. Use <fomat message> in FormatMessage() with the command string as the variable.
	// Else fail.
	// Open the Shell Folders registry key.

	// Check for command string.
	if (GetVerbCommand(sExt, sVerb, sCommand))
	{
		// Found Command.
		return true;
	}
	
	// Not Found, check for default.

	CString sCLSID;
	CString sDefault;
	CString sProgram = sExt;

	// Get the default value.
	if (!GetKeyValueString(sProgram, _T("Default"), sDefault))
	{
		return false;
	}

	// While the default is not the source.
	while (sDefault.Compare(sProgram) != 0)
	{
		// Check for command string.
		if(GetKeyValueString(sDefault + _T("\\Shell\\Open\\Command"), _T("Default"), sCommand))
		{
			// Found Command.
			return true;
		}

		// Not Found, check for CLSID.

		// Check for CLSID association.
		if(!GetKeyValueString(sDefault, _T("CLSID"), sCLSID))
		{
			return false;
		}

		// Check CLSID base for program key.
		if(!GetKeyValueString(_T("CLSID\\") + sCLSID, _T("ProgID"), sProgram))
		{
			return false;
		}

		// Check for command string.
		if (GetVerbCommand(sProgram, sVerb, sCommand))
		{
			// Found Command.
			return true;
		}

		// Not Found, check for another default.
		if (!GetKeyValueString(sProgram, _T("Default"), sDefault))
		{
			return false;
		}
	}

	// Failed.
	return false;
}


//// Retrieves the string data from a Registry key's value.
// sKey   - (IN)  Name of the key to open.
// sValue - (IN)  Name of the value to retrieve.
// sData  - (OUT) The string retrieved from the registry key.
// hRoot  - (IN)  Default classes root. Root Registry branch to use.
// Returns true on success and false on failure. See GetLastError for details.
bool GetKeyValueString(CString sKey, CString sValue, CString& sData, HKEY hRoot /* = HKEY_CLASSES_ROOT */)
{
	// Validate parameters.
	if (sKey.IsEmpty()   ||
		sValue.IsEmpty() ||
		(hRoot != HKEY_LOCAL_MACHINE &&
		 hRoot != HKEY_CLASSES_ROOT  &&
		 hRoot != HKEY_CURRENT_USER  &&
		 hRoot != HKEY_USERS) )
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Open Open Key
	HKEY hKey;
	if ( ERROR_SUCCESS != RegOpenKeyEx(hRoot,
									   sKey,
									   NULL,
									   KEY_QUERY_VALUE,
									   &hKey) )
	{
		return false;
	}

	// Loop Variables.
	LONG   nRetVal = 0;
	DWORD  nSize   = (sizeof(TCHAR) * MAX_PATH);
	LPBYTE pBuffer = (LPBYTE)malloc(nSize); // 16,383W or 260A Max reg value.

	// Get the subkey's Command value.
	while ( ERROR_MORE_DATA == (nRetVal = RegQueryValueEx(hKey,
														  sValue,
														  NULL,
														  NULL,
														  pBuffer,
														  &nSize)) )
	{
		// Increase Buffer to returned Size.
		pBuffer = (LPBYTE)realloc(pBuffer, nSize);
	}

	// Check results.
	if (pBuffer == NULL || nRetVal != ERROR_SUCCESS)
	{
		// Unable to retieve value.
		free(pBuffer);
		RegCloseKey(hKey);
		return false;
	}

	// Copy String.
	sData = CString(pBuffer);

	// Cleanup.
	free(pBuffer);
	RegCloseKey(hKey);

	// Success.
	return true;
}


//// Retrieve the shell folder paths definied in the registry.
// paFolders - (OUT) Array of keys and their path values.
// Returns true if folders successfully retrieved, false if not.
bool GetShellFolders(CMapStringToString* pmShellFolders)
{
	// Validate parameter.
	if (pmShellFolders == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Add Relative Path .\ Directory.
	CString sLocal;
	GetAppDirectory(sLocal);
	if (!sLocal.IsEmpty())
	{
		// Remove trailing backslash.
		if (sLocal.GetAt(sLocal.GetLength()-1) == _T('\\'))
		{
			sLocal = sLocal.Left(sLocal.GetLength() -1);
		}

		// Add to the array.
		pmShellFolders->SetAt(_T("."), sLocal);
	}

	// Open the Shell Folders registry key.
	HKEY hKey;
	if ( ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
									   REG_PATH_SHELL_FOLDERS,
									   NULL,
									   KEY_QUERY_VALUE,
									   &hKey) )
	{
		Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_REGISTRY);
		Prompt(PT_ERROR, IDS_APP_TITLE, IDS_ERROR_REGISTRY_ANSWER);
		return false;
	}

	//// Retrieve all Shell Folders Registry Key Values ////
	// Do not use this method on HKEY_PERFORMANCE due to
	//  fluctuating value sizes.
	DWORD  nKey    = 0;
	DWORD  nSize   = (sizeof(TCHAR) * MAX_PATH);
	DWORD  nLength = (sizeof(WCHAR) * MAX_PATH);
	DWORD  nName   = 0;
	LONG   nRetVal = 0;
	LONG   nReturn = ERROR_SUCCESS;
	LPBYTE pBuffer = (LPBYTE)malloc(nSize); // 16,383W or 260A Max reg value.
	LPWSTR psName  = (LPWSTR)malloc(nLength); // 255 Max reg key name.
	
	// Loop through each child key.
	while (nReturn != ERROR_NO_MORE_ITEMS)
	{
		// Reset key name size.
		nName = nLength;

		// Retieve the subkey's name.
		//  Use RegQueryValueEx to retrieve its value becaues it is buffer safer.
		nReturn = RegEnumValue(hKey, nKey++, psName, &nName, NULL, NULL, NULL, NULL);

		// Check results.
		if (psName == NULL || nName == 0 || nReturn != ERROR_SUCCESS)
		{
			// Skip to next key.
			continue;
		}

		// Get the subkey's value.
		while ( ERROR_MORE_DATA == (nRetVal = RegQueryValueEx(hKey,
															  (LPCWSTR)psName,
															  NULL,
															  NULL,
															  pBuffer,
															  &nSize)) )
		{
			// Increase Buffer to returned Size.
			pBuffer = (LPBYTE)realloc(pBuffer, nSize);
		}

		// Check results.
		if (pBuffer == NULL || nRetVal != ERROR_SUCCESS)
		{
			// Skip to next key.
			continue;
		}

		// Add Key and Value to the list.
		CString sTemp = CString(psName).MakeLower();
		pmShellFolders->SetAt(sTemp, CString((LPTSTR)pBuffer));
		if (HasWhitespace(sTemp))
		{
			// Add Whitespace free version.
			sTemp = RemoveWhitespace(sTemp);
			pmShellFolders->SetAt(sTemp, CString((LPTSTR)pBuffer));
		}
	}

	// Close Key.
	RegCloseKey(hKey);
	
	// Free the Buffers
	free(psName);
	free(pBuffer);

	// DO NOT DO HERE!
	// Validate Folders.
	//  Even though the path does not exist they should be left in for
	//   compliance with commands that use the path keywords as second
	//   and third arguments.
	//CMapStringToString::CPair* p = paFolders->PGetFirstAssoc();
	//while (p != NULL)
	//{
	//	if (!FileExists(p->value, true))
	//	{
	//		// Remove the invalid path.
	//		pmShellFolders->RemoveKey(p->key);

	//		// Reset pointer due to the deletion of an item.
	//		p = pmShellFolders->PGetFirstAssoc();
	//	}
	//	p = pmShellFolders->PGetNextAssoc(p);
	//}

	// TODO Pull in all paths retrieved through SHGetPathFromIDList and SHGetSpecialFolderLocation

	// User Variables
	// HKEY_CURRENT_USER\Environment
	// System Variables
	// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment
	// App Path
	// HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\App Paths
	// http://commandwindows.com/runline.htm

	// DEBUG Validation of array contents.
	#ifdef SANDBOX
		CMapStringToString::CPair* q = pmShellFolders->PGetFirstAssoc();
		while (q != NULL)
		{
			Prompt(PT_DEFAULT, _T("Debug"), _T("GetShellFolders\n\n") + q->key + _T("=") + q->value);
			q = pmShellFolders->PGetNextAssoc(q);
		}
	#endif

	// Return the directory
	return true;
}


//// Retrieve a string resource from the string table.
// nResourceID - (IN) Resource id number to access.
// Returns the loaded string or a blank string upon failure.
CString GetTableString(UINT nResourceID)
{
	// Setup.
	SetLastError(ERROR_SUCCESS);
	CString sString;

	// Attempt to Load.
	if (sString.LoadString(nResourceID) == FALSE)
	{
		// Failed.
		SetLastError(ERROR_NOT_FOUND);
		return _T("");
	}
	return sString;
}


//// Retrieve the command string for the extension verbs.
// sProgram  - (IN) CLSID to check for a command string.
// sVerb     - (IN)(OUT) Verb to check for first, if not found defaults checked.
//               Verb is modifided to equal verb command string found within.
// sCommand  - (OUT) The command string retrieved from the verb.
// bOnlyVerb - (IN) Only check for the specified verb.
bool GetVerbCommand(CString sProgram, CString& sVerb, CString& sCommand, bool bOnlyVerb /*= false*/)
{
	// Validate parameters.
	if (sProgram.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Check for command string.
	if(!sVerb.IsEmpty() &&
		GetKeyValueString(sProgram + _T("\\Shell\\") + sVerb + _T("\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		return true;
	}

	// Perform default checks upon failure?
	if (bOnlyVerb)
	{
		// No.
		return false;
	}

	// Specified verb not found, check in terms of defaults.
	if(GetKeyValueString(sProgram + _T("\\Shell\\Open\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		sVerb = _T("Open");
		return true;
	}
	if(GetKeyValueString(sProgram + _T("\\Shell\\Edit\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		sVerb = _T("Edit");
		return true;
	}
	if(GetKeyValueString(sProgram + _T("\\Shell\\Print\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		sVerb = _T("Print");
		return true;
	}
	if(GetKeyValueString(sProgram + _T("\\Shell\\Find\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		sVerb = _T("Find");
		return true;
	}
	/* Following verbs are not supported in Windows Mobile. */
	if(GetKeyValueString(sProgram + _T("\\Shell\\Explore\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		sVerb = _T("Explore");
		return true;
	}
	if(GetKeyValueString(sProgram + _T("\\Shell\\Properties\\Command"), _T("Default"), sCommand))
	{
		// Found Command.
		sVerb = _T("Properties");
		return true;
	}

	// Not found.
	return false;
}


//// Identifies the number of symbol wraps around a string.
// sString - (IN) String to count symbol wraps.
// cSymbol - (IN) Symbol to identify. (Default ")
// Returns the number of continuous symbol wraps.
int GetWrapCount(CString sString, TCHAR cSymbol /*= _T('\"')*/)
{
	// Loop.
	int nTimes = 0;
	int nLen = sString.GetLength() -1; // Zero Base.
	for (int i = 0; i < nLen -i; ++i)
	{
		// Symbol Check.
		if (sString.GetAt(i) == cSymbol &&
			sString.GetAt(nLen -i) == cSymbol)
		{
			// Wrap Found.
			++nTimes;
		}
		else
		{
			// Continuous Wrap Broken.
			return nTimes;
		}
	}

	// Done.
	return nTimes;
}


//// Checks if a string contains any whitespace.
// sString - (IN) String to search for whitespace.
// Returns true if whitespace found, false if none found.
bool HasWhitespace(CString sString)
{
	// Search for whitespace.
	if (WhitespaceFind(sString) != -1)
	{
		// Whitespace found.
		return true;
	}

	// No whitespace found.
	return false;
}


//// Check if a string contains only the specified symbol.
// sString - (IN) String to check.
// cSymbol - (IN) Symbol to check for in the string.
// Returns true if only symbols found, otherwise false.
bool IsSymbolOnly(CString sString, TCHAR cSymbol)
{
	// Loop.
	int nLen = sString.GetLength();
	for (int i = 0; i < nLen; ++i)
	{
		if (sString.GetAt(i) != cSymbol)
		{
			return false; // Non Symbol Found.
		}
	}
	return true; // Only Symbols Found.
}


//// Checks if the string is wrapped by the specified symbol.
// sString - (IN) The string to check for symbol wrap.
// cSymbol - (IN) The symbol character. (Default quote)
// nTimes  - (IN) The number/count of wraps to find. (Default -1 = any)
// Return true if wrapped, false if not. See GetLastError for details.
//bool IsWrapped(CString sString, TCHAR cSymbol /*= _T('\"')*/, UINT nTimes /* = 1*/)
//{
//	// Setup.
//	SetLastError(ERROR_SUCCESS);
//
//	// Validate parameters.
//	int nLen = sString.GetLength();
//	if (sString.IsEmpty() || nLen < 2)
//	{
//		SetLastError(ERROR_INVALID_PARAMETER);
//		return false;
//	}
//
//	// Loop through each wrap.
//	UINT nWraps = nTimes;
//	while(nTimes > 0)
//	{
//		// Check for symbol wrap.
//		if (sString.GetAt(nTimes-1) == cSymbol &&
//			sString.GetAt(nLen-nTimes) == cSymbol)
//		{
//			--nWraps; // Found, Subtract.
//		}
//
//		// Decrement.
//		--nTimes;
//	}
//
//	// Verify Results.
//	if (nWraps == nTimes) // 0
//	{
//		// Found wrap(s).
//		return true;
//	}
//
//	// No nTimes wrap found.
//	return false;
//}
bool IsWrapped(CString sString, TCHAR cSymbol /*= _T('\"')*/, int nTimes /*= -1*/)
{
	// Get Wrap Count.
	int nCount = GetWrapCount(sString, cSymbol);

	// Check for Wrap (any times).
	if (nTimes < 0)
	{
		return (nCount > 0);
	}
	
	// Check for specified number of Wraps.
	return (nCount == nTimes);
}


//// Parse the individual arguments of the command string.
// Valid format example: "argument one" two three "number four".
// sString - (IN)  Source command string to parse.
// paArgs  - (OUT) CArray of CStrings containing the individual arguments.
// Returns true on success and false on failure. See GetLastError for details.
bool ParseCommandString(CString sString, CArray<CString, CString>* paArgs)
{
	// Validate Parameters.
	if (sString.IsEmpty() || paArgs == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Parse input setup.
	int  i        = 0;
	int  nStart   = 0;
	bool bInQuote = false;

	// Parse input string.
	for (i = 0; i < sString.GetLength(); ++i)
	{
		switch(sString.GetAt(i))
		{

		// Check for quotation strings.
		case _T('\"'):  // Quotation
			bInQuote = !bInQuote;
			break;

		// Check for whitespace.
		case _T(' '):   // Space
			if (!bInQuote)
			{
				paArgs->Add(sString.Mid(nStart, i - nStart));
				nStart = i+1; // Skip space.
			}
			break;
		}
	}

	// Add last parsed string.
	if (nStart < i)
	{
		paArgs->Add(sString.Mid(nStart, i - nStart));
	}

	// DEBUG Validation of array contents.
	#ifdef SANDBOX
		for (int k = 0; k < paArgs->GetCount(); ++k)
		{
			Prompt(PT_DEFAULT, _T("Debug"), _T("ParseCommandString\n\n") + paArgs->GetAt(k));
		}
	#endif

	// Verify results.
	if (paArgs->GetCount() < 1)
	{
		// Nothing parsed.
		return false;
	}

	// Done.
	return true;
}


//// Retrieve the associated extension command string and insert the arguments.
// sString - (IN)  String containing the extension.
// paArgs  - (OUT) Arguments array which to add the new items.
// Returns true if successful, false upon failure.
bool ParseExtensionCommand(CString sString, CArray<CString, CString>* paArgs, CMapStringToString* paFolders /*= NULL*/)
{
	// TODO Provide error notifications for different types of errors.
	//  Failed due to not found, invalid command, etc...

	// Validate parameters.
	if (sString.IsEmpty() || paArgs == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Extract the extension from the string.
	int nPeriod = sString.ReverseFind(_T('.'));
	if (nPeriod < 0) { nPeriod = 0; }
	CString sExt = sString.Mid(nPeriod);
	if (sExt.IsEmpty() || sExt.Find(_T('.')) == -1)
	{
		// No extension found.
		SetLastError(ERROR_NOT_FOUND);
		return false;
	}

	// Retrieve the associated extension command format string.
	CString sFormat;
	if (!GetExtensionCommand(sExt, sFormat))
	{
		return false;
	}

	// Insert the argument into the format string.
	CString sFormatted;
	//sFormatted.Format(sFormat, sString);
	if (!FormatIntoString(sFormatted, sFormat, sString))
	{
		return false;
	}

	// Replace System Path Keywords.
	if (paFolders != NULL)
	{
		ReplaceSystemKeywords(paFolders, sFormatted);
	}

	// Parse the commands from the formatted string.
	CArray<CString, CString> aTemp;
	if (!ParseCommandString(sFormatted, &aTemp))
	{
		return false;
	}

	// Replace Relative Paths.
	ReplaceRelativePaths(&aTemp);

	// TODO Validate new command.

	// TODO Check for existance of new command.

	// Remove first argument (its already at the end of the new array).
	paArgs->RemoveAt(0);

	// Insert new arguments at the beginning.
	paArgs->InsertAt(0, &aTemp);

	// DEBUG Validation of array contents.
	#ifdef SANDBOX
		for (int k = 0; k < paArgs->GetCount(); ++k)
		{
			Prompt(PT_DEFAULT, _T("Debug"), _T("ParseExtensionCommand\n\n") + paArgs->GetAt(k));
		}
	#endif

	// Success.
	return true;
}


//// Parse the keywords and data values from the external keywords file.
// Valid format example: keyword=value "keyword 2"="value 2".
// paKeywords - (OUT) CString CMap of the parsed keywords.
// Returns true on success and false on failure. See GetLastError for details.
bool ParseExternalKeywords(CMapStringToString* pmKeywords)
{
	// Validate Parameters.
	if (pmKeywords == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Retrieve keywords file location.
	CString sFile = GetTableString(IDS_KEYWORDS_FILE);
	if (sFile.IsEmpty())
	{
		SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
		return false;
	}

	// Replace relative paths in the file string.
	ReplaceRelativePaths(sFile);

	// Remove quote wrap.
	sFile = RemoveWrap(sFile);

	CString sKeywords;
	//if (!ReadEntireFileString(sFile, sKeywords))
	if (!ReadEntireTextFile(sFile, sKeywords))
	{
		// See GetLastError().
		return false;
	}

	// Parse the Keywords.
	SetLastError(ERROR_SUCCESS);
	return ParseKeywords(pmKeywords, sKeywords);
}


//// Parse the keywords and data values from the internal resource.
// Valid format example: keyword=value "keyword 2"="value 2".
// paKeywords - (OUT) CString CMap of the parsed keywords.
// Returns true on success and false on failure. See GetLastError for details.
bool ParseInternalKeywords(CMapStringToString* pmKeywords)
{
	// Validate Parameters.
	if (pmKeywords == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Load resource keywords string.
	CString sKeywords = _T("");
	if (sKeywords.LoadStringW(IDS_KEYWORDS) == 0)
	{
		SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
		return false;
	}

	// Parse the Keywords.
	SetLastError(ERROR_SUCCESS);
	return ParseKeywords(pmKeywords, sKeywords);;
}


//// Parse the keywords and data values from the keywords string.
// Valid format example: keyword=value "keyword 2"="value 2".
// pmKeywords - (OUT) String map of the parsed keywords.
// Returns true on success and false on failure. See GetLastError for details.
bool ParseKeywords(CMapStringToString* pmKeywords, CString sKeywords)
{
	// Validate Parameters.
	if (pmKeywords == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	// Setup parse.
	int  i        = 0;
	int  nStart   = 0;
	bool bKeyword = true;
	bool bInQuote = false;
	CString sKey  = _T("");

	// Parse keywords.
	int nLength = sKeywords.GetLength();
	for (i = 0; i < nLength; ++i)
	{
		switch(sKeywords.GetAt(i))
		{
			// Check for quotation strings.
			case _T('\"'):  // Quotation
					bInQuote = !bInQuote;
				break;

			// Check for whitespace.
			case (TCHAR)13: // Carriage Return (Fallthrough)
				if (i+1 < nLength && sKeywords.GetAt(i+1) == (TCHAR)10)
				{
					++i; // Skip Line Feed.
				}
			case (TCHAR)10: // Line Feed (Fallthrough)
			case (TCHAR)12: // Form Feed (Fallthrough)
			case _T(' '):   // Space (Fallthrough)
			case (TCHAR)9:  // Horizontal Tab
				if (!bInQuote && !bKeyword && !sKey.IsEmpty())
				{
					pmKeywords->SetAt(sKey.MakeLower(), sKeywords.Mid(nStart, i - nStart));
					sKey.Empty(); // Reset Key
					nStart = i+1; // Skip space.
					bKeyword = true;
				}
				break;

			case _T('='):   // Equals Symbol.
				if (!bInQuote && bKeyword)
				{
					sKey = sKeywords.Mid(nStart, i - nStart);
					nStart = i+1; // Skip symbol.
					bKeyword = false;
				}
				break;
		}
	}

	// Add last data value to array.
	if (nStart < i && !bKeyword && !sKey.IsEmpty())
	{
		pmKeywords->SetAt(sKey.MakeLower(), sKeywords.Mid(nStart, i - nStart));
	}

	// DEBUG Validation of array contents.
	#ifdef SANDBOX
		CMapStringToString::CPair* p = pmKeywords->PGetFirstAssoc();
		while (p != NULL)
		{
			Prompt(PT_DEFAULT, _T("Debug"), _T("ParseKeywords\n\n") + p->key + _T("=") + p->value);
			p = pmKeywords->PGetNextAssoc(p);
		}
	#endif

	// Done.
	return true;
}


//// Extract the Show option if defined in the command argument.
// sArg  - (IN)  Command argument string.
// nShow - (OUT) The show option value.
// Returns true on success and false on failure. See GetLastError for details.
bool ParseShowOption(CString sArg, int& nShow)
{
	// Validate Parameters.
	if (sArg.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		nShow = -1;
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	// Identify Option.
	if (sArg.CompareNoCase(_T("hide")) == 0)
	{
		nShow = SW_HIDE; // 0
	}
	else if (sArg.CompareNoCase(_T("show")) == 0)
	{
		nShow = SW_SHOW; // 5
	}
	else if (sArg.CompareNoCase(_T("showna")) == 0)
	{
		nShow = SW_SHOWNA; // 8
	}
	else if (sArg.CompareNoCase(_T("shownormal")) == 0)
	{
		nShow = SW_SHOWNORMAL; // 1
	}
	else if (sArg.CompareNoCase(_T("minimize")) == 0)
	{
		nShow = SW_MINIMIZE; // 6
	}
	else if (sArg.CompareNoCase(_T("shownoactivate")) == 0)
	{
		nShow = SW_SHOWNOACTIVATE; // 4
	}
	else if (sArg.CompareNoCase(_T("showmaximized")) == 0)
	{
		nShow = SW_SHOWMAXIMIZED; // 11
	}
	else if (sArg.CompareNoCase(_T("maximize")) == 0)
	{
		nShow = SW_MAXIMIZE; // 12
	}
	else if (sArg.CompareNoCase(_T("restore")) == 0)
	{
		nShow = SW_RESTORE; // 13
	}
	else
	{
		// Option not found.
		nShow = -1;
		return false;
	}

	return true;
}


//// Extract the Verb option if defined in the command argument.
// sArg  - (IN)  Command argument string.
// sVerb - (OUT) Verb option string.
// Returns true on success and false on failure. See GetLastError for details.
bool ParseVerbOption(CString sArg, CString& sVerb)
{
	// Validate Parameters.
	if (sArg.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		sVerb.Empty();
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	// Identify Option.
	if (sArg.CompareNoCase(_T("Open")) == 0)
	{
		sVerb = _T("Open");
	}
	else if (sArg.CompareNoCase(_T("Edit")) == 0)
	{
		sVerb = _T("Edit");
	}
	else if (sArg.CompareNoCase(_T("Print")) == 0)
	{
		sVerb = _T("Print");
	}
	else if (sArg.CompareNoCase(_T("Find")) == 0)
	{
		sVerb = _T("Find");
	}
	// The following verbs are not supported in Windows CE
	else if (sArg.CompareNoCase(_T("Explore")) == 0)
	{
		sVerb = _T("Explore");
	}
	else if (sArg.CompareNoCase(_T("Properties")) == 0)
	{
		sVerb = _T("Properties");
	}
	else
	{
		// Option not found.
		sVerb.Empty();
		return false;
	}

	return true;
}


//// Display a dialog prompt to the user for the command string.
// sCommand - (IN)(OUT) The command string entered by the user.
// Returns 0 for success, else the do modal return value.
int PromptForCommand(CString& sCommand)
{
	// Create Dialog.
	CRunDialog RunDlg;

	// Set the string.
	RunDlg.SetValue(sCommand);

	// Not fullscreen.
	//RunDlg.m_bFullScreen = FALSE;

	// Show Dialog.
	INT_PTR nReturn = RunDlg.DoModal();

	// TODO Show suggestions in the dialog.

	// Parse return value.
	switch (nReturn)
	{
	case IDOK:
		// Retrieve string.
		sCommand = RunDlg.GetValue();
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


//// Read the entire file.
// sFilename - (IN) String filename to read.
// pData     - (OUT) Pointer to a buffer of the data. Free when done!
// nSize     - (OUT) The size/count of the data bytes.
// Returns true on success, false on failure. See GetLastError() for details.
bool ReadEntireFile(CString sFilename, byte* pData, DWORD& nSize)
{
	// Validate Parameters.
	if (sFilename.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Open the file.
	HANDLE hFile = CreateFile(sFilename,
							  GENERIC_READ,
							  NULL,
							  NULL,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return false;
	}

	// Get the size (in bytes).
	nSize = GetFileSize(hFile, NULL);
	if (nSize == 0xFFFFFFFF)
	{
		// See GetLastError().
		return false;
	}
	++nSize; // Add room for null terminator.

	// Setup.
	DWORD nRead = 0; // Number of bytes read each time.
	pData = (byte*)malloc(nSize); // Allocate the Buffer.
	if (pData == NULL)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}

	// Read.
	BOOL bResult = ReadFile(hFile, (LPVOID)pData, nSize, &nRead, NULL);

	// Cleanup.
	CloseHandle(hFile);

	// Verify pointer.
	if (pData == NULL || bResult == FALSE)
	{
		// See GetLastError().
		free(pData);
		pData = NULL;
		return false;
	}

	// Terminate.
	pData[nSize-1] = 0;

	// Done.
	SetLastError(ERROR_SUCCESS);
	return true;
}


//// Read the entire file.
// sFilename - (IN) String filename to read.
// sData     - (OUT) String of the entire file contents.
// Returns true on success, false on failure. See GetLastError() for details.
bool ReadEntireFileAsText(CString sFilename, CString& sData)
{
	// Clear Buffer.
	sData.Empty();

	// Read the file.
	byte* pData = NULL;
	DWORD nSize = 0;
	if (!ReadEntireFile(sFilename, pData, nSize) || pData == NULL)
	{
		// See GetLastError().
		return false;
	}

	// Detect Type.
	TFormat Encoding = DetectTextEncoding(pData);

	// TODO...
	// Convert Data.
	TCHAR* pString = NULL;
	switch(Encoding)
	{
		case TF_ASCII:
			pString = xAsciiToTChar(pData, nSize);
			break;
		case TF_BOCU_1:
			// Not Supported.
			Prompt(PT_INFO, _T("Unsupported Format"), _T("The Keywords.txt file's current format is not supported. Please use UTF 8 or 16."));
			break;
		case TF_GB_18030:
			// Not Supported.
			Prompt(PT_INFO, _T("Unsupported Format"), _T("The Keywords.txt file's current format is not supported. Please use UTF 8 or 16."));
			break;
		case TF_SCSU:
			// Not Supported.
			Prompt(PT_INFO, _T("Unsupported Format"), _T("The Keywords.txt file's current format is not supported. Please use UTF 8 or 16."));
			break;
		case TF_UTF_1:
			// Not Supported.
			Prompt(PT_INFO, _T("Unsupported Format"), _T("The Keywords.txt file's current format is not supported. Please use UTF 8 or 16."));
			break;
		case TF_UTF_16BE:
			break;
		case TF_UTF_16LE:
			break;
		case TF_UTF_32BE:
			break;
		case TF_UTF_32LE:
			break;
		case TF_UTF_7:
			// Not Supported.
			Prompt(PT_INFO, _T("Unsupported Format"), _T("The Keywords.txt file's current format is not supported. Please use UTF 8 or 16."));
			break;
		case TF_UTF_8:
			break;
		case TF_UTF_EBCDIC:
			// Not Supported.
			Prompt(PT_INFO, _T("Unsupported Format"), _T("The Keywords.txt file's current format is not supported. Please use UTF 8 or 16."));
			break;
		case TF_UNKNOWN: // Fall Through.
		default:
			// Assume UTF-8 instead of ASCII for broader support.
			break;
	}

	// Validate.
	if (pString == NULL)
	{
		// See GetLastError().
		free(pData);
		return false;
	}
	free(pData);

	// Save.
	sData = CString(pString);
	delete[] pString;

	// Done.
	SetLastError(ERROR_SUCCESS);
	return true;
}


//// NEED TO REIMPLEMENT USING FOPEN FOR CACHING.
//// HOWEVER, UNICODE RESEARCH IS BENEFICIAL!
//// Read the entire file.
// sFilename - (IN) String filename to read.
// pData     - (OUT) Pointer to a buffer of the data. Free when done!
// Returns true on success, false on failure. See GetLastError() for details.
bool ReadEntireFileString(CString sFilename, CString& sData)
{
	// Validate Parameters.
	if (sFilename.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Open the file.
	HANDLE hFile = CreateFile(sFilename,
							  GENERIC_READ,
							  NULL,
							  NULL,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return false;
	}

	// Get the size (in bytes).
	DWORD nSize = GetFileSize(hFile, NULL);
	if (nSize == 0xFFFFFFFF)
	{
		// See GetLastError().
		return false;
	}
	++nSize; // Add room for null terminator.

	// Setup.
	DWORD nRead = 0; // Number of bytes read each time.
	byte* pData = (byte*)malloc(nSize); // Allocate the Buffer.
	if (pData == NULL)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return false;
	}

	// Read.
	BOOL bResult = ReadFile(hFile, (LPVOID)pData, nSize, &nRead, NULL);

	// Cleanup.
	CloseHandle(hFile);

	// Verify pointer.
	if (pData == NULL || bResult == FALSE)
	{
		// See GetLastError().
		free(pData);
		return false;
	}

	// Terminate.
	pData[nSize-1] = 0;

	// Convert.
	TCHAR* pString = xCharToTChar((char*)pData, nSize);
	if (pString == NULL)
	{
		// See GetLastError().
		free(pData);
		return false;
	}
	free(pData);

	// Save.
	sData = CString(pString);
	delete[] pString;

	// Done.
	SetLastError(ERROR_SUCCESS);
	return true;
}


//// Read the entire file.
// sFilename - (IN) String filename to read.
// pData     - (OUT) Pointer to a buffer of the data. Free when done!
// Returns true on success, false on failure. See GetLastError() for details.
bool ReadEntireTextFile(CString sFilename, CString& sData)
{
	// Setup.
	sData.Empty();

	// Validate Parameters.
	if (sFilename.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Open the file.
	FILE* pFile = _tfopen(sFilename, _T("rt, ccs=UTF-8"));
	if (pFile == NULL)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return false;
	}

	// Read the file.
	CString sTemp;
	while (!feof(pFile))
	{
		if (_fgetts(sTemp.GetBuffer(256), 256, pFile) != NULL)
		{
			// Save data.
			sTemp.ReleaseBuffer();
			sData.Append(sTemp);
		}
		else if (ferror(pFile) != 0)
		{
			// Error occurred.
			sTemp.ReleaseBuffer();
			fclose(pFile);
			return false;
		}
		else
		{
			sTemp.ReleaseBuffer();
		}
	}

	// Done.
	fclose(pFile);
	SetLastError(ERROR_SUCCESS);
	return true;
}


//// OBSOLETE
// pData   - (IN) The byte array.
// Returns true on success, false on failure.
// See GetLastError() for details.
bool RemoveByteOrderMark(byte* pData, TFormat Encoding)
{
	// Validate Parameter.
	if (pData == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);

	// Get the Length of the BOM (in bits).
	UINT nBits = 0;
	switch(Encoding)
	{
		case TF_BOCU_1:
			nBits = 24;
			break;
		case TF_GB_18030:
			nBits = 32;
			break;
		case TF_SCSU:
			nBits = 24;
			break;
		case TF_UTF_1:
			nBits = 24;
			break;
		case TF_UTF_16BE:
			nBits = 16;
			break;
		case TF_UTF_16LE:
			nBits = 16;
			break;
		case TF_UTF_32BE:
			nBits = 32;
			break;
		case TF_UTF_32LE:
			nBits = 32;
			break;
		case TF_UTF_7:
			nBits = 30;
			break;
		case TF_UTF_8:
			nBits = 24;
			break;
		case TF_UTF_EBCDIC:
			nBits = 32;
			break;
		case TF_UNKNOWN:
		case TF_ASCII:
		default:
			// Nothing To Do.
			// Already set to zero.
			break;
	}

	//pData += (int)(nBits / 8);

	return true;
}


//// Remove whitespace from a string.
// sString - (IN) String to clean.
// Returns the input string without any whitespace.
CString RemoveWhitespace(CString sString)
{
	// Setup
	SetLastError(ERROR_SUCCESS);

	// Validate parameters.
	if (sString.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return sString;
	}

	// Search string for whitespace.
	TCHAR c;
	CString sTemp = _T("");
	int len = sString.GetLength();
	for (int i = 0; i < len; ++i)
	{
		c = sString.GetAt(i);
		switch (c)
		{
		case (TCHAR)32: // Space
		case (TCHAR)9:  // Horizontal Tab
		case (TCHAR)11: // Vertical Tab
		case (TCHAR)10: // Line Feed
		case (TCHAR)12: // Form Feed
		case (TCHAR)13: // Carriage Return
			continue; // Whitespace Found!
			break;
		default:
			sTemp.AppendChar(c); // Keep
			break;
		}
	}

	return sTemp;
}


//// Remove all continuous Symbol Wraps from a string.
// sString - (IN) String to Unwrap.
// cSymbol - (IN) Wrapping Symbol.
// nTimes  - (IN) Specific number of wraps to remove. (Default -1 = all)
// Returns the string unwrapped from the symbol.
CString RemoveWrap(CString sString, TCHAR cSymbol /*= _T('\"')*/, int nTimes /*= -1*/)
{
	// Loop.
	int nLen = sString.GetLength() -1; // Zero Base.
	for (int i = 0; i < nLen -i; ++i)
	{
		// Stop When Zero.
		if (nTimes == 0)
		{
			return sString;
		}

		// Symbol Check.
		if (sString.GetAt(i) == cSymbol &&
			sString.GetAt(nLen -i) == cSymbol)
		{
			// Wrap Found, Remove Wrap.
			sString = sString.Mid(i+1, nLen-(i+1));
			--nTimes;
		}
		else
		{
			// Continuous Wrap Broken.
			return sString;
		}
	}

	// Return UnWrapped.
	return sString;
}


//// Wrap or unwrap the string from the specified symbol.
// sString - (IN) The string to be symboled or unsymboled.
// cSymbol - (IN) Character to wrap or unwrap from the string. (Default quote)
// bRemove - (IN) Only remove the symbols. (Default false)
// bDouble - (IN) Allow adding double symbols. (Default false)
// Nothing is done if both flags are set to true.
// Returns a string which has been modified.
//CString SymbolWrap(CString sString, TCHAR cSymbol /*= _T('\"')*/, bool bRemove /*= false*/, bool bDouble /*= false*/)
//{
//	// Validate Parameter.
//	if ( sString.IsEmpty() ||
//		(sString.GetLength() == 1 && sString.GetAt(0) == cSymbol) )
//	{
//		// Do nothing if string empty or if it is just a symbol.
//		return sString;
//	}
//
//	// Remove symbol if any, or insert symbol.
//	if ( !bDouble &&
//		 sString.GetAt(0) == cSymbol &&
//		 sString.GetAt(sString.GetLength() -1) == cSymbol )
//	{
//		return sString.Mid(1, sString.GetLength()-2);
//	}
//	else
//	{
//		// Check if remove only mode.
//		if (!bRemove)
//		{
//			return (cSymbol + sString + cSymbol);
//		}
//	}
//
//	// Do nothing.
//	return sString;
//}


//// Wrap a string with a specified symbol.
// sString - (IN) String with which to wrap.
// cSymbol - (IN) Symbol to wrap around the string.
// nTimes  - (IN) Number of wraps to apply.
// bExact  - (IN) Only allow the exact number of wraps.
// Returns the string wrapped in the symbol.
CString SymbolWrap(CString sString, TCHAR cSymbol /*= _T('\"')*/, int nTimes /*= 1*/, bool bExact /*= false*/)
{
	// Subtract existing Wraps.
	if (bExact)
	{
		nTimes -= GetWrapCount(sString, cSymbol);
	}

	// Remove excess Wraps.
	if (nTimes < 0)
	{
		sString = RemoveWrap(sString, cSymbol, -nTimes);
	}

	// Wrap Loop.
	while(nTimes-- > 0)
	{
		// Wrap.
		sString.Insert(0, cSymbol);
		sString.AppendChar(cSymbol);
	}

	return sString;
}


//// Wrap or unwrap the string from the specified triple symbol.
// sString - (IN) The string to be symboled or unsymboled.
// cSymbol - (IN) Character to wrap or unwrap from the string. (Default quote)
// bRemove - (IN) Only remove the symbol. (Default false)
// If the string is partially wrapped in the symbol, complete the triple wrap.
// Returns a string which has been modified.
//CString TripleWrap(CString sString, TCHAR cSymbol /*= _T('\"')*/, bool bRemove /*= false*/)
//{
//	// Validate Parameter.
//	if ( sString.IsEmpty() ||
//		(sString.GetLength() == 1 && sString.GetAt(0) == cSymbol) )
//	{
//		// Do nothing if string empty or if it is just a symbol.
//		return sString;
//	}
//
//	// Create triple string.
//	CString sSingle;
//	CString sDouble;
//	CString sTriple;
//	sSingle.Format(_T("%c"), cSymbol);
//	sDouble.Format(_T("%c%c"), cSymbol, cSymbol);
//	sTriple.Format(_T("%c%c%c"), cSymbol, cSymbol, cSymbol);
//
//	// Remove triple symbol if any, or insert triple symbol.
//	if ( sString.GetLength() > 5 &&
//		 sString.Left(3).CompareNoCase(sTriple) == 0 &&
//		 sString.Right(3).CompareNoCase(sTriple) == 0 )
//	{
//		return sString.Mid(3, sString.GetLength()-6);
//	}
//	/* If it is partially wrapped already complete triple wrap. */
//	/* Double to Triple. */
//	else if ( !bRemove &&
//			  sString.GetLength() > 3 &&
//			  sString.Left(2).CompareNoCase(sDouble) == 0 &&
//			  sString.Right(2).CompareNoCase(sDouble) == 0 )
//	{
//		return (sSingle + sString + sSingle);
//	}
//	/* Single to Triple */
//	else if ( !bRemove &&
//			  sString.GetLength() > 1 &&
//			  sString.Left(1).CompareNoCase(sSingle) == 0 &&
//			  sString.Right(1).CompareNoCase(sSingle) == 0 )
//	{
//		return (sDouble + sString + sDouble);
//	}
//	else // Wrap with triple symbol.
//	{
//		// Do not add if only removing the symbols.
//		if (!bRemove)
//		{
//			return (sTriple + sString + sTriple);
//		}
//	}
//
//	// Do nothing.
//	return sString;
//}


//// Overload to loop through each argument of string array.
// paArgs - (IN)(OUT) array of path strings to update.
// Returns true if successful, false if failed. See GetLastError for details.
bool ReplaceRelativePaths(CArray<CString, CString>* paArgs)
{
	// Validate Parameters.
	if (paArgs == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Loop Setup.
	CString sArg  = _T("");

	// Loop through each argument.
	for (int i = 0; i < paArgs->GetCount(); ++i)
	{
		// Loop through each directory in the path argument.
		sArg = paArgs->GetAt(i);
		if (ReplaceRelativePaths(sArg))
		{
			// Save changes.
			paArgs->SetAt(i, sArg);
		}
	}

	// Done.
	return true;
}


//// Replace the relative paths with actual paths in a filesystem path string.
// sPath - (IN)(OUT) Path to be updated with actual paths.
// Returns true if successful, false if failed. See GetLastError for details.
bool ReplaceRelativePaths(CString& sPath)
{
	// Add support for relative directories with mapped directories (S:..\temp.txt).
	// However, this feature is not supported in Windows CE, therefore it will be
	//  skipped.

	// Relative path formats are:
	// STANDARD:
	// Single .\ or ".\
	// Double ..\, "..\, ..\..\, "..\..\, or etc.
	// NON STANDARD:
	// (n)    ...\, ....\, .....\, (n)\

	// Validate parameter.
	if (sPath.IsEmpty())
	{
		return false;
	}

	// Retrieve Application Directory.
	CString sDirectory = _T("");
	if (!GetAppDirectory(sDirectory))
	{
		return false;
	}

	// Verify Directory contains trailing backslash.
	if (sDirectory.GetAt(sDirectory.GetLength()-1) != _T('\\'))
	{
		sDirectory.AppendChar(_T('\\'));
	}

	// Unwrap path for processing.
	if (IsWrapped(sPath))
	{
		//sPath = SymbolWrap(sPath, _T('\"'), true);
		sPath = RemoveWrap(sPath);
	}

	// Loop Setup.
	int nOffset = 0;
	int nParent = 0;
	bool bFirst = true;
	bool bNext  = true;
	bool bFound = false;
	CString sName = _T("");

	// Loop through each directory in the path argument.
	while (bNext && GetDirectoryAt(sPath, nOffset, sName))
	{
		// Single relative path only allowed on first pass.
		if (bFirst && sName.CompareNoCase(_T(".")) == 0)
		{
			// Single relative path.
			bFound = true;
			bNext  = false;
		}
		else if (sName.CompareNoCase(_T("..")) == 0)
		{
			// Double relative path.
			bFound = true;
			nParent++;
		}
		// TODO Determine if this is allowable by Windows path standards.
		else if (IsSymbolOnly(sName, _T('.')))
		{
			// (n) relative path.
			bFound = true;
			nParent += sName.GetLength() -1;
		}
		else
		{
			// Non relative path directory name found.
			bNext = false;

			// Back up offset to previous directory.
			nOffset -= (sName.GetLength() + 1);
			if (nOffset < 0) { nOffset = 0; }

			/* nOffset was 10, now its 5.
			 * 01234567890
			 * ..\..\Temp\
			 */
		}

		// No longer first directory.
		bFirst = false;
	}

	// Replace relative path with actual path.
	if (bFound)
	{
		// Get the appropriate parent directory.
		CString sTemp = sDirectory;
		while (nParent-- > 0)
		{
			/* 0123456789012345678901234567890123456
			 * C:\Program Files\Company\Application\
			 */

			// For each parent remove right most directory.
			sTemp = sTemp.Left(sTemp.ReverseFind(_T('\\'))); // Left of Backslash
			sTemp = sTemp.Left(sTemp.ReverseFind(_T('\\')) +1); // Remove Folder Name
		}

		// If more parent identifiers were listed than actual parent
		//  directories, use the root directory.
		if (sTemp.IsEmpty())
		{
			sTemp = sDirectory.Left(sDirectory.Find(_T('\\')) +1);
		}

		// Replace the relative path and save changes.
		sPath = sTemp + sPath.Mid(nOffset+1);

		// Check if quote wrap needed.
		if (HasWhitespace(sPath) && !IsWrapped(sPath))
		{
			sPath = SymbolWrap(sPath);
		}
	}

	// DEBUG Validation of array contents.
	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Debug"), _T("ReplaceRelativePaths\n\n") + sPath);
	#endif

	// Done.
	return true;
}


//// Convert the system keywords in the command string to their appropriate values.
// paKeywords - (IN) Array of system keywords and their values.
// sString    - (IN)(OUT) String to search for and replace the keywords.
// The Symbol cannot be a quotation mark!
// Returns true if successful, false if not. See GetLastError for more details.
bool ReplaceSystemKeywords(CMapStringToString* paKeywords, CString& sString)
{
	// Validate Parameters.
	if (paKeywords == NULL || sString.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Local variables.
	int nStart = 0;
	bool bInQuote  = false;
	bool bInSymbol = false;
	CString sKeyword = _T("");
	CString sValue   = _T("");

	// Loop through string.
	for (int i = 0; i < sString.GetLength(); ++i)
	{
		// Find the symbol.
		switch (sString.GetAt(i))
		{
		case SYSTEM_PATH_SYMBOL:
			// Is it the start or end.
			bInSymbol = !bInSymbol;
			if (bInSymbol)
			{
				// Set the starting position, exclusive.
				nStart = i +1;
			}
			else
			{
				// Locate the keyword's value (if set).
				sKeyword = sString.Mid(nStart, i - nStart);
				if (paKeywords->Lookup(sKeyword.MakeLower(), sValue) == TRUE)
				{
					// Replace the keword in the command string.
					// Include the symbols when being replaced.
					sString = ( sString.Left(nStart -1) +
							    sValue +
							    sString.Right(sString.GetLength() - i -1) );

					// Reset the position in case the value length < keyword length.
					i = nStart + sValue.GetLength() -1;

					// Check if Keyword needs to be wrapped in quotes.
					if (!bInQuote && HasWhitespace(sValue))
					{
						// Add left quote.
						int nIndex = sString.Left(nStart).ReverseFind(_T(' ')) + 1;
						sString.Insert(nIndex, _T('\"'));

						// Add right quote.
						nStart += sValue.GetLength();
						nIndex = WhitespaceFind(sString.Mid(nStart)) + nStart;
						if (nIndex < nStart) { nIndex = sString.GetLength(); }
						sString.Insert(nIndex, _T('\"'));
					}
				}
				// Else the keyword has not been defined on the user's device.
			}
			break;

		case _T('\"'):
			bInQuote = !bInQuote;
			break;
		}
	}

	// DEBUG Validation of array contents.
	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Debug"), _T("ReplaceSystemKeywords\n\n") + sString);
	#endif

	return true;
}


//// Check if a the file exists in any of the paths.
// paPaths - (IN) String array of paths to check (uses CMap value field).
// sFile   - (IN)(OUT) Filename to search for in the paths.
// bMatch  - (IN) Return first match to the file path.
// Returns true if found and appends the path to the file string
//  false if not found or error. See GetLastError() for more details.
bool SearchSystemPaths(CMapStringToString* paPaths, CString& sFile, bool bMatch /* = false*/)
{
	// Validate parameters.
	if (paPaths == NULL || sFile.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// TODO Search Local Directory First.

	// Local variables.
	CString sTemp = _T("");

	// Check if file exists.
	if (!FileExists(sFile, false, bMatch))
	{
		// Loop through folders.
		CMapStringToString::CPair* p = paPaths->PGetFirstAssoc();
		while (p != NULL)
		{
			// DEBUG Validation.
			#ifdef SANDBOX
				Prompt(PT_DEFAULT, _T("Run"), p->value);
			#endif

			// Check for the file in the folder.
			sTemp = p->value + _T('\\') + sFile;
			if (FileExists(sTemp, false, bMatch))
			{
				// Found.
				sFile = sTemp;

				// DEBUG Validation.
				#ifdef SANDBOX
					Prompt(PT_DEFAULT, _T("Debug"), _T("SearchSystemPaths\n\nFound: ") + sFile);
				#endif

				return true;
			}
			p = paPaths->PGetNextAssoc(p);
		}

		// DEBUG Validation.
		#ifdef SANDBOX
			Prompt(PT_DEFAULT, _T("Debug"), _T("SearchSystemPaths\n\nNot Found: ") + sFile);
		#endif

		// Not found.
		return false;
	}

	// TODO Combine the file and folder searching logic.

	// TODO If item is a folder, then fexplore must be inserted at the beginning
	//  of the command string.  This is going to require a redesign of code.

	// Check if folder exists.
	//if (!FileExists(sFile, true, bMatch))
	//{
	//	// Loop through folders.
	//	CMapStringToString::CPair* p = paPaths->PGetFirstAssoc();
	//	while (p != NULL)
	//	{
	//		// DEBUG Validation.
	//		#ifdef SANDBOX
	//			Prompt(PT_DEFAULT, _T("Run"), p->value);
	//		#endif

	//		// Check for the folder in the folder.
	//		sTemp = p->value + _T('\\') + sFile;
	//		if (FileExists(sTemp, true, bMatch))
	//		{
	//			// Found.
	//			sFile = sTemp;

	//			// DEBUG Validation.
	//			#ifdef SANDBOX
	//				Prompt(PT_DEFAULT, _T("Debug"), _T("SearchSystemPaths\n\nFound: ") + sFile);
	//			#endif

	//			return true;
	//		}
	//		p = paPaths->PGetNextAssoc(p);
	//	}

	//	// DEBUG Validation.
	//	#ifdef SANDBOX
	//		Prompt(PT_DEFAULT, _T("Debug"), _T("SearchSystemPaths\n\nNot Found: ") + sFile);
	//	#endif

	//	// Not found.
	//	return false;
	//}

	// DEBUG Validation.
	#ifdef SANDBOX
		Prompt(PT_DEFAULT, _T("Debug"), _T("SearchSystemPaths\n\nFound: ") + sFile);
	#endif

	// Exists at defined location.
	return true;
}


//bool Search(CArray<CString, CString>* paPath, CString& sFile)
//{
//}


//// Check the string for any reserved Windows characters.
// sString    - (IN) String to be checked.
// bBackslash - (IN) Include the backslash in the check, otherwise excluded.
// Returns true if string is valid, false if invalid character found.
bool ValidateString(CString sString, bool bBackslash /* = false */)
{
	// Validate parameters.
	if (sString.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	//Check against each character in the string.
	int nLength = sString.GetLength();
	for (int i = 0; i < nLength; ++i)
	{
		switch (sString.GetAt(i))
		{
		// Windows Reserved Characters
		case _T('\"'): // Quotation Mark
			if (i == 0 || i == nLength-1)
			{
				// Skip leading and trailing quotation marks.
				break;
			}
		case _T(':'):  // Colon
		case _T('*'):  // Asterisk
		case _T('?'):  // Question Mark
		case _T('<'):  // Left Chevron
		case _T('>'):  // Right Chevron
		case _T('|'):  // Pipe
		case _T('/'):  // Forwardslash
			// Invalid character found.
			return false;
		case _T('\\'): // Backslash (Filesystem & Registry)
			// Invalid character found.
			if (bBackslash)
			{
				return false;
			}
			break;
		}
	}
	return true;
}


//// Search a string for whitespace.
// sString - (IN) The string to search for whitespace.
// Returns the index of the first whitespace character found or -1 if not found.
int WhitespaceFind(CString sString)
{
	// Validate parameters.
	if (sString.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -1;
	}

	// Search string for whitespace.
	int nLen = sString.GetLength();
	for (int i = 0; i < nLen; ++i)
	{
		switch (sString.GetAt(i))
		{
		case (TCHAR)32: // Space
		case (TCHAR)9:  // Horizontal Tab
		case (TCHAR)11: // Vertical Tab
		case (TCHAR)10: // Line Feed
		case (TCHAR)12: // Form Feed
		case (TCHAR)13: // Carriage Return
			return i; // Whitespace Found!
			break;
		}
	}

	// No whitespace found.
	return -1;
}


//// Convert a Char array into a TCHAR CString.
// pData  - (IN) Pointer to the char array.
// nCount - (IN) Number of characters in the array.
// Return a string of the characters. Null string on failure.
// See GetLastError() for details.
CString xCharToCString(char* pData, UINT nCount)
{
	// Validate Parameters.
	if (pData == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return _T("");
	}

	// Setup.
	CString sString = _T("");

	// Convert Characters.
	for (UINT i = 0; i < nCount; ++i)
	{
		// Null check.
		if (pData[i] == NULL)
		{
			SetLastError(ERROR_INVALID_INDEX);
			return _T("");
		}

		// Save.
		sString.AppendChar((TCHAR)pData[i]);
	}
	SetLastError(ERROR_SUCCESS);
	return sString;
}


TCHAR* xAsciiToTChar(byte* pData, UINT nCount)
{
	return xCharToTChar((char*)pData, nCount);
}


//// Convert a char array to a TCHAR array.
// pData  - (IN) Pointer to a char array.
// nCount - (IN) Number of characters in the array.
// Returns a pointer to a TCHAR array. delete[] when done!
// NULL on failure. See GetLastError() for details.
TCHAR* xCharToTChar(char* pData, UINT nCount)
{
	// Validate Parameters.
	if (pData == NULL || nCount == 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	// Setup.
	TCHAR* pString = new TCHAR[nCount];

	// Convert Characters.
	for (UINT i = 0; i < nCount; ++i)
	{
		// Null Check.
		if (pData[i] == NULL)
		{
			SetLastError(ERROR_INVALID_INDEX);
			return NULL;
		}

		// Save.
		pString[i] = (TCHAR)pData[i];
	}
	SetLastError(ERROR_SUCCESS);
	return pString;
}


//// Convert a TCHAR CString into a char array.
// sString - (IN) String to convert.
// Return a char array pointer of the characters within the string.
// delete[] when done! NULL on failure. See GetLastError() for details.
char* xCStringToChar(CString sString)
{
	// Validate Parameters.
	if (sString.IsEmpty())
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	// Setup.
	UINT nCount = sString.GetLength();
	char* pData = new char[nCount];

	// Convert Characters.
	TCHAR c;
	for (UINT i = 0; i < nCount; ++i)
	{
		c = sString.GetAt(i);
		pData[i] = (char)(c > 0xFE ? 0xFE : c);
	}
	SetLastError(ERROR_SUCCESS);
	return pData;
}


//// Convert a TCHAR array into a char array.
// pString - (IN) TCHAR array.
// nCount  - (IN) Number of characters in the array.
// Returns a pointer to a char array. delete[] when done!
// NULL on failure. See GetLastError() for details.
char* xTCharToChar(TCHAR* pString, UINT nCount)
{
	// Validate Parameters.
	if (pString == NULL || nCount == 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	// Setup.
	char* pData = new char[nCount];

	// Convert Characters.
	for (UINT i = 0; i < nCount; ++i)
	{
		// Null Check.
		if (pString[i] == NULL)
		{
			SetLastError(ERROR_INVALID_INDEX);
			return NULL;
		}

		// Save.
		pData[i] = (char)(pString[i] > 0xFE ? 0xFE : pString[i]);
	}
	SetLastError(ERROR_SUCCESS);
	return pData;
}


// NOTE: This function is not complete!  This was written as an exercise to
// understand the inner workings of UNICODE conversions.
// See http://msdn.microsoft.com/en-us/library/aa450778
bool xUTF8ToTChar(byte* pData, UINT nCount, TCHAR* pBuffer, UINT& nTotal)
{
	// Validate Parameters.
	if (pData == NULL || nCount == 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Setup.
	nTotal = nCount;
	DWORD nChar = 0;
	UINT nBytes = 0;
	UINT nIndex = 0;
	bool bValid = true;
	UINT nSize = sizeof(TCHAR);
	//pString = new TCHAR[nCount];
	pBuffer = (TCHAR*)malloc(nTotal);

	// Convert Characters.
	for (UINT i = 0; i < nCount; ++i)
	{
		// Get Next Byte.
		nChar = nChar << 8;
		nChar += pData[i];
		++nBytes;

		// Two Byte Check.
		if (nChar > 0x7F)
		{
			// Character is Multi-Byte.

			// Double Byte Flow.
			// Validate Character.
			if (nChar < 0xC2)
			{
				// Invalid UTF-8 character.
				bValid = false;
			}
			else if (nBytes < 2)
			{
				// Need to read the next byte.
				continue;
			}
			else if (nBytes == 2 && (pData[i] < 0x80 || pData[i] > 0xBF))
			{
				// Second Byte is invalid for a UTF-8 character.
				bValid = false;
			}

			// Triple Byte Check.
			else if (nChar > 0xDFBF)
			{
				// Triple Byte Flow.
				// Validate Character.
				if (nChar < 0xE0A0)
				{
					// Invalid UTF-8 character.
					bValid = false;
				}
				else if (nBytes < 3)
				{
					// Need to read the next byte.
					continue;
				}
				else if (nBytes == 3 && (pData[i] < 0x80 || pData[i] > 0xBF))
				{
					// Third Byte is invalid for a UTF-8 character.
					bValid = false;
				}

				// Skip the BOM (if any).
				else if (i == 2 && nChar == 0xEFBBBF)
				{
					// Reset.
					nChar  = 0;
					nBytes = 0;
					bValid = true;
					continue;
				}

				// Quad Byte Check.
				else if (nChar > 0xEFBFBF)
				{
					// Quad Byte Flow.
					// Validate Character.
					if (nChar < 0xF09080)
					{
						// Invalid UTF-8 character.
						bValid = false;
					}
					else if (nBytes < 4)
					{
						// Need to read the next byte.
						continue;
					}
					else if (nBytes == 4 && (pData[i] < 0x80 || pData[i] > 0xBF))
					{
						// Fourth Byte is invalid for a UTF-8 character.
						bValid = false;
					}
					else if (nChar > 0xF48FBFBF) // U+10FFFF
					{
						// Character Value is outside of Unicode limits.
						bValid = false;
					}
				}
			}
		}

		// Normalize or Correct the Character.
		if (bValid && nBytes > nSize)
		{
			// The character is larger than the TCHAR definition.
			// TODO Normalize or Replace with compatible character.
			// TODO Display the actual unicode value in ascii text.
			DWORD nTemp = 0;
			if (nChar < 0x80)
			{
				// Should not get here.
				nTemp = nChar;
			}
			else // if (nTemp > 0x7F)
			{
				// Do nothing (for now).
			}

			//     C2-DF      80-BF
			// 30:194-223 64:128-191 | 1920:80-2047

			//     E0-      80-BF
			// 14:194-207 64:128-191


			// Adjust the Buffer.
			if (nIndex + 8 > nTotal)
			{
				// Less than 8 characters available.
				// Pad adjustment to allow for additional conversion.
				nTotal += 256; // Add more than 8 to reduce additional reallocs.
				pBuffer = (TCHAR*)realloc(pBuffer, nTotal);
			}

			// Generate the Unicode Value String. /u10FFFF
			TCHAR* pTemp = new TCHAR[8];
			_stprintf(pTemp, _T("/u%06X"), nTemp);

			// Append the String.
			for (int x = 0; x < 8; ++x)
			{
				pBuffer[nIndex++] = (TCHAR)pTemp[x];
			}
			delete[] pTemp;
		}
		else if (!bValid)
		{
			// Warning!
			//  This function does not preserve the original byte structure.
			//   Therefore, mojibake can occur.
			// Solution: Use the output wisely (for display purposes only)!

			// TODO Display the actual unicode value in ascii text.
			if (nChar < 0x80)
			{
				// %u+FFFF% or /uFFFF
			}

			// Character is invalid.
			if (nSize > 1) // Unicode Capable.
			{
				// Use 'replacement character'.
				nChar = 0xEFBFBD; // �
			}
			else // ASCII.
			{
				nChar = 0xFE; // ■
			}
		}

		// Save Character.
		// if (!IsControlChar((TCHAR)nChar))
		//{
		pBuffer[nIndex++] = (TCHAR)nChar;
		//}

		// Reset.
		nChar  = 0;
		nBytes = 0;
		bValid = true;
	}
	SetLastError(ERROR_SUCCESS);
	return bValid;
}
