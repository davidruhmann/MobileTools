/* File:	Logger.cpp
 * Created: Nov 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

//// External Includes (see stdafx.h)

//// Local Includes
#include "stdafx.h"
#include "Logger.h" // Class Definition

//// Definitions
#define LOG_PATH     _T("C:\\Temp")
#define DEFAULT_NAME _T("Snapshot")


//// Class Initialization
void Logger::Init()
{
	m_pFile = NULL;
}


//// Class Constructor
Logger::Logger()
{
	Init();
}


//// Class Constructor Overload
Logger::Logger(bool bAutoName, LPTSTR psName /*= NULL*/)
{
	Init();
	if (bAutoName)
	{
		SYSTEMTIME Time;
		GetLocalTime(&Time);
		TCHAR* psFilename = new TCHAR[256];
		_stprintf_s(psFilename, 256,
				  _T("%s\\%u-%02u-%02u-%02u%02u-%02u_%s.txt\0"), LOG_PATH,
																 Time.wYear,
																 Time.wMonth,
																 Time.wDay,
																 Time.wHour,
																 Time.wMinute,
																 Time.wSecond,
																 psName == NULL ? DEFAULT_NAME : psName);
		Open(psFilename);
		delete[] psFilename;
	}
}


//// Class Constructor Overload
Logger::Logger(LPTSTR psLogFile)
{
	Init();
	Open(psLogFile);
	// Leave psLogFile cleanup to caller.
}


//// Class Destructor
Logger::~Logger()
{
	Close();
}


//// Close the open log file.
bool Logger::Close()
{
	// Verify open handle.
	if (m_pFile != NULL)
	{
		// Close file.
		if (fclose(m_pFile) == EOF)
		{
			// See _get_errno().
			return false;
		}
		m_pFile = NULL;
	}
	return true;
}


//// Open the specified log file.
bool Logger::Open(LPTSTR psLogFile)
{
	// Verify string.
	if (psLogFile == NULL)
	{
		// Set failure reasoning.
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Close opened file.
	if (m_pFile != NULL)
	{
		if (!Close())
		{
			// Close failed, ignore.
			m_pFile = NULL;
		}
	}

	// Open file.
	errno_t err = _tfopen_s(&m_pFile, psLogFile, _T("at")); // ANSI
	//errno_t err = _tfopen_s(&m_pFile, psLogFile, _T("at, ccs=UTF-8")); // Unicode
	//m_pFile = _tfopen(psLogFile, _T("wt")); // ANSI
	//m_pFile = _tfopen(psLogFile, _T("ab")); // Use with fwrite for direct data support.
	//m_pFile = _tfopen(psLogFile, _T("wb")); // Use with fwrite for direct data support.
	if (0 != err || NULL == m_pFile)
	{
		// See _get_errno().
		m_pFile = NULL;
		return false;
	}

	// Success.
	return true;
}


//// TCHAR to ANSI.
//#ifdef _UNICODE
//	#define _ALLOC((LPTSTR)s) (PCHAR)_alloca(_tcslen(s))
//	#define T2A((LPTSTR)s) wcstombs(
//#else
//	#define T2A((LPTSTR)s) (PCHAR)s
//#endif


//// Write a single byte to the log file.
bool Logger::WriteByte(byte* pb)
{
	// Verify handles.
	if (m_pFile == NULL || pb == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Write the byte to the file.
	if (fwrite((void*)pb, 1, 1, m_pFile) != 1)
	{
		// See _get_errno().
		return false;
	}

	// Success.
	return true;
}


//// Write a single character to the log file.
bool Logger::WriteCharacter(TCHAR c)
{
	// Verify handles.
	if (m_pFile == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Write the character to the file.
	if (_fputtc(c, m_pFile) == EOF)
	{
		// See _get_errno().
		return false;
	}

	// Success.
	return true;
}


//// Write a message to the log file.
bool Logger::WriteMessage( LPTSTR psMessage,
						   bool   bNewLine   /*= true*/,
						   bool   bTimestamp /*= true*/)
{
	// Verify string.
	if (psMessage == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Verify handle.
	if (m_pFile == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	// Start new line.
	if (bNewLine && !WriteNewLine())
	{
		// See GetLastError().
		return false;
	}

	// Write Timestamp.
	if (bTimestamp && !WriteTimestamp())
	{
		// See GetLastError().
		return false;
	}

	// Write the message.
	if (_fputts(psMessage, m_pFile) == EOF)
	//if (fwrite((void*)psMessage, sizeof(TCHAR) * _tcslen(psMessage), 1, m_pFile) != 1)
	{
		// See _get_errno().
		return false;
	}

	// Success.
	return true;
}


//// Overload handler
bool Logger::WriteLastError( LPTSTR psMessage,
							 bool   bNewLine   /*= true*/,
							 bool   bTimestamp /*= true*/)
{
	// Retrieve error code.
	DWORD nError = GetLastError();
	return WriteLastError(psMessage, nError, bNewLine, bTimestamp);
}


//// Write an error message to the log file.
bool Logger::WriteLastError( LPTSTR  sMessage,
							 DWORD   nError,
							 bool    bNewLine   /*= true*/,
							 bool    bTimestamp /*= true*/)
{
	// Verify string.
	if (sMessage == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Verify handle.
	if (m_pFile == NULL)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}

	// Start new line.
	if (bNewLine && !WriteNewLine())
	{
		return false;
	}

	// Write timestamp.
	if (bTimestamp && !WriteTimestamp())
	{
		return false;
	}

	// Retrieve system error message.
	TCHAR* psSystemMessage = NULL; // Max 64K bytes ~2000 characters.
	DWORD  nCount = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
								   FORMAT_MESSAGE_FROM_SYSTEM |
								   FORMAT_MESSAGE_IGNORE_INSERTS,
								   NULL,
								   nError,
								   0, /* Default language */
								   (LPTSTR)&psSystemMessage,
								   0, /* Minimum allocation size */
								   NULL );

	// Verify retrieval.
	if (nCount == 0 || psSystemMessage == NULL)
	{
		// See GetLastError()
		return false;
	}

	// Replace LF, CR, FF characters.
	TCHAR c;
	for (DWORD i = 0; i < nCount; ++i)
	{
		c = psSystemMessage[i];
		switch(c)
		{
		case 10: // Line Feed
		case 12: // Form Feed
		case 13: // Carriage Return
			psSystemMessage[i] = _T(' '); // Space
			break;
		default:
			break;
		}
	}

	// Update error code.
	SetLastError(ERROR_INVALID_HANDLE);

	// Calculate string size.
	int nSize = 12; // 10 is longest value of DWORD.
	nSize += _tcslen(sMessage);
	nSize += _tcslen(psSystemMessage);
	nSize += 48; // Room for formating string.

	// Format the message some more.
	TCHAR* psTemp = new TCHAR[nSize];
	if (_stprintf_s(psTemp, nSize,
				  _T(" WARNING:  %s failed with error %d (%s)"),
				  sMessage,
				  nError,
				  psSystemMessage) == -1)
	{
		SetLastError(ERROR_INVALID_MESSAGE);
		LocalFree(psSystemMessage);
		delete[] psTemp;
		return false;
	}

	// Write the formatted message to the log file.
	if (_fputts(psTemp, m_pFile) == EOF)
	//if (fwrite((void*)psTemp, sizeof(TCHAR) * _tcslen(psTemp), 1, m_pFile) != 1)
	{
		// See _get_errno().
		LocalFree(psSystemMessage);
		delete[] psTemp;
		return false;
	}

	// Clean up.
	LocalFree(psSystemMessage);
	delete[] psTemp;

	// Success.
	return true;
}


//// Start a new line in the log file.
bool Logger::WriteNewLine()
{
	// Verify handle.
	if (m_pFile == NULL)
	{
		return false;
	}

	TCHAR c = _T('\n');
	//if (fwrite((void*)&c, sizeof(TCHAR) * 1, 1, m_pFile) != 1)
	if (_fputtc(c, m_pFile) == EOF)
	{
		// See _get_errno().
		return false;
	}

	// Success.
	return true;
}


//// Write a timestamp to the log file.
bool Logger::WriteTimestamp()
{
	// Verify handle.
	if (m_pFile == NULL)
	{
		return false;
	}

	// Get current time.
	SYSTEMTIME Time;
	GetLocalTime(&Time);

	// Generate a formatted timestamp string.
	TCHAR* psTimestamp = new TCHAR[64];
	if (_stprintf_s(psTimestamp, 64, _T("%u-%02u-%02u-%02u%02u-%02u\0"), Time.wYear,
																  Time.wMonth,
																  Time.wDay,
																  Time.wHour,
																  Time.wMinute,
																  Time.wSecond) == -1)
	{
		// Failed to generate string.
		SetLastError(ERROR_INVALID_PARAMETER);
		delete[] psTimestamp;
		return false;
	}

	// Write timestamp to the log.
	if (_fputts(psTimestamp, m_pFile) == EOF)
	//if (fwrite((void*)psTimestamp, sizeof(TCHAR) * _tcslen(psTimestamp), 1, m_pFile) != 1)
	{
		// See _get_errno().
		delete[] psTimestamp;	
		return false;
	}

	// Cleanup.
	delete[] psTimestamp;

	// Success.
	return true;
}
