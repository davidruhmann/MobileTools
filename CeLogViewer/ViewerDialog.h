/* File:	ViewerDialog.h
 * Created: June 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

//// Compiler Directives.
#pragma once

//// Local Inclues.
#include "resource.h"

//// Class Definition for ViewerDialog.
class ViewerDialog : public CDialog
{
	DECLARE_DYNAMIC(ViewerDialog)

private:
	// Member Variables.
	CListCtrl m_ListCtrl;

public:
	// Structors.
	ViewerDialog(CWnd* pParent = NULL);
	virtual ~ViewerDialog();

	// Dialog Data.
	enum { IDD = IDD_VIEWER };

protected:
	// Function Prototypes.
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// Message Handlers.
	afx_msg void OnBnClickedOk();
	afx_msg void OnLvnItemchangedListlog(NMHDR *pNMHDR, LRESULT *pResult);
};
