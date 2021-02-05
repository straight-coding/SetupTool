/*
  InstallerDlg.h
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#pragma once

#include "InstallThread.h"

// CInstallerDlg dialog
class CInstallerDlg : public CDialogEx
{
// Construction
public:
	CInstallerDlg(CWnd* pParent = nullptr);	// standard constructor

	CString m_strTitle;
	CRect m_rectInitPosition;
	long m_nBusy;

	void Layout();

	CString m_strJsonFile;
	CInstallUtil m_Installer;
	CInstallThread m_InstallThread;

	static CMutex m_Lock;
	static vector<CString> m_LogMessages;
public:

	static void Log(CString strFormat, ...)
	{
		CString s;

		va_list argList;
		va_start(argList, strFormat);
		s.FormatV(strFormat, argList);
		va_end(argList);

		CSingleLock single(&m_Lock);
		single.Lock();
			m_LogMessages.push_back(s);
		single.Unlock();
	}

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INSTALLER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CBitmap m_imageHeader;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonInstall();
	CListBox m_ctrlLogList;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	CString m_strInstallFolder;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
