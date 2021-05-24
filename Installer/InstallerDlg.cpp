/*
  InstallerDlg.cpp
  Author: Straight Coder<simpleisrobust@gmail.com>
  Date: Feb. 04, 2021
*/

#include "pch.h"
#include "framework.h"
#include "Installer.h"
#include "InstallerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CInstallerDlg dialog

CInstallerDlg::CInstallerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INSTALLER_DIALOG, pParent)
	, m_strInstallFolder(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nBusy = 0;
}

void CInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_ctrlLogList);
	DDX_Text(pDX, IDC_EDIT_FOLDER, m_strInstallFolder);
}

BEGIN_MESSAGE_MAP(CInstallerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CInstallerDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_INSTALL, &CInstallerDlg::OnBnClickedButtonInstall)
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_GETMINMAXINFO()
	ON_WM_TIMER()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CInstallerDlg message handlers
CMutex CInstallerDlg::m_Lock;
vector<CString> CInstallerDlg::m_LogMessages;

BOOL CInstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_imageHeader.LoadBitmap(IDB_BITMAP_HEADER);

	GetWindowRect(&m_rectInitPosition);

	Layout();

	m_Installer.SetLogger(Log);
	m_InstallThread.SetLogger(Log);

	m_strJsonFile = CInstallUtil::GetTempFile(L"script_", L"json");
	CInstallUtil::ExtractBinResource(L"JSON", IDR_JSON, m_strJsonFile);

	m_Installer.AppendRecyle(m_strJsonFile);
	m_Installer.ParseScript(m_strJsonFile);
	m_strInstallFolder = m_Installer.GetInstallFolder();

	m_strTitle.LoadStringW(IDS_APP_TITLE);
	m_strTitle = m_Installer.GetProductName() + " " + m_strTitle;
	SetWindowText(m_strTitle);

	UpdateData(FALSE);

	CString strRunning = m_Installer.GetRunning();
	if (!strRunning.IsEmpty())
	{
		CString strFormat, strWarn;

		strFormat.LoadStringW(IDS_STILL_RUNNING);
		strWarn.Format(strFormat, L"\r\n" + strRunning);

		::MessageBox(NULL, strWarn, m_strTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_OK);

		EndDialog(0);
		return TRUE;
	}

	CString strUninstaller = m_Installer.GetUninstaller();
	if (!strUninstaller.IsEmpty())
	{
		CString strFormat;
		strFormat.LoadStringW(IDS_CONFIRM);
		CString strWarn;
		strWarn.Format(strFormat, m_Installer.GetProductName());// +L" @ " + CInstallUtil::GetCodeBase());

		if (IDYES != ::MessageBox(NULL, strWarn, m_strTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_YESNO))
		{
			EndDialog(0);
			return TRUE;
		}

		InterlockedExchange(&m_nBusy, 1);
		CInstallUtil::ExecuteCommandLine(strUninstaller + L" silent", INFINITE, TRUE, TRUE);
		SetTimer(3, 2000, NULL);
	}

	SetTimer(2, 100, NULL);

	CenterWindow();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CInstallerDlg::OnPaint()
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

HCURSOR CInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CInstallerDlg::OnOK()
{
	m_Installer.Cleanup();
	CDialogEx::OnOK();
}

void CInstallerDlg::OnCancel()
{
	m_Installer.Cleanup();
	CDialogEx::OnCancel();
}

void CInstallerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
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
		strFormat.LoadStringW(IDS_SUB_TITLE);

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

BOOL CInstallerDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect r;

	////GetClientRect gets the width & height of the client area of the Dialog
	//GetClientRect(&r);
	//CBrush br(RGB(254, 254, 254));
	//pDC->SelectObject(br);
	//pDC->FillRect(r, &br);

	return CDialogEx::OnEraseBkgnd(pDC);
}

HBRUSH CInstallerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		//pDC->SetTextColor(RGB(255, 0, 0));
		//return (HBRUSH)GetStockObject(NULL_BRUSH);
	default:
		break;
	}

	return hbr;
}

void CInstallerDlg::Layout()
{
	int nMargin = 8;
	int nButtonWidth = 120;
	int nButtonHeight = 40;
	int nEditHeight = 30;

	CRect rcDlg;
	GetClientRect(&rcDlg);
	GetDlgItem(IDB_INSTALL_HEADER)->SetWindowPos(this, 0, 0, rcDlg.Width(), 64, SWP_NOZORDER | SWP_SHOWWINDOW);

	CRect rcBar;
	GetDlgItem(IDB_INSTALL_HEADER)->GetClientRect(&rcBar);

	CRect rcBrowse;
	rcBrowse.top = rcBar.Height() + nMargin;
	rcBrowse.bottom = rcBrowse.top + nButtonHeight;
	rcBrowse.left = nMargin;
	rcBrowse.right = nButtonWidth + nMargin;
	GetDlgItem(IDC_BUTTON_BROWSE)->SetWindowPos(this, rcBrowse.left, rcBrowse.top, rcBrowse.Width(), rcBrowse.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

	CRect rcInstall;
	rcInstall.top = rcBar.Height() + nMargin;
	rcInstall.bottom = rcInstall.top + nButtonHeight;
	rcInstall.right = rcDlg.Width() - nMargin;
	rcInstall.left = rcInstall.right - nButtonWidth;
	GetDlgItem(IDC_BUTTON_INSTALL)->SetWindowPos(this, rcInstall.left, rcInstall.top, rcInstall.Width(), rcInstall.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
	
	CRect rcFolder;
	rcFolder.top = rcBar.Height() + nMargin + (nButtonHeight - nEditHeight)/2;
	rcFolder.bottom = rcFolder.top + nEditHeight;
	rcFolder.left = rcBrowse.Width() + 2*nMargin;
	rcFolder.right = rcDlg.Width() - rcInstall.Width() - 2*nMargin;
	GetDlgItem(IDC_EDIT_FOLDER)->SetWindowPos(this, rcFolder.left, rcFolder.top, rcFolder.Width(), rcFolder.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

	CRect rcLog;
	rcLog.top = rcBar.Height() + nButtonHeight + 2*nMargin;
	rcLog.bottom = rcDlg.Height() - nMargin;
	rcLog.left = nMargin;
	rcLog.right = rcDlg.Width() - nMargin;
	GetDlgItem(IDC_LIST_LOG)->SetWindowPos(this, rcLog.left, rcLog.top, rcLog.Width(), rcLog.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
}

void CInstallerDlg::OnBnClickedButtonBrowse()
{
	CFolderPickerDialog dlg;
	CString strFolder;
	GetDlgItem(IDC_EDIT_FOLDER)->GetWindowTextW(strFolder);

	dlg.m_ofn.lpstrTitle = _T("Select Folder to Install");
	dlg.m_ofn.lpstrInitialDir = strFolder;
	if (dlg.DoModal() == IDOK) 
	{
		strFolder = dlg.GetPathName();
		strFolder += _T("\\");

		GetDlgItem(IDC_EDIT_FOLDER)->SetWindowTextW(strFolder);
	}
}

void CInstallerDlg::OnBnClickedButtonInstall()
{
	UpdateData(TRUE);

	CString strFormat;

	strFormat.LoadStringW(IDS_LOG_SHORTCUT);
	m_Installer.SetLogPrefix(L"shortcut", strFormat);

	strFormat.LoadStringW(IDS_LOG_REGISTRY);
	m_Installer.SetLogPrefix(L"registry", strFormat);

	strFormat.LoadStringW(IDS_LOG_EXECUTE);
	m_Installer.SetLogPrefix(L"execute", strFormat);

	m_Installer.SetInstallFolder(m_strInstallFolder);

	CString strRunning = m_Installer.GetRunning();
	if (!strRunning.IsEmpty())
	{
		CString strFormat;
		strFormat.LoadStringW(IDS_STILL_RUNNING);

		CString strWarn;
		strWarn.Format(strFormat, L"\r\n" + strRunning);

		::MessageBox(NULL, strWarn, m_strTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_OK);
		return;
	}

	InterlockedExchange(&m_nBusy, 1);

	GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_INSTALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_FOLDER)->EnableWindow(FALSE);

	CString strZipFile = CInstallUtil::GetTempFile(L"pack-", L".zip");
	m_Installer.AppendRecyle(strZipFile);
	m_InstallThread.SetUnzipInfo(L"ZIP", IDR_ZIP, strZipFile);
	if (m_InstallThread.Start(&m_Installer))
	{
		SetTimer(1, 20, NULL);
	}
}

void CInstallerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	Invalidate(TRUE);
}

BOOL CInstallerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CMenu* pSysMenu1 = GetSystemMenu(FALSE);
	if (InterlockedAdd(&m_nBusy, 0) > 0)
	{
		if (pSysMenu1 != NULL)
			pSysMenu1->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);

		GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_INSTALL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FOLDER)->EnableWindow(FALSE);

		SetCursor(LoadCursor(NULL, IDC_WAIT));
		return TRUE; // Prevent MFC changing the cursor
	}
	else
	{
		if (pSysMenu1 != NULL)
			pSysMenu1->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);

		GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_FOLDER)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_INSTALL)->EnableWindow(TRUE);
	}

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void CInstallerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = max(600, m_rectInitPosition.Width());
	lpMMI->ptMinTrackSize.y = max(450, m_rectInitPosition.Height());

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CInstallerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		if (m_InstallThread.IsStopped())
		{
			KillTimer(nIDEvent);
			InterlockedExchange(&m_nBusy, 0);

			CString strSuccess;
			strSuccess.LoadString(IDS_SUCCEEDED);

			::MessageBox(NULL, strSuccess, m_strTitle,
				MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_OK);

			CDialogEx::OnOK();
		}
	}
	else if (nIDEvent == 2)
	{
		CSingleLock single(&m_Lock);
		single.Lock();
			for (size_t i = 0; i < m_LogMessages.size(); i++)
			{
				m_ctrlLogList.AddString(m_LogMessages[i]);
			}
			m_LogMessages.clear();
		single.Unlock();

		m_ctrlLogList.SetCurSel(m_ctrlLogList.GetCount()-1);
	}
	else if (nIDEvent == 3)
	{
		KillTimer(nIDEvent);

		InterlockedExchange(&m_nBusy, 0);
		Invalidate(TRUE);
	}
}

BOOL CInstallerDlg::PreTranslateMessage(MSG* pMsg)
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

void CInstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_CLOSE)
	{
		if (InterlockedAdd(&m_nBusy, 0) > 0)
			return;
	}
	CDialogEx::OnSysCommand(nID, lParam);
}
