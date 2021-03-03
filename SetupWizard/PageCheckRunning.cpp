// PageCheckRunning.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageCheckRunning.h"

// CPageCheckRunning

//IMPLEMENT_DYNAMIC(CPageCheckRunning, CPropertyPage)

CPageCheckRunning::CPageCheckRunning() : CPropertyPage(CPageCheckRunning::IDD)
{
}

CPageCheckRunning::~CPageCheckRunning()
{
}

BEGIN_MESSAGE_MAP(CPageCheckRunning, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CPageCheckRunning::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CPageCheckRunning::OnBnClickedButtonRemove)
	ON_LBN_SELCHANGE(IDC_LIST_FILES_CHECK, &CPageCheckRunning::OnLbnSelchangeListFilesCheck)
	ON_LBN_SELCHANGE(IDC_LIST_FILES_SRC, &CPageCheckRunning::OnLbnSelchangeListFilesSrc)
	ON_LBN_DBLCLK(IDC_LIST_FILES_SRC, &CPageCheckRunning::OnLbnDblclkListFilesSrc)
	ON_LBN_DBLCLK(IDC_LIST_FILES_CHECK, &CPageCheckRunning::OnLbnDblclkListFilesCheck)
END_MESSAGE_MAP()

void CPageCheckRunning::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILES_SRC, m_ctrlSrcFiles);
	DDX_Control(pDX, IDC_LIST_FILES_CHECK, m_ctrlCheckFiles);
}

// CPageCheckRunning message handlers

BOOL CPageCheckRunning::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageCheckRunning::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

	//sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
	//sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);

	//sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	ReloadList();
	return CPropertyPage::OnSetActive();
}

void CPageCheckRunning::ReloadList()
{
	map<CString, CString> selected;

	m_ctrlCheckFiles.ResetContent();
	vector<CString> files = GetCheckList();
	for (size_t i = 0; i < files.size(); i++)
	{
		m_ctrlCheckFiles.AddString(files[i]);
		selected[files[i]] = "";
	}

	CStringArray aryFileList;
	CString strFolder = GetValue(L"source_folder");
	if ((strFolder.Right(1) != "\\") && (strFolder.Right(1) != "\\"))
		strFolder += "\\";

	int nPrefixLen = strFolder.GetLength();
	CInstallUtil::ScanFolder(strFolder, L"*.exe", aryFileList);
	m_ctrlSrcFiles.ResetContent();
	for (int i = 0; i < aryFileList.GetCount(); i++)
	{
		TRACE(L"%s\r\n", aryFileList[i]);
		CString strPath = aryFileList[i].Mid(nPrefixLen);
		if (selected.find(strPath) == selected.end())
			m_ctrlSrcFiles.AddString(strPath);
	}

	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(m_ctrlSrcFiles.GetCurSel() >= 0);
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(m_ctrlCheckFiles.GetCurSel() >= 0);
}

BOOL CPageCheckRunning::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}

void CPageCheckRunning::OnBnClickedButtonAdd()
{
	int idx = m_ctrlSrcFiles.GetCurSel();
	if (idx >= 0)
	{
		CString strPath;
		m_ctrlSrcFiles.GetText(idx, strPath);
		AppendCheckList(strPath);
	}

	ReloadList();
}

void CPageCheckRunning::OnBnClickedButtonRemove()
{
	int idx = m_ctrlCheckFiles.GetCurSel();
	if (idx >= 0)
	{
		CString strPath;
		m_ctrlCheckFiles.GetText(idx, strPath);
		RemoveFromCheckList(strPath);
	}

	ReloadList();
}

void CPageCheckRunning::OnLbnSelchangeListFilesSrc()
{
	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(m_ctrlSrcFiles.GetCurSel() >= 0);
}

void CPageCheckRunning::OnLbnSelchangeListFilesCheck()
{
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(m_ctrlCheckFiles.GetCurSel() >= 0);
}

void CPageCheckRunning::OnLbnDblclkListFilesSrc()
{
	OnBnClickedButtonAdd();
}

void CPageCheckRunning::OnLbnDblclkListFilesCheck()
{
	OnBnClickedButtonRemove();
}
