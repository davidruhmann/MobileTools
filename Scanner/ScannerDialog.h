/* File:	ScannerDialog.h
 * Created: Jan 2012
 * Author:	David Ruhmann
 *
 * Copyright (c) 2012 David Ruhmann
 */

//// Compiler Directives.
#pragma once

//// Local Inclues.
#include "resourceppc.h"

//// Class Definition for ScannerDialog.
class ScannerDialog : public CDialog
{
	DECLARE_DYNAMIC(ScannerDialog)
private:
	// Member Variables.
	CString m_sBarcode;

public:
	// Structors.
	ScannerDialog(CWnd* pParent = NULL);
	virtual ~ScannerDialog();

	// Member Access.
	CString GetBarcode() const                  { return m_sBarcode; };
	void    SetBarcode(const CString& sBarcode) { m_sBarcode = sBarcode; };

	// Dialog Data.
	enum { IDD = IDD_SCANNERDLG };

protected:
	// Function Prototypes.
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// Message Handlers.
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnChangeEditBarcode();
};
