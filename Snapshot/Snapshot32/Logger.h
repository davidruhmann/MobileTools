/* File:	Logger.h
 * Created: Nov 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

//// Logger is a self contained log file api.
class Logger
{
private:
	// Variables
	FILE* m_pFile;

public:
	// Structors
	Logger ();
	Logger (bool bAutoName, LPTSTR psName = NULL);
	Logger (LPTSTR psLogFile);
	~Logger();

	// Function Prototypes
	bool Close         ();
	void Init          ();
	bool Open          (LPTSTR psLogFile);
	bool WriteByte     (byte* pb);
	bool WriteCharacter(TCHAR c);
	bool WriteLastError(LPTSTR sMessage, bool bNewLine = true, bool bTimestamp = true);
	bool WriteLastError(LPTSTR sMessage, DWORD nError, bool bNewLine = true, bool bTimestamp = true);
	bool WriteMessage  (LPTSTR sMessage, bool bNewLine = true, bool bTimestamp = false);
	bool WriteNewLine  ();
	bool WriteTimestamp();
};
