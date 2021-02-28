
// SetupWizardDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "SetupWizard.h"
#include "SetupWizardDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CSetupWizardDlg dialog

CSetupWizardDlg::CSetupWizardDlg(CWnd* pParent /*=nullptr*/)
	: CPropertySheet(IDS_WIZARD_TITLE, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	SetupDefaultPairs();

	AddPage(&m_PageGeneral);
	AddPage(&m_PageLocation);
	AddPage(&m_PageCheckRunning);
	AddPage(&m_PageShortcuts);
	AddPage(&m_PageExecutor);
	AddPage(&m_PageCreater);

	SetWizardMode();
}

void CSetupWizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertySheet::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSetupWizardDlg, CPropertySheet)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CSetupWizardDlg message handlers

BOOL CSetupWizardDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CButton* btnCancel = reinterpret_cast<CButton*>(GetDlgItem(IDCANCEL));
	CButton* btnHelp = reinterpret_cast<CButton*>(GetDlgItem(IDHELP));
	CButton* btnBack = reinterpret_cast<CButton*>(GetDlgItem(ID_WIZBACK));
	CButton* btnNext = reinterpret_cast<CButton*>(GetDlgItem(ID_WIZNEXT));
	CButton* btnFinish = reinterpret_cast<CButton*>(GetDlgItem(ID_WIZFINISH));

	// Get a handle to the Help button
	CRect rectHelp;
	btnHelp->GetWindowRect(&rectHelp);
	ScreenToClient(&rectHelp);
	btnHelp->DestroyWindow();

	CRect rectBack;
	btnBack->GetWindowRect(&rectBack);
	ScreenToClient(&rectBack);

	CRect rectCancel;
	btnCancel->GetWindowRect(&rectCancel);
	ScreenToClient(&rectCancel);

	btnBack->SetWindowPos(NULL, 10, rectBack.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	btnNext->SetWindowPos(NULL, rectCancel.left, rectCancel.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	btnCancel->SetWindowPos(NULL, rectHelp.left, rectHelp.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	btnFinish->SetWindowPos(NULL, rectHelp.left, rectHelp.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

	GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSetupWizardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CPropertySheet::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSetupWizardDlg::OnPaint()
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
		CPropertySheet::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSetupWizardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CSetupWizardDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertySheet::PreTranslateMessage(pMsg);
}
