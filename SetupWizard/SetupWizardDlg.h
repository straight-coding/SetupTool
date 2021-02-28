
// SetupWizardDlg.h : header file
//

#pragma once

#include "PageGeneral.h"
#include "PageLocation.h"
#include "PageCheckRunning.h"
#include "PageShortcuts.h"
#include "PageExecutor.h"
#include "PageCreater.h"

// CSetupWizardDlg dialog
class CSetupWizardDlg : public CPropertySheet
{
// Construction
public:
	CSetupWizardDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETUPWIZARD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CPageGeneral	m_PageGeneral;
	CPageLocation	m_PageLocation;
	CPageCheckRunning m_PageCheckRunning;
	CPageShortcuts	m_PageShortcuts;
	CPageExecutor	m_PageExecutor;
	CPageCreater	m_PageCreater;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
