// PageGeneral.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageGeneral.h"

// CPageGeneral

//IMPLEMENT_DYNAMIC(CPageGeneral, CPropertyPage)

CPageGeneral::CPageGeneral() : CPropertyPage(CPageGeneral::IDD)
, m_strProductName(_T(""))
, m_strPublisher(_T(""))
, m_strProductVersion(_T(""))
{
}

CPageGeneral::~CPageGeneral()
{
}

BEGIN_MESSAGE_MAP(CPageGeneral, CPropertyPage)
END_MESSAGE_MAP()

// CPageGeneral message handlers

BOOL CPageGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ctrlPlatform.AddString(L"x86");
	m_ctrlPlatform.AddString(L"x64");
	m_ctrlPlatform.SetCurSel(0);

	m_ctrlLanguage.AddString(L"en");
	m_ctrlLanguage.AddString(L"zh");
	m_ctrlLanguage.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageGeneral::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);
	sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);
	//sheet->GetDlgItem(ID_WIZNEXT)->EnableWindow(FALSE);

	m_strProductName = GetValue(L"product_name");
	m_strProductVersion = GetValue(L"product_version");
	m_strPublisher = GetValue(L"publisher");

	m_ctrlPlatform.ResetContent();
	m_ctrlPlatform.AddString(L"x86");
	m_ctrlPlatform.AddString(L"x64");
	CString strPlatform = GetValue(L"platform");
	int idx = m_ctrlPlatform.FindString(-1, strPlatform);
	if (idx >= 0)
		m_ctrlPlatform.SetCurSel(idx);
	else
		m_ctrlPlatform.SetCurSel(0);

	m_ctrlLanguage.ResetContent();
	m_ctrlLanguage.AddString(L"en");
	m_ctrlLanguage.AddString(L"zh");
	CString strLanguage = GetValue(L"language");
	idx = m_ctrlLanguage.FindString(-1, strLanguage);
	if (idx >= 0)
		m_ctrlLanguage.SetCurSel(idx);
	else
		m_ctrlLanguage.SetCurSel(0);

	UpdateData(FALSE);

	return CPropertyPage::OnSetActive();
}

BOOL CPageGeneral::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData(TRUE);

	SetValue(L"product_name", m_strProductName);
	SetValue(L"product_version", m_strProductVersion);
	SetValue(L"publisher", m_strPublisher);

	CString strPlatform = L"x86";
	m_ctrlPlatform.GetLBText(m_ctrlPlatform.GetCurSel(), strPlatform);
	SetValue(L"platform", strPlatform);

	CString strLanguage = L"en";
	m_ctrlLanguage.GetLBText(m_ctrlLanguage.GetCurSel(), strLanguage);
	SetValue(L"language", strLanguage);

	CString strDefaultPath = L"%SystemDrive%/%programfiles(x86)%/%product_name%";
	if (strPlatform == L"x64")
		strDefaultPath = L"%SystemDrive%/%programfiles%/%product_name%";
	SetValue(L"install_folder", strDefaultPath);

	return CPropertyPage::OnKillActive();
}


void CPageGeneral::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strProductName);
	DDX_Text(pDX, IDC_EDIT_PUBLISHER, m_strPublisher);
	DDX_Control(pDX, IDC_COMBO_PLATFORM, m_ctrlPlatform);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_ctrlLanguage);
	DDX_Text(pDX, IDC_EDIT_VERSION, m_strProductVersion);
}
