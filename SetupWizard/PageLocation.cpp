// PageLocation.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageLocation.h"

// CPageLocation

//IMPLEMENT_DYNAMIC(CPageLocation, CPropertyPage)

CPageLocation::CPageLocation() : CPropertyPage(CPageLocation::IDD)
, m_strSourceFolder(_T(""))
, m_strTargetFolder(_T(""))
{
}

CPageLocation::~CPageLocation()
{
}

BEGIN_MESSAGE_MAP(CPageLocation, CPropertyPage)
END_MESSAGE_MAP()

// CPageLocation message handlers

BOOL CPageLocation::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageLocation::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
	sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);

	sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	m_strTargetFolder = GetValue(L"install_folder");
	UpdateData(FALSE);

	return CPropertyPage::OnSetActive();
}

BOOL CPageLocation::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData(TRUE);

	SetValue(L"install_folder", m_strTargetFolder);

	return CPropertyPage::OnKillActive();
}


void CPageLocation::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SRC_FOLDER, m_strSourceFolder);
	DDX_Text(pDX, IDC_EDIT_FOLDER, m_strTargetFolder);
}
