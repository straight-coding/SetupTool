// CPageCreater.cpp : implementation file
//

#include "pch.h"
#include "SetupWizard.h"
#include "PageCreater.h"

// CPageCreater

//IMPLEMENT_DYNAMIC(CPageCreater, CPropertyPage)

CPageCreater::CPageCreater() : CPropertyPage(CPageCreater::IDD)
{
}

CPageCreater::~CPageCreater()
{
}

BEGIN_MESSAGE_MAP(CPageCreater, CPropertyPage)
END_MESSAGE_MAP()

// CPageCreater message handlers

BOOL CPageCreater::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageCreater::OnSetActive()
{
	// TODO: Add your specialized code here and/or call the base class
	CPropertySheet* sheet = (CPropertySheet*)GetParent();
	ASSERT(sheet);

	sheet->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);
	sheet->GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	sheet->GetDlgItem(ID_WIZNEXT)->ShowWindow(SW_HIDE);

	sheet->GetDlgItem(ID_WIZFINISH)->ShowWindow(SW_SHOW);
	sheet->SetFinishText(_T("Done"));

	return CPropertyPage::OnSetActive();
}

BOOL CPageCreater::OnKillActive()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnKillActive();
}



void CPageCreater::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class

	CPropertyPage::DoDataExchange(pDX);
}
