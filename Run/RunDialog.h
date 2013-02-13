/* File:	RunDialog.h
 * Created: Dec 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

//// Compiler Directives.
#pragma once

//// Local Inclues.
#include "resourceppc.h"

//// Class Definition for CRunDialog.
class CRunDialog : public CDialog
{
	DECLARE_DYNAMIC(CRunDialog)
private:
	// Member Variables.
	CString m_sValue;

public:
	// Structors.
	CRunDialog(CWnd* pParent = NULL);
	virtual ~CRunDialog();

	// Member Access.
	CString GetValue() const                { return m_sValue; };
	void    SetValue(const CString& sValue) { m_sValue = sValue; };

	// Dialog Data.
	enum { IDD = IDD_DLG_RUN };

protected:
	// Function Prototypes.
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// Message Handlers.
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnChangeEditRun();
};
