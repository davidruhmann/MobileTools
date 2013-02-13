/* File:	ViewerDialog.cpp
 * Created: June 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

//// External Includes (see stdafx.h).

//// Local Includes.
#include "stdafx.h"
#include "ViewerDialog.h"

//// ScannerDialog Class.
IMPLEMENT_DYNAMIC(ViewerDialog, CDialog)

//// Constructor.
ViewerDialog::ViewerDialog(CWnd* pParent /*=NULL*/) : CDialog(ViewerDialog::IDD, pParent)
{
	// Initialize member variables.
	m_ListCtrl.DeleteAllItems();
}


//// Destructor.
ViewerDialog::~ViewerDialog()
{
}


//// Perform the data swap between the dialog and member variables.
void ViewerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTLOG, m_ListCtrl);
}


//// Initialize dialog and controls.
BOOL ViewerDialog::OnInitDialog()
{
	// Call parent.
	// To override the default position and sizing
	//  it may be best to not call the parent.
	CDialog::OnInitDialog();

	// Perform custom setup after parent call to prevent
	//  them from being overwritten by the parent call.

	// Initialize control values.

	// Bring to Top.
	//SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_SHOWWINDOW);
	BringWindowToTop();

	// Make it an X button instead of OK.
	::SHDoneButton(GetSafeHwnd(), SHDB_SHOWCANCEL);

	return TRUE; // No control received focus.
}


//// Message Handler Assignments.
BEGIN_MESSAGE_MAP(ViewerDialog, CDialog)
	ON_BN_CLICKED(IDOK, &ViewerDialog::OnBnClickedOk)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTLOG, &ViewerDialog::OnLvnItemchangedListlog)
END_MESSAGE_MAP()


//// Message Handlers.
void ViewerDialog::OnBnClickedOk()
{
	// Call Parent.
	OnOK();
}


void ViewerDialog::OnLvnItemchangedListlog(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
