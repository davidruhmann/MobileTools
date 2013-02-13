/* File:	ScannerDialog.cpp
 * Created: Jan 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"
#include "ScannerDialog.h"

//// ScannerDialog Class.
IMPLEMENT_DYNAMIC(ScannerDialog, CDialog)

//// Constructor.
ScannerDialog::ScannerDialog(CWnd* pParent /*=NULL*/) : CDialog(ScannerDialog::IDD, pParent)
{
	// Initialize member variables.
	m_sBarcode = _T("");
}


//// Destructor.
ScannerDialog::~ScannerDialog()
{
}


//// Perform the data swap between the dialog and member variables.
void ScannerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


//// Initialize dialog and controls.
BOOL ScannerDialog::OnInitDialog()
{
	// Call parent.
	// To override the default position and sizing
	//  it may be best to not call the parent.
	CDialog::OnInitDialog();

	// Perform custom setup after parent call to prevent
	//  them from being overwritten by the parent call.

	// Initialize control values.
	if (!m_sBarcode.IsEmpty())
	{
		SetDlgItemText(IDC_EDIT_BARCODE, m_sBarcode);
	}

	// Bring to Top.
	//SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_SHOWWINDOW);
	BringWindowToTop();

	// Make it an X button instead of OK.
	::SHDoneButton(GetSafeHwnd(), SHDB_SHOWCANCEL);

	// Set Focus to Edit Box.
	CWnd* pWnd = GetDlgItem(IDC_EDIT_BARCODE);
	if (pWnd)
	{
		::SetFocus(pWnd->GetSafeHwnd());
		return FALSE; // Focus set manually.
	}

	return TRUE; // No control received focus.
}


//// Message Handler Assignments.
BEGIN_MESSAGE_MAP(ScannerDialog, CDialog)
	ON_BN_CLICKED(IDOK, &ScannerDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &ScannerDialog::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT_BARCODE, &ScannerDialog::OnEnChangeEditBarcode)
END_MESSAGE_MAP()


//// Message Handlers.
void ScannerDialog::OnBnClickedOk()
{
	// Save the edit box string.
	GetDlgItemText(IDC_EDIT_BARCODE, m_sBarcode);

	// Validate input string.
	if (m_sBarcode.IsEmpty())
	{
		AfxMessageBox(IDS_ERROR_EMPTY, 0, 0);
		return;
	}

	// Call Parent.
	OnOK();
}


void ScannerDialog::OnBnClickedCancel()
{
	// Call Parent.
	OnCancel();
}


void ScannerDialog::OnEnChangeEditBarcode()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO Show suggestions based upon current command.
}
