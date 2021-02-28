// PageShortcuts.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageShortcuts.h"

// CPageShortcuts

//IMPLEMENT_DYNAMIC(CPageShortcuts, CPropertyPage)

CPageShortcuts::CPageShortcuts() : CPropertyPage(CPageShortcuts::IDD)
{

}

CPageShortcuts::~CPageShortcuts()
{
}

BEGIN_MESSAGE_MAP(CPageShortcuts, CPropertyPage)
END_MESSAGE_MAP()

// CPageShortcuts message handlers

BOOL CPageShortcuts::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageShortcuts::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
	sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_SHOW);

	sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_HIDE);

	return CPropertyPage::OnSetActive();
}

BOOL CPageShortcuts::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}


void CPageShortcuts::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
}
