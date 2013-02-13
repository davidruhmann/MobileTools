/* File:	Crash.cpp
 * Created: Nov 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

/* Icon Source "Sign Error"
 * License: CC Attribution-Noncommercial-Share Alike 3.0
 * http://www.iconarchive.com/show/phuzion-icons-by-kyo-tux/Sign-Error-icon.html
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"

//// Program entry point.
int _tmain(int argc, _TCHAR* argv[])
{
	// Set High Thread Priority.
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// Invoke Dump for calling thread with custom error code.
	//  Use 0 instead of EXCEPTION_NONCONTINUABLE if you wish
	//  code execution to continue (Second Parameter).
	RaiseException(0x77777777, NULL, 0, NULL);

	// Examples of Crashes Below

	// Array Out of Bounds.
	//RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED, EXCEPTION_NONCONTINUABLE, 0, NULL);
	//argv[argc];

	// Divide by Zero.
	//RaiseException(EXCEPTION_INT_DIVIDE_BY_ZERO, EXCEPTION_NONCONTINUABLE, 0, NULL);
	//int i = 0;
	//i = (87 / i);

	// Format mismatch. (Data misaligned).
	//RaiseException(EXCEPTION_DATATYPE_MISALIGNMENT, EXCEPTION_NONCONTINUABLE, 0, NULL);
	//TCHAR* s = new TCHAR[64];
	//_stprintf( s, _T("%c%s"), _T("I am a string not a character!"), _T('c') );
	//delete[] s;

	// Null pointer.
	//RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, NULL);
	//int* p = NULL;
	//int  k = *p++;

	// Throw.
	//throw;

	// End of program.
	return 0;
}

