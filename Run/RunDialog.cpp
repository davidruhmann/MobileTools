/* File:	RunDialog.cpp
 * Created: Dec 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"
#include "RunDialog.h"

//// CRunDialog Class.
IMPLEMENT_DYNAMIC(CRunDialog, CDialog)

//// Constructor.
CRunDialog::CRunDialog(CWnd* pParent /* = NULL */) : CDialog(CRunDialog::IDD, pParent)
{
	// Initialize member variables.
	m_sValue = _T("");
}


//// Destructor.
CRunDialog::~CRunDialog()
{
}


//// Perform the data swap between the dialog and member variables.
void CRunDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


//// Initialize dialog and controls.
BOOL CRunDialog::OnInitDialog()
{
	// Call parent.
	// To override the default position and sizing
	//  it may be best to not call the parent.
	CDialog::OnInitDialog();

	// Perform custom setup after parent call to prevent
	//  them from being overwritten by the parent call.

	// Initialize control values.
	if (!m_sValue.IsEmpty())
	{
		SetDlgItemText(IDC_EDIT_RUN, m_sValue);
	}

	// Bring to Top.
	//SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_SHOWWINDOW);
	BringWindowToTop();

	// Make it an X button instead of OK.
	::SHDoneButton(GetSafeHwnd(), SHDB_SHOWCANCEL);

	// Set Focus to Edit Box.
	CWnd* pWnd = GetDlgItem(IDC_EDIT_RUN);
	if (pWnd)
	{
		::SetFocus(pWnd->GetSafeHwnd());
		return FALSE; // Focus set manually.
	}

	return TRUE; // No control received focus.
}


//// Message Handler Assignments.
BEGIN_MESSAGE_MAP(CRunDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CRunDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CRunDialog::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT_RUN, &CRunDialog::OnEnChangeEditRun)
END_MESSAGE_MAP()


//// Message Handlers.
void CRunDialog::OnBnClickedOk()
{
	// Save the edit box string.
	GetDlgItemText(IDC_EDIT_RUN, m_sValue);

	// Validate input string.
	if (m_sValue.IsEmpty())
	{
		AfxMessageBox(IDS_ERROR_EMPTY, 0, 0);
		return;
	}

	// Call Parent.
	OnOK();
}


void CRunDialog::OnBnClickedCancel()
{
	// Call Parent.
	OnCancel();
}


void CRunDialog::OnEnChangeEditRun()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO Show suggestions based upon current command.
}
