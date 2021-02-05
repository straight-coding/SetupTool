/*
  UninstallerDlg.cpp
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#include "pch.h"
#include "framework.h"
#include "Uninstaller.h"
#include "UninstallerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUninstallerDlg dialog

CMutex CUninstallerDlg::m_Lock;
vector<CString> CUninstallerDlg::m_LogMessages;

CUninstallerDlg::CUninstallerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UNINSTALLER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nBusy = 0;
}

void CUninstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUninstallerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

BOOL CUninstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetCursor(LoadCursor(NULL, IDC_WAIT));

	m_imageHeader.LoadBitmap(IDB_BITMAP_HEADER);

	GetWindowRect(&m_rectInitPosition);

	Layout();

	m_Installer.SetLogger(Log);
	m_UninstallThread.SetLogger(Log);

	m_strJsonFile = CInstallUtil::GetTempFile(L"script_", L"json");
	CInstallUtil::ExtractBinResource(L"JSON", IDR_JSON, m_strJsonFile);
	m_Installer.AppendRecyle(m_strJsonFile);
	m_Installer.ParseScript(m_strJsonFile);

	m_strTitle.LoadString(IDS_APP_TITLE);
	m_strTitle = m_Installer.GetProductName() + " " + m_strTitle;
	SetWindowText(m_strTitle);

	UpdateData(FALSE);

	CString strFormat;
	strFormat.LoadStringW(IDS_CONFIRM);
	CString strWarn;
	strWarn.Format(strFormat, m_Installer.GetProductName() + L" @ " + CInstallUtil::GetCodeBase());

	int nArgs;
	CStringArray aryArguments;
	LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL != szArglist)
	{
		for (int i = 0; i < nArgs; i++)
		{
			TRACE(L"Argument#%d: %s\r\n", i, szArglist[i]);
			aryArguments.Add(szArglist[i]);
		}
	}
	LocalFree(szArglist);

	if ((aryArguments.GetCount() < 2) || (aryArguments[1] != L"silent"))
	{
		if (IDYES != ::MessageBox(NULL, strWarn, m_strTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_YESNO))
		{
			EndDialog(0);
			return TRUE;
		}
	}

	CString strRunning = m_Installer.GetRunning();
	if (!strRunning.IsEmpty())
	{
		strFormat.LoadStringW(IDS_STILL_RUNNING);
		strWarn.Format(strFormat, L"\r\n" + strRunning);

		::MessageBox(NULL, strWarn, m_strTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_OK);

		EndDialog(0);
		return TRUE;
	}

	InterlockedExchange(&m_nBusy, 1);

	if (m_UninstallThread.Start(&m_Installer))
	{
		SetTimer(1, 20, NULL);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUninstallerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		Layout();

		CDialogEx::OnPaint();
	}
}

HCURSOR CUninstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUninstallerDlg::OnOK()
{
	if (InterlockedAdd(&m_nBusy, 0) > 0)
		return;

	m_Installer.Cleanup();
	CDialogEx::OnOK();
}

void CUninstallerDlg::OnCancel()
{
	if (InterlockedAdd(&m_nBusy, 0) > 0)
		return;

	m_Installer.Cleanup();
	CDialogEx::OnCancel();
}

void CUninstallerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		if (m_UninstallThread.IsStopped())
		{
			KillTimer(nIDEvent);
			InterlockedExchange(&m_nBusy, 0);

			CString strSuccess;
			strSuccess.LoadString(IDS_SUCCEEDED);

			//::MessageBox(NULL, strSuccess, m_strTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_OK);

			CDialogEx::OnOK();
			return;
		}

		CSingleLock single(&m_Lock);
		single.Lock();
		for (size_t i = 0; i < m_LogMessages.size(); i++)
		{
			GetDlgItem(IDC_STATIC_LOG)->SetWindowText(m_LogMessages[i]);
		}
		m_LogMessages.clear();
		single.Unlock();
	}
}

void CUninstallerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 500;
	lpMMI->ptMinTrackSize.y = 100;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CUninstallerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = GetDlgItem(nIDCtl)->GetDC();

	CDC  memDC;
	if (memDC.CreateCompatibleDC(pDC) == FALSE)
	{
		ReleaseDC(pDC);
		return;
	}

	if (nIDCtl == IDB_INSTALL_HEADER)
	{
		CRect rcBar = lpDrawItemStruct->rcItem;

		BITMAP bm;
		m_imageHeader.GetBitmap(&bm);

		double dblRatio = (double)rcBar.Height() / bm.bmHeight;
		int nWidth = (int)(dblRatio * bm.bmWidth);

		CDC bmDC;
		bmDC.CreateCompatibleDC(pDC);
		CBitmap* pOldbmp = bmDC.SelectObject(&m_imageHeader);

		CBrush br(RGB(254, 254, 254));
		pDC->SelectObject(br);
		pDC->FillRect(rcBar, &br);

		pDC->SetStretchBltMode(HALFTONE);
		pDC->StretchBlt(0, 0, nWidth, rcBar.Height(), &bmDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

		CString strFormat;
		strFormat.LoadString(IDS_SUB_TITLE);

		CString strSubTitle;
		strSubTitle.Format(strFormat, m_Installer.GetProductName());

		CRect rectText;
		CSize sizeText = pDC->GetTextExtent(strSubTitle, strSubTitle.GetLength());

		CRect textBox(nWidth + 16, 0, rcBar.right, rcBar.bottom);
		pDC->DrawText(strSubTitle, -1, &textBox, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		bmDC.SelectObject(pOldbmp);
		ReleaseDC(pDC);

		return;
	}
	ReleaseDC(pDC);

	CDialogEx::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

BOOL CUninstallerDlg::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CUninstallerDlg::Layout()
{
	int nMargin = 8;
	int nLogHeight = 30;

	CRect rcDlg;
	GetClientRect(&rcDlg);
	GetDlgItem(IDB_INSTALL_HEADER)->SetWindowPos(this, 0, 0, rcDlg.Width(), 64, SWP_NOZORDER | SWP_SHOWWINDOW);

	CRect rcBar;
	GetDlgItem(IDB_INSTALL_HEADER)->GetClientRect(&rcBar);

	CRect rcLog;
	rcLog.top = rcBar.Height() + 3*nMargin;
	rcLog.bottom = rcLog.top + nLogHeight;
	rcLog.left = nMargin;
	rcLog.right = rcBar.Width() - nMargin;
	GetDlgItem(IDC_STATIC_LOG)->SetWindowPos(this, rcLog.left, rcLog.top, rcLog.Width(), rcLog.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
}

BOOL CUninstallerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CMenu* pSysMenu1 = GetSystemMenu(FALSE);
	if (InterlockedAdd(&m_nBusy, 0) > 0)
	{
		if (pSysMenu1 != NULL)
			pSysMenu1->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);

		SetCursor(LoadCursor(NULL, IDC_WAIT));
		return TRUE; // Prevent MFC changing the cursor
	}
	else
	{
		if (pSysMenu1 != NULL)
			pSysMenu1->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
	}

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void CUninstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_CLOSE)
	{
		if (InterlockedAdd(&m_nBusy, 0) > 0)
			return;
	}
	CDialogEx::OnSysCommand(nID, lParam);
}

BOOL CUninstallerDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		switch (pMsg->wParam)
		{
		case VK_F4:
		case VK_RETURN:
		case VK_ESCAPE:
			return TRUE;
		}
		break;
	}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
