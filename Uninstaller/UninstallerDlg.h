/*
  UninstallerDlg.h
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#pragma once

#include "UninstallThread.h"

// CUninstallerDlg dialog
class CUninstallerDlg : public CDialogEx
{
// Construction
public:
	CUninstallerDlg(CWnd* pParent = nullptr);	// standard constructor

	CString m_strTitle;

	void Layout();
	long m_nBusy;

	CString m_strJsonFile;
	CInstallUtil m_Installer;
	CUninstallThread m_UninstallThread;

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
	enum { IDD = IDD_UNINSTALLER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CBitmap m_imageHeader;
	CRect m_rectInitPosition;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
