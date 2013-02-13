#include "stdafx.h"
//#include "ScreenCapture.h"
//
//ScreenCapture::Capture(int nBitDepth /* = -1 */)
//{
//	// Retrieve the device context handle to the entire screen.
//	HDC hDC = GetDC( NULL );
//	if (hDC == NULL)
//	{
//		// See GetLastError().
//		return false;
//	}
//
//	// Create a compatible device context handle for the bitmap.
//	HDC cDC = CreateCompatibleDC(hDC);
//	if (cDC == NULL)
//	{
//		// See GetLastError().
//		ReleaseDC( NULL, hDC );
//		return false;
//	}
//
//	// Get the system metrics.
//	//  Note: Define HI_RES_AWARE   CEUX   {1} in the resource for
//	//   compatibility with screen resolutions higher than 240x320.
//	int w = GetDeviceCaps( hDC, HORZRES );  // Width (in pixels).
//	int h = GetDeviceCaps( hDC, VERTRES );  // Height (in pixels).
//	int b = nBitDepth;
//	if (b < 1)
//	{
//		b = GetDeviceCaps( hDC, COLORRES ); // Bit Depth
//	}
//
//	// Validate bit depth.
//	// Bit depths below 16 are not supported (for now). Need to add
//	//  support for colorTables to use bit depths lower than 16.
//	/*if      (b <  4) { b =  1; }
//	else if (b <  8) { b =  4; }
//	else if (b < 16) { b =  8; }*/
//	if      (b < 16) { SetLastError(ERROR_UNSUPPORTED_TYPE); return false; }
//	else if (b < 24) { b = 16; }
//	else if (b < 32) { b = 24; }
//	else             { b = 32; }
//
//	// Bitmap Information Header "DIB" Section.
//	//  Initilize to zero, then only set what is needed.
//	BITMAPINFOHEADER bih = {0};
//	bih.biSize        = sizeof(BITMAPINFOHEADER);
//	bih.biWidth       = abs(w);
//	bih.biHeight      = abs(h);
//	bih.biPlanes      = 1;
//	bih.biBitCount    = b;
//	bih.biCompression = BI_RGB;
//	
//	// Bitmap Info Structure.
//	//  No color table is needed for bit depths above 8.
//	//  Color Table only provides optimizations for bit depths above 8.
//	BITMAPINFO bi;
//	bi.bmiHeader = bih;
//	bi.bmiColors[0].rgbBlue     = 0;
//	bi.bmiColors[0].rgbGreen    = 0;
//	bi.bmiColors[0].rgbRed      = 0;
//	bi.bmiColors[0].rgbReserved = 0;
//
//	// Create the bitmap specified in the header and assign the pointers.
//	LPVOID pBytes = NULL;
//	HBITMAP hBitmap = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pBytes, NULL, 0);
//	if (hBitmap == NULL || pBytes == NULL)
//	{
//		// See GetLastError()
//		ReleaseDC( NULL, hDC );
//		DeleteDC(cDC);
//		return false;
//	}
//
//	// Associate the bitmap with the compatible device context handle.
//	HGDIOBJ hOld = SelectObject(cDC, hBitmap);
//	if (hOld == NULL || hOld == (HGDIOBJ)HGDI_ERROR)
//	{
//		// See GetLastError()
//		ReleaseDC( NULL, hDC );
//		DeleteDC(cDC);
//		DeleteObject(hBitmap);
//		return false;
//	}
//
//	// Copy the bit data.
//	//  Use StretchBlt to flip the data vertically.
//	//if (BitBlt(cDC,
//	//		   0, 0,
//	//		   Capture.pBitmapInfo->bmiHeader.biWidth,
//	//		   Capture.pBitmapInfo->bmiHeader.biHeight,
//	//		   hDC,
//	//		   0, 0,
//	//		   SRCCOPY) == FALSE)
//	if (StretchBlt(cDC,
//				   0, 0,
//				   bi.bmiHeader.biWidth,
//				   bi.bmiHeader.biHeight,
//				   hDC,
//				   0, bi.bmiHeader.biHeight,
//				   bi.bmiHeader.biWidth,
//				   -1 * bi.bmiHeader.biHeight,
//				   SRCCOPY) == FALSE)
//	{
//		// See GetLastError()
//		ReleaseDC( NULL, hDC );
//		DeleteDC(cDC);
//		DeleteObject(hBitmap);
//		return false;
//	}
//
//	// Cleanup
//	SelectObject(cDC, hOld); // Restore DC object.
//	ReleaseDC( NULL, hDC );
//	DeleteDC(cDC);
//
//	// Save the screen capture.
//
//	// Cleanup.
//	//DeleteObject(Capture.hBitmap); // (Note: This also deletes the pBytes array!)
//
//	return bSuccess;
//}