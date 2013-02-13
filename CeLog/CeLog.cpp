/* File:	CeLog.cpp
 * Created: Jun 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"

//// Definitions.
typedef void (CALLBACK *PFNNKDBG)(LPCWSTR); // Fix for FARPROC with C++.
typedef BOOL (CALLBACK *PFNKLIOC)(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD);
#define CELZONE_CELOG_VIEWER 0x08000000
#define CELID_HWND 1000
#ifndef CELID_CALLSTACK
	#define CELID_CALLSTACK 91
#endif

//// Structures.
typedef struct _CEL_DWORDARRAY {
	DWORD adwData[0]; // Ignore Warning C4200.
} CEL_FLAGGED, *PCEL_FLAGGED, CEL_CALLSTACK, *PCEL_CALLSTACK;

//// Globals.
CeLogImportTable g_CeImports; // Interface provided by the kernel.
DWORD g_dwZoneCE; // Current zone settings for this DLL.
HWND g_hViewer; // CeLog Viewer Application.

//// Prototypes.
bool InitLibrary      (PFNKLIOC pfnKernelLibIoControl);
void MyCeLogData      (BOOL fTimeStamp, WORD wID, PVOID pData, WORD wLen, DWORD dwZoneUser, DWORD dwZoneCE, DWORD wFlag, DWORD fFlagged);
void MyCeLogInterrupt (DWORD dwlogvalue);
void MyCeLogSetZones  (DWORD dwZoneUser, DWORD dwZoneCE, DWORD dwZoneProcess);
BOOL MyCeLogQueryZones(LPDWORD lpdwZoneUser, LPDWORD lpdwZoneCE, LPDWORD ldpwZoneProcess);


//// DLL entry point.
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	// Setup.
	g_dwZoneCE = 0;
	g_hViewer = NULL;

	// Test Trust Level.
	switch (CeGetCurrentTrust())
	{
		case OEM_CERTIFY_TRUST:
			MessageBox(NULL, _T("Trusted."), _T("CeLog"), MB_OK);
			break;

		case OEM_CERTIFY_RUN:
			MessageBox(NULL, _T("Run Only."), _T("CeLog"), MB_OK);
			break;

		case OEM_CERTIFY_FALSE: // Fall Through
		default:
			MessageBox(NULL, _T("Not Trusted."), _T("CeLog"), MB_OK);
			break;
	}

	// Determine Workflow.
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			if (lpReserved != NULL)
			{
				// Reserved parameter is a pointer to KernelLibIoControl 
				if (InitLibrary((PFNKLIOC)lpReserved))
				{
					((PFNNKDBG)g_CeImports.pNKDbgPrintfW)(_T("CeLog DLL initialized!\r\n"));
					return TRUE;
				}
			}
			return FALSE;

		case DLL_PROCESS_DETACH:
			break;

		default:
			break;
    }
    return TRUE;
}


// Initialize and Attach DLL.
bool InitLibrary(PFNKLIOC pfnKernelLibIoControl)
{
	// Setup.
	CeLogExportTable CeExports = {0};

    // Begin with all zones enabled except CELZONE_KCALL.
    g_dwZoneCE = 0xFFBFFFFF; // Default.

    // Note:
    // KernelLibIoControl provides the interface we need to obtain kernel
    // function pointers and register logging functions.
    
    // Get the imports from the kernel.
    g_CeImports.dwVersion = 4; // Must be set to 4.
    if (pfnKernelLibIoControl((HANDLE)KMOD_CELOG,
							  IOCTL_CELOG_IMPORT,
							  &g_CeImports,
							  sizeof(CeLogImportTable),
							  NULL,
							  0,
							  NULL) == FALSE)
	{
        return false;
    }

    // Check preset zones in the desktop computer's registry.
    pfnKernelLibIoControl((HANDLE)KMOD_CELOG,
						  IOCTL_CELOG_GETDESKTOPZONE,
						  _T("CeLogZoneCE"),
						  sizeof(WCHAR) * 11,
						  &(g_dwZoneCE),
						  sizeof(DWORD),
						  NULL);

    // Force CELZONE_ALWAYSON to always be turned on.
    g_dwZoneCE |= CELZONE_ALWAYSON;
    
    // Register logging functions with the kernel.
    CeExports.dwVersion             = 2;
    CeExports.pfnCeLogData          = (PFNVOID)MyCeLogData;
    CeExports.pfnCeLogInterrupt     = (PFNVOID)MyCeLogInterrupt;
    CeExports.pfnCeLogSetZones      = (PFNVOID)MyCeLogSetZones;
    CeExports.pfnCeLogQueryZones    = (FARPROC)MyCeLogQueryZones;
    CeExports.dwCeLogTimerFrequency = 0;
    if (pfnKernelLibIoControl((HANDLE)KMOD_CELOG,
							  IOCTL_CELOG_REGISTER,
							  &CeExports,
							  sizeof(CeLogExportTable),
							  NULL,
							  0,
							  NULL) == FALSE)
	{
        ((PFNNKDBG)g_CeImports.pNKDbgPrintfW)(_T("Unable to register logging functions with kernel\r\n"));
        return false;
    }

    // Now that the logging functions will receive data from the kernel,
    // request a re-sync to get the kernel to log all existing processes,
    // threads and modules to the MyCeLogData function.
    g_CeImports.pCeLogReSync();

	return true;
}


//// CAUTIONS:
// The kernel often calls CeLogData from within parts of the kernel where it is
// unsafe to make system calls, and unsafe to interact with the kernel debugger,
// such as to set a breakpoint or step through code. As a result, you must be
// careful about such actions when you are implementing or debugging your
// CeLogData function. Failing to do so can cause an unrecoverable system crash.
//
// The following table shows CeLogImportTable functions that are safe to call
// from within a CeLogData implementation. Do not call any other functions from
// the CeLogImportTable inside your CeLogData implementation.
//
// CeLogImportTable function	Description
// --------------------------	------------
// pEventModify					Pointer to the DLL version of PEventModify.
//								 EventModify is used to implement the SetEvent,
//								 ResetEvent, and PulseEvent functions.
// pQueryPerformanceCounter		Pointer to the DLL's version of QueryPerformanceCounter.
// pQueryPerformanceFrequency	Pointer to the DLL's version of QueryPerformanceFrequency.
// pGetLastError				Pointer to the DLL's version of GetLastError.
// pSetLastError				Pointer to the DLL's version of SetLastError.
// pInSysCall					Pointer to InSysCall wrapper function.
//// http://msdn.microsoft.com/en-us/library/ms905110


//// References
// http://msdn.microsoft.com/en-us/library/ms922774
// http://msdn.microsoft.com/en-us/library/aa908988.aspx
// http://msdn.microsoft.com/en-us/library/aa909050.aspx
// http://msdn.microsoft.com/en-us/library/aa909194.aspx
void MyCeLogData(BOOL fTimeStamp, WORD wID, PVOID pData, WORD wLen, DWORD dwZoneUser, DWORD dwZoneCE, DWORD wFlag, DWORD fFlagged)
{
	// Setup Header (4 bytes).
	CEL_HEADER ch = {0}; // Initialize to Zero.
	ch.Length = wLen; // Does not include the Header, Timestamp, Flag, or Padding.
	ch.ID = wID; // CELID Value.
	//ch.Reserved = 0; // Done in Initialization.
	ch.fTimeStamp = fTimeStamp; // Timestamp boolean flag.

	// Timestamp (4 bytes).
	DWORD dwTimeStamp = (fTimeStamp == TRUE ? GetTickCount() : 0);

	// Flagging (4 bytes).
	// http://msdn.microsoft.com/en-us/library/ms905096
	//WORD wID = wID; // Use existing variable.
	//WORD wFlag = wFlag; // Use existing variable.

	// Calculate Padding.
	WORD wRemainder(wLen % sizeof(DWORD));

	// Determine Zone & Event.
	switch (dwZoneCE)
	{
		case CELZONE_ALWAYSON: // Always Logged Events.
			switch (wID)
			{
				case CELID_FLAGGED:
				{
					// pData[wLen]
					break;
				}

				case CELID_DATA_LOSS:
				{
					// Validate.
					if (wLen < sizeof(CEL_DATA_LOSS))
					{
						// Warning, Data Incomplete!
						// TODO
					}

					// Cast.
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK: // Previous Logged Event Stack.
				{
					// Validate.
					if (wLen < sizeof(CEL_CALLSTACK))
					{
						// Warning, Data Incomplete!
						// TODO
					}

					// Cast.
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					// Warning, Be Careful when writing out Data because it may not be full DWORDs.
					// The Last DWORD may be missing 1 to 3 bytes.
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		//case CELZONE_DEBUG:			// Debugging.
		//	switch (wID)
		//	{
		//		//case CELID_DEBUG_MSG:
		//		//{
		//		//	// Validate.
		//		//	if (wLen < sizeof(CEL_DEBUG_MSG))
		//		//	{
		//		//		// Warning, Data Incomplete!
		//		//		// TODO
		//		//	}

		//		//	// Cast.
		//		//	PCEL_DEBUG_MSG pCDM((PCEL_DEBUG_MSG)pData);
		//		//	break;
		//		//}

		//		case CELID_DEBUG_TRAP:
		//		{
		//			// Validate.
		//			if (wLen < sizeof(CEL_DEBUG_TRAP))
		//			{
		//				// Warning, Data Incomplete!
		//				// TODO
		//			}

		//			// Cast.
		//			//PCEL_DEBUG_TRAP pCDM((PCEL_DEBUG_TRAP)pData);
		//			break;
		//		}

		//		case CELID_FLAGGED:
		//		{
		//			PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
		//			break;
		//		}

		//		case CELID_DATA_LOSS:
		//		{
		//			PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
		//			break;
		//		}

		//		case CELID_CALLSTACK:
		//		{
		//			PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
		//			break;
		//		}

		//		case CELID_SYNC_END: // CeLogReSync() End
		//		{
		//			// pData[wLen]
		//			break;
		//		}

		//		default:
		//		{
		//			// pData[wLen]
		//			break;
		//		}
		//	}
		//	break;

		case CELZONE_BOOT_TIME:		// Boot Time.
			switch (wID)
			{
				case CELID_BOOT_TIME:
				{
					PCEL_BOOT_TIME pCBT = (PCEL_BOOT_TIME)pData;
					WORD nNameLength(wLen - sizeof(pCBT->dwAction));
					switch(pCBT->dwAction)
					{
						case BOOT_TIME_LAUNCHING_FS:
							break;

						case BOOT_TIME_FS_INITED:
							break;

						case BOOT_TIME_FS_OBJ_STORE_INITIALIZED:
							break;

						case BOOT_TIME_FS_FILES_INITIALIZED:
							break;

						case BOOT_TIME_FS_REG_INITIALIZED:
							break;

						case BOOT_TIME_FS_DB_INITIALIZED:
							break;

						case BOOT_TIME_FS_LAUNCH:
							break;

						case BOOT_TIME_DEV_ACTIVATE:
							break;

						case BOOT_TIME_DEV_FINISHED:
							break;

						case BOOT_TIME_GWES_FINISHED:
							break;

						case BOOT_TIME_SYSTEM_STARTED:
							break;

						default:
							break;
					}
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_INTERRUPT:		// Interrupts.
			switch (wID)
			{
				case CELID_INTERRUPTS:
				{
					PCEL_INTERRUPTS pCI((PCEL_INTERRUPTS)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_RESCHEDULE:	// The scheduler.
			switch (wID)
			{
				case CELID_THREAD_SWITCH:
				{
					PCEL_THREAD_SWITCH pCTS((PCEL_THREAD_SWITCH)pData);
					break;
				}

				case CELID_THREAD_SUSPEND:
				{
					PCEL_THREAD_SUSPEND pCTS((PCEL_THREAD_SUSPEND)pData);
					break;
				}

				case CELID_THREAD_RESUME:
				{
					PCEL_THREAD_RESUME pCTR((PCEL_THREAD_RESUME)pData);
					break;
				}

				//case CELID_THREAD_QUANTUMEXPIRE:
				//{
				//	break;
				//}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		//case CELZONE_IDLE:		// Idle processing. Reserved for future use.
		//	break;

		case CELZONE_TLB:			// The translation look-aside buffer (TLB).
			switch (wID)
			{
				case CELID_SYSTEM_TLB:
				{
					PCEL_SYSTEM_TLB pCST((PCEL_SYSTEM_TLB)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_DEMANDPAGE:	// Paging.
			switch (wID)
			{
				case CELID_SYSTEM_PAGE:
				{
					PCEL_SYSTEM_PAGE pCTS((PCEL_SYSTEM_PAGE)pData);
					break;
				}

				//case CELID_SYSTEM_PAGE_IN:	// 6.5
				//	break;

				//case CELID_SYSTEM_PAGE_OUT:
				//{
				//	break;
				//}

				//case CELID_MAPFILE_CREATE:
				//{
				//	PCEL_MAPFILE_CREATE pCMC((PCEL_MAPFILE_CREATE)pData);
				//	break;
				//}

				//case CELID_MAPFILE_DESTROY:
				//{
				//	PCEL_MAPFILE_DESTROY pCMD((PCEL_MAPFILE_DESTROY)pData);
				//	break;
				//}

				//case CELID_MAPFILE_FLUSH:		// FlushViewOfFile()
				//{
				//	PCEL_MAPFILE_FLUSH pCMF((PCEL_MAPFILE_FLUSH)pData);
				//	break;
				//}

				//case CELID_MAPFILE_VIEW_CLOSE:	// UnmapViewOfFile()
				//{
				//	PCEL_MAPFILE_VIEW_CLOSE pCMVC((PCEL_MAPFILE_VIEW_CLOSE)pData);
				//	break;
				//}

				//case CELID_MAPFILE_VIEW_OPEN:
				//{
				//	PCEL_MAPFILE_VIEW_OPEN pCMVO((PCEL_MAPFILE_VIEW_OPEN)pData);
				//	break;
				//}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_THREAD:		// Threads.
			switch (wID)
			{
				case CELID_THREAD_CREATE:
				{
					PCEL_THREAD_CREATE pCTC((PCEL_THREAD_CREATE)pData);
					break;
				}

				case CELID_THREAD_TERMINATE:
				{
					PCEL_THREAD_TERMINATE pCTT((PCEL_THREAD_TERMINATE)pData);
					break;
				}

				case CELID_THREAD_CLOSE:
				{
					PCEL_THREAD_CLOSE pCTC((PCEL_THREAD_CLOSE)pData);
					break;
				}

				case CELID_THREAD_DELETE:
				{
					PCEL_THREAD_DELETE pCTD((PCEL_THREAD_DELETE)pData);
					break;
				}

				case CELID_THREAD_PRIORITY:
				{
					PCEL_THREAD_PRIORITY pCTP((PCEL_THREAD_PRIORITY)pData);
					break;
				}

				case CELID_THREAD_QUANTUM:
				{
					PCEL_THREAD_QUANTUM pCTQ((PCEL_THREAD_QUANTUM)pData);
					break;
				}

				case CELID_MODULE_LOAD:
				{
					PCEL_MODULE_LOAD pML((PCEL_MODULE_LOAD)pData);
					break;
				}

				case CELID_MODULE_FREE:
				{
					PCEL_MODULE_FREE pMF((PCEL_MODULE_FREE)pData);
					break;
				}

				case CELID_PROCESS_CREATE:		// CreateProcess() or OpenProcess()
				{
					PCEL_PROCESS_CREATE pCPC((PCEL_PROCESS_CREATE)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_MIGRATE:		// Thread Migrate.
			switch (wID)
			{
				case CELID_THREAD_MIGRATE:
				{
					PCEL_THREAD_MIGRATE pCTM((PCEL_THREAD_MIGRATE)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_PROCESS:		// Processes.
			switch (wID)
			{
				case CELID_PROCESS_CREATE:
				{
					PCEL_PROCESS_CREATE pCPC((PCEL_PROCESS_CREATE)pData);
					break;
				}

				case CELID_PROCESS_TERMINATE:
				{
					PCEL_PROCESS_TERMINATE pCPT((PCEL_PROCESS_TERMINATE)pData);
					break;
				}

				case CELID_PROCESS_CLOSE:
				{
					PCEL_PROCESS_CLOSE pCPC((PCEL_PROCESS_CLOSE)pData);
					break;
				}

				case CELID_PROCESS_DELETE:
				{
					PCEL_PROCESS_DELETE pCPD((PCEL_PROCESS_DELETE)pData);
					break;
				}

				//case CELID_EXTRA_PROCESS_INFO:
				//{
				//	PCEL_EXTRA_PROCESS_INFO pCEMI((PCEL_EXTRA_PROCESS_INFO)pData);
				//	break;
				//}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_PRIORITYINV:	// Priority inversion.
			switch (wID)
			{
				case CELID_SYSTEM_INVERT:
				{
					PCEL_SYSTEM_INVERT pCSI((PCEL_SYSTEM_INVERT)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_CRITSECT:		// Critical sections.
			switch (wID)
			{
				//case CELID_CS_INIT:		// InitializeCriticalSection()
				//{
				//	PCEL_CRITSEC_INIT pCCI((PCEL_CRITSEC_INIT)pData);
				//	break;
				//}

				//case CELID_CS_DELETE:	// DeleteCriticalSection()
				//{
				//	PCEL_CRITSEC_DELETE pCCD((PCEL_CRITSEC_DELETE)pData);
				//	break;
				//}

				case CELID_CS_ENTER:	// EnterCriticalSection()
				{
					PCEL_CRITSEC_ENTER pCCE((PCEL_CRITSEC_ENTER)pData);
					break;
				}

				case CELID_CS_LEAVE:	// LeaveCriticalSection()
				{
					PCEL_CRITSEC_LEAVE pCCL((PCEL_CRITSEC_LEAVE)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_SYNCH:			// Synchronization.
			switch (wID)
			{
				case CELID_EVENT_CREATE:	// CreateEvent()
				{
					PCEL_EVENT_CREATE pCEC((PCEL_EVENT_CREATE)pData);
					break;
				}

				case CELID_EVENT_SET:		// SetEvent()
				{
					PCEL_EVENT_SET pCES((PCEL_EVENT_SET)pData);
					break;
				}

				case CELID_EVENT_RESET:		// ResetEvent()
				{
					PCEL_EVENT_RESET pCER((PCEL_EVENT_RESET)pData);
					break;
				}

				case CELID_EVENT_PULSE:		// PulseEvent()
				{
					PCEL_EVENT_PULSE pCEP((PCEL_EVENT_PULSE)pData);
					break;
				}

				case CELID_EVENT_CLOSE:
				{
					PCEL_EVENT_CLOSE pCEC((PCEL_EVENT_CLOSE)pData);
					break;
				}

				case CELID_EVENT_DELETE:
				{
					PCEL_EVENT_DELETE pCED((PCEL_EVENT_DELETE)pData);
					break;
				}

				case CELID_WAIT_MULTI:
				{
					PCEL_WAIT_MULTI pCWM((PCEL_WAIT_MULTI)pData);
					break;
				}

				case CELID_SLEEP:
				{
					PCEL_SLEEP pCS((PCEL_SLEEP)pData);
					break;
				}

				case CELID_SEM_CREATE:
				{
					PCEL_SEM_CREATE pCSC((PCEL_SEM_CREATE)pData);
					break;
				}

				case CELID_SEM_RELEASE:
				{
					PCEL_SEM_RELEASE pCSR((PCEL_SEM_RELEASE)pData);
					break;
				}

				case CELID_SEM_CLOSE:
				{
					PCEL_SEM_CLOSE pCSC((PCEL_SEM_CLOSE)pData);
					break;
				}
				case CELID_SEM_DELETE:
				{
					PCEL_SEM_DELETE pCSD((PCEL_SEM_DELETE)pData);
					break;
				}

				case CELID_MUTEX_CREATE:		// CreateMutex()
				{
					PCEL_MUTEX_CREATE pCMC((PCEL_MUTEX_CREATE)pData);
					break;
				}

				case CELID_MUTEX_CLOSE:
				{
					PCEL_MUTEX_CLOSE pCMC((PCEL_MUTEX_CLOSE)pData);
					break;
				}

				case CELID_MUTEX_RELEASE:		// ReleaseMutex()
				{
					PCEL_MUTEX_RELEASE pCMR((PCEL_MUTEX_RELEASE)pData);
					break;
				}

				case CELID_MUTEX_DELETE:
				{
					PCEL_MUTEX_DELETE pCMD((PCEL_MUTEX_DELETE)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_HEAP:			// Heaps.
			switch (wID)
			{
				case CELID_HEAP_CREATE:		// HeapCreate()
				{
					PCEL_HEAP_CREATE pCHC((PCEL_HEAP_CREATE)pData);
					break;
				}

				case CELID_HEAP_ALLOC:		// HeapAlloc()
				{
					PCEL_HEAP_ALLOC pCHA((PCEL_HEAP_ALLOC)pData);
					break;
				}

				case CELID_HEAP_REALLOC:
				{
					PCEL_HEAP_REALLOC pCHR((PCEL_HEAP_REALLOC)pData);
					break;
				}

				case CELID_HEAP_FREE:		// HeapFree()
				{
					PCEL_HEAP_FREE pCHF((PCEL_HEAP_FREE)pData);
					break;
				}

				case CELID_HEAP_DESTROY:	// HeapDestroy()
				{
					PCEL_HEAP_DESTROY pCHD((PCEL_HEAP_DESTROY)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_PROFILER:		// Profiler.
			switch (wID)
			{
				case CELID_MONTECARLO_HIT:
				{
					// pData should be a single DWORD.
					break;
				}

				case CELID_PROFILER_STOP:
				{
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_VIRTMEM:		// Virtual memory.
			switch (wID)
			{
				case CELID_VIRTUAL_ALLOC:
				{
					PCEL_VIRTUAL_ALLOC pCVA((PCEL_VIRTUAL_ALLOC)pData);
					break;
				}

				//case CELID_VIRTUAL_ALLOC_EX:	// 6.5
				//	break;

				case CELID_VIRTUAL_COPY:
				{
					PCEL_VIRTUAL_COPY pCVC((PCEL_VIRTUAL_COPY)pData);
					break;
				}

				case CELID_VIRTUAL_FREE:
				{
					PCEL_VIRTUAL_FREE pCVF((PCEL_VIRTUAL_FREE)pData);
					break;
				}

				//case CELID_VIRTUAL_FREE_EX:	// 6.5
				//	break;

				//case CELID_MAPFILE_FLUSH:		// FlushViewOfFile()
				//{
				//	PCEL_MAPFILE_FLUSH pCMF((PCEL_MAPFILE_FLUSH)pData);
				//	break;
				//}

				//case CELID_MAPFILE_VIEW_CLOSE:	// UnmapViewOfFile()
				//{
				//	PCEL_MAPFILE_VIEW_CLOSE pCMVC((PCEL_MAPFILE_VIEW_CLOSE)pData);
				//	break;
				//}

				//case CELID_MAPFILE_VIEW_OPEN:	// MapViewOfFile()
				//{
				//	PCEL_MAPFILE_VIEW_OPEN pCMVO((PCEL_MAPFILE_VIEW_OPEN)pData);
				//	break;
				//}

				//case CELID_MAPFILE_VIEW_OPEN_EX: // 6.5
				//	break;

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		//case CELZONE_CACHE:		// The cache. Reserved for future use.
		//	break;

		case CELZONE_LOADER:		// The loader.
			switch (wID)
			{
				case CELID_MODULE_LOAD:
				{
					PCEL_MODULE_LOAD pMF((PCEL_MODULE_LOAD)pData);
					break;
				}

				case CELID_MODULE_FREE:
				{
					PCEL_MODULE_FREE pMF((PCEL_MODULE_FREE)pData);
					break;
				}

				//case CELID_EXTRA_MODULE_INFO:
				//{
				//	PCEL_EXTRA_MODULE_INFO pCEMI((PCEL_EXTRA_MODULE_INFO)pData);
				//	break;
				//}

				//case CELID_EXTRA_PROCESS_INFO:
				//{
				//	PCEL_EXTRA_PROCESS_INFO pCEMI((PCEL_EXTRA_PROCESS_INFO)pData);
				//	break;
				//}

				//case CELID_MODULE_REFERENCES:
				//{
				//	PCEL_MODULE_REFERENCES pCMR((PCEL_MODULE_REFERENCES)pData);
				//	break;
				//}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_MEMTRACKING:	// Memory tracking. Reserved for future use.
			switch (wID)
			{
				case CELID_MEMTRACK_DETACHP:
				{
					PCEL_MEMTRACK_DETACHP pCMD((PCEL_MEMTRACK_DETACHP)pData);
					break;
				}

				case CELID_MEMTRACK_BASELINE:
				{
					PCEL_MEMTRACK_BASELINE pCMB((PCEL_MEMTRACK_BASELINE)pData);
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		//case CELZONE_RAWDATA:		// Capturing raw data.
		//	break;

		//case CELZONE_WINDOW:		// The Window Manager. Reserved for future use.
		//	break;

		//case CELZONE_MESSAGE:		// Window messages. Reserved for future use.
		//	break;

		case CELZONE_KCALL:			// KCALLs.
			switch (wID)
			{
				case CELID_KCALL_ENTER:
				{
					// pData[wLen]
					break;
				}

				//case CELID_KCALL_EXIT:
				case CELID_KCALL_LEAVE:
				{
					// pData[wLen]
					break;
				}

				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		case CELZONE_MISC:			// Events not belonging to a particular category.
			switch (wID)
			{
				case CELID_FLAGGED:
				{
					PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
					break;
				}

				case CELID_DATA_LOSS:
				{
					PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
					break;
				}

				case CELID_CALLSTACK:
				{
					PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
					break;
				}

				case CELID_SYNC_END: // CeLogReSync() End
				{
					// pData[wLen]
					break;
				}

				default:
				{
					// pData[wLen]
					break;
				}
			}
			break;

		//case CELZONE_RESERVED1:		// System Use Only.
		//	switch (wID)
		//	{
		//		case CELID_FLAGGED:
		//		{
		//			PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
		//			break;
		//		}

		//		case CELID_DATA_LOSS:
		//		{
		//			PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
		//			break;
		//		}

		//		case CELID_CALLSTACK:
		//		{
		//			PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
		//			break;
		//		}

		//		case CELID_SYNC_END: // CeLogReSync() End
		//		{
		//			// pData[wLen]
		//			break;
		//		}

		//		default:
		//		{
		//			// pData[wLen]
		//			break;
		//		}
		//	}
		//	break;

		//case CELZONE_RESERVED2:		// System Use Only.
		//	switch (wID)
		//	{
		//		case CELID_FLAGGED:
		//		{
		//			PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
		//			break;
		//		}

		//		case CELID_DATA_LOSS:
		//		{
		//			PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
		//			break;
		//		}

		//		case CELID_CALLSTACK:
		//		{
		//			PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
		//			break;
		//		}

		//		case CELID_SYNC_END: // CeLogReSync() End
		//		{
		//			// pData[wLen]
		//			break;
		//		}

		//		default:
		//		{
		//			// pData[wLen]
		//			break;
		//		}
		//	}
		//	break;

		default: // User Defined or Zoneless.
			switch (dwZoneUser)
			{
				// User Defined.
				case CELZONE_CELOG_VIEWER:
					switch (wID)
					{
						case CELID_HWND:
							g_hViewer = (HWND)pData;
							break;

						default:
							// Do Nothing.
							break;
					}
					break;

				// Unknown User Defined or Zoneless.
				default:
					switch (wID)
					{
						case CELID_RAW_LONG:
						{
							LONG nValue(*(PLONG)pData);
							break;
						}

						case CELID_RAW_ULONG:
						{
							ULONG nValue(*(ULONG*)pData);
							break;
						}

						case CELID_RAW_SHORT:
						{
							SHORT nValue(*(SHORT*)pData);
							break;
						}

						case CELID_RAW_USHORT:
						{
							USHORT nValue(*(USHORT*)pData);
							break;
						}

						case CELID_RAW_WCHAR:
						{
							WCHAR cValue(*(WCHAR*)pData);
							break;
						}

						case CELID_RAW_CHAR:
						{
							CHAR cValue(*(CHAR*)pData);
							break;
						}

						case CELID_RAW_UCHAR:
						{
							UCHAR cValue(*(UCHAR*)pData);
							break;
						}

						case CELID_RAW_FLOAT:
						{
							FLOAT nValue(*(FLOAT*)pData);
							break;
						}

						case CELID_RAW_DOUBLE:
						{
							DOUBLE nValue(*(DOUBLE*)pData);
							break;
						}

						//case CELID_CEPERF:
						//{
						//	//pData[wLen]
						//	break;
						//}

						//case CELID_TIMER_START:
						//{
						//	break;
						//}

						//case CELID_TIMER_STOP:
						//{
						//	break;
						//}

						case CELID_FLAGGED:
						{
							PCEL_FLAGGED pCF((PCEL_FLAGGED)pData);
							break;
						}

						case CELID_DATA_LOSS:
						{
							PCEL_DATA_LOSS pCDL((PCEL_DATA_LOSS)pData);
							break;
						}

						case CELID_CALLSTACK:
						{
							PCEL_CALLSTACK pCC((PCEL_CALLSTACK)pData);
							break;
						}

						case CELID_SYNC_END: // CeLogReSync() End
						{
							// pData[wLen]
							break;
						}
						default:
						{
							// pData[wLen]
							break;
						}
					}
					break;
			}
			break;
	}

	// Append the Remainder bytes.

	// TODO
}


void MyCeLogInterrupt(DWORD dwlogvalue)
{
	// Setup.
	CEL_INT_DATA cid = {0};
	cid.dwTimeStamp = GetTickCount();
	cid.wSysIntr = HIWORD(dwlogvalue);
	cid.wNestingLevel = LOWORD(dwlogvalue);
	// TODO
}


void MyCeLogSetZones(DWORD dwZoneUser, DWORD dwZoneCE, DWORD dwZoneProcess)
{
	// TODO
}


BOOL MyCeLogQueryZones(LPDWORD lpdwZoneUser, LPDWORD lpdwZoneCE, LPDWORD ldpwZoneProcess)
{
	// TODO
	return TRUE;
}
