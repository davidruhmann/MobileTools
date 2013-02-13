/* File:	Screenshot.cpp
 * Created: Dec 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

// TODO
// 1. Modularize Screenshot capabilities so that the data does not have to be saved to a file.
// 2. Cleanup Command Line Code.

//// External Inclues (see stdafx.h).

//// Local Includes (also see stdafx.h).
#include "stdafx.h"
// Added support for PPC (Imaging.h, GdiplusImaging.h, GdiplusPixelFormats.h)
#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
	#include "Imaging.h"
#else
	// Will not compile if this include is located in stdafx.h.
	#include <Imaging.h>
	// Use #include <WinGDI.h> // for desktop PCs.
#endif
#include "Missing.h" // Microsoft's Missing Definitnions.

//// Definitions
#define DEFAULT_DELAY 0

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


//// Funciton Prototypes.
bool  CaptureScreenToFile(LPTSTR sFilename, EncoderType nEncoder = EncoderTypePNG);
bool  SaveCapture        (PCAPTURE pCapture, bool bCompress = false);
bool  SaveEncodedCapture (PCAPTURE pCapture, EncoderType nEncoder = EncoderTypePNG);
DWORD StringToDWORD      (LPTSTR sValue);


//// Program entry point.
int _tmain(int argc, _TCHAR* argv[])
{
	// Set High Thread Priority.
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	// Local Variables.
	DWORD nDelay      = DEFAULT_DELAY;
	TCHAR* psFilename = NULL;
	EncoderType nType = EncoderTypePNG;

	// Parse Command Line Parameters.
	// d  Time Delay Command.
	// f  Filename Command.
	// e  Image Encoder Command.
	// s  Hide the Start Menu.
	// - or /  Escape Command Characters.
	// e.g. Screenshot.exe /d <ms> /f "<filename>" /e <#>
	TCHAR c;
	for (int i = 1; i < argc; ++i)
	{
		// Validate string.
		if (_tcslen(argv[i]) <= 1)
		{
			// Skip invalid length.
			continue;
		}

		// Check for command.
		c = argv[i][0];
		if(c == _T('/') || c == _T('-'))
		{
			switch(_totlower(argv[i][1]))
			{
			case _T('d'): // Time Delay Command.
				// Next argument should be an unsigned long number.
				if (i+1 <= argc)
				{
					nDelay = StringToDWORD(argv[i+1]);
				}
				break;
			case _T('f'): // Filename Command.
				// Next argument should be a string.
				if (i+1 <= argc)
				{
					int o = 0;
					int n = _tcslen(argv[i+1]);

					// Check for surrounding quotations to remove.
					if (argv[i+1][0]   == _T('\"')) { --n; o = 1; }
					if (argv[i+1][n-1] == _T('\"')) { --n; }

					// Copy string.
					psFilename = new TCHAR[n+1];
					_tcsncpy(psFilename, argv[i+1] + o, n);
					_tcsncpy(psFilename + n, _T("\0"), 1);
				}
				break;
			case _T('e'): // Encoder Command.
				// Next argument should be a number.
				if (i+1 <= argc)
				{
					nType = (EncoderType)StringToDWORD(argv[i+1]);
				}
				break;
			case _T('s'): // Start Menu Command.
				// Close the Menu. Ctrl + Esc
				// Not needed on WM 6.1 and earlier.
				INPUT input[4]; // Ctrl Down, Escape Down, Escape Up, Ctrl Up.
				input[0].type = input[1].type = input[2].type = input[3].type = INPUT_KEYBOARD;
				input[1].ki.wVk = input[2].ki.wVk = VK_ESCAPE;
				input[0].ki.wVk = input[3].ki.wVk = VK_CONTROL;
				input[2].ki.dwFlags = input[3].ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(4, input, sizeof(INPUT)); // This Works.
				//keybd_event(VK_CONTROL, VK_ESCAPE, KEYEVENTF_KEYUP, NULL); // keybd_event does not work.
				break;
			}
		}
	}

	// Validate string.
	if (psFilename != NULL)
	{
		int nLength = _tcslen(psFilename);
		for (int i = 0; i < nLength; ++i)
		{
			c = psFilename[i];
			switch (c)
			{
			// Windows Reserved Characters
			case _T(':'):  // Colon
			case _T('*'):  // Asterisk
			case _T('?'):  // Question Mark
			case _T('\"'): // Quotation Mark
			case _T('<'):  // Left Chevron
			case _T('>'):  // Right Chevron
			case _T('|'):  // Pipe
			case _T('/'):  // Forwardslash
			//case _T('\\'): // Backslash (Filesystem & Registry)
				// Invalid filename.
				i = nLength;
				delete[] psFilename;
				psFilename = NULL;
				break;
			}
		}
	}

	// Time delay before screenshot (in milliseconds).
	if (nDelay > 0)
	{
		Sleep(nDelay);
	}

	// Generate filename.
	if (psFilename == NULL)
	{
		SYSTEMTIME Time;
		GetLocalTime(&Time);
		psFilename = new TCHAR[256];
		_stprintf(psFilename,
					  _T("%s\\%u-%02u-%02u-%02u%02u-%02u_%s\0"), _T("\\Temp"),
																 Time.wYear,
																 Time.wMonth,
																 Time.wDay,
																 Time.wHour,
																 Time.wMinute,
																 Time.wSecond,
																 _T("Screenshot"));
	}

	#ifdef SANDBOX
		// Stopwatch Start.
		int  nEnd;
		int  nStart = GetTickCount();
	#endif

	// Capture Screen.
	bool nResult = CaptureScreenToFile(psFilename, nType);

	#ifdef SANDBOX
		// Stopwatch Stop.
		nEnd = GetTickCount();

		// Display Results.
		TCHAR* sTemp = new TCHAR[48];
		_stprintf(sTemp, _T("Result     = %s\nTime(ms) = %d"), nResult ? _T("Success") : _T("Failed"), nEnd - nStart);
		MessageBox(NULL, sTemp, _T("Capture"), 0);
		delete[] sTemp;
	#endif

	// Cleanup.
	delete[] psFilename;

	// End of Program.
	return 0;
}


// Convert String Numeric Value to Unsigned Long.
//  Only analyzes string til first space character or end line found,
//   then attempts to convert the characters up to that break.
//  Returns 0 if conversion failed.
DWORD StringToDWORD(LPTSTR sValue)
{
	if (sValue == NULL) { return 0; }
	DWORD d;
	LPTSTR pEnd;
	d = (DWORD)_tcstoul(sValue, &pEnd, 10);
	return d;
}


//// Save a screenshot of the primary display to an image file.
// sFilename - (IN) Path and filename (excluding extension) of the image file.
// nEncoder  - (IN) Image encoder to use if available, else manual bitmap is generated.
// Returns true on success and false on failure. See GetLastError for details.
bool CaptureScreenToFile(LPTSTR sFilename, EncoderType nEncoder /* = EncoderTypePNG */)
{
	// Validate parameters.
	if (sFilename == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	// Retrieve the device context handle to the entire screen.
	HDC hDC = GetDC( NULL );
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
	int w = GetDeviceCaps( hDC, HORZRES );  // Width (in pixels).
	int h = GetDeviceCaps( hDC, VERTRES );  // Height (in pixels).
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
