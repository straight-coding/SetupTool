// PageExecutor.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageExecutor.h"

// CPageExecutor

//IMPLEMENT_DYNAMIC(CPageExecutor, CPropertyPage)

CPageExecutor::CPageExecutor() : CPropertyPage(CPageExecutor::IDD)
{
}

CPageExecutor::~CPageExecutor()
{
}

BEGIN_MESSAGE_MAP(CPageExecutor, CPropertyPage)
END_MESSAGE_MAP()

// CPageExecutor message handlers

BOOL CPageExecutor::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageExecutor::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
	sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);

	sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	return CPropertyPage::OnSetActive();
}

BOOL CPageExecutor::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}


void CPageExecutor::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
}
